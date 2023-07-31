// SPDX-License-Identifier: GPL-2.0
//
// Stream Unlimited Stream195x ASoC driver
// Based on simple-card.c
//
// Copyright (C) 2012 Renesas Solutions Corp.
// Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
// Copyright (C) 2020 StreamUnlimited Engineering GmbH
// Quentin Schulz <quentin.schulz@streamunlimited.com>

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <sound/simple_card.h>
#include <sound/soc-dai.h>
#include <sound/soc.h>

/* START SUE addition */
#include <linux/gpio/consumer.h>
#include "../fsl/fsl_sai.h"
struct snd_soc_stream195x_dai_link_data {
	struct gpio_desc *mute_gpio;
};

/* All instances of asoc_simple_priv have been replcaed with stream195x_simple_priv */
/* Stolen from asoc_simple_priv struct */
struct stream195x_simple_priv {
	struct snd_soc_card snd_card;
	struct simple_dai_props *dai_props;
	struct asoc_simple_jack hp_jack;
	struct asoc_simple_jack mic_jack;
	struct snd_soc_dai_link *dai_link;
	struct asoc_simple_dai *dais;
	struct snd_soc_codec_conf *codec_conf;
	struct gpio_desc *pa_gpio;
	/* START SUE addition 2 */
	struct clk *pll8k_clk;
	struct clk *pll11k_clk;
	int cur_ppm;
	struct gpio_desc *powerdown_gpio;
	struct snd_soc_stream195x_dai_link_data *dai_link_data;
	/* END SUE addition 2 */
};

#define MCLK_RATE_48k	(24576000UL)
#define MCLK_RATE_44k1	(22579200UL)

#define PLL_NOMINAL_RATE_48k	(786432000UL)
#define PLL_NOMINAL_RATE_44k1	(722534400UL)

static int snd_soc_stream195x_ppm_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->value.integer.min = -500;
	uinfo->value.integer.max = 500;
	uinfo->count = 1;

	return 0;
}

static int snd_soc_stream195x_ppm_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct stream195x_simple_priv *priv = snd_soc_card_get_drvdata(card);

	ucontrol->value.integer.value[0] = priv->cur_ppm;

	return 0;
}

static int snd_soc_stream195x_ppm_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct stream195x_simple_priv *priv = snd_soc_card_get_drvdata(card);

	int ret;
	unsigned int comp;
	unsigned int clk_rate;
	int ppm = ucontrol->value.integer.value[0];

	comp = DIV_ROUND_CLOSEST_ULL((u64)PLL_NOMINAL_RATE_48k * abs(ppm), 1000000UL);
	clk_rate = PLL_NOMINAL_RATE_48k;
	if (ppm > 0)
		clk_rate += comp;
	else
		clk_rate -= comp;

	ret = clk_set_rate(priv->pll8k_clk, clk_rate);
	if (ret)
		dev_warn(card->dev, "failed to set pll8k rate %d\n", ret);


	comp = DIV_ROUND_CLOSEST_ULL((u64)PLL_NOMINAL_RATE_44k1 * abs(ppm), 1000000UL);
	clk_rate = PLL_NOMINAL_RATE_44k1;
	if (ppm > 0)
		clk_rate += comp;
	else
		clk_rate -= comp;

	ret = clk_set_rate(priv->pll11k_clk, clk_rate);
	if (ret)
		dev_warn(card->dev, "failed to set pll11k rate %d\n", ret);

	priv->cur_ppm = ppm;

	return 1;
}

static const struct snd_kcontrol_new snd_soc_stream195x_controls[] = {
	{
		.name = "Drift compensator",
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.info = snd_soc_stream195x_ppm_info,
		.get = snd_soc_stream195x_ppm_get,
		.put = snd_soc_stream195x_ppm_put,
	},
};

static void snd_soc_stream195x_set_powerdown(struct stream195x_simple_priv *priv, int value)
{
	if (priv->powerdown_gpio)
		gpiod_set_value(priv->powerdown_gpio, value);
}

