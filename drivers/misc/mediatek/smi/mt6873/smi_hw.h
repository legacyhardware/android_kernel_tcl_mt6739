
#ifndef __SMI_HW_H__
#define __SMI_HW_H__

#include <clk-mt6873-pg.h>
#include <smi_port.h>

static const u32 smi_subsys_to_larbs[NR_SYSS] = {
	[SYS_ISP] = (1 << 9),
	[SYS_ISP2] = (1 << 11),
	[SYS_IPE] = (1 << 19) | (1 << 20) | (1 << 29),
	[SYS_VDE] = (1 << 5) | (1 << 26),
	[SYS_VDE2] = (1 << 4),
	[SYS_VEN] = (1 << 7),
	[SYS_MDP] = (1 << 2),
	[SYS_DIS] = (1 << 0) | (1 << 1) |
		    (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25),
	[SYS_CAM] = (1 << 13) | (1 << 14) | (1 << 27) | (1 << 28),
	[SYS_CAM_RAWA] = (1 << 16),
	[SYS_CAM_RAWB] = (1 << 17),
	[SYS_CAM_RAWC] = (1 << 18),
};

#if IS_ENABLED(CONFIG_MMPROFILE)
#include <mmprofile.h>

static const char *smi_mmp_name[NR_SYSS] = {
	[SYS_ISP] = "ISP",
	[SYS_ISP2] = "ISP2",
	[SYS_IPE] = "IPE",
	[SYS_VDE] = "VDE",
	[SYS_VDE2] = "VDE2",
	[SYS_VEN] = "VEN",
	[SYS_MDP] = "MDP",
	[SYS_DIS] = "DIS",
	[SYS_CAM] = "CAM",
	[SYS_CAM_RAWA] = "CAM_RAWA",
	[SYS_CAM_RAWB] = "CAM_RAWB",
	[SYS_CAM_RAWC] = "CAM_RAWC",
};
static mmp_event smi_mmp_event[NR_SYSS];
#endif
#endif