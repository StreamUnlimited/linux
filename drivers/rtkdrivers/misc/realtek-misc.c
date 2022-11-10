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
#include <misc/realtek-misc.h>
#include <linux/delay.h>
#include <linux/errno.h>

static int realtek_misc_major;
struct mutex misc_mutex;

struct realtek_misc_dev {
	struct cdev cdev;
	int current_affair;
};
struct realtek_misc_dev *realtek_misc_devp;

/*****************************************************************************************/
/*******************************  Customize ioctl functions  *****************************/
/*****************************************************************************************/

/* Example: Share virtual regmap with drivers, using of_iomap. */
#define REG_LSYS_OTP_SYSCFG0        0x0230
#define LSYS_GET_BOOT_SELECT(x)     ((u32)(((x >> 20) & 0x00000003)))
static int read_flash_type(char *result)
{
	u32 val = 0;
	int ret = 0;
	char nand[] = "Nand";
	char nor[] = "Nor";
	struct device_node *np;
	void __iomem *base;

	np = of_find_compatible_node(NULL, NULL, "realtek,amebad2-system-ctrl-ls");
	if (np) {
		base = of_iomap(np, 0);
		if (base) {
			val =  readl(base + REG_LSYS_OTP_SYSCFG0);
		} else {
			pr_err("%s: of_iomap(plat_lsys_base) failed\n", __FUNCTION__);
			return -EFAULT;
		}
		of_node_put(np);
	}

	if (LSYS_GET_BOOT_SELECT(val) == 2) {
		/* Flash type: Nor. */
		strcpy(result, nor);
	} else if (LSYS_GET_BOOT_SELECT(val) == 1) {
		/* Flash type: Nand. */
		strcpy(result, nand);
	} else {
		pr_info("Wrong type.");
		return -EFAULT;
	}
	return ret;
}

/* BT functions. */
#define BIT(__n)       (1<<(__n))
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
		set_reg_value(reg_base + 0x740, BIT(5) | BIT(6), 0);  //0x42008940//disable RFAFE (if WIFI active, keep 2’b11)
		mdelay(5);
		set_reg_value(reg_base + 0x8, BIT(24), 0); 		//0x42008208//disable WL RFAFE control circuit (if WIFI active, keep 1’b1)
		mdelay(5);
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
	struct device_node *np;
	void __iomem *reg_base;

	np = of_find_compatible_node(NULL, NULL, "realtek,amebad2-system-ctrl-ls");
	if (np) {
		reg_base = of_iomap(np, 0);
		if (reg_base) {
			if ((arg != 0) && (arg != 1)) {
				pr_err("\n%s: error bt ant %llu\n", __FUNCTION__, arg);
				return -EFAULT;
			}
			bt_ant_switch = arg;
			pr_info("\n%s: bt ant %d\n", __FUNCTION__, bt_ant_switch);
			hci_platform_controller_reset(reg_base + 0x200);
		} else {
			pr_err("%s: of_iomap(plat_lsys_base) failed\n", __FUNCTION__);
			return -EFAULT;
		}
		of_node_put(np);
	} else {
		pr_info("\nof_find_compatible_node fail");
	}
	return 0;
}

static int hci_platform_controller_deinit(unsigned long arg)
{
	/* BT Controller Power Off */
	struct device_node *np;
	void __iomem *reg_base;

	np = of_find_compatible_node(NULL, NULL, "realtek,amebad2-system-ctrl-ls");
	if (np) {
		reg_base = of_iomap(np, 0);
		if (reg_base) {
			if ((arg != 0) && (arg != 1)) {
				pr_err("\n%s: error bt ant %llu\n", __FUNCTION__, arg);
				return -EFAULT;
			}
			bt_ant_switch = arg;
			pr_info("\n%s: bt ant %d\n", __FUNCTION__, bt_ant_switch);
			bt_power_off(reg_base + 0x200);
		} else {
			pr_err("%s: of_iomap(plat_lsys_base) failed\n", __FUNCTION__);
			return -EFAULT;
		}
		of_node_put(np);
	}
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
	pr_info("Realtek miscellaneous affairs: open success.");
	return 0;
}