static void snd_soc_stream195x_set_all_links_mute(struct stream195x_simple_priv *priv, int value)
{
	int i;
	struct snd_soc_card *card = simple_priv_to_card(priv);

	for (i = 0; i < card->num_links; i++) {
		struct snd_soc_stream195x_dai_link_data *dai_link_data = priv->dai_link_data + i;
		if (dai_link_data->mute_gpio)
			gpiod_set_value_cansleep(dai_link_data->mute_gpio, value);
	}
}

static int snd_soc_stream195x_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	int ret;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct stream195x_simple_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_dai *cpu_dai = asoc_rtd_to_cpu(rtd, 0);
	struct simple_dai_props *dai_props = simple_priv_to_props(priv, rtd->num);

	unsigned int rate = params_rate(params);
	unsigned int mclk_rate, pll_rate;
	struct clk *pll;

	if ((rate % 8000) == 0) {
		pll_rate = PLL_NOMINAL_RATE_48k;
		pll = priv->pll8k_clk;
		mclk_rate = MCLK_RATE_48k;
	} else {
		pll_rate = PLL_NOMINAL_RATE_44k1;
		pll = priv->pll11k_clk;
		mclk_rate = MCLK_RATE_44k1;
	}

	/*
	 * We could have done the tdm slot setup only once, and not in every hw_params()
	 * call, however sooner or later we will support DSD where the tdm slot setup
	 * will change at runtime, so let's just add it here.
	 */
	ret = snd_soc_dai_set_tdm_slot(codec_dai, BIT(params_channels(params)) - 1, BIT(params_channels(params)) - 1, dai_props->codec_dai->slots, dai_props->codec_dai->slot_width);
	if (ret && ret != -ENOTSUPP)
		return ret;

	ret = snd_soc_dai_set_tdm_slot(cpu_dai, 0, 0, dai_props->cpu_dai->slots, dai_props->cpu_dai->slot_width);
	if (ret && ret != -ENOTSUPP)
		return ret;


	/*
	 * First we reset the PLL rate to the nominal value, otherwise, when
	 * the PLL is in a state where the frequency is skewed, the
	 * calculations inside snd_soc_dai_set_sysclk() might fail, because
	 * the skewed PLL value cannot be cleanly divided down anymore.
	 */
	priv->cur_ppm = 0;
	ret = clk_set_rate(pll, pll_rate);
	if (ret)
		return ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	if (ret && ret != -ENOTSUPP)
		return ret;

	/* TODO: make the id parameter configurable via the device-tree */
	ret = snd_soc_dai_set_sysclk(cpu_dai, FSL_SAI_CLK_MAST1, mclk_rate, SND_SOC_CLOCK_OUT);
	if (ret && ret != -ENOTSUPP)
		return ret;


	return 0;
}

/*
 * This is a wrapper around devm_gpiod_get_from_of_node() to assemble
 * a nice gpio label based on dai_link->codecs->dai_name.
 */
static struct gpio_desc *devm_dailink_gpiod_get_from_of_node(struct device *dev, struct snd_soc_dai_link *dai_link,
								struct device_node *np, const char *name, enum gpiod_flags flags)
{
	char buf[32], *gpio_label;

	snprintf(buf, sizeof(buf), "%s-%s", dai_link->codecs->dai_name, name);
	gpio_label = devm_kstrdup(dev, buf, GFP_KERNEL);

	return devm_gpiod_get_from_of_node(dev, np, name, 0, flags, gpio_label);
}
/* END SUE addition */

#define DAI	"sound-dai"
#define CELL	"#sound-dai-cells"
#define PREFIX	"sue-card,"

static const struct snd_soc_ops simple_ops = {
	.startup	= asoc_simple_startup,
	.shutdown	= asoc_simple_shutdown,
	/* START SUE changes */
	/* .hw_params	= asoc_simple_hw_params, */
	.hw_params	= snd_soc_stream195x_hw_params,
	/* END SUE changes */
};

static int asoc_simple_parse_platform(struct device_node *node,
				      struct snd_soc_dai_link_component *dlc)
{
	struct of_phandle_args args;
	int ret;

	if (!node)
		return 0;

	/*
	 * Get node via "sound-dai = <&phandle port>"
	 * it will be used as xxx_of_node on soc_bind_dai_link()
	 */
	ret = of_parse_phandle_with_args(node, DAI, CELL, 0, &args);
	if (ret)
		return ret;

	/* dai_name is not required and may not exist for plat component */

