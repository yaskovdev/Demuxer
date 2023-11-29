﻿#pragma once
#include <cstdint>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
}

struct buffer_data
{
    uint8_t* ptr;
    size_t size;
};

class demuxer
{
public:
    demuxer();

    void write_packet(const uint8_t* packet, int packet_length);

    int read_frame(uint8_t* decoded_data, int* is_video);

    ~demuxer();

private:
    bool initialized_;
    AVFormatContext* fmt_ctx_;
    buffer_data source_buffer_;
    AVFrame* frame_;
    AVPacket* pkt_;
    AVStream* video_stream_;
    AVStream* audio_stream_;
    int width_;
    int height_;
    AVPixelFormat pix_fmt_;
    int video_dst_bufsize_;
    uint8_t* video_dst_data_[4];
    int video_dst_linesize_[4];
    int audio_stream_idx_;
    int video_stream_idx_;
    AVCodecContext* audio_dec_ctx_;
    AVCodecContext* video_dec_ctx_;
    bool decoder_needs_packet_;
    int current_stream_index_;

    int initialize();

    AVCodecContext* current_context() const;

    static int read_packet(void* opaque, uint8_t* dst_buffer, int dst_buffer_size);

    static int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, AVMediaType type);
};
