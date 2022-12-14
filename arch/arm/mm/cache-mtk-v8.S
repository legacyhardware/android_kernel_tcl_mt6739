#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>
#include <asm/errno.h>
#include <asm/unwind.h>

#include "proc-macros.S"


ENTRY(__clean_dcache_user_area)
	uaccess_enable r2
	add	r1, r1, r0
	dcache_line_size r2, r3
	sub	r3, r2, #1
	bic	r0, r0, r3
#ifdef CONFIG_ARM_ERRATA_764369
	ALT_SMP(W(dsb))
	ALT_UP(W(nop))
#endif
1:
#if defined(CONFIG_ARM_ERRATA_855873) || defined(CONFIG_ARM_ERRATA_824069)
	mcr	p15, 0, r0, c7, c14, 1	@ clean and invalidate D / U line
#else
	mcr	p15, 0, r0, c7, c10, 1	@ clean D / U line
#endif
	add	r0, r0, r2
	cmp	r0, r1
	blo	1b
	dsb	st
	uaccess_disable r2
	ret	lr
ENDPROC(__clean_dcache_user_area)

ENTRY(__clean_dcache_area_poc)
	add	r1, r1, r0
	b	v7_dma_clean_range
ENDPROC(__clean_dcache_area_poc)

ENTRY(__flush_dcache_user_area)
	uaccess_enable r2
	add	r1, r1, r0
	dcache_line_size r2, r3
	sub	r3, r2, #1
	bic	r0, r0, r3
#ifdef CONFIG_ARM_ERRATA_764369
	ALT_SMP(W(dsb))
	ALT_UP(W(nop))
#endif
1:
	mcr	p15, 0, r0, c7, c14, 1	@ clean & invalidate D / U line
	add	r0, r0, r2
	cmp	r0, r1
	blo	1b
	dsb	st
	uaccess_disable r2
	ret	lr
ENDPROC(__flush_dcache_user_area)

ENTRY(__flush_dcache_area)
	add	r1, r1, r0
	b	v7_dma_flush_range
ENDPROC(__flush_dcache_area)

ENTRY(__inval_dcache_user_area)
	b	__flush_dcache_user_area
ENDPROC(__inval_dcache_user_area)

ENTRY(__inval_dcache_area)
	add	r1, r1, r0
	b	v7_dma_inv_range
ENDPROC(__inval_dcache_area)

