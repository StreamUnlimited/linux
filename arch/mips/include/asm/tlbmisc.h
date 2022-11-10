/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_TLBMISC_H
#define __ASM_TLBMISC_H

/*
 * - add_wired_entry() add a fixed TLB entry, and move wired register
 */
#ifndef CONFIG_CPU_RLX
extern void add_wired_entry(unsigned long entrylo0, unsigned long entrylo1,
	unsigned long entryhi, unsigned long pagemask);
#else
extern void add_wired_entry(unsigned long entrylo, unsigned long entryhi);
#endif

#endif /* __ASM_TLBMISC_H */
