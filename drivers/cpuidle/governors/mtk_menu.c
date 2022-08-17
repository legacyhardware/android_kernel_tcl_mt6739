#include <linux/kernel.h>
#include <linux/cpuidle.h>
#include <linux/pm_qos.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/sched/stat.h>
#include <linux/math64.h>
#include <linux/cpu.h>

#include <linux/fb.h>
#include <linux/notifier.h>
#include "mtk_menu.h"

static bool screen_on;
static DEFINE_SPINLOCK(mtk_menu_spin_lock);

bool __attribute__((weak)) system_idle_hint_result(void)
{
	return false;
}

bool __attribute__((weak)) is_all_cpu_idle_criteria(void)
{
	return false;
}

void __attribute__((weak)) mtk_idle_dump_cnt_in_interval(void)
{
}

void __attribute__((weak)) mcdi_heart_beat_log_dump(void)
{
}

void __attribute__((weak)) mtk_cpuidle_framework_init(void)
{
}

static int mtk_menu_fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int blank;
	unsigned long flags = 0;

	if (event != FB_EVENT_BLANK)
		return 0;

	blank = *(int *)evdata->data;

	switch (blank) {
	case FB_BLANK_UNBLANK:

		spin_lock_irqsave(&mtk_menu_spin_lock, flags);

		screen_on = true;

		spin_unlock_irqrestore(&mtk_menu_spin_lock, flags);

		break;
	case FB_BLANK_POWERDOWN:

		spin_lock_irqsave(&mtk_menu_spin_lock, flags);

		screen_on = false;

		spin_unlock_irqrestore(&mtk_menu_spin_lock, flags);

		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block mtk_menu_fb_notifier = {
		.notifier_call = mtk_menu_fb_notifier_callback,
};

static bool is_screen_on(void)
{
	bool result = false;
	unsigned long flags = 0;

	spin_lock_irqsave(&mtk_menu_spin_lock, flags);

	result = screen_on;

	spin_unlock_irqrestore(&mtk_menu_spin_lock, flags);

	return result;
}

#define BUCKETS 12
#define INTERVAL_SHIFT 3
#define INTERVALS (1UL << INTERVAL_SHIFT)
#define RESOLUTION 1024
#define DECAY 8
#define MAX_INTERESTING 50000

#define USE_CORRELATION_FACTOR
#define USE_TYPICAL_INTERVAL
#define USE_INTERACTIVITY_REQ


struct menu_device {
	int		last_state_idx;
	int             needs_update;
	int             tick_wakeup;

	unsigned int	next_timer_us;
	unsigned int	predicted_us;
	unsigned int	bucket;
	unsigned int	correction_factor[BUCKETS];
	unsigned int	intervals[INTERVALS];
	int		interval_ptr;
};


#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

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

#ifdef USE_TYPICAL_INTERVAL
static unsigned int get_typical_interval(struct menu_device *data)
{
	int i, divisor;
	unsigned int max, thresh, avg;
	uint64_t sum, variance;

	thresh = UINT_MAX; /* Discard outliers above this value */

again:

	/* First calculate the average of past intervals */
	max = 0;
	sum = 0;
	divisor = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];

		if (value <= thresh) {
			sum += value;
			divisor++;
			if (value > max)
				max = value;
		}
	}
	if (divisor == INTERVALS)
		avg = sum >> INTERVAL_SHIFT;
	else
		avg = div_u64(sum, divisor);

	/* Then try to determine variance */
	variance = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];

		if (value <= thresh) {
			int64_t diff = (int64_t)value - avg;

			variance += diff * diff;
		}
	}
	if (divisor == INTERVALS)
		variance >>= INTERVAL_SHIFT;
	else
		do_div(variance, divisor);

	/*
	 * The typical interval is obtained when standard deviation is
	 * small (stddev <= 20 us, variance <= 400 us^2) or standard
	 * deviation is small compared to the average interval (avg >
	 * 6*stddev, avg^2 > 36*variance). The average is smaller than
	 * UINT_MAX aka U32_MAX, so computing its square does not
	 * overflow a u64. We simply reject this candidate average if
	 * the standard deviation is greater than 715 s (which is
	 * rather unlikely).
	 *
	 * Use this result only if there is no timer to wake us up sooner.
	 */
	if (likely(variance <= U64_MAX / 36)) {
		if ((((u64)avg * avg > variance * 36)
			&& (divisor * 4 >= INTERVALS * 3))
			|| variance <= 400) {
			return avg;
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
		return UINT_MAX;

	thresh = max - 1;
	goto again;
}
#endif

static int menu_select(struct cpuidle_driver *drv, struct cpuidle_device *dev,
		       bool *stop_tick)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	struct device *device = get_cpu_device(dev->cpu);
	int latency_req = pm_qos_request(PM_QOS_CPU_DMA_LATENCY);
	int i;
	int first_idx;
	int idx;
#ifdef USE_INTERACTIVITY_REQ
	unsigned int interactivity_req;
#endif
#ifdef USE_TYPICAL_INTERVAL
	unsigned int expected_interval;
#endif
	unsigned long nr_iowaiters, cpu_load;
	int resume_latency = dev_pm_qos_raw_read_value(device);
	ktime_t delta_next;

	if (data->needs_update) {
		menu_update(drv, dev);
		data->needs_update = 0;
	}

	/* resume_latency is 0 means no restriction */
	if (resume_latency && resume_latency < latency_req)
		latency_req = resume_latency;

	/* Special case when user has set very strict latency requirement */
	if (unlikely(latency_req == 0)) {
		*stop_tick = false;
		return 0;
	}

	/* determine the expected residency time, round up */
	data->next_timer_us =
		ktime_to_us(tick_nohz_get_sleep_length(&delta_next));

	get_iowait_load(&nr_iowaiters, &cpu_load);
	data->bucket = which_bucket(data->next_timer_us, nr_iowaiters);

#ifdef USE_CORRELATION_FACTOR
	/*
	 * Force the result of multiplication to be 64 bits even if both
	 * operands are 32 bits.
	 * Make sure to round up for half microseconds.
	 */
	data->predicted_us =
		DIV_ROUND_CLOSEST_ULL((uint64_t)data->next_timer_us *
			data->correction_factor[data->bucket],
			RESOLUTION * DECAY);

	/* do NOT use correlation_factor if screen OFF */
	if (!is_screen_on() && system_idle_hint_result())
		data->predicted_us = data->next_timer_us;
#else
	data->predicted_us = data->next_timer_us;
#endif

#ifdef USE_TYPICAL_INTERVAL
	if (!is_all_cpu_idle_criteria()) {
		expected_interval = get_typical_interval(data);
		expected_interval = min(expected_interval, data->next_timer_us);
		/*
		 * Use the lowest expected idle interval to pick the idle state.
		 */
		data->predicted_us = min(data->predicted_us, expected_interval);
	}
#endif

	first_idx = 0;
	if (drv->states[0].flags & CPUIDLE_FLAG_POLLING) {
		struct cpuidle_state *s;
		unsigned int polling_threshold;

		s = &drv->states[1];
		/*
		 * We want to default to C1 (hlt), not to busy polling
		 * unless the timer is happening really really soon, or
		 * C1's exit latency exceeds the user configured limit.
		 */
		polling_threshold =
			max_t(unsigned int, 20, s->target_residency);
		if (data->next_timer_us > polling_threshold &&
		    latency_req > s->exit_latency && !s->disabled &&
		    !dev->states_usage[1].disable)
			first_idx = 1;
	}

	if (tick_nohz_tick_stopped()) {
		/*
		 * If the tick is already stopped, the cost of possible short
		 * idle duration misprediction is much higher, because the CPU
		 * may be stuck in a shallow idle state for a long time as a
		 * result of it.  In that case say we might mispredict and try
		 * to force the CPU into a state for which we would have stopped
		 * the tick, unless a timer is going to expire really soon
		 * anyway.
		 */
		if (data->predicted_us < TICK_USEC)
			data->predicted_us = min_t(unsigned int, TICK_USEC,
						   ktime_to_us(delta_next));
	} else {
		/*
		 * Use the performance multiplier and the user-configurable
		 * latency_req to determine the maximum exit latency.
		 */
		interactivity_req = data->predicted_us /
				performance_multiplier(nr_iowaiters, cpu_load);

#ifdef USE_INTERACTIVITY_REQ
		if (latency_req > interactivity_req)
			latency_req = interactivity_req;
#endif
	}

	expected_interval = data->predicted_us;
	/*
	 * Find the idle state with the lowest power while satisfying
	 * our constraints.
	 */
	idx = -1;
	for (i = first_idx; i < drv->state_count; i++) {
		struct cpuidle_state *s = &drv->states[i];
		struct cpuidle_state_usage *su = &dev->states_usage[i];

		if (s->disabled || su->disable)
			continue;
		if (idx == -1)
			idx = i; /* first enabled state */
		if (s->target_residency > data->predicted_us)
			break;
		if (s->exit_latency > latency_req) {
			/*
			 * If we break out of the loop for latency reasons, use
			 * the target residency of the selected state as the
			 * expected idle duration so that the tick is retained
			 * as long as that target residency is low enough.
			 */
			expected_interval = drv->states[idx].target_residency;
			break;
		}
		idx = i;
	}

	if (idx == -1)
		idx = 0; /* No states enabled. Must use 0. */

	/*
	 * Don't stop the tick if the selected state is a polling one or if the
	 * expected idle duration is shorter than the tick period length.
	 */
	if ((drv->states[idx].flags & CPUIDLE_FLAG_POLLING) ||
	    expected_interval < TICK_USEC) {
		unsigned int delta_next_us = ktime_to_us(delta_next);

		*stop_tick = false;

		if (!tick_nohz_tick_stopped() && idx > 0 &&
		    drv->states[idx].target_residency > delta_next_us) {
			/*
			 * The tick is not going to be stopped and the target
			 * residency of the state to be returned is not within
			 * the time until the next timer event including the
			 * tick, so try to correct that.
			 */
			for (i = idx - 1; i >= 0; i--) {
				if (drv->states[i].disabled ||
					dev->states_usage[i].disable)
					continue;

				idx = i;
				if (drv->states[i].target_residency <=
					delta_next_us)
					break;
			}
		}
	}

	data->last_state_idx = idx;

	return data->last_state_idx;
}

