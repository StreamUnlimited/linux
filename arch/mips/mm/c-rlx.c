/*
 * Realtek Semiconductor Corp.
 *
 * cache-rlx.c: RLX specific mmu/cache code.
 *
 * Tony Wu (tonywu@realtek.com)
 * Dec. 07, 2008
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/mm.h>

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/mmu_context.h>
#include <asm/isadep.h>
#include <asm/io.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>

void rlx_cache_init(void);

#define cache_max	(cpu_icache_size >= cpu_dcache_size ? \
				cpu_icache_size : cpu_dcache_size)
#define cache_min	(cpu_icache_size >= cpu_dcache_size ? \
				cpu_dcache_size : cpu_icache_size)

/*
 * Determine whether CPU has CACHE OP
 */
#undef CONFIG_CPU_TAROKO
#undef CONFIG_CPU_WULING
#undef CONFIG_CPU_HAS_DCACHE_OP
#undef CONFIG_CPU_HAS_ICACHE_OP

#if defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
  #define CONFIG_CPU_TAROKO
  #define CONFIG_CPU_HAS_DCACHE_OP
  #define CONFIG_CPU_HAS_ICACHE_OP
#endif

#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
  #define CONFIG_CPU_WULING
  #define CONFIG_CPU_HAS_DCACHE_OP
#endif

#ifdef CONFIG_CPU_HAS_WBIC
# define CACHE_COMBO_CCTL
# define CACHE_COMBO_OP
#endif

#ifdef CONFIG_SMP
# undef CACHE_COMBO_OP
# define SMP_BARRIER()		__asm__ __volatile__ ("sync" : : : "memory")
#else
# define SMP_BARRIER()		do { } while (0)
#endif
/*
 *  CACHE OP
 *   0x10 = IInval
 *   0x11 = DInval
 *   0x15 = DWBInval
 *   0x19 = DWB
 *   0x1b = DWB_IInval
 */
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define DCACHE_INV	0x11
  #define DCACHE_WBINV	0x15
  #define DCACHE_WB	0x19
#else
  #define DCACHE_INV	0x11
  #define DCACHE_WBINV	0x11
  #define DCACHE_WB	0x11
#endif

#ifdef CACHE_COMBO_OP
  #define ICACHE_INV	0x1b
#else
  #define ICACHE_INV	0x10
#endif

#define CACHE_OP(op, p)			\
	__asm__ __volatile__ (		\
		".set  push\n"		\
		".set  noreorder\n"	\
		"cache %0, 0x000(%1)\n"	\
		".set  pop\n"		\
		: : "i" (op), "r" (p)	\
	)

#define CACHE32_UNROLL08(op, p)		\
	__asm__ __volatile__ (		\
		".set  push\n"		\
		".set  noreorder\n"	\
		"cache %0, 0x000(%1)\n"	\
		"cache %0, 0x020(%1)\n"	\
		"cache %0, 0x040(%1)\n"	\
		"cache %0, 0x060(%1)\n"	\
		"cache %0, 0x080(%1)\n"	\
		"cache %0, 0x0a0(%1)\n"	\
		"cache %0, 0x0c0(%1)\n"	\
		"cache %0, 0x0e0(%1)\n"	\
		".set  pop\n"		\
		: : "i" (op), "r" (p)	\
	)

#define CACHE16_UNROLL16(op, p)		\
	__asm__ __volatile__ (		\
		".set  push\n"		\
		".set  noreorder\n"	\
		"cache %0, 0x000(%1)\n"	\
		"cache %0, 0x010(%1)\n"	\
		"cache %0, 0x020(%1)\n"	\
		"cache %0, 0x030(%1)\n"	\
		"cache %0, 0x040(%1)\n"	\
		"cache %0, 0x050(%1)\n"	\
		"cache %0, 0x060(%1)\n"	\
		"cache %0, 0x070(%1)\n"	\
		"cache %0, 0x080(%1)\n"	\
		"cache %0, 0x090(%1)\n"	\
		"cache %0, 0x0a0(%1)\n"	\
		"cache %0, 0x0b0(%1)\n"	\
		"cache %0, 0x0c0(%1)\n"	\
		"cache %0, 0x0d0(%1)\n"	\
		"cache %0, 0x0e0(%1)\n"	\
		"cache %0, 0x0f0(%1)\n"	\
		".set  pop\n"		\
		: : "i" (op), "r" (p)	\
	)

#if (cpu_dcache_line == 32)
  #define DCACHE_UNROLL(op,p)  CACHE32_UNROLL08(op,(p))
#else
  #define DCACHE_UNROLL(op,p)  CACHE16_UNROLL16(op,(p))
#endif

#if (cpu_icache_line == 32)
  #define ICACHE_UNROLL(op,p)  CACHE32_UNROLL08(op,(p))
#else
  #define ICACHE_UNROLL(op,p)  CACHE16_UNROLL16(op,(p))
#endif

/*
 *  CCTL OP
 *   0x1   = DInval
 *   0x2   = IInval
 *   0x100 = DWB
 *   0x200 = DWB_Inval
 */
