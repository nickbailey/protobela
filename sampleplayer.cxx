/* sampleplayer.cxx
 *
 * A bela module which reads samples in to memory and loops them independently.
 */

#include <array>
#include <cmath>
#include <cstring>
#include "BelaContext.h"
#include "samplereader.h"

static std::array samples {
    SampleReader("samples/Audio 1.wav"),
    SampleReader("samples/Audio 2.wav"),
    SampleReader("samples/Audio 3.wav"),
};

bool setup(BelaContext *context, void *userData) {
    return true;
}

void render(BelaContext *context, void *userData) {
    memset(context->audioOut, 0, context->audioFrames*context->audioOutChannels*sizeof(float));

    for (auto& s : samples)
        s.accumulate(context->audioOut, context->audioFrames, context->audioOutChannels);
}

void cleanup(BelaContext *context, void *userData) {
    // End-of-life memory cleanup
}
