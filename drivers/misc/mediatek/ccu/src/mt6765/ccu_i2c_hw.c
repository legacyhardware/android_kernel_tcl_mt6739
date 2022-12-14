
#include "ccu_cmn.h"
#include "ccu_i2c.h"
#include "ccu_i2c_hw.h"
#include "ccu_n3d_a.h"

static unsigned long g_n3d_base;

int ccu_i2c_set_n3d_base(unsigned long n3d_base)
{
	g_n3d_base = n3d_base;

	return 0;
}

int ccu_trigger_i2c_hw(enum CCU_I2C_CHANNEL channel,
	int transac_len, MBOOL do_dma_en)
{
/*trigger i2c start from n3d_a*/
	n3d_a_writew(0x0000007C, g_n3d_base, OFFSET_CTL);
	switch (channel) {
	case CCU_I2C_CHANNEL_MAINCAM:
		{
			n3d_a_writew(0x00000002, g_n3d_base, OFFSET_TRIG);
			break;
		}
	case CCU_I2C_CHANNEL_SUBCAM:
		{
			n3d_a_writew(0x00000001, g_n3d_base, OFFSET_TRIG);
			break;
		}
	default:
		{
			LOG_ERR("ccu_trigger_i2c fail, unknown channel: %d\n",
				channel);
			return MNULL;
		}
	}
	return 0;
}
