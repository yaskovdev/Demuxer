#include "demuxer.h"

#include <iostream>

extern "C" {
#include "libavutil/file.h"
#include "libavformat/avformat.h"
}

struct buffer_data
{
    uint8_t* ptr;
    size_t size;
};

demuxer::demuxer(): buffer_size_(4096), file_name_(R"(c:\dev\experiment3\capture.webm)")
{
    const int result = av_file_map(file_name_, &buffer_, &buffer_size_, 0, nullptr);
    std::cout << "Demuxer created with result " << result << "\n";
}

void demuxer::read_frame() const
{
    buffer_data bd = {nullptr, 0};
    bd.ptr = buffer_;
    bd.size = buffer_size_;
    AVFormatContext* fmt_ctx = avformat_alloc_context();
    std::cout << "Allocated format context " << fmt_ctx << "\n";

    const auto io_ctx_buffer = static_cast<uint8_t*>(av_malloc(buffer_size_));
    if (io_ctx_buffer)
    {
        std::cout << "Allocated context buffer " << io_ctx_buffer << "\n";
    }
    else
    {
        exit(1);
    }

    AVIOContext* io_ctx = avio_alloc_context(io_ctx_buffer, buffer_size_, 0, &bd, &read_packet, nullptr, nullptr);

    if (io_ctx)
    {
        std::cout << "Allocated IO context " << io_ctx << "\n";
    }
    else
    {
        exit(1);
    }

    fmt_ctx->pb = io_ctx;

    if (avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) >= 0)
    {
        std::cout << "Opened input" << "\n";
    }
    else
    {
        exit(1);
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) >= 0)
    {
        std::cout << "Found stream info" << "\n";
    }
    else
    {
        exit(1);
    }

    av_dump_format(fmt_ctx, 0, file_name_, 0);
}

demuxer::~demuxer()
{
    std::cout << "Demuxer destructor called" << "\n";
    av_file_unmap(buffer_, buffer_size_);
}

int demuxer::read_packet(void* opaque, uint8_t* buf, const int buf_size)
{
    const auto bd = static_cast<struct buffer_data*>(opaque);
    std::cout << "read_packet called with buf_size " << buf_size << "\n";

    const int available_buf_size = FFMIN(buf_size, bd->size);
    if (available_buf_size)
    {
        printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

        memcpy(buf, bd->ptr, available_buf_size);
        bd->ptr += available_buf_size;
        bd->size -= available_buf_size;

        return available_buf_size;
    }

    return AVERROR_EOF;
}
