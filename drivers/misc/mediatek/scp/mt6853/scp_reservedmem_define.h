
#ifndef __SCP_RESERVEDMEM_DEFINE_H__
#define __SCP_RESERVEDMEM_DEFINE_H__

static struct scp_reserve_mblock scp_reserve_mblock[] = {
#ifdef CONFIG_MTK_VOW_SUPPORT
	{
		.num = VOW_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x4A700,  /* 297KB (2 model size)*/
	},
#endif
	{
		.num = SENS_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x100000,  /* 1 MB */
	},
	{
		.num = SCP_A_LOGGER_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x180000,  /* 1.5 MB */
	},
#if defined(CONFIG_SND_SOC_MTK_SCP_SMARTPA) || \
	defined(CONFIG_MTK_AURISYS_PHONE_CALL_SUPPORT) || \
	defined(CONFIG_MTK_AUDIO_TUNNELING_SUPPORT) || \
	defined(CONFIG_MTK_VOW_SUPPORT)
	{
		.num = AUDIO_IPI_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x19000,  /* 100 KB */
	},
#endif
#ifdef CONFIG_MTK_VOW_BARGE_IN_SUPPORT
	{
		.num = VOW_BARGEIN_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x4600,  /* 17KB */
	},
#endif
#ifdef SCP_PARAMS_TO_SCP_SUPPORT
	{
		.num = SCP_DRV_PARAMS_MEM_ID,
		.start_phys = 0x0,
		.start_virt = 0x0,
		.size = 0x100,  /* 256 bytes */
	},
#endif
};


#endif

