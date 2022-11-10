/*
 * Realtek Semiconductor Corp.
 *
 * arch/mips/kernel/taroko-mmcr.c
 *     MMCR initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/version.h>
#include <linux/smp.h>
#include <linux/interrupt.h>

#include <asm/taroko-mmcr.h>

void mmcr_init_core(unsigned int core)
{
	/*
	 * CORE_STATE
	 *
	 * bit 0 => online
	 * bit 1 => coherent
	 */
	MMCR_REG32p(CORESTATE, core * 4) = 3;
}
