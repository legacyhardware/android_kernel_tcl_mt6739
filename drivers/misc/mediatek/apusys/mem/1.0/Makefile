#
ccflags-y += -I$(obj)/../../mdla/2.0
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/mdla/$(MTK_PLATFORM)
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/mdla/1.0
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/mt6779
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include
ccflags-y += -I$(srctree)/drivers/devfreq
ccflags-y += -I$(srctree)/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/vpu/$(MTK_PLATFORM)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/qos/mt6779
ccflags-y += -I$(srctree)/drivers/misc/mediatek/power/1.0


#obj-$(CONFIG_MTK_MDLA_SUPPORT) := mdla_main.o gsm.o mdla_dvfs.o
#obj-$(CONFIG_MTK_MDLA_DEBUG) += mdla_debug.o mdla_trace.o mdla_decoder.o
#obj-$(CONFIG_MTK_MDLA_ION) += mdla_ion.o

obj-y += apusys_mem.o

apusys_mem-objs:= gsm.o mdla_ion.o

