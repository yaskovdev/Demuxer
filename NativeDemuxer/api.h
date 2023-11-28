#pragma once

#include "demuxer.h"

extern "C" __declspec(dllexport) demuxer* create_demuxer();

extern "C" __declspec(dllexport) void write_packet(demuxer* demuxer, uint8_t* packet, int packet_length);

extern "C" __declspec(dllexport) void read_frame(const demuxer* demuxer);

extern "C" __declspec(dllexport) void delete_demuxer(const demuxer* demuxer);
