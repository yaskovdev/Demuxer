#include "demuxer.h"

#include <iostream>

extern "C" {
#include "libavutil/file.h"
}

demuxer::demuxer(): buffer_size_(4096)
{
    const int result = av_file_map(R"(c:\dev\experiment3\capture.webm)", &buffer_, &buffer_size_, 0, nullptr);
    std::cout << "Demuxer created with result " << result << "\n";
}

demuxer::~demuxer()
{
    std::cout << "Demuxer destructor called" << "\n";
    av_file_unmap(buffer_, buffer_size_);
}
