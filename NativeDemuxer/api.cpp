#include "api.h"
#include <iostream>

#include "demuxer.h"

demuxer* create_demuxer()
{
    return new demuxer();
}

void write_packet(demuxer* demuxer, uint8_t* packet, const int packet_length)
{
    demuxer->write_packet(packet, packet_length);
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
