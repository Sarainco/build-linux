#include "video_save.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "sys_env.h"
#include "sys_tick.h"
#if 0
int ffmpeg_test() 
{
    const char *outfile = "/userdata/ums/video/out114.mp4";
    int width = 640;
    int height = 360;
    int fps = 25;
    int num_frames = 2; // 写 10 帧做演示

    av_register_all();

    // ================= 创建编码器 =================
    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) { printf("libx264 not found\n"); return -1; }

    AVCodecContext *c = avcodec_alloc_context3(codec);
    c->width = width;
    c->height = height;
    c->time_base = (AVRational){1, fps};
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->bit_rate = 400000;
    c->gop_size = 10;
    c->max_b_frames = 0;
    c->thread_count = 4;
    c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(c, codec, NULL) < 0) 
    {
        printf("Failed to open codec\n");
        return -1;
    }

    // ================= 创建输出 MP4 =================
    AVFormatContext *fmt_ctx = NULL;
    avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, outfile);
    if (!fmt_ctx) { printf("Failed to create format context\n"); return -1; }

    AVStream *st = avformat_new_stream(fmt_ctx, NULL);
    st->time_base = (AVRational){1, fps};
    avcodec_parameters_from_context(st->codecpar, c);

    if (avio_open(&fmt_ctx->pb, outfile, AVIO_FLAG_WRITE) < 0) 
    {
        printf("Failed to open output file\n"); return -1;
    }

    avformat_write_header(fmt_ctx, NULL);

    // ================= 创建 frame =================
    AVFrame *frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;
    av_frame_get_buffer(frame, 32);

    // ================= 创建 RGB 输入 =================
    uint8_t *rgb = (uint8_t*)malloc(width * height * 3);

    struct SwsContext *sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_RGB24,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, NULL, NULL, NULL
    );

    // ================= 循环写帧 =================
    for (int i = 0; i < num_frames; i++) 
    {
        // 随机生成 RGB 数据
        for (int j = 0; j < width*height*3; j++) rgb[j] = rand() % 256;

        const uint8_t *inData[1] = { rgb };
        int inLinesize[1] = { width*3 };
        sws_scale(sws_ctx, inData, inLinesize, 0, height, frame->data, frame->linesize);

        frame->pts = i;
        frame->pict_type = AV_PICTURE_TYPE_I; // 强制关键帧

        if (avcodec_send_frame(c, frame) < 0) continue;

        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        while (avcodec_receive_packet(c, &pkt) == 0) {
            pkt.stream_index = st->index;
            av_interleaved_write_frame(fmt_ctx, &pkt);
            av_packet_unref(&pkt);
        }
    }

    // ================= flush 编码器 =================
    avcodec_send_frame(c, NULL);
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    // ================= 写 trailer 并关闭 =================
    av_write_trailer(fmt_ctx);
    avio_closep(&fmt_ctx->pb);
    avformat_free_context(fmt_ctx);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    free(rgb);

    printf("MP4 file generated: %s\n", outfile);
    return 0;
}
#endif

#if 1//测试通过，格式h.264
// 初始化 MP4 写入（实时接口）
int mp4_writer_realtime_init(MP4WriterRealtime *w, int width, int height, int fps, const char *outfile) 
{
    av_register_all();
    w->width = width;
    w->height = height;
    w->fps = fps;
    w->pts = 0;

    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) { printf("mjpeg not found\n"); return -1; }

    w->codec_ctx = avcodec_alloc_context3(codec);
    w->codec_ctx->width = width;
    w->codec_ctx->height = height;
    w->codec_ctx->time_base = (AVRational){1, fps};
    w->codec_ctx->framerate = (AVRational){fps, 1};
    w->codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    w->codec_ctx->bit_rate = 400000;
    w->codec_ctx->gop_size = 50;
    w->codec_ctx->max_b_frames = 0;
    w->codec_ctx->thread_count = 4;
    w->codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(w->codec_ctx, codec, NULL) < 0) { printf("Failed to open codec\n"); return -1; }

    avformat_alloc_output_context2(&w->fmt_ctx, NULL, NULL, outfile);
    if (!w->fmt_ctx) { printf("Failed to create format context\n"); return -1; }

    w->stream = avformat_new_stream(w->fmt_ctx, NULL);
    w->stream->time_base = (AVRational){1, fps};
    avcodec_parameters_from_context(w->stream->codecpar, w->codec_ctx);

    if (avio_open(&w->fmt_ctx->pb, outfile, AVIO_FLAG_WRITE) < 0) { printf("Failed to open output file\n"); return -1; }

    avformat_write_header(w->fmt_ctx, NULL);

    w->frame = av_frame_alloc();
    w->frame->format = AV_PIX_FMT_YUV420P;
    w->frame->width = width;
    w->frame->height = height;
    av_frame_get_buffer(w->frame, 32);

    w->sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                width, height, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, NULL, NULL, NULL);
    return 0;
}