#define ICCTL_INV		0x2
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define DCCTL_INV		0x001
  #define DCCTL_WB		0x100
  #define DCCTL_WBINV		0x200
#else
  #define DCCTL_INV		0x1
  #define DCCTL_WB		0x1
  #define DCCTL_WBINV		0x1
#endif

#ifdef CONFIG_CPU_TAROKO
#define CCTL_OP(op)			\
	__asm__ __volatile__(		\
		".set  push\n"		\
		".set  noreorder\n"	\
		"mtc0	$0, $20\n"	\
		"li	$8, %0\n"	\
		"mtc0	$8, $20\n"	\
		".set  pop\n"		\
		: : "i" (op)		\
	)
#else
#define CCTL_OP(op)			\
	__asm__ __volatile__(		\
		".set  push\n"		\
		".set  noreorder\n"	\
		"mfc0	$8, $20\n"	\
		"ori	$8, %0\n"	\
		"xori	$9, $8, %0\n"	\
		"mtc0	$9, $20\n"	\
		"mtc0	$8, $20\n"	\
		".set pop\n"		\
		: : "i" (op)		\
	)
#endif

static inline int has_valid_asid(const struct mm_struct *mm)
{
	return cpu_context(smp_processor_id(), mm);
}

/*
 * Dummy cache handling routines for machines without boardcaches
 */
static void cache_noop(void) {}

static __always_inline void blast_dcache_loop(unsigned long start, unsigned long end,
				     unsigned int cache_op)
{
	unsigned long p;

	for (p = start; p <= end-0x100; p += 0x100) {
		DCACHE_UNROLL(cache_op, p);
	}

	p = p & ~(cpu_dcache_line -1);
	while (p < end) {
		CACHE_OP(cache_op, p);
		p += cpu_dcache_line;
	}
}

static __always_inline void blast_icache_loop(unsigned long start, unsigned long end,
				     unsigned int cache_op)
{
	unsigned long p;

	for (p = start; p <= end-0x100; p += 0x100) {
		ICACHE_UNROLL(cache_op, p);
	}

	p = p & ~(cpu_icache_line -1);
	while (p < end) {
		CACHE_OP(cache_op, p);
		p += cpu_icache_line;
	}
}

/*
 * DCACHE part
 *
 * flush_dcache_range:
 * flush_data_cache_page: flush a data cache page
 */
static __always_inline void rlx_flush_dcache_range(unsigned long start, unsigned long end,
					  unsigned int cctl_op, unsigned int cache_op)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	if (end - start > cpu_dcache_size) {
		CCTL_OP(cctl_op);
		return;
	}
	blast_dcache_loop(start, end, cache_op);
#else
	CCTL_OP(cctl_op);
#endif
}

static inline void local_rlx_flush_data_cache_page(void *addr)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	unsigned long start = (unsigned long)addr;
	blast_dcache_loop(start, start + PAGE_SIZE, DCACHE_WBINV);
#else
	CCTL_OP(DCCTL_WBINV);
#endif
}

static void rlx_flush_data_cache_page(unsigned long addr)
{
	if (in_atomic())
		local_rlx_flush_data_cache_page((void *)addr);
	else {
		preempt_disable();
		local_rlx_flush_data_cache_page((void *)addr);
		preempt_enable();
	}
}

/*
 * ICACHE part
 *
 * flush_icache_range: flush a range of instructions on all CPUs
 * local_flush_icache_range: flush a range of instructions on this CPU
 */
static void rlx_flush_icache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_ICACHE_OP
	unsigned long size;

	size = end - start;
	if (size > cache_max) {
  #ifdef CACHE_COMBO_CCTL
		CCTL_OP(ICCTL_INV | DCCTL_WBINV);
  #else
		CCTL_OP(DCCTL_WBINV);
		SMP_BARRIER();
		CCTL_OP(ICCTL_INV);
  #endif
		return;
	}

  #ifdef CACHE_COMBO_OP
	if (size <= cache_min) {
		blast_icache_loop(start, end, ICACHE_INV);
		return;
	}
  #endif

	/*
	 * If we get here, it means we cannot do combo_op.
	 *
	 * Due to:
	 * 1. combo_op = n
	 * 2. combo_op = y, but in one of the following cases
	 *    cpu_icache_size < size < cpu_dcache_size
	 *    cpu_dcache_size < size < cpu_icache_size
	 */
	rlx_flush_dcache_range(start, end, DCCTL_WBINV, DCACHE_WBINV);
	SMP_BARRIER();
	if (size > cpu_icache_size)
		CCTL_OP(ICCTL_INV);
	else
		blast_icache_loop(start, end, ICACHE_INV);
#else
	rlx_flush_dcache_range(start, end, DCCTL_WBINV, DCACHE_WBINV);
	SMP_BARRIER();
	CCTL_OP(ICCTL_INV);
#endif
}

/*
 * CACHE part
 *
 * flush_cache_all:
 * __flush_cache_all: flush all caches
 */
