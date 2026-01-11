# sunao
# 2026/01/11

MAJOR_VERSION := 0
MINOR_VERSION := 1

TOP := $(CURDIR)

all: host

include common.mk

menuconfig: host/kconfig
	$(HOST_OUTPUT)/kconfig/mconf Config.in
