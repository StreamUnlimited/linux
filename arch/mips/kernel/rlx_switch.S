/* SPDX-License-Identifier: GPL-2.0 */
/*
 * r2300_switch.S: R2300 specific task switching code.
 *
 * Copyright (C) 1994, 1995, 1996, 1999 by Ralf Baechle
 * Copyright (C) 1994, 1995, 1996 by Andreas Busse
 *
 * Multi-cpu abstraction and macros for easier reading:
 * Copyright (C) 1996 David S. Miller (davem@davemloft.net)
 *
 * Further modifications to make this work:
 * Copyright (c) 1998-2000 Harald Koerfgen
 *
 * Modified for RLX processors
 * Copyright (C) 2008-2012 Tony Wu (tonywu@realtek.com)
 */
#include <asm/asm.h>
#include <asm/cachectl.h>
#include <asm/export.h>
#include <asm/fpregdef.h>
#include <asm/mipsregs.h>
#include <asm/asm-offsets.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/thread_info.h>

#include <asm/asmmacro.h>

	.align	5

/*
 * task_struct *resume(task_struct *prev, task_struct *next,
 *		       struct thread_info *next_ti)
 */
LEAF(resume)
	mfc0	t1, CP0_STATUS
	sw	t1, THREAD_STATUS(a0)
	cpu_save_nonscratch a0
	sw	ra, THREAD_REG31(a0)

#if defined(CONFIG_CC_STACKPROTECTOR) && !defined(CONFIG_SMP)
	PTR_LA	t8, __stack_chk_guard
	LONG_L	t9, TASK_STACK_CANARY(a1)
	LONG_S	t9, 0(t8)
#endif

#ifdef CONFIG_CPU_HAS_RADIAX
	/*
	 * Save radiax registers
	 */
	radiax_save_regs a0 t0 t1 t2 t3 t4 t5 t6
#endif
	/*
	 * The order of restoring the registers takes care of the race
	 * updating $28, $29 and kernelsp without disabling ints.
	 */
	move	$28, a2
	cpu_restore_nonscratch a1

#ifdef CONFIG_CPU_HAS_RADIAX
	/*
	 * Restore radiax registers
	 */
	radiax_restore_regs a1 t0 t1 t2 t3 t4 t5 t6
#endif
	PTR_ADDU	t0, $28, _THREAD_SIZE - 32
	set_saved_sp	t0, t1, t2

	mfc0	t1, CP0_STATUS		/* Do we really need this? */
	li	a3, 0xff01
	and	t1, a3
	lw	a2, THREAD_STATUS(a1)
	nor	a3, $0, a3
	and	a2, a3
	or	a2, t1
	mtc0	a2, CP0_STATUS
	move	v0, a0
	jr	ra
	END(resume)

#ifdef CONFIG_CPU_HAS_RADIAX

LEAF(_save_radiax)
	radiax_save_regs a0 t0 t1 t2 t3 t4 t5 t6
	jr	ra
	END(_save_radiax)

LEAF(_restore_radiax)
	radiax_restore_regs a0 t0 t1 t2 t3 t4 t5 t6
	jr	ra
	END(_restore_radiax)


LEAF(_init_radiax)
	radiax_init_regs
	jr ra
	END(_init_radiax)

#endif
