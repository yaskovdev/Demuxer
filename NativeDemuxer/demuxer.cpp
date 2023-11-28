﻿#include "demuxer.h"

#include <iostream>

extern "C" {
#include "libavutil/file.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

struct buffer_data
{
    uint8_t* ptr;
    size_t size;
};

demuxer::demuxer(): buffer_size_(4096), file_name_(R"(c:\dev\experiment3\capture.webm)"), audio_dst_name_(R"(c:\dev\experiment3\capture.audio)"), video_dst_name_(R"(c:\dev\experiment3\capture.video)")
{
    const int result = av_file_map(file_name_, &buffer_, &buffer_size_, 0, nullptr);
    std::cout << "Demuxer created with result " << result << "\n";
}

void demuxer::read_frame() const
{
    buffer_data bd = {buffer_, buffer_size_};
    AVFormatContext* fmt_ctx = avformat_alloc_context();
    std::cout << "Allocated format context " << fmt_ctx << "\n";

    const auto io_ctx_buffer = static_cast<uint8_t*>(av_malloc(buffer_size_));
    if (io_ctx_buffer)
    {
        std::cout << "Allocated context buffer " << io_ctx_buffer << "\n";
    }
    else
    {
        exit(1);
    }

    AVIOContext* io_ctx = avio_alloc_context(io_ctx_buffer, buffer_size_, 0, &bd, &read_packet, nullptr, nullptr);

    if (io_ctx)
    {
        std::cout << "Allocated IO context " << io_ctx << "\n";
    }
    else
    {
        exit(1);
    }

    fmt_ctx->pb = io_ctx;

    const int open_input_res = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
    if (open_input_res >= 0)
    {
        std::cout << "Opened input" << "\n";
    }
    else
    {
        std::cout << "Cannot opened input, result is " << (open_input_res == AVERROR_INVALIDDATA) << "\n";
        exit(1);
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) >= 0)
    {
        std::cout << "Found stream info" << "\n";
    }
    else
    {
        exit(1);
    }

    int ret = 0;
    static AVFrame* frame = nullptr;
    static AVPacket* pkt = nullptr;
    static AVStream *video_stream = nullptr, *audio_stream = nullptr;
    static AVCodecContext *video_dec_ctx = nullptr, *audio_dec_ctx;
    static int video_stream_idx = -1, audio_stream_idx = -1;
    static FILE* video_dst_file = nullptr;
    static FILE* audio_dst_file = nullptr;
    static int width, height;
    static AVPixelFormat pix_fmt;
    static int video_dst_bufsize;
    static uint8_t* video_dst_data[4] = {NULL};
    static int video_dst_linesize[4];

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        video_stream = fmt_ctx->streams[video_stream_idx];

        // video_dst_file = fopen(video_dst_name_, "wb");
        fopen_s(&video_dst_file, video_dst_name_, "wb");
        if (!video_dst_file)
        {
            fprintf(stderr, "Could not open destination file %s\n", video_dst_name_);
            exit(1);
        }

        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize, width, height, pix_fmt, 1);
        if (ret < 0)
        {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            exit(1);
        }
        video_dst_bufsize = ret;
    }

    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0)
    {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
        // audio_dst_file = fopen(audio_dst_name_, "wb");
        fopen_s(&audio_dst_file, audio_dst_name_, "wb");
        if (!audio_dst_file)
        {
            fprintf(stderr, "Could not open destination file %s\n", audio_dst_name_);
            ret = 1;
            exit(1);
        }
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, file_name_, 0);

    if (!audio_stream && !video_stream)
    {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        exit(1);
    }

    if (video_stream)
        printf("Demuxing video\n");
    if (audio_stream)
        printf("Demuxing audio\n");

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, pkt) >= 0)
    {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt->stream_index == video_stream_idx)
            ret = decode_packet(video_dec_ctx, pkt, frame, width, height, pix_fmt, video_dst_data, video_dst_linesize, video_dst_bufsize, video_dst_file, audio_dst_file);
        else if (pkt->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_ctx, pkt, frame, width, height, pix_fmt, video_dst_data, video_dst_linesize, video_dst_bufsize, video_dst_file, audio_dst_file);
        av_packet_unref(pkt);
        if (ret < 0)
            break;
    }

    /* flush the decoders */
    if (video_dec_ctx)
        decode_packet(video_dec_ctx, NULL, frame, width, height, pix_fmt, video_dst_data, video_dst_linesize, video_dst_bufsize, video_dst_file, audio_dst_file);
    if (audio_dec_ctx)
        decode_packet(audio_dec_ctx, NULL, frame, width, height, pix_fmt, video_dst_data, video_dst_linesize, video_dst_bufsize, video_dst_file, audio_dst_file);

    printf("Demuxing succeeded.\n");

    if (video_stream)
    {
        printf("Play the output video file with the command:\n"
            "ffplay -f rawvideo -pix_fmt %i -video_size %dx%d %s\n",
            pix_fmt, width, height,
            video_dst_name_);
    }

    if (audio_stream)
    {
        enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
        int n_channels = audio_dec_ctx->ch_layout.nb_channels;
        const char* fmt;

        if (av_sample_fmt_is_planar(sfmt))
        {
            const char* packed = av_get_sample_fmt_name(sfmt);
            printf("Warning: the sample format the decoder produced is planar "
                "(%s). This example will output the first channel only.\n",
                packed ? packed : "?");
            sfmt = av_get_packed_sample_fmt(sfmt);
            n_channels = 1;
        }

        if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
            exit(1);

        printf("Play the output audio file with the command:\n"
            "ffplay -f %s -ac %d -ar %d %s\n",
            fmt, n_channels, audio_dec_ctx->sample_rate,
            audio_dst_name_);
    }
}

demuxer::~demuxer()
{
    std::cout << "Demuxer destructor called" << "\n";
    av_file_unmap(buffer_, buffer_size_);
}

int demuxer::read_packet(void* opaque, uint8_t* buf, const int buf_size)
{
    const auto bd = static_cast<struct buffer_data*>(opaque);
    std::cout << "read_packet called with buf_size " << buf_size << "\n";

    const int available_buf_size = FFMIN(buf_size, bd->size);
    if (available_buf_size)
    {
        printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

        memcpy(buf, bd->ptr, available_buf_size);
        bd->ptr += available_buf_size;
        bd->size -= available_buf_size;

        return available_buf_size;
    }

    return AVERROR_EOF;
}

int demuxer::decode_packet(AVCodecContext* dec, const AVPacket* pkt, AVFrame* frame, int width, int height, AVPixelFormat pix_fmt, uint8_t* video_dst_data[4], int video_dst_linesize[4], int video_dst_bufsize, FILE* video_dst_file, FILE* audio_dst_file)
{
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", "");
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec, frame);
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
            ret = output_video_frame(frame, width, height, pix_fmt, video_dst_data, video_dst_linesize, video_dst_bufsize, video_dst_file);
        else
            ret = output_audio_frame(frame, audio_dst_file);

        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }

    return 0;
}

int demuxer::open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream* st;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
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
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int demuxer::get_format_from_sample_fmt(const char** fmt,
    enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
            {AV_SAMPLE_FMT_U8, "u8", "u8"},
            {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
            {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
            {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
            {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
        };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry* entry = &sample_fmt_entries[i];
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

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}