static void menu_reflect(struct cpuidle_device *dev, int index)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	data->last_state_idx = index;
	data->needs_update = 1;
	data->tick_wakeup = tick_nohz_idle_got_tick();

	mtk_idle_dump_cnt_in_interval();
	mcdi_heart_beat_log_dump();
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

	if (data->tick_wakeup && data->next_timer_us > TICK_USEC) {
		/*
		 * The nohz code said that there wouldn't be any events within
		 * the tick boundary (if the tick was stopped), but the idle
		 * duration predictor had a differing opinion.  Since the CPU
		 * was woken up by a tick (that wasn't stopped after all), the
		 * predictor was not quite right, so assume that the CPU could
		 * have been idle long (but not forever) to help the idle
		 * duration predictor do a better job next time.
		 */
		measured_us = 9 * MAX_INTERESTING / 10;
	} else {
		/* measured value */
		measured_us = cpuidle_get_last_residency(dev);

		/* Deduct exit latency */
		if (measured_us > 2 * target->exit_latency)
			measured_us -= target->exit_latency;
		else
			measured_us /= 2;
	}

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

unsigned int get_menu_predict_us(void)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	return data->predicted_us;
}

unsigned int get_menu_next_timer_us(void)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	return data->next_timer_us;
}

static struct cpuidle_governor menu_governor = {
	.name =		"mtk_menu",
	.rating =	100,
	.enable =	menu_enable_device,
	.select =	menu_select,
	.reflect =	menu_reflect,
};

static int __init init_menu(void)
{
	unsigned long flags = 0;
	int r;

	/* Register FB notifier */
	r = fb_register_client(&mtk_menu_fb_notifier);
	if (r) {
		pr_info("FAILED TO REGISTER FB CLIENT (%d)\n", r);
		return r;
	}

	spin_lock_irqsave(&mtk_menu_spin_lock, flags);

	screen_on = true;

	spin_unlock_irqrestore(&mtk_menu_spin_lock, flags);

	mtk_cpuidle_framework_init();

	return cpuidle_register_governor(&menu_governor);
}

postcore_initcall(init_menu);