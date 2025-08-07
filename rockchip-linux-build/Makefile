# 项目 Makefile：支持 RK3568 平台 SDK 的初始化与构建

SHELL := /bin/bash

# ===== 配置变量 =====
SDK_TAR      := ../rk3568_dl/atompi-ca1_buildroot_release_v1.0_20240129.tgz
BUILDROOT_DL := ../rk3568_dl/dl.tgz
SDK_SRC_DIR  := stating/source/rk3568_linux_sdk
SDK_BUILD_DIR:= stating/build
BOARD_CONFIG := BoardConfig-rk3568-atk-atompi-ca1.mk
BUILDROOT_DIR := buildroot
KERNEL_DIR := kernel
UBOOT_DIR := uboot
IMAGE_DIR := $(SDK_BUILD_DIR)/image
FS_OVERLAY_DIR := $(SDK_SRC_DIR)/buildroot/board/rockchip/rk356x/fs-overlay


# ===== 默认目标 =====
.PHONY: all
all: buildall

# ===== SDK 初始化 =====
.PHONY: sdk-prepare
sdk-prepare:
	@echo "==> 解压 SDK..."
# 	mkdir -p $(SDK_SRC_DIR)
# 	tar xvf $(SDK_TAR) -C $(SDK_SRC_DIR)
# 	cd $(SDK_SRC_DIR) && .repo/repo/repo sync -l -j8
	tar xzf $(BUILDROOT_DL) -C $(SDK_SRC_DIR)/buildroot/

