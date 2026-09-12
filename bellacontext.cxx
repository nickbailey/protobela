#include <atomic>
#include <cstdint>
#include <iostream>
#include <RtAudio.h>
#include <vector>
#include "BelaContext.h"

// Forward declare the user-implemented Bela hooks (from render.cpp)
bool setup(BelaContext *context, void *userData);
void render(BelaContext *context, void *userData);
void cleanup(BelaContext *context, void *userData);

// Buffers to represent digital and ananlogue I/O
static uint32_t* digitalInputBuffer;
// To be implemented as requred. Don't forget to allocate them!'
//static uint32_t* digitalOutputBuffer;
//static float* analogueInputBuffer;
//static float* anlogueOutputBuffer;

// RtAudio callback routing
int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData)
{
    BelaContext context;
    context.audioIn = (const float*)inputBuffer;
    context.audioOut = (float*)outputBuffer;
    context.audioInChannels = 2;   // Mirror standard Bela hardware
    context.audioOutChannels = 2;
    context.audioFrames = nBufferFrames;
    context.audioSampleRate = 44100.0; // Target sample rate
    context.analogFrames = 0; // Not currently implemented
    context.analogInChannels = 0;
    context.analogOutChannels = 0;
    context.analogIn = nullptr;
    context.analogOut = nullptr;
    context.digitalFrames = nBufferFrames;
    context.digitalChannels = 4; // set this somewhere sensible!
    context.digitalIn = digitalInputBuffer;
    context.digitalOut = nullptr; // not yet implemented

    // Create a local buffer for this block duration
    std::vector<uint32_t> blockDigitalBuffer(nBufferFrames);

    // Snapshot the atomic value once per block to avoid thread contention mid-loop
    extern std::atomic<int> gMasterDigitalIn;
    uint32_t currentInputSnapshot = gMasterDigitalIn.load();

    // Fill the frame block with the snapshot state
    // Fill the raw array directly using the pointer.
    // This executes at maximum hardware speed.
    std::fill_n(digitalInputBuffer, nBufferFrames, currentInputSnapshot);

    // Call your portable Bela engine loop
    render(&context, nullptr);
    return 0;
}

int main() {
    RtAudio dac(RtAudio::LINUX_PULSE);

    // RtAudio 6 API: Query explicit device IDs directly
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

    // Stream configuration
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice(); // Retrieves a valid default ID
    parameters.nChannels = 2;
    parameters.firstChannel = 0;

    unsigned int bufferFrames = 128; // Low latency block size
    unsigned int sampleRate = 44100;

    // Define known local values before the audio callback first happens
    BelaContext initialContext = {
        // Audio I/O
        nullptr, nullptr, 2, 2, bufferFrames, static_cast<float>(sampleRate),
        // Analogue I/O ("sliders")
        0, 0, bufferFrames/2, nullptr, nullptr, static_cast<float>(sampleRate/2),
        // Ditigal I/O ("switches")
        16, bufferFrames, nullptr, nullptr
    };

    // Allocate storage for the digital and analogue I/O buffers if implemented
    digitalInputBuffer = new uint32_t[bufferFrames];

    if(!setup(&initialContext, nullptr)) {
        std::cerr << "Bela setup failed.\n";
        return 1;
    }

    // RtAudio v6 structural change: No exceptions. Check return enums directly instead.
    RtAudioErrorType error;

    error = dac.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &rtAudioCallback);
    if (error != RTAUDIO_NO_ERROR) {
        std::cerr << "RtAudio Error: Failed to open stream (Code: " << error << ")\n";
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
    std::cout << "Press Enter to quit...\n";
    std::cin.get();

    dac.stopStream();
    if (dac.isStreamOpen()) dac.closeStream();

    // Run clean up routine on exit
    cleanup(&initialContext, nullptr);
    delete[] digitalInputBuffer;
    return 0;
}
