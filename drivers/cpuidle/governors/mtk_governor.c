
#include <linux/cpu.h>
#include <linux/cpuidle.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/pm_qos.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/time.h>

#undef INTEGRATE_WITH_MENU_GOV
#if defined(CONFIG_MACH_MT6799) || defined(CONFIG_MACH_MT6759) ||              \
	defined(CONFIG_MACH_MT6763) || defined(CONFIG_MACH_MT6758)
#define INTEGRATE_WITH_MENU_GOV
#endif

struct mtk_idle_device {
	unsigned int cpu;
	int last_state_idx;
};

static DEFINE_PER_CPU(struct mtk_idle_device, mtk_idle_devices);

int __attribute__((weak)) mtk_idle_select(int cpu)
{
	/* Default: CPUidle state select failed */
	return -1;
}

int __attribute__((weak))
mtk_idle_select_base_on_menu_gov(int cpu, int menu_select_state)
{
	/* Default: CPUidle state select failed */
	return -1;
}

void __attribute__((weak)) __init mtk_cpuidle_framework_init(void)
{
}

#ifdef INTEGRATE_WITH_MENU_GOV

#define BUCKETS 12
#define INTERVAL_SHIFT 3
#define INTERVALS (1UL << INTERVAL_SHIFT)
#define RESOLUTION 1024
#define DECAY 8
#define MAX_INTERESTING 50000


struct menu_device {
	int last_state_idx;
	int needs_update;

	unsigned int next_timer_us;
	unsigned int predicted_us;
	unsigned int bucket;
	unsigned int correction_factor[BUCKETS];
	unsigned int intervals[INTERVALS];
	int interval_ptr;
};

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)

static inline int get_loadavg(unsigned long load)
{
	return LOAD_INT(load) * 10 + LOAD_FRAC(load) / 10;
}

static inline int which_bucket(unsigned int duration,
			       unsigned long nr_iowaiters)
{
	int bucket = 0;

	/*
	 * We keep two groups of stats; one with no
	 * IO pending, one without.
	 * This allows us to calculate
	 * E(duration)|iowait
	 */
	if (nr_iowaiters)
		bucket = BUCKETS / 2;

	if (duration < 10)
		return bucket;
	if (duration < 100)
		return bucket + 1;
	if (duration < 1000)
		return bucket + 2;
	if (duration < 10000)
		return bucket + 3;
	if (duration < 100000)
		return bucket + 4;
	return bucket + 5;
}

static inline int performance_multiplier(unsigned long nr_iowaiters,
					 unsigned long load)
{
	int mult = 1;

	/* for higher loadavg, we are more reluctant */

	/*
	 * this doesn't work as intended - it is almost always 0, but can
	 * sometimes, depending on workload, spike very high into the hundreds
	 * even when the average cpu load is under 10%.
	 */
	/* mult += 2 * get_loadavg(); */

	/* for IO wait tasks (per cpu!) we add 5x each */
	mult += 10 * nr_iowaiters;

	return mult;
}

static DEFINE_PER_CPU(struct menu_device, menu_devices);

static void menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev);

static void get_typical_interval(struct menu_device *data)
{
	int i, divisor;
	unsigned int max, thresh;
	uint64_t avg, stddev;

	thresh = UINT_MAX; /* Discard outliers above this value */

again:

	/* First calculate the average of past intervals */
	max = 0;
	avg = 0;
	divisor = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];

		if (value <= thresh) {
			avg += value;
			divisor++;
			if (value > max)
				max = value;
		}
	}
	if (divisor == INTERVALS)
		avg >>= INTERVAL_SHIFT;
	else
		do_div(avg, divisor);

	/* Then try to determine standard deviation */
	stddev = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];

		if (value <= thresh) {
			int64_t diff = value - avg;

			stddev += diff * diff;
		}
	}
	if (divisor == INTERVALS)
		stddev >>= INTERVAL_SHIFT;
	else
		do_div(stddev, divisor);

	/*
	 * The typical interval is obtained when standard deviation is small
	 * or standard deviation is small compared to the average interval.
	 *
	 * int_sqrt() formal parameter type is unsigned long. When the
	 * greatest difference to an outlier exceeds ~65 ms * sqrt(divisor)
	 * the resulting squared standard deviation exceeds the input domain
	 * of int_sqrt on platforms where unsigned long is 32 bits in size.
	 * In such case reject the candidate average.
	 *
	 * Use this result only if there is no timer to wake us up sooner.
	 */
	if (likely(stddev <= ULONG_MAX)) {
		stddev = int_sqrt(stddev);
		if (((avg > stddev * 6) && (divisor * 4 >= INTERVALS * 3)) ||
		    stddev <= 20) {
			if (data->next_timer_us > avg)
				data->predicted_us = avg;
			return;
		}
	}

	/*
	 * If we have outliers to the upside in our distribution, discard
	 * those by setting the threshold to exclude these outliers, then
	 * calculate the average and standard deviation again. Once we get
	 * down to the bottom 3/4 of our samples, stop excluding samples.
	 *
	 * This can deal with workloads that have long pauses interspersed
	 * with sporadic activity with a bunch of short pauses.
	 */
	if ((divisor * 4) <= INTERVALS * 3)
		return;

	thresh = max - 1;
	goto again;
}

