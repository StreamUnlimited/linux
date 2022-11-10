/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_TLB_H
#define __ASM_TLB_H

#include <asm/cpu-features.h>
#include <asm/mipsregs.h>

#if defined(CONFIG_CPU_R3000) || defined(CONFIG_CPU_TX39XX) || defined(CONFIG_CPU_RLX)
# define ENTRYHI_VPN_SHIFT	(PAGE_SHIFT)
# define ENTRYHI_VPN_MASK	(PAGE_MASK)
#else
# define ENTRYHI_VPN_SHIFT	(PAGE_SHIFT + 1)
# define ENTRYHI_VPN_MASK	(PAGE_MASK << 1)
#endif

#define _UNIQUE_ENTRYHI(base, idx)					\
		(((base) + ((idx) << ENTRYHI_VPN_SHIFT)) |		\
		 (cpu_has_tlbinv ? MIPS_ENTRYHI_EHINV : 0))
#define UNIQUE_ENTRYHI(idx)		_UNIQUE_ENTRYHI(CKSEG0, idx)
#define UNIQUE_GUEST_ENTRYHI(idx)	_UNIQUE_ENTRYHI(CKSEG1, idx)

static inline unsigned int num_wired_entries(void)
{
	unsigned int wired = read_c0_wired();

	if (cpu_has_mips_r6)
		wired &= MIPSR6_WIRED_WIRED;

	return wired;
}

#include <asm-generic/tlb.h>

#endif /* __ASM_TLB_H */
