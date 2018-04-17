#include <asm/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include <misc/hwcounter-mx7.h>

#define MXC_GPT_REG_CR	0x00
#define MXC_GPT_REG_PR	0x04
#define MXC_GPT_REG_IR	0x0C
#define MXC_GPT_REG_CNT	0x24

#define MXC_GPT_CR_EN		(1 << 0) /* Enable the counter */
#define MXC_GPT_CR_ENMOD	(1 << 1) /* Clear counter on disable */
#define MXC_GPT_CR_DBGEN	(1 << 2) /* Keep counter enabled in debug mode */
#define MXC_GPT_CR_WAITEN	(1 << 3) /* Keep counter enabled in wait mode */
#define MXC_GPT_CR_DOZEEN	(1 << 4) /* Keep counter enabled in doze mode */
#define MXC_GPT_CR_STOPEN	(1 << 5) /* Keep counter enabled in stop mode */
#define MXC_GPT_CR_FRR		(1 << 9) /* Free running mode */

#define MXC_GPT_CR_CLKSRC_CLKIN (3 << 6)


inline u32 hwcounter_get_value(struct hwcounter_data *hwcounter)
{
	return ioread32(hwcounter->timer_base + MXC_GPT_REG_CNT);
}
EXPORT_SYMBOL_GPL(hwcounter_get_value);

static ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int written;
	struct hwcounter_data *pdata = (struct hwcounter_data *)dev_get_drvdata(dev);

	u32 gpio_counter = hwcounter_get_value(pdata);

	written = scnprintf(buf, PAGE_SIZE, "%u\n", gpio_counter);

	return written;
}
DEVICE_ATTR(value, 0400, value_show, NULL);

static int hwcounter_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hwcounter_data *pdata;
	u32 reg;

	pdata = devm_kzalloc(dev, sizeof(struct hwcounter_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->timer_base = of_iomap(dev->of_node, 0);
	if (!pdata->timer_base) {
		dev_err(dev, "failed of_iomap() call\n");
		return -ENXIO;
	}
	dev_info(dev, "timer_base is %p\n", pdata->timer_base);

	pdata->clk_per = of_clk_get_by_name(dev->of_node, "per");
	if (IS_ERR(pdata->clk_per)) {
		iounmap(pdata->timer_base);
		dev_err(dev, "failed to get per clk\n");
		return -ENXIO;
	}
	clk_prepare_enable(pdata->clk_per);

	/* Set configuration to 0 */
	iowrite32(0, pdata->timer_base + MXC_GPT_REG_CR);
	/* Set prescaler to 0 */
	iowrite32(0, pdata->timer_base + MXC_GPT_REG_PR);
	/* Disable all interrupts */
	iowrite32(0, pdata->timer_base + MXC_GPT_REG_IR);

	reg = 0;
	reg |= MXC_GPT_CR_FRR | MXC_GPT_CR_CLKSRC_CLKIN | MXC_GPT_CR_ENMOD;
	reg |= MXC_GPT_CR_STOPEN | MXC_GPT_CR_DOZEEN | MXC_GPT_CR_WAITEN | MXC_GPT_CR_DBGEN;

	iowrite32(reg, pdata->timer_base + MXC_GPT_REG_CR);

	msleep(5);

	reg |= MXC_GPT_CR_EN;
	iowrite32(reg, pdata->timer_base + MXC_GPT_REG_CR);

	dev_info(dev, "timer configured\n");

	device_create_file(dev, &dev_attr_value);

	dev_set_drvdata(dev, pdata);

	return 0;
}

static int hwcounter_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hwcounter_data *pdata = (struct hwcounter_data *)dev_get_drvdata(dev);

	dev_warn(dev, "Unloading module, do not access the value register throught /dev/mem anymore or your system will hang\n");
	device_remove_file(dev, &dev_attr_value);

	/* Set configuration to 0, thus disabling the counter */
	iowrite32(0, pdata->timer_base + MXC_GPT_REG_CR);

	clk_disable_unprepare(pdata->clk_per);

	iounmap(pdata->timer_base);

	return 0;
}

static const struct of_device_id hwcounter_dt_ids[] = {
	{ .compatible = "sue,hwcounter-mx7" },
	{}
};

MODULE_DEVICE_TABLE(of, hwcounter_dt_ids);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");

static struct platform_driver hwcounter_driver = {
	.probe		= hwcounter_probe,
	.remove		= hwcounter_remove,
	.driver		= {
		.name	= "sue_hwcounter",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(hwcounter_dt_ids),
	},
};

module_platform_driver(hwcounter_driver);
