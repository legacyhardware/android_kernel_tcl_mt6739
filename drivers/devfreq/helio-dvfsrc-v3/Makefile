#
ccflags-y += -Werror -I$(srctree)/drivers/devfreq
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/spm/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/dramc/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/watchdog/mediatek/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/dramc/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sspm/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sspm/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/submodule/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/smi/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/$(CONFIG_MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmdvfs/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/qos/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/dbgtop/
ccflags-y += -I$(srctree)/drivers/watchdog/mediatek/include/
ccflags-y += -I$(srctree)/include/memory/mediatek/

ifneq (,$(filter $(CONFIG_MACH_MT6885), y))
ccflags-y += -DDVFSRC_SMC_CONTROL
endif
ifneq (,$(filter $(CONFIG_MACH_MT6873), y))
ccflags-y += -DDVFSRC_SMC_CONTROL
ccflags-y += -DDVFSRC_SUSPEND_SUPPORT
endif
ifneq (,$(filter $(CONFIG_MACH_MT6853), y))
ccflags-y += -DDVFSRC_SMC_CONTROL
ccflags-y += -DDVFSRC_SUSPEND_SUPPORT
endif
ifneq (,$(filter $(CONFIG_MACH_MT6893), y))
ccflags-y += -DDVFSRC_SMC_CONTROL
ccflags-y += -DDVFSRC_SUSPEND_SUPPORT
endif
ifneq (,$(filter $(CONFIG_MACH_MT6833), y))
ccflags-y += -DDVFSRC_SMC_CONTROL
ccflags-y += -DDVFSRC_SUSPEND_SUPPORT
endif

obj-y += helio-dvfsrc-qos.o helio-dvfsrc-sysfs.o helio-dvfsrc-opp.o
obj-$(CONFIG_MACH_MT6785) += helio-dvfsrc-opp-mt6785.o helio-dvfsrc-mt6785.o helio-dvfsrc-ip-v2.o
obj-$(CONFIG_MACH_MT6885) += helio-dvfsrc-opp-mt6885.o helio-dvfsrc-mt6885.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
obj-$(CONFIG_MACH_MT6873) += helio-dvfsrc-opp-mt6873.o helio-dvfsrc-mt6873.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
ifeq ($(CONFIG_MTK_DVFSRC_MT6833_PRETEST),y)
obj-$(CONFIG_MTK_DVFSRC_MT6833_PRETEST) += helio-dvfsrc-opp-mt6833.o helio-dvfsrc-mt6833.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
else
obj-$(CONFIG_MACH_MT6853) += helio-dvfsrc-opp-mt6853.o helio-dvfsrc-mt6853.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
endif
obj-$(CONFIG_MACH_MT6893) += helio-dvfsrc-opp-mt6893.o helio-dvfsrc-mt6893.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
obj-$(CONFIG_MACH_MT6833) += helio-dvfsrc-opp-mt6833.o helio-dvfsrc-mt6833.o helio-dvfsrc-ip-v2.o helio-dvfsrc-smc-control.o
