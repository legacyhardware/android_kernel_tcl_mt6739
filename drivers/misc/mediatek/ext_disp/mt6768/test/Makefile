#
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ext_disp/  \
             -I$(srctree)/drivers/misc/mediatek/ext_disp/$(MTK_PLATFORM)/test/ \
             -I$(srctree)/drivers/misc/mediatek/video/   \
             -I$(srctree)/drivers/misc/mediatek/video/common/rdma10/ \
             -I$(srctree)/drivers/misc/mediatek/video/include/ \
             -I$(srctree)/drivers/misc/mediatek/video/common/ \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/ \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/ \
             -I$(srctree)/drivers/misc/mediatek/sync/ \
             -I$(srctree)/drivers/misc/mediatek/hdmi/$(CONFIG_CUSTOM_KERNEL_HDMI)/ \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v3/ \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v3/$(MTK_PLATFORM)/ \
             -I$(srctree)/drivers/misc/mediatek/hdmi/inc/ \
             -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/misc/mediatek/lcm/inc \
	     -I$(srctree)/drivers/misc/mediatek/emi_bwl/mt6799/

subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/
ccflags-y += -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/ion/mtk

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI), "MT8193")
ccflags-y +=	-DHDMI_MT8193_SUPPORT
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/mt8193/
endif

ccflags-y += -I$(srctree)/drivers/misc/mediatek/ext_disp/$(MTK_PLATFORM)/

obj-y +=  \
    dpi_dvt_test.o \
    dpi_dvt_platform.o \
    ddp_dpi_ext.o
