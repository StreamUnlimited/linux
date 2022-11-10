/*
 * Realtek Semiconductor Corp.
 *
 * bsp/include/mach/entry-macro.S
 *
 * Copyright 2012  Tony Wu (tonywu@realtek.com)
 */

#if defined(CONFIG_ARM_GIC)
#include <mach/entry-macro-gic.S>
#else
#include <mach/entry-macro-vic.S>
#endif