static int menu_select(struct cpuidle_driver *drv, struct cpuidle_device *dev)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	int latency_req = pm_qos_request(PM_QOS_CPU_DMA_LATENCY);
	int i;
	unsigned int interactivity_req;
	unsigned long nr_iowaiters, cpu_load;

	if (data->needs_update) {
		menu_update(drv, dev);
		data->needs_update = 0;
	}

	data->last_state_idx = CPUIDLE_DRIVER_STATE_START - 1;

	/* Special case when user has set very strict latency requirement */
	if (unlikely(latency_req == 0))
		return 0;

	/* determine the expected residency time, round up */
	data->next_timer_us = ktime_to_us(tick_nohz_get_sleep_length());

	get_iowait_load(&nr_iowaiters, &cpu_load);
	data->bucket = which_bucket(data->next_timer_us, nr_iowaiters);

	/*
	 * Force the result of multiplication to be 64 bits even if both
	 * operands are 32 bits.
	 * Make sure to round up for half microseconds.
	 */
	data->predicted_us = DIV_ROUND_CLOSEST_ULL(
		(uint64_t)data->next_timer_us *
			data->correction_factor[data->bucket],
		RESOLUTION * DECAY);

	get_typical_interval(data);

	/*
	 * Performance multiplier defines a minimum predicted idle
	 * duration / latency ratio. Adjust the latency limit if
	 * necessary.
	 */
	interactivity_req = data->predicted_us /
			    performance_multiplier(nr_iowaiters, cpu_load);
	if (latency_req > interactivity_req)
		latency_req = interactivity_req;

	/*
	 * We want to default to C1 (hlt), not to busy polling
	 * unless the timer is happening really really soon.
	 */
	if (data->next_timer_us > 5 &&
	    !drv->states[CPUIDLE_DRIVER_STATE_START].disabled &&
	    dev->states_usage[CPUIDLE_DRIVER_STATE_START].disable == 0)
		data->last_state_idx = CPUIDLE_DRIVER_STATE_START;

	/*
	 * Find the idle state with the lowest power while satisfying
	 * our constraints.
	 */
	for (i = CPUIDLE_DRIVER_STATE_START; i < drv->state_count; i++) {
		struct cpuidle_state *s = &drv->states[i];
		struct cpuidle_state_usage *su = &dev->states_usage[i];

		if (s->disabled || su->disable)
			continue;
		if (s->target_residency > data->predicted_us)
			continue;
		if (s->exit_latency > latency_req)
			continue;

		data->last_state_idx = i;
	}

	return data->last_state_idx;
}

static void menu_reflect(struct cpuidle_device *dev, int index)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	data->last_state_idx = index;
	data->needs_update = 1;
}

