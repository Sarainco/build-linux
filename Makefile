TOPDIR := $(CURDIR)
SRC_DIR := $(TOPDIR)/staging/source
BUILD_DIR := $(TOPDIR)/staging/build
DL_DIR := $(TOPDIR)/../rk3568_dl
ROOTFS := $(TOPDIR)/staging/rootfs
IMAGE := $(TOPDIR)/staging/image

ARCH := arm64
#CROSS_COMPILE := $(HOME)/toolchain/toolchain-rk3568/bin/aarch64-buildroot-linux-gnu-
CROSS_COMPILE := $(HOME)/toolchain/toolchain-rk3562/bin/aarch64-none-linux-gnu-

IMAGE_FILE_ROOTFS := rootfs.ext4
IMAGE_SIZE_ROOTFS := 512M

all: busybox;

KERNEL := linux-5.10.239
KERNEL_SRC := $(SRC_DIR)/$(KERNEL)
KERNEL_BUILD := $(BUILD_DIR)/$(KERNEL)
KERNEL_ENV := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) -C $(KERNEL_SRC)

kernel/prepare:
	mkdir -p $(SRC_DIR) && mkdir -p $(KERNEL_BUILD)
	tar xf $(DL_DIR)/$(KERNEL)* -C $(SRC_DIR) --overwrite

kernel/menuconfig:
	$(KERNEL_ENV) defconfig
	$(KERNEL_ENV) menuconfig

kernel/compile:
	$(KERNEL_ENV) all -j$(nproc)

kernel/clean:
	$(KERNEL_ENV) distclean

BUSYBOX := busybox-1.36.0
BUSYBOX_SRC := $(SRC_DIR)/$(BUSYBOX)
BUSYBOX_BUILD := $(BUILD_DIR)/$(BUSYBOX)
BUSYBOX_CONFIG := $(TOPDIR)/configs/busybox.config
BUSYBOX_ENV := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) -C $(BUSYBOX_SRC) O=$(BUSYBOX_BUILD)


busybox/prepare:
	mkdir -p $(BUSYBOX_BUILD)

busybox/menuconfig: busybox/prepare
	-cp $(BUSYBOX_CONFIG) $(BUSYBOX_BUILD)/.config
	$(BUSYBOX_ENV) menuconfig
	cp $(BUSYBOX_BUILD)/.config $(BUSYBOX_CONFIG)

busybox/compile: busybox/prepare
	cp $(BUSYBOX_CONFIG) $(BUSYBOX_BUILD)/.config
	+$(BUSYBOX_ENV) all

busybox: busybox/compile
	$(BUSYBOX_ENV) install CONFIG_PREFIX=$(ROOTFS)

busybox/clean:
	rm -fr $(BUSYBOX_BUILD)


OPEN_SSL := openssl-3.0.17
OPEN_SSL_SRC := $(SRC_DIR)/$(OPEN_SSL)
OPEN_SSL_BUILD := $(BUILD_DIR)/$(OPEN_SSL)

open_ssl/prepare:
	mkdir -p $(OPEN_SSL_BUILD)

open_ssl/compile: open_ssl/prepare
	cd $(OPEN_SSL_SRC) && \
	./Configure linux-aarch64 \
		--prefix=/usr \
		--openssldir=/etc/ssl \
		shared no-tests \
		--cross-compile-prefix=$(CROSS_COMPILE) && \
	make -j$(nproc)

open_ssl/install:
	cd $(OPEN_SSL_SRC) && \
	make install DESTDIR=$(OPEN_SSL_BUILD)

UTIL_LINUX := util-linux-2.35.2
UTIL_LINUX_SRC := $(SRC_DIR)/$(UTIL_LINUX)
UTIL_LINUX_BUILD := $(BUILD_DIR)/$(UTIL_LINUX)

util_linux/prepare:
	tar xf $(DL_DIR)/$(UTIL_LINUX)* -C $(SRC_DIR) --overwrite
	mkdir -p $(UTIL_LINUX_BUILD)

util_linux:
	cd $(UTIL_LINUX_SRC) \
	&& ./configure --prefix=/opt/util-linux \
		--without-tinfo \
		--host=aarch64-none-linux-gnu \
		CC=$(CROSS_COMPILE)gcc \
	&& make \
	&& sudo make install DESTDIR=$(UTIL_LINUX_BUILD)


NTFS3G := ntfs-3g_ntfsprogs-2022.10.3
NTFS3G_SRC := $(SRC_DIR)/$(NTFS3G)
NTFS3G_BUILD := $(BUILD_DIR)/$(NTFS3G)

ntfs3g/prepare:
	mkdir -p $(SRC_DIR)
	tar xf $(DL_DIR)/$(NTFS3G)* -C $(SRC_DIR) --overwrite
	mkdir -p $(NTFS3G_BUILD)

ntfs3g: ntfs3g/prepare
	cd $(NTFS3G_SRC) && \
	./configure --prefix=/opt/ntfs-3g \
		--disable-static     \
		--with-fuse=internal \
		--host=aarch64-none-linux-gnu \
		CC=$(CROSS_COMPILE)gcc \
	&& make \
	&& sudo make install DESTDIR=$(NTFS3G_BUILD)

FUSE := fuse-2.9.9
FUSE_SRC := $(SRC_DIR)/$(FUSE)
FUSE_BUILD := $(BUILD_DIR)/$(FUSE)

