#ifndef BELACONTEXT_H
#define BELACONTEXT_H

struct BelaContext {
    const float* audioIn;
    float* audioOut;
    unsigned int audioInChannels;
    unsigned int audioOutChannels;
    unsigned int audioFrames;
    float audioSampleRate;

    // Stub out other fields to prevent compilation errors if referenced
    unsigned int analogInChannels = 0;
    unsigned int analogOutChannels = 0;
    const float* analogIn = nullptr;
    float* analogOut = nullptr;
};

#endif
