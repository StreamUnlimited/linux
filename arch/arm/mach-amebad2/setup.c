// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek Setup support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/system_misc.h>

#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>

#include <linux/init.h>
#include <linux/io.h>

#include <linux/reboot.h>
#include <linux/sysfs.h>
#include <mach/hardware.h>
#include <misc/realtek-misc.h>

#define BOOTLOADER_PHYSICAL_ADDR     0x42008D04UL
#define BOOTLOADER_IMAGE2_PHY_ADDR   0x80220001UL

/* SYSTEM_CTRL_BASE_LP */
static void __iomem *plat_lsys_base = NULL;

static void __iomem *mapped_boot_addr;
static u32 boot_slot_addr;
struct kobject *boot_slotinfo_kobj;

static ssize_t boot_slotinfo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int slot = 0; // defalut A
	printk("boot_slotinfo_show enter");
	if (boot_slot_addr == BOOTLOADER_IMAGE2_PHY_ADDR) { // slota
		slot = 0;
	} else {
		slot = 1;    //slot B
	}

	return sprintf(buf, "%d\n", slot);
}

static ssize_t boot_slotinfo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	printk("boot_slotinfo_store enter");
	boot_slot_addr = (buf[0] == '0') ? BOOTLOADER_IMAGE2_PHY_ADDR : 1;
	return n;
}


static struct kobj_attribute boot_slotinfo_attr =
	__ATTR(boot_slotinfo, 0644, boot_slotinfo_show, boot_slotinfo_store);

static void boot_slotinfo_sysfs_create(void)
{
	int error = 0;
	boot_slotinfo_kobj = kobject_create_and_add("boot_slotinfo", NULL);
	if (!boot_slotinfo_kobj) {
		printk(KERN_ERR "kobject_create_and_add failed\n");
		return;
	}
	error = sysfs_create_file(boot_slotinfo_kobj, &boot_slotinfo_attr.attr);
	if (error) {
		printk(KERN_ERR "sysfs_create_file failed: %d\n", error);
		return;
	}

	mapped_boot_addr = ioremap(BOOTLOADER_PHYSICAL_ADDR, 1);
	if (mapped_boot_addr == NULL) {
		printk("cannot ioremap registers\n");
		sysfs_remove_file(boot_slotinfo_kobj, &boot_slotinfo_attr.attr);
		return;
	}

	boot_slot_addr = readl(mapped_boot_addr);
	printk("%s: bootloader register value: %x\n", __func__, boot_slot_addr);
}

/* allocate io resource */
static struct map_desc bsp_io_desc[] __initdata = {
	{
		.virtual = BSP_SMP_VADDR,
		.pfn = __phys_to_pfn(BSP_SMP_PADDR),
		.length = SZ_4,
		.type = MT_DEVICE_NONSHARED,
	},
	{
		.virtual = BSP_EARLYCON_VADDR,
		.pfn = __phys_to_pfn(BSP_EARLYCON_PADDR),
		.length = SZ_256,
		.type = MT_DEVICE,
	},
};

static void __init plat_map_io(void)
{
	extern void plat_smp_map_io(void);

	iotable_init(bsp_io_desc, ARRAY_SIZE(bsp_io_desc));
#ifdef CONFIG_SMP
	plat_smp_map_io();
#endif
}

static void plat_arch_restart(enum reboot_mode mode, const char *cmd)
{
	if (plat_lsys_base) {
		writel(0U, plat_lsys_base + REG_AON_SYSRST_MSK);
		writel(0U, plat_lsys_base + REG_LSYS_SYSRST_MSK0);
		writel(0U, plat_lsys_base + REG_LSYS_SYSRST_MSK1);
		writel(0U, plat_lsys_base + REG_LSYS_SYSRST_MSK2);

		writel(SYS_RESET_KEY, plat_lsys_base + REG_LSYS_SW_RST_TRIG);
		writel(LSYS_BIT_LPSYS_RST << AP_CPU_ID, plat_lsys_base + REG_LSYS_SW_RST_CTRL);
		writel(SYS_RESET_TRIG, plat_lsys_base + REG_LSYS_SW_RST_TRIG);
	}
}

static void __init plat_init_machine(void)
{
	struct device_node *np;
	int rl_version;
	int rl_numer;

	rl_numer = rtk_misc_get_rl_number();
	if (rl_numer >= 0) {
		printk("SoC RL number: 0x%04X\n", rl_numer);
	} else {
		pr_err("%s: get rl number failed\n", __func__);
	}

	rl_version = rtk_misc_get_rl_version();
	if (rl_version >= 0) {
		printk("SoC RL version: %d\n", rl_version);
	} else {
		pr_err("%s: get rl version failed\n", __func__);
	}

	np = of_find_compatible_node(NULL, NULL, "realtek,amebad2-system-ctrl-ls");
	if (np) {
		plat_lsys_base = of_iomap(np, 0);
		if (plat_lsys_base) {
			arm_pm_restart = plat_arch_restart;
		} else {
			pr_err("%s:of_iomap(plat_lsys_base) failed\n", __func__);
		}
		of_node_put(np);
	}

	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);

	boot_slotinfo_sysfs_create();
}

#ifdef CONFIG_SMP
extern struct smp_operations plat_smp_ops;
#endif

/* Realtek Ameba DT machine */
static const char *const plat_dt_match[] __initconst = {
	"realtek,ameba",
	NULL
};

DT_MACHINE_START(RLXARM_DT, "Realtek ARM (Flattened Device Tree)")
	.dt_compat = plat_dt_match,
	.init_machine = plat_init_machine,
	.map_io = plat_map_io,
#ifdef CONFIG_SMP
	.smp = smp_ops(plat_smp_ops),
#endif
MACHINE_END
