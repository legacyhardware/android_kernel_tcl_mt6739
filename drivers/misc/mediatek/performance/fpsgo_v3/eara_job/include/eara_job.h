#ifndef __EARA_JOB_H__
#define __EARA_JOB_H__

#define MAX_DEVICE 2
#if defined(CONFIG_MTK_EARA_AI)
void fpsgo_ctrl2eara_nn_job_collect(int pid, int tid, unsigned long long mid,
	int hw_type, int num_step, __s32 *boost,
	__s32 *device, __u64 *exec_time);
int fpsgo_ctrl2eara_get_nn_priority(unsigned int pid,
	unsigned long long mid);
void fpsgo_ctrl2eara_get_nn_ttime(unsigned int pid,
	unsigned long long mid, int num_step, __u64 *time);
void fpsgo_fstb2eara_notify_fps_bound(void);
void fpsgo_fstb2eara_notify_fps_active(int active);
void fpsgo_fstb2eara_get_exec_time(int pid,
	unsigned long long mid, unsigned long long *t_v,
	unsigned long long *t_m);
void fpsgo_fstb2eara_get_boost_value(int pid, unsigned long long mid,
	int *b_v, int *b_m);
void fpsgo_fstb2eara_optimize_power(unsigned long long mid,
	int tgid, long long *t_c_time, long long t_t_t,
	long long c_time, long long v_time, long long m_time,
	unsigned int c_cap, unsigned int v_cap, unsigned int m_cap);
void fpsgo_fstb2eara_get_jobs_status(int *vpu_cross,
		int *mdla_cross, int *vpu_bg, int *mdla_bg);
#else
static inline void fpsgo_ctrl2eara_nn_job_collect(
	int pid, int tid, unsigned long long mid,
	int hw_type, int num_step, __s32 *boost,
	__s32 *device, __u64 *exec_time) { }
static inline int fpsgo_ctrl2eara_get_nn_priority(unsigned int pid,
	unsigned long long mid) { return 0; }
static inline void fpsgo_ctrl2eara_get_nn_ttime(unsigned int pid,
	unsigned long long mid, int num_step, __u64 *time) { }
static inline void fpsgo_fstb2eara_notify_fps_bound(void) { }
static inline void fpsgo_fstb2eara_notify_fps_active(int active) { }
static inline void fpsgo_fstb2eara_get_exec_time(int pid,
	unsigned long long mid, unsigned long long *t_v,
	unsigned long long *t_m) {
	*t_v = 0;
	*t_m = 0;
}
static inline void fpsgo_fstb2eara_get_boost_value(
	int pid, unsigned long long mid,
	int *b_v, int *b_m) {
	*b_v = 0;
	*b_m = 0;
}

static inline void fpsgo_fstb2eara_optimize_power(unsigned long long mid,
	int tgid, long long *t_c_time, long long t_t_t,
	long long c_time, long long v_time, long long m_time,
	unsigned int c_cap, unsigned int v_cap, unsigned int m_cap) {
	*t_c_time = t_t_t;
}
static inline void fpsgo_fstb2eara_get_jobs_status(int *vpu_cross,
		int *mdla_cross, int *vpu_bg, int *mdla_bg){
	*vpu_cross = 0;
	*mdla_cross = 0;
	*vpu_bg = 0;
	*mdla_bg = 0;
}
#endif
#endif

