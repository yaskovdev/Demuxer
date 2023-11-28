#include "api.h"
#include <iostream>

#include "demuxer.h"

demuxer* create_demuxer()
{
    return new demuxer();
}

void read_frame(const demuxer* demuxer)
{
    demuxer->read_frame();
}

void delete_demuxer(const demuxer* demuxer)
{
    delete demuxer;
    std::cout << "Demuxer deleted" << "\n";
}
