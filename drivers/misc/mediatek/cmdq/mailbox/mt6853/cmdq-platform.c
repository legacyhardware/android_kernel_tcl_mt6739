// SPDX-License-Identifier: GPL-2.0

#include <dt-bindings/gce/mt6853-gce.h>
#include "../cmdq-util.h"

const char *cmdq_thread_module_dispatch(phys_addr_t gce_pa, s32 thread)
{
	switch (thread) {
	case 0 ... 6:
	case 8 ... 9:
		return "DISP";
	case 7:
		return "VDEC";
	case 10:
	case 19 ... 22:
		return "MDP";
	case 11:
	case 13 ... 14:
	case 16 ... 18:
		return "ISP";
	case 12:
		return "VENC";
	case 15:
		return "CMDQ";
	case 23:
		return "SMI";
	default:
		return "CMDQ";
	}
}

const char *cmdq_event_module_dispatch(phys_addr_t gce_pa, const u16 event,
	s32 thread)
{
	switch (event) {
	case CMDQ_EVENT_ISP_FRAME_DONE_A:
	case CMDQ_EVENT_ISP_FRAME_DONE_B:
		return "ISP";
	case CMDQ_EVENT_CAMSV1_PASS1_DONE:
	case CMDQ_EVENT_CAMSV2_PASS1_DONE:
	case CMDQ_EVENT_CAMSV3_PASS1_DONE:
	case CMDQ_EVENT_MRAW_0_PASS1_DONE:
	case CMDQ_EVENT_SENINF_CAM0_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM1_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM2_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM3_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM4_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM5_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM6_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM7_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM8_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM9_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM10_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM11_FIFO_FULL:
	case CMDQ_EVENT_SENINF_CAM12_FIFO_FULL:
		return "CAM";
	case CMDQ_EVENT_TG_OVRUN_A_INT:
	case CMDQ_EVENT_DMA_R1_ERROR_A_INT:
	case CMDQ_EVENT_TG_OVRUN_B_INT:
	case CMDQ_EVENT_DMA_R1_ERROR_B_INT:
	case CMDQ_EVENT_TG_OVRUN_M0_INT:
	case CMDQ_EVENT_DMA_R1_ERROR_M0_INT:
	case CMDQ_EVENT_TG_GRABERR_M0_INT:
	case CMDQ_EVENT_TG_GRABERR_A_INT:
	case CMDQ_EVENT_CQ_VR_SNAP_A_INT:
	case CMDQ_EVENT_TG_GRABERR_B_INT:
	case CMDQ_EVENT_CQ_VR_SNAP_B_INT:
		return "IMG";
	case CMDQ_EVENT_VENC_CMDQ_FRAME_DONE:
	case CMDQ_EVENT_VENC_CMDQ_PAUSE_DONE:
	case CMDQ_EVENT_JPGENC_CMDQ_DONE:
	case CMDQ_EVENT_VENC_CMDQ_MB_DONE:
	case CMDQ_EVENT_VENC_CMDQ_128BYTE_CNT_DONE:
	case CMDQ_EVENT_VENC_CMDQ_PPS_DONE:
	case CMDQ_EVENT_VENC_CMDQ_SPS_DONE:
	case CMDQ_EVENT_VENC_CMDQ_VPS_DONE:
		return "VENC";
	case CMDQ_EVENT_VDEC_CORE0_SOF_0:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_0:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_1:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_2:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_3:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_4:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_5:
	case CMDQ_EVENT_VDEC_CORE0_FRAME_DONE_6:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_0:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_1:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_2:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_3:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_4:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_5:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_6:
	case CMDQ_EVENT_VDEC_CORE0_ENG_EVENT_7:
		return "VDEC";
	case CMDQ_EVENT_FDVT_DONE:
		return "FDVT";
	case CMDQ_EVENT_RSC_DONE:
		return "RSC";
	case CMDQ_EVENT_DVS_DONE_ASYNC_SHOT:
	case CMDQ_EVENT_DVP_DONE_ASYNC_SHOT:
	case CMDQ_EVENT_IMG2_MFB_DONE_LINK_MISC:
	case CMDQ_EVENT_IMG2_WPE_A_DONE_LINK_MISC:
	case CMDQ_EVENT_IMG2_MSS_DONE_LINK_MISC:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_0:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_1:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_2:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_3:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_4:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_5:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_6:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_7:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_8:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_9:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_10:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_11:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_12:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_13:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_14:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_15:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_16:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_17:
	case CMDQ_EVENT_IMG1_DIP_FRAME_DONE_P2_18:
	case CMDQ_EVENT_IMG1_DIP_DMA_ERR_EVENT:
		return "IMG";
	case CMDQ_EVENT_MDP_RDMA0_SOF:
	case CMDQ_EVENT_MDP_RDMA1_SOF:
	case CMDQ_EVENT_MDP_AAL0_SOF:
	case CMDQ_EVENT_MDP_AAL1_SOF:
	case CMDQ_EVENT_MDP_HDR0_SOF:
	case CMDQ_EVENT_MDP_RSZ0_SOF:
	case CMDQ_EVENT_MDP_RSZ1_SOF:
	case CMDQ_EVENT_MDP_WROT0_SOF:
	case CMDQ_EVENT_MDP_WROT1_SOF:
	case CMDQ_EVENT_MDP_TDSHP0_SOF:
	case CMDQ_EVENT_MDP_TDSHP1_SOF:
		return "MDP";
	case CMDQ_EVENT_IMG_DL_RELAY0_SOF:
	case CMDQ_EVENT_IMG_DL_RELAY1_SOF:
		return "IMG";
	case CMDQ_EVENT_MDP_COLOR0_SOF:
	case CMDQ_EVENT_MDP_WROT1_FRAME_DONE:
	case CMDQ_EVENT_MDP_WROT0_FRAME_DONE:
	case CMDQ_EVENT_MDP_TDSHP1_FRAME_DONE:
	case CMDQ_EVENT_MDP_TDSHP0_FRAME_DONE:
	case CMDQ_EVENT_MDP_RSZ1_FRAME_DONE:
	case CMDQ_EVENT_MDP_RSZ0_FRAME_DONE:
	case CMDQ_EVENT_MDP_RDMA1_FRAME_DONE:
	case CMDQ_EVENT_MDP_RDMA0_FRAME_DONE:
	case CMDQ_EVENT_MDP_HDR0_FRAME_DONE:
	case CMDQ_EVENT_MDP_COLOR0_FRAME_DONE:
	case CMDQ_EVENT_MDP_AAL1_FRAME_DONE:
	case CMDQ_EVENT_MDP_AAL0_FRAME_DONE:
		return "MDP";
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_0:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_1:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_2:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_3:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_4:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_5:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_6:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_7:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_8:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_9:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_10:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_11:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_12:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_13:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_14:
	case CMDQ_EVENT_DISP_STREAM_DONE_ENG_EVENT_15:
		return "DISP";
	case CMDQ_EVENT_MDP_WROT1_SW_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_MDP_WROT0_SW_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_MDP_RDMA1_SW_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_MDP_RDMA0_SW_RST_DONE_ENG_EVENT:
		return "MDP";
	case CMDQ_EVENT_DISP_OVL0_SOF:
	case CMDQ_EVENT_DISP_OVL0_2L_SOF:
	case CMDQ_EVENT_DISP_RDMA0_SOF:
	case CMDQ_EVENT_DISP_RSZ0_SOF:
	case CMDQ_EVENT_DISP_COLOR0_SOF:
	case CMDQ_EVENT_DISP_CCORR0_SOF:
	case CMDQ_EVENT_DISP_CCORR1_SOF:
	case CMDQ_EVENT_DISP_AAL0_SOF:
	case CMDQ_EVENT_DISP_GAMMA0_SOF:
	case CMDQ_EVENT_DISP_POSTMASK0_SOF:
	case CMDQ_EVENT_DISP_DITHER0_SOF:
	case CMDQ_EVENT_DISP_CM0_SOF:
	case CMDQ_EVENT_DISP_SPR0_SOF:
	case CMDQ_EVENT_DISP_DSC_WRAP0_SOF:
	case CMDQ_EVENT_DSI0_SOF:
	case CMDQ_EVENT_DISP_WDMA0_SOF:
	case CMDQ_EVENT_DISP_PWM0_SOF:
	case CMDQ_EVENT_MUTEX_SOF_17:
	case CMDQ_EVENT_MUTEX_SOF_18:
	case CMDQ_EVENT_MUTEX_SOF_19:
	case CMDQ_EVENT_MUTEX_SOF_20:
	case CMDQ_EVENT_MUTEX_SOF_21:
	case CMDQ_EVENT_MUTEX_SOF_22:
	case CMDQ_EVENT_MUTEX_SOF_23:
	case CMDQ_EVENT_MUTEX_SOF_24:
	case CMDQ_EVENT_MUTEX_SOF_25:
	case CMDQ_EVENT_DSI0_FRAME_DONE:
	case CMDQ_EVENT_DISP_WDMA0_FRAME_DONE:
	case CMDQ_EVENT_DISP_SPR0_FRAME_DONE:
	case CMDQ_EVENT_DISP_RSZ0_FRAME_DONE:
	case CMDQ_EVENT_DISP_RDMA0_FRAME_DONE:
	case CMDQ_EVENT_DISP_POSTMASK0_FRAME_DONE:
	case CMDQ_EVENT_DISP_OVL0_FRAME_DONE:
	case CMDQ_EVENT_DISP_OVL0_2L_FRAME_DONE:
	case CMDQ_EVENT_DISP_GAMMA0_FRAME_DONE:
	case CMDQ_EVENT_DISP_DSC_WRAP0_CORE1_FRAME_DONE:
	case CMDQ_EVENT_DISP_DSC_WRAP0_CORE0_FRAME_DONE:
	case CMDQ_EVENT_DISP_DITHER0_FRAME_DONE:
	case CMDQ_EVENT_DISP_COLOR0_FRAME_DONE:
	case CMDQ_EVENT_DISP_CM0_FRAME_DONE:
	case CMDQ_EVENT_DISP_CCORR1_FRAME_DONE:
	case CMDQ_EVENT_DISP_CCORR0_FRAME_DONE:
	case CMDQ_EVENT_DISP_AAL0_FRAME_DONE:
		return "DISP";
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_0:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_1:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_2:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_3:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_4:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_5:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_6:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_7:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_8:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_9:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_10:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_11:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_12:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_13:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_14:
	case CMDQ_EVENT_MDP_STREAM_DONE_ENG_EVENT_15:
		return "MDP";
	case CMDQ_EVENT_DSI0_TE_ENG_EVENT:
	case CMDQ_EVENT_DSI0_IRQ_ENG_EVENT:
	case CMDQ_EVENT_DSI0_DONE_ENG_EVENT:
	case CMDQ_EVENT_DISP_WDMA0_SW_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_DISP_SMIASSERT_ENG_EVENT:
	case CMDQ_EVENT_DISP_POSTMASK0_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_DISP_OVL0_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_DISP_OVL0_2L_RST_DONE_ENG_EVENT:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_0:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_1:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_2:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_3:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_4:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_5:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_6:
	case CMDQ_EVENT_BUF_UNDERRUN_ENG_EVENT_7:
		return "DISP";
	default:
		return cmdq_thread_module_dispatch(gce_pa, thread);
	}
}
EXPORT_SYMBOL(cmdq_event_module_dispatch);

u32 cmdq_util_hw_id(u32 pa)
{
	return 0;
}

u32 cmdq_test_get_subsys_list(u32 **regs_out)
{
	static u32 regs[] = {
		0x14000100,	/* mmsys MMSYS_CG_CON0 */
		0x11f600a0,	/* msdc0 SW_DBG_SEL */
		0x1121004c,	/* audio AFE_I2S_CON3_OFFSET */
		0x110020bc,	/* uart0 */
	};

	*regs_out = regs;
	return ARRAY_SIZE(regs);
}

const char *cmdq_util_hw_name(void *chan)
{
	return "GCE";
}

bool cmdq_thread_ddr_user_check(const s32 thread)
{
	switch (thread) {
	case 0 ... 6:
	case 8 ... 9:
	case 15:
		return false;
	default:
		return true;
	}
}
EXPORT_SYMBOL(cmdq_thread_ddr_user_check);
