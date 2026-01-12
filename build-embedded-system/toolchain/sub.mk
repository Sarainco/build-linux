# -------------------------------------------------
# Toolchain
# -------------------------------------------------

ifeq ($(CONFIG_TOOLCHAIN),y)

TOOLCHAIN_DIR        := $(CONFIG_TOOLCHAIN_DIR)/$(CONFIG_TOOLCHAIN_NAME)
TOOLCHAIN_BIN_DIR    := $(TOOLCHAIN_DIR)/bin
TOOLCHAIN_SYSROOT    := $(TOOLCHAIN_DIR)/$(CONFIG_TOOLCHAIN_TARGET)/sysroot

CROSS_COMPILE ?= $(TOOLCHAIN_BIN_DIR)/$(CONFIG_TOOLCHAIN_TARGET)-

endif

# -------------------------------------------------
# Compiler tools
# -------------------------------------------------

CC        ?= $(CROSS_COMPILE)gcc
CXX       ?= $(CROSS_COMPILE)g++
AS        ?= $(CROSS_COMPILE)as
LD        ?= $(CROSS_COMPILE)ld
AR        ?= $(CROSS_COMPILE)ar
NM        ?= $(CROSS_COMPILE)nm
STRIP     ?= $(CROSS_COMPILE)strip
OBJCOPY  ?= $(CROSS_COMPILE)objcopy
OBJDUMP  ?= $(CROSS_COMPILE)objdump
READELF  ?= $(CROSS_COMPILE)readelf
RANLIB   ?= $(CROSS_COMPILE)ranlib
SIZE     ?= $(CROSS_COMPILE)size
STRINGS  ?= $(CROSS_COMPILE)strings
ADDR2LINE?= $(CROSS_COMPILE)addr2line

