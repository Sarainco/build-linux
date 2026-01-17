









# -------------------------------------------------
# kernel
# -------------------------------------------------


# -------------------------------------------------
# Enable package
# -------------------------------------------------
ifeq ($(CONFIG_KERNEL_ROCKCHIP_5_15),y)

# PACKAGE_TARGETS += 

# =================================================
# 1. Extracting
# =================================================


# =================================================
# 2. Patching
# =================================================


# =================================================
# 3. Configuring
# =================================================
# /home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
# make ARCH=arm64 CROSS_COMPILE=/home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- rockchip_linux_defconfig
kernel/prepare:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) rockchip_linux_defconfig

kernel/menuconfig:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) menuconfig

# =================================================
# 4. Building
# =================================================
kernel/compile:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) rk3568-kickpi-k1-linux.img

# =================================================
# 5. Installing
# =================================================


# =================================================
# Meta targets
# =================================================
kernel/clean:
	make -j8 CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) distclean

endif