static void menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	int last_idx = data->last_state_idx;
	struct cpuidle_state *target = &drv->states[last_idx];
	unsigned int measured_us;
	unsigned int new_factor;

	/*
	 * Try to figure out how much time passed between entry to low
	 * power state and occurrence of the wakeup event.
	 *
	 * If the entered idle state didn't support residency measurements,
	 * we use them anyway if they are short, and if long,
	 * truncate to the whole expected time.
	 *
	 * Any measured amount of time will include the exit latency.
	 * Since we are interested in when the wakeup begun, not when it
	 * was completed, we must subtract the exit latency. However, if
	 * the measured amount of time is less than the exit latency,
	 * assume the state was never reached and the exit latency is 0.
	 */

	/* measured value */
	measured_us = cpuidle_get_last_residency(dev);

	/* Deduct exit latency */
	if (measured_us > target->exit_latency)
		measured_us -= target->exit_latency;

	/* Make sure our coefficients do not exceed unity */
	if (measured_us > data->next_timer_us)
		measured_us = data->next_timer_us;

	/* Update our correction ratio */
	new_factor = data->correction_factor[data->bucket];
	new_factor -= new_factor / DECAY;

	if (data->next_timer_us > 0 && measured_us < MAX_INTERESTING)
		new_factor += RESOLUTION * measured_us / data->next_timer_us;
	else
		/*
		 * we were idle so long that we count it as a perfect
		 * prediction
		 */
		new_factor += RESOLUTION;

	/*
	 * We don't want 0 as factor; we always want at least
	 * a tiny bit of estimated time. Fortunately, due to rounding,
	 * new_factor will stay nonzero regardless of measured_us values
	 * and the compiler can eliminate this test as long as DECAY > 1.
	 */
	if (DECAY == 1 && unlikely(new_factor == 0))
		new_factor = 1;

	data->correction_factor[data->bucket] = new_factor;

	/* update the repeating-pattern data */
	data->intervals[data->interval_ptr++] = measured_us;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;
}

static int menu_enable_device(struct cpuidle_driver *drv,
			      struct cpuidle_device *dev)
{
	struct menu_device *data = &per_cpu(menu_devices, dev->cpu);
	int i;

	memset(data, 0, sizeof(struct menu_device));

	/*
	 * if the correction factor is 0 (eg first time init or cpu hotplug
	 * etc), we actually want to start out with a unity factor.
	 */
	for (i = 0; i < BUCKETS; i++)
		data->correction_factor[i] = RESOLUTION * DECAY;

	return 0;
}

#endif /* INTEGRATE_WITH_MENU_GOV */

#ifdef INTEGRATE_WITH_MENU_GOV
static int mtk_governor_select(struct cpuidle_driver *drv,
			       struct cpuidle_device *dev)
{
	int select_state = 0;
	int menu_select_state = 0;
	int mtk_gov_select_state = 0;
	int cpu = smp_processor_id();

	/* Get result of MENU governor */
	menu_select_state = menu_select(drv, dev);

	mtk_gov_select_state =
		mtk_idle_select_base_on_menu_gov(cpu, menu_select_state);

	select_state = mtk_gov_select_state;

	return select_state;
}
#else
static int mtk_governor_select(struct cpuidle_driver *drv,
			       struct cpuidle_device *dev, bool *stop_tick)
{
	struct mtk_idle_device *data = this_cpu_ptr(&mtk_idle_devices);
	int state;

	state = mtk_idle_select(data->cpu);

	return state;
}
#endif

static void mtk_governor_reflect(struct cpuidle_device *dev, int index)
{
#ifdef INTEGRATE_WITH_MENU_GOV
	menu_reflect(dev, index);
#endif
}

static int mtk_governor_enable_device(struct cpuidle_driver *drv,
				      struct cpuidle_device *dev)
{
	struct mtk_idle_device *data = &per_cpu(mtk_idle_devices, dev->cpu);

	memset(data, 0, sizeof(struct mtk_idle_device));
	data->cpu = dev->cpu;

#ifdef INTEGRATE_WITH_MENU_GOV
	return menu_enable_device(drv, dev);
#else
	return 0;
#endif
}

static struct cpuidle_governor mtk_governor = {
	.name = "mtk_governor",
	.rating = 120,
	.enable = mtk_governor_enable_device,
	.select = mtk_governor_select,
	.reflect = mtk_governor_reflect,
};

static int __init init_mtk_governor(void)
{
/* TODO: check if debugfs_create_file() failed */
#if !defined(CONFIG_FPGA_EARLY_PORTING)
	mtk_cpuidle_framework_init();
#endif
	return cpuidle_register_governor(&mtk_governor);
}

MODULE_LICENSE("GPL");
postcore_initcall(init_mtk_governor);
