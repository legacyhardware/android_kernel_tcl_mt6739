/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __LINUX_REGULATOR_MT6315_H
#define __LINUX_REGULATOR_MT6315_H

#define MT6315_SLAVE_ID_3	3
#define MT6315_SLAVE_ID_6	6
#define MT6315_SLAVE_ID_7	7

enum {
	MT6315_ID_6_VBUCK1 = 0,
	MT6315_ID_6_VBUCK3,
	MT6315_ID_6_MAX,
};

enum {
	MT6315_ID_7_VBUCK1 = 0,
	MT6315_ID_7_VBUCK3,
#if defined(CONFIG_MACH_MT6893)
	MT6315_ID_7_VBUCK4,
#endif
	MT6315_ID_7_MAX,
};

enum {
	MT6315_ID_3_VBUCK1 = 0,
	MT6315_ID_3_VBUCK3,
	MT6315_ID_3_VBUCK4,
	MT6315_ID_3_MAX,
};

#endif /* __LINUX_REGULATOR_MT6315_H */
