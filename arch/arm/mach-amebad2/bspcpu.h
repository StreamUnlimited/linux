/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspcpu.h
 *     bsp cpu and memory configuration file
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#ifndef _BSPCPU_H_
#define _BSPCPU_H_

/*
 * scache => L2 cache
 * dcache => D cache
 * icache => I cache
 */
#define cpu_mem_size        (256 << 20)

#define cpu_imem_size       0
#define cpu_dmem_size       0
#define cpu_smem_size       0

#endif /* _BSPCPU_H_ */
