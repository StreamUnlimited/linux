/*
 * Driver for the MA12040 Audio Amplifier
 *
 * Author: André Groenewald <andre.groenewald@streamunlimited.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#define MA12040_DRV_NAME 		"ma12040"

#define MA12040_PMC_REG 		0x00
#define MA12040_EHC_REG 		0x2d
#define MA12040_MRC0_REG 		0x60
#define MA12040_ERROR_REG 		0x7c

#define MA12040_MAX_REG 		0x7f

#define MA12040_POLLING_INTERVAL 	1000 /* msec */

static const struct reg_sequence ma12040_init_sequence[] = {
	/* enable audio input mode overwrite (enables writes to register 0x25) */
	{ 0x27, 0x28 },
	/* change audio input mode to 26dB (increases maximum volume by 6dB) */
	{ 0x25, 0x10 },
	/* enable power mode profile PMP01 (Filterfree: optimized audio
	 * performance, active speaker applications) */
	{ 0x1d, 0x01 },
};

struct ma12040_data {
	struct regmap *regmap;
	struct work_struct cleanup_task;
	struct snd_soc_component *component;
#ifdef CONFIG_SYSFS
	struct workqueue_struct *polling_queue;
	struct delayed_work polling_work;
	bool polling;
	int faults;
#endif
};

/* The address space from 0x00 to 0x50 holds read/write registers and the
 * address space from 0x60 to 0x7f are read only as specified in the datasheet.
 * However only up to 0x2d is specified (r/w) in the regmap and only up to 0x7c
 * is specified for read only */

static const struct regmap_range ma12040_wr_yes_range[] = {
	regmap_reg_range(MA12040_PMC_REG, MA12040_EHC_REG),
};

static const struct regmap_access_table ma12040_wr_table = {
	.yes_ranges = ma12040_wr_yes_range,
	.n_yes_ranges = ARRAY_SIZE(ma12040_wr_yes_range),
};

static const struct regmap_range ma12040_rd_yes_range[] = {
	regmap_reg_range(MA12040_PMC_REG, MA12040_ERROR_REG),
};

static const struct regmap_access_table ma12040_rd_table = {
	.yes_ranges = ma12040_rd_yes_range,
	.n_yes_ranges = ARRAY_SIZE(ma12040_rd_yes_range),
};

static const struct regmap_range ma12040_volatile_yes_range[] = {
	regmap_reg_range(MA12040_MRC0_REG, MA12040_ERROR_REG),
};

static const struct regmap_access_table ma12040_volatile_table = {
	.yes_ranges = ma12040_volatile_yes_range,
	.n_yes_ranges = ARRAY_SIZE(ma12040_volatile_yes_range),
};

static const struct regmap_config ma12040_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.max_register = MA12040_MAX_REG,
	.wr_table = &ma12040_wr_table,
	.rd_table = &ma12040_rd_table,
	.volatile_table = &ma12040_volatile_table,
};

static int ma12040_dai_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
#ifdef CONFIG_SYSFS
	struct ma12040_data *ma12040 = snd_soc_component_get_drvdata(dai->component);

	ma12040->faults = 0;
	ma12040->polling = true;

	queue_delayed_work(ma12040->polling_queue, &ma12040->polling_work,
			msecs_to_jiffies(MA12040_POLLING_INTERVAL));
#endif

	dev_dbg(component->dev, "dai startup\n");

	return 0;
}

static void ma12040_dai_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
#ifdef CONFIG_SYSFS
	struct ma12040_data *ma12040 = snd_soc_component_get_drvdata(dai->component);

	ma12040->polling = false;
	cancel_delayed_work_sync(&ma12040->polling_work);
#endif

	dev_dbg(component->dev, "dai shutdown\n");
}

static const struct snd_soc_dai_ops ma12040_dai_ops = {
	.startup		= ma12040_dai_startup,
	.shutdown		= ma12040_dai_shutdown,
};

static struct snd_soc_dai_driver ma12040_dai = {
	.name		= MA12040_DRV_NAME,
	.id       = 0,
	.playback	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S24_LE
	},
	.ops = &ma12040_dai_ops,
};

#ifdef CONFIG_SYSFS
static void ma12040_polling_handler(struct work_struct *work)
{
	struct ma12040_data *ma12040 =
		container_of(work, struct ma12040_data, polling_work.work);
	int faults = snd_soc_component_read(ma12040->component, MA12040_ERROR_REG);

	if (faults != ma12040->faults) {
		ma12040->faults = faults;
		sysfs_notify(&ma12040->component->dev->kobj, NULL, "fault_list");
	}

	if (ma12040->polling)
		queue_delayed_work(ma12040->polling_queue,
				&ma12040->polling_work,
				msecs_to_jiffies(MA12040_POLLING_INTERVAL));
}

const char * const ma12040_faults[] = {
	"FC over-voltage err",
	"over-current",
	"PLL err",
	"PVDD under-voltage",
	"over-temp warn",
	"over-temp err",
	"P2P low impedance",
	"DC protection",
};