	dlc->of_node = args.np;

	return 0;
}

static int asoc_simple_parse_dai(struct device_node *node,
				 struct snd_soc_dai_link_component *dlc,
				 int *is_single_link)
{
	struct of_phandle_args args;
	int ret;

	if (!node)
		return 0;

	/*
	 * Get node via "sound-dai = <&phandle port>"
	 * it will be used as xxx_of_node on soc_bind_dai_link()
	 */
	ret = of_parse_phandle_with_args(node, DAI, CELL, 0, &args);
	if (ret)
		return ret;

	/*
	 * FIXME
	 *
	 * Here, dlc->dai_name is pointer to CPU/Codec DAI name.
	 * If user unbinded CPU or Codec driver, but not for Sound Card,
	 * dlc->dai_name is keeping unbinded CPU or Codec
	 * driver's pointer.
	 *
	 * If user re-bind CPU or Codec driver again, ALSA SoC will try
	 * to rebind Card via snd_soc_try_rebind_card(), but because of
	 * above reason, it might can't bind Sound Card.
	 * Because Sound Card is pointing to released dai_name pointer.
	 *
	 * To avoid this rebind Card issue,
	 * 1) It needs to alloc memory to keep dai_name eventhough
	 *    CPU or Codec driver was unbinded, or
	 * 2) user need to rebind Sound Card everytime
	 *    if he unbinded CPU or Codec.
	 */
	ret = snd_soc_of_get_dai_name(node, &dlc->dai_name);
	if (ret < 0)
		return ret;

	dlc->of_node = args.np;

	if (is_single_link)
		*is_single_link = !args.args_count;

	return 0;
}

static void simple_parse_convert(struct device *dev,
				 struct device_node *np,
				 struct asoc_simple_data *adata)
{
	struct device_node *top = dev->of_node;
	struct device_node *node = of_get_parent(np);

	asoc_simple_parse_convert(top,  PREFIX, adata);
	asoc_simple_parse_convert(node, PREFIX, adata);
	asoc_simple_parse_convert(node, NULL,   adata);
	asoc_simple_parse_convert(np,   NULL,   adata);

	of_node_put(node);
}

static void simple_parse_mclk_fs(struct device_node *top,
				 struct device_node *np,
				 struct simple_dai_props *props,
				 char *prefix)
{
	struct device_node *node = of_get_parent(np);
	char prop[128];

	snprintf(prop, sizeof(prop), "%smclk-fs", PREFIX);
	of_property_read_u32(top,	prop, &props->mclk_fs);

	snprintf(prop, sizeof(prop), "%smclk-fs", prefix);
	of_property_read_u32(node,	prop, &props->mclk_fs);
	of_property_read_u32(np,	prop, &props->mclk_fs);

	of_node_put(node);
}

static int simple_parse_node(struct stream195x_simple_priv *priv,
			     struct device_node *np,
			     struct link_info *li,
			     char *prefix,
			     int *cpu)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct device_node *top = dev->of_node;
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, li->link);
	struct simple_dai_props *dai_props = simple_priv_to_props(priv, li->link);
	struct snd_soc_dai_link_component *dlc;
	struct asoc_simple_dai *dai;
	int ret;

	if (cpu) {
		dlc = asoc_link_to_cpu(dai_link, 0);
		dai = simple_props_to_dai_cpu(dai_props, 0);
	} else {
		dlc = asoc_link_to_codec(dai_link, 0);
		dai = simple_props_to_dai_codec(dai_props, 0);
	}

	simple_parse_mclk_fs(top, np, dai_props, prefix);

	ret = asoc_simple_parse_dai(np, dlc, cpu);
	if (ret)
		return ret;

	ret = asoc_simple_parse_clk(dev, np, dai, dlc);
	if (ret)
		return ret;

	ret = asoc_simple_parse_tdm(np, dai);
	if (ret)
		return ret;

	return 0;
}

static int simple_link_init(struct stream195x_simple_priv *priv,
			    struct device_node *node,
			    struct device_node *codec,
			    struct link_info *li,
			    char *prefix, char *name)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, li->link);
	int ret;

	ret = asoc_simple_parse_daifmt(dev, node, codec,
				       prefix, &dai_link->dai_fmt);
	if (ret < 0)
		return 0;

	dai_link->init			= asoc_simple_dai_init;
	dai_link->ops			= &simple_ops;

	return asoc_simple_set_dailink_name(dev, dai_link, name);
}

