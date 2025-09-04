#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct 
{
    lv_obj_t * list;
    char current_path[512];
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
static void show_dir(const char * path);
//static void lv_example_ffmpeg_blz(const char * play_path);
static void lv_example_open_file(const char * full_path);
extern int ffmpeg_play_video_test(const char *play_path);
// extern void video_canvas_init(lv_obj_t * parent, int width, int height);

extern void lv_example_open_video(const char * full_path);

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
        lv_example_open_file(full_path);
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

/* 创建文件浏览器 */
lv_obj_t * file_browser_create(lv_obj_t * parent, const char * start_path) 
{
    g_browser.list = lv_list_create(parent);
    lv_obj_set_size(g_browser.list, lv_pct(100), lv_pct(100));
    lv_obj_align(g_browser.list, LV_ALIGN_CENTER, 0, 0);

    if(!start_path) start_path = "/userdata/ums";
    show_dir(start_path);

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

#if 0
    // 判断是否为图片（可根据文件扩展名扩展）
    bool is_image = false;
    const char * ext = strrchr(full_path, '.');
    if(ext) {
        if(strcmp(ext, ".mp4") == 0 || strcmp(ext, ".png") == 0) is_image = true;
    }

    lv_obj_t * obj = NULL;

    video_canvas_init(lv_scr_act(), 360, 240);

    lv_timer_handler();
    ffmpeg_play_video_test(full_path);

    // 创建全屏透明覆盖层，绑定点击关闭
    lv_obj_t * overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(overlay, 0, 0); // 完全透明
    lv_obj_add_event_cb(overlay, overlay_close_cb, LV_EVENT_CLICKED, obj);
    lv_obj_move_foreground(overlay);
#endif

    lv_example_open_video(full_path);

    lv_obj_t * obj = NULL;

    lv_obj_t * overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(overlay, 0, 0); // 完全透明
    lv_obj_add_event_cb(overlay, overlay_close_cb, LV_EVENT_CLICKED, obj);
    lv_obj_move_foreground(overlay);
}

