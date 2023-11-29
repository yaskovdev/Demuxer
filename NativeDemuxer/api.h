#pragma once

#include "demuxer.h"
#include "callback.h"

extern "C" __declspec(dllexport) demuxer* create_demuxer(callback callback);

extern "C" __declspec(dllexport) int read_frame(demuxer* demuxer, uint8_t* decoded_data, int* is_video);

extern "C" __declspec(dllexport) void delete_demuxer(const demuxer* demuxer);