// 写入一帧 RGB24（实时接口）
int mp4_writer_realtime_write_frame(MP4WriterRealtime *w, const uint8_t *rgb_buf) 
{
    const uint8_t *inData[1] = { rgb_buf };
    int inLinesize[1] = { w->width * 3 };
    int firstFrame = 0;
    sws_scale(w->sws_ctx, inData, inLinesize, 0, w->height, w->frame->data, w->frame->linesize);

    w->frame->pts = w->pts++;
    w->frame->pict_type = AV_PICTURE_TYPE_NONE;

    if (avcodec_send_frame(w->codec_ctx, w->frame) < 0) return -1;

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (avcodec_receive_packet(w->codec_ctx, &pkt) == 0) 
    {
        av_packet_rescale_ts(&pkt, w->codec_ctx->time_base, w->stream->time_base);
        pkt.stream_index = w->stream->index;
        av_interleaved_write_frame(w->fmt_ctx, &pkt);
        av_packet_unref(&pkt);
    }
    return 0;
}

// 关闭 MP4 写入（实时接口）
int mp4_writer_realtime_close(MP4WriterRealtime *w) 
{
    avcodec_send_frame(w->codec_ctx, NULL);
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    w->stream->duration = w->pts;

    av_write_trailer(w->fmt_ctx);
    avio_closep(&w->fmt_ctx->pb);
    avformat_free_context(w->fmt_ctx);
    avcodec_free_context(&w->codec_ctx);
    av_frame_free(&w->frame);
    sws_freeContext(w->sws_ctx);
    return 0;
}



// =================== 示例 ===================
int ffmpeg_test() 
{
    int width = 360, height = 240, fps = 25;
    int num_frames = 20;  // 模拟实时帧数

    MP4WriterRealtime writer;
    mp4_writer_realtime_init(&writer, width, height, fps, "/userdata/ums/video/out_realtime019.mp4");

    // 分配一帧 RGB buffer
    uint8_t *rgb_buf = (uint8_t*)malloc(width * height * 3);

    for (int i = 0; i < num_frames; i++) {
        // 填充示例图像，每帧颜色变化
        for (int j = 0; j < width * height; j++) {
            rgb_buf[j*3+0] = (uint8_t)(255 * i / num_frames); // R
            rgb_buf[j*3+1] = 0;                               // G
            rgb_buf[j*3+2] = (uint8_t)(255 * (num_frames-i)/num_frames); // B
        }

        mp4_writer_realtime_write_frame(&writer, rgb_buf);
    }

    mp4_writer_realtime_close(&writer);
    free(rgb_buf);

    printf("Realtime MP4 generated: out_realtime.mp4\n");
    return 0;
}

#endif


