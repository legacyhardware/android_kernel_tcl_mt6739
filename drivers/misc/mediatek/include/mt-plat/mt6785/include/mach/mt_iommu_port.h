
#ifndef __MT_IOMMU_PORT_H__
#define __MT_IOMMU_PORT_H__
//enable this option if M4U new design of multiple bank
//#define IOMMU_DESIGN_OF_BANK

struct mtk_iommu_port {
	char *name;
	unsigned m4u_id: 2;
	unsigned m4u_slave: 2;
	unsigned larb_id: 6;
	unsigned larb_port: 8;
	unsigned tf_id: 14;     /* 14 bits */
	bool enable_tf;
	mtk_iommu_fault_callback_t fault_fn;
	void *fault_data;
};

#define MTK_IOMMU_PORT_INIT(name, id, slave, larb, tf_larb, port)  {\
	name, id, slave, larb, port, (((tf_larb)<<7)|((port)<<2)), 1\
}

#define SLAVE_L0 (0)
#define SLAVE_L1 (0)
#define SLAVE_L2 (0)
#define SLAVE_L3 (0)
#define SLAVE_L4 (0)
#define SLAVE_L5 (0)
#define SLAVE_L6 (0)

/* MM_IOMMU AXI_ID */
#define TF_LARB_L0 (0)
#define TF_LARB_L1 (4)
#define TF_LARB_L2 (8)
#define TF_LARB_L3 (12)
#define TF_LARB_L4 (0)
#define TF_LARB_L5 (16)
#define TF_LARB_L6 (28)
#define TF_LARB_CCU (24)

int mtk_iommu_larb_port_count[] = {
	9, 14, 12, 19, 0,//0~4
	26, 31, 2, 2 //5~8
};

int mtk_iommu_larb_distance[] = {
	0, 9, 23, 35, -1, //0~4
	54, 80, 111, 113, //5~8
};

