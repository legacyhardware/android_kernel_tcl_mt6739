

/* system includes */
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/sched/clock.h>

#include "apusys_device.h"
#include "mnoc_hw.h"
#include "mnoc_drv.h"

/* for Kernel Native SMC API */
#include <linux/arm-smccc.h>
#include <mtk_secure_api.h>

enum APUSYS_MNOC_SMC_ID {
	MNOC_INFRA2APU_SRAM_EN,
	MNOC_INFRA2APU_SRAM_DIS,
	MNOC_APU2INFRA_BUS_PROTECT_EN,
	MNOC_APU2INFRA_BUS_PROTECT_DIS,

	NR_APUSYS_MNOC_SMC_ID
};

static const char * const mni_int_sta_string[] = {
	"MNI_QOS_IRQ_FLAG",
	"ADDR_DEC_ERR_FLAG",
	"MST_PARITY_ERR_FLAG",
	"MST_MISRO_ERR_FLAG",
	"MST_CRDT_ERR_FLAG",
};

static const char * const sni_int_sta_string[] = {
	"SLV_PARITY_ERR_FLA",
	"SLV_MISRO_ERR_FLAG",
	"SLV_CRDT_ERR_FLAG",
};

static const char * const rt_int_sta_string[] = {
	"REQRT_MISRO_ERR_FLAG",
	"RSPRT_MISRO_ERR_FLAG",
	"REQRT_TO_ERR_FLAG",
	"RSPRT_TO_ERR_FLAG",
	"REQRT_CBUF_ERR_FLAG",
	"RSPRT_CBUF_ERR_FLAG",
	"REQRT_CRDT_ERR_FLAG",
	"RSPRT_CRDT_ERR_FLAG",
};

static const char * const mni_map_string[][NR_MNI_PER_GROUP] = {
	{"MNI_MDLA0_1", "MNI_NONE"},
	{"MNI_MDLA0_0", "MNI_NONE"},
	{"MNI_XPU", "MNI_EDMA0"},
	{"MNI_VPU1", "MNI_NONE"},
	{"MNI_VPU0", "MNI_MD32"},
};

static const char * const sni_map_string[][NR_SNI_PER_GROUP] = {
	{"SNI_TCM1", "SNI_TCM3"},
	{"SNI_TCM0", "SNI_TCM2"},
	{"SNI_EMI0", "SNI_EMI1"},
	{"SNI_VPU1", "SNI_NONE"},
	{"SNI_VPU0", "SNI_MD32"},
};

static const unsigned int mni_int_sta_offset[NR_MNI_INT_STA] = {
	MNI_QOS_IRQ_FLAG,
	ADDR_DEC_ERR_FLAG,
	MST_PARITY_ERR_FLAG,
	MST_MISRO_ERR_FLAG,
	MST_CRDT_ERR_FLAG,
};

static const unsigned int sni_int_sta_offset[NR_SNI_INT_STA] = {
	SLV_PARITY_ERR_FLA,
	SLV_MISRO_ERR_FLAG,
	SLV_CRDT_ERR_FLAG,
};

static const unsigned int rt_int_sta_offset[NR_RT_INT_STA] = {
	REQRT_MISRO_ERR_FLAG,
	RSPRT_MISRO_ERR_FLAG,
	REQRT_TO_ERR_FLAG,
	RSPRT_TO_ERR_FLAG,
	REQRT_CBUF_ERR_FLAG,
	RSPRT_CBUF_ERR_FLAG,
	REQRT_CRDT_ERR_FLAG,
	RSPRT_CRDT_ERR_FLAG,
};

static const unsigned int grp_int_map[NR_GROUP] = {
	GRP_INT_MAP_0,
	GRP_INT_MAP_1,
	GRP_INT_MAP_2,
	GRP_INT_MAP_3,
	GRP_INT_MAP_4
};

