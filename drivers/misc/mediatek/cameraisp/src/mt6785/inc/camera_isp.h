
#ifndef _MT_ISP_H
#define _MT_ISP_H

#include <linux/ioctl.h>

#define TS_BOOT_T

#ifndef CONFIG_OF
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
#endif

#define KERNEL_LOG
#define ISR_LOG_ON

#define SIG_ERESTARTSYS 512
#define ISP_DEV_MAJOR_NUMBER    251
#define ISP_MAGIC               'k'

#define UNI_A_BASE_HW   0x1A003000
#define CAM_A_BASE_HW   0x1A004000
#define CAM_B_BASE_HW   0x1A006000
#define CAM_C_BASE_HW   0x1A008000
#define CAMSV_0_BASE_HW 0x1A050000
#define CAMSV_1_BASE_HW 0x1A051000
#define CAMSV_2_BASE_HW 0x1A052000
#define CAMSV_3_BASE_HW 0x1A053000
#define CAMSV_4_BASE_HW 0x1A054000
#define CAMSV_5_BASE_HW 0x1A055000
#define SENINF_BASE_HW  0x18040000
#define MIPI_RX_BASE_HW 0x10400000
#define GPIO_BASE_HW    0x102D0000

#define ISP_REG_RANGE           (PAGE_SIZE*2)

/* In order with the suquence of device nodes defined in dtsi */
/* in dtsi rule, one hw module should mapping to one node. */
enum ISP_DEV_NODE_ENUM {
	ISP_CAMSYS_CONFIG_IDX = 0,
	ISP_UNI_A_IDX, /* CAMTOP in .dtsi */
	ISP_CAM_A_IDX,
	ISP_CAM_B_IDX,
	ISP_CAM_C_IDX,
	ISP_CAMSV0_IDX,
	ISP_CAMSV1_IDX,
	ISP_CAMSV2_IDX,
	ISP_CAMSV3_IDX,
	ISP_CAMSV4_IDX,
	ISP_CAMSV5_IDX,
	ISP_DEV_NODE_NUM
};

/* defined if want to support multiple dequne and enque or camera 3.0 */
/* #define _rtbc_buf_que_2_0_ */
enum CAM_FrameST {
	CAM_FST_NORMAL = 0, CAM_FST_DROP_FRAME = 1, CAM_FST_LAST_WORKING_FRAME = 2,
};

enum ISP_IRQ_CLEAR_ENUM {
	ISP_IRQ_CLEAR_NONE, /* non-clear wait, clear after wait */
	ISP_IRQ_CLEAR_WAIT, /* clear wait, clear before and after wait */
	ISP_IRQ_CLEAR_STATUS, /* clear specific status only */
	ISP_IRQ_CLEAR_ALL /* clear all status */
};

enum ISP_IRQ_TYPE_ENUM {
	ISP_IRQ_TYPE_INT_CAM_A_ST,
	ISP_IRQ_TYPE_INT_CAM_B_ST,
	ISP_IRQ_TYPE_INT_CAM_C_ST,
	ISP_IRQ_TYPE_INT_CAMSV_0_ST,
	ISP_IRQ_TYPE_INT_CAMSV_1_ST,
	ISP_IRQ_TYPE_INT_CAMSV_2_ST,
	ISP_IRQ_TYPE_INT_CAMSV_3_ST,
	ISP_IRQ_TYPE_INT_CAMSV_4_ST,
	ISP_IRQ_TYPE_INT_CAMSV_5_ST,
	ISP_IRQ_TYPE_INT_UNI_A_ST,
	ISP_IRQ_TYPE_AMOUNT
};

enum ISP_ST_ENUM {
	SIGNAL_INT = 0, DMA_INT, ISP_IRQ_ST_AMOUNT
};

