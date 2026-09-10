/* beep.cxx
 *
 * A very simple bela module which generates a continuous tone
 * at a constant amplitude.
 */

#include <cmath>
#include "BelaContext.h"

bool setup(BelaContext *context, void *userData) {
    return true;
}

void render(BelaContext *context, void *userData) {
    constexpr float frequency {440};
    constexpr float amplitude {0.05};
    static const float omega{2.0f*frequency*(float)M_PI/context->audioSampleRate};

    static float y1{amplitude * sinf(-1.0f * omega)};
    static float y2{amplitude * sinf(-2.0f * omega)};
    static float a1{2.0f*cosf(omega)};

    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // Generate a simple sine wave using the difference equation
        // y[n] = a1 * y[n-1] - y[n-2]
        float out{a1*y1 - y2};

        // Write to all available audio output channels
        for(unsigned int ch = 0; ch < context->audioOutChannels; ch++)
            context->audioOut[n * context->audioOutChannels + ch] = out;

        // Update state variables
        y2 = y1;
        y1 = out;
    }
}

void cleanup(BelaContext *context, void *userData) {
    // End-of-life memory cleanup
}
