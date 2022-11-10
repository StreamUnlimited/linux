/*
 * Realtek Semiconductor Corp.
 *
 * bsp/include/mach/clkdev.h:
 *
 * Copyright 2012  Tony Wu (tonywu@realtek.com)
 */

#ifndef _CLKDEV_H_
#define _CLKDEV_H_

#include <mach/hardware.h>

static inline int __clk_get(struct clk *clk)
{
	return 1;
}

static inline void __clk_put(struct clk *clk)
{
}

#endif /* _CLKDEV_H_ */
