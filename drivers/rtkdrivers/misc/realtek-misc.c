// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek Misc support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/notifier.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <misc/realtek-misc.h>
#include <linux/nvmem-consumer.h>

static int realtek_misc_major;
static struct proc_dir_entry *rl_version_proc_ent;
static struct proc_dir_entry *uuid_proc_ent;
struct mutex misc_mutex;

struct realtek_misc_dev {
	struct cdev cdev;
	void __iomem *base;
	int current_affair;
	u32 uuid;
};
struct realtek_misc_dev *realtek_misc_devp;

/*****************************************************************************************/
/*******************************  Customize ioctl functions  *****************************/
/*****************************************************************************************/

/* BT functions. */
static uint8_t bt_ant_switch = 0;
//0, means bt_rfafe        1, means wl_rfafe
static uint32_t cal_bit_shift(uint32_t Mask)
{
	uint32_t i;
	for (i = 0; i < 31; i++) {
		if (((Mask >> i) & 0x1) == 1) {
			break;
		}
	}
	return (i);
}

static void set_reg_value(void __iomem	*reg_address, uint32_t Mask, uint32_t val)
{
	uint32_t shift = 0;
	uint32_t data = 0;
	data = readl(reg_address);
	shift = cal_bit_shift(Mask);
	data = ((data & (~Mask)) | (val << shift));
	writel(data, reg_address);
	data = readl(reg_address);
}

static void bt_power_on(void __iomem *reg_base)
{
	set_reg_value(reg_base, BIT(9), 1);  		//0x42008200//enable power cut of BTON
	mdelay(5);
	set_reg_value(reg_base + 0x4, BIT(9), 0);  		//0x42008204//disable ISO of BTON
	mdelay(5);
	set_reg_value(reg_base + 0x50, BIT(21) | BIT(22), 3); //0x42008250
	mdelay(5);
	set_reg_value(reg_base + 0x50, BIT(1) | BIT(2), 3);  //0x42008250// enable BT AFE & BT S0 RF, BT S1 also enable S0 RF
	mdelay(5);
	if (bt_ant_switch == 0) {
		set_reg_value(reg_base + 0x50, BIT(0), 0);  		//0x42008250//BT use BT RFAFE
		mdelay(5);
	} else if (bt_ant_switch == 1) {
		set_reg_value(reg_base + 0x50, BIT(0), 1);  		//0x42008250//BT use WL RFAFE
		mdelay(5);
		set_reg_value(reg_base + 0x8, BIT(24), 1); 		//0x42008208//enable WL RFAFE control circuit
		mdelay(5);
		set_reg_value(reg_base + 0x740, BIT(5) | BIT(6), 3);  //0x42008940//enable WL RFAFE
		mdelay(5);
	}
	set_reg_value(reg_base, BIT(25), 1);  //0x42008200//Release BTON por,BT Memory
	mdelay(5);
	set_reg_value(reg_base + 0x8, BIT(13), 1);  		//0x42008208//release BTON reset
}

static void bt_power_off(void __iomem *reg_base)
{
	set_reg_value(reg_base + 0x8, BIT(13), 0);  		//0x42008208//BTON reset
	mdelay(5);
	set_reg_value(reg_base, BIT(25), 0);  //0x42008200//Release BTON por,BT Memory
	mdelay(5);
	if (bt_ant_switch == 0) {
		set_reg_value(reg_base + 0x50, BIT(1) | BIT(2), 0); //0x42008250//disable BT AFE & BT S0 RF
		mdelay(5);
	} else {
		//will cause wifi enter ps
		/*set_reg_value(reg_base + 0x740, BIT(5) | BIT(6), 0);  //0x42008940//disable RFAFE (if WIFI active, keep 2’b11)
		mdelay(5);
		set_reg_value(reg_base + 0x8, BIT(24), 0); 		//0x42008208//disable WL RFAFE control circuit (if WIFI active, keep 1’b1)
		mdelay(5);*/
		set_reg_value(reg_base + 0x50, BIT(1) | BIT(0), 0);  		//0x42008250//disable BT AFE & BT S0 RF
		mdelay(5);
	}
	set_reg_value(reg_base + 0x4, BIT(9), 1);  		//0x42008204//enable ISO of BTON
	mdelay(5);
	set_reg_value(reg_base, BIT(9), 0);  		//0x42008200//disable power cut of BTON
	mdelay(5);
}