#if 0
// 初始化 MJPEG MP4 写入（实时接口）
int mjpeg_mp4_writer_realtime_init(MP4WriterRealtime *w, int width, int height, int fps, const char *outfile) 
{
    av_register_all();
    w->width = width;
    w->height = height;
    w->fps = fps;
    w->pts = 0;

    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG); // MJPEG
    if (!codec) { printf("MJPEG codec not found\n"); return -1; }

    w->codec_ctx = avcodec_alloc_context3(codec);
    w->codec_ctx->width = width;
    w->codec_ctx->height = height;
    w->codec_ctx->time_base = (AVRational){1, fps};
    w->codec_ctx->framerate = (AVRational){fps, 1};
    w->codec_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P; // MJPEG常用
    w->codec_ctx->bit_rate = 400000;

    if (avcodec_open2(w->codec_ctx, codec, NULL) < 0) { printf("Failed to open codec\n"); return -1; }

    avformat_alloc_output_context2(&w->fmt_ctx, NULL, "mp4", outfile); // 强制MP4
    if (!w->fmt_ctx) { printf("Failed to create format context\n"); return -1; }

    w->stream = avformat_new_stream(w->fmt_ctx, NULL);
    w->stream->time_base = (AVRational){1, fps};
    avcodec_parameters_from_context(w->stream->codecpar, w->codec_ctx);

    if (avio_open(&w->fmt_ctx->pb, outfile, AVIO_FLAG_WRITE) < 0) { printf("Failed to open output file\n"); return -1; }

    avformat_write_header(w->fmt_ctx, NULL);

    w->frame = av_frame_alloc();
    w->frame->format = AV_PIX_FMT_YUVJ420P;
    w->frame->width = width;
    w->frame->height = height;
    av_frame_get_buffer(w->frame, 32);

    w->sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                width, height, AV_PIX_FMT_YUVJ420P,
                                SWS_BICUBIC, NULL, NULL, NULL);
    return 0;
}

// 写入一帧 RGB24
int mjpeg_mp4_writer_realtime_write_frame(MP4WriterRealtime *w, const uint8_t *rgb_buf) 
{
    const uint8_t *inData[1] = { rgb_buf };
    int inLinesize[1] = { w->width * 3 };
    sws_scale(w->sws_ctx, inData, inLinesize, 0, w->height, w->frame->data, w->frame->linesize);

    w->frame->pts = w->pts++;

    if (avcodec_send_frame(w->codec_ctx, w->frame) < 0) return -1;

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (avcodec_receive_packet(w->codec_ctx, &pkt) == 0) 
    {
        av_packet_rescale_ts(&pkt, w->codec_ctx->time_base, w->stream->time_base);
        pkt.stream_index = w->stream->index;
        av_interleaved_write_frame(w->fmt_ctx, &pkt);
        av_packet_unref(&pkt);
    }
    return 0;
}

// 关闭 MJPEG MP4 写入
int mjpeg_mp4_writer_realtime_close(MP4WriterRealtime *w) 
{
    avcodec_send_frame(w->codec_ctx, NULL);
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    w->stream->duration = w->pts;

    av_write_trailer(w->fmt_ctx);
    avio_closep(&w->fmt_ctx->pb);
    avformat_free_context(w->fmt_ctx);
    avcodec_free_context(&w->codec_ctx);
    av_frame_free(&w->frame);
    sws_freeContext(w->sws_ctx);
    return 0;
}

// =================== 示例 ===================
int ffmpeg_mjpeg_mp4_test() 
{
    int width = 360, height = 240, fps = 25;
    int num_frames = 100;

    MP4WriterRealtime writer;
    mjpeg_mp4_writer_realtime_init(&writer, width, height, fps, "/userdata/ums/video/outmjpeg_realtime.mp4");

    uint8_t *rgb_buf = (uint8_t*)malloc(width * height * 3);

    for (int i = 0; i < num_frames; i++) {
        for (int j = 0; j < width * height; j++) {
            rgb_buf[j*3+0] = (uint8_t)(255 * i / num_frames); 
            rgb_buf[j*3+1] = 0;                               
            rgb_buf[j*3+2] = (uint8_t)(255 * (num_frames-i)/num_frames); 
        }
        mjpeg_mp4_writer_realtime_write_frame(&writer, rgb_buf);
    }

    mjpeg_mp4_writer_realtime_close(&writer);
    free(rgb_buf);

    printf("Realtime MJPEG MP4 generated: out_realtime.mp4\n");
    return 0;
}

