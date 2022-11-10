// SPDX-License-Identifier: GPL-2.0-only
/*
 * Suspend support specific for mips.
 *
 * Copyright (C) 2009 Lemote Inc.
 * Author: Hu Hongbing <huhb@lemote.com>
 *	   Wu Zhangjin <wuzhangjin@gmail.com>
 */
#include <asm/sections.h>
#include <asm/fpu.h>
#include <asm/dsp.h>
#include <asm/radiax.h>

static u32 saved_status;
struct pt_regs saved_regs;

void save_processor_state(void)
{
	saved_status = read_c0_status();

	if (is_fpu_owner())
		save_fp(current);
	if (cpu_has_dsp)
		save_dsp(current);
#ifdef CONFIG_CPU_HAS_RADIAX
	save_radiax(current);
#endif
}

void restore_processor_state(void)
{
	write_c0_status(saved_status);

	if (is_fpu_owner())
		restore_fp(current);
	if (cpu_has_dsp)
		restore_dsp(current);
#ifdef CONFIG_CPU_HAS_RADIAX
	restore_radiax(current);
#endif
}

int pfn_is_nosave(unsigned long pfn)
{
	unsigned long nosave_begin_pfn = PFN_DOWN(__pa(&__nosave_begin));
	unsigned long nosave_end_pfn = PFN_UP(__pa(&__nosave_end));

	return	(pfn >= nosave_begin_pfn) && (pfn < nosave_end_pfn);
}
