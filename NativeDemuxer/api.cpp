#include "api.h"
#include <iostream>

#include "demuxer.h"

demuxer* create_demuxer()
{
    return new demuxer();
}

void delete_demuxer(const demuxer* demuxer)
{
    delete demuxer;
    std::cout << "Demuxer deleted" << "\n";
}