#endif


#if 0 //播放视频测试通过
void video_canvas_show_frame(uint8_t *rgb_buf, int width, int height);

int ffmpeg_play_video_test(const char *play_path)
{

    // 初始化FFmpeg并注册所有编解码器和格式
    av_register_all();

    AVFormatContext *pFormatCtx = NULL;
 
    // 打开视频文件
    if(avformat_open_input(&pFormatCtx, play_path, NULL, NULL) != 0) {
        // 文件打开失败
        printf("Couldn't open file.\n");
        return -1;
    }

    // 获取流信息
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        // 获取失败
        printf("Couldn't find stream information.\n");
        return -1;
    }
    
    AVCodecContext *pCodecCtx = NULL;
    int videoStreamIndex = -1;
    
    // 查找视频流的索引位置
    for(int i=0; i<pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    
    if(videoStreamIndex == -1) {
        // 没有找到视频流
        printf("Didn't find a video stream.\n");
        return -1;
    }
    
    // 获取视频流的编解码器上下文
    pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    //AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    AVCodec *pCodec = avcodec_find_decoder_by_name("h264");
    if (!pCodec) {
        printf("h264 decoder not found\n");
        return -1;
    }

    // 打开编解码器
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        // 打开编解码器失败
        printf("Could not open codec.\n");
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    // 分配 RGB buffer
    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes);
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);
    
    // 建立转换上下文
    struct SwsContext *sws_ctx = sws_getContext(
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL);

    AVPacket packet;


    while(av_read_frame(pFormatCtx, &packet) >= 0) {
        if(packet.stream_index == videoStreamIndex) {
            // 解码视频帧
            int frameFinished;
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            
            if(frameFinished) {
                // 这里我们已经得到了一帧视频，可以进行后续的处理
                                // 解码到一帧，进行转换
                sws_scale(
                    sws_ctx,
                    (uint8_t const * const *)pFrame->data,
                    pFrame->linesize,
                    0,
                    pCodecCtx->height,
                    pFrameRGB->data,
                    pFrameRGB->linesize);

                //此时 pFrameRGB->data 里就是一帧 RGB 图像
                video_canvas_show_frame(pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height);
                lv_timer_handler();
                usleep(40000);
                printf("Got one RGB frame: %dx%d\n", pCodecCtx->width, pCodecCtx->height);
            }
        }
        // 释放packet
        av_packet_unref(&packet);
    }

    // 释放资源
    av_free(buffer);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}


// 全局 LVGL 画布对象
static lv_obj_t * canvas;
static lv_color_t * canvas_buf;

// 初始化 LVGL 画布
void video_canvas_init(lv_obj_t * parent, int width, int height) {
    // LVGL 的颜色缓冲，RGB888 每像素 3 字节，这里用 LV_COLOR_SIZE
    canvas_buf = malloc(width * height * 3);

    canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, canvas_buf, width, height, LV_COLOR_FORMAT_RGB888);
    lv_obj_center(canvas);

    lv_obj_move_foreground(canvas);
}

// 显示一帧 RGB 数据 (FFmpeg -> LVGL)
void video_canvas_show_frame(uint8_t *rgb_buf, int width, int height) {
    // 注意：LVGL 的 lv_color_t 默认是 16bit (RGB565)，而 FFmpeg sws_scale 出来是 RGB24
    // 所以这里要做一次 RGB24 -> RGB565 的转换
    // uint16_t *dst = (uint16_t *)canvas_buf;
    // uint8_t *src = rgb_buf;

    // for(int y = 0; y < height; y++) {
    //     for(int x = 0; x < width; x++) {
    //         uint8_t r = *src++;
    //         uint8_t g = *src++;
    //         uint8_t b = *src++;

    //         // 转成 RGB565
    //         *dst++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    //     }
    // }
    memcpy(canvas_buf, rgb_buf, width * height * 3);

    // 刷新画布
    lv_obj_invalidate(canvas);
}

