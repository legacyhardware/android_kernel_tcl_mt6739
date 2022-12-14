
#undef TRACE_SYSTEM
#define TRACE_SYSTEM apu_power_events
#if !defined(_TRACE_APUSYS_FREQ_EVENTS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_APUSYS_FREQ_EVENTS_H
#include <linux/tracepoint.h>
#include "apu_power_api.h"
TRACE_EVENT(APUSYS_DFS,
	TP_PROTO(struct apu_power_info *pwr_info),
	TP_ARGS(pwr_info),
	TP_STRUCT__entry(
		__field(int, dsp_freq)
		__field(int, dsp1_freq)
		__field(int, dsp2_freq)
		__field(int, ipuif_freq)
	),
	TP_fast_assign(
		__entry->dsp_freq = pwr_info->dsp_freq;
		__entry->dsp1_freq = pwr_info->dsp1_freq;
		__entry->dsp2_freq = pwr_info->dsp2_freq;
		__entry->ipuif_freq = pwr_info->ipuif_freq;
	),
	TP_printk("conn=%d,vpu0=%d,vpu1=%d,ipuif=%d",
		  __entry->dsp_freq, __entry->dsp1_freq,
		  __entry->dsp2_freq,
		  __entry->ipuif_freq)
);

#endif /* _TRACE_APUSYS_FREQ_EVENTS_H */
/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE apu_power_events
#include <trace/define_trace.h>
