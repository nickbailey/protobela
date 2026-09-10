#include <cmath>
#include "BelaContext.h"

float gPhase = 0.0;
float gInverseSampleRate;

bool setup(BelaContext *context, void *userData) {
    gInverseSampleRate = 1.0 / context->audioSampleRate;
    return true;
}

void render(BelaContext *context, void *userData) {
    float frequency = 440.0;

    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // Generate a simple sine wave
        float out = sinf(gPhase);
        gPhase += 2.0 * M_PI * frequency * gInverseSampleRate;
        if(gPhase >= 2.0 * M_PI) gPhase -= 2.0 * M_PI;

        // Write to all available audio output channels
        for(unsigned int ch = 0; ch < context->audioOutChannels; ch++) {
            context->audioOut[n * context->audioOutChannels + ch] = out;
        }
    }
}

void cleanup(BelaContext *context, void *userData) {
    // End-of-life memory cleanup
}

