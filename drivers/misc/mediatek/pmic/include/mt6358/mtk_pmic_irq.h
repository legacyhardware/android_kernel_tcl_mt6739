
#ifndef __MT_PMIC_IRQ_H
#define __MT_PMIC_IRQ_H

#define PMIC_INT_WIDTH 16

#define PMIC_SP_IRQ_GEN(_used, _name)	\
	{	\
		.used = _used,	\
		.name = #_name,	\
	}

#define PMIC_SP_INTS_GEN(sp, _con_len, _sp_irqs, _top_int_bit)	\
	{	\
		.enable = MT6358_##sp##_INT_CON0,		\
		.mask = MT6358_##sp##_INT_MASK_CON0,	\
		.status = MT6358_##sp##_INT_STATUS0,	\
		.raw_status = MT6358_##sp##_INT_RAW_STATUS0,	\
		.con_len = _con_len,	\
		.int_offset = SP_##sp##_START,	\
		.sp_irqs = _sp_irqs,	\
		.top_int_bit = _top_int_bit,		\
	}

struct pmic_sp_irq {
	unsigned short used;
	const char *name;
	void (*callback)(void);
	void (*oc_callback)(enum PMIC_IRQ_ENUM intNo, const char *int_name);
	unsigned int times;
};

struct pmic_sp_interrupt {
	unsigned int enable;
	unsigned int mask;
	unsigned int status;
	unsigned int raw_status;
	unsigned int con_len;
	unsigned int int_offset; /* start intNo of this subpack */
	struct pmic_sp_irq (*sp_irqs)[PMIC_INT_WIDTH];
	unsigned int top_int_bit;
};

#if defined(MTK_EVB_PLATFORM) || defined(CONFIG_FPGA_EARLY_PORTING)
#define ENABLE_ALL_OC_IRQ 0
#else
#define ENABLE_ALL_OC_IRQ 0
#endif

#endif /*--PMIC_IRQ_H--*/
