#pragma once

typedef int (__stdcall *callback)(uint8_t*, int);

struct frame_metadata
{
    int type;
    int64_t timestamp;
};