static const unsigned int grp_base_addr[NR_GROUP] = {
	APU_NOC_GROUP0,
	APU_NOC_GROUP1,
	APU_NOC_GROUP2,
	APU_NOC_GROUP3,
	APU_NOC_GROUP4
};
/* MNI03 in group 3 is dummy */
static unsigned int grp_nr_mni[NR_GROUP] = {1, 1, 2, 2, 2};
static unsigned int grp_nr_sni[NR_GROUP] = {2, 2, 2, 1, 2};

static unsigned int grp_map[NR_APU_QOS_MNI] = {4, 3, 1, 0, 2, 4};
static unsigned int mni_map[NR_APU_QOS_MNI] = {0, 0, 0, 0, 1, 1};

static bool arr_mni_pre_ultra[NR_APU_QOS_MNI] = {0};
static bool arr_mni_lt_guardian_pre_ultra[NR_APU_QOS_MNI] = {0};

static struct mnoc_int_dump mnoc_int_dump[NR_GROUP];
struct int_sta_info apusys_int_sta_dump;

int apusys_dev_to_core_id(int dev_type, int dev_core)
{
	int ret = -1;

	switch (dev_type) {
	case APUSYS_DEVICE_VPU:
	case APUSYS_DEVICE_VPU_RT:
		if (dev_core >= 0 && dev_core < NR_APU_ENGINE_VPU)
			ret = dev_core;
		break;
	case APUSYS_DEVICE_MDLA:
	case APUSYS_DEVICE_MDLA_RT:
		if (dev_core >= 0 && dev_core < NR_APU_ENGINE_MDLA)
			ret = NR_APU_ENGINE_VPU + dev_core;
		break;
	case APUSYS_DEVICE_EDMA:
		if (dev_core >= 0 && dev_core < NR_APU_ENGINE_EDMA)
			ret = NR_APU_ENGINE_VPU + NR_APU_ENGINE_MDLA + dev_core;
		break;
	/* for midware UT */
	case APUSYS_DEVICE_SAMPLE:
	case APUSYS_DEVICE_SAMPLE_RT:
		ret = NR_APU_QOS_ENGINE;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* register to apusys power on callback */
static void mnoc_qos_reg_init(void)
{
	int ni_idx;
	unsigned long flags;

	LOG_DEBUG("+\n");

	spin_lock_irqsave(&mnoc_spinlock, flags);

	/* time slot setting */
	for (ni_idx = 0; ni_idx < NR_APU_QOS_MNI; ni_idx++) {
		/* QoS watcher BW time slot set to 1.26 ms */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			2, mni_map[ni_idx]), 1:0, 0x1);
		/* QoS guardian BW time slot set to 1.26 ms */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			16, mni_map[ni_idx]), 1:0, 0x1);

		/* 26M cycle count = {QW_LT_PRD,8'h0} << QW_LT_PRD_SHF */
		/* QW_LT_PRD = 0x80, QW_LT_PRD_SHF = 0x0 */
		/* QoS watcher LT time slot set to 1.26 ms */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			5, mni_map[ni_idx]), 7:0, 0x80);
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			5, mni_map[ni_idx]), 10:8, 0x0);
		/* QoS guardian LT time slot set to 1.26 ms */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			19, mni_map[ni_idx]), 7:0, 0x80);
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			19, mni_map[ni_idx]), 10:8, 0x0);

		/* MNI to SNI path setting */
		/* set QoS guardian to monitor DRAM only */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			31, mni_map[ni_idx]), 31:16, QOS_MON_SLV_SEL_DRAM);
		/* set QoS watcher to monitor DRAM+TCM */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			31, mni_map[ni_idx]), 15:0, QOS_MON_SLV_SEL_DRAM_TCM);

		/* set QW_BW_INT_EN = 1 to enable monitor */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			1, mni_map[ni_idx]), 2:2, 0x1);
		/* set QG_BW_INT_EN = 1 to enable monitor */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			15, mni_map[ni_idx]), 2:2, 0x1);
		/* set QW_LT_INT_EN = 1 to enable monitor */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			1, mni_map[ni_idx]), 4:4, 0x1);
		/* set QG_LT_INT_EN = 1 to enable monitor */
		mnoc_write_field(MNI_QOS_REG(grp_map[ni_idx], MNI_QOS_CTRL,
			15, mni_map[ni_idx]), 4:4, 0x1);
	}

	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