int realtek_misc_release(struct inode *inode, struct file *filp)
{
	pr_info("\nRealtek miscellaneous affairs: release.");

	switch (realtek_misc_devp->current_affair) {
	case RTK_CMD_READ_FLASH_TYPE:
		/* May release something. */
		pr_info("Example: do nothing while release.");
		break;
	case RTK_CMD_SET_BT_POWER_ON:
	case RTK_CMD_SET_BT_POWER_OFF:
		break;
	default:
		pr_info("Please set a misc ioctl affair first.");
	}

	return 0;
}

static ssize_t realtek_misc_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	char *result;
	int ret = 0;

	pr_info("\nRealtek miscellaneous affairs: read.");
	if (!mutex_trylock(&misc_mutex)) {
		return -EBUSY;
	}

	/* malloc kernel space for result. */
	result = kmalloc(count, GFP_KERNEL);

	switch (realtek_misc_devp->current_affair) {
	case RTK_CMD_READ_FLASH_TYPE:
		read_flash_type(result);
		break;
	default:
		pr_info("Please set the misc ioctl affair first.");
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

	pr_info("\nRealtek miscellaneous affairs: ioctl.");
	if (!mutex_trylock(&misc_mutex)) {
		return -EBUSY;
	}

	switch (cmd) {
	case RTK_CMD_SET_MODE:
		realtek_misc_set_mode(arg);
		break;
	case RTK_CMD_READ_FLASH_TYPE:
		/* May add some tasks.. */
		break;
	case RTK_CMD_SET_BT_POWER_ON:
		hci_platform_controller_init(arg);
		break;
	case RTK_CMD_SET_BT_POWER_OFF:
		hci_platform_controller_deinit(arg);
		break;
	default:
		pr_info("\n\nFree to configure the Realtek miscellaneous affairs in <sdk>/openwrt/realtek/linux-5.4/include/misc/realtek-misc.h.\n");
	}

	mutex_unlock(&misc_mutex);
	return ret;
}

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
		pr_err("Error %d add realtek_misc %d", err, index);
	}
}

int realtek_misc_init(void)
{
	int ret;
	dev_t devno;
	struct class *realtek_misc_class;

	ret = alloc_chrdev_region(&devno, 0, 1, "misc-ioctl");
	if (ret < 0) {
		return ret;
	}
	realtek_misc_major = MAJOR(devno);

	realtek_misc_devp = kmalloc(sizeof(struct realtek_misc_dev), GFP_KERNEL);
	if (!realtek_misc_devp) {
		ret = -ENOMEM;
		pr_err("Error add realtek_misc");
		goto fail_malloc;
	}

	memset(realtek_misc_devp, 0, sizeof(struct realtek_misc_dev));
	realtek_misc_setup_cdev(realtek_misc_devp, 0);
	realtek_misc_devp->current_affair = RTK_CMD_NOT_SET;

	realtek_misc_class = class_create(THIS_MODULE, "misc-ioctl");
	device_create(realtek_misc_class, NULL, MKDEV(realtek_misc_major, 0), NULL, "misc-ioctl");
	mutex_init(&misc_mutex);

	return 0;
fail_malloc:
	unregister_chrdev_region(devno, 1);
}

void realtek_misc_exit(void)
{
	pr_info("End realtek_misc");
	cdev_del(&realtek_misc_devp->cdev);
	kfree(realtek_misc_devp);
	unregister_chrdev_region(MKDEV(realtek_misc_major, 0), 1);
}

module_init(realtek_misc_init);
module_exit(realtek_misc_exit);

MODULE_DESCRIPTION("Realtek misc ioctl");
MODULE_AUTHOR("Realtek Corporation");
MODULE_LICENSE("GPL");
