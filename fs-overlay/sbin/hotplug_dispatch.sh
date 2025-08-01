#!/bin/sh

# LOG="/tmp/hotplug_debug.log"
# echo "$(date) ACTION=$ACTION SUBSYSTEM=$SUBSYSTEM DEVPATH=$DEVPATH MDEV=$MDEV" >> $LOG

case "$SUBSYSTEM" in
    block)
        # echo "$(date) - block device $MDEV" >> $LOG
        /sbin/mdev "$@"  # 触发 /etc/mdev.conf 中的规则
        ;;
    android_usb)
        # echo "$(date) - usb gadget event: calling ums_hotplug.sh" >> $LOG
        /sbin/ums_hotplug.sh "$@"
        ;;
    *)
        # echo "$(date) - unhandled SUBSYSTEM=$SUBSYSTEM" >> $LOG
        ;;
esac
