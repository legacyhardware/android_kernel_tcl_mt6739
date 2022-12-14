#ifndef __MTK_GPU_POWER_MODEL_H__
#define __MTK_GPU_POWER_MODEL_H__
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

enum {
	GPU_PM_POWER_STATUE,
	GPU_PM_SWITCH,	// 0 : API 1 : SSPM
	GPU_PM_LAST,
	GPU_PM_LAST2
};

enum {
	gpm_off,
	gpm_kernel_side,
	gpm_sspm_side
};

struct gpu_pm_ipi_cmds {
	unsigned int cmd[GPU_PM_LAST];
};

void MTKGPUPower_model_stop(void);
void MTKGPUPower_model_start(unsigned int interval_ns);
void MTKGPUPower_model_start_swpm(unsigned int interval_ns);
void MTKGPUPower_model_suspend(void);
void MTKGPUPower_model_resume(void);
int MTKGPUPower_model_init(void);
void MTKGPUPower_model_sspm_enable(void);

#endif
