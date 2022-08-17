
#ifndef __MD_SYS1_PLATFORM_H__
#define __MD_SYS1_PLATFORM_H__

#include <linux/skbuff.h>
#include "ccif_hif_platform.h"

/* this is the platform header file for CLDMA MODEM, not just CLDMA! */

#define AP_REMAP_ADDR_FOR_MD_CLDMA 0xA0000000

/* Modem WDT */
/* BASE_ADDR_MDRSTCTL+ */
#define REG_MDRSTCTL_WDTCR            (0x0000) /*WDT_MODE*/
#define REG_MDRSTCTL_WDTRR            (0x0010) /*WDT_RESTART*/
#define REG_MDRSTCTL_WDTIR            (0x023C) /*LENGTH*/
#define REG_MDRSTCTL_WDTSR            (0x0034) /*WDT_STATUS*/
#define WDT_MD_MODE		REG_MDRSTCTL_WDTCR
#define WDT_MD_LENGTH           REG_MDRSTCTL_WDTIR
#define WDT_MD_RESTART          REG_MDRSTCTL_WDTRR
#define WDT_MD_STA              REG_MDRSTCTL_WDTSR
#define WDT_MD_MODE_KEY	        (0x55000030)
/* L1_BASE_ADDR_L1RGU+ */
#define REG_L1RSTCTL_WDT_MODE         (0x0000)
#define REG_L1RSTCTL_WDT_LENGTH       (0x0004)
#define REG_L1RSTCTL_WDT_RESTART      (0x0008)
#define REG_L1RSTCTL_WDT_STA          (0x000C)
#define REG_L1RSTCTL_WDT_SWRST        (0x001C)
#define L1_WDT_MD_MODE_KEY	(0x00002200)

/* MD1 PLL */
/* MD_CLKSW_BASE+ */
#define R_CLKSEL_CTL			(0x0024)
#define R_FLEXCKGEN_SEL0		(0x0028)
#define R_FLEXCKGEN_SEL1		(0x002C)
#define R_FLEXCKGEN_SEL2		(0x0044)
/* Bit  0: PSMCUPLL_RDY, Bit  1: L1MCUPLL_RDY */
#define R_PLL_STS			(0x0040)
#define R_FLEXCKGEN_STS0		(0x0030)
/* Bit  31: EQ_CK_RDY */
/* Bit  23: BRP_CK_RDY */
/* Bit  7: BUS_CK_RDY */
#define R_FLEXCKGEN_STS1		(0x0034)
#define R_FLEXCKGEN_STS2		(0x0048)
/* Bit  15: INTF_CK_RDY, Bit  23: MD2G_CK_RDY */
/* PSMCU DCM: MD_GLOBAL_CON_DCM_BASE+ */
/* Bit 26-20: DBC_CNT, Bit 16-12: IDLE_FSEL, Bit 2: DBC_EN, Bit 1: DCM_EN */
#define R_PSMCU_DCM_CTL0		(0x0010)
/* Bit 5: APB_LOAD_TOG, Bit 4-0: APB_SEL */
#define R_PSMCU_DCM_CTL1		(0x0014)
/* MD_GLOBAL_CON_DCM_BASE+ */
/* Bit 26-20: DBC_CNT, Bit 16-12: IDLE_FSEL, Bit 2: DBC_EN, Bit 1: DCM_EN */
#define R_ARM7_DCM_CTL0			(0x0020)
/* Bit 5: APB_LOAD_TOG, Bit 4-0: APB_SEL */
#define R_ARM7_DCM_CTL1			(0x0024)
#define MD_GLOBAL_CON_DUMMY		(0x1000)
#define MD_PLL_MAGIC_NUM		(0x67550000)
/* MDSYS_CLKCTL_BASE+ */
#define R_DCM_SHR_SET_CTL		(0x0004)
#define R_LTEL2_BUS_DCM_CTL		(0x0010)
#define R_MDDMA_BUS_DCM_CTL		(0x0014)
#define R_MDREG_BUS_DCM_CTL		(0x0018)
#define R_MODULE_BUS2X_DCM_CTL		(0x001C)
#define R_MODULE_BUS1X_DCM_CTL		(0x0020)
#define R_MDINFRA_CKEN			(0x0044)
#define R_MDPERI_CKEN			(0x0048)
#define R_MDPERI_DCM_MASK		(0x0064)
#define R_PSMCU_AO_CLK_CTL		(0x00C0)
/* MD_PERI_MISC_BASE+ */
#define R_L1_PMS			(0x00C4)
/* PMDL1A0_BASE+ */
/* Bit 7: 0: clock do not from PLL, 1: clock from PLL */
#define REG_DCM_PLLCK_SEL		(0x0188)
#define R_L1MCU_PWR_AWARE		(0x0190)
#define R_L1AO_PWR_AWARE		(0x0194)
#define R_BUSL2DCM_CON3			(0x0198)
#define R_L1MCU_DCM_CON2		(0x0184)
#define R_L1MCU_DCM_CON			(0x0180)
/* ap_mixed_base+ */
/* ((UINT32P)(APMIXED_BASE+0x0)) */
#define AP_PLL_CON0		0x0
/* ((UINT32P)(APMIXED_BASE+0x02C8)) */
#define MDPLL1_CON0		0x3A0