void mnoc_int_endis(bool endis)
{
	unsigned long flags;

	LOG_DEBUG("+\n");

	spin_lock_irqsave(&mnoc_spinlock, flags);

	if (!mnoc_reg_valid) {
		spin_unlock_irqrestore(&mnoc_spinlock, flags);
		return;
	}
	if (endis)
		mnoc_set_bit(APUSYS_INT_EN, MNOC_INT_MAP);
	else
		mnoc_clr_bit(APUSYS_INT_EN, MNOC_INT_MAP);

	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

/* register to apusys power on callback */
static void mnoc_reg_init(void)
{
	int rt_idx;
	unsigned long flags;

	LOG_DEBUG("+\n");

	spin_lock_irqsave(&mnoc_spinlock, flags);

	/* EMI fine tune: SLV04_QOS/SLV05_QOS = 0x7 */
	mnoc_write_field(MNOC_REG(2, SLV_QOS_CTRL0), 7:0, 0x77);

	/* set request router timeout interrupt */
	for (rt_idx = 0; rt_idx < NR_MNOC_RT; rt_idx++) {
		/* all VC enabled */
		mnoc_write(MNOC_RT_PMU_REG(rt_idx, REQ_RT_PMU, 3), 0xFFFFFFFF);
		mnoc_write(MNOC_RT_PMU_REG(rt_idx, REQ_RT_PMU, 4), 0xFFFFFFFF);
		/* set timeout threshold to 510 cycles */
		mnoc_write_field(MNOC_RT_PMU_REG(rt_idx, REQ_RT_PMU, 2),
			8:0, 510);
		/* enable timeout counting */
		mnoc_write_field(MNOC_RT_PMU_REG(rt_idx, REQ_RT_PMU, 2),
			31:31, 1);
	}

	/* set response router timeout interrupt */
	for (rt_idx = 0; rt_idx < NR_MNOC_RT; rt_idx++) {
		/* all VC enabled */
		mnoc_write(MNOC_RT_PMU_REG(rt_idx, RSP_RT_PMU, 3), 0xFFFFFFFF);
		mnoc_write(MNOC_RT_PMU_REG(rt_idx, RSP_RT_PMU, 4), 0xFFFFFFFF);
		/* set timeout threshold to 510 cycles */
		mnoc_write_field(MNOC_RT_PMU_REG(rt_idx, RSP_RT_PMU, 2),
			8:0, 510);
		/* enable timeout counting */
		mnoc_write_field(MNOC_RT_PMU_REG(rt_idx, RSP_RT_PMU, 2),
			31:31, 1);
	}

	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

int mnoc_check_int_status(void)
{
	int mnoc_irq_triggered = 0;
	unsigned int val, int_sta;
	int grp_idx, int_idx, ni_idx;
	struct mnoc_int_dump *d;
	uint64_t cur_timestamp;

	LOG_DEBUG("+\n");

	int_sta = mnoc_read(APUSYS_INT_STA);

	LOG_DEBUG("APUSYS INT STA = 0x%x\n", int_sta);

	cur_timestamp = sched_clock();

	if (int_sta != 0) {
		apusys_int_sta_dump.reg_val = int_sta;
		apusys_int_sta_dump.timestamp = cur_timestamp;
	}

	if ((int_sta & MNOC_INT_MAP) == 0)
		return mnoc_irq_triggered;

	for (grp_idx = 0; grp_idx < NR_GROUP; grp_idx++) {
		if ((int_sta & grp_int_map[grp_idx]) == 0)
			continue;
		d = &(mnoc_int_dump[grp_idx]);
		d->count++;

		for (int_idx = 0; int_idx < NR_MNI_INT_STA; int_idx++) {
			val = mnoc_read(MNOC_REG(grp_idx,
				mni_int_sta_offset[int_idx]));
			if ((val & 0xFFFF) != 0) {
				d->mni_int_sta[int_idx].reg_val = val;
				d->mni_int_sta[int_idx].timestamp =
					cur_timestamp;
				LOG_DEBUG("RT(%d): %s = 0x%x\n", grp_idx,
					mni_int_sta_string[int_idx], val);
				for (ni_idx = 0; ni_idx < grp_nr_mni[grp_idx];
					ni_idx++)
					if ((val & (1 << ni_idx)) != 0)
						LOG_DEBUG("From %s\n",
							mni_map_string[grp_idx]
							[ni_idx]);
				mnoc_write_field(
					MNOC_REG(grp_idx,
						mni_int_sta_offset[int_idx]),
						15:0, 0xFFFF);
				mnoc_irq_triggered = 1;
			}
		}

		for (int_idx = 0; int_idx < NR_SNI_INT_STA; int_idx++) {
			val = mnoc_read(MNOC_REG(grp_idx,
				sni_int_sta_offset[int_idx]));
			if ((val & 0xFFFF) != 0) {
				d->sni_int_sta[int_idx].reg_val = val;
				d->sni_int_sta[int_idx].timestamp =
					cur_timestamp;
				LOG_DEBUG("RT(%d): %s = 0x%x\n", grp_idx,
					sni_int_sta_string[int_idx], val);
				for (ni_idx = 0; ni_idx < grp_nr_sni[grp_idx];
					ni_idx++)
					if ((val & (1 << ni_idx)) != 0)
						LOG_DEBUG("From %s\n",
							sni_map_string[grp_idx]
							[ni_idx]);
				mnoc_write_field(
					MNOC_REG(grp_idx,
						sni_int_sta_offset[int_idx]),
						15:0, 0xFFFF);
				mnoc_irq_triggered = 1;
			}
		}

		for (int_idx = 0; int_idx < NR_RT_INT_STA; int_idx++) {
			val = mnoc_read(MNOC_REG(grp_idx,
				rt_int_sta_offset[int_idx]));
			if ((val & 0x1F) != 0) {
				d->rt_int_sta[int_idx].reg_val = val;
				d->rt_int_sta[int_idx].timestamp =
					cur_timestamp;
				LOG_DEBUG("RT(%d): %s = 0x%x\n", grp_idx,
					rt_int_sta_string[int_idx], val);
				mnoc_write_field(
					MNOC_REG(grp_idx,
						rt_int_sta_offset[int_idx]),
						4:0, 0x1F);
				/* timeout interrupt may be only perf
				 * hint but not actually hang
				 */
				if (mnoc_irq_triggered != 1 && (
					int_idx == MNOC_INT_REQRT_TO_ERR_FLAG ||
					int_idx == MNOC_INT_RSPRT_TO_ERR_FLAG))
					mnoc_irq_triggered = 2;
				else
					mnoc_irq_triggered = 1;
			}
		}

		/* additional check: sw triggered irq */
		val = mnoc_read_field(MNOC_REG(grp_idx,
			MISC_CTRL), 18:16);
		if (val != 0) {
			d->sw_irq_sta.reg_val = val;
			d->sw_irq_sta.timestamp = cur_timestamp;
			LOG_DEBUG("RT(%d): From SW_IRQ = 0x%x\n",
				grp_idx, val);
			mnoc_write_field(MNOC_REG(grp_idx, MISC_CTRL),
				18:16, 0x0);
			mnoc_irq_triggered = 1;
		}
	}

	LOG_DEBUG("-\n");

	return mnoc_irq_triggered;
}

/* read PMU_COUNTER_OUT 0~15 value to pmu buffer */
void mnoc_get_pmu_counter(unsigned int *buf)
{
	int grp_idx, cntr_idx;
	unsigned long flags;

	LOG_DEBUG("+\n");

	spin_lock_irqsave(&mnoc_spinlock, flags);
	for (grp_idx = 0; grp_idx < NR_GROUP; grp_idx++) {
		if (mnoc_reg_valid)
			for (cntr_idx = 0; cntr_idx < NR_PMU_CNTR_PER_GRP;
				cntr_idx++)
				buf[grp_idx*NR_PMU_CNTR_PER_GRP+cntr_idx] =
				mnoc_read(MNOC_RT_PMU_REG(grp_idx,
					PMU_COUNTER0_OUT, cntr_idx));
		else
			for (cntr_idx = 0; cntr_idx < NR_PMU_CNTR_PER_GRP;
				cntr_idx++)
				buf[grp_idx*NR_PMU_CNTR_PER_GRP+cntr_idx] = 0;
	}
	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

void mnoc_clear_pmu_counter(unsigned int grp)
{
	unsigned long flags;

	LOG_DEBUG("+\n");

	if (grp >= NR_GROUP)
		return;

	spin_lock_irqsave(&mnoc_spinlock, flags);
	if (mnoc_reg_valid) {
		mnoc_write_field(MNOC_REG(grp, APU_NOC_PMU_CTRL0),
			29:29, 1);
		mnoc_write_field(MNOC_REG(grp, APU_NOC_PMU_CTRL0),
			29:29, 0);
	}
	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

bool mnoc_pmu_reg_in_range(unsigned int addr)
{
	int grp_idx;
	unsigned int start, end;

	for (grp_idx = 0; grp_idx < NR_GROUP; grp_idx++) {
		start = APU_NOC_PMU_ADDR + grp_idx*APU_NOC_GRP_REG_SZ;
		end = start + APU_NOC_PMU_RANGE;
		if (addr >= start && addr < end)
			return true;
	}

	return false;
}

#if 0
void mnoc_tcm_hash_set(unsigned int sel, unsigned int en0, unsigned int en1)
{
	unsigned long flags;

	LOG_DEBUG("+\n");

	spin_lock_irqsave(&mnoc_spinlock, flags);
	mnoc_write_field(APU_TCM_HASH_TRUNCATE_CTRL0, 2:0, sel);
	mnoc_write_field(APU_TCM_HASH_TRUNCATE_CTRL0, 6:3, en0);
	mnoc_write_field(APU_TCM_HASH_TRUNCATE_CTRL0, 10:7, en1);
	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("APU_TCM_HASH_TRUNCATE_CTRL0 = 0x%x\n",
		mnoc_read(APU_TCM_HASH_TRUNCATE_CTRL0));

	LOG_DEBUG("-\n");
}
#endif

static void set_mni_pre_ultra_locked(unsigned int idx, bool endis)
{
	unsigned int map, val;

	LOG_DEBUG("+\n");

	/* bit 24 : force AW urgent enable
	 * bit 25 : force AR urgent enable
	 * bit 29 : AW pre-urgent value
	 * bit 31 : AR pre-urgent value
	 */
	map = (1 << 24) | (1 << 25) | (1 << 29) | (1 << 31);

	val = mnoc_read(MNI_QOS_REG(grp_map[idx], MNI_QOS_CTRL,
		15, mni_map[idx]));
	if (endis)
		mnoc_write(MNI_QOS_REG(grp_map[idx], MNI_QOS_CTRL,
			15, mni_map[idx]), (val | map));
	else
		mnoc_write(MNI_QOS_REG(grp_map[idx], MNI_QOS_CTRL,
			15, mni_map[idx]), (val & (~map)));

	LOG_DEBUG("-\n");
}

void mnoc_set_mni_pre_ultra(int dev_type, int dev_core, bool endis)
{
	unsigned long flags;
	unsigned int idx;
	int core;

	LOG_DEBUG("+\n");

	core = apusys_dev_to_core_id(dev_type, dev_core);

	if (core == -1) {
		LOG_ERR("illegal dev_type(%d), dev_core(%d)\n",
			dev_type, dev_core);
		return;
	}

	spin_lock_irqsave(&mnoc_spinlock, flags);

	switch (core) {
	case APU_QOS_ENGINE_VPU0:
	case APU_QOS_ENGINE_VPU1:
		idx = core;
		arr_mni_pre_ultra[idx] = endis;
		if (mnoc_reg_valid)
			set_mni_pre_ultra_locked(idx, endis);
		break;
	case APU_QOS_ENGINE_MDLA0:
		idx = (core - APU_QOS_ENGINE_MDLA0) * 2 + MNI_MDLA0_0;
		arr_mni_pre_ultra[idx] = endis;
		arr_mni_pre_ultra[idx+1] = endis;
		if (mnoc_reg_valid) {
			set_mni_pre_ultra_locked(idx, endis);
			set_mni_pre_ultra_locked(idx+1, endis);
		}
		break;
	case APU_QOS_ENGINE_EDMA0:
	case APU_QOS_ENGINE_MD32:
		idx = core - APU_QOS_ENGINE_EDMA0 + MNI_EDMA0;
		arr_mni_pre_ultra[idx] = endis;
		if (mnoc_reg_valid)
			set_mni_pre_ultra_locked(idx, endis);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

static void set_lt_guardian_pre_ultra_locked(unsigned int idx, bool endis)
{
	LOG_DEBUG("+\n");

	if (endis) {
		/* set QG_LT_THH */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 20, mni_map[idx]),
			12:0, QG_LT_THH_PRE_ULTRA);
		/* set QG_LT_THL */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 20, mni_map[idx]),
			28:16, QG_LT_THL_PRE_ULTRA);
		/* set QCC_LT_LV_DIS[3:0] = 4'b1001 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			11:8, 0x9);
		/* set STM mode QCC_LT_TH_MODE = 1 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			16:16, 0x1);
		/* set QCC_TOP_URGENT_EN = 0 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			19:19, 0x0);
	} else {
		/* set QG_LT_THH */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 20, mni_map[idx]),
			12:0, 0x0);
		/* set QG_LT_THL */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 20, mni_map[idx]),
			28:16, 0x0);
		/* set QCC_LT_LV_DIS[3:0] = 4'b0000 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			11:8, 0x0);
		/* set STM mode QCC_LT_TH_MODE = 0 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			16:16, 0x0);
		/* set QCC_TOP_URGENT_EN = 1 */
		mnoc_write_field(
			MNI_QOS_REG(grp_map[idx],
				MNI_QOS_CTRL, 29, mni_map[idx]),
			19:19, 0x1);
	}

	LOG_DEBUG("-\n");
}

