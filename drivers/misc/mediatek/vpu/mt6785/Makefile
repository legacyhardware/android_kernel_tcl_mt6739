#
ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/smi/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/vpu/3.0
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include
ccflags-y += -I$(srctree)/drivers/devfreq
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mdla/1.0

# EMI MPU
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/submodule
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/$(CONFIG_MTK_PLATFORM)

ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/qos/$(MTK_PLATFORM)/
ifeq ($(CONFIG_MTK_IOMMU_V2),y)
ccflags-y += -I$(srctree)/drivers/iommu
endif

obj-$(CONFIG_MTK_VPU_SUPPORT) += vpu_hw.o vpu_reg.o apu_dvfs.o apu_dvfs_sysfs.o
obj-$(CONFIG_MTK_VPU_SUPPORT) += vpu_profile.o vpu_dump.o

ifeq ($(CONFIG_MTK_QOS_V2),y)
obj-$(CONFIG_MTK_VPU_SUPPORT) += vpu_qos.o
endif
