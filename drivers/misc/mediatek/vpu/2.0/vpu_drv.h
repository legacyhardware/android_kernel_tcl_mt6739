
#ifndef __VPU_DRV_H__
#define __VPU_DRV_H__

#include <linux/types.h>

#define VPU_MAX_NUM_PORTS 32
#define VPU_MAX_NUM_PROPS 32
#define VPU_MAX_NUM_CORES 3
extern unsigned int efuse_data;
extern struct ion_client *my_ion_client;

typedef uint8_t vpu_id_t;

/* the last byte of string must be '/0' */
//typedef char vpu_name_t[32];





/*---------------------------------------------------------------------------*/
/*  VPU Property                                                             */
/*---------------------------------------------------------------------------*/
enum vpu_prop_type {
	VPU_PROP_TYPE_CHAR,
	VPU_PROP_TYPE_INT32,
	VPU_PROP_TYPE_INT64,
	VPU_PROP_TYPE_FLOAT,
	VPU_PROP_TYPE_DOUBLE,
	VPU_NUM_PROP_TYPES
};

enum vpu_prop_access {
	VPU_PROP_ACCESS_RDONLY,
	VPU_PROP_ACCESS_RDWR
};

struct vpu_prop_desc {
	vpu_id_t id;
	uint8_t type;      /* the property's data type */
	uint8_t access;    /* directional data exchange */
	uint32_t offset;   /* offset = previous offset + previous size */
	uint32_t count;    /* size = sizeof(type) x count */
	char name[32];
};

/*---------------------------------------------------------------------------*/
/*  VPU Ports                                                                */
/*---------------------------------------------------------------------------*/
enum vpu_port_usage {
	VPU_PORT_USAGE_IMAGE,
	VPU_PORT_USAGE_DATA,
	VPU_NUM_PORT_USAGES
};

enum vpu_port_dir {
	VPU_PORT_DIR_IN,
	VPU_PORT_DIR_OUT,
	VPU_PORT_DIR_IN_OUT,
	VPU_NUM_PORT_DIRS
};

struct vpu_port {
	vpu_id_t id;
	uint8_t usage;
	uint8_t dir;
	char name[32];
};

/*---------------------------------------------------------------------------*/
/*  VPU Algo                                                                 */
/*---------------------------------------------------------------------------*/
struct vpu_algo {
	vpu_id_t id[VPU_MAX_NUM_CORES];
	uint8_t port_count;
	uint8_t info_desc_count;
	uint8_t sett_desc_count;
	uint32_t info_length;    /* the size of info data buffer */
	uint32_t sett_length;
	uint32_t bin_length;
	uint64_t info_ptr;       /* the pointer to info data buffer */
	uint64_t bin_ptr;     /* mva of algo bin, which is accessible by VPU */
	char name[32];
	struct vpu_prop_desc info_descs[VPU_MAX_NUM_PROPS];
	struct vpu_prop_desc sett_descs[VPU_MAX_NUM_PROPS];
	struct vpu_port ports[VPU_MAX_NUM_PORTS];
};

/*---------------------------------------------------------------------------*/
/*  VPU Register                                                             */
/*---------------------------------------------------------------------------*/
struct vpu_reg_value {
	uint32_t field;
	uint32_t value;
};

struct vpu_reg_values {
	uint8_t reg_count;
	struct vpu_reg_value *regs;
};


/*---------------------------------------------------------------------------*/
/*  VPU Power                                                                */
/*---------------------------------------------------------------------------*/

enum vpu_power_mode {
	VPU_POWER_MODE_DYNAMIC,
	VPU_POWER_MODE_ON,
};

enum vpu_power_opp {
	VPU_POWER_OPP_UNREQUEST = 0xFF,
};

struct vpu_power {
	uint8_t opp_step;
	uint8_t freq_step;
	uint32_t bw; /* unit: MByte/s */

	/* align with core index defined in user space header file */
	unsigned int core;
};


/*---------------------------------------------------------------------------*/
/*  VPU Plane                                                                */
/*---------------------------------------------------------------------------*/
struct vpu_plane {
	uint32_t stride;         /* if buffer type is image */
	uint32_t length;
	uint64_t ptr;            /* mva which is accessible by VPU */
};

