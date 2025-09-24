#if 1

#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include "../../third/lvgl/src/misc/lv_fs.h"
#include "../video/video_save.h"


typedef struct 
{
    lv_obj_t * list;
    char current_path[512];
    lv_obj_t * thumb_scr;  // 缩略图独立屏幕
} file_browser_t;


static file_browser_t g_browser;

/* 判断是否目录 */
static bool is_dir(const char * path) 
{
    struct stat st;
    if(stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

/* 刷新列表 */
static void show_dir(const char *dir_path);
//static void lv_example_ffmpeg_blz(const char * play_path);
static void lv_example_open_file(const char * full_path);
extern int ffmpeg_play_video_test(const char *play_path);
// extern void video_canvas_init(lv_obj_t * parent, int width, int height);

extern void lv_example_open_video(lv_obj_t * parent, const char * full_path);

static void show_thumbnails(const char *dir_path);

#if 1

/*-----------------------------
 * File open/close/read/write/seek/tell
 *----------------------------*/

static void * lv_fs_open_cb(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    const char * real_path = path + 2; // skip "X:/"
    FILE * f = NULL;
    if(mode & LV_FS_MODE_WR) {
        f = fopen(real_path, "rb+");
        if(!f) f = fopen(real_path, "wb+");
    } else {
        f = fopen(real_path, "rb");
    }
    return f;
}

static lv_fs_res_t lv_fs_close_cb(lv_fs_drv_t * drv, void * file_p)
{
    if(file_p) fclose((FILE*)file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lv_fs_read_cb(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    size_t r = fread(buf, 1, btr, (FILE*)file_p);
    if(br) *br = r;
    return LV_FS_RES_OK;
}

static lv_fs_res_t lv_fs_write_cb(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    size_t w = fwrite(buf, 1, btw, (FILE*)file_p);
    if(bw) *bw = w;
    return LV_FS_RES_OK;
}

static lv_fs_res_t lv_fs_seek_cb(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    int origin = SEEK_SET;
    if(whence == LV_FS_SEEK_CUR) origin = SEEK_CUR;
    else if(whence == LV_FS_SEEK_END) origin = SEEK_END;
    if(fseek((FILE*)file_p, pos, origin) != 0) return LV_FS_RES_FS_ERR;
    return LV_FS_RES_OK;
}

static lv_fs_res_t lv_fs_tell_cb(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    if(!pos_p) return LV_FS_RES_INV_PARAM;
    long pos = ftell((FILE*)file_p);
    if(pos < 0) return LV_FS_RES_FS_ERR;
    *pos_p = (uint32_t)pos;
    return LV_FS_RES_OK;
}

/*-----------------------------
 * Directory open/read/close
 *----------------------------*/

typedef struct {
    DIR * dir;
} lv_fs_posix_dir_t;

static void * lv_fs_dir_open_cb(lv_fs_drv_t * drv, const char * path)
{
    const char * real_path = path + 2; // skip "X:/"
    DIR * d = opendir(real_path);
    if(!d) return NULL;
    lv_fs_posix_dir_t * pd = malloc(sizeof(lv_fs_posix_dir_t));
    pd->dir = d;
    return pd;
}

static lv_fs_res_t lv_fs_dir_read_cb(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len)
{
    lv_fs_posix_dir_t * pd = (lv_fs_posix_dir_t*)rddir_p;
    if(!pd || !pd->dir) return LV_FS_RES_FS_ERR;

    struct dirent * entry = readdir(pd->dir);
    if(!entry) return LV_FS_RES_NOT_EX; // 目录结束

    strncpy(fn, entry->d_name, fn_len-1);
    fn[fn_len-1] = '\0';
    return LV_FS_RES_OK;
}

static lv_fs_res_t lv_fs_dir_close_cb(lv_fs_drv_t * drv, void * rddir_p)
{
    lv_fs_posix_dir_t * pd = (lv_fs_posix_dir_t*)rddir_p;
    if(!pd) return LV_FS_RES_FS_ERR;
    if(pd->dir) closedir(pd->dir);
    free(pd);
    return LV_FS_RES_OK;
}

/*-----------------------------
 * Register POSIX FS
 *----------------------------*/

void lv_fs_posix_register(void)
{
    lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);

    drv.letter = 'U';
    drv.cache_size = 0;
    drv.ready_cb = NULL;

    drv.open_cb = lv_fs_open_cb;
    drv.close_cb = lv_fs_close_cb;
    drv.read_cb = lv_fs_read_cb;
    drv.write_cb = lv_fs_write_cb;
    drv.seek_cb = lv_fs_seek_cb;
    drv.tell_cb = lv_fs_tell_cb;

    drv.dir_open_cb = lv_fs_dir_open_cb;
    drv.dir_read_cb = lv_fs_dir_read_cb;
    drv.dir_close_cb = lv_fs_dir_close_cb;

    drv.user_data = NULL;

    lv_fs_drv_register(&drv);
}

#endif

#if 0
/* 全屏覆盖层点击回调，关闭显示 */
static void overlay_close_cb(lv_event_t * e)
{
    lv_obj_t * overlay = lv_event_get_target(e);
    lv_obj_t * target = lv_event_get_user_data(e);

    if(target) lv_obj_del(target);    // 删除视频/图片对象
    if(overlay) lv_obj_del(overlay);  // 删除覆盖层

    // 恢复文件浏览器列表显示
    if(g_browser.list) {
        lv_obj_clear_flag(g_browser.list, LV_OBJ_FLAG_HIDDEN);
    }
}

/* 按钮点击回调 */
static void btn_event_cb(lv_event_t * e) 
{
    lv_obj_t * btn = lv_event_get_target(e);
    const char * full_path = lv_obj_get_user_data(btn);
    if(!full_path) return;

    /* 排除特殊目录 */
    if(strcmp(full_path, "System Volume Information") == 0) return;

    if(is_dir(full_path)) 
    {
        show_dir(full_path);  // 进入子目录
    } 
    else 
    {
        LV_LOG_USER("Selected file: %s", full_path);
        // TODO: 打开文件的操作
        //lv_example_open_file(full_path);
        ///show_image_dir(full_path);
    }
}

/* 返回上一级目录按钮回调 */
static void back_event_cb(lv_event_t * e) 
{
    if(strcmp(g_browser.current_path, "/userdata/ums") == 0) return; // 已在根目录

    char * last_slash = strrchr(g_browser.current_path, '/');
    if(last_slash) {
        if(last_slash == g_browser.current_path) { // 根目录
            g_browser.current_path[1] = '\0';
        } else {
            *last_slash = '\0';
        }
    }
    show_dir(g_browser.current_path);
}

/* 显示目录内容 */
static void show_dir(const char * path) 
{
    if(!path) return;

    strncpy(g_browser.current_path, path, sizeof(g_browser.current_path)-1);
    g_browser.current_path[sizeof(g_browser.current_path)-1] = '\0';

    lv_obj_clean(g_browser.list);

    /* 返回上一级按钮 */
    if(strcmp(path, "/") != 0) 
    {
        lv_obj_t * back_btn = lv_list_add_btn(g_browser.list, LV_SYMBOL_LEFT, "Back");
        lv_obj_set_user_data(back_btn, strdup("/")); // 保存根目录，用作标记
        lv_obj_add_event_cb(back_btn, back_event_cb, LV_EVENT_CLICKED, NULL);
    }

    DIR * dir = opendir(path);
    if(!dir) {
        LV_LOG_USER("Open dir failed: %s", path);
        return;
    }

    struct dirent * entry;
    while((entry = readdir(dir)) != NULL) 
    {
        /* 排除 '.' 和 '..' */
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        /* 排除 System Volume Information */
        if(strcmp(entry->d_name, "System Volume Information") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        lv_obj_t * btn = lv_list_add_btn(g_browser.list, LV_SYMBOL_FILE, entry->d_name);
        lv_obj_set_user_data(btn, strdup(full_path));
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    }

    closedir(dir);
}
#else

#endif

/* 判断文件类型 */
static bool is_image(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if(!ext) return false;
    return (strcasecmp(ext, ".jpg") == 0 ||
            strcasecmp(ext, ".png") == 0 ||
            strcasecmp(ext, ".bmp") == 0);
}

static bool is_video(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if(!ext) return false;
    return (strcasecmp(ext, ".mp4") == 0 ||
            strcasecmp(ext, ".avi") == 0 ||
            strcasecmp(ext, ".mov") == 0);
}


/* 点击大图关闭 */
static void close_big_image_cb(lv_event_t * e)
{
    lv_obj_t * bg = lv_event_get_target(e);
    lv_obj_del(bg);
}

/* 获取相对路径并转换为 LVGL FS 格式 */
static void make_lvgl_path(const char * full_path, char * buf, size_t buf_len)
{
    // 假设 LVGL POSIX FS 映射 'U:' -> /userdata/ums
    const char * prefix = "/userdata/ums";
    const char * rel = full_path;

    if(strncmp(full_path, prefix, strlen(prefix)) == 0) {
        rel = full_path + strlen(prefix); // 去掉前缀
    }

    snprintf(buf, buf_len, "U:%s", rel); // LVGL FS 路径
    printf("make_lvgl_path %s \n", buf);
}


static lv_obj_t * g_big_img = NULL;    // 大图对象
static char g_cur_path[256] = {0};

typedef struct {
    char thumb_path[256];   // 缩略图路径
    char video_path[256];   // 视频原文件路径
} ThumbUserData;

static void big_img_click_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);
    if(cont) lv_obj_del(cont);
    if(g_browser.thumb_scr) lv_obj_clear_flag(g_browser.thumb_scr, LV_OBJ_FLAG_HIDDEN);
}

/* 点击关闭视频的回调 */
static void video_close_cb(lv_event_t * e)
{
    lv_obj_t * video_cont = lv_event_get_target(e);
    video_context_t *ctx = (video_context_t *)lv_event_get_user_data(e);

    if(ctx) {
        /* 停止定时器 */
        if(ctx->timer) lv_timer_del(ctx->timer);

        /* 释放 FFmpeg 资源 */
        if(ctx->pFrame) av_frame_free(&ctx->pFrame);
        if(ctx->pFrameRGB) av_frame_free(&ctx->pFrameRGB);
        if(ctx->buffer) av_free(ctx->buffer);
        if(ctx->pCodecCtx) avcodec_close(ctx->pCodecCtx);
        if(ctx->pFormatCtx) avformat_close_input(&ctx->pFormatCtx);
        if(ctx->sws_ctx) sws_freeContext(ctx->sws_ctx);

        free(ctx);
    }

    lv_obj_del(video_cont); // 删除全屏容器

    /* 显示缩略图 */
    if(g_browser.thumb_scr)
        lv_obj_clear_flag(g_browser.thumb_scr, LV_OBJ_FLAG_HIDDEN);
}

static void thumb_click_cb(lv_event_t * e)
{
    ThumbUserData *ud = lv_event_get_user_data(e);
    if(!ud) return;
    printf("thumb_click_cb %s \n", ud->video_path);

    if(is_image(ud->video_path)) {
        char lvgl_path[256];
        if(g_browser.thumb_scr) lv_obj_add_flag(g_browser.thumb_scr, LV_OBJ_FLAG_HIDDEN);

        make_lvgl_path(ud->video_path, lvgl_path, sizeof(lvgl_path));

        /* 全屏容器 */
        lv_obj_t * big_cont = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(big_cont);
        lv_obj_set_size(big_cont, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
        lv_obj_add_flag(big_cont, LV_OBJ_FLAG_CLICKABLE);

        /* 图片 */
        g_big_img = lv_img_create(big_cont);
        lv_img_set_src(g_big_img, lvgl_path);
        lv_obj_center(g_big_img);

        lv_obj_add_event_cb(big_cont, big_img_click_cb, LV_EVENT_CLICKED, NULL);
    } else if(is_video(ud->video_path)) {
        //todo
        /* 隐藏缩略图 */
        if(g_browser.thumb_scr)
            lv_obj_add_flag(g_browser.thumb_scr, LV_OBJ_FLAG_HIDDEN);

        /* 创建全屏视频容器 */
        lv_obj_t * video_cont = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(video_cont);
        lv_obj_set_size(video_cont, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
        lv_obj_add_flag(video_cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_move_foreground(video_cont);

        /* 播放视频 */
        lv_example_open_video(video_cont, ud->video_path);

        /* 给全屏容器添加点击事件 */
        lv_obj_add_event_cb(video_cont, video_click_cb, LV_EVENT_CLICKED, NULL);
    }
}

/* 计算缩略图缩放比例，保证完整显示 */
static int calc_fit_zoom(int orig_w, int orig_h, int target_w, int target_h)
{
    if (orig_w <= 0 || orig_h <= 0) return 256; // 避免除零，保持原大小

    int zoom_w = 256 * target_w / orig_w;
    int zoom_h = 256 * target_h / orig_h;

    /* 取较小的比例，保证完整显示 */
    int zoom = zoom_w < zoom_h ? zoom_w : zoom_h;

    /* 限制 zoom 不小于 1 */
    if (zoom < 1) zoom = 1;

    return zoom;
}

#if 1//20250923

static bool is_date_folder(const char *name)
{
    // 简单判断：YYYYMMDD（8位数字）
    if(strlen(name) != 8) return false;
    for(int i = 0; i < 8; i++) {
        if(!isdigit((unsigned char)name[i])) return false;
    }
    return true;
}

/* 点击事件：目录/文件/日期目录分流 */
static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    char * path = (char *)lv_obj_get_user_data(btn);
    if(!path) return;

    /* 返回上一级 */
    if(strcmp(path, "..") == 0) {
        // 不允许返回到 /userdata 上层
        if(strcmp(g_browser.current_path, "/userdata/ums") == 0) {
            // 已经到顶，不再返回
            show_dir("/userdata/ums");
            return;
        }
        char *last = strrchr(g_browser.current_path, '/');
        if(last && last != g_browser.current_path) {
            *last = '\0';
            show_dir(g_browser.current_path);
        } else {
            show_dir("/userdata/ums");
        }
        return;
    }

    struct stat st;
    if(stat(path, &st) == 0) {
        if(S_ISDIR(st.st_mode)) {
            /* 判断是不是日期目录 */
            const char * folder_name = strrchr(path, '/');
            folder_name = folder_name ? folder_name + 1 : path;

            if(is_date_folder(folder_name)) {
                /* 日期目录 → 显示缩略图 */
                show_thumbnails(path);
            } else {
                /* 普通目录 → 继续浏览 */
                show_dir(path);
            }
        } else {
            /* 文件点击逻辑 */
            LV_LOG_USER("Clicked file: %s", path);
        }
    }

}


/* 定时器回调：在安全时机执行删除与界面切换 */
static void back_btn_timer_cb(lv_timer_t * t)
{
    /* 删除缩略图容器（如果存在且有效） */
    if(g_browser.thumb_scr && lv_obj_is_valid(g_browser.thumb_scr)) {
        lv_obj_del(g_browser.thumb_scr);
        g_browser.thumb_scr = NULL;
    }

    /* 显示 UMS 目录 */
    if(g_browser.list && lv_obj_is_valid(g_browser.list)) {
        show_dir(g_browser.current_path);
    }

    /* 删除一次性定时器 */
    lv_timer_del(t);
}

/* back 按钮事件回调（事件里只创建定时器，不做删除） */
static void back_btn_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    /* 创建一个短延迟的定时器（1ms）来执行实际删除/切换工作 */
    lv_timer_t * t = lv_timer_create(back_btn_timer_cb, 1, NULL);
    (void)t; /* 不需要保留 t 指针，回调会自删除 */
}

/* 缩略图模式 */
static void show_thumbnails(const char *dir_path)
{
    DIR * dir = opendir(dir_path);
    if(!dir) return;

    //lv_obj_clean(lv_scr_act());  // 清空之前界面
    g_browser.thumb_scr = lv_list_create(lv_scr_act());
    lv_obj_set_size(g_browser.thumb_scr, 240, 320);
    lv_obj_center(g_browser.thumb_scr);

    lv_obj_set_flex_flow(g_browser.thumb_scr, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_scroll_dir(g_browser.thumb_scr, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_browser.thumb_scr, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(g_browser.thumb_scr, 0, 0);

        // int remainder = thumb_count % 3;
    // int back_btns = (remainder == 0) ? 1 : 3 - remainder - 2;
    lv_obj_t * back_btn = lv_btn_create(g_browser.thumb_scr);
    lv_obj_remove_style_all(back_btn);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(back_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_size(back_btn, 240, 30);

    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_16, 0); 
    lv_label_set_text(back_label, "< Back");
    //lv_obj_center(back_label);
    lv_obj_align(back_label, LV_ALIGN_LEFT_MID, 10, 0);  // 距左边 10px

    lv_obj_add_event_cb(back_btn, back_btn_cb, LV_EVENT_CLICKED, NULL);

    //int thumb_count = 0;
    struct dirent * entry;
    while((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0] == '.') continue;
        if(entry->d_type != DT_REG) continue;

        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        char lvgl_path[256];
        // make_lvgl_path(full_path, lvgl_path, sizeof(lvgl_path));
        char thumb_path[256];
        if(is_image(full_path)) {
            make_lvgl_path(full_path, lvgl_path, sizeof(lvgl_path));
        } else if(is_video(full_path)) {
            // video: 构造缩略图路径：thumb/<日期>/<filename>.png
            const char *date_folder = strrchr(dir_path, '/');
            if(date_folder) date_folder++; // 指向日期目录名
            snprintf(thumb_path, sizeof(thumb_path), "/userdata/ums/thumb/%s/%s", date_folder, entry->d_name);
            char *dot = strrchr(thumb_path, '.');
            if(dot) *dot = '\0';        // 去掉扩展名
            strncat(thumb_path, ".png", sizeof(thumb_path) - strlen(thumb_path) - 1);
            make_lvgl_path(thumb_path, lvgl_path, sizeof(lvgl_path));
        } else {
            continue;
        }

        /* 缩略图按钮 */
        lv_obj_t * btn = lv_btn_create(g_browser.thumb_scr);
        lv_obj_remove_style_all(btn);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_size(btn, 78, 100);

        lv_obj_t * img = lv_img_create(btn);
        lv_img_set_src(img, lvgl_path);
        lv_img_set_zoom(img, 128);
        lv_obj_center(img);

        /* 视频播放图标叠加 */
        if(is_video(full_path)) {
            lv_obj_t * play_icon = lv_img_create(btn);
            lv_img_set_src(play_icon, LV_SYMBOL_PLAY);
            lv_obj_align(play_icon, LV_ALIGN_CENTER, 0, 0);
        }

        /* 设置用户数据：缩略图路径 + 视频路径 */
        ThumbUserData *ud = malloc(sizeof(ThumbUserData));
        strcpy(ud->thumb_path, lvgl_path);
        strcpy(ud->video_path, full_path);  // 原视频路径

        lv_obj_add_event_cb(btn, thumb_click_cb, LV_EVENT_CLICKED, ud);
        //thumb_count++;
    }

    closedir(dir);
    
}

/* 通用目录浏览 */
static void show_dir(const char *path)
{
    if(!path) return;

    strncpy(g_browser.current_path, path, sizeof(g_browser.current_path)-1);
    g_browser.current_path[sizeof(g_browser.current_path)-1] = '\0';

    // lv_obj_clean(lv_scr_act());  // 清屏
    // g_browser.list = lv_list_create(lv_scr_act());
    // lv_obj_set_size(g_browser.list, 240, 360);
    // lv_obj_center(g_browser.list);

    lv_obj_clean(g_browser.list);
    /* 返回按钮 */
    if(strcmp(path, "/") != 0) {
        lv_obj_t * back_btn = lv_list_add_btn(g_browser.list, LV_SYMBOL_LEFT, "Back");
        lv_obj_set_style_text_font(back_btn, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_user_data(back_btn, strdup(".."));
        lv_obj_add_event_cb(back_btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    }

    DIR *dir = opendir(path);
    if(!dir) return;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..")  == 0 || strcmp(entry->d_name, "thumb") == 0
                                        || strcmp(entry->d_name, "System Volume Information") == 0)
            continue;
        if(entry->d_type != DT_DIR) continue;  // 只显示目录

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if(stat(full_path, &st) == 0) {
           if(stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
               // char *icon = is_date_folder(entry->d_name) ? LV_SYMBOL_IMAGE : LV_SYMBOL_DIRECTORY;
                lv_obj_t * btn = lv_list_add_btn(g_browser.list, LV_SYMBOL_DIRECTORY, entry->d_name);
                lv_obj_set_style_text_font(g_browser.list, &lv_font_montserrat_16, LV_PART_MAIN);
                lv_obj_set_user_data(btn, strdup(full_path));
                lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
            }
        }
    }
    closedir(dir);
}


#endif

/* 创建文件浏览器 */
lv_obj_t * file_browser_create(lv_obj_t * parent, const char * start_path) 
{
    lv_fs_posix_register();

    g_browser.list = lv_list_create(parent);
    lv_obj_set_size(g_browser.list, lv_pct(100), lv_pct(100));
    lv_obj_align(g_browser.list, LV_ALIGN_CENTER, 0, 0);


    if(!start_path) start_path = "/userdata/ums";
    show_dir(start_path);
    //show_image_dir(start_path);

    return g_browser.list;
}

#if 0
static void lv_example_ffmpeg_blz(const char * play_path)
{
    lv_obj_t * player = lv_ffmpeg_player_create(lv_screen_active());
    lv_ffmpeg_player_set_src(player, play_path);
    lv_ffmpeg_player_set_auto_restart(player, true);
    lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
    lv_obj_center(player);
}
#endif


/* 打开文件（视频或图片） */
static void lv_example_open_file(const char * full_path)
{
    // 隐藏文件浏览器列表
    if(g_browser.list) {
        lv_obj_add_flag(g_browser.list, LV_OBJ_FLAG_HIDDEN);
    }

    //lv_example_open_video(full_path);

    lv_obj_t * obj = NULL;

    lv_obj_t * overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(overlay, 0, 0); // 完全透明
    //lv_obj_add_event_cb(overlay, overlay_close_cb, LV_EVENT_CLICKED, obj);
    lv_obj_move_foreground(overlay);
}

#else
/*************************************************************************/
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "m_lv_filepicker.h"
#include "dispatcher/subject_dispatcher.h"

typedef struct {
    WidgetBaseStruct base;       // 基类
    lv_obj_t *list;              // 文件列表
    char current_path[512];      // 当前目录路径
    uint32_t id;                 // 页面ID
} FileBrowserViewStruct;


static FileBrowserViewStruct g_browser;

/* 判断是否目录 */
static bool is_dir(const char * path) 
{
    struct stat st;
    if(stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

/* 刷新列表 */
static void show_dir(FileBrowserViewStruct* fb, const char * path);
//static void lv_example_ffmpeg_blz(const char * play_path);
static void lv_example_open_file(const char * full_path);
extern int ffmpeg_play_video_test(const char *play_path);
// extern void video_canvas_init(lv_obj_t * parent, int width, int height);

extern void lv_example_open_video(const char * full_path);

/* 全屏覆盖层点击回调，关闭显示 */
// static void overlay_close_cb(lv_event_t * e)
// {
//     lv_obj_t * overlay = lv_event_get_target(e);
//     lv_obj_t * target = lv_event_get_user_data(e);

//     if(target) lv_obj_del(target);    // 删除视频/图片对象
//     if(overlay) lv_obj_del(overlay);  // 删除覆盖层

//     // 恢复文件浏览器列表显示
//     if(g_browser.list) {
//         lv_obj_clear_flag(g_browser.list, LV_OBJ_FLAG_HIDDEN);
//     }
// }

/* 文件按钮点击回调 */
static void btn_event_cb(lv_event_t * e) 
{
    lv_obj_t * btn = lv_event_get_target(e);
    const char * full_path = lv_obj_get_user_data(btn);
    if (!full_path) return;

    /* 排除特殊目录 */
    if (strcmp(full_path, "System Volume Information") == 0) return;

    /* 获取当前文件浏览器对象 */
    FileBrowserViewStruct* fb = (FileBrowserViewStruct*)lv_event_get_user_data(e);
    if (!fb || !fb->list) return;

    if (is_dir(full_path)) 
    {
        show_dir(fb, full_path);  // 进入子目录
    } 
    else 
    {
        LV_LOG_USER("Selected file: %s", full_path);
        //lv_example_open_file(full_path);
    }
}

/* 返回上一级目录按钮回调 */
static void back_event_cb(lv_event_t * e) 
{
    FileBrowserViewStruct* fb = (FileBrowserViewStruct*)lv_event_get_user_data(e);
    if (!fb || !fb->list) return;

    /* 已在根目录，直接返回 */
    if (strcmp(fb->current_path, "/userdata/ums") == 0) return;

    char * last_slash = strrchr(fb->current_path, '/');
    if (last_slash) {
        if (last_slash == fb->current_path) { // 根目录
            fb->current_path[1] = '\0';
        } else {
            *last_slash = '\0';
        }
    }

    show_dir(fb, fb->current_path);
}


/* 显示目录内容，传入 FileBrowserViewStruct 指针 */
static void show_dir(FileBrowserViewStruct* fb, const char * path) 
{
    if (!fb || !fb->list || !path) return;

    strncpy(fb->current_path, path, sizeof(fb->current_path)-1);
    fb->current_path[sizeof(fb->current_path)-1] = '\0';

    /* 安全清空列表 */
    lv_obj_clean(fb->list);

    /* 返回上一级按钮 */
    if(strcmp(path, "/") != 0) 
    {
        lv_obj_t * back_btn = lv_list_add_btn(fb->list, LV_SYMBOL_LEFT, "Back");
        lv_obj_set_user_data(back_btn, strdup("/")); // 保存根目录，用作标记
        lv_obj_add_event_cb(back_btn, back_event_cb, LV_EVENT_CLICKED, fb);
    }

    DIR * dir = opendir(path);
    if(!dir) {
        LV_LOG_USER("Open dir failed: %s", path);
        return;
    }

    struct dirent * entry;
    while((entry = readdir(dir)) != NULL) 
    {
        /* 排除 '.' 和 '..' */
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        /* 排除 System Volume Information */
        if(strcmp(entry->d_name, "System Volume Information") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        /* 根据类型显示图标 */
        const char * icon = is_dir(full_path) ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;
        lv_obj_t * btn = lv_list_add_btn(fb->list, icon, entry->d_name);
        lv_obj_set_user_data(btn, strdup(full_path));
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, fb);
    }

    closedir(dir);
}


/* 打开文件（视频或图片） */
static void lv_example_open_file(const char * full_path)
{
    // 隐藏文件浏览器列表
    // if(g_browser.list) {
    //     lv_obj_add_flag(g_browser.list, LV_OBJ_FLAG_HIDDEN);
    // }

    lv_example_open_video(full_path);

    // lv_obj_t * obj = NULL;

    // lv_obj_t * overlay = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    // lv_obj_set_style_bg_opa(overlay, 0, 0); // 完全透明
    // //lv_obj_add_event_cb(overlay, overlay_close_cb, LV_EVENT_CLICKED, obj);
    // lv_obj_move_foreground(overlay);
}

#if 0
/* 文件浏览器刷新回调 */
static void ProcessFileBrowserRefreshSubject(const SubjectBaseDataStruct* pSubjectData,
                                             WidgetBaseStruct* pThis)
{
    printf("[FileBrowser] Refresh callback pThis=%p\n", pThis);
    if (!pThis) {
        printf("[FileBrowser] Refresh ignored: pThis == NULL\n");
        return;
    }

    FileBrowserViewStruct* fb = (FileBrowserViewStruct*)pThis;
    if (!fb->list) {
        printf("[FileBrowser] Refresh ignored: fb->list == NULL (path=%s)\n",
               fb->current_path[0] ? fb->current_path : "(empty)");
        return;
    }

    /* 清空列表并重新加载 */
    lv_obj_clean(fb->list);

    if (fb->current_path[0]) {
        show_dir(fb->current_path);
        printf("[FileBrowser] Refresh OK: path=%s\n", fb->current_path);
    } else {
        printf("[FileBrowser] Refresh skipped: no path set\n");
    }
}
#endif


WidgetBaseStruct* UI_CreateFileBrowserView(lv_obj_t* parent, const uint32_t id)
{
    WidgetBaseStruct* pWidget = UI_CreateWidget(sizeof(FileBrowserViewStruct), parent);
    if (pWidget)
    {
        /** 容器初始化 */
        lv_obj_t* pContainer = UI_GetWidgetObj(pWidget);
        lv_obj_set_style_pad_ver(pContainer, 0, ALL_PART_AND_STATE_STYLE);
        lv_obj_set_style_pad_row(pContainer, 0, ALL_PART_AND_STATE_STYLE);
        lv_obj_set_style_radius(pContainer, 0, ALL_PART_AND_STATE_STYLE);
        lv_obj_set_style_bg_color(pContainer, lv_color_hex(STYLE_OBJ_VIEW_BG_COLOR), ALL_PART_AND_STATE_STYLE);
        lv_obj_set_flex_flow(pContainer, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(pContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        /** 创建标题栏 */
        //CreateTitleArea(pContainer, pWidget);

        /** 创建主体区域 */
        FileBrowserViewStruct* pFileBrowser = (FileBrowserViewStruct*)pWidget;
        pFileBrowser->list = lv_list_create(pContainer);
        printf("[FileBrowser] Create list=%p container=%p\n", pFileBrowser->list, pContainer);
        lv_obj_set_size(pFileBrowser->list, lv_pct(100), lv_pct(100));
        lv_obj_align(pFileBrowser->list, LV_ALIGN_CENTER, 0, 0);

        /** 设置起始路径 */
        const char *start_path = "/userdata/ums";
        strncpy(pFileBrowser->current_path, start_path, sizeof(pFileBrowser->current_path));
        FileBrowserViewStruct* fb = (FileBrowserViewStruct*)pWidget;
        show_dir(fb, "/userdata/ums");


        /** 保存 page id */
        pFileBrowser->id = id;

        /** 订阅（用于刷新目录） */
        //UI_SubjectSubscribe(E_SUBJECT_ID_FILE_REFRESH, ProcessFileBrowserRefreshSubject, pWidget);
    }

    return pWidget;
}


#endif

