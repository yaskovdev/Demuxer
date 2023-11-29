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

    void read_frame();

    ~demuxer();

private:
    uint8_t* buffer_;
    size_t buffer_size_;
    AVFrame* frame_;
    AVPacket* pkt_;
    AVStream *video_stream_;
    AVStream *audio_stream_;
    FILE* video_dst_file_;
    FILE* audio_dst_file_;
    int width_;
    int height_;
    AVPixelFormat pix_fmt_;
    int video_dst_bufsize_;
    uint8_t* video_dst_data_[4];
    int video_dst_linesize_[4];
    int audio_stream_idx_;
    int video_stream_idx_;
    AVCodecContext *audio_dec_ctx_;
    AVCodecContext *video_dec_ctx_;
    const char* audio_dst_name_;
    const char* video_dst_name_;
    bool decoder_needs_packet_;
    int current_stream_index_;

    AVCodecContext* current_context() const;

    static int output_video_frame(AVFrame* frame, int width, int height, AVPixelFormat pix_fmt, uint8_t* video_dst_data[4], int video_dst_linesize[4], int video_dst_bufsize, FILE* video_dst_file);

    static int output_audio_frame(AVFrame* frame, FILE* audio_dst_file);

    static int read_packet(void* opaque, uint8_t* dst_buffer, int dst_buffer_size);

    int decode_packet(AVCodecContext* dec, const AVPacket* pkt);

    static int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, AVMediaType type);

    static int get_format_from_sample_fmt(const char** fmt, AVSampleFormat sample_fmt);
};
