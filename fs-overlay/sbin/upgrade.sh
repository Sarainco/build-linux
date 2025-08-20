#!/bin/sh

KERNEL_IMAGE="/app/rk3562/rk3562.itb"
FS_IMAGE="/app/rk3562/fs_overlay.tar"
RK3562_MD5="/app/md5sum"
RK3562_TAR="/app/rk3562.tar"
KERNEL_PART="/dev/mmcblk0p3"
FS_OVERLAY_DIR="/app/rk3562/fs-overlay"

log() {
    echo "[upgrade] $@"
}

usage() {
    echo "usage: $0 [kernel|fs|all]"
    exit 1
}

# 检查参数
[ $# -eq 1 ] || usage

case "$1" in
    kernel)
        do_kernel=1
        do_fs=0
        ;;
    fs)
        do_kernel=0
        do_fs=1
        ;;
    all)
        do_kernel=1
        do_fs=1
        ;;
    *)
        usage
        ;;
esac

mkdir -p /app/rk3562 && mkdir -p /app/rk3562/fs-overlay
tar -x -f "$RK3562_TAR" -C /app/rk3562
tar -x -f "$FS_IMAGE" -C /app/rk3562

# 升级文件系统
if [ "$do_fs" -eq 1 ]; then
    if [ ! -d "$FS_OVERLAY_DIR" ]; then
        log "Error: The file system overlay directory cannot be found $FS_OVERLAY_DIR"
        exit 1
    fi

    log "Verifying fs image..."
    expected=$(grep "fs_overlay.tar" "$RK3562_MD5" | awk '{print $1}')
    actual=$(md5sum "$FS_IMAGE" | awk '{print $1}')

    if [ "$expected" != "$actual" ]; then
        log "Error: fs image md5 mismatch!"
        log "Expected: $expected"
        log "Actual:   $actual"
        exit 1
    fi
    log "fs image verified successfully."

    log "Start upgrading the file system..."
    cp -a ${FS_OVERLAY_DIR}/* /
    sync
    log "The file system has been updated"

fi

# 升级内核
if [ "$do_kernel" -eq 1 ]; then
    missing=""
    [ ! -f "$KERNEL_IMAGE" ] && missing="$missing kernel image ($KERNEL_IMAGE)"
    # [ ! -f "$KERNEL_MD5" ] && missing="$missing kernel image ($KERNEL_MD5)"

    if [ -n "$missing" ]; then
        log "Error: Missing file:$missing"
        exit 1
    fi

    log "Verifying kernel image..."
    expected=$(grep "rk3562.itb" "$RK3562_MD5" | awk '{print $1}')
    actual=$(md5sum "$KERNEL_IMAGE" | awk '{print $1}')

    if [ "$expected" != "$actual" ]; then
        log "Error: Kernel image md5 mismatch!"
        log "Expected: $expected"
        log "Actual:   $actual"
        exit 1
    fi
    log "Kernel image verified successfully."

    log "Start writing to the kernel image $KERNEL_PART..."
    dd if=${KERNEL_IMAGE} of=${KERNEL_PART} conv=fsync
    sync
    log "The kernel update is completed."
fi

# 重启逻辑
if [ "$do_kernel" -eq 1 ]; then
    log "The system will restart to load the new kernel..."
    reboot -f
else
    log "The file system has been updated and there is no need to restart"
fi
