// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek ALSA support
*
* Copyright (C) 2021, Realtek Corporation. All rights reserved.
*/

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <uapi/sound/rtk_audio_pll.h>
#include "ameba_audio_clock.h"

struct ameba_priv {
	int cur_pll_ppm;
};

enum {
	DAI_LINK_PLAYBACK,
	DAI_LINK_CAPTURE,
};

SND_SOC_DAILINK_DEFS(aif1,
	DAILINK_COMP_ARRAY(COMP_CPU("4100d000.sport")),
	DAILINK_COMP_ARRAY(COMP_CODEC("4100b000.codec", "ameba-aif1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("audiodma@0")));

SND_SOC_DAILINK_DEFS(aif2,
	DAILINK_COMP_ARRAY(COMP_CPU("4100e000.sport")),
	DAILINK_COMP_ARRAY(COMP_CODEC("4100b000.codec", "ameba-aif2")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("audiodma@1")));

SND_SOC_DAILINK_DEFS(aif3,
	DAILINK_COMP_ARRAY(COMP_CPU("4100f000.sport")),
	DAILINK_COMP_ARRAY(COMP_DUMMY()),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("audiodma@2")));

SND_SOC_DAILINK_DEFS(aif4,
	DAILINK_COMP_ARRAY(COMP_CPU("41010000.sport")),
	DAILINK_COMP_ARRAY(COMP_DUMMY()),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("audiodma@3")));

#define KCONTROL_DRIFT_COMPENSATOR_NAME "Drift compensator"

static int snd_soc_ameba_set_pll_ppm(int new_ppm)
{
	u32 ppm = abs(new_ppm);
	u32 action = new_ppm < 0 ? PLL_SLOWER : PLL_FASTER;

	pll_i2s_98P304M_clk_tune(ppm, action);
	// We do not need to tune the 24.576 MHz PLL as it is just
	// divided down from the 98.304 MHz PLL above.
	// pll_i2s_24P576M_clk_tune(ppm, action);
	pll_i2s_45P1584M_clk_tune(ppm, action);

	return 0;
}

static int snd_soc_ameba_ppm_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->value.integer.min = -500;
	uinfo->value.integer.max = 500;
	uinfo->count = 1;

	return 0;
}

static int snd_soc_ameba_ppm_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);

	ucontrol->value.integer.value[0] = priv->cur_pll_ppm;

	return 0;
}

static int snd_soc_ameba_ppm_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);
	int ppm = ucontrol->value.integer.value[0];

	priv->cur_pll_ppm = ppm;
	snd_soc_ameba_set_pll_ppm(priv->cur_pll_ppm);

	return 1;
}

static const struct snd_kcontrol_new snd_soc_ameba_controls[] = {
	{
		.name = KCONTROL_DRIFT_COMPENSATOR_NAME,
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.info = snd_soc_ameba_ppm_info,
		.get = snd_soc_ameba_ppm_get,
		.put = snd_soc_ameba_ppm_put,
	},
};

static int ameba_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_kcontrol *kcontrol;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct ameba_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	// Re-set the PLLs to 0
	priv->cur_pll_ppm = 0;
	snd_soc_ameba_set_pll_ppm(priv->cur_pll_ppm);

	kcontrol = snd_soc_card_get_kcontrol(rtd->card, KCONTROL_DRIFT_COMPENSATOR_NAME);
	if (kcontrol) {
		snd_ctl_notify(rtd->card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE, &kcontrol->id);
	}

	return 0;
}

static struct snd_soc_ops ameba_ops = {
	.hw_params = ameba_hw_params,
};

static struct snd_soc_dai_link ameba_dai[] = {
	{
		.name = "codec AIF1",
		.stream_name = "Pri_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif1),
	},
	{
		.name = "codec AIF2",
		.stream_name = "Sec_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif2),
	},
	{
		.name = "codec AIF3",
		.stream_name = "SPORT2_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif3),
	},
	{
		.name = "codec AIF4",
		.stream_name = "SPORT3_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif4),
	},
};

static struct snd_soc_dai_link ameba_dai_digital_only[] = {
	{
		.name = "codec AIF3",
		.stream_name = "SPORT2_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif3),
	},
	{
		.name = "codec AIF4",
		.stream_name = "SPORT3_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_LEFT_J,
		SND_SOC_DAILINK_REG(aif4),
	},
};

static struct snd_soc_card ameba_snd = {
	.name = "Ameba-snd",
	.owner = THIS_MODULE,
	.dai_link = ameba_dai,
	.num_links = ARRAY_SIZE(ameba_dai),
	.controls = snd_soc_ameba_controls,
	.num_controls = ARRAY_SIZE(snd_soc_ameba_controls),
};

static int ameba_audio_probe(struct platform_device *pdev)
{
	int ret;
	struct ameba_priv *priv;
	struct snd_soc_card *card = &ameba_snd;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	snd_soc_card_set_drvdata(card, priv);

	struct device_node *np = pdev->dev.of_node;
	bool disable_analog_links = of_property_read_bool(np, "sue,disable-analog-links");

	card->dev = &pdev->dev;

	if (disable_analog_links) {
		card->dai_link = ameba_dai_digital_only;
		card->num_links = ARRAY_SIZE(ameba_dai_digital_only);
	} else {
		card->dai_link = ameba_dai;
		card->num_links = ARRAY_SIZE(ameba_dai);
	}

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	return ret;
}

static const struct of_device_id ameba_audio_of_match[] = {
	{ .compatible = "realtek,ameba-audio", },
	{},
};
MODULE_DEVICE_TABLE(of, ameba_audio_of_match);


static struct platform_driver ameba_audio_driver = {
	.driver		= {
		.name	= "ameba-audio",
		.of_match_table = of_match_ptr(ameba_audio_of_match),
		.pm	= &snd_soc_pm_ops,
	},
	.probe		= ameba_audio_probe,
};

module_platform_driver(ameba_audio_driver);

MODULE_DESCRIPTION("Realtek Ameba ALSA driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Realtek Corporation");
