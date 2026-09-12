#include <algorithm>
#include <iostream>
#include <memory>
#include <sndfile.h>
#include <stdexcept>
#include <string>

#include "samplereader.h"

SampleReader::SampleReader(const char* path, bool paused) :
    idx {0},
    paused{paused}
{
    // Open the named sound file and read it into memory.
    soundfile = sf_open(path, SFM_READ, &sfinfo);

    // It must exist
    if (!soundfile)
        throw std::runtime_error(std::string("Failed to open ") + path);

    // It must be mono
    if (sfinfo.channels != 1)
        throw std::runtime_error(std::string(path) + " is not a mono sound file");

    // Allocate storage and read the file
    data = std::unique_ptr<float[]>(new float[sfinfo.frames]);
    sf_count_t frames {sf_read_float(soundfile, &data[0], sfinfo.frames)};

    // Finished with the soundfile now.
    sf_close(soundfile);
    soundfile = nullptr;

    // Report success
    std::cout << frames << " audio frames read from " << path << std::endl;
}

bool SampleReader::accumulate(float* buf, std::size_t frames, int channels) {

    if (paused) return true;

    // Process the first chunk up to the end of the sound file data memory boundary
    std::size_t first_leg = std::min<std::size_t>(frames, sfinfo.frames - idx);
    for (std::size_t i = 0; i < first_leg; i++) {
        for (int ch = 0; ch < channels; ch++) {
            *buf++ += data[idx];
        }
        idx++;
    }

    // Handle wrap-around loops safely without any branch checks inside your rendering code
    if (first_leg < frames) {
        std::size_t remaining_frames = frames - first_leg;
        idx = 0; // Reset player head back to the beginning of the file array

        // If the sample file length is incredibly small, it might wrap around completely
        // multiple times inside this single audio block request block
        while (remaining_frames >= sfinfo.frames) {
            // Fill an entire layout loop of the sample file instantly
            for (std::size_t i = 0; i < sfinfo.frames; i++) {
                for (int ch = 0; ch < channels; ch++) {
                    *buf++ += data[i];
                }
            }
            remaining_frames -= sfinfo.frames;
        }

        // Process the final remaining fractional frames left in the block request
        for (std::size_t i = 0; i < remaining_frames; i++) {
            for (int ch = 0; ch < channels; ch++) {
                *buf++ += data[idx];
            }
            idx++;
        }
    }

    // For the moment we'll always loop'
    return true;
}