static int simple_dai_link_of_dpcm(struct stream195x_simple_priv *priv,
				   struct device_node *np,
				   struct device_node *codec,
				   struct link_info *li,
				   bool is_top)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, li->link);
	struct simple_dai_props *dai_props = simple_priv_to_props(priv, li->link);
	struct device_node *top = dev->of_node;
	struct device_node *node = of_get_parent(np);
	/* START SUE addition */
	struct snd_soc_stream195x_dai_link_data *dai_link_data = priv->dai_link_data + li->link;
	/* END SUE addition */
	char *prefix = "";
	char dai_name[64];
	int ret;

	dev_dbg(dev, "link_of DPCM (%pOF)\n", np);

	/* START SUE additions */
	dai_link_data->mute_gpio = devm_dailink_gpiod_get_from_of_node(dev, dai_link, np, "mute-gpios", GPIOD_OUT_HIGH);
	if (IS_ERR(dai_link_data->mute_gpio))
		dai_link_data->mute_gpio = NULL;
	/* END SUE additions */

	/* For single DAI link & old style of DT node */
	if (is_top)
		prefix = PREFIX;

	if (li->cpu) {
		struct snd_soc_dai_link_component *cpus = asoc_link_to_cpu(dai_link, 0);
		struct snd_soc_dai_link_component *platforms = asoc_link_to_platform(dai_link, 0);
		int is_single_links = 0;

		/* Codec is dummy */

		/* FE settings */
		dai_link->dynamic		= 1;
		dai_link->dpcm_merged_format	= 1;

		ret = simple_parse_node(priv, np, li, prefix, &is_single_links);
		if (ret < 0)
			goto out_put_node;

		snprintf(dai_name, sizeof(dai_name), "fe.%s", cpus->dai_name);

		asoc_simple_canonicalize_cpu(cpus, is_single_links);
		asoc_simple_canonicalize_platform(platforms, cpus);
	} else {
		struct snd_soc_dai_link_component *codecs = asoc_link_to_codec(dai_link, 0);
		struct snd_soc_codec_conf *cconf;

		/* CPU is dummy */

		/* BE settings */
		dai_link->no_pcm		= 1;
		dai_link->be_hw_params_fixup	= asoc_simple_be_hw_params_fixup;

		cconf	= simple_props_to_codec_conf(dai_props, 0);

		ret = simple_parse_node(priv, np, li, prefix, NULL);
		if (ret < 0)
			goto out_put_node;

		snprintf(dai_name, sizeof(dai_name), "be.%s", codecs->dai_name);

		/* check "prefix" from top node */
		snd_soc_of_parse_node_prefix(top, cconf, codecs->of_node,
					      PREFIX "prefix");
		snd_soc_of_parse_node_prefix(node, cconf, codecs->of_node,
					     "prefix");
		snd_soc_of_parse_node_prefix(np, cconf, codecs->of_node,
					     "prefix");
	}

	simple_parse_convert(dev, np, &dai_props->adata);

	snd_soc_dai_link_set_capabilities(dai_link);

	ret = simple_link_init(priv, node, codec, li, prefix, dai_name);

out_put_node:
	li->link++;

	of_node_put(node);
	return ret;
}

