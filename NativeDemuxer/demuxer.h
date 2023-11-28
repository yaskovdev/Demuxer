#pragma once
#include <cstdint>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
}

class demuxer
{
public:
    demuxer();

    void write_packet(const uint8_t* packet, int packet_length);

    void read_frame() const;

    ~demuxer();

private:
    uint8_t* buffer_;
    size_t buffer_size_;
    const char* audio_dst_name_;
    const char* video_dst_name_;

    static int output_video_frame(AVFrame* frame, int width, int height, AVPixelFormat pix_fmt, uint8_t* video_dst_data[4], int video_dst_linesize[4], int video_dst_bufsize, FILE* video_dst_file);
    static int output_audio_frame(AVFrame* frame, FILE* audio_dst_file);

    static int read_packet(void* opaque, uint8_t* buf, int buf_size);
    static int decode_packet(AVCodecContext* dec, const AVPacket* pkt, AVFrame* frame, int width, int height, AVPixelFormat pix_fmt, uint8_t* video_dst_data[4], int video_dst_linesize[4], int video_dst_bufsize, FILE* video_dst_file, FILE* audio_dst_file);
    static int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, AVMediaType type);
    static int get_format_from_sample_fmt(const char** fmt, AVSampleFormat sample_fmt);
};
