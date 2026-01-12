$(info CONFIG_PACKAGE_BUSYBOX=$(CROSS_COMPILE))

BUSYBOX_VER := busybox-1.36.0
BUSYBOX_CONFIG := $(MODEL_DIR)/busybox.config
BUSYBOX_ENV := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) -C $(SOURCE)/busybox O=$(OUTPUT)/busybox


ifeq ($(CONFIG_PACKAGE_BUSYBOX),y)

PACKAGE_TARGETS += busybox

busybox/prepare:
	

busybox/menuconfig: busybox/prepare
	-cp $(BUSYBOX_CONFIG) $(SOURCE)/busybox/.config
	$(BUSYBOX_ENV) menuconfig
	cp $(SOURCE)/busybox/.config $(BUSYBOX_CONFIG)

busybox/compile: busybox/prepare
	cp $(BUSYBOX_CONFIG) $(SOURCE)/busybox/.config
	+$(BUSYBOX_ENV) all

busybox/install: busybox/compile
	$(BUSYBOX_ENV) install CONFIG_PREFIX=$(ROOTFS)

busybox/clean:
	rm -fr $(OUTPUT)/busybox


busybox: busybox/install
	echo ">>> build busybox"

endif