static int simple_dai_link_of(struct stream195x_simple_priv *priv,
			      struct device_node *np,
			      struct device_node *codec,
			      struct link_info *li,
			      bool is_top)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct snd_soc_dai_link *dai_link = simple_priv_to_link(priv, li->link);
	struct snd_soc_dai_link_component *cpus = asoc_link_to_cpu(dai_link, 0);
	struct snd_soc_dai_link_component *codecs = asoc_link_to_codec(dai_link, 0);
	struct snd_soc_dai_link_component *platforms = asoc_link_to_platform(dai_link, 0);
	struct device_node *cpu = NULL;
	struct device_node *node = NULL;
	struct device_node *plat = NULL;
	/* START SUE addition */
	struct snd_soc_stream195x_dai_link_data *dai_link_data = priv->dai_link_data + li->link;
	/* END SUE addition */
	char dai_name[64];
	char prop[128];
	char *prefix = "";
	int ret, single_cpu = 0;

	cpu  = np;
	node = of_get_parent(np);

	/* START SUE additions */
	dai_link_data->mute_gpio = devm_dailink_gpiod_get_from_of_node(dev, dai_link, node, "mute-gpios", GPIOD_OUT_HIGH);
	if (IS_ERR(dai_link_data->mute_gpio))
		dai_link_data->mute_gpio = NULL;
	/* END SUE additions */

	dev_dbg(dev, "link_of (%pOF)\n", node);

	/* For single DAI link & old style of DT node */
	if (is_top)
		prefix = PREFIX;

	snprintf(prop, sizeof(prop), "%splat", prefix);
	plat = of_get_child_by_name(node, prop);

	ret = simple_parse_node(priv, cpu, li, prefix, &single_cpu);
	if (ret < 0)
		goto dai_link_of_err;

	ret = simple_parse_node(priv, codec, li, prefix, NULL);
	if (ret < 0)
		goto dai_link_of_err;

	ret = asoc_simple_parse_platform(plat, platforms);
	if (ret < 0)
		goto dai_link_of_err;

	snprintf(dai_name, sizeof(dai_name),
		 "%s-%s", cpus->dai_name, codecs->dai_name);

	asoc_simple_canonicalize_cpu(cpus, single_cpu);
	asoc_simple_canonicalize_platform(platforms, cpus);

	ret = simple_link_init(priv, node, codec, li, prefix, dai_name);

dai_link_of_err:
	of_node_put(plat);
	of_node_put(node);

	li->link++;

	return ret;
}

static int __simple_for_each_link(struct stream195x_simple_priv *priv,
			struct link_info *li,
			int (*func_noml)(struct stream195x_simple_priv *priv,
					 struct device_node *np,
					 struct device_node *codec,
					 struct link_info *li, bool is_top),
			int (*func_dpcm)(struct stream195x_simple_priv *priv,
					 struct device_node *np,
					 struct device_node *codec,
					 struct link_info *li, bool is_top))
{
	struct device *dev = simple_priv_to_dev(priv);
	struct device_node *top = dev->of_node;
	struct device_node *node;
	uintptr_t dpcm_selectable = (uintptr_t)of_device_get_match_data(dev);
	bool is_top = 0;
	int ret = 0;

	/* Check if it has dai-link */
	node = of_get_child_by_name(top, PREFIX "dai-link");
	if (!node) {
		node = of_node_get(top);
		is_top = 1;
	}

	/* loop for all dai-link */
	do {
		struct asoc_simple_data adata;
		struct device_node *codec;
		struct device_node *plat;
		struct device_node *np;
		int num = of_get_child_count(node);

		/* get codec */
		codec = of_get_child_by_name(node, is_top ?
					     PREFIX "codec" : "codec");
		if (!codec) {
			ret = -ENODEV;
			goto error;
		}
		/* get platform */
		plat = of_get_child_by_name(node, is_top ?
					    PREFIX "plat" : "plat");

		/* get convert-xxx property */
		memset(&adata, 0, sizeof(adata));
		for_each_child_of_node(node, np)
			simple_parse_convert(dev, np, &adata);

		/* loop for all CPU/Codec node */
		for_each_child_of_node(node, np) {
			if (plat == np)
				continue;
			/*
			 * It is DPCM
			 * if it has many CPUs,
			 * or has convert-xxx property
			 */
			if (dpcm_selectable &&
			    (num > 2 || asoc_simple_is_convert_required(&adata))) {
				/*
				 * np
				 *	 |1(CPU)|0(Codec)  li->cpu
				 * CPU	 |Pass  |return
				 * Codec |return|Pass
				 */
				if (li->cpu != (np == codec))
					ret = func_dpcm(priv, np, codec, li, is_top);
			/* else normal sound */
			} else {
				/*
				 * np
				 *	 |1(CPU)|0(Codec)  li->cpu
				 * CPU	 |Pass  |return
				 * Codec |return|return
				 */
				if (li->cpu && (np != codec))
					ret = func_noml(priv, np, codec, li, is_top);
			}

			if (ret < 0) {
				of_node_put(codec);
				of_node_put(np);
				goto error;
			}
		}

		of_node_put(codec);
		node = of_get_next_child(top, node);
	} while (!is_top && node);

