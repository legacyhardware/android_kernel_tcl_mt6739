#
ccflags-$(CONFIG_USB_MTK_HDRC) += -I$(srctree)/drivers/misc/mediatek/usb20

# for battery related
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat

# for SPM control usage
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/

# for TYPEC connection management
ccflags-y += -I$(srctree)/drivers/misc/mediatek/usb_c/inc
ifeq ($(CONFIG_TCPC_CLASS),y)
	ccflags-y += -I$(srctree)/drivers/misc/mediatek/usb_c/tcpc/inc
endif
# for ep0 test
ccflags-y += -I$(srctree)/drivers/usb/core/

obj-$(CONFIG_USB_MTK_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_MTK_HDRC)  += usb20_host.o
obj-$(CONFIG_USBIF_COMPLIANCE)   += usb20_otg_if.o
obj-$(CONFIG_DEBUG_FS)	+= usb20_phy_debugfs.o
