#
ifeq ($(CONFIG_MTK_ECCCI_DRIVER), y)

ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/port
ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/$(MTK_PLATFORM)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/scp/$(MTK_PLATFORM)

# fsm objects
obj-$(CONFIG_MTK_ECCCI_DRIVER) := ccci_fsm_all.o
ccci_fsm_all-y += ccci_fsm.o
ccci_fsm_all-y += ccci_fsm_ioctl.o
ccci_fsm_all-y += ccci_fsm_monitor.o
ccci_fsm_all-y += ccci_fsm_poller.o
ccci_fsm_all-y += ccci_fsm_scp.o
ccci_fsm_all-y += ccci_fsm_ee.o
ccci_fsm_all-y += ccci_fsm_sys.o
ccci_fsm_all-y += mdee_dumper_v1.o
ccci_fsm_all-y += mdee_dumper_v2.o
ccci_fsm_all-y += mdee_dumper_v3.o
ccci_fsm_all-y += mdee_dumper_v5.o

else
obj-  := dummy.o # avoid build error
endif




