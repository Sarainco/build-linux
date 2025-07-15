#!/bin/sh

configfs_mount=/sys/kernel/config
usb_gadget_dir=${configfs_mount}/usb_gadget
usb_gadget=rockchip

gadget_id=0x0000
gadget_instance=mass_storage.0

ums_file=/userdata/ums.img
ums_size=10240M
ums_mount_dir=/userdata/ums

lun_file=${usb_gadget_dir}/${usb_gadget}/functions/${gadget_instance}/lun.0/file

setv() {
	echo "echo $1 > $2"
	echo $1 > $2 || exit 1
}

run () {
        echo "$@"
        "$@" >/dev/null || exit 1
}

start () {
	[ -d ${configfs_mount} ] || run mkdir -p ${configfs_mount}

	mountpoint -q ${configfs_mount} && {
		echo "Error: ${configfs_mount} mounted already, try $0 destory'" 1>&2
		return 1
	}


	serial=`cat /proc/cpuinfo | grep Serial | awk '{print $3}'`

	run mount -t configfs none ${configfs_mount}
	run cd ${usb_gadget_dir}
	run mkdir ${usb_gadget}
	run cd ${usb_gadget}

	setv 0x2207 idVendor
	setv 0x0310 bcdDevice
	setv 0x0200 bcdUSB

	run mkdir strings/0x409
	setv "${serial}" strings/0x409/serialnumber
	setv rockchip strings/0x409/manufacturer
	setv rk3568 strings/0x409/product

	run mkdir functions/${gadget_instance}
	run mkdir configs/b.1
	run mkdir configs/b.1/strings/0x409

	setv 0x1 os_desc/b_vendor_code
	setv MSFT100 os_desc/qw_sign
	setv 500 configs/b.1/MaxPower

	run ln -s configs/b.1 os_desc/b.1

	setv ${gadget_id} idProduct
	run ln -s functions/${gadget_instance} configs/b.1/f1
}

bind ()
{
	UDC=`ls /sys/class/udc/| awk '{print $1}'`
	setv "$UDC" ${usb_gadget_dir}/${usb_gadget}/UDC
}

stop ()
{
	if [ -d ${usb_gadget_dir}/${usb_gadget} ] ; then
		run cd ${usb_gadget_dir}/${usb_gadget}
	else
		mountpoint -q ${configfs_mount} && run umount ${configfs_mount}
		return 0
	fi
	
	desc_cfgs=`find os_desc -maxdepth 1 -type l`
	for desc in ${desc_cfgs}; do
		run rm ${desc}
	done

	configs=`ls configs`
	for cfg in $configs; do
		functions=`find configs/${cfg} -maxdepth 1 -type l`
		for func in $functions ; do
			run rm ${func}
		done
		strings=`ls configs/${cfg}/strings`
		for str in ${strings}; do
			run rmdir configs/${cfg}/strings/${str}
		done
		run rmdir configs/${cfg}
	done

	functions=`ls functions`
	for func in $functions ; do
		run rmdir functions/${func}
	done

	strings=`ls strings`
	for str in ${strings}; do
		run rmdir strings/${str}
	done

	run cd /
	run rmdir ${usb_gadget_dir}/${usb_gadget}
	run umount ${configfs_mount}
}

enable () {
	mountpoint -q ${ums_mount_dir} && {
		run umount ${ums_mount_dir} || return 1
	}

	setv "${ums_file}" "${lun_file}"
}

disable () {
	setv "" "${lun_file}"

	mountpoint -q ${ums_mount_dir} || {
		run mount -t vfat ${ums_file} ${ums_mount_dir}
	}
}

update () {
	run mkdir -p ${ums_mount_dir}

	[ -f ${ums_file} ] || {
		run truncate -s ${ums_size} ${ums_file}
		run mkfs.vfat ${ums_file}
	}

	USB_STATE=$(cat /sys/class/android_usb/android*/state)

	if [ "$USB_STATE" = "CONFIGURED" ] ; then
		enable
	else
		disable
	fi
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
enable)
	enable
	;;
disable)
	disable
	;;
update)
	update
	;;
*)
	echo "Usage: $0 {start|bind|stop}" 1>&2
	;;
esac

