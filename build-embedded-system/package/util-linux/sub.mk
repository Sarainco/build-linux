# -------------------------------------------------
# Util-linux package
# -------------------------------------------------

$(info CONFIG_PACKAGE_UTIL_LINUX=$(CONFIG_PACKAGE_UTIL_LINUX))

UTIL_LINUX_NAME    := util-linux
UTIL_LINUX_VERSION := 2.35.2
UTIL_LINUX_DIR     := $(SOURCE)/$(UTIL_LINUX_NAME)-$(UTIL_LINUX_VERSION)
UTIL_LINUX_OUTPUT  := $(OUTPUT)/$(UTIL_LINUX_NAME)-$(UTIL_LINUX_VERSION)
UTIL_LINUX_PATCH_DIR   := $(PACKAGE)/util-linux/patches/$(UTIL_LINUX_VERSION)
UTIL_LINUX_PATCH_STAMP := $(UTIL_LINUX_OUTPUT)/.stamp_patched_$(UTIL_LINUX_VERSION)


# -------------------------------------------------
# Enable package
# -------------------------------------------------
ifeq ($(CONFIG_PACKAGE_UTIL_LINUX),y)

PACKAGE_TARGETS += util-linux

# =================================================
# 1. Extracting
# =================================================
util-linux/extract:
	tar xf $(CONFIG_DL_DIR)/$(UTIL_LINUX_NAME)-$(UTIL_LINUX_VERSION).tar.xz -C $(SOURCE)

# =================================================
# 2. Patching
# =================================================


# =================================================
# 3. Configuring
# =================================================


# =================================================
# 4. Building
# =================================================
util-linux/build:
	mkdir -p $(UTIL_LINUX_OUTPUT)
	cd $(UTIL_LINUX_DIR) \
	&& ./configure --prefix=/usr \
		--without-tinfo \
		--host=aarch64-none-linux-gnu \
		CC=$(CC) \
	&& make

# =================================================
# 5. Installing
# =================================================
util-linux/install:
	cd $(UTIL_LINUX_DIR) \
	&& make install DESTDIR=$(UTIL_LINUX_OUTPUT)

# =================================================
# Meta targets
# =================================================


endif
