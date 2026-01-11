#module的记忆机制
MODEL := $(if $(M),$(M)$(file >.model,$(M)),$(file <.model))

#stating_dir的记忆机制+优先级
STAGING_DIR := $(or $(if $(O),$(O)$(file >.staging,$(O)),$(file <.staging)),$(TOP)/staging)

#删除变量 O 和 M; 防止子Makefile再次改module
override undefine O
override undefine M

# MAKEOVERRIDES记录的是传给子make的命令行变量
# 从 MAKEOVERRIDES 里 删除 M=xxx
MAKEOVERRIDES:=$(patsubst M=%,,$(MAKEOVERRIDES))

HOST := $(TOP)/host
TOOLCHAIN := $(TOP)/toolchain
TARGET := $(TOP)/target
MODEL_DIR := $(TARGET)/$(MODEL)
MODEL_CONFIG := $(MODEL_DIR)/model.config
OUTPUT := $(STAGING_DIR)/$(MODEL)
HOST_OUTPUT := $(STAGING_DIR)/host

ifeq ($(wildcard $(MODEL_CONFIG)),)
$(error "Invalid model, usage: make M=<model>")
endif



include $(HOST)/sub.mk