#endif

// 全局 LVGL 画布对象
static lv_obj_t * canvas;
static lv_color_t * canvas_buf;

void video_canvas_init(lv_obj_t * parent, int width, int height) {
    // LVGL 的颜色缓冲，RGB888 每像素 3 字节，这里用 LV_COLOR_SIZE
    canvas_buf = malloc(width * height * 3);

    canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, canvas_buf, width, height, LV_COLOR_FORMAT_RGB888);
    lv_obj_center(canvas);

    lv_obj_move_foreground(canvas);
}

// 显示一帧 RGB 数据 (FFmpeg -> LVGL)
void video_canvas_show_frame(uint8_t *rgb_buf, int width, int height) {
    // 注意：LVGL 的 lv_color_t 默认是 16bit (RGB565)，而 FFmpeg sws_scale 出来是 RGB24
    // 所以这里要做一次 RGB24 -> RGB565 的转换
    // uint16_t *dst = (uint16_t *)canvas_buf;
    // uint8_t *src = rgb_buf;

    // for(int y = 0; y < height; y++) {
    //     for(int x = 0; x < width; x++) {
    //         uint8_t r = *src++;
    //         uint8_t g = *src++;
    //         uint8_t b = *src++;

    //         // 转成 RGB565
    //         *dst++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    //     }
    // }
    memcpy(canvas_buf, rgb_buf, width * height * 3);

    // 刷新画布
    lv_obj_invalidate(canvas);
}

static void video_timer_cb(lv_timer_t * timer)
{
    video_context_t *ctx = lv_timer_get_user_data(timer);
#if 0
    if(ctx->stop) {
        lv_timer_del(timer);

        // 删除视频 canvas
        if(canvas) {
            lv_obj_del(canvas);
            canvas = NULL;
        }

        // 释放 FFmpeg 资源
        av_free(ctx->buffer);
        av_frame_free(&ctx->pFrameRGB);
        av_frame_free(&ctx->pFrame);
        avcodec_close(ctx->pCodecCtx);
        avformat_close_input(&ctx->pFormatCtx);
        free(ctx);
        return;
    }
#endif
    // 如果需要 seek
    // if(ctx->need_seek && ctx->pFormatCtx && ctx->videoStreamIndex >= 0) {
    //     int64_t seek_target = av_rescale_q(ctx->current_pts,
    //                                        (AVRational){1,1000},
    //                                        ctx->pFormatCtx->streams[ctx->videoStreamIndex]->time_base);
    //     av_seek_frame(ctx->pFormatCtx, ctx->videoStreamIndex, seek_target, AVSEEK_FLAG_ANY);
    //     avcodec_flush_buffers(ctx->pCodecCtx);
    //     ctx->need_seek = false;
    // }
    if(!ctx || ctx->stop || ctx->seeking) return;  // 暂停时不播放

    AVPacket packet;
    if(av_read_frame(ctx->pFormatCtx, &packet) >= 0) {
        if(packet.stream_index == ctx->videoStreamIndex) {
            int frameFinished = 0;
            avcodec_decode_video2(ctx->pCodecCtx, ctx->pFrame, &frameFinished, &packet);
            if(frameFinished) {
                sws_scale(ctx->sws_ctx,
                          (uint8_t const * const *)ctx->pFrame->data,
                          ctx->pFrame->linesize,
                          0,
                          ctx->pCodecCtx->height,
                          ctx->pFrameRGB->data,
                          ctx->pFrameRGB->linesize);

                // 显示到 LVGL canvas
                video_canvas_show_frame(ctx->pFrameRGB->data[0],
                                        ctx->pCodecCtx->width,
                                        ctx->pCodecCtx->height);

                // 更新当前播放时间 (ms)
                if (ctx->pFrame->pts != AV_NOPTS_VALUE) {
                    AVRational time_base = ctx->pFormatCtx->streams[ctx->videoStreamIndex]->time_base;
                    ctx->current_pts = av_rescale_q(ctx->pFrame->pts, time_base, (AVRational){1,1000});
                }

                // 更新进度条
                if (!ctx->seeking && ctx->slider) {
                    lv_slider_set_value(ctx->slider, (int32_t)ctx->current_pts, LV_ANIM_OFF);
                }
            }
        }
        av_packet_unref(&packet);
    } else {
        // 视频播放完
        ctx->stop = true;
        if(timer) lv_timer_del(timer);
        // 删除视频 canvas
        if (ctx->slider) {
            lv_obj_del(ctx->slider);
            ctx->slider = NULL;
        }
        if(canvas) {
            lv_obj_del(canvas);
            canvas = NULL;
        }

        // 释放 FFmpeg 资源
        av_free(ctx->buffer);
        av_frame_free(&ctx->pFrameRGB);
        av_frame_free(&ctx->pFrame);
        avcodec_close(ctx->pCodecCtx);
        avformat_close_input(&ctx->pFormatCtx);
        free(ctx);
        free(canvas_buf);
        printf("video_timer_cb stop.....");
    }
}

