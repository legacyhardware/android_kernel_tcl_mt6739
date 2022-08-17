
#ifndef __MODEM_REG_BASE_H__
#define __MODEM_REG_BASE_H__

/* ============================================================ */
/* Modem 1 part */
/* ============================================================ */

#define MD_BOOT_VECTOR_EN 0x20000024
#define INFRA_AO_MD_SRCCLKENA    (0xF0C) /* SRC CLK ENA */

#define MD_PCORE_PCCIF_BASE 0x20510000

#define CCIF_SRAM_SIZE 512


/*========== MD Exception dump register list start[ */
/*MD bootup register*/
#define MD1_CFG_BASE (0x1020E300)
#define MD1_BOOT_STATS_SELECT (0x1020E700)
#define MD1_CFG_BOOT_STATS (MD1_CFG_BASE+0x00)

/* MD Exception dump register list end]======== */
#endif				/* __MODEM_REG_BASE_H__ */