 error:
	of_node_put(node);
	return ret;
}

static int simple_for_each_link(struct stream195x_simple_priv *priv,
				struct link_info *li,
				int (*func_noml)(struct stream195x_simple_priv *priv,
						 struct device_node *np,
						 struct device_node *codec,
						 struct link_info *li, bool is_top),
				int (*func_dpcm)(struct stream195x_simple_priv *priv,
						 struct device_node *np,
						 struct device_node *codec,
						 struct link_info *li, bool is_top))
{
	int ret;
	/*
	 * Detect all CPU first, and Detect all Codec 2nd.
	 *
	 * In Normal sound case, all DAIs are detected
	 * as "CPU-Codec".
	 *
	 * In DPCM sound case,
	 * all CPUs   are detected as "CPU-dummy", and
	 * all Codecs are detected as "dummy-Codec".
	 * To avoid random sub-device numbering,
	 * detect "dummy-Codec" in last;
	 */
	for (li->cpu = 1; li->cpu >= 0; li->cpu--) {
		ret = __simple_for_each_link(priv, li, func_noml, func_dpcm);
		if (ret < 0)
			break;
	}

	return ret;
}

static int simple_parse_of(struct stream195x_simple_priv *priv, struct link_info *li)
{
	struct snd_soc_card *card = simple_priv_to_card(priv);
	int ret;

	ret = asoc_simple_parse_widgets(card, PREFIX);
	if (ret < 0)
		return ret;

	ret = asoc_simple_parse_routing(card, PREFIX);
	if (ret < 0)
		return ret;

	/* START SUE modifications */
	/* Not wanted as it can set controls making the addition of Drift
	 * Compensator more complex */
	/* ret = asoc_simple_parse_pin_switches(card, PREFIX); */
	/* if (ret < 0) */
	/* 	return ret; */
	/* STOP SUE modifications */

	ret = asoc_simple_parse_pin_switches(card, PREFIX);
	if (ret < 0)
		return ret;

	/* Single/Muti DAI link(s) & New style of DT node */
	memset(li, 0, sizeof(*li));
	ret = simple_for_each_link(priv, li,
				   simple_dai_link_of,
				   simple_dai_link_of_dpcm);
	if (ret < 0)
		return ret;

	ret = asoc_simple_parse_card_name(card, PREFIX);
	if (ret < 0)
		return ret;

	ret = snd_soc_of_parse_aux_devs(card, PREFIX "aux-devs");

	return ret;
}

static int simple_count_noml(struct stream195x_simple_priv *priv,
			     struct device_node *np,
			     struct device_node *codec,
			     struct link_info *li, bool is_top)
{
	if (li->link >= SNDRV_MAX_LINKS) {
		struct device *dev = simple_priv_to_dev(priv);

		dev_err(dev, "too many links\n");
		return -EINVAL;
	}

	li->num[li->link].cpus		= 1;
	li->num[li->link].codecs	= 1;
	li->num[li->link].platforms	= 1;

	li->link += 1;

	return 0;
}

static int simple_count_dpcm(struct stream195x_simple_priv *priv,
			     struct device_node *np,
			     struct device_node *codec,
			     struct link_info *li, bool is_top)
{
	if (li->link >= SNDRV_MAX_LINKS) {
		struct device *dev = simple_priv_to_dev(priv);

		dev_err(dev, "too many links\n");
		return -EINVAL;
	}

	if (li->cpu) {
		li->num[li->link].cpus		= 1;
		li->num[li->link].platforms	= 1;

		li->link++; /* CPU-dummy */
	} else {
		li->num[li->link].codecs	= 1;

		li->link++; /* dummy-Codec */
	}

	return 0;
}

static int simple_get_dais_count(struct stream195x_simple_priv *priv,
				 struct link_info *li)
{
	struct device *dev = simple_priv_to_dev(priv);
	struct device_node *top = dev->of_node;