# ===== 环境配置 =====
.PHONY: envset
envset:
	@echo "==> 设置开发板配置环境变量..."
	cp $(UBOOT_DIR)/*defconfig $(SDK_SRC_DIR)/u-boot/configs/rk3568_defconfig
	cp $(BUILDROOT_DIR)/*defconfig $(SDK_SRC_DIR)/buildroot/configs/rockchip_rk3568_defconfig
#cp $(KERNEL_DIR)/*defconfig $(SDK_SRC_DIR)/kernel/arch/arm64/configs/rockchip_linux_defconfig
	cd $(SDK_SRC_DIR) && ./build.sh $(BOARD_CONFIG)
	cd $(SDK_SRC_DIR)/buildroot && source build/envsetup.sh rockchip_rk3568


UBOOT_PATCH_TMP := $(SDK_SRC_DIR)/u-boot/.patch_applied

# ==== uboot 配置 ====
.PHONY: uboot/menuconfig
uboot/menuconfig:
	@echo "==> 配置uboot..."
	cd $(SDK_SRC_DIR)/u-boot && make rk3568_defconfig && make menuconfig && \
	make savedefconfig && cp defconfig configs/rk3568_defconfig && cp configs/rk3568_defconfig $(UBOOT_DIR)/rk3568_defconfig

# ===== 编译 uboot =====
.PHONY: uboot
uboot:
	@echo "==> 编译 U-Boot..."
ifneq ("$(wildcard $(UBOOT_DIR)/patches-2017.09/*.patch)","")
ifeq ("$(wildcard $(UBOOT_PATCH_TMP))", "")
	cd $(SDK_SRC_DIR)/u-boot && git am $(abspath $(UBOOT_DIR)/patches-2017.09)/*.patch
	touch $(UBOOT_PATCH_TMP)
endif
endif
	cd $(SDK_SRC_DIR) && ./build.sh uboot

KERNEL_PATCH_TMP := $(SDK_SRC_DIR)/kernel/.patch_applied

# ===== 内核配置 =====
.PHONY: kernel/menuconfig
kernel/menuconfig:
	@echo "==> 配置内核..."
	cd $(SDK_SRC_DIR)/kernel && \
	make ARCH=arm64 rockchip_linux_defconfig && \
	make ARCH=arm64 menuconfig && \
	make ARCH=arm64 savedefconfig && \
	cp defconfig arch/arm64/configs/rockchip_linux_defconfig
#cp arch/arm64/configs/rockchip_linux_defconfig $(KERNEL_DIR)/rockchip_linux_defconfig

# ===== 编译 kernel =====
.PHONY: kernel
kernel:
	@echo "==> 编译内核..."
ifneq ("$(wildcard $(KERNEL_DIR)/patches-4.19-rk3568/*.patch)","")
ifeq ("$(wildcard $(KERNEL_PATCH_TMP))", "")
	cd $(SDK_SRC_DIR)/kernel && git am $(abspath $(KERNEL_DIR)/patches-4.19-rk3568)/*.patch
	touch $(KERNEL_PATCH_TMP)
endif
endif
	cd $(SDK_SRC_DIR) && ./build.sh kernel

BUILDROOT_PATCH_TMP := $(SDK_SRC_DIR)/buildroot/.patch_applied


# ==== rootfs 准备 =====
.PHONY: buildroot/prepare
buildroot/prepare:
	@echo "==> 准备 rootfs..."
	cd $(SDK_SRC_DIR)/buildroot && source build/envsetup.sh rockchip_rk3568

# ===== rootfs 配置 =====
.PHONY: buildroot/menuconfig
buildroot/menuconfig: buildroot/prepare
	@echo "==> 配置 rootfs..."
	cd $(SDK_SRC_DIR)/buildroot && make menuconfig && make savedefconfig && \
	cp $(SDK_SRC_DIR)/buildroot/configs/rockchip_rk3568_defconfig  $(BUILDROOT_DIR)/rockchip_rk3568_defconfig

# ===== 编译 rootfs =====
.PHONY: buildroot
buildroot: buildroot/prepare
	@echo "==> 编译 buildroot..."
ifneq ("$(wildcard $(BUILDROOT_DIR)/patches-2018.02/*.patch)","")
ifeq ("$(wildcard $(BUILDROOT_PATCH_TMP))", "")
	cd $(SDK_SRC_DIR)/buildroot && git am $(abspath $(BUILDROOT_DIR)/patches-2018.02)/*.patch
	touch $(BUILDROOT_PATCH_TMP)
endif
endif
	cp -rf $(BUILDROOT_DIR)/fs-overlay/* $(FS_OVERLAY_DIR)
	cp -rf $(BUILDROOT_DIR)/squashfs/*.patch $(SDK_SRC_DIR)/buildroot/package/squashfs/
	cd $(SDK_SRC_DIR) && ./build.sh buildroot

.PHONY: image
image:
	@echo "==> 打包镜像..."
	mkdir -p $(IMAGE_DIR)
# 	cd $(SDK_SRC_DIR) && ./build.sh firmware && ./build.sh updateimg
# 	cp -rf $(SDK_SRC_DIR)/rockdev/update.img $(IMAGE_DIR)
	cp -rf $(SDK_SRC_DIR)/device/rockchip/rk356x/parameter-buildroot-fit.txt $(IMAGE_DIR)
# 	cp -rf $(SDK_SRC_DIR)/u-boot/uboot.img $(IMAGE_DIR)
# 	cp -rf $(SDK_SRC_DIR)/kernel/boot.img $(IMAGE_DIR)
	cp -rf $(SDK_SRC_DIR)/buildroot/output/rockchip_rk3568/images/rootfs.ext2 $(IMAGE_DIR)

# ==== 全量编译 ===
# 如果想得到一个完整的update.img,第一次需要先全量编译
# .PHONY: buildall
# buildall: envset
# 	@echo "===> 全量编译..."
# 	cd $(SDK_SRC_DIR) && ./build.sh all

# ===== 清理 =====
.PHONY: clean
clean:
	@echo "==> 清理编译输出..."
	rm -rf $(SDK_SRC_DIR)

.PHONY: clean-dist
clean-dist:
	@echo "==> 清理所有..."
	rm -rf stating

