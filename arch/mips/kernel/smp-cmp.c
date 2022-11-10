// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2007 MIPS Technologies, Inc.
 *    Chris Dearman (chris@mips.com)
 */

#undef DEBUG

#include <linux/kernel.h>
#include <linux/sched/task.h>
#include <linux/sched/task_stack.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/compiler.h>

#include <linux/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/smp.h>
#include <asm/smp-boot.h>
#include <asm/time.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/mips_mt.h>
#include <asm/smp-boot.h>
#include <asm/taroko-mmcr.h>

static void cmp_init_secondary(void)
{
	struct cpuinfo_mips *c __maybe_unused = &current_cpu_data;
	extern char handle_vec __maybe_unused;

	/* Enable per-cpu interrupts: platform specific */
	if (IS_ENABLED(CONFIG_CPU_RLX))
		write_lxc0_intvec(&handle_vec);

	/* Call platform init_secondary hook */
	plat_smp_init_secondary();
}

static void cmp_smp_finish(void)
{
	pr_debug("SMPCMP: CPU%d: %s\n", smp_processor_id(), __func__);

	/* CDFIXME: remove this? */
	write_c0_compare(read_c0_count() + (8 * mips_hpt_frequency / HZ));

#ifdef CONFIG_MIPS_MT_FPAFF
	/* If we have an FPU, enroll ourselves in the FPU-full mask */
	if (cpu_has_fpu)
		cpumask_set_cpu(smp_processor_id(), &mt_fpu_cpumask);
#endif /* CONFIG_MIPS_MT_FPAFF */

	local_irq_enable();
}

/*
 * Setup the PC, SP, and GP of a secondary processor and start it running
 * smp_bootstrap is the place to resume from
 * __KSTK_TOS(idle) is apparently the stack pointer
 * (unsigned long)idle->thread_info the gp
 */
static int cmp_boot_secondary(int cpu, struct task_struct *idle)
{
	struct thread_info *gp = task_thread_info(idle);
	unsigned long sp = __KSTK_TOS(idle);
	unsigned long pc = (unsigned long)&smp_bootstrap;
	unsigned long a0 = 0;

	pr_debug("SMPCMP: CPU%d: %s cpu %d\n", smp_processor_id(),
		__func__, cpu);

#if defined(CONFIG_MIPS_MT_SMP)
	/* Needed? */
	flush_icache_range((unsigned long)gp,
			   (unsigned long)(gp + sizeof(struct thread_info)));
#endif
	if (IS_ENABLED(CONFIG_CPU_RLX))
		mmcr_init_core(cpu);

	amon_cpu_start(cpu, pc, sp, (unsigned long)gp, a0);
	return 0;
}

static unsigned core_vpe_count(unsigned core)
{
	unsigned int nvpe;
	unsigned int mvpconf0;

	if (!IS_ENABLED(CONFIG_MIPS_MT_SMP) || !cpu_has_mipsmt)
		return 1;

	mvpconf0 = read_c0_mvpconf0();
	nvpe = ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT) + 1;

	return nvpe;
}

/*
 * Common setup before any secondaries are started
 */
void __init cmp_smp_setup(void)
{
	int i;
	int ncpu = 0;
	int nvpe = 0;

	pr_debug("SMPCMP: CPU%d: %s\n", smp_processor_id(), __func__);

#ifdef CONFIG_MIPS_MT_FPAFF
	/* If we have an FPU, enroll ourselves in the FPU-full mask */
	if (cpu_has_fpu)
		cpumask_set_cpu(0, &mt_fpu_cpumask);
#endif /* CONFIG_MIPS_MT_FPAFF */

	nvpe = core_vpe_count(0);
	smp_num_siblings = nvpe;

	for (i = 1; i < NR_CPUS; i++) {
		if (amon_cpu_avail(i)) {
			set_cpu_possible(i, true);
			__cpu_number_map[i]	= ++ncpu;
			__cpu_logical_map[ncpu] = i;

			cpu_data[ncpu].globalnumber = ncpu / nvpe;
#ifdef CONFIG_MIPS_MT_SMP
			cpu_data[ncpu].vpe_id = ncpu % nvpe;
#endif
		}
	}
	pr_info("Detected %i available secondary CPU(s)\n", ncpu);
}

void __init cmp_prepare_cpus(unsigned int max_cpus)
{
	pr_debug("SMPCMP: CPU%d: %s max_cpus=%d\n",
		 smp_processor_id(), __func__, max_cpus);

#ifdef CONFIG_MIPS_MT
	/*
	 * FIXME: some of these options are per-system, some per-core and
	 * some per-cpu
	 */
	mips_mt_set_cpuoptions();
#endif
}

const struct plat_smp_ops cmp_smp_ops = {
	.send_ipi_single	= mips_smp_send_ipi_single,
	.send_ipi_mask		= mips_smp_send_ipi_mask,
	.init_secondary		= cmp_init_secondary,
	.smp_finish		= cmp_smp_finish,
	.boot_secondary		= cmp_boot_secondary,
	.smp_setup		= cmp_smp_setup,
	.prepare_cpus		= cmp_prepare_cpus,
};
