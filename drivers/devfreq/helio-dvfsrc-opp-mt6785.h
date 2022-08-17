
#ifndef __HELIO_DVFSRC_OPP_MT6785_H
#define __HELIO_DVFSRC_OPP_MT6785_H

#include <linux/pm_qos.h>

int ddr_level_to_step(int opp);

enum ddr_opp {
	DDR_OPP_0 = 0,
	DDR_OPP_1,
	DDR_OPP_2,
	DDR_OPP_3,
	DDR_OPP_4,
	DDR_OPP_5,
	DDR_OPP_NUM,
	DDR_OPP_UNREQ = PM_QOS_DDR_OPP_DEFAULT_VALUE,
};

enum vcore_opp {
	VCORE_OPP_0 = 0,
	VCORE_OPP_1,
	VCORE_OPP_2,
	VCORE_OPP_NUM,
	VCORE_OPP_UNREQ = PM_QOS_VCORE_OPP_DEFAULT_VALUE,
};

enum emi_opp {
	EMI_OPP_0 = 0,
	EMI_OPP_1,
	EMI_OPP_2,
	EMI_OPP_NUM,
	EMI_OPP_UNREQ = -1,
};

enum vcore_dvfs_opp {
	VCORE_DVFS_OPP_0 = 0,
	VCORE_DVFS_OPP_1,
	VCORE_DVFS_OPP_2,
	VCORE_DVFS_OPP_3,
	VCORE_DVFS_OPP_4,
	VCORE_DVFS_OPP_5,
	VCORE_DVFS_OPP_6,
	VCORE_DVFS_OPP_7,
	VCORE_DVFS_OPP_8,
	VCORE_DVFS_OPP_9,
	VCORE_DVFS_OPP_10,
	VCORE_DVFS_OPP_11,
	VCORE_DVFS_OPP_12,
	VCORE_DVFS_OPP_NUM,
	VCORE_DVFS_OPP_UNREQ = PM_QOS_VCORE_DVFS_FORCE_OPP_DEFAULT_VALUE,
};

#endif /* __HELIO_DVFSRC_OPP_MT6785_H */