static void slider_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * slider = lv_event_get_target(e);
    video_context_t * ctx = lv_event_get_user_data(e);

    if(!ctx || !slider) return;

    if(code == LV_EVENT_PRESSED) {
        ctx->seeking = true;  // 开始拖动
    } 
    else if(code == LV_EVENT_VALUE_CHANGED && ctx->seeking) {
        // 拖动时实时显示帧
        ctx->current_pts = lv_slider_get_value(slider);

        if(ctx->pFormatCtx && ctx->videoStreamIndex >= 0) {
            int64_t seek_target = av_rescale_q(ctx->current_pts,
                                               (AVRational){1,1000},
                                               ctx->pFormatCtx->streams[ctx->videoStreamIndex]->time_base);

            av_seek_frame(ctx->pFormatCtx, ctx->videoStreamIndex, seek_target, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
            avcodec_flush_buffers(ctx->pCodecCtx);

            // 解码一帧显示
            AVPacket packet;
            while(av_read_frame(ctx->pFormatCtx, &packet) >= 0) {
                if(packet.stream_index == ctx->videoStreamIndex) {
                    int frameFinished = 0;
                    avcodec_decode_video2(ctx->pCodecCtx, ctx->pFrame, &frameFinished, &packet);
                    if(frameFinished) {
                        sws_scale(ctx->sws_ctx,
                                  (uint8_t const * const *)ctx->pFrame->data,
                                  ctx->pFrame->linesize,
                                  0,
                                  ctx->pCodecCtx->height,
                                  ctx->pFrameRGB->data,
                                  ctx->pFrameRGB->linesize);

                        video_canvas_show_frame(ctx->pFrameRGB->data[0],
                                                ctx->pCodecCtx->width,
                                                ctx->pCodecCtx->height);
                        ctx->current_pts = av_rescale_q(ctx->pFrame->pts,
                                                       ctx->pFormatCtx->streams[ctx->videoStreamIndex]->time_base,
                                                       (AVRational){1,1000});
                        lv_slider_set_value(ctx->slider, (int32_t)ctx->current_pts, LV_ANIM_OFF);
                        av_packet_unref(&packet);
                        break; // 只显示一帧
                    }
                }
                av_packet_unref(&packet);
            }
        }
    } 
    else if(code == LV_EVENT_RELEASED) {
        // 拖动结束，继续 timer 播放
        ctx->seeking = false;
        ctx->current_pts = lv_slider_get_value(slider);
    }
}


