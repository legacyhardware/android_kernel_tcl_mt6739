
#include <linux/module.h>       /* needed by all modules */
#include <mt-plat/sync_write.h>
#include "sspm_define.h"
#include "sspm_ipi_id.h"

/* debug API */
#include <memory/mediatek/emi.h>
__weak void dump_emi_outstanding(void) {}
__weak void mtk_spm_dump_debug_info(void) {}
__weak void usb_dump_debug_register(void) {}
__weak void dpmaif_dump_reg(void) {}
__weak void ccci_md_debug_dump(char *user_info) {}

static char *pin_name[SSPM_IPI_COUNT] = {
	"PPM",
	"QOS",
	"PMIC",
	"MET",
	"THERMAL",
	"GPU_DVFS",
	"GPU_PM",
	"PLATFORM",
	"SMI",
	"CM",
	"SLBC",
	"QOS",
	"MET",
	"GPU_DVFS",
	"PLATFORM",
	"SLBC",
};

/* platform callback when ipi timeout */
void sspm_ipi_timeout_cb(int ipi_id)
{
	pr_info("Error: possible error IPI %d pin=%s\n",
		ipi_id, pin_name[ipi_id]);

	ipi_monitor_dump(&sspm_ipidev);

	/* for emi debug dump */
	mtk_emidbg_dump();
	/* for debug CCCI */
	ccci_md_debug_dump("sspm");

	BUG_ON(1);
}

