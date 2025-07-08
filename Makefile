TOPDIR := $(CURDIR)
SRC_DIR := $(TOPDIR)/staging/source
BUILD_DIR := $(TOPDIR)/staging/build
DL_DIR := $(TOPDIR)/../rk3568_dl
ROOTFS := $(TOPDIR)/staging/rootfs
IMAGE := $(TOPDIR)/staging/image

ARCH := arm64
CROSS_COMPILE := $(HOME)/toolchain/toolchain-rk3568/bin/aarch64-buildroot-linux-gnu-

IMAGE_FILE_ROOTFS := rootfs.ext4
IMAGE_SIZE_ROOTFS := 128M

all: busybox;

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
	$(ROOTFS)/userdata
	cp -av files/* $(ROOTFS)/

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


.PHONY: all busybox
