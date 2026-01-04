
ARCH := arm64
CROSS_COMPILE := /home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-


kernel/prepare:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) rockchip_linux_defconfig

kernel/menuconfig:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) menuconfig

kernel/compile:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) rk3568-kickpi-k1-linux.img

kernel/clean:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) distclean