struct ISP_IRQ_TIME_STRUCT {
	unsigned int tLastSig_sec; /* time stamp of the latest occurring signal*/
	unsigned int tLastSig_usec; /* time stamp of the latest occurring signal*/
	unsigned int tMark2WaitSig_sec; /* time period from marking a signal to user try to wait and get the signal*/
	unsigned int tMark2WaitSig_usec; /* time period from marking a signal to user try to wait and get the signal*/
	unsigned int tLastSig2GetSig_sec; /* time period from latest signal to user try to wait and get the signal*/
	unsigned int tLastSig2GetSig_usec; /* time period from latest signal to user try to wait and get the signal*/
	int passedbySigcnt; /* the count for the signal passed by  */
};

struct ISP_WAIT_IRQ_ST {
	enum ISP_IRQ_CLEAR_ENUM Clear;
	enum ISP_ST_ENUM St_type;
	unsigned int Status; /*ref. enum:ENUM_CAM_INT / ENUM_CAM_DMA_INT ...etc in isp_drv_stddef.h*/
	int UserKey; /* user key for doing interrupt operation */
	unsigned int Timeout;
	struct ISP_IRQ_TIME_STRUCT TimeInfo;
};

struct ISP_WAIT_IRQ_STRUCT {
	enum ISP_IRQ_TYPE_ENUM Type;
	unsigned int bDumpReg;
	struct ISP_WAIT_IRQ_ST EventInfo;
};

struct ISP_REGISTER_USERKEY_STRUCT {
	int userKey;
	char userName[32]; /* this size must the same as the icamiopipe api - registerIrq(...) */
};

struct ISP_CLEAR_IRQ_ST {
	int UserKey; /* user key for doing interrupt operation */
	enum ISP_ST_ENUM St_type;
	unsigned int Status;
};

struct ISP_CLEAR_IRQ_STRUCT {
	enum ISP_IRQ_TYPE_ENUM Type;
	struct ISP_CLEAR_IRQ_ST EventInfo;
};

struct ISP_REG_STRUCT {
	unsigned int module; /*plz refer to ISP_DEV_NODE_ENUM */
	unsigned int Addr; /* register's addr */
	unsigned int Val; /* register's value */
};

struct ISP_REG_IO_STRUCT {
	struct ISP_REG_STRUCT *pData; /* pointer to ISP_REG_STRUCT */
	unsigned int Count; /* count */
};

#ifdef CONFIG_COMPAT
struct compat_ISP_REG_IO_STRUCT {
	compat_uptr_t pData;
	unsigned int Count; /* count */
};
#endif

enum ISP_DUMP_CMD {
	ISP_DUMP_TPIPEBUF_CMD = 0,
	ISP_DUMP_TUNINGBUF_CMD,
	ISP_DUMP_ISPVIRBUF_CMD,
	ISP_DUMP_CMDQVIRBUF_CMD
};


struct ISP_DUMP_BUFFER_STRUCT {
	unsigned int DumpCmd;
	unsigned int *pBuffer;
	unsigned int BytesofBufferSize;
};

enum ISP_MEMORY_INFO_CMD {
	ISP_MEMORY_INFO_TPIPE_CMD = 1,
	ISP_MEMORY_INFO_CMDQ_CMD
};

struct ISP_GET_DUMP_INFO_STRUCT {
	unsigned int extracmd;
	unsigned int imgi_baseaddr;
	unsigned int tdri_baseaddr;
	unsigned int dmgi_baseaddr;
};
#ifdef CONFIG_COMPAT
struct compat_ISP_DUMP_BUFFER_STRUCT {
	unsigned int DumpCmd;
	compat_uptr_t pBuffer;
	unsigned int BytesofBufferSize;
};

#endif

/* length of the two memory areas */
#define P1_DEQUE_CNT    1
#define RT_BUF_TBL_NPAGES 16
#define ISP_RT_BUF_SIZE 16
#define ISP_RT_CQ0C_BUF_SIZE (ISP_RT_BUF_SIZE)/* (ISP_RT_BUF_SIZE>>1) */
/* pass1 setting sync index */
#define ISP_REG_P1_CFG_IDX 0x4090
/* how many clk levels */
#define ISP_CLK_LEVEL_CNT 10

enum _isp_tg_enum_ {
	_cam_tg_ = 0,
	_cam_tg2_,
	_camsv_tg_,
	_camsv2_tg_,
	_cam_tg_max_
};


