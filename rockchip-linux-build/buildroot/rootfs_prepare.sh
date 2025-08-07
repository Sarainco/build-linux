#!/bin/bash
set -e

# ==== 参数 ====
BUILDROOT_DIR=$1
BOARD=rockchip_rk3568   # 可根据需要固定或再传入

if [ -z "$BUILDROOT_DIR" ]; then
    echo "用法: $0 <buildroot目录路径>"
    echo "例如: $0 stating/source/rk3568_linux_sdk/buildroot"
    exit 1
fi

# ==== 转到 buildroot 目录 ====
cd "$BUILDROOT_DIR"

# ==== 加载环境 ====
source build/envsetup.sh "$BOARD"