static ssize_t fault_list_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ma12040_data *ma12040 = dev_get_drvdata(dev);
	int faults = (u8)ma12040->faults;
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(ma12040_faults); i++) {
		if (faults & BIT(i)) {
			int res = sprintf(&buf[ret], "%s\n", ma12040_faults[i]);
			if (res < 0)
				break;
			ret += res;
		}
	}

	return ret;
}
static DEVICE_ATTR_RO(fault_list);

static struct attribute *ma12040_fault_attrs[] = {
	&dev_attr_fault_list.attr,
	NULL,
};

static struct attribute_group ma12040_fault_group = {
	.attrs = ma12040_fault_attrs,
};

#endif

static int ma12040_codec_probe(struct snd_soc_component *component)
{
	struct ma12040_data *ma12040 = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	int ret;

	ma12040->component = component;

#ifdef CONFIG_SYSFS
	ma12040->polling_queue = create_singlethread_workqueue("ma12040");
	if (ma12040->polling_queue == NULL) {
		dev_err(dev, "Failed to create workqueue\n");
		return -EINVAL;
	}
	INIT_DELAYED_WORK(&ma12040->polling_work, ma12040_polling_handler);

	ret = sysfs_create_group(&dev->kobj, &ma12040_fault_group);
	if (ret != 0) {
		dev_err(dev, "failed to create sysfs group: %d\n", ret);
		return ret;
	}
#endif

	ret = regmap_register_patch(component->regmap,
			ma12040_init_sequence,
			ARRAY_SIZE(ma12040_init_sequence));
	if (ret != 0) {
		dev_err(component->dev, "failed to write init sequence: %d\n", ret);
		return ret;
	}

	dev_dbg(dev, "%s ok\n", __func__);

	return 0;
}

static void ma12040_codec_remove(struct snd_soc_component *component)
{
#ifdef CONFIG_SYSFS
	struct ma12040_data *ma12040 = snd_soc_component_get_drvdata(component);

	ma12040->polling = false;
	cancel_delayed_work_sync(&ma12040->polling_work);
	flush_workqueue(ma12040->polling_queue);
	destroy_workqueue(ma12040->polling_queue);

	sysfs_remove_group(&component->dev->kobj, &ma12040_fault_group);
#endif
};

static const struct snd_soc_dapm_widget ma120x0_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("OUT_A"),
	SND_SOC_DAPM_OUTPUT("OUT_B"),
};

static const struct snd_soc_dapm_route ma120x0_dapm_routes[] = {
	{ "OUT_B",  NULL, "Playback" },
	{ "OUT_A",  NULL, "Playback" },
};

static struct snd_soc_component_driver soc_codec_ma12040 = {
	.name             = MA12040_DRV_NAME,
	.dapm_widgets		= ma120x0_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(ma120x0_dapm_widgets),
	.dapm_routes		= ma120x0_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(ma120x0_dapm_routes),
	.probe = ma12040_codec_probe,
	.remove = ma12040_codec_remove,
};

static int ma12040_i2c_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct ma12040_data *ma12040;
	int ret;

	ma12040 = devm_kzalloc(dev, sizeof(struct ma12040_data),
			GFP_KERNEL);
	if (!ma12040)
		return -ENOMEM;

	i2c_set_clientdata(client, ma12040);
	dev_set_drvdata(dev, ma12040);

	ma12040->regmap = devm_regmap_init_i2c(client, &ma12040_regmap);
	if (IS_ERR(ma12040->regmap)) {
		ret = PTR_ERR(ma12040->regmap);
		dev_err(dev, "failed to allocate register map: %d\n", ret);
		return ret;
	}

	ret = devm_snd_soc_register_component(dev, &soc_codec_ma12040, &ma12040_dai, 1);
	if (ret != 0) {
		dev_err(dev, "failed to register component: %d\n", ret);
		return ret;
	}

	return 0;
}

static void ma12040_i2c_remove(struct i2c_client *client) {}

static const struct i2c_device_id ma12040_i2c_id[] = {
	{ "ma12040", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ma12040_i2c_id);

#ifdef CONFIG_OF
static const struct of_device_id ma12040_of_match[] = {
	{ .compatible = "infineon,ma12040", },
	{ }
};
MODULE_DEVICE_TABLE(of, ma12040_of_match);
#endif

static struct i2c_driver ma12040_i2c_driver = {
	.probe 		= ma12040_i2c_probe,
	.remove 	= ma12040_i2c_remove,
	.id_table	= ma12040_i2c_id,
	.driver		= {
		.name	= MA12040_DRV_NAME,
		.of_match_table = ma12040_of_match,
	},
};

module_i2c_driver(ma12040_i2c_driver);

MODULE_AUTHOR("André Groenewald <andre.groenewald@streamunlimited.com>");
MODULE_DESCRIPTION("MA12040 Audio Amplifier Driver");
MODULE_LICENSE("GPL v2");