enum _isp_dma_enum_ {
	_imgi_ = 0,
	_imgbi_,
	_imgci_,
	_vipi_,
	_vip2i_,
	_vip3i_, /* 5 */
	_ufdi_,
	_lcei_,
	_dmgi_,
	_depi_,
	_img2o_, /* 10 */
	_img2bo_,
	_img3o_,
	_img3bo_,
	_img3co_,
	_mfbo_, /* 15 */
	_feo_,
	_wrot_,
	_wdma_,
	_jpeg_,
	_venc_stream_, /* 20 */
	_dip_max,
	_rt_dma_max_ = _dip_max,

	_imgo_ = 0,
	_rrzo_,
	_ufeo_,
	_aao_,
	_afo_,
	_lcso_, /* 5 */
	_pdo_,
	_lmvo_,
	_flko_,
	_rsso_,
	_pso_,  /* 10*/
	_ufgo_,
	_bpci_,/* Need to be the first port after dmao */
	_lsci_,
	_rawi_,
	_pdi_,
	_cam_max_,
	_dmao_max_ = _bpci_, /* For user space usage, it is easy to have the total numbers of dmao */
	_camsv_imgo_ = _imgo_,
	_camsv_max_,
};

/* for keep ion handle */
enum ISP_WRDMA_ENUM {
	_dma_cq0i_ = 0,/* 0168 */
	_dma_cq0i_vir, /* 0168 */
	_dma_cq1i_,    /* 0174 */
	_dma_cq2i_,    /* 0180 */
	_dma_cq3i_,    /* 018c */
	_dma_cq4i_,    /* 0198 *//*5*/
	_dma_cq5i_,    /* 01a4 */
	_dma_cq6i_,    /* 01b0 */
	_dma_cq7i_,    /* 01bc */
	_dma_cq8i_,    /* 01c8 */
	_dma_cq9i_,    /* 01d4 *//*10*/
	_dma_cq10i_,   /* 01e0 */
	_dma_cq11i_,   /* 01ec */
	_dma_cq12i_,   /* 01f8 */
	_dma_bpci_,    /* 0370 */
	_dma_caci_,    /* 03a0 *//*15*/
	_dma_lsci_,    /* 03d0 */
	_dma_pdi_,     /* 0d50 */
	_dma_imgo_,    /* 0220 */
	_dma_rrzo_,    /* 0250 */
	_dma_aao_,     /* 0280 *//*20*/
	_dma_afo_,     /* 02b0 */
	_dma_lcso_,    /* 02e0 */
	_dma_ufeo_,    /* 0310 */
	_dma_pdo_,     /* 0340 */
	_dma_eiso_,    /* 0220 *//*25*/
	_dma_flko_,    /* 0250 */
	_dma_rsso_,    /* 0280 */
	_dma_pso_,     /* 0D80 */
	_dma_ufgo_,    /* 1320 */
	_dma_imgo_fh_, /* 0c04 */
	_dma_rrzo_fh_, /* 0c08 *//*30*/
	_dma_aao_fh_,  /* 0c0c */
	_dma_afo_fh_,  /* 0c10 */
	_dma_lcso_fh_, /* 0c14 */
	_dma_ufeo_fh_, /* 0c18 */
	_dma_pdo_fh_,  /* 0c1c *//*35*/
	_dma_eiso_fh_,  /* 03C4 */
	_dma_flko_fh_, /* 03C8 */
	_dma_rsso_fh_, /* 03CC */
	_dma_pso_fh_,  /* 0E20 */
	_dma_ufgo_fh_,  /* 1430 */
	_dma_max_wr_
};

struct ISP_DEV_ION_NODE_STRUCT {
	unsigned int       devNode;
	enum ISP_WRDMA_ENUM     dmaPort;
	int                memID;
};

struct ISP_LARB_MMU_STRUCT {
	unsigned int LarbNum;
	unsigned int regOffset;
	unsigned int regVal;
};

