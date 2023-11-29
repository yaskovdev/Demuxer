#include "api.h"
#include <iostream>

#include "demuxer.h"

demuxer* create_demuxer(const callback callback)
{
    return new demuxer(callback);
}

int read_frame(demuxer* demuxer, uint8_t* decoded_data, int* is_video)
{
    return demuxer->read_frame(decoded_data, is_video);
}

void delete_demuxer(const demuxer* demuxer)
{
    delete demuxer;
    std::cout << "Demuxer deleted" << "\n";
}
