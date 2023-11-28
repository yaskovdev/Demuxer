#pragma once
#include <cstdint>

class demuxer
{
public:
    demuxer();

    void read_frame() const;

    ~demuxer();

private:
    uint8_t* buffer_;
    size_t buffer_size_;
    const char* file_name_;

    static int read_packet(void* opaque, uint8_t* buf, int buf_size);
};
