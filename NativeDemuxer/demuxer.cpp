﻿#include "demuxer.h"

#include <iostream>

extern "C" {
#include "libavutil/file.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

struct sample_fmt_entry
{
    AVSampleFormat sample_fmt;
    const char *fmt_be, *fmt_le;
};

demuxer::demuxer(): initialized_(false), fmt_ctx_(nullptr), source_buffer_({nullptr, 0}), frame_(nullptr), pkt_(nullptr), video_stream_(nullptr), audio_stream_(nullptr),
    video_dst_file_(nullptr), audio_dst_file_(nullptr), width_(0), height_(0), pix_fmt_(AV_PIX_FMT_NONE), video_dst_bufsize_(0), video_dst_data_{}, video_dst_linesize_{},
    audio_stream_idx_(-1), video_stream_idx_(-1), audio_dec_ctx_(nullptr), video_dec_ctx_(nullptr),
    audio_dst_name_(R"(c:\dev\experiment3\capture.audio)"), video_dst_name_(R"(c:\dev\experiment3\capture.video)"), decoder_needs_packet_(true), current_stream_index_(-1)
{
    std::cout << "Demuxer created" << "\n";
}

int demuxer::initialize()
{
    fmt_ctx_ = avformat_alloc_context();
    std::cout << "Allocated format context " << fmt_ctx_ << "\n";

    constexpr size_t io_ctx_buffer_size = 4096;
    const auto io_ctx_buffer = static_cast<uint8_t*>(av_malloc(io_ctx_buffer_size));
    if (io_ctx_buffer)
    {
        std::cout << "Allocated context buffer " << io_ctx_buffer << "\n";
    }
    else
    {
        return -1;
    }

    AVIOContext* io_ctx = avio_alloc_context(io_ctx_buffer, io_ctx_buffer_size, 0, &source_buffer_, &read_packet, nullptr, nullptr);

    if (io_ctx)
    {
        std::cout << "Allocated IO context " << io_ctx << "\n";
    }
    else
    {
        return -1;
    }

    fmt_ctx_->pb = io_ctx;

    const int open_input_res = avformat_open_input(&fmt_ctx_, nullptr, nullptr, nullptr);
    if (open_input_res >= 0)
    {
        std::cout << "Opened input" << "\n";
    }
    else
    {
        std::cout << "Cannot opened input, result is " << (open_input_res == AVERROR_INVALIDDATA) << "\n";
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx_, nullptr) >= 0)
    {
        std::cout << "Found stream info" << "\n";
    }
    else
    {
        return -1;
    }

    if (open_codec_context(&video_stream_idx_, &video_dec_ctx_, fmt_ctx_, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        video_stream_ = fmt_ctx_->streams[video_stream_idx_];

        if (fopen_s(&video_dst_file_, video_dst_name_, "wb"))
        {
            std::cerr << "Could not open destination file " << video_dst_name_ << "\n";
            return -1;
        }

        /* allocate image where the decoded image will be put */
        width_ = video_dec_ctx_->width;
        height_ = video_dec_ctx_->height;
        pix_fmt_ = video_dec_ctx_->pix_fmt;
        const int ret = av_image_alloc(video_dst_data_, video_dst_linesize_, width_, height_, pix_fmt_, 1);
        if (ret < 0)
        {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            return -1;
        }
        video_dst_bufsize_ = ret;
    }

    if (open_codec_context(&audio_stream_idx_, &audio_dec_ctx_, fmt_ctx_, AVMEDIA_TYPE_AUDIO) >= 0)
    {
        audio_stream_ = fmt_ctx_->streams[audio_stream_idx_];

        if (fopen_s(&audio_dst_file_, audio_dst_name_, "wb"))
        {
            fprintf(stderr, "Could not open destination file %s\n", audio_dst_name_);
            return -1;
        }
    }

    av_dump_format(fmt_ctx_, 0, nullptr, 0);

    if (!audio_stream_ && !video_stream_)
    {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        return -1;
    }

    frame_ = av_frame_alloc();
    if (!frame_)
    {
        fprintf(stderr, "Could not allocate frame\n");
        return -1;
    }

    pkt_ = av_packet_alloc();
    if (!pkt_)
    {
        fprintf(stderr, "Could not allocate packet\n");
        return -1;
    }

    if (video_stream_)
        printf("Demuxing video\n");
    if (audio_stream_)
        printf("Demuxing audio\n");

    return 0;
}

void demuxer::write_packet(const uint8_t* packet, const int packet_length)
{
    std::cout << "Writing source packet of length " << packet_length << "\n";
    source_buffer_.ptr = new uint8_t[packet_length]; // TODO: append to the buffer instead of rewriting. Return Status.BufferFull if the buffer is full to show that it's time to read frames. Or probably use dynamic collection.
    source_buffer_.size = packet_length;
    memcpy(source_buffer_.ptr, packet, source_buffer_.size);
}

int demuxer::read_frame(uint8_t* decoded_data, int* is_video)
{
    if (!initialized_)
    {
        const int status = initialize();
        if (status == 0)
        {
            std::cout << "Initialized demuxer" << "\n";
            initialized_ = true;
        }
        else
        {
            std::cout << "Cannot initialize demuxer, not enough data in the buffer, send more data" << "\n";
            return -1;
        }
    }

    // ----

    while (true)
    {
        if (decoder_needs_packet_)
        {
            if (av_read_frame(fmt_ctx_, pkt_) < 0)
            {
                return -1;
            }
            current_stream_index_ = pkt_->stream_index;
            if (avcodec_send_packet(current_context(), pkt_) < 0)
            {
                exit(1); // TODO: throw exception instead
            }
            decoder_needs_packet_ = false;
        }

        const int decoding_status = avcodec_receive_frame(current_context(), frame_);
        if (decoding_status < 0)
        {
            if (decoding_status == AVERROR_EOF || decoding_status == AVERROR(EAGAIN))
            {
                decoder_needs_packet_ = true;
            }
            else
            {
                exit(1);
            }
        }
        else if (pkt_->stream_index == video_stream_idx_)
        {
            const int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width_, height_, 1);
            if (av_image_copy_to_buffer(decoded_data, buffer_size, frame_->data, frame_->linesize, AV_PIX_FMT_YUV420P, width_, height_, 1) < 0)
            {
                exit(1);
            }
            std::cout << "Got video frame" << "\n";
            *is_video = 1;
            return 0;
        }
        else
        {
            const size_t unpadded_linesize = frame_->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame_->format));
            memcpy(decoded_data, frame_->extended_data[0], unpadded_linesize);
            std::cout << "Got audio frame" << "\n";
            *is_video = 0;
            return 0;
        }
    }
}

