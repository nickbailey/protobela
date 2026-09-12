/* sampleplayer.cxx
 *
 * A bela module which reads samples in to memory and loops them independently.
 */

#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include "BelaContext.h"
#include "samplereader.h"

struct Track {
    const char* path;
    bool initiallyPaused;
};

static constexpr std::array tracks {
    Track{"samples/Audio 1.wav", false},
    Track{"samples/Audio 2.wav", true},
    Track{"samples/Audio 3.wav", true},
};

static constexpr size_t trackCount = tracks.size();
static std::array<std::unique_ptr<SampleReader>, trackCount> samples;

bool setup(BelaContext *context, void *userData) {
    bool result{true};

    try {
        // Instantiate the sample readers inside the fixed-size array
        for (size_t i = 0; i < trackCount; ++i) {
            samples[i] = std::make_unique<SampleReader>(tracks[i].path);
            samples[i]->paused = tracks[i].initiallyPaused;
        }
    } catch (const std::exception& e) {
        result = false;
    }

    return result;
}

void render(BelaContext *context, void *userData) {
    memset(context->audioOut, 0, context->audioFrames*context->audioOutChannels*sizeof(float));

    // Drive states directly from simulated digital pins
    for (size_t i = 0; i < trackCount; i++) {
        bool buttonActive = digitalRead(context, 0, i);

        if (samples[i]) {
            // XOR toggles the initiallyPaused configuration state:
            // If initiallyPaused is true, buttonActive=true flips it to false (unpaused).
            // If initiallyPaused is false, buttonActive=true flips it to true (paused).
            samples[i]->paused = tracks[i].initiallyPaused ^ buttonActive;
        }
    }

    // Accumulate audio from the individual tracks
    for (auto& s : samples) {
        if (s)
            s->accumulate(context->audioOut, context->audioFrames, context->audioOutChannels);
    } // Fixed missing closing brace
}

void cleanup(BelaContext *context, void *userData) {
    // End-of-life memory cleanup
    for (auto& s : samples) {
        s.reset();
    }
}
