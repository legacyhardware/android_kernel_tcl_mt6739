#
ifneq ($(KERNELRELEASE),)

    ccflags-$(CONFIG_MTK_ECCCI_DRIVER) += -I$(srctree)/drivers/misc/mediatek/eccci/$(MTK_PLATFORM)
    ccflags-$(CONFIG_MTK_ECCCI_DRIVER) += -I$(srctree)/drivers/misc/mediatek/eccci/
    ccflags-$(CONFIG_MTK_CCCI_EXT) += -I$(srctree)/drivers/misc/mediatek/dual_ccci/

    ccflags-y += -I$(src)/include/

    obj-y += conn_md_bridge.o
    conn_md_bridge-objs := conn_md.o
    conn_md_bridge-objs += conn_md_exp.o
    conn_md_bridge-objs += conn_md_log.o
    conn_md_bridge-objs += conn_md_dump.o
    conn_md_bridge-objs += conn_md_dbg.o
    conn_md_bridge-objs += conn_md_test.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)

default:
	$(MAKE) -C

endif
