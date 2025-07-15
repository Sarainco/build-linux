#!/bin/sh

[ "$1" = "android_usb" ] && {
	if [ "$USB_STATE" = "DISCONNECTED" ] ; then
		ums.sh disable
	elif [ "$USB_STATE" = "CONFIGURED" ] ; then
		ums.sh enable
	fi
}

