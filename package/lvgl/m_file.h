#ifndef __M_LV_FILEPICKER_H__
#define __M_LV_FILEPICKER_H__

#include "lvgl.h"

/**
 * @brief 创建文件选择器
 * @param parent 父对象
 * @param path 要显示的目录路径
 * @param cb 点击文件的回调函数
 * @return 返回列表对象
 */
// lv_obj_t * m_lv_filepicker_create(lv_obj_t * parent, const char * path, lv_event_cb_t cb);
lv_obj_t * file_browser_create(lv_obj_t * parent, const char * start_path);

#endif /* M_LV_FILEPICKER_H */
