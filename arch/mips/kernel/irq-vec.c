/*
 * Realtek Semiconductor Corp.
 *
 * This file define the irq handler for RLX CPU interrupts.
 *
 * Tony Wu (tonywu@realtek.com)
 * Feb. 28, 2008
 */

#include <linux/irq.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <asm/irq_vec.h>
#include <asm/mipsregs.h>

static inline void unmask_mips_vec_irq(struct irq_data *d)
{
	set_lxc0_estatus(0x10000 << (d->irq - MIPS_VEC_IRQ_BASE));
	irq_enable_hazard();
}

static inline void mask_mips_vec_irq(struct irq_data *d)
{
	clear_lxc0_estatus(0x10000 << (d->irq - MIPS_VEC_IRQ_BASE));
	irq_disable_hazard();
}

static struct irq_chip mips_vec_irq_controller = {
	.name		= "LOPI",
	.irq_ack	= mask_mips_vec_irq,
	.irq_mask	= mask_mips_vec_irq,
	.irq_mask_ack	= mask_mips_vec_irq,
	.irq_unmask	= unmask_mips_vec_irq,
	.irq_eoi	= unmask_mips_vec_irq,
	.irq_disable	= mask_mips_vec_irq,
	.irq_enable	= unmask_mips_vec_irq,
};

void __init mips_vec_irq_init(void)
{
	int i;
	int irq_base;
	extern char handle_vec;

	/* Mask interrupts. */
	clear_lxc0_estatus(EST0_IM);
	clear_lxc0_ecause(ECAUSEF_IP);

	irq_base = MIPS_VEC_IRQ_BASE;

	for (i = irq_base; i < irq_base + 8; i++)
		irq_set_chip_and_handler(i, &mips_vec_irq_controller,
					 handle_percpu_irq);

	write_lxc0_intvec(&handle_vec);
}

asmlinkage void mips_do_lopi_IRQ(int irq_offset)
{
	do_IRQ(MIPS_VEC_IRQ_BASE + irq_offset);
}
