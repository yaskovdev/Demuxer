#pragma once

#include "demuxer.h"

extern "C" __declspec(dllexport) demuxer* create_demuxer();

extern "C" __declspec(dllexport) void read_frame(const demuxer* demuxer);

extern "C" __declspec(dllexport) void delete_demuxer(const demuxer* demuxer);