static void hci_platform_controller_reset(void __iomem *reg_base)
{
	/* BT Controller Power */
	bt_power_on(reg_base);
	mdelay(5);

	/* GNT_BT */
	// set_reg_value(reg_base + 0x564, BIT(9) | BIT(10) | BIT(11) | BIT(12), 15);//0x40000764
	// mdelay(5);
	mdelay(50);
}

static int hci_platform_controller_init(unsigned long arg)
{
	/* BT Controller Reset */
	if ((arg != 0) && (arg != 1)) {
		pr_err("\n%s: error bt ant %lu\n", __FUNCTION__, arg);
		return -EFAULT;
	}
	bt_ant_switch = arg;
	pr_info("\n%s: bt ant %d\n", __FUNCTION__, bt_ant_switch);
	hci_platform_controller_reset(realtek_misc_devp->base + 0x200);

	return 0;
}

static int hci_platform_controller_deinit(unsigned long arg)
{
	/* BT Controller Power Off */
	if ((arg != 0) && (arg != 1)) {
		pr_err("\n%s: error bt ant %lu\n", __FUNCTION__, arg);
		return -EFAULT;
	}
	bt_ant_switch = arg;
	pr_info("\n%s: bt ant %d\n", __FUNCTION__, bt_ant_switch);
	bt_power_off(realtek_misc_devp->base + 0x200);
	return 0;
}

/* Other functions. */
/* ....... */

/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

static int realtek_misc_set_mode(unsigned long arg)
{
	realtek_misc_devp->current_affair = arg;
	return 0;
}

int realtek_misc_open(struct inode *inode, struct file *filp)
{
	pr_debug("Realtek miscellaneous affairs: open\n");
	return 0;
}

int realtek_misc_release(struct inode *inode, struct file *filp)
{
	pr_debug("Realtek miscellaneous affairs: release\n");

	switch (realtek_misc_devp->current_affair) {
	case RTK_CMD_GET_RL_INFO:
		/* May release something. */
		break;
	case RTK_CMD_SET_BT_POWER_ON:
	case RTK_CMD_SET_BT_POWER_OFF:
		break;
	default:
		pr_warn("Please set a misc ioctl affair first\n");
		break;
	}

	return 0;
}

static void get_rl_version(char *result)
{
	int val = rtk_misc_get_rl_version();
	*result = val & 0xFF;
}

static ssize_t realtek_misc_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	char *result;
	int ret = 0;

	pr_debug("Realtek miscellaneous affairs: read\n");
	if (!mutex_trylock(&misc_mutex)) {
		return -EBUSY;
	}

	/* malloc kernel space for result. */
	result = kmalloc(count, GFP_KERNEL);

	switch (realtek_misc_devp->current_affair) {
	case RTK_CMD_GET_RL_INFO:
		get_rl_version(result);
		break;
	default:
		pr_warn("Please set the misc ioctl affair first\n");
		break;
	}

	/* copy memery from kernel space to user space. */
	ret = copy_to_user(buf, result, count) ? -EFAULT : ret;
	kfree(result);

	mutex_unlock(&misc_mutex);
	return ret;
}

static long realtek_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	pr_debug("Realtek miscellaneous affairs: ioctl\n");
	if (!mutex_trylock(&misc_mutex)) {
		return -EBUSY;
	}

	switch (cmd) {
	case RTK_CMD_SET_MODE:
		realtek_misc_set_mode(arg);
		break;
	case RTK_CMD_GET_RL_INFO:
		/* May add some tasks.. */
		break;
	case RTK_CMD_SET_BT_POWER_ON:
		hci_platform_controller_init(arg);
		break;
	case RTK_CMD_SET_BT_POWER_OFF:
		hci_platform_controller_deinit(arg);
		break;
	default:
		pr_warn("Unsupported misc ioctl cmd: %d\n", cmd);
		break;
	}

	mutex_unlock(&misc_mutex);
	return ret;
}

static int realtek_misc_get_uuid(struct device *dev, u32 *uuid)
{
	struct nvmem_cell *cell;
	unsigned char *efuse_buf;
	size_t len;

	cell = nvmem_cell_get(dev, "uuid");
	if (IS_ERR(cell))
		return PTR_ERR(cell);

	efuse_buf = nvmem_cell_read(cell, &len);
	nvmem_cell_put(cell);

	if (IS_ERR(efuse_buf))
		return PTR_ERR(efuse_buf);

	*uuid = (efuse_buf[3] << 24) | (efuse_buf[2] << 16)
		| (efuse_buf[1] << 8) | efuse_buf[0];

	return 0;
}

