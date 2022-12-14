#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>

#ifndef __DFRC_H__
#define __DFRC_H__

#define DFRC_ALLOW_VIDEO 0x01
#define DFRC_ALLOW_TOUCH 0x02

#define DFRC_DRV_ALL_PID -1
#define DFRC_DRV_ALL_CID 0

#define DFRC_DRV_FPS_NON_ASSIGN 0
#define DFRC_DRV_API_NON_ASSIGN -1

int dfrc_allow_rrc_adjust_fps(void);

long dfrc_set_kernel_policy(int api, int fps, int mode, int target_pid,
				unsigned long long gl_context_id);

long dfrc_get_frr_setting(int pid, unsigned long long gl_context_id,
				int *fps, int *mode);

long dfrc_get_frr_config(int pid, unsigned long long gl_context_id,
				int *fps, int *mode, int *api);

#endif