struct ISP_RT_IMAGE_INFO_STRUCT {
	unsigned int w; /* tg size */
	unsigned int h;
	unsigned int xsize; /* dmao xsize */
	unsigned int stride;
	unsigned int fmt;
	unsigned int pxl_id;
	unsigned int wbn;
	unsigned int ob;
	unsigned int lsc;
	unsigned int rpg;
	unsigned int m_num_0;
	unsigned int frm_cnt;
	unsigned int bus_size;
};

struct ISP_RT_RRZ_INFO_STRUCT {
	unsigned int srcX; /* crop window start point */
	unsigned int srcY;
	unsigned int srcW; /* crop window size */
	unsigned int srcH;
	unsigned int dstW; /* rrz out size */
	unsigned int dstH;
};

struct ISP_RT_DMAO_CROPPING_STRUCT {
	unsigned int x; /* in pix */
	unsigned int y; /* in pix */
	unsigned int w; /* in byte */
	unsigned int h; /* in byte */
};

struct ISP_RT_BUF_INFO_STRUCT {
	unsigned int memID;
	unsigned int size;
	long long base_vAddr;
	unsigned int base_pAddr;
	unsigned int timeStampS;
	unsigned int timeStampUs;
	unsigned int bFilled;
	unsigned int bProcessRaw;
	struct ISP_RT_IMAGE_INFO_STRUCT image;
	struct ISP_RT_RRZ_INFO_STRUCT rrzInfo;
	struct ISP_RT_DMAO_CROPPING_STRUCT dmaoCrop; /* imgo */
	unsigned int bDequeued;
	signed int bufIdx; /* used for replace buffer */
};

struct ISP_DEQUE_BUF_INFO_STRUCT {
	unsigned int count;
	unsigned int sof_cnt; /* cnt for current sof */
	unsigned int img_cnt; /* cnt for mapping to which sof */
	/* support only deque 1 image at a time */
	/* ISP_RT_BUF_INFO_STRUCT  data[ISP_RT_BUF_SIZE]; */
	struct ISP_RT_BUF_INFO_STRUCT data[P1_DEQUE_CNT];
};

struct ISP_RT_RING_BUF_INFO_STRUCT {
	unsigned int start; /* current DMA accessing buffer */
	unsigned int total_count; /* total buffer number.Include Filled and empty */
	unsigned int empty_count; /* total empty buffer number include current DMA accessing buffer */
	unsigned int pre_empty_count; /* previous total empty buffer number include current DMA accessing buffer */
	unsigned int active;
	unsigned int read_idx;
	unsigned int img_cnt; /* cnt for mapping to which sof */
	struct ISP_RT_BUF_INFO_STRUCT data[ISP_RT_BUF_SIZE];
};

enum ISP_RT_BUF_CTRL_ENUM {
	ISP_RT_BUF_CTRL_DMA_EN, ISP_RT_BUF_CTRL_CLEAR, ISP_RT_BUF_CTRL_MAX
};

enum ISP_RTBC_STATE_ENUM {
	ISP_RTBC_STATE_INIT,
	ISP_RTBC_STATE_SOF,
	ISP_RTBC_STATE_DONE,
	ISP_RTBC_STATE_MAX
};

enum ISP_RTBC_BUF_STATE_ENUM {
	ISP_RTBC_BUF_EMPTY,
	ISP_RTBC_BUF_FILLED,
	ISP_RTBC_BUF_LOCKED,
};

enum ISP_RAW_TYPE_ENUM {
	ISP_RROCESSED_RAW,
	ISP_PURE_RAW,
};

struct ISP_RT_BUF_STRUCT {
	enum ISP_RTBC_STATE_ENUM state;
	unsigned long dropCnt;
	struct ISP_RT_RING_BUF_INFO_STRUCT ring_buf[_cam_max_];
};

struct ISP_BUFFER_CTRL_STRUCT {
	enum ISP_RT_BUF_CTRL_ENUM ctrl;
	enum ISP_IRQ_TYPE_ENUM module;
	enum _isp_dma_enum_ buf_id;
	/* unsigned int            data_ptr; */
	/* unsigned int            ex_data_ptr; exchanged buffer */
	struct ISP_RT_BUF_INFO_STRUCT *data_ptr;
	struct ISP_RT_BUF_INFO_STRUCT *ex_data_ptr; /* exchanged buffer */
	unsigned char *pExtend;
};

