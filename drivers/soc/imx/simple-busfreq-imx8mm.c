/*
 * Simple busfreq driver for the i.MX8MM.
 *
 * Copyright (C) 2019 Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/busfreq-imx.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/arm-smccc.h>

#define FSL_SIP_DDR_DVFS                0xc2000004
#define HIGH_FREQ			0x00
#define IMX_SIP_DDR_DVFS_GET_FREQ_INFO	0x11

#define HIGH_NOC_RATE		750000000
#define HIGH_AHB_RATE		133333333

#define LOW_NOC_RATE		150000000
#define LOW_AHB_RATE		22222222


struct simple_busfreq_data
{
	bool high;
	bool user_high;
	unsigned high_bus_count;
	u8 low_freq_index;

	struct clk *noc_div;
	struct clk *ahb_div;
	struct clk *main_axi_src;
	struct clk *osc_24m;
	struct clk *sys_pll2_333m;
	struct clk *dram_pll;

	struct device *dev;

	struct mutex bus_freq_mutex;
};

static struct simple_busfreq_data pdata;

/*
 * From the original busfreq-imx8mq.c implementation, but renamed
 * from `update_bus_freq()` to `update_ddr_freq()`.
 */
static void update_ddr_freq(int target)
{
	struct arm_smccc_res res;
	u32 online_cpus = 0;
	int cpu = 0;

	local_irq_disable();

	for_each_online_cpu(cpu) {
		online_cpus |= (1 << (cpu * 8));
	}
	/* change the ddr freqency */
	arm_smccc_smc(FSL_SIP_DDR_DVFS, target, online_cpus,
		0, 0, 0, 0, 0, &res);

	local_irq_enable();
}

/*
 * This sets the various bus frequencies to the following values:
 * * HIGH mode:
 *   * DDR:     (high freq, DDR4 = 2400 MT/s)
 *   * NOC:     750 MHz
 *   * AHB:     133 MHz
 *   * AXI:     333 MHz
 * * LOW mode:
 *   * DDR:    (low freq, DDR4 = 400 MT/s)
 *   * NOC:     150 MHz
 *   * AHB:     22 MHz
 *   * AXI:     25 MHz
 */
static void update_bus_freq(void)
{
	bool high = pdata.user_high || pdata.high_bus_count;

	if (high == pdata.high)
		return;

	if (high) {
		update_ddr_freq(HIGH_FREQ);
		clk_set_rate(pdata.noc_div, HIGH_NOC_RATE);
		clk_set_rate(pdata.ahb_div, HIGH_AHB_RATE);
		clk_set_parent(pdata.main_axi_src, pdata.sys_pll2_333m);
	} else {
		// Wait here a bit in high mode before returning
		// Seems like the caamrng can't handle immidiate switch to low freq mode
		usleep_range(50000, 100000);
		update_ddr_freq(pdata.low_freq_index);
		clk_set_rate(pdata.noc_div, LOW_NOC_RATE);
		clk_set_rate(pdata.ahb_div, LOW_AHB_RATE);
		clk_set_parent(pdata.main_axi_src, pdata.osc_24m);
	}

	pdata.high = high;
}

void request_bus_freq(enum bus_freq_mode mode)
{
	mutex_lock(&pdata.bus_freq_mutex);
	if (mode == BUS_FREQ_HIGH) {
		pdata.high_bus_count++;
	}
	update_bus_freq();
	mutex_unlock(&pdata.bus_freq_mutex);
}
EXPORT_SYMBOL(request_bus_freq);

void release_bus_freq(enum bus_freq_mode mode)
{
	mutex_lock(&pdata.bus_freq_mutex);
	if (mode == BUS_FREQ_HIGH) {
		if (pdata.high_bus_count == 0) {
			dev_err(pdata.dev, "high bus count mismatch!\n");
			dump_stack();
			mutex_unlock(&pdata.bus_freq_mutex);
			return;
		}
		pdata.high_bus_count--;
	}
	update_bus_freq();
	mutex_unlock(&pdata.bus_freq_mutex);
}
EXPORT_SYMBOL(release_bus_freq);

