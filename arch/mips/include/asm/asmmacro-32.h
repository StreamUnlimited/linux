/* SPDX-License-Identifier: GPL-2.0 */
/*
 * asmmacro.h: Assembler macros to make things easier to read.
 *
 * Copyright (C) 1996 David S. Miller (davem@davemloft.net)
 * Copyright (C) 1998, 1999, 2003 Ralf Baechle
 */
#ifndef _ASM_ASMMACRO_32_H
#define _ASM_ASMMACRO_32_H

#include <asm/asm-offsets.h>
#include <asm/regdef.h>
#include <asm/fpregdef.h>
#include <asm/mipsregs.h>

	.macro	fpu_save_single thread tmp=t0
	.set push
	SET_HARDFLOAT
	cfc1	\tmp,  fcr31
	s.d	$f0,  THREAD_FPR0(\thread)
	s.d	$f2,  THREAD_FPR2(\thread)
	s.d	$f4,  THREAD_FPR4(\thread)
	s.d	$f6,  THREAD_FPR6(\thread)
	s.d	$f8,  THREAD_FPR8(\thread)
	s.d	$f10, THREAD_FPR10(\thread)
	s.d	$f12, THREAD_FPR12(\thread)
	s.d	$f14, THREAD_FPR14(\thread)
	s.d	$f16, THREAD_FPR16(\thread)
	s.d	$f18, THREAD_FPR18(\thread)
	s.d	$f20, THREAD_FPR20(\thread)
	s.d	$f22, THREAD_FPR22(\thread)
	s.d	$f24, THREAD_FPR24(\thread)
	s.d	$f26, THREAD_FPR26(\thread)
	s.d	$f28, THREAD_FPR28(\thread)
	s.d	$f30, THREAD_FPR30(\thread)
	sw	\tmp, THREAD_FCR31(\thread)
	.set pop
	.endm

	.macro	fpu_restore_single thread tmp=t0
	.set push
	SET_HARDFLOAT
	lw	\tmp, THREAD_FCR31(\thread)
	l.d	$f0,  THREAD_FPR0(\thread)
	l.d	$f2,  THREAD_FPR2(\thread)
	l.d	$f4,  THREAD_FPR4(\thread)
	l.d	$f6,  THREAD_FPR6(\thread)
	l.d	$f8,  THREAD_FPR8(\thread)
	l.d	$f10, THREAD_FPR10(\thread)
	l.d	$f12, THREAD_FPR12(\thread)
	l.d	$f14, THREAD_FPR14(\thread)
	l.d	$f16, THREAD_FPR16(\thread)
	l.d	$f18, THREAD_FPR18(\thread)
	l.d	$f20, THREAD_FPR20(\thread)
	l.d	$f22, THREAD_FPR22(\thread)
	l.d	$f24, THREAD_FPR24(\thread)
	l.d	$f26, THREAD_FPR26(\thread)
	l.d	$f28, THREAD_FPR28(\thread)
	l.d	$f30, THREAD_FPR30(\thread)
	ctc1	\tmp, fcr31
	.set pop
	.endm

	.macro	cpu_save_nonscratch thread
	LONG_S	s0, THREAD_REG16(\thread)
	LONG_S	s1, THREAD_REG17(\thread)
	LONG_S	s2, THREAD_REG18(\thread)
	LONG_S	s3, THREAD_REG19(\thread)
	LONG_S	s4, THREAD_REG20(\thread)
	LONG_S	s5, THREAD_REG21(\thread)
	LONG_S	s6, THREAD_REG22(\thread)
	LONG_S	s7, THREAD_REG23(\thread)
	LONG_S	sp, THREAD_REG29(\thread)
	LONG_S	fp, THREAD_REG30(\thread)
	.endm

	.macro	cpu_restore_nonscratch thread
	LONG_L	s0, THREAD_REG16(\thread)
	LONG_L	s1, THREAD_REG17(\thread)
	LONG_L	s2, THREAD_REG18(\thread)
	LONG_L	s3, THREAD_REG19(\thread)
	LONG_L	s4, THREAD_REG20(\thread)
	LONG_L	s5, THREAD_REG21(\thread)
	LONG_L	s6, THREAD_REG22(\thread)
	LONG_L	s7, THREAD_REG23(\thread)
	LONG_L	sp, THREAD_REG29(\thread)
	LONG_L	fp, THREAD_REG30(\thread)
	LONG_L	ra, THREAD_REG31(\thread)
	.endm

