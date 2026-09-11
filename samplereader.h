// Reads a sample from a wav file and stores it in RAM as floats.

#include <memory>
#include <stdio.h>
#include <sndfile.h>

class SampleReader {
public:
    SampleReader(const char* path);
    bool accumulate(float* buffer, std::size_t length, int channels);

private:
    SF_INFO sfinfo;
    SNDFILE* soundfile;
    std::unique_ptr<float[]> data;
    std::size_t idx;
};
