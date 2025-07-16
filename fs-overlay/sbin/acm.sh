#!/bin/sh

configfs_mount=/sys/kernel/config
usb_gadget_dir=${configfs_mount}/usb_gadget
usb_gadget=acm_gadget

gadget_instance=acm.usb0

setv() {
	echo "echo $1 > $2"
	echo $1 > $2 || exit 1
}

run () {
	echo "$@"
	"$@" >/dev/null || exit 1
}

start() {
	[ -d ${configfs_mount} ] || run mkdir -p ${configfs_mount}

	mountpoint -q ${configfs_mount} && {
		echo "Error: ${configfs_mount} mounted already, try '$0 stop'" 1>&2
		return 1
	}

	serial=$(cat /proc/cpuinfo | grep Serial | awk '{print $3}')
	[ -z "$serial" ] && serial="1234567890"

	run mount -t configfs none ${configfs_mount}
	run cd ${usb_gadget_dir}
	run mkdir ${usb_gadget}
	run cd ${usb_gadget}

	setv 0x1d6b idVendor      # Linux Foundation
	setv 0x0104 idProduct     # Multifunction Composite Gadget
	setv 0x0100 bcdDevice
	setv 0x0200 bcdUSB

	run mkdir strings/0x409
	setv "${serial}" strings/0x409/serialnumber
	setv "RK3562" strings/0x409/manufacturer
	setv "ACM USB Device" strings/0x409/product

	run mkdir configs/c.1
	run mkdir configs/c.1/strings/0x409
	setv "ACM Config" configs/c.1/strings/0x409/configuration
	setv 120 configs/c.1/MaxPower

	run mkdir functions/${gadget_instance}
	run ln -s functions/${gadget_instance} configs/c.1/f1
}

bind() {
	UDC=$(ls /sys/class/udc | head -n 1)
	[ -z "$UDC" ] && { echo "No UDC found!"; exit 1; }
	setv "$UDC" ${usb_gadget_dir}/${usb_gadget}/UDC
}

stop() {
	if [ -d ${usb_gadget_dir}/${usb_gadget} ]; then
		run cd ${usb_gadget_dir}/${usb_gadget}
	else
		mountpoint -q ${configfs_mount} && run umount ${configfs_mount}
		return 0
	fi

	configs=$(ls configs)
	for cfg in $configs; do
		functions=$(find configs/${cfg} -maxdepth 1 -type l)
		for func in $functions ; do
			run rm ${func}
		done
		strings=$(ls configs/${cfg}/strings)
		for str in ${strings}; do
			run rmdir configs/${cfg}/strings/${str}
		done
		run rmdir configs/${cfg}
	done

	functions=$(ls functions)
	for func in $functions ; do
		run rmdir functions/${func}
	done

	strings=$(ls strings)
	for str in ${strings}; do
		run rmdir strings/${str}
	done

	run cd /
	run rmdir ${usb_gadget_dir}/${usb_gadget}
	run umount ${configfs_mount}
}

case "$1" in
	start)
		start
		;;
	bind)
		bind
		;;
	stop)
		stop
		;;
	*)
		echo "Usage: $0 {start|bind|stop}"
		exit 1
		;;
esac