void mnoc_set_lt_guardian_pre_ultra(int dev_type, int dev_core, bool endis)
{
	unsigned long flags;
	unsigned int idx;
	int core;

	LOG_DEBUG("+\n");

	core = apusys_dev_to_core_id(dev_type, dev_core);

	if (core == -1) {
		LOG_ERR("illegal dev_type(%d), dev_core(%d)\n",
			dev_type, dev_core);
		return;
	}

	spin_lock_irqsave(&mnoc_spinlock, flags);

	switch (core) {
	case APU_QOS_ENGINE_VPU0:
	case APU_QOS_ENGINE_VPU1:
		idx = core;
		arr_mni_lt_guardian_pre_ultra[idx] = endis;
		if (mnoc_reg_valid)
			set_lt_guardian_pre_ultra_locked(idx, endis);
		break;
	case APU_QOS_ENGINE_MDLA0:
		idx = (core - APU_QOS_ENGINE_MDLA0) * 2 + MNI_MDLA0_0;
		arr_mni_lt_guardian_pre_ultra[idx] = endis;
		arr_mni_lt_guardian_pre_ultra[idx+1] = endis;
		if (mnoc_reg_valid) {
			set_lt_guardian_pre_ultra_locked(idx, endis);
			set_lt_guardian_pre_ultra_locked(idx+1, endis);
		}
		break;
	case APU_QOS_ENGINE_EDMA0:
	case APU_QOS_ENGINE_MD32:
		idx = core - APU_QOS_ENGINE_EDMA0 + MNI_EDMA0;
		arr_mni_lt_guardian_pre_ultra[idx] = endis;
		if (mnoc_reg_valid)
			set_lt_guardian_pre_ultra_locked(idx, endis);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

/* After APUSYS top power on */
void infra2apu_sram_en(void)
{
	struct arm_smccc_res res;

	LOG_DEBUG("+\n");

	/*
	 * arm_smccc_smc (unsigned long a0, unsigned long a1,
	 *	unsigned long a2, unsigned long a3, unsigned long a4,
	 *	unsigned long a5, unsigned long a6, unsigned long a7,
	 *	struct arm_smccc_res *res)
	 */
	arm_smccc_smc(MTK_SIP_APUSYS_MNOC_CONTROL,
		MNOC_INFRA2APU_SRAM_EN,
		0, 0, 0, 0, 0, 0, &res);

	LOG_DEBUG("-\n");
}

/* Before APUSYS top power off */
void infra2apu_sram_dis(void)
{
	struct arm_smccc_res res;

	LOG_DEBUG("+\n");

	/*
	 * arm_smccc_smc (unsigned long a0, unsigned long a1,
	 *	unsigned long a2, unsigned long a3, unsigned long a4,
	 *	unsigned long a5, unsigned long a6, unsigned long a7,
	 *	struct arm_smccc_res *res)
	 */
	arm_smccc_smc(MTK_SIP_APUSYS_MNOC_CONTROL,
		MNOC_INFRA2APU_SRAM_DIS,
		0, 0, 0, 0, 0, 0, &res);

	LOG_DEBUG("-\n");
}

/* Before APUSYS reset */
void apu2infra_bus_protect_en(void)
{
	struct arm_smccc_res res;

	LOG_DEBUG("+\n");

	/*
	 * arm_smccc_smc (unsigned long a0, unsigned long a1,
	 *	unsigned long a2, unsigned long a3, unsigned long a4,
	 *	unsigned long a5, unsigned long a6, unsigned long a7,
	 *	struct arm_smccc_res *res)
	 */
	arm_smccc_smc(MTK_SIP_APUSYS_MNOC_CONTROL,
		MNOC_APU2INFRA_BUS_PROTECT_EN,
		0, 0, 0, 0, 0, 0, &res);

	LOG_DEBUG("-\n");
}

/* After APUSYS reset */
void apu2infra_bus_protect_dis(void)
{
	struct arm_smccc_res res;

	LOG_DEBUG("+\n");

	/*
	 * arm_smccc_smc (unsigned long a0, unsigned long a1,
	 *	unsigned long a2, unsigned long a3, unsigned long a4,
	 *	unsigned long a5, unsigned long a6, unsigned long a7,
	 *	struct arm_smccc_res *res)
	 */
	arm_smccc_smc(MTK_SIP_APUSYS_MNOC_CONTROL,
		MNOC_APU2INFRA_BUS_PROTECT_DIS,
		0, 0, 0, 0, 0, 0, &res);

	LOG_DEBUG("-\n");
}

void mnoc_hw_reinit(void)
{
	unsigned long flags;
	int idx;

	LOG_DEBUG("+\n");

	mnoc_qos_reg_init();
	mnoc_reg_init();

	spin_lock_irqsave(&mnoc_spinlock, flags);
	for (idx = 0; idx < NR_APU_QOS_MNI; idx++) {
		if (arr_mni_pre_ultra[idx])
			set_mni_pre_ultra_locked(idx, 1);
		if (arr_mni_lt_guardian_pre_ultra[idx])
			set_lt_guardian_pre_ultra_locked(idx, 1);
	}
	spin_unlock_irqrestore(&mnoc_spinlock, flags);

	LOG_DEBUG("-\n");
}

static void print_int_sta_info(struct seq_file *m,
	const char *str, struct int_sta_info *info)
{
	uint64_t t, nanosec_rem;

	t = info->timestamp;
	nanosec_rem = do_div(t, 1000000000);

	INT_STA_PRINTF(m, "%s = 0x%x [%lu.%06lu]\n",
		str,
		info->reg_val,
		(unsigned long) t,
		(unsigned long) (nanosec_rem / 1000));
}

void print_int_sta(struct seq_file *m)
{
	int grp_idx, idx, ni_idx;
	uint64_t t, nanosec_rem;
	unsigned int val;

	t = sched_clock();
	nanosec_rem = do_div(t, 1000000000);

	INT_STA_PRINTF(m, "[%lu.%06lu]\n",
		(unsigned long) t, (unsigned long) (nanosec_rem / 1000));

	print_int_sta_info(m, "apusys_int_sta",
		&(apusys_int_sta_dump));

	for (grp_idx = 0; grp_idx < NR_GROUP; grp_idx++) {
		INT_STA_PRINTF(m, "========= Group %d =========\n", grp_idx);
		INT_STA_PRINTF(m, "count = %d\n", mnoc_int_dump[grp_idx].count);

		for (idx = 0; idx < NR_MNI_INT_STA; idx++) {
			print_int_sta_info(m, mni_int_sta_string[idx],
				&(mnoc_int_dump[grp_idx].mni_int_sta[idx]));
			val = mnoc_int_dump[grp_idx].mni_int_sta[idx].reg_val;
			for (ni_idx = 0; ni_idx < grp_nr_mni[grp_idx]; ni_idx++)
				if ((val & (1 << ni_idx)) != 0)
					INT_STA_PRINTF(m, "\t-From %s\n",
					mni_map_string[grp_idx][ni_idx]);
		}

		for (idx = 0; idx < NR_SNI_INT_STA; idx++) {
			print_int_sta_info(m, sni_int_sta_string[idx],
				&(mnoc_int_dump[grp_idx].sni_int_sta[idx]));
			val = mnoc_int_dump[grp_idx].sni_int_sta[idx].reg_val;
			for (ni_idx = 0; ni_idx < grp_nr_sni[grp_idx]; ni_idx++)
				if ((val & (1 << ni_idx)) != 0)
					INT_STA_PRINTF(m, "\t-From %s\n",
					sni_map_string[grp_idx][ni_idx]);
		}

		for (idx = 0; idx < NR_RT_INT_STA; idx++)
			print_int_sta_info(m, rt_int_sta_string[idx],
				&(mnoc_int_dump[grp_idx].rt_int_sta[idx]));

		print_int_sta_info(m, "sw_irq_sta",
			&(mnoc_int_dump[grp_idx].sw_irq_sta));
	}
}

void mnoc_hw_init(void)
{
	int grp_idx, idx;

	LOG_DEBUG("+\n");

	apusys_int_sta_dump.reg_val = 0;
	apusys_int_sta_dump.timestamp = 0;

	for (grp_idx = 0; grp_idx < NR_GROUP; grp_idx++) {
		mnoc_int_dump[grp_idx].count = 0;
		for (idx = 0; idx < NR_MNI_INT_STA; idx++) {
			mnoc_int_dump[grp_idx].mni_int_sta[idx].reg_val = 0;
			mnoc_int_dump[grp_idx].mni_int_sta[idx].timestamp = 0;
		}
		for (idx = 0; idx < NR_SNI_INT_STA; idx++) {
			mnoc_int_dump[grp_idx].sni_int_sta[idx].reg_val = 0;
			mnoc_int_dump[grp_idx].sni_int_sta[idx].timestamp = 0;
		}
		for (idx = 0; idx < NR_RT_INT_STA; idx++) {
			mnoc_int_dump[grp_idx].rt_int_sta[idx].reg_val = 0;
			mnoc_int_dump[grp_idx].rt_int_sta[idx].timestamp = 0;
		}
		mnoc_int_dump[grp_idx].sw_irq_sta.reg_val = 0;
		mnoc_int_dump[grp_idx].sw_irq_sta.timestamp = 0;
	}

	LOG_DEBUG("-\n");
}

void mnoc_hw_exit(void)
{
}

phys_addr_t get_apu_iommu_tfrp(unsigned int id)
{
	return 0;
}
