
#include <linux/module.h>
#include "apu_power_table.h"

// FIXME: update vpu power table in DVT stage
/* opp, mW */
struct apu_opp_info vpu_power_table[APU_OPP_NUM] = {
	{APU_OPP_0, 226},
	{APU_OPP_1, 226},
	{APU_OPP_2, 176},
	{APU_OPP_3, 133},
	{APU_OPP_4, 98},
	{APU_OPP_5, 44},
};
EXPORT_SYMBOL(vpu_power_table);

// FIXME: update mdla power table in DVT stage
/* opp, mW */
struct apu_opp_info mdla_power_table[APU_OPP_NUM] = {
	{APU_OPP_0, 161},
	{APU_OPP_1, 161},
	{APU_OPP_2, 161},
	{APU_OPP_3, 120},
	{APU_OPP_4, 86},
	{APU_OPP_5, 44},
};
EXPORT_SYMBOL(mdla_power_table);
