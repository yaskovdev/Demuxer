#pragma once
#include <cstdint>

extern "C" __declspec(dllexport) int fill_chroma_and_luma(uint8_t* chroma, uint8_t* luma, uint8_t number_of_bytes_in_each_plane);
