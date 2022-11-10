/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MACH_GENERIC_KMALLOC_H
#define __ASM_MACH_GENERIC_KMALLOC_H

#ifdef CONFIG_DMA_NONCOHERENT
/*
 * Set this one if any device in the system might do non-coherent DMA.
 */
#define ARCH_DMA_MINALIGN	L1_CACHE_BYTES
#endif

#endif /* __ASM_MACH_GENERIC_KMALLOC_H */