	/*
	 * link_num :	number of links.
	 *		CPU-Codec / CPU-dummy / dummy-Codec
	 * dais_num :	number of DAIs
	 * ccnf_num :	number of codec_conf
	 *		same number for "dummy-Codec"
	 *
	 * ex1)
	 * CPU0 --- Codec0	link : 5
	 * CPU1 --- Codec1	dais : 7
	 * CPU2 -/		ccnf : 1
	 * CPU3 --- Codec2
	 *
	 *	=> 5 links = 2xCPU-Codec + 2xCPU-dummy + 1xdummy-Codec
	 *	=> 7 DAIs  = 4xCPU + 3xCodec
	 *	=> 1 ccnf  = 1xdummy-Codec
	 *
	 * ex2)
	 * CPU0 --- Codec0	link : 5
	 * CPU1 --- Codec1	dais : 6
	 * CPU2 -/		ccnf : 1
	 * CPU3 -/
	 *
	 *	=> 5 links = 1xCPU-Codec + 3xCPU-dummy + 1xdummy-Codec
	 *	=> 6 DAIs  = 4xCPU + 2xCodec
	 *	=> 1 ccnf  = 1xdummy-Codec
	 *
	 * ex3)
	 * CPU0 --- Codec0	link : 6
	 * CPU1 -/		dais : 6
	 * CPU2 --- Codec1	ccnf : 2
	 * CPU3 -/
	 *
	 *	=> 6 links = 0xCPU-Codec + 4xCPU-dummy + 2xdummy-Codec
	 *	=> 6 DAIs  = 4xCPU + 2xCodec
	 *	=> 2 ccnf  = 2xdummy-Codec
	 *
	 * ex4)
	 * CPU0 --- Codec0 (convert-rate)	link : 3
	 * CPU1 --- Codec1			dais : 4
	 *					ccnf : 1
	 *
	 *	=> 3 links = 1xCPU-Codec + 1xCPU-dummy + 1xdummy-Codec
	 *	=> 4 DAIs  = 2xCPU + 2xCodec
	 *	=> 1 ccnf  = 1xdummy-Codec
	 */
	if (!top) {
		li->num[0].cpus		= 1;
		li->num[0].codecs	= 1;
		li->num[0].platforms	= 1;

		li->link = 1;
		return 0;
	}

	return simple_for_each_link(priv, li,
				    simple_count_noml,
				    simple_count_dpcm);
}

static int simple_soc_probe(struct snd_soc_card *card)
{
	struct stream195x_simple_priv *priv = snd_soc_card_get_drvdata(card);
	int ret;

	ret = asoc_simple_init_hp(card, &priv->hp_jack, PREFIX);
	if (ret < 0)
		return ret;

	ret = asoc_simple_init_mic(card, &priv->mic_jack, PREFIX);
	if (ret < 0)
		return ret;

	return 0;
}