/* reference count */
#define _use_kernel_ref_cnt_

enum ISP_REF_CNT_CTRL_ENUM {
	ISP_REF_CNT_GET,
	ISP_REF_CNT_INC,
	ISP_REF_CNT_DEC,
	ISP_REF_CNT_DEC_AND_RESET_P1_P2_IF_LAST_ONE,
	ISP_REF_CNT_DEC_AND_RESET_P1_IF_LAST_ONE,
	ISP_REF_CNT_DEC_AND_RESET_P2_IF_LAST_ONE,
	ISP_REF_CNT_MAX
};

enum ISP_REF_CNT_ID_ENUM {
	ISP_REF_CNT_ID_IMEM,
	ISP_REF_CNT_ID_ISP_FUNC,
	ISP_REF_CNT_ID_GLOBAL_PIPE,
	ISP_REF_CNT_ID_P1_PIPE,
	ISP_REF_CNT_ID_P2_PIPE,
	ISP_REF_CNT_ID_MAX,
};

struct ISP_REF_CNT_CTRL_STRUCT {
	enum ISP_REF_CNT_CTRL_ENUM ctrl;
	enum ISP_REF_CNT_ID_ENUM id;
	signed int *data_ptr;
};

struct ISP_CLK_INFO {
	unsigned char clklevelcnt; /* how many clk levels */
	unsigned int clklevel[ISP_CLK_LEVEL_CNT]; /* Reocrd each clk level */
};

struct ISP_GET_CLK_INFO {
	unsigned int curClk;
	unsigned int targetClk;
};

struct ISP_BW {
	unsigned int peak;
	unsigned int avg;
};

struct ISP_PM_QOS_INFO_STRUCT {
	unsigned int    module;
	struct ISP_BW   port_bw[_cam_max_];
};

struct ISP_SV_PM_QOS_INFO_STRUCT {
	unsigned int    module;
	struct ISP_BW   port_bw[_camsv_max_];
};

struct ISP_MULTI_RAW_CONFIG {
	unsigned char HWmodule;
	unsigned char master_module;
	unsigned char slave_cam_num;
	unsigned char twin_module;
	uintptr_t        cq_base_pAddr;
};

struct ISP_RAW_INT_STATUS {
	unsigned int ispIntErr;
	unsigned int ispInt3Err;
};


#define _rtbc_use_cq0c_

#define _MAGIC_NUM_ERR_HANDLING_

#ifdef CONFIG_COMPAT


struct compat_ISP_BUFFER_CTRL_STRUCT {
	enum ISP_RT_BUF_CTRL_ENUM ctrl;
	enum ISP_IRQ_TYPE_ENUM module;
	enum _isp_dma_enum_ buf_id;
	compat_uptr_t data_ptr;
	compat_uptr_t ex_data_ptr; /* exchanged buffer */
	compat_uptr_t pExtend;
};

struct compat_ISP_REF_CNT_CTRL_STRUCT {
	enum ISP_REF_CNT_CTRL_ENUM ctrl;
	enum ISP_REF_CNT_ID_ENUM id;
	compat_uptr_t data_ptr;
};

#endif



