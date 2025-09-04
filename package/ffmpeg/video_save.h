// video_save.h
#ifndef __VIDEO_SAVE_H__
#define __VIDEO_SAVE_H__

#include <stdint.h>
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#if 0
typedef struct VideoWriterCtx {
    AVFormatContext *fmt_ctx; //输出容器
    AVStream *video_st;  //视频流对象
    AVCodecContext *codec_ctx; //编码器上下文，存储编码器参数和状态
    AVFrame *frame; //一帧原始图像数据
    struct SwsContext *sws_ctx; //图像缩放/格式转换上下文
    int frame_index;//帧计数器
    int width, height;
 } VideoWriterCtx;

/**
 * 打开 MP4 文件
 * @param filename  输出文件名
 * @param width     视频宽度
 * @param height    视频高度
 * @param fps       帧率
 */
VideoWriterCtx* VideoWriter_Open(const char* filename, int width, int height, int fps);

/**
 * 写入一帧 RGB888 图像
 * @param ctx       VideoWriter_Open 返回的句柄
 * @param rgb_data  一帧图像数据 (RGB888)
 */
int VideoWriter_WriteFrame(VideoWriterCtx* ctx, const uint8_t* rgb_data);

/**
 * 关闭文件，释放资源
 */
void VideoWriter_Close(VideoWriterCtx* ctx);


int rgb_to_mp4(const char *infile, const char *outfile, int width, int height, int fps);

typedef struct {
    AVFormatContext *oc;
    AVCodecContext  *c;
    AVStream        *st;
    struct SwsContext *sws_ctx;
    AVFrame         *yuv;
    int pts;
    int width, height;
} RgbMp4Context;

RgbMp4Context* rgb_mp4_init(const char *outfile, int width, int height, int fps);

int rgb_mp4_write_frame(RgbMp4Context *ctx, const uint8_t *rgb);

void rgb_mp4_close(RgbMp4Context *ctx);

int ffmpeg_test();

int write_rgb_frame_to_mp4(
    const uint8_t *rgb_buf,   // 输入 RGB buffer，RGB24，每像素3字节
    int width, int height,
    int fps,
    const char *outfile       // 输出 MP4 文件路径
);

#endif

typedef struct {
    AVCodecContext *codec_ctx;
    AVFormatContext *fmt_ctx;
    AVStream *stream;
    AVFrame *frame;
    struct SwsContext *sws_ctx;
    int width;
    int height;
    int fps;
    int pts;
    int total_frames;   // 总帧数
    int frame_index;    // 当前帧索引
} MP4WriterRealtime;


typedef struct {
    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVFrame         *pFrame;
    AVFrame         *pFrameRGB;
    struct SwsContext *sws_ctx;
    uint8_t         *buffer;
    int              videoStreamIndex;
    bool             stop;
} video_context_t;


int mp4_writer_realtime_init(MP4WriterRealtime *w, int width, int height, int fps, const char *outfile);
int mp4_writer_realtime_write_frame(MP4WriterRealtime *w, const uint8_t *rgb_buf);
int mp4_writer_realtime_close(MP4WriterRealtime *w);
int ffmpeg_test();
int ffmpeg_mjpeg_mp4_test();
//int ffmpeg_play_video_test(const char *play_path);

#endif