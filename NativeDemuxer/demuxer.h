#pragma once
#include <cstdint>

class demuxer
{
public:
    demuxer();
    ~demuxer();

private:
    uint8_t* buffer_;
    size_t buffer_size_;
};
