/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _DT_BINDINGS_GCE_MT6382_H
#define _DT_BINDINGS_GCE_MT6382_H

/* GCE hardware events */
#define CMDQ_BRIDGE_EVENT_DISP_DSC_WRAP0_CORE0_SOF		0
#define CMDQ_BRIDGE_EVENT_DISP_DSC_WRAP0_CORE1_SOF		1
#define CMDQ_BRIDGE_EVENT_DISP_RDMA0_SOF			2
#define CMDQ_BRIDGE_EVENT_DISP_DSI0_SOF				3
#define CMDQ_BRIDGE_EVENT_DISP_DSC_WRAP0_CORE0_FRAME_DONE	4
#define CMDQ_BRIDGE_EVENT_DISP_DSC_WRAP0_CORE1_FRAME_DONE	5
#define CMDQ_BRIDGE_EVENT_DISP_RDMA0_FRAME_DONE			6
#define CMDQ_BRIDGE_EVENT_DSI_FRAME_DONE			7
#define CMDQ_BRIDGE_EVENT_STREAM_DONE_ENG_EVENT_0		8
#define CMDQ_BRIDGE_EVENT_STREAM_DONE_ENG_EVENT_1		9
#define CMDQ_BRIDGE_EVENT_STREAM_DONE_ENG_EVENT_2		10
#define CMDQ_BRIDGE_EVENT_DSI0_TE_ENG_EVENT			11
#define CMDQ_BRIDGE_EVENT_DSI0_IRQ_ENG_EVENT			12
#define CMDQ_BRIDGE_EVENT_DSI0_DONE_ENG_EVENT			13
#define CMDQ_BRIDGE_EVENT_DSI0_TARGET_LINE_EVENT		14
#define CMDQ_BRIDGE_EVENT_DSI0_PHY_READY_EVENT			15
#define CMDQ_BRIDGE_EVENT_DISP_RDMA0_TARGET_LINE_ENG_EVENT	16
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_0		17
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_1		18
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_2		19
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_3		20
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_4		21
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_5		22
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_6		23
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_7		24
#define CMDQ_BRIDGE_EVENT_BUF_UNDERRUN_ENG_EVENT_8		25

#define CMDQ_BRIDGE_EVENT_GCE_EVENT_0				898
#define CMDQ_BRIDGE_EVENT_GCE_EVENT_1				899

#endif