enum vpu_buf_format {
	VPU_BUF_FORMAT_DATA,
	VPU_BUF_FORMAT_IMG_Y8,
	VPU_BUF_FORMAT_IMG_YV12,
	VPU_BUF_FORMAT_IMG_NV21,
	VPU_BUF_FORMAT_IMG_YUY2,
	VPU_BUF_FORMAT_IMPL_DEFINED = 0xFF,
};

struct vpu_buffer {
	vpu_id_t port_id;
	uint8_t format;
	uint8_t plane_count;
	uint32_t width;
	uint32_t height;
	struct vpu_plane planes[3];
};


enum vpu_req_status {
	VPU_REQ_STATUS_SUCCESS,
	VPU_REQ_STATUS_BUSY,
	VPU_REQ_STATUS_TIMEOUT,
	VPU_REQ_STATUS_INVALID,
	VPU_REQ_STATUS_FLUSH,
	VPU_REQ_STATUS_FAILURE,
};

struct vpu_request {
	/* to recognize the request is from which user */
	unsigned long *user_id;
	/* to recognize the request object id for unorder enque/deque
	 * procedure
	 */
	uint64_t request_id;
	/* core index that user want to run the request on */
	unsigned int requested_core;
	/* the final occupied core index for request,
	 * especially for request in common pool
	 */
	unsigned int occupied_core;
	vpu_id_t algo_id[VPU_MAX_NUM_CORES];
	int frame_magic; /* mapping for user space/kernel space */
	uint8_t status;
	uint8_t buffer_count;
	uint32_t sett_length;
	uint64_t sett_ptr;       /* pointer to the request setting */
	uint64_t priv;           /* reserved for user */
	struct vpu_buffer buffers[VPU_MAX_NUM_PORTS];
	/* driver usage only, fd in user space / ion handle in kernel */
	uint64_t buf_ion_infos[VPU_MAX_NUM_PORTS * 3];
	struct vpu_power power_param;
};

struct vpu_status {
	int vpu_core_index;
	bool vpu_core_available;
	int pool_list_size;
};

struct vpu_dev_debug_info {
	int dev_fd;
	char callername[32];
	pid_t open_pid;
	pid_t open_tgid;
};
#ifdef CONFIG_MTK_GZ_SUPPORT_SDSP
extern int mtee_sdsp_enable(u32 on);
#endif

/*---------------------------------------------------------------------------*/
/*  IOCTL Command                                                            */
/*---------------------------------------------------------------------------*/
#define VPU_MAGICNO                 'v'
#define VPU_IOCTL_SET_POWER         _IOW(VPU_MAGICNO,   0, int)
#define VPU_IOCTL_ENQUE_REQUEST     _IOW(VPU_MAGICNO,   1, int)
#define VPU_IOCTL_DEQUE_REQUEST     _IOWR(VPU_MAGICNO,  2, int)
#define VPU_IOCTL_FLUSH_REQUEST     _IOW(VPU_MAGICNO,   3, int)
#define VPU_IOCTL_GET_ALGO_INFO     _IOWR(VPU_MAGICNO,  4, int)
#define VPU_IOCTL_LOCK              _IOW(VPU_MAGICNO,   5, int)
#define VPU_IOCTL_UNLOCK            _IOW(VPU_MAGICNO,   6, int)
#define VPU_IOCTL_LOAD_ALG_TO_POOL  _IOW(VPU_MAGICNO,   7, int)
#define VPU_IOCTL_REG_WRITE         _IOW(VPU_MAGICNO,   8, int)
#define VPU_IOCTL_REG_READ          _IOWR(VPU_MAGICNO,  9, int)
#define VPU_IOCTL_GET_CORE_STATUS   _IOWR(VPU_MAGICNO,  10, int)
#define VPU_IOCTL_OPEN_DEV_NOTICE   _IOWR(VPU_MAGICNO,  11, int)
#define VPU_IOCTL_CLOSE_DEV_NOTICE  _IOWR(VPU_MAGICNO,  12, int)


#define VPU_IOCTL_SDSP_SEC_LOCK     _IOW(VPU_MAGICNO,   60, int)
#define VPU_IOCTL_SDSP_SEC_UNLOCK   _IOW(VPU_MAGICNO,   61, int)

#endif
