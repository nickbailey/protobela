#include <algorithm>
#include <iostream>
#include <memory>
#include <sndfile.h>
#include <stdexcept>
#include <string>

#include "samplereader.h"

SampleReader::SampleReader(const char* path) :
    idx {0}
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

    // Implemenation limitation: soundfiles can not be smaller than
    // a single Bela audio fragment.

    // Accumulate up to the end of the buffer or the length of the sample,
    // whichever comes first.
    std::size_t last {std::min<std::size_t>(frames, sfinfo.frames - idx)};

    //std::cout << last << '/' << frames << ", " << more << " left\n";

    for (std::size_t i {0}; i < last; i++, idx++)
        for (int ch {0}; ch < channels ; ch++)
            *buf++ += data[idx];

    if (last < frames) {
        last = frames - last;
        idx = 0;
        for (std::size_t i {idx}; i < last; i++, idx++)
            for (int ch {0}; ch < channels ; ch++)
                *buf++ += data[idx++];
    }

    // For the moment we'll always loop'
    return true;
}
