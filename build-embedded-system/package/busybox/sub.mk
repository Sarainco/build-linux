# -------------------------------------------------
# BusyBox package
# -------------------------------------------------


BUSYBOX_NAME    := busybox
BUSYBOX_VERSION := 1.36.0
BUSYBOX_DIR     := $(SOURCE)/busybox
BUSYBOX_OUTPUT  := $(OUTPUT)/busybox
BUSYBOX_CONFIG  := $(MODEL_DIR)/busybox.config
BUSYBOX_PATCH_DIR   := $(PACKAGE)/busybox/patches/$(BUSYBOX_VERSION)
BUSYBOX_PATCH_STAMP := $(BUSYBOX_OUTPUT)/.stamp_patched_$(BUSYBOX_VERSION)


BUSYBOX_MAKE = \
	ARCH=$(ARCH) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	$(MAKE) -C $(BUSYBOX_DIR) O=$(BUSYBOX_OUTPUT)

# -------------------------------------------------
# Enable package
# -------------------------------------------------
ifeq ($(CONFIG_PACKAGE_BUSYBOX),y)

PACKAGE_TARGETS += busybox

# =================================================
# 1. Extracting
# =================================================
busybox/extract:
	@echo ">>> Extract busybox"
	@mkdir -p $(SOURCE)
	@[ -d $(BUSYBOX_DIR) ] || tar xf $(CONFIG_DL_DIR)/busybox-$(BUSYBOX_VERSION).tar.bz2 -C $(SOURCE)
	@ln -snf busybox-$(BUSYBOX_VERSION) $(BUSYBOX_DIR)

# =================================================
# 2. Patching
# =================================================
busybox/patch: busybox/extract
	@echo ">>> Patch busybox"
	@$(call apply_patches,$(BUSYBOX_PATCH_DIR),$(BUSYBOX_DIR),$(BUSYBOX_PATCH_STAMP))

# =================================================
# 3. Configuring
# =================================================
busybox/configure:
	@echo ">>> Configure busybox"
	@mkdir -p $(BUSYBOX_OUTPUT)
	@cp $(BUSYBOX_CONFIG) $(BUSYBOX_OUTPUT)/.config
	@$(BUSYBOX_MAKE) olddefconfig

busybox/menuconfig:
	@mkdir -p $(BUSYBOX_OUTPUT)
# 	@cp $(BUSYBOX_CONFIG) $(BUSYBOX_OUTPUT)/.config
	@$(BUSYBOX_MAKE) menuconfig
	@cp $(BUSYBOX_OUTPUT)/.config $(BUSYBOX_CONFIG)

# =================================================
# 4. Building
# =================================================
busybox/build:
	@echo ">>> Build busybox"
	+@$(BUSYBOX_MAKE) -j$(NPROC)

# =================================================
# 5. Installing
# =================================================
busybox/install:
	@echo ">>> Install busybox"
	@$(BUSYBOX_MAKE) install CONFIG_PREFIX=$(ROOTFS)

# =================================================
# Meta targets
# =================================================
busybox: busybox/install

busybox/clean:
	@rm -rf $(BUSYBOX_OUTPUT)

busybox/distclean:
	@rm -rf $(BUSYBOX_OUTPUT) $(BUSYBOX_DIR)

endif
