
#ifndef __CCU_CMN_H__
#define __CCU_CMN_H__

#include <linux/wait.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include "ccu_drv.h"

//#define CCU_LDVT
#define CCU_I2C_DMA_BUF_SIZE (4*PAGE_SIZE)

/* Common Structure */
enum ccu_req_type_e {
	CCU_IRQ_TYPE_XXX,
	CCU_IRQ_NUM_TYPES
};

struct ccu_device_s {
	struct proc_dir_entry *proc_dir;
	struct device *dev;
	struct dentry *debug_root;
	unsigned long ccu_base;
	unsigned long camsys_base;
	unsigned long bin_base;
	unsigned long dmem_base;
	unsigned long n3d_a_base;
	unsigned int irq_num;
	struct mutex dev_mutex;
	struct mutex user_mutex;
	struct mutex clk_mutex;
	struct mutex ion_client_mutex;
	u8 *i2c_dma_vaddr;
	dma_addr_t i2c_dma_paddr;
	uint32_t i2c_dma_mva;
	/* list of vlist_type(ccu_user_t) */
	struct list_head user_list;
	/* notify enque thread */
	wait_queue_head_t cmd_wait;
};

struct ccu_user_s {
	pid_t open_pid;
	pid_t open_tgid;
	uint32_t id;
	/* to enque/deque must have mutex protection */
	struct mutex data_mutex;

	bool running;
	bool flush;
	/* list of vlist_type(ccu_cmd_st) */
	struct list_head enque_ccu_cmd_list;
	struct list_head deque_ccu_cmd_list;
	wait_queue_head_t deque_wait;
};

/*type must be struct*/
/*#define DECLARE_VLIST(type) \*/
/*typedef struct { \*/
		/*type node; \*/
		/*struct list_head link; \*/
/*} type ## _list*/

#define vlist_node_of(ptr, type) ({ \
		const struct list_head *__mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type ## _list, link)); })

#define vlist_link(ptr, type) (&((type ## _list *)ptr)->link)

#define vlist_type(type) type ## _list

#define vlist_node(ptr, type)  ((type *) ptr)


/*typedef struct work_struct work_struct_t;*/

/*DECLARE_VLIST(work_struct_t);*/
/*DECLARE_VLIST(struct ccu_user_s);*/
struct ccu_user_s_list {
	struct ccu_user_s node;
	struct list_head link;
};

/*DECLARE_VLIST(struct ccu_cmd_s);*/
struct ccu_cmd_s_list {
	struct ccu_cmd_s node;
	struct list_head link;
};

/* ================= define in ccu_hw.c  =================== */

int ccu_init_hw(struct ccu_device_s *device);

int ccu_uninit_hw(struct ccu_device_s *device);

int ccu_mmap_hw(struct file *filp, struct vm_area_struct *vma);

int ccu_send_command(struct ccu_cmd_s *pCmd);

int ccu_power(struct ccu_power_s *power);

int ccu_force_powerdown(void);

int ccu_run(void);

int ccu_waitirq(struct CCU_WAIT_IRQ_STRUCT *WaitIrq);
int ccu_AFwaitirq(struct CCU_WAIT_IRQ_STRUCT *WaitIrq, int sensoridx);

int ccu_flushLog(int argc, int *argv);



int ccu_memcpy(void *dest, void *src, int length);

int ccu_memclr(void *dest, int length);

int ccu_read_info_reg(int regNo);

int ccu_query_power_status(void);


/* ================ define in ccu_drv.c  ================ */

int ccu_create_user(struct ccu_user_s **ruser);

int ccu_delete_user(struct ccu_user_s *user);

int ccu_lock_user_mutex(void);

int ccu_unlock_user_mutex(void);

int ccu_lock_ion_client_mutex(void);

int ccu_unlock_ion_client_mutex(void);

int ccu_push_command_to_queue(struct ccu_user_s *user,
	struct ccu_cmd_s *cmd);


int ccu_pop_command_from_queue(struct ccu_user_s *user,
	struct ccu_cmd_s **rcmd);


int ccu_flush_commands_from_queue(struct ccu_user_s *user);

int ccu_clock_enable(void);

void ccu_clock_disable(void);

/* LOG & AEE */
#define CCU_TAG "[ccu]"

#define LOG_DBG_MUST(format, args...) \
	pr_debug(CCU_TAG "[%s] " format, __func__, ##args)
#define LOG_INF_MUST(format, args...) \
	pr_info(CCU_TAG "[%s] " format, __func__, ##args)
#ifdef CCU_LDVT
#define LOG_DBG(format, args...) \
	pr_info(CCU_TAG "[%s] " format, __func__, ##args)
#else
#define LOG_DBG(format, args...)
#endif
#ifdef CCU_LDVT
#define LOG_INF(format, args...) \
	pr_info(CCU_TAG "[%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif
#define LOG_WARN(format, args...) \
	pr##_##warn(CCU_TAG "[%s] " format, __func__, ##args)
#define LOG_ERR(format, args...) \
	pr##_##err(CCU_TAG "[%s] " format, __func__, ##args)
#define LOG_DERR(device, format, args...) \
	dev##_##err(device, CCU_TAG "[%s] " format, __func__, ##args)

#define ccu_print_seq(seq_file, fmt, args...) \
	do {\
		if (seq_file)\
			seq_printf(seq_file, fmt, ##args);\
		else\
			pr_debug(fmt, ##args);\
	} while (0)

#define ccu_error(format, args...) \
	do {\
		LOG_ERR(CCU_TAG " error:"format, ##args);  \
		aee_kernel_exception("CCU", "[CCU] error:"format, ##args);  \
	} while (0)

#define ccu_aee(format, args...) \
	do {\
		char ccu_name[100];\
		snprintf(ccu_name, 100, CCU_TAG format, ##args); \
		aee_kernel_warning_api(__FILE__, __LINE__, \
		DB_OPT_MMPROFILE_BUFFER | DB_OPT_DUMP_DISPLAY, \
		ccu_name, CCU_TAG "error" format, ##args); \
		LOG_ERR(CCU_TAG " error:" format, ##args);  \
	} while (0)
#endif
