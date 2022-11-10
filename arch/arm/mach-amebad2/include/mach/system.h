/*
 * Realtek Semiconductor Corp.
 *
 * bsp/include/mach/system.h:
 *
 * Copyright 2012  Tony Wu (tonywu@realtek.com)
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <asm/proc-fns.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

extern void (*arch_reset)(char, const char *);

#endif /* _SYSTEM_H_ */