enum ISP_CMD_ENUM {
	ISP_CMD_RESET_BY_HWMODULE,
	ISP_CMD_READ_REG, /* Read register from driver */
	ISP_CMD_WRITE_REG, /* Write register to driver */
	ISP_CMD_WAIT_IRQ, /* Wait IRQ */
	ISP_CMD_CLEAR_IRQ, /* Clear IRQ */
	ISP_CMD_RT_BUF_CTRL, /* for pass buffer control */
	ISP_CMD_REF_CNT, /* get imem reference count */
	ISP_CMD_DEBUG_FLAG, /* Dump message level */
	ISP_CMD_DUMP_ISR_LOG, /* dump isr log */
	ISP_CMD_GET_CUR_SOF,
	ISP_CMD_GET_DMA_ERR,
	ISP_CMD_GET_INT_ERR,
	ISP_CMD_GET_DROP_FRAME, /* dump current frame informaiton, 1 for drop frmae, 2 for last working frame */
	ISP_CMD_WAKELOCK_CTRL,
	ISP_CMD_REGISTER_IRQ_USER_KEY, /* register for a user key to do irq operation */
	ISP_CMD_MARK_IRQ_REQUEST, /* mark for a specific register before wait for the interrupt if needed */
	ISP_CMD_GET_MARK2QUERY_TIME, /* query time information between read and mark */
	ISP_CMD_FLUSH_IRQ_REQUEST, /* flush signal */
	ISP_CMD_GET_START_TIME,
	ISP_CMD_DFS_CTRL, /* turn on/off camsys pmqos */
	ISP_CMD_DFS_UPDATE, /* Update clock at run time */
	ISP_CMD_GET_SUPPORTED_ISP_CLOCKS, /* Get supported isp clocks on current platform */
	ISP_CMD_GET_CUR_ISP_CLOCK, /* Get cur isp clock level */
	ISP_CMD_GET_GLOBAL_TIME, /* Get the global time */
	ISP_CMD_VF_LOG, /* dbg only, prt log on kernel when vf_en is driven */
	ISP_CMD_GET_VSYNC_CNT,
	ISP_CMD_RESET_VSYNC_CNT,
	ISP_CMD_ION_IMPORT, /* get ion handle */
	ISP_CMD_ION_FREE,  /* free ion handle */
	ISP_CMD_CQ_SW_PATCH,  /* sim cq update behavior as atomic behavior */
	ISP_CMD_ION_FREE_BY_HWMODULE,  /* free all ion handle */
	ISP_CMD_LARB_MMU_CTL, /* toggle mmu config for smi larb ports of isp */
	ISP_CMD_DUMP_BUFFER,
	ISP_CMD_GET_DUMP_INFO,
	ISP_CMD_SET_MEM_INFO,
	ISP_CMD_RESET_PM_QOS,
	SV_CMD_RESET_PM_QOS,
	ISP_CMD_SET_PM_QOS_INFO,
	SV_CMD_SET_PM_QOS_INFO,
	ISP_CMD_SET_SEC_DAPC_REG,
	ISP_CMD_NOTE_CQTHR0_BASE,
	ISP_CMD_SET_VIR_CQCNT
};

enum ISP_HALT_DMA_ENUM {
	ISP_HALT_DMA_IMGO = 0,
	ISP_HALT_DMA_RRZO,
	ISP_HALT_DMA_AAO,
	ISP_HALT_DMA_AFO,
	ISP_HALT_DMA_LSCI,
	ISP_HALT_DMA_RSSO,
	ISP_HALT_DMA_CAMSV0,
	ISP_HALT_DMA_CAMSV1,
	ISP_HALT_DMA_CAMSV2,
	ISP_HALT_DMA_LSCO,
	ISP_HALT_DMA_UFEO,
	ISP_HALT_DMA_BPCI,
	ISP_HALT_DMA_PDO,
	ISP_HALT_DMA_RAWI,
	ISP_HALT_DMA_AMOUNT
};


/* reset ioctl */
#define ISP_RESET_BY_HWMODULE    _IOW(ISP_MAGIC, ISP_CMD_RESET_BY_HWMODULE, unsigned long)

/* read phy reg  */
#define ISP_READ_REGISTER        _IOWR(ISP_MAGIC, ISP_CMD_READ_REG, struct ISP_REG_IO_STRUCT)

/* write phy reg */
#define ISP_WRITE_REGISTER       _IOWR(ISP_MAGIC, ISP_CMD_WRITE_REG, struct ISP_REG_IO_STRUCT)