/*
* uuid proc ops
*/
static int realtek_uuid_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "uuid: %u\n", realtek_misc_devp->uuid);

	return 0;
}

static int realtek_uuid_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, realtek_uuid_proc_show, NULL);
}

static const struct file_operations realtek_uuid_proc_fops = {
	.open = realtek_uuid_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
* rl_version proc ops
*/
static int realtek_rl_version_proc_show(struct seq_file *m, void *v)
{
	int val = rtk_misc_get_rl_version();
	seq_printf(m, "%d\n", val);

	return 0;
}

static int realtek_rl_verion_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, realtek_rl_version_proc_show, NULL);
}

static const struct file_operations realtek_rl_verion_proc_fops = {
	.open = realtek_rl_verion_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations realtek_misc_fops = {
	.owner = THIS_MODULE,
	.open = realtek_misc_open,
	.release = realtek_misc_release,
	.read = realtek_misc_read,
	.unlocked_ioctl	= realtek_misc_ioctl,
};

static void realtek_misc_setup_cdev(struct realtek_misc_dev *dev, int index)
{
	int err, devno = MKDEV(realtek_misc_major, index);

	cdev_init(&dev->cdev, &realtek_misc_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &realtek_misc_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err) {
		pr_err("Fail to add realtek_misc %d: %d\n", index, err);
	}
}

int realtek_misc_init(void)
{
	int ret;
	dev_t devno;
	struct device_node *np;
	void __iomem *base;
	struct class *realtek_misc_class;
	struct proc_dir_entry *realtek_misc_proc_dir;
	struct device *dev;

	ret = alloc_chrdev_region(&devno, 0, 1, "misc-ioctl");
	if (ret < 0) {
		return ret;
	}
	realtek_misc_major = MAJOR(devno);

	realtek_misc_devp = kmalloc(sizeof(struct realtek_misc_dev), GFP_KERNEL);
	if (!realtek_misc_devp) {
		ret = -ENOMEM;
		pr_err("%s: Fail to kmalloc realtek_misc_devp\n", __FUNCTION__);
		goto fail_malloc;
	}

	memset(realtek_misc_devp, 0, sizeof(struct realtek_misc_dev));
	realtek_misc_setup_cdev(realtek_misc_devp, 0);
	realtek_misc_devp->current_affair = RTK_CMD_NOT_SET;

	np = of_find_compatible_node(NULL, NULL, "realtek,amebad2-system-ctrl-ls");
	if (np) {
		base = of_iomap(np, 0);
		if (base) {
			realtek_misc_devp->base = base;
		} else {
			pr_err("%s: iomap failed\n", __FUNCTION__);
			goto fail_iomap;
		}
		of_node_put(np);
	} else {
		goto fail_of_find_node;
	}

	realtek_misc_class = class_create(THIS_MODULE, "misc-ioctl");
	dev = device_create(realtek_misc_class, NULL, MKDEV(realtek_misc_major, 0), NULL, "misc-ioctl");
	dev->of_node = np;
	mutex_init(&misc_mutex);

	realtek_misc_proc_dir = proc_mkdir("realtek", NULL);
	if (!realtek_misc_proc_dir) {
		pr_err("%s: Fail to mkdir for procsys\n", __FUNCTION__);
		goto fail_iomap;
	}

	rl_version_proc_ent = proc_create("rl_version", 0644, realtek_misc_proc_dir, &realtek_rl_verion_proc_fops);
	uuid_proc_ent = proc_create("uuid", 0644, realtek_misc_proc_dir, &realtek_uuid_proc_fops);

	realtek_misc_get_uuid(dev, &realtek_misc_devp->uuid);

	return 0;

fail_iomap:
	of_node_put(np);

fail_of_find_node:
	kfree(realtek_misc_devp);

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}

void realtek_misc_exit(void)
{
	proc_remove(rl_version_proc_ent);

	if (realtek_misc_devp != NULL) {
		iounmap(realtek_misc_devp->base);
		cdev_del(&realtek_misc_devp->cdev);
		kfree(realtek_misc_devp);
	}
	unregister_chrdev_region(MKDEV(realtek_misc_major, 0), 1);
}

module_init(realtek_misc_init);
module_exit(realtek_misc_exit);

MODULE_DESCRIPTION("Realtek Ameba Misc driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Realtek Corporation");