fuse/prepare:
	tar xf $(DL_DIR)/$(FUSE)* -C $(SRC_DIR) --overwrite
	mkdir -p $(FUSE_BUILD)

fuse:
	cd $(FUSE_SRC) && \
	./configure \
		--host=aarch64-none-linux-gnu \
		--prefix=/opt/fuse \
		CC=$(CROSS_COMPILE)gcc \
	&& make \
	&& sudo make install DESTDIR=$(FUSE_BUILD)

EXFAT := fuse-exfat-1.4.0
EXFAT_SRC := $(SRC_DIR)/$(EXFAT)
EXFAT_BUILD := $(BUILD_DIR)/$(EXFAT)

exfat/prepare:
	tar xf $(DL_DIR)/$(EXFAT)* -C $(SRC_DIR) --overwrite
	mkdir -p $(EXFAT_BUILD)

exfat:
	cd $(EXFAT_SRC) && \
	autoreconf --install && \
	./configure \
		--host=aarch64-none-linux-gnu \
		--prefix=/opt/exfat \
		CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ \
		FUSE2_CFLAGS="-I/home/sunao/workspace/build-linux/staging/build/fuse-2.9.9/opt/fuse/include -L/home/sunao/workspace/build-linux/staging/build/fuse-2.9.9/opt/fuse/lib -lfuse -D_FILE_OFFSET_BITS=64" \
		FUSE2_LIBS=/home/sunao/workspace/build-linux/staging/build/fuse-2.9.9/opt/fuse/lib/libfuse.a \
	&& make \
	&& sudo make install DESTDIR=$(EXFAT_BUILD)

E2FS := e2fsprogs-1.47.0
E2FS_SRC := $(SRC_DIR)/$(E2FS)
E2FS_BUILD := $(BUILD_DIR)/$(E2FS)

e2fs/prepare:
	tar xf $(DL_DIR)/$(E2FS)* -C $(SRC_DIR) --overwrite
	mkdir -p $(E2FS_BUILD)

e2fs:
	cd $(E2FS_SRC) && \
	./configure --host=aarch64-none-linux-gnu --prefix=/opt/e2fsprogs CC=$(CROSS_COMPILE)gcc LDFLAGS=-static \
	&& make \
	&& make install DESTDIR=$(E2FS_BUILD)


V4L := v4l-utils-1.30.0
V4L_SRC := $(SRC_DIR)/$(V4L)
V4L_BUILD := $(BUILD_DIR)/$(V4L)
V4L_CROSS_FILE := package/v4l/cross-rk3568.txt

v4l/prepare:
	mkdir -p $(V4L_BUILD)
	cp $(V4L_CROSS_FILE) $(V4L_SRC)

v4l/compile: v4l/prepare
	cd $(V4L_SRC) && \
	meson setup build --cross-file cross-rk3568.txt \
		-Dv4l-utils=true \
		-Dgconv=disabled \
		-Dlibdvbv5=disabled \
		-Dqv4l2=disabled \
		-Dqvidcap=disabled \
		-Dv4l2-compliance-32=false \
		-Dv4l2-compliance-32-time64=false \
		-Dv4l2-ctl-32=false \
		-Dv4l2-ctl-32-time64=false \
		-Dv4l2-compliance-libv4l=false && \
	meson compile -C build

v4l/install: v4l/compile
	cd $(V4L_SRC) && \
	meson install -C build --destdir $(V4L_BUILD)


GIT_LOG_VERSION := $(shell git log -1 --format="%h")
BUILD_TIME := $(shell date +"%y%m%d%H%M")

version:
	echo $(GIT_LOG_VERSION) $(BUILD_TIME) > $(ROOTFS)/etc/version

rootfs:
	@echo "Preparing rootfs..."
	mkdir -p \
	$(ROOTFS)/proc \
	$(ROOTFS)/sys \
	$(ROOTFS)/dev/pts \
	$(ROOTFS)/dev/shm \
	$(ROOTFS)/tmp \
	$(ROOTFS)/run \
	$(ROOTFS)/mnt \
	$(ROOTFS)/userdata \
	$(ROOTFS)/bin
	cp -av fs-overlay/* $(ROOTFS)/
	cp -av $(V4L_BUILD)/* $(ROOTFS)/
	cp -av $(OPEN_SSL_BUILD)/* $(ROOTFS)/

#@echo "Copying busybox shared libs..."
#ldd $(ROOTFS_DIR)/bin/busybox | awk '{print $$3}' | xargs -I{} cp -uv {} $(ROOTFS_DIR)/lib/


image/ext4fs: rootfs
	@echo "Creating ext4 image..."
	mkdir -p $(IMAGE)
	rm -f $(IMAGE)/$(IMAGE_FILE_ROOTFS)
	fakeroot mkfs.ext4 -q -d $(ROOTFS) $(IMAGE)/$(IMAGE_FILE_ROOTFS) $(IMAGE_SIZE_ROOTFS)



clean:
	rm -fr $(BUILD_DIR) $(ROOTFS) $(IMAGE)

clean-dist:
	rm -fr $(TOPDIR)/staging

prepare-src:
	mkdir -p $(SRC_DIR)
	tar xf $(DL_DIR)/$(BUSYBOX)* -C $(SRC_DIR) --overwrite
	tar xf $(DL_DIR)/$(V4L)* -C $(SRC_DIR) --overwrite
	tar xf $(DL_DIR)/$(OPEN_SSL)* -C $(SRC_DIR) --overwrite


.PHONY: all busybox v4l image
