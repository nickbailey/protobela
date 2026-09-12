#include <atomic>
#include <cstdint>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <RtAudio.h>
#include <vector>

#include "BelaContext.h"
#include "digitalinputsim.h"

// Forward declare the user-implemented Bela hooks
bool setup(BelaContext *context, void *userData);
void render(BelaContext *context, void *userData);
void cleanup(BelaContext *context, void *userData);

// Expose the thread-safe atomic tracker owned by digitalinputsim
extern std::atomic<uint32_t> gSimulatedDigitalWord;

// Persistent heap-allocated memory arrays for safe I/O tracking
static std::unique_ptr<uint32_t[]> gDigitalInputBuffer;
static std::unique_ptr<uint32_t[]> gDigitalOutputBuffer;

// RtAudio callback routing
int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData)
{
    // Snapshot the atomic value once per block to prevent thread contention mid-loop
    uint32_t currentInputSnapshot = static_cast<uint32_t>(gSimulatedDigitalWord.load());

    // Fast SIMD-optimized fill directly on the safely-allocated pointer block
    std::fill_n(gDigitalInputBuffer.get(), nBufferFrames, currentInputSnapshot);

    // Clear output state words cleanly for the current frame iteration block
    std::fill_n(gDigitalOutputBuffer.get(), nBufferFrames, 0);

    BelaContext context;
    context.audioIn = (const float*)inputBuffer;
    context.audioOut = (float*)outputBuffer;
    context.audioInChannels = 2;
    context.audioOutChannels = 2;
    context.audioFrames = nBufferFrames;
    context.audioSampleRate = 44100.0;

    // Analogue stubs
    context.analogFrames = nBufferFrames / 2;
    context.analogInChannels = 0;
    context.analogOutChannels = 0;
    context.analogIn = nullptr;
    context.analogOut = nullptr;
    context.analogSampleRate = 22050.0;

    // Digital implementation
    context.digitalFrames = nBufferFrames;
    context.digitalChannels = 16; // Standard physical Bela I/O layout width
    context.digitalIn = gDigitalInputBuffer.get();
    context.digitalOut = gDigitalOutputBuffer.get();

    // Call the portable user Bela loop
    render(&context, nullptr);
    return 0;
}

int main(int argc, char* argv[]) {

    // Default keys for digital I/O simulator
    std::string keysToMonitor("asdf");
    char quitKey = 'q'; // Default quit shortcut

    // 2. Configure getopt_long option structures
    // Add other application flags here in the future
    static struct option long_options[] = {
        {"keys",    required_argument, 0, 'k'},
        {"quit",    required_argument, 0, 'q'},
        {0,         0,                 0,  0 } // Array terminator
    };

    // Reset getopt internal index pointers in case other parsing occurred
    optind = 1;
    int option_index = 0;
    int c;

    // Parse command line arguments.
    // ":" tells getopt to ignore default stdout error prints so we don't pollute the terminal
    // String "k:q:" signals that both flags expect arguments
    while ((c = getopt_long(argc, argv, "k:q:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'k':
                if (optarg) keysToMonitor = optarg;
                break;
            case 'q':
                if (optarg && optarg[0] != '\0') {
                    quitKey = optarg[0]; // Extract the first character
                }
                break;
            default:
                break;
        }
    }

    // Start a background thread to monitor keystrokes and simulate the toggling of
    // digital input lines.
    DigitalInputSimulator inputSimulator(keysToMonitor, quitKey);
    inputSimulator.start();

    // Explicitly target the selected desktop sound driver framework chosen at compile time
    RtAudio dac(RtAudio::LINUX_PULSE);

    std::vector<unsigned int> deviceIds = dac.getDeviceIds();
    if (deviceIds.empty()) {
        std::cerr << "No audio devices found!\n";
        return 1;
    }

    const size_t devices(deviceIds.size());
    std::cout << "Found " << devices << " devices:\n";
    for (auto i : deviceIds) {
        RtAudio::DeviceInfo info(dac.getDeviceInfo(i));
        std::cout << "Index/ID: " << i
        << " -> Name: " << info.name
        << " (Inputs: " << info.inputChannels
        << ", Outputs: " << info.outputChannels << ")\n";
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;

    unsigned int bufferFrames = 128; // Requested block size
    unsigned int sampleRate = 44100;

    // Open stream first. RtAudio will update the 'bufferFrames' reference
    // variable if the sound card demands a different buffer size!
    RtAudioErrorType error;
    error = dac.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &rtAudioCallback);
    if (error != RTAUDIO_NO_ERROR) {
        std::cerr << "RtAudio Error: Failed to open stream (Code: " << error << ")\n";
        return 1;
    }

    // Allocation happens AFTER openStream so arrays scale to match exactly what the sound card uses
    gDigitalInputBuffer = std::make_unique<uint32_t[]>(bufferFrames);
    gDigitalOutputBuffer = std::make_unique<uint32_t[]>(bufferFrames);

    // Call the user setup function with runtime-validated structural frames
    BelaContext initialContext = {
        nullptr, nullptr, 2, 2, bufferFrames, static_cast<float>(sampleRate),
        0, 0, bufferFrames / 2, nullptr, nullptr, 22050.0f,
        16, bufferFrames, nullptr, nullptr
    };

    if(!setup(&initialContext, nullptr)) {
        std::cerr << "Bela setup failed.\n";
        return 1;
    }

    error = dac.startStream();
    if (error != RTAUDIO_NO_ERROR) {
        std::cerr << "RtAudio Error: Failed to start stream (Code: " << error << ")\n";
        if (dac.isStreamOpen()) dac.closeStream();
        return 1;
    }

    std::cout << "Emulating Bela audio loop (RtAudio v6+ No-Exceptions Engine).\n";
    std::cout << "Using Device ID: " << parameters.deviceId << " with Buffer size: " << bufferFrames << "\n";
    std::cout << "Monitoring Latching Toggle Pins using Keys: [" << keysToMonitor << "]\n";
    std::cout << "[" << quitKey << "] to quit.\n";

    inputSimulator.join();

    std::cout << "Halting audio engine components safely...\n";
    dac.stopStream();
    if (dac.isStreamOpen()) dac.closeStream();

    cleanup(&initialContext, nullptr);
    std::cout << "[Bela Engine] Shutdown complete.\n";
    return 0;
}