int get_bus_freq_mode(void)
{
	return pdata.high;
}
EXPORT_SYMBOL(get_bus_freq_mode);

static ssize_t is_high_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", pdata.high);
}

static DEVICE_ATTR_RO(is_high);

static ssize_t high_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", pdata.high_bus_count);
}

static DEVICE_ATTR_RO(high_count);

static ssize_t set_high_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", pdata.user_high);
}

static ssize_t set_high_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	mutex_lock(&pdata.bus_freq_mutex);
	if (strncmp(buf, "1", 1) == 0) 
		pdata.user_high = true;
	else if (strncmp(buf, "0", 1) == 0)
		pdata.user_high = false;
	else {
		mutex_unlock(&pdata.bus_freq_mutex);
		return -EINVAL;
	}

	update_bus_freq();
	mutex_unlock(&pdata.bus_freq_mutex);

	return size;
}
static DEVICE_ATTR(set_high, 0644, set_high_show, set_high_store);

static struct attribute *busfreq_attrs[] = {
	&dev_attr_is_high.attr,
	&dev_attr_high_count.attr,
	&dev_attr_set_high.attr,
	NULL,
};

static struct attribute_group busfreq_group = {
	.attrs = busfreq_attrs,
};

static int simple_busfreq_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	unsigned int fsp_table[4], i;
	struct arm_smccc_res res;

	memset(&pdata, 0, sizeof(pdata));
	mutex_init(&pdata.bus_freq_mutex);
	pdata.dev = dev;

	dev_set_drvdata(dev, &pdata);

#define CLK_GET(__clk, __name) \
	__clk = devm_clk_get(dev, __name); \
	if (IS_ERR(__clk)) { \
		dev_err(dev, "Failed to get %s clk.\n", __name); \
		return -EINVAL; \
	}

	CLK_GET(pdata.noc_div, "noc_div");
	CLK_GET(pdata.ahb_div, "ahb_div");
	CLK_GET(pdata.main_axi_src, "main_axi_src");
	CLK_GET(pdata.osc_24m, "osc_24m");
	CLK_GET(pdata.sys_pll2_333m, "sys_pll2_333m");

#undef CLK_GET

	/*
	 * Get the supported frequency, normally the lowest frequency point
	 * is used for low bus & audio bus mode.
	 */
	for (i = 0; i < 4; i++) {
		arm_smccc_smc(FSL_SIP_DDR_DVFS, IMX_SIP_DDR_DVFS_GET_FREQ_INFO,
			      i, 0, 0, 0, 0, 0, &res);
		ret = res.a0;
		if (ret < 0)
			return -EINVAL;

		fsp_table[i] = res.a0;
	}

	/* get the lowest fsp index */
	for (i = 0; i < 4; i++)
		if (fsp_table[i] == 0)
			break;

	pdata.low_freq_index = i - 1;

	pdata.user_high = true;
	update_bus_freq();

	ret = devm_device_add_group(dev, &busfreq_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs entry.\n");
		return ret;
	}

	return 0;
}

static int simple_busfreq_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	devm_device_remove_group(dev, &busfreq_group);
	
	pdata.user_high = true;
	update_bus_freq();

	return 0;
}

static const struct of_device_id simple_busfreq_dt_ids[] = {
	{ .compatible = "sue,simple-busfreq-imx8mm" },
	{}
};
MODULE_DEVICE_TABLE(of, simple_busfreq_dt_ids);

static struct platform_driver simple_busfreq_driver = {
	.probe		= simple_busfreq_probe,
	.remove		= simple_busfreq_remove,
	.driver		= {
		.name	= "simple-busfreq",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(simple_busfreq_dt_ids),
	},
};
module_platform_driver(simple_busfreq_driver);

MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_DESCRIPTION("Simple busfreq driver for the i.MX8MM");
MODULE_LICENSE("GPL v2");
