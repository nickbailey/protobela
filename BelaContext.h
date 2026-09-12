#ifndef BELACONTEXT_H
#define BELACONTEXT_H

#include <cstdint>

struct BelaContext {
    const float* audioIn;
    float* audioOut;
    unsigned int audioInChannels;
    unsigned int audioOutChannels;
    unsigned int audioFrames;
    float audioSampleRate;

    // Analog I/O (Sliders)
    unsigned int analogInChannels;
    unsigned int analogOutChannels;
    unsigned int analogFrames;          // Will be context->audioFrames / 2
    const float* analogIn;              // Interleaved array of analog inputs
    float* analogOut;                   // Interleaved array of analog outputs
    float analogSampleRate;

    // Digital I/O (Switches and Lights)
    unsigned int digitalChannels;       // Usually 16 on Bela
    unsigned int digitalFrames;         // Will match context->audioFrames
    // Bela natively uses a custom type or packed bits for digital,
    // but a standard float or uint32_t pointer array works for basic mocking
    const uint32_t* digitalIn;
    uint32_t* digitalOut;
};

#endif
