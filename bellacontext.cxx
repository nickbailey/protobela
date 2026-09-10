#include <iostream>
#include <vector>
#include <RtAudio.h>
#include "BelaContext.h"

// Forward declare the user-implemented Bela hooks (from render.cpp)
bool setup(BelaContext *context, void *userData);
void render(BelaContext *context, void *userData);
void cleanup(BelaContext *context, void *userData);

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

    // Call your portable Bela engine loop
    render(&context, nullptr);
    return 0;
}

int main() {
    RtAudio dac;

    // RtAudio 6 API: Query explicit device IDs directly
    std::vector<unsigned int> deviceIds = dac.getDeviceIds();
    if (deviceIds.empty()) {
        std::cerr << "No audio devices found!\n";
        return 1;
    }

    // Stream configuration
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice(); // Retrieves a valid default ID
    parameters.nChannels = 2;
    parameters.firstChannel = 0;

    unsigned int bufferFrames = 128; // Low latency block size
    unsigned int sampleRate = 44100;

    // Call the user setup function before audio stream initializes
    BelaContext initialContext = {nullptr, nullptr, 2, 2, bufferFrames, (float)sampleRate};
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
    return 0;
}