demuxer::~demuxer()
{
    std::cout << "Demuxer destructor called" << "\n";
    // TODO: figure out how to delete the buffer given that read_packet shifts it
    // delete[] source_buffer_.ptr;
}

int demuxer::read_packet(void* opaque, uint8_t* dst_buffer, const int dst_buffer_size)
{
    const auto source_buffer = static_cast<struct buffer_data*>(opaque);
    const int number_of_bytes_to_copy = FFMIN(source_buffer->size, dst_buffer_size);
    printf("ptr:%p size:%zu\n", source_buffer->ptr, source_buffer->size);
    if (number_of_bytes_to_copy == 0)
    {
        std::cout << "read_packet is about to return AVERROR_EOF" << "\n";
        return AVERROR_EOF;
    }
    memcpy(dst_buffer, source_buffer->ptr, number_of_bytes_to_copy);
    source_buffer->ptr += number_of_bytes_to_copy;
    source_buffer->size -= number_of_bytes_to_copy;
    return number_of_bytes_to_copy;
}

int demuxer::decode_packet(AVCodecContext* dec, const AVPacket* pkt)
{
    // submit the packet to the decoder
    int ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", "");
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec, frame_);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding (%s)\n", "");
            return ret;
        }

        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
            ret = output_video_frame(frame_, width_, height_, pix_fmt_, video_dst_data_, video_dst_linesize_, video_dst_bufsize_, video_dst_file_);
        else
            ret = output_audio_frame(frame_, audio_dst_file_);

        av_frame_unref(frame_);
        if (ret < 0)
            return ret;
    }

    return 0;
}

int demuxer::open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, AVMediaType type)
{
    int ret, stream_index;
    AVStream* st;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file\n", av_get_media_type_string(type));
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        const AVCodec* dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, nullptr)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int demuxer::get_format_from_sample_fmt(const char** fmt, AVSampleFormat sample_fmt)
{
    sample_fmt_entry sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},
        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
        {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = nullptr;

    for (int i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        sample_fmt_entry* entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
        "sample format %s is not supported as output format\n",
        av_get_sample_fmt_name(sample_fmt));
    return -1;
}

AVCodecContext* demuxer::current_context() const
{
    if (current_stream_index_ == audio_stream_idx_)
    {
        return audio_dec_ctx_;
    }
    if (current_stream_index_ == video_stream_idx_)
    {
        return video_dec_ctx_;
    }
    // TODO: handle this case properly (should skip such packet)
    exit(1);
}

int demuxer::output_video_frame(AVFrame* frame, int width, int height, AVPixelFormat pix_fmt, uint8_t* video_dst_data[4], int video_dst_linesize[4], int video_dst_bufsize, FILE* video_dst_file)
{
    if (frame->width != width || frame->height != height || frame->format != pix_fmt)
    {
        /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
        fprintf(stderr, "Error: Width, height and pixel format have to be "
            "constant in a rawvideo file, but the width, height or "
            "pixel format of the input video changed:\n"
            "old: width = %d, height = %d, format = %s\n"
            "new: width = %d, height = %d, format = %s\n",
            width, height, av_get_pix_fmt_name(pix_fmt),
            frame->width, frame->height,
            av_get_pix_fmt_name((AVPixelFormat)frame->format));
        return -1;
    }

    /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
    av_image_copy2(video_dst_data, video_dst_linesize,
        frame->data, frame->linesize,
        pix_fmt, width, height);

    /* write to rawvideo file */
    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
    return 0;
}

int demuxer::output_audio_frame(AVFrame* frame, FILE* audio_dst_file)
{
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
    return 0;
}
