# -------------------------------------------------
# CUPS package
# -------------------------------------------------

CUPS_NAME    := cups
CUPS_VERSION := 2.4.11
CUPS_DIR     := $(SOURCE)/cups
CUPS_OUTPUT  := $(OUTPUT)/cups
CUPS_PATCH_DIR   := $(PACKAGE)/cups/patches/$(CUPS_VERSION)
CUPS_PATCH_STAMP := $(CUPS_OUTPUT)/.stamp_patched_$(CUPS_VERSION)

CUPS_TARBALL := cups-$(CUPS_VERSION)-source.tar.gz
CUPS_SITE    := https://github.com/OpenPrinting/cups/releases/download/v$(CUPS_VERSION)

# -------------------------------------------------
# Tool / env
# -------------------------------------------------
CUPS_ENV = \
	CC=$(CROSS_COMPILE)gcc \
	CXX=$(CROSS_COMPILE)g++ \
	AR=$(CROSS_COMPILE)ar \
	RANLIB=$(CROSS_COMPILE)ranlib
# 	PKG_CONFIG=$(HOST_DIR)/bin/pkg-config

CUPS_CONF_OPTS = \
	--host=aarch64-none-linux-gnu \
	--prefix=/usr \
	--libdir=/usr/lib \
	--with-docdir=/usr/share/cups/doc-root \
	--with-cups-user=lp \
	--with-cups-group=lp \
	--with-system-groups="lpadmin sys root" \
	--disable-gssapi \
	--disable-pam \
	--disable-libpaper \
	--without-rcdir

ifeq ($(CONFIG_PACKAGE_DBUS),y)
CUPS_CONF_OPTS += --enable-dbus
else
CUPS_CONF_OPTS += --disable-dbus
endif

ifeq ($(CONFIG_PACKAGE_OPENSSL),y)
CUPS_CONF_OPTS += --with-tls=openssl
else
CUPS_CONF_OPTS += --with-tls=no
endif

ifeq ($(CONFIG_PACKAGE_LIBUSB),y)
CUPS_CONF_OPTS += --enable-libusb
else
CUPS_CONF_OPTS += --disable-libusb
endif

# -------------------------------------------------
# Enable package
# -------------------------------------------------
ifeq ($(CONFIG_PACKAGE_CUPS),y)

PACKAGE_TARGETS += cups

# =================================================
# 1. Extracting
# =================================================
cups/extract:
	@echo ">>> Extract cups"
	@mkdir -p $(SOURCE)
	@[ -d $(CUPS_DIR) ] || tar xf $(CONFIG_DL_DIR)/$(CUPS_TARBALL) -C $(SOURCE)

# =================================================
# 2. Patching
# =================================================
cups/patch: cups/extract
	@echo ">>> Patch cups"
	@$(call apply_patches,$(CUPS_PATCH_DIR),$(CUPS_DIR),$(CUPS_PATCH_STAMP))

# =================================================
# 3. Configuring
# =================================================
cups/configure: cups/patch
	@echo ">>> Configure cups"
	@mkdir -p $(CUPS_OUTPUT)
	@cd $(CUPS_DIR) && \
		autoconf -f
	@cd $(CUPS_OUTPUT) && \
		$(CUPS_ENV) \
		$(CUPS_DIR)/configure $(CUPS_CONF_OPTS)

# =================================================
# 4. Building
# =================================================
cups/build: cups/configure
	@echo ">>> Build cups"
	+@$(MAKE) -C $(CUPS_OUTPUT) -j$(NPROC)

# =================================================
# 5. Installing
# =================================================
cups/install: cups/build
	@echo ">>> Install cups"
	@$(MAKE) -C $(CUPS_OUTPUT) DESTDIR=$(ROOTFS) install
	@install -D -m 0755 $(PACKAGE)/cups/S81cupsd \
		$(ROOTFS)/etc/init.d/S81cupsd

ifeq ($(CONFIG_PACKAGE_HAS_UDEV),y)
	@install -D -m 0644 $(PACKAGE)/cups/70-usb-printers.rules \
		$(ROOTFS)/lib/udev/rules.d/70-usb-printers.rules
endif

# =================================================
# Meta targets
# =================================================
cups: cups/install

cups/clean:
	@rm -rf $(CUPS_OUTPUT)

cups/distclean:
	@rm -rf $(CUPS_OUTPUT) $(CUPS_DIR)

endif