static int asoc_simple_probe(struct platform_device *pdev)
{
	struct stream195x_simple_priv *priv;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct snd_soc_card *card;
	struct link_info *li;
	int ret;

	/* Allocate the private data and the DAI link array */
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	card = simple_priv_to_card(priv);
	card->owner		= THIS_MODULE;
	card->dev		= dev;
	card->probe		= simple_soc_probe;
	card->driver_name       = "simple-card";

	li = devm_kzalloc(dev, sizeof(*li), GFP_KERNEL);
	if (!li)
		return -ENOMEM;

	ret = simple_get_dais_count(priv, li);
	if (ret < 0)
		return ret;

	if (!li->link)
		return -EINVAL;

	/* START SUE modifications */
	/* ret = asoc_simple_init_priv(priv, &li); */
	ret = asoc_simple_init_priv((struct asoc_simple_priv *)priv, li);
	/* END SUE modifications */
	if (ret < 0)
		return ret;

	/* START SUE addition */
	priv->dai_link_data = devm_kcalloc(dev, li->link, sizeof(*priv->dai_link_data), GFP_KERNEL);
	if (!priv->dai_link_data)
		return -ENOMEM;
	/* END SUE addition */

	if (np && of_device_is_available(np)) {

		ret = simple_parse_of(priv, li);
		if (ret < 0) {
			dev_err_probe(dev, ret, "parse error\n");
			goto err;
		}

	} else {
		struct asoc_simple_card_info *cinfo;
		struct snd_soc_dai_link_component *cpus;
		struct snd_soc_dai_link_component *codecs;
		struct snd_soc_dai_link_component *platform;
		struct snd_soc_dai_link *dai_link = priv->dai_link;
		struct simple_dai_props *dai_props = priv->dai_props;

		cinfo = dev->platform_data;
		if (!cinfo) {
			dev_err(dev, "no info for asoc-simple-card\n");
			return -EINVAL;
		}

		if (!cinfo->name ||
		    !cinfo->codec_dai.name ||
		    !cinfo->codec ||
		    !cinfo->platform ||
		    !cinfo->cpu_dai.name) {
			dev_err(dev, "insufficient asoc_simple_card_info settings\n");
			return -EINVAL;
		}

		cpus			= dai_link->cpus;
		cpus->dai_name		= cinfo->cpu_dai.name;

		codecs			= dai_link->codecs;
		codecs->name		= cinfo->codec;
		codecs->dai_name	= cinfo->codec_dai.name;

		platform		= dai_link->platforms;
		platform->name		= cinfo->platform;

		card->name		= (cinfo->card) ? cinfo->card : cinfo->name;
		dai_link->name		= cinfo->name;
		dai_link->stream_name	= cinfo->name;
		dai_link->dai_fmt	= cinfo->daifmt;
		dai_link->init		= asoc_simple_dai_init;
		memcpy(dai_props->cpu_dai, &cinfo->cpu_dai,
					sizeof(*dai_props->cpu_dai));
		memcpy(dai_props->codec_dai, &cinfo->codec_dai,
					sizeof(*dai_props->codec_dai));
	}

	snd_soc_card_set_drvdata(card, priv);

	/* START SUE additions */
	/* TODO: should we return an error if no clock is specified? */
	priv->pll8k_clk = devm_clk_get(dev, "pll8k");
	if (IS_ERR(priv->pll8k_clk))
		priv->pll8k_clk = NULL;

	priv->pll11k_clk = devm_clk_get(dev, "pll11k");
	if (IS_ERR(priv->pll11k_clk))
		priv->pll11k_clk = NULL;

	priv->powerdown_gpio = devm_gpiod_get_optional(dev, "powerdown", GPIOD_OUT_HIGH);

	card->controls = snd_soc_stream195x_controls;
	card->num_controls = ARRAY_SIZE(snd_soc_stream195x_controls);

	/*
	 * Power up the card before calling register. There might be cases
	 * when the registing and probing of the components will fail if the
	 * powerdown is enabled
	 */
	snd_soc_stream195x_set_powerdown(priv, 0);
	/* END SUE additions */

	asoc_simple_debug_info(priv);

	ret = devm_snd_soc_register_card(dev, card);
	if (ret < 0)
		goto err;

	/* START SUE additions */
	/* Unmute all links */
	snd_soc_stream195x_set_all_links_mute(priv, 0);
	/* END SUE additions */

	devm_kfree(dev, li);
	return 0;
err:
	/* START SUE additions */
	snd_soc_stream195x_set_all_links_mute(priv, 1);
	snd_soc_stream195x_set_powerdown(priv, 1);
	/* END SUE additions */

	asoc_simple_clean_reference(card);

	return ret;
}

static int asoc_simple_remove_sue(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	/* START SUE additions */
	struct stream195x_simple_priv *priv = snd_soc_card_get_drvdata(card);

	snd_soc_stream195x_set_all_links_mute(priv, 1);
	snd_soc_stream195x_set_powerdown(priv, 1);
	/* STOP SUE additions */

	asoc_simple_clean_reference(card);

	return 0;
}

static const struct of_device_id simple_of_match[] = {
	{ .compatible = "sue,stream195x-audio", },
	{},
};
MODULE_DEVICE_TABLE(of, simple_of_match);

static struct platform_driver stream195x_simple_card = {
	.driver = {
		.name = "snd-soc-stream195x",
		.pm = &snd_soc_pm_ops,
		.of_match_table = simple_of_match,
	},
	.probe = asoc_simple_probe,
	.remove = asoc_simple_remove_sue,
};

module_platform_driver(stream195x_simple_card);

MODULE_ALIAS("platform:snd-soc-stream195x");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Stream Unlimited Stream195x ASoC driver");
MODULE_AUTHOR("Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>");
MODULE_AUTHOR("Quentin Schulz <quentin.schulz@streamunlimited.com>");