#define ISP_WAIT_IRQ        _IOW(ISP_MAGIC, ISP_CMD_WAIT_IRQ,      struct ISP_WAIT_IRQ_STRUCT)
#define ISP_CLEAR_IRQ       _IOW(ISP_MAGIC, ISP_CMD_CLEAR_IRQ,     struct ISP_CLEAR_IRQ_STRUCT)
#define ISP_BUFFER_CTRL     _IOWR(ISP_MAGIC, ISP_CMD_RT_BUF_CTRL,   struct ISP_BUFFER_CTRL_STRUCT)
#define ISP_DEBUG_FLAG      _IOW(ISP_MAGIC, ISP_CMD_DEBUG_FLAG,    unsigned char*)
#define ISP_GET_CUR_SOF     _IOWR(ISP_MAGIC, ISP_CMD_GET_CUR_SOF,          unsigned char*)
#define ISP_GET_DMA_ERR     _IOWR(ISP_MAGIC, ISP_CMD_GET_DMA_ERR,          unsigned char*)
#define ISP_GET_INT_ERR     _IOR(ISP_MAGIC, ISP_CMD_GET_INT_ERR,        struct ISP_RAW_INT_STATUS)
#define ISP_GET_DROP_FRAME  _IOWR(ISP_MAGIC, ISP_CMD_GET_DROP_FRAME,    unsigned long)
#define ISP_GET_START_TIME  _IOWR(ISP_MAGIC, ISP_CMD_GET_START_TIME,    unsigned char*)
#define ISP_DFS_CTRL              _IOWR(ISP_MAGIC, ISP_CMD_DFS_CTRL, unsigned int)
#define ISP_DFS_UPDATE              _IOWR(ISP_MAGIC, ISP_CMD_DFS_UPDATE, unsigned int)
#define ISP_GET_SUPPORTED_ISP_CLOCKS   _IOWR(ISP_MAGIC, ISP_CMD_GET_SUPPORTED_ISP_CLOCKS, struct ISP_CLK_INFO)
#define ISP_GET_CUR_ISP_CLOCK   _IOWR(ISP_MAGIC, ISP_CMD_GET_CUR_ISP_CLOCK, struct ISP_GET_CLK_INFO)
#define ISP_GET_GLOBAL_TIME   _IOWR(ISP_MAGIC, ISP_CMD_GET_GLOBAL_TIME, unsigned long long)
#define ISP_NOTE_CQTHR0_BASE                      \
	_IOWR(ISP_MAGIC, ISP_CMD_NOTE_CQTHR0_BASE, unsigned int*)
#define ISP_RESET_PM_QOS \
	_IOWR(ISP_MAGIC, ISP_CMD_RESET_PM_QOS, unsigned int)
#define SV_RESET_PM_QOS \
	_IOWR(ISP_MAGIC, SV_CMD_RESET_PM_QOS, unsigned int)
#define ISP_SET_PM_QOS_INFO \
	_IOWR(ISP_MAGIC, ISP_CMD_SET_PM_QOS_INFO, \
		struct ISP_PM_QOS_INFO_STRUCT)
#define SV_SET_PM_QOS_INFO \
	_IOWR(ISP_MAGIC, SV_CMD_SET_PM_QOS_INFO, \
		struct ISP_SV_PM_QOS_INFO_STRUCT)

#define ISP_REGISTER_IRQ_USER_KEY   _IOR(ISP_MAGIC, ISP_CMD_REGISTER_IRQ_USER_KEY, struct ISP_REGISTER_USERKEY_STRUCT)

#define ISP_FLUSH_IRQ_REQUEST       _IOW(ISP_MAGIC, ISP_CMD_FLUSH_IRQ_REQUEST, struct ISP_WAIT_IRQ_STRUCT)

