/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Ralf Baechle
 *
 * Modified for RLX Linux for MIPS
 * Copyright (C) 2014 Tony Wu
 */
#ifndef __ASM_MACH_GENERIC_CPU_FEATURE_OVERRIDES_H
#define __ASM_MACH_GENERIC_CPU_FEATURE_OVERRIDES_H

#include <bspcpu.h>

#define cpu_has_tlb			1
#define cpu_has_mips16			1
#define cpu_has_mmips			0
#define cpu_has_tlbinv			0
#define cpu_has_segment			0
#define cpu_has_eva			0
#define cpu_has_ftlb			0
#define cpu_has_htw			0
#define cpu_has_rixiex			0
#define cpu_has_maar			0
#define cpu_has_xpa			0
#define cpu_has_fre			0
#define cpu_has_cdmm			0
#define cpu_has_bp_ghist		0

#ifdef CONFIG_CPU_HAS_SLEEP
#define cpu_has_sleep			1
#endif
#ifdef CONFIG_CPU_HAS_RADIAX
#define cpu_has_radiax			1
#endif
#ifdef CONFIG_CPU_HAS_CLS
#define cpu_has_cls			1
#endif

#ifdef CONFIG_CPU_RLX
#define cpu_has_3kex			1
#define cpu_has_4kex			0
#define cpu_has_3k_cache		1
#define cpu_has_4k_cache		0
#define cpu_has_6k_cache		0
#define cpu_has_8k_cache		0
#define cpu_has_tx39_cache		0
#define cpu_has_octeon_cache		0
#define cpu_has_divec			0
#define cpu_has_vce			0
#define cpu_has_prefetch		0
#define cpu_has_tr			0
#define cpu_has_mcheck			0
#define cpu_has_mips_2			0
#define cpu_has_mips_3			0
#define cpu_has_mips_4			0
#define cpu_has_mips_5			0
#define cpu_has_mips32r1		0
#define cpu_has_mips32r2		0
#define cpu_has_mips64r1		0
#define cpu_has_mips64r2		0
#define cpu_has_dc_aliases		0
#define cpu_has_pindexed_dcache		1
#define cpu_has_local_ebase		0
#define cpu_has_llsc			1
#define cpu_has_ic_fills_f_dc		0
#define cpu_has_vtag_icache		0
#define cpu_has_cache_cdex_p 		0
#define cpu_has_cache_cdex_s 		0
#else
#define cpu_has_3kex			0
#define cpu_has_4kex			1
#define cpu_has_3k_cache		0
#define cpu_has_4k_cache		1
#define cpu_has_6k_cache		0
#define cpu_has_8k_cache		0
#define cpu_has_tx39_cache		0
#define cpu_has_octeon_cache		0
#define cpu_has_divec			1
#define cpu_has_vce			0
#define cpu_has_prefetch		1
#define cpu_has_tr			1
#define cpu_has_mcheck			1
#define cpu_has_mips_2			0
#define cpu_has_mips_3			0
#define cpu_has_mips_4			0
#define cpu_has_mips_5			0
#define cpu_has_mips32r1		0
#define cpu_has_mips32r2		1
#define cpu_has_mips64r1		0
#define cpu_has_mips64r2		0
#ifdef CONFIG_CPU_HAS_AR7
#define cpu_has_dc_aliases		0
#define cpu_has_pindexed_dcache		1
#endif
#define cpu_has_local_ebase		1
#define cpu_has_llsc			1
#define cpu_has_ic_fills_f_dc		0
#define cpu_has_vtag_icache		0
#define cpu_has_cache_cdex_p 		0
#define cpu_has_cache_cdex_s 		0
#endif

#ifdef CONFIG_MIPS_FP_SUPPORT
#define cpu_has_fpu			1
#define raw_cpu_has_fpu			1
#define cpu_has_32fpr			1
#define cpu_has_nofpuex			0
#else
#define cpu_has_fpu			0
#define raw_cpu_has_fpu			0
#define cpu_has_32fpr			0
#define cpu_has_nofpuex			1
#endif

#ifdef CONFIG_CPU_HAS_DSP
#define cpu_has_dsp			1
#else
#define cpu_has_dsp			0
#endif

#ifdef CONFIG_CPU_HAS_DSP2
#define cpu_has_dsp2			1
#else
#define cpu_has_dsp2			0
#endif

#define cpu_has_ejtag			0
#define cpu_has_rixi			0
#define cpu_has_smartmips		0
#define cpu_has_mips3d			0
#define cpu_has_mdmx			0

#ifdef CONFIG_MIPS_MT
#define cpu_has_mipsmt			1
#else
#define cpu_has_mipsmt			0
#endif

#ifdef CONFIG_CPU_HAS_TLS
#define cpu_has_userlocal		1
#else
#define cpu_has_userlocal		0
#endif

#define cpu_has_counter			1
#ifdef CONFIG_HARDWARE_WATCHPOINTS
#define cpu_has_watch			1
#else
#define cpu_has_watch			0
#endif

#if defined CONFIG_CPU_MIPS74K || defined CONFIG_CPU_MIPS1074K
#  define cpu_has_mips_r2_exec_hazard	0
#endif

#define cpu_has_64bits			0
#define cpu_has_64bits_zero_reg		0
#define cpu_has_64bits_gp_regs		0
#define cpu_has_64bit_addresses		0
#define cpu_has_vz			0

#ifdef cpu_dcache_line
#define cpu_dcache_line_size()		cpu_dcache_line
#endif

#ifdef cpu_icache_line
#define cpu_icache_line_size()		cpu_icache_line
#endif

#ifdef cpu_scache_line
#define cpu_scache_line_size()		cpu_scache_line
#endif

#endif /* __ASM_MACH_GENERIC_CPU_FEATURE_OVERRIDES_H */
