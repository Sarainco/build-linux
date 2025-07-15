#!/bin/sh

# 文件名：/etc/init.d/S01resize_rootfs
# 作用：首次开机扩展根文件系统到整个分区
# 标志文件：/var/.resized_done

MARKER_FILE="/var/.resized_done"
ROOT_DEV="/dev/mmcblk0p6"

echo "[resize_rootfs] 检查是否需要执行 resize2fs..."

if [ ! -f "$MARKER_FILE" ]; then
    echo "[resize_rootfs] 开始扩展 rootfs 到整个 $ROOT_DEV ..."
    resize2fs $ROOT_DEV
    if [ $? -eq 0 ]; then
        echo "[resize_rootfs] 扩容成功，记录标志位"
        touch "$MARKER_FILE"
    else
        echo "[resize_rootfs] 扩容失败！"
    fi
else
    echo "[resize_rootfs] 已执行过扩容，跳过"
fi