#define ISP_WAKELOCK_CTRL           _IOWR(ISP_MAGIC, ISP_CMD_WAKELOCK_CTRL, unsigned long)
#define ISP_VF_LOG                  _IOW(ISP_MAGIC, ISP_CMD_VF_LOG,         unsigned char*)
#define ISP_GET_VSYNC_CNT           _IOWR(ISP_MAGIC, ISP_CMD_GET_VSYNC_CNT,      unsigned int)
#define ISP_RESET_VSYNC_CNT         _IOW(ISP_MAGIC, ISP_CMD_RESET_VSYNC_CNT,      unsigned int)
#define ISP_ION_IMPORT              _IOW(ISP_MAGIC, ISP_CMD_ION_IMPORT, struct ISP_DEV_ION_NODE_STRUCT)
#define ISP_ION_FREE                _IOW(ISP_MAGIC, ISP_CMD_ION_FREE,   struct ISP_DEV_ION_NODE_STRUCT)
#define ISP_ION_FREE_BY_HWMODULE    _IOW(ISP_MAGIC, ISP_CMD_ION_FREE_BY_HWMODULE, unsigned int)
#define ISP_CQ_SW_PATCH             _IOW(ISP_MAGIC, ISP_CMD_CQ_SW_PATCH, struct ISP_MULTI_RAW_CONFIG)
#define ISP_LARB_MMU_CTL            _IOW(ISP_MAGIC, ISP_CMD_LARB_MMU_CTL, struct ISP_LARB_MMU_STRUCT)
#define ISP_DUMP_BUFFER            _IOWR(ISP_MAGIC, ISP_CMD_DUMP_BUFFER, struct ISP_DUMP_BUFFER_STRUCT)
#define ISP_GET_DUMP_INFO          _IOWR(ISP_MAGIC, ISP_CMD_GET_DUMP_INFO, struct ISP_GET_DUMP_INFO_STRUCT)

#define ISP_SET_SEC_DAPC_REG        _IOW(ISP_MAGIC, ISP_CMD_SET_SEC_DAPC_REG, unsigned int)

#define ISP_SET_VIR_CQCNT \
	_IOWR(ISP_MAGIC, ISP_CMD_SET_VIR_CQCNT, unsigned int*)

#ifdef CONFIG_COMPAT
#define COMPAT_ISP_READ_REGISTER    _IOWR(ISP_MAGIC, ISP_CMD_READ_REG,      struct compat_ISP_REG_IO_STRUCT)
#define COMPAT_ISP_WRITE_REGISTER   _IOWR(ISP_MAGIC, ISP_CMD_WRITE_REG,     struct compat_ISP_REG_IO_STRUCT)
#define COMPAT_ISP_DEBUG_FLAG      _IOW(ISP_MAGIC, ISP_CMD_DEBUG_FLAG,     compat_uptr_t)
#define COMPAT_ISP_GET_DMA_ERR     _IOWR(ISP_MAGIC, ISP_CMD_GET_DMA_ERR,   compat_uptr_t)

#define COMPAT_ISP_BUFFER_CTRL     _IOWR(ISP_MAGIC, ISP_CMD_RT_BUF_CTRL,    struct compat_ISP_BUFFER_CTRL_STRUCT)
#define COMPAT_ISP_REF_CNT_CTRL    _IOWR(ISP_MAGIC, ISP_CMD_REF_CNT,        struct compat_ISP_REF_CNT_CTRL_STRUCT)
#define COMPAT_ISP_GET_START_TIME  _IOWR(ISP_MAGIC, ISP_CMD_GET_START_TIME, compat_uptr_t)

#define COMPAT_ISP_WAKELOCK_CTRL    _IOWR(ISP_MAGIC, ISP_CMD_WAKELOCK_CTRL, compat_uptr_t)
#define COMPAT_ISP_GET_DROP_FRAME   _IOWR(ISP_MAGIC, ISP_CMD_GET_DROP_FRAME, compat_uptr_t)
#define COMPAT_ISP_GET_CUR_SOF      _IOWR(ISP_MAGIC, ISP_CMD_GET_CUR_SOF,   compat_uptr_t)
#define COMPAT_ISP_RESET_BY_HWMODULE _IOW(ISP_MAGIC, ISP_CMD_RESET_BY_HWMODULE, compat_uptr_t)
#define COMPAT_ISP_VF_LOG           _IOW(ISP_MAGIC, ISP_CMD_VF_LOG,         compat_uptr_t)

#define COMPAT_ISP_DUMP_BUFFER      _IOWR(ISP_MAGIC, ISP_CMD_DUMP_BUFFER, struct compat_ISP_DUMP_BUFFER_STRUCT)

#endif

#endif