struct ccci_clk_node {
	struct clk *clk_ref;
	unsigned char *clk_name;
};

struct md_pll_reg {
	void __iomem *md_top_clkSW;
	void __iomem *md_top_Pll;

	void __iomem *md_boot_stats0;
	void __iomem *md_boot_stats1;
};
struct md_hw_info {
	/* HW info - Register Address */
	unsigned long md_rgu_base;
	unsigned long l1_rgu_base;
	unsigned long ap_mixed_base;
	void __iomem *ap_topclkgen_base;
	unsigned long md_boot_slave_Vector;
	unsigned long md_boot_slave_Key;
	unsigned long md_boot_slave_En;
	unsigned long ap_ccif_base;
	unsigned long md_ccif_base;
	unsigned int sram_size;

	/* HW info - Interrutpt ID */
	unsigned int ap_ccif_irq0_id;
	unsigned int ap_ccif_irq1_id;
	unsigned int md_wdt_irq_id;
	unsigned int ap2md_bus_timeout_irq_id;
	void __iomem *md_pcore_pccif_base;

	/* HW info - Interrupt flags */
	unsigned long ap_ccif_irq0_flags;
	unsigned long ap_ccif_irq1_flags;
	unsigned long md_wdt_irq_flags;
	unsigned long ap2md_bus_timeout_irq_flags;
	void *hif_hw_info;
};

struct cldma_hw_info {
	unsigned long cldma_ap_ao_base;
	unsigned long cldma_ap_pdn_base;
	unsigned int cldma_irq_id;
	unsigned long cldma_irq_flags;
};

int ccci_modem_remove(struct platform_device *dev);
void ccci_modem_shutdown(struct platform_device *dev);
int ccci_modem_suspend(struct platform_device *dev, pm_message_t state);
int ccci_modem_resume(struct platform_device *dev);
int ccci_modem_pm_suspend(struct device *device);
int ccci_modem_pm_resume(struct device *device);
int ccci_modem_pm_restore_noirq(struct device *device);
int md_cd_power_on(struct ccci_modem *md);
int md_cd_power_off(struct ccci_modem *md, unsigned int timeout);
int md_cd_soft_power_off(struct ccci_modem *md, unsigned int mode);
int md_cd_soft_power_on(struct ccci_modem *md, unsigned int mode);
int md_cd_let_md_go(struct ccci_modem *md);
void md_cd_lock_cldma_clock_src(int locked);
void md_cd_lock_modem_clock_src(int locked);
int md_cd_bootup_cleanup(struct ccci_modem *md, int success);
int md_cd_low_power_notify(struct ccci_modem *md,
	enum LOW_POEWR_NOTIFY_TYPE type, int level);
int md_cd_get_modem_hw_info(struct platform_device *dev_ptr,
	struct ccci_dev_cfg *dev_cfg, struct md_hw_info *hw_info);
int md_cd_io_remap_md_side_register(struct ccci_modem *md);
void md_cd_dump_debug_register(struct ccci_modem *md);
void md_cd_dump_md_bootup_status(struct ccci_modem *md);
void md_cd_get_md_bootup_status(struct ccci_modem *md,
	unsigned int *buff, int length);
void md_cd_check_emi_state(struct ccci_modem *md, int polling);
void cldma_dump_register(struct md_cd_ctrl *md_ctrl);
void md_cldma_hw_reset(unsigned char md_id);
int md_cd_pccif_send(struct ccci_modem *md, int channel_id);
void md_cd_dump_pccif_reg(struct ccci_modem *md);
int md_cd_vcore_config(unsigned int md_id, unsigned int hold_req);

/* ADD_SYS_CORE */
int ccci_modem_syssuspend(void);
void ccci_modem_sysresume(void);
void md_cd_check_md_DCM(struct md_cd_ctrl *md_ctrl);

extern unsigned long infra_ao_base;
extern volatile unsigned int devapc_check_flag;
extern void ccci_mem_dump(int md_id, void *start_addr, int len);
#endif				/* __CLDMA_PLATFORM_H__ */
