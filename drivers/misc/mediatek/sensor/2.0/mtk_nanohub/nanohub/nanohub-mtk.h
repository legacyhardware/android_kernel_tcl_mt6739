
#ifndef _NANOHUB_MTK_IPI_H
#define _NANOHUB_MTK_IPI_H
#include <scp_ipi.h>
#include <scp_helper.h>
#include <linux/notifier.h>

int nanohub_ipi_init(void);
void scp_wdt_reset(enum scp_core_id cpu_id);

#endif