struct mtk_iommu_port iommu_port[] = {
	/* larb0 -MMSYS-9 */
	MTK_IOMMU_PORT_INIT("DISP_POSTMASK0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 0),
	MTK_IOMMU_PORT_INIT("DISP_OVL0_HDR", 0,
		SLAVE_L0, 0, TF_LARB_L0, 1),
	MTK_IOMMU_PORT_INIT("DISP_OVL1_HDR", 0,
		SLAVE_L0, 0, TF_LARB_L0, 2),
	MTK_IOMMU_PORT_INIT("DISP_OVL0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 3),
	MTK_IOMMU_PORT_INIT("DISP_OVL1", 0,
		SLAVE_L0, 0, TF_LARB_L0, 4),
	MTK_IOMMU_PORT_INIT("DISP_PVRIC0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 5),
	MTK_IOMMU_PORT_INIT("DISP_RDMA0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 6),
	MTK_IOMMU_PORT_INIT("DISP_WDMA0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 7),
	MTK_IOMMU_PORT_INIT("DISP_FAKE0", 0,
		SLAVE_L0, 0, TF_LARB_L0, 8),
	/*larb1-MMSYS-14*/
	MTK_IOMMU_PORT_INIT("DISP_OVL0_2L_HDR", 0,
		SLAVE_L1, 1, TF_LARB_L1, 0),
	MTK_IOMMU_PORT_INIT("DISP_OVL1_2L_HDR", 0,
		SLAVE_L1, 1, TF_LARB_L1, 1),
	MTK_IOMMU_PORT_INIT("DISP_OVL0_2L", 0,
		SLAVE_L1, 1, TF_LARB_L1, 2),
	MTK_IOMMU_PORT_INIT("DISP_OVL1_2L", 0,
		SLAVE_L1, 1, TF_LARB_L1, 3),
	MTK_IOMMU_PORT_INIT("DISP_RDMA1", 0,
		SLAVE_L1, 1, TF_LARB_L1, 4),
	MTK_IOMMU_PORT_INIT("MDP_PVRIC0", 0,
		SLAVE_L1, 1, TF_LARB_L1, 5),
	MTK_IOMMU_PORT_INIT("MDP_PVRIC1", 0,
		SLAVE_L1, 1, TF_LARB_L1, 6),
	MTK_IOMMU_PORT_INIT("MDP_RDMA0", 0,
		SLAVE_L1, 1, TF_LARB_L1, 7),
	MTK_IOMMU_PORT_INIT("MDP_RDMA1", 0,
		SLAVE_L1, 1, TF_LARB_L1, 8),
	MTK_IOMMU_PORT_INIT("MDP_WROT0_R", 0,
		SLAVE_L1, 1, TF_LARB_L1, 9),
	MTK_IOMMU_PORT_INIT("MDP_WROT0_W", 0,
		SLAVE_L1, 1, TF_LARB_L1, 10),
	MTK_IOMMU_PORT_INIT("MDP_WROT1_R", 0,
		SLAVE_L1, 1, TF_LARB_L1, 11),
	MTK_IOMMU_PORT_INIT("MDP_WROT1_W", 0,
		SLAVE_L1, 1, TF_LARB_L1, 12),
	MTK_IOMMU_PORT_INIT("DISP_FAKE1", 0,
		SLAVE_L1, 1, TF_LARB_L1, 13),
	/*larb2-VDEC-12*/
	MTK_IOMMU_PORT_INIT("VDEC_MC_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 0),
	MTK_IOMMU_PORT_INIT("VDEC_UFO_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 1),
	MTK_IOMMU_PORT_INIT("VDEC_PP_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 2),
	MTK_IOMMU_PORT_INIT("VDEC_PRED_RD_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 3),
	MTK_IOMMU_PORT_INIT("VDEC_PRED_WR_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 4),
	MTK_IOMMU_PORT_INIT("VDEC_PPWRAP_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 5),
	MTK_IOMMU_PORT_INIT("VDEC_TILE_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 6),
	MTK_IOMMU_PORT_INIT("VDEC_VLD_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 7),
	MTK_IOMMU_PORT_INIT("VDEC_VLD2_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 8),
	MTK_IOMMU_PORT_INIT("VDEC_AVC_MV_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 9),
	MTK_IOMMU_PORT_INIT("VDEC_UFO_ENC_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 10),
	MTK_IOMMU_PORT_INIT("VDEC_RG_CTRL_DMA_EXT", 0,
		SLAVE_L2, 2, TF_LARB_L2, 11),
	/*larb3-VENC-19*/
	MTK_IOMMU_PORT_INIT("VENC_RCPU", 0,
		SLAVE_L3, 3, TF_LARB_L3, 0),
	MTK_IOMMU_PORT_INIT("VENC_REC", 0,
		SLAVE_L3, 3, TF_LARB_L3, 1),
	MTK_IOMMU_PORT_INIT("VENC_BSDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 2),
	MTK_IOMMU_PORT_INIT("VENC_SV_COMV", 0,
		SLAVE_L3, 3, TF_LARB_L3, 3),
	MTK_IOMMU_PORT_INIT("VENC_RD_COMV", 0,
		SLAVE_L3, 3, TF_LARB_L3, 4),

	MTK_IOMMU_PORT_INIT("VENC_NBM_RDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 5),
	MTK_IOMMU_PORT_INIT("VENC_NBM_RDMA_LITE", 0,
		SLAVE_L3, 3, TF_LARB_L3, 6),
	MTK_IOMMU_PORT_INIT("JPGENC_Y_RDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 7),
	MTK_IOMMU_PORT_INIT("JPGENC_C_RDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 8),
	MTK_IOMMU_PORT_INIT("JPGENC_Q_TABLE", 0,
		SLAVE_L3, 3, TF_LARB_L3, 9),

	MTK_IOMMU_PORT_INIT("JPGENC_BSDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 10),
	MTK_IOMMU_PORT_INIT("JPGDEC_WDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 11),
	MTK_IOMMU_PORT_INIT("JPGDEC_BSDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 12),
	MTK_IOMMU_PORT_INIT("VENC_NBM_WDMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 13),
	MTK_IOMMU_PORT_INIT("VENC_NBM_WDMA_LITE", 0,
		SLAVE_L3, 3, TF_LARB_L3, 14),

	MTK_IOMMU_PORT_INIT("VENC_CUR_LUMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 15),
	MTK_IOMMU_PORT_INIT("VENC_CUR_CHROMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 16),
	MTK_IOMMU_PORT_INIT("VENC_REF_LUMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 17),
	MTK_IOMMU_PORT_INIT("VENC_REF_CHROMA", 0,
		SLAVE_L3, 3, TF_LARB_L3, 18),
	/*larb4-dummy*/

	/*larb5-IMG-26*/
	MTK_IOMMU_PORT_INIT("IMGI", 0,
		SLAVE_L5, 5, TF_LARB_L5, 0),
	MTK_IOMMU_PORT_INIT("IMG2O", 0,
		SLAVE_L5, 5, TF_LARB_L5, 1),
	MTK_IOMMU_PORT_INIT("IMG3O", 0,
		SLAVE_L5, 5, TF_LARB_L5, 2),
	MTK_IOMMU_PORT_INIT("VIPI", 0,
		SLAVE_L5, 5, TF_LARB_L5, 3),
	MTK_IOMMU_PORT_INIT("LCEI", 0,
		SLAVE_L5, 5, TF_LARB_L5, 4),
	MTK_IOMMU_PORT_INIT("SMXI", 0,
		SLAVE_L5, 5, TF_LARB_L5, 5),
	MTK_IOMMU_PORT_INIT("SMXO", 0,
		SLAVE_L5, 5, TF_LARB_L5, 6),
	MTK_IOMMU_PORT_INIT("WPE0_RDMA1", 0,
		SLAVE_L5, 5, TF_LARB_L5, 7),

	MTK_IOMMU_PORT_INIT("WPE0_RDMA0", 0,
		SLAVE_L5, 5, TF_LARB_L5, 8),
	MTK_IOMMU_PORT_INIT("WPE0_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 9),
	MTK_IOMMU_PORT_INIT("FDVT_RDB", 0,
		SLAVE_L5, 5, TF_LARB_L5, 10),
	MTK_IOMMU_PORT_INIT("FDVT_WRA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 11),
	MTK_IOMMU_PORT_INIT("FDVT_RDA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 12),
	MTK_IOMMU_PORT_INIT("WPE1_RDMA0", 0,
		SLAVE_L5, 5, TF_LARB_L5, 13),
	MTK_IOMMU_PORT_INIT("WPE1_RDMA1", 0,
		SLAVE_L5, 5, TF_LARB_L5, 14),
	MTK_IOMMU_PORT_INIT("WPE1_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 15),

	MTK_IOMMU_PORT_INIT("DPE_RDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 16),
	MTK_IOMMU_PORT_INIT("DPE_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 17),
	MTK_IOMMU_PORT_INIT("MFB_RDMA0", 0,
		SLAVE_L5, 5, TF_LARB_L5, 18),
	MTK_IOMMU_PORT_INIT("MFB_RDMA1", 0,
		SLAVE_L5, 5, TF_LARB_L5, 19),
	MTK_IOMMU_PORT_INIT("MFB_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 20),
	MTK_IOMMU_PORT_INIT("RSC_RDMA0", 0,
		SLAVE_L5, 5, TF_LARB_L5, 21),
	MTK_IOMMU_PORT_INIT("RSC_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 22),
	MTK_IOMMU_PORT_INIT("OWE_RDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 23),
	MTK_IOMMU_PORT_INIT("OWE_WDMA", 0,
		SLAVE_L5, 5, TF_LARB_L5, 24),
	MTK_IOMMU_PORT_INIT("FDVT_WRB", 0,
		SLAVE_L5, 5, TF_LARB_L5, 25),

	/*larb6-IMG-31 */
	MTK_IOMMU_PORT_INIT("IMGO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 0),
	MTK_IOMMU_PORT_INIT("RRZO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 1),
	MTK_IOMMU_PORT_INIT("AAO,", 0,
		SLAVE_L6, 6, TF_LARB_L6, 2),
	MTK_IOMMU_PORT_INIT("AFO,", 0,
		SLAVE_L6, 6, TF_LARB_L6, 3),
	MTK_IOMMU_PORT_INIT("LSCI_0", 0,
		SLAVE_L6, 6, TF_LARB_L6, 4),
	MTK_IOMMU_PORT_INIT("LSCI_1", 0,
		SLAVE_L6, 6, TF_LARB_L6, 5),
	MTK_IOMMU_PORT_INIT("PDO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 6),
	MTK_IOMMU_PORT_INIT("BPCI", 0,
		SLAVE_L6, 6, TF_LARB_L6, 7),

	MTK_IOMMU_PORT_INIT("LSCO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 8),
	MTK_IOMMU_PORT_INIT("RSSO_A", 0,
		SLAVE_L6, 6, TF_LARB_L6, 9),
	MTK_IOMMU_PORT_INIT("UFEO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 10),
	MTK_IOMMU_PORT_INIT("SOCO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 11),
	MTK_IOMMU_PORT_INIT("SOC1", 0,
		SLAVE_L6, 6, TF_LARB_L6, 12),
	MTK_IOMMU_PORT_INIT("SOC2", 0,
		SLAVE_L6, 6, TF_LARB_L6, 13),
	MTK_IOMMU_PORT_INIT("CCUI", 0,
		SLAVE_L6, 6, TF_LARB_L6, 14),
	MTK_IOMMU_PORT_INIT("CCUO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 15),
	MTK_IOMMU_PORT_INIT("RAWI_A", 0,
		SLAVE_L6, 6, TF_LARB_L6, 16),

	MTK_IOMMU_PORT_INIT("CCUG", 0,
		SLAVE_L6, 6, TF_LARB_L6, 17),
	MTK_IOMMU_PORT_INIT("PS0", 0,
		SLAVE_L6, 6, TF_LARB_L6, 18),
	MTK_IOMMU_PORT_INIT("AFO_1", 0,
		SLAVE_L6, 6, TF_LARB_L6, 19),
	MTK_IOMMU_PORT_INIT("LSCI_2", 0,
		SLAVE_L6, 6, TF_LARB_L6, 20),
	MTK_IOMMU_PORT_INIT("PDI", 0,
		SLAVE_L6, 6, TF_LARB_L6, 21),
	MTK_IOMMU_PORT_INIT("FLKO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 22),
	MTK_IOMMU_PORT_INIT("LMVO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 23),
	MTK_IOMMU_PORT_INIT("UFGO", 0,
		SLAVE_L6, 6, TF_LARB_L6, 24),
	MTK_IOMMU_PORT_INIT("SPARE", 0,
		SLAVE_L6, 6, TF_LARB_L6, 25),
	MTK_IOMMU_PORT_INIT("SPARE2", 0,
		SLAVE_L6, 6, TF_LARB_L6, 26),
	MTK_IOMMU_PORT_INIT("SPARE3", 0,
		SLAVE_L6, 6, TF_LARB_L6, 27),
	MTK_IOMMU_PORT_INIT("SPARE4", 0,
		SLAVE_L6, 6, TF_LARB_L6, 28),
	MTK_IOMMU_PORT_INIT("SPARE5", 0,
		SLAVE_L6, 6, TF_LARB_L6, 29),
	MTK_IOMMU_PORT_INIT("FAKE_ENGINE", 0,
		SLAVE_L6, 6, TF_LARB_L6, 30),

	/*SMI COMMON*/
	MTK_IOMMU_PORT_INIT("CCU0", 0, 0, 7, TF_LARB_CCU, 0),
	MTK_IOMMU_PORT_INIT("CCU1", 0, 0, 7, TF_LARB_CCU, 1),

	MTK_IOMMU_PORT_INIT("VPU_CODE", 1, 0, 8, 0, 0),
	MTK_IOMMU_PORT_INIT("VPU_DATA", 1, 0, 8, 0, 1),

	MTK_IOMMU_PORT_INIT("UNKNOWN", 0, 0, 0, 0, 0)
};

/* APU_IOMMU AXI_ID */
#define TF_IDMA_0   0x9
#define TF_CORE_0   0x1
#define TF_EDMA     0x5
#define TF_IDMA_1   0xB
#define TF_CORE_1   0x3
#define TF_EDMB     0x7
#define TF_CORE_2   0x0
#define TF_EDMC     0x2

#define MSK_IDMA_0   F_MSK(3, 0)
#define MSK_CORE_0   (F_MSK(3, 0) | F_MSK(9, 8))
#define MSK_EDMA     (F_MSK(2, 0) | F_BIT_SET(9))
#define MSK_IDMA_1   F_MSK(3, 0)
#define MSK_CORE_1   (F_MSK(3, 0) | F_MSK(9, 8))
#define MSK_EDMB     (F_MSK(2, 0) | F_BIT_SET(9))
#define MSK_CORE_2   (F_MSK(1, 0) | F_BIT_SET(9))
#define MSK_EDMC     (F_MSK(1, 0) | F_MSK(9, 8))

#define IOMMU_APU_AXI_PORT_NR (8)

static uint32_t vpu_axi_bus_id[IOMMU_APU_AXI_PORT_NR] = {
	TF_IDMA_0, TF_CORE_0, TF_EDMA, TF_IDMA_1,
	TF_CORE_1, TF_EDMB, TF_CORE_2, TF_EDMC};

static uint32_t vpu_axi_bus_mask[IOMMU_APU_AXI_PORT_NR] = {
	MSK_IDMA_0, MSK_CORE_0, MSK_EDMA, MSK_IDMA_1,
	MSK_CORE_1, MSK_EDMB, MSK_CORE_2, MSK_EDMC};

static char *vpu_axi_bus_name[IOMMU_APU_AXI_PORT_NR] = {
	"APU_IDMA0", "APU_CORE_0", "APU_EDMA",
	"APU_IDMA_1", "APU_CORE_1", "APU_EDMB",
	"APU_CORE_2", "APU_EDMC"};

#ifndef M4U_PORT_NR
#define M4U_PORT_NR (115)
#endif

#endif
