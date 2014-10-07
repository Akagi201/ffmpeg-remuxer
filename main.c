
/*
 * @file main.c
 * @author Akagi201
 * @date 2014/10/07
 *
 * remux without recodec using ffmpeg
 */

#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>

#include "log.h"

int main(int argc, char *argv[]) {
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL; // Input AVFormatContext
    AVFormatContext *ofmt_ctx = NULL; // Output AVFormatContext
    AVPacket pkt;
    const char *in_filename = NULL;
    const char *out_filename = NULL;
    int ret = 0;
    int i = 0;
    AVStream *in_stream = NULL;
    AVStream *out_stream = NULL;
    int frame_index = 0;

    if (argc < 3) {
        printf("usage: %s input output\n"
                "Remux a media file with libavformat and libavcodec.\n"
                "The output format is guessed according to the file extension.\n", argv[0]);
        return -1;
    }

    in_filename = argv[1]; // Input file URL
    out_filename = argv[2]; // Output file URL

    av_register_all();

    // Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        LOG("Open input file failed.");
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOG("Retrieve input stream info failed.");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);

    if (!ofmt_ctx) {
        LOG("Create output context failed.");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; ++i) {
        // Create output AVStream according to input AVStream
        in_stream = ifmt_ctx->streams[i];
        out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);

        if (!out_stream) {
            LOG("Allocate output stream failed.");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        // Copy the settings of AVCodecContext
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            LOG("Failed to copy context from input to output stream codec context.");
            goto end;
        }

        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    // Output format
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    // Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOG("Open output file '%s' failed.", out_filename);
            goto end;
        }
    }

    // Write file header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOG("Error occurred when opening output file.");
        goto end;
    }

    frame_index = 0;

    while (1) {
        // Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        /* copy packet */
        // Convert PTS / DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        // Write
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOG("Error muxing packet.");
            break;
        }
        LOG("Write %8d frames to output file\n", frame_index);
        av_free_packet(&pkt);
        frame_index++;
    }

    // Write file trailer
    av_write_trailer(ofmt_ctx);

    end:
    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)) {
        avio_close(ofmt_ctx->pb);
    }

    avformat_free_context(ofmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF) {
        LOG("Error occurred.");
        return -1;
    }

    return 0;
}
