static inline void rlx___flush_cache_all(void)
{
#ifdef CACHE_COMBO_CCTL
	CCTL_OP(DCCTL_WBINV | ICCTL_INV);
#else
	CCTL_OP(DCCTL_WBINV);
	SMP_BARRIER();
	CCTL_OP(ICCTL_INV);
#endif
}

static inline void rlx_flush_cache_page(struct vm_area_struct *vma,
					unsigned long addr, unsigned long pfn)
{
	unsigned long kaddr = KSEG0ADDR(pfn << PAGE_SHIFT);
	int exec = vma->vm_flags & VM_EXEC;
	struct mm_struct *mm = vma->vm_mm;
	pgd_t *pgdp;
	pud_t *pudp;
	pmd_t *pmdp;
	pte_t *ptep;

	pr_debug("cpage[%08llx,%08lx]\n",
		 cpu_context(smp_processor_id(), mm), addr);

	/* No ASID => no such page in the cache.  */
	if (cpu_context(smp_processor_id(), mm) == 0)
		return;

	pgdp = pgd_offset(mm, addr);
	pudp = pud_offset(pgdp, addr);
	pmdp = pmd_offset(pudp, addr);
	ptep = pte_offset(pmdp, addr);

	/* Invalid => no such page in the cache.  */
	if (!(pte_val(*ptep) & _PAGE_PRESENT))
		return;

#ifdef CACHE_COMBO_OP
	if (exec)
		blast_icache_loop(kaddr, kaddr + PAGE_SIZE, ICACHE_INV);
	else
		blast_dcache_loop(kaddr, kaddr + PAGE_SIZE, DCACHE_WBINV);
#else
	blast_dcache_loop(kaddr, kaddr + PAGE_SIZE, DCACHE_WBINV);
	if (exec) {
		SMP_BARRIER();
		blast_icache_loop(kaddr, kaddr + PAGE_SIZE, ICACHE_INV);
	}
#endif
}

#ifdef CONFIG_DMA_NONCOHERENT
static void rlx_dma_cache_wback_inv(unsigned long start, unsigned long size)
{
	/* Catch bad driver code */
	BUG_ON(size == 0);

	iob();
	rlx_flush_dcache_range(start, start + size, DCCTL_WBINV, DCACHE_WBINV);
}

static void rlx_dma_cache_wback(unsigned long start, unsigned long size)
{
	/* Catch bad driver code */
	BUG_ON(size == 0);

	iob();
	rlx_flush_dcache_range(start, start + size, DCCTL_WB, DCACHE_WB);
}

static void rlx_dma_cache_inv(unsigned long start, unsigned long size)
{
	/* Catch bad driver code */
	BUG_ON(size == 0);

	iob();
  #ifdef CONFIG_CPU_HAS_DCACHE_OP
	blast_dcache_loop(start, start + size, DCACHE_INV);
  #else
	CCTL_OP(DCCTL_WBINV);
  #endif
}
#endif

static void rlx_flush_kernel_vmap_range(unsigned long vaddr, int size)
{
	BUG();
}

static void coherency_setup(void)
{
	unsigned int cca;

#ifdef CONFIG_TAROKO_CMP
	cca = _CACHE_CACHABLE_COHERENT;
#else
	cca = _CACHE_CACHABLE_NONCOHERENT;
#endif
	pr_debug("Using cache attribute %d\n", cca);

	_page_cachable_default = cca;
}

static void probe_pcache(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	c->icache.linesz = cpu_icache_line;
	c->dcache.linesz = cpu_dcache_line;
	c->scache.linesz = cpu_scache_line;
	c->tcache.linesz = 0;
}

void rlx_cache_init(void)
{
	extern void build_clear_page(void);
	extern void build_copy_page(void);

	probe_pcache();

	flush_cache_all = (void *) cache_noop;
	__flush_cache_all = rlx___flush_cache_all;
	flush_cache_mm = (void *) cache_noop;
	flush_cache_range = (void *) cache_noop;
	flush_cache_page = rlx_flush_cache_page;
	flush_icache_range = rlx_flush_icache_range;
	local_flush_icache_range = rlx_flush_icache_range;
	__flush_icache_user_range = rlx_flush_icache_range;
	__local_flush_icache_user_range = rlx_flush_icache_range;

	__flush_kernel_vmap_range = rlx_flush_kernel_vmap_range;

	/* MIPS specific */
	flush_data_cache_page = rlx_flush_data_cache_page;
	local_flush_data_cache_page = local_rlx_flush_data_cache_page;

#ifdef CONFIG_DMA_NONCOHERENT
	_dma_cache_wback_inv = rlx_dma_cache_wback_inv;
	_dma_cache_wback = rlx_dma_cache_wback;
	_dma_cache_inv = rlx_dma_cache_inv;
#endif

	printk("icache: %dkB/%dB, dcache: %dkB/%dB, scache: %dkB/%dB\n",
		cpu_icache_size >> 10, cpu_icache_line,
		cpu_dcache_size >> 10, cpu_dcache_line,
		cpu_scache_size >> 10, cpu_scache_line);

	build_clear_page();
	build_copy_page();
	rlx___flush_cache_all();

	coherency_setup();
}
