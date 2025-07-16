#!/bin/sh

case "$1" in
    ums_en)
        echo "[usb_mode] Enabling UMS gadget..."
        ums.sh start || exit 1
        ums.sh bind || exit 1
        echo /sbin/ums_hotplug.sh > /proc/sys/kernel/hotplug
        ums.sh update || exit 1
        echo "ums" > /tmp/.usb_config
        ;;
    ums_enable)
        echo "[usb_mode] Enabling UMS file..."
        ums.sh enable || exit 1
        ;;
    ums_disable)
        echo "[usb_mode] Disabling UMS file..."
        ums.sh disable || exit 1
        ;;
    ums_stop)
        echo "[usb_mode] Stopping UMS gadget..."
        ums.sh stop || exit 1
        echo "" > /tmp/.usb_config
        ;;
    acm_en)
        echo "[usb_mode] Enabling ACM gadget..."
        acm.sh start || exit 1
        acm.sh bind || exit 1
        echo "acm" > /tmp/.usb_config
        ;;
    acm_stop)
        echo "[usb_mode] Stopping ACM gadget..."
        acm.sh stop || exit 1
        echo "" > /tmp/.usb_config
        ;;
    *)
        echo "Usage: $0 {ums_en|ums_enable|ums_disable|ums_stop|acm_en|acm_stop}"
        exit 1
        ;;
esac

