#include "api.h"
#include <iostream>

int fill_chroma_and_luma(uint8_t* chroma, uint8_t* luma, const uint8_t number_of_bytes_in_each_plane)
{
    std::cout << "Native: starting to fill chroma and luma planes of the frame..." << "\n";
    for (uint8_t i = 0; i < number_of_bytes_in_each_plane; ++i)
    {
        chroma[i] = luma[i] = i;
    }
    return 0;
}
