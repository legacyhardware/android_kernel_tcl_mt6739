#
ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/
ccflags-y += -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/ion/mtk
ccflags-y += -I$(srctree)/drivers/misc/mediatek/smi/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmdvfs/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/timer/timesync/
ccflags-y += -I$(srctree)/include/dt-bindings/memory/
ccflags-y += -I$(srctree)/drivers/iommu/
ccflags-${CONFIG_MACH_MT6833} += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/mt6833/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/clk/mediatek/

obj-y += camera_isp.o
obj-y += cam_qos.o