void lv_example_open_video(lv_obj_t * parent, const char * full_path)
{
    if(!full_path) return;

    // 只处理 mp4
    const char * ext = strrchr(full_path, '.');
    if(!ext || strcmp(ext, ".mp4") != 0) return;

    // 初始化视频上下文
    video_context_t *ctx = calloc(1, sizeof(video_context_t));

    // FFmpeg 打开文件
    av_register_all();
    if(avformat_open_input(&ctx->pFormatCtx, full_path, NULL, NULL) != 0) return;
    if(avformat_find_stream_info(ctx->pFormatCtx, NULL) < 0) return;

    // 查找视频流
    ctx->videoStreamIndex = -1;
    for(int i=0; i<ctx->pFormatCtx->nb_streams; i++) {
        if(ctx->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ctx->videoStreamIndex = i;
            break;
        }
    }
    if(ctx->videoStreamIndex == -1) return;

    ctx->pCodecCtx = ctx->pFormatCtx->streams[ctx->videoStreamIndex]->codec;
    AVCodec *pCodec = avcodec_find_decoder_by_name("h264");
    if (!pCodec) {
        printf("h264 decoder not found\n");
        return;
    }
    if(avcodec_open2(ctx->pCodecCtx, pCodec, NULL) < 0) return;

    ctx->pFrame = av_frame_alloc();
    ctx->pFrameRGB = av_frame_alloc();
    ctx->buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24,
                    ctx->pCodecCtx->width, ctx->pCodecCtx->height));
    avpicture_fill((AVPicture *)ctx->pFrameRGB, ctx->buffer, AV_PIX_FMT_RGB24,
                    ctx->pCodecCtx->width, ctx->pCodecCtx->height);

    ctx->sws_ctx = sws_getContext(ctx->pCodecCtx->width,
                                  ctx->pCodecCtx->height,
                                  ctx->pCodecCtx->pix_fmt,
                                  ctx->pCodecCtx->width,
                                  ctx->pCodecCtx->height,
                                  AV_PIX_FMT_RGB24,
                                  SWS_BILINEAR,
                                  NULL, NULL, NULL);

    // 获取总时长 (ms)
    if(ctx->pFormatCtx->duration != AV_NOPTS_VALUE) {
        ctx->duration = (ctx->pFormatCtx->duration / AV_TIME_BASE) * 1000;
    } else {
        ctx->duration = 0;
    }

    if(!parent) parent = lv_scr_act(); // 默认屏幕
    // 创建视频 canvas
    video_canvas_init(parent, ctx->pCodecCtx->width, ctx->pCodecCtx->height);
    lv_obj_move_foreground(canvas);

    // 创建进度条 slider
    ctx->slider = lv_slider_create(parent);
    lv_slider_set_range(ctx->slider, 0, ctx->duration > 0 ? ctx->duration : 100); // 避免0
    lv_slider_set_value(ctx->slider, 0, LV_ANIM_OFF);
    lv_obj_set_width(ctx->slider, ctx->pCodecCtx->width);
    lv_obj_align_to(ctx->slider, canvas, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(ctx->slider, slider_event_cb, LV_EVENT_PRESSED, ctx);
    lv_obj_add_event_cb(ctx->slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
    lv_obj_add_event_cb(ctx->slider, slider_event_cb, LV_EVENT_RELEASED, ctx);

    // 创建定时器逐帧播放
    lv_timer_create(video_timer_cb, 100, ctx); // 25fps

}

void GenerateMediaPath(char* folder, size_t folderSize,
                              char* videoPath, size_t videoSize,
                              int index)
{
    DateTimeStruct curDateTime = GetCurDateTime();

    // 目录: /userdata/ums/video/20250809
    snprintf(folder, folderSize, "/userdata/ums/video/%d%02d%02d",
             curDateTime.tm_year, curDateTime.tm_mon, curDateTime.tm_mday);
    CheckToCreateDir(folder);

    // 文件名前缀: 20170809_070141
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%d%02d%02d_%02d%02d%02d",
             curDateTime.tm_year, curDateTime.tm_mon, curDateTime.tm_mday,
             curDateTime.tm_hour, curDateTime.tm_min, curDateTime.tm_sec);

    // 视频路径: .../20170809_070141_001.mp4
    snprintf(videoPath, videoSize, "%s/%s_%03d.mp4", folder, prefix, index);

}

