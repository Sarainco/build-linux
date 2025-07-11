# 项目 Makefile：支持 RK3568 平台 SDK 的初始化与构建

# ===== 配置变量 =====
SDK_TAR      := atompi-ca1_buildroot_release_v1.0_20240129.tgz
SDK_SRC_DIR  := stating/source/rk3568_linux_sdk
SDK_BUILD_DIR:= stating/build
BOARD_CONFIG := BoardConfig-rk3568-atk-atompi-ca1.mk

# ===== 默认目标 =====
.PHONY: all
all: uboot kernel buildroot

# ===== SDK 初始化 =====
.PHONY: sdk-init
sdk-init:
	@echo "==> 解压 SDK..."
	mkdir -p $(SDK_SRC_DIR)
	tar xvf $(SDK_TAR) -C $(SDK_SRC_DIR)
	cd $(SDK_SRC_DIR) && .repo/repo/repo sync -l -j8

# ===== 环境配置 =====
.PHONY: env
env:
	@echo "==> 设置开发板配置环境变量..."
	cd $(SDK_SRC_DIR) && ./build.sh $(BOARD_CONFIG)

# ===== 编译 uboot =====
.PHONY: uboot
uboot: env
	@echo "==> 编译 U-Boot..."
	cd $(SDK_SRC_DIR) && ./build.sh uboot

# ===== 内核配置 =====
.PHONY: kernel/menuconfig
kernel/menuconfig:
	@echo "==> 配置内核..."
	cd $(SDK_SRC_DIR)/kernel && \
	make ARCH=arm64 rockchip_linux_defconfig && \
	make ARCH=arm64 menuconfig && \
	make ARCH=arm64 savedefconfig && \
	cp defconfig arch/arm64/configs/rockchip_linux_defconfig

# ===== 编译 kernel =====
.PHONY: kernel
kernel: env
	@echo "==> 编译内核..."
	cd $(SDK_SRC_DIR) && ./build.sh kernel

# ===== rootfs 配置 =====
.PHONY: buildroot/menuconfig
buildroot/menuconfig:
	@echo "==> 配置 rootfs..."
	cd $(SDK_SRC_DIR)/buildroot && source build/envsetup.sh rockchip_rk3568 && make menuconfig && make savedefconfig

# ===== 编译 rootfs =====
.PHONY: buildroot
buildroot: env
	@echo "==> 编译 buildroot..."
	cd $(SDK_SRC_DIR) && ./build.sh buildroot

# ===== 清理 =====
.PHONY: clean
clean:
	@echo "==> 清理编译输出..."
	rm -rf $(SDK_BUILD_DIR)

