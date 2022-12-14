
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <asm/signal.h>
#include <mt-plat/sync_write.h>
#include <mtk_ram_console.h>

#include "backtrace.h"


void dump_backtrace_entry_ramconsole_print (unsigned long where,
	unsigned long from, unsigned long frame)
{
	char str_buf[256];
	int ret;

#ifdef CONFIG_KALLSYMS
	ret = snprintf(str_buf, sizeof(str_buf),
		"[<%08lx>] (%pS) from [<%08lx>] (%pS)\n",
			where, (void *)where, from, (void *)from);
	if (ret < 0)
		pr_info("Fail snprintf: %d\n", ret);
#else
	ret = snprintf(str_buf, sizeof(str_buf),
			"Function entered at [<%08lx>] from [<%08lx>]\n",
				where, from);
	if (ret < 0)
		pr_info("Fail snprintf: %d\n", ret);
#endif
	aee_sram_fiq_log(str_buf);
}

void dump_regs(const char *fmt,
	const char v1, const unsigned int reg,
		const unsigned int reg_val)
{
	char str_buf[256];
	int ret;

	ret = snprintf(str_buf, sizeof(str_buf), fmt, v1, reg, reg_val);

	if (ret < 0)
		pr_info("Fail snprintf in dump rg: %d\n", ret);

	aee_sram_fiq_log(str_buf);
}

static int verify_stack(unsigned long sp)
{
	if (sp < PAGE_OFFSET || (sp > (unsigned long)high_memory
		&& high_memory != NULL))
		return -EFAULT;

	return 0;
}

void aee_dump_backtrace(struct pt_regs *regs, struct task_struct *tsk)
{
	char str_buf[256];
	unsigned int fp, mode;
	int ok = 1;
	int ret;

	ret = snprintf(str_buf, sizeof(str_buf), "PC is 0x%lx, LR is 0x%lx\n",
			regs->ARM_pc, regs->ARM_lr);

	if (ret < 0)
		pr_info("Fail snprintf aee dump: %d\n", ret);

	aee_sram_fiq_log(str_buf);

	if (!tsk)
		tsk = current;

	if (regs) {
		fp = regs->ARM_fp;
		mode = processor_mode(regs);
	} else if (tsk != current) {
		fp = thread_saved_fp(tsk);
		mode = 0x10;
	} else {
		asm("mov %0, fp" : "=r" (fp) : : "cc");
		mode = 0x10;
	}

	if (!fp) {
		aee_sram_fiq_log("no frame pointer");
		ok = 0;
	} else if (verify_stack(fp)) {
		aee_sram_fiq_log("invalid frame pointer");
		ok = 0;
	} else if (fp < (unsigned long)end_of_stack(tsk))
		aee_sram_fiq_log("frame pointer underflow");
	aee_sram_fiq_log("\n");

	if (ok)
		c_backtrace_ramconsole_print(fp, mode);

}
