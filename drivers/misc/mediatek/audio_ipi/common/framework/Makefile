#
ccflags-y += \
	-I$(srctree)/drivers/misc/mediatek/base/power/include \
	-I$(srctree)/drivers/misc/mediatek/scp/$(MTK_PLATFORM) \
	-I$(srctree)/drivers/misc/mediatek/adsp/common \
	-I$(srctree)/drivers/misc/mediatek/adsp/$(MTK_PLATFORM) \
	-I$(srctree)/drivers/misc/mediatek/audio_ipi/$(CONFIG_MTK_PLATFORM) \
	-I$(srctree)/drivers/misc/mediatek/audio_ipi/common/utility

### DO
ifeq ($(CONFIG_MTK_DO),y)
ccflags-y += -I$(srctree)/drivers/staging/dyn_obj/$(MTK_PLATFORM)
endif

### call
ifeq ($(CONFIG_MTK_AURISYS_PHONE_CALL_SUPPORT),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common/tasks/call
endif

### mp3
ifeq ($(CONFIG_MTK_AUDIO_TUNNELING_SUPPORT),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common/tasks/mp3
endif


### spkprotect for headfile include path
ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common/tasks/spkprotect
ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common/tasks/audplayback

### c files
obj-y += audio_ringbuf.o
obj-y += audio_ipi_dma.o
obj-y += audio_ipi_driver.o
obj-y += audio_ipi_queue.o
obj-y += audio_messenger_ipi.o
obj-y += audio_task_manager.o
obj-y += adsp_ipi_queue.o

ifeq ($(CONFIG_MT_ENG_BUILD),y)
CFLAGS_audio_ringbuf.o += -DDEBUG
CFLAGS_audio_ipi_dma.o += -DDEBUG
CFLAGS_audio_ipi_driver.o += -DDEBUG
CFLAGS_audio_ipi_queue.o += -DDEBUG
CFLAGS_audio_messenger_ipi.o += -DDEBUG
CFLAGS_audio_task_manager.o += -DDEBUG
CFLAGS_adsp_ipi_queue.o += -DDEBUG
endif



