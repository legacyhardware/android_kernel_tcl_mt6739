
#if !defined(AEE_COMMON_H)
#define AEE_COMMON_H
#include <linux/console.h>

#ifdef CONFIG_MTK_RAM_CONSOLE
extern void ram_console_write(struct console *console, const char *s,
				unsigned int count);
#endif

extern int aee_rr_reboot_reason_show(struct seq_file *m, void *v);
extern int aee_rr_last_fiq_step(void);
extern void aee_rr_rec_exp_type(unsigned int type);

extern int debug_locks;
#ifdef WDT_DEBUG_VERBOSE
extern int dump_localtimer_info(char *buffer, int size);
extern int dump_idle_info(char *buffer, int size);
#endif
#ifdef CONFIG_SMP
extern void dump_log_idle(void);
extern void irq_raise_softirq(const struct cpumask *mask, unsigned int irq);
#endif

/* for test case only */
extern int no_zap_locks;

#endif				/* AEE_COMMON_H */