#ifdef CONFIG_CPU_HAS_RADIAX
	.macro radiax_save_regs thread tmp0 tmp1 tmp2 tmp3 tmp4 tmp5 tmp6
	.set push
	.set noat
	mfru	\tmp3, $0
	mfru	\tmp4, $1
	mfru	\tmp5, $2
	sw	\tmp3, THREAD_CBS0(\thread)
	sw	\tmp4, THREAD_CBS1(\thread)
	sw	\tmp5, THREAD_CBS2(\thread)
	mfru	\tmp0, $4
	mfru	\tmp1, $5
	mfru	\tmp2, $6
	mfru	\tmp3, $16
	mfru	\tmp4, $17
	mfru	\tmp5, $18
	mfru	\tmp6, $24
	sw	\tmp0, THREAD_CBE0(\thread)
	sw	\tmp1, THREAD_CBE1(\thread)
	sw	\tmp2, THREAD_CBE2(\thread)
	sw	\tmp3, THREAD_LPS0(\thread)
	sw	\tmp4, THREAD_LPE0(\thread)
	sw	\tmp5, THREAD_LPC0(\thread)
	sw	\tmp6, THREAD_MMD(\thread)
	mfa	\tmp1, $1, 8
	mfa	\tmp3, $2, 8
	sw	\tmp1, THREAD_M0LH(\thread)
	sw	\tmp3, THREAD_M0HH(\thread)
	mfa	\tmp0, $5
	mfa	\tmp1, $5, 8
	mfa	\tmp2, $6
	mfa	\tmp3, $6, 8
	sw	\tmp0, THREAD_M1LL(\thread)
	sw	\tmp1, THREAD_M1LH(\thread)
	sw	\tmp2, THREAD_M1HL(\thread)
	sw	\tmp3, THREAD_M1HH(\thread)
	mfa	\tmp0, $9
	mfa	\tmp1, $9, 8
	mfa	\tmp2, $10
	mfa	\tmp3, $10, 8
	sw	\tmp0, THREAD_M2LL(\thread)
	sw	\tmp1, THREAD_M2LH(\thread)
	sw	\tmp2, THREAD_M2HL(\thread)
	sw	\tmp3, THREAD_M2HH(\thread)
	mfa	\tmp0, $13
	mfa	\tmp1, $13, 8
	mfa	\tmp2, $14
	mfa	\tmp3, $14, 8
	sw	\tmp0, THREAD_M3LL(\thread)
	sw	\tmp1, THREAD_M3LH(\thread)
	sw	\tmp2, THREAD_M3HL(\thread)
	sw	\tmp3, THREAD_M3HH(\thread)
	.set pop
	.endm

	.macro radiax_restore_regs thread tmp0 tmp1 tmp2 tmp3 tmp4 tmp5 tmp6
	.set push
	.set noat
	lw	\tmp3, THREAD_CBS0(\thread)
	lw	\tmp4, THREAD_CBS1(\thread)
	lw	\tmp5, THREAD_CBS2(\thread)
	mtru	\tmp3, $0
	mtru	\tmp4, $1
	mtru	\tmp5, $2
	lw	\tmp0, THREAD_CBE0(\thread)
	lw	\tmp1, THREAD_CBE1(\thread)
	lw	\tmp2, THREAD_CBE2(\thread)
	lw	\tmp3, THREAD_LPS0(\thread)
	lw	\tmp4, THREAD_LPE0(\thread)
	lw	\tmp5, THREAD_LPC0(\thread)
	lw	\tmp6, THREAD_MMD(\thread)
	mtru	\tmp0, $4
	mtru	\tmp1, $5
	mtru	\tmp2, $6
	mtru	\tmp3, $16
	mtru	\tmp4, $17
	mtru	\tmp5, $18
	mtru	\tmp6, $24
	lw	\tmp0, THREAD_M0LH(\thread)
	lw	\tmp1, THREAD_M0HH(\thread)
	mta2.g	\tmp0, $1
	mta2.g	\tmp1, $2
	lw	\tmp0, THREAD_M1LL(\thread)
	lw	\tmp1, THREAD_M1LH(\thread)
	lw	\tmp2, THREAD_M1HL(\thread)
	lw	\tmp3, THREAD_M1HH(\thread)
	mta2	\tmp0, $5
	mta2.g	\tmp1, $5
	mta2	\tmp2, $6
	mta2.g	\tmp3, $6
	lw	\tmp0, THREAD_M2LL(\thread)
	lw	\tmp1, THREAD_M2LH(\thread)
	lw	\tmp2, THREAD_M2HL(\thread)
	lw	\tmp3, THREAD_M2HH(\thread)
	mta2	\tmp0, $9
	mta2.g	\tmp1, $9
	mta2	\tmp2, $10
	mta2.g	\tmp3, $10
	lw	\tmp0, THREAD_M3LL(\thread)
	lw	\tmp1, THREAD_M3LH(\thread)
	lw	\tmp2, THREAD_M3HL(\thread)
	lw	\tmp3, THREAD_M3HH(\thread)
	mta2	\tmp0, $13
	mta2.g	\tmp1, $13
	mta2	\tmp2, $14
	mta2.g	\tmp3, $14
	.set pop
	.endm

	.macro radiax_init_regs
	.set push
	.set noat
	mtru	$0, $0
	mtru	$0, $1
	mtru	$0, $2
	mtru	$0, $4
	mtru	$0, $5
	mtru	$0, $6
	mtru	$0, $16
	mtru	$0, $17
	mtru	$0, $18
	mtru	$0, $24
	mta2	$0, $5
	mta2.g	$0, $5
	mta2	$0, $6
	mta2.g	$0, $6
	mta2	$0, $9
	mta2.g	$0, $9
	mta2	$0, $10
	mta2.g	$0, $10
	mta2	$0, $13
	mta2.g	$0, $13
	mta2	$0, $14
	mta2.g	$0, $14
	.set pop
	.endm

#endif

#endif /* _ASM_ASMMACRO_32_H */
