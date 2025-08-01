#!/bin/sh
# LOG="/tmp/udisk_action.log"
# echo "$(date): ACTION=$ACTION MDEV=$MDEV DEVPATH=$DEVPATH" >> $LOG

if [ $ACTION == "add" ] && [ -e "/dev/$MDEV" ] ; then

    MOUNT_PATH="/mnt/usbdisk/$MDEV"
    FORMAT=`lsblk -l -o NAME,FSTYPE | grep "$MDEV " | awk '{print $2}' `
    
    # 如果挂载点不存在，那么就自动创建挂载点
    if [ ! -z $FORMAT ] && [ ! -x $MOUNT_PATH ]; then
        mkdir -p $MOUNT_PATH
    fi

    echo "[udisk_insert] $MDEV $MOUNT_PATH $FORMAT" > /dev/console   

    # 针对格式进行挂载
    if [ $FORMAT == "vfat" ]; then
        mount -t vfat /dev/$MDEV $MOUNT_PATH
        echo "[udisk_insert] mount -t vfat /dev/$MDEV $MOUNT_PATH" > /dev/console
    elif [ $FORMAT == "ntfs" ]; then
        if [ -f /bin/ntfs-3g ]; then
            ntfs-3g /dev/$MDEV $MOUNT_PATH
            echo "[udisk_insert] /ntfs-3g /dev/$MDEV $MOUNT_PATH" > /dev/console
        else
            mount -t ntfs /dev/$MDEV $MOUNT_PATH
            echo "[udisk_insert] mount -t ntfs /dev/$MDEV $MOUNT_PATH" > /dev/console
        fi
    elif [ "$FORMAT" == "exfat" ]; then
    export LD_LIBRARY_PATH=/opt/exfat/lib/:/opt/fuse/lib:${LD_LIBRARY_PATH}
    /opt/exfat/sbin/mount.exfat /dev/$MDEV $MOUNT_PATH
    echo "[udisk_insert] mount.exfat /dev/$MDEV $MOUNT_PATH" > /dev/console
    # 没有针对的格式，直接进行挂载
    elif [ ! -z $FORMAT ]; then
        mount /dev/$MDEV $MOUNT_PATH
        echo "[udisk_insert] mount /dev/$MDEV $MOUNT_PATH" > /dev/console
    fi

fi

if [ $ACTION == "remove" ]; then
    echo "[udisk_remove] removing $MDEV" > /dev/console
    umount -l /mnt/usbdisk/${MDEV}*
    rm -r /mnt/usbdisk/sd*
fi