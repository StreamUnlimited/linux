// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek ALSA support
*
* Copyright (C) 2021, Realtek Corporation. All rights reserved.
*/

#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <linux/pm_runtime.h>
#include <sound/simple_card_utils.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>

#include <uapi/ameba/audio_pll.h>
#include <ameba/ameba_audio_clock.h>

struct ameba_priv {
	int cur_pll_ppm;
	struct gpio_desc *amp_mute_gpio;
	struct gpio_desc *hp_mute_gpio;
	struct work_struct mute_work;
	atomic_t playing;

	struct snd_kcontrol *drift_kcontrol;
	struct asoc_simple_jack hp_jack;
	atomic_t hp_jack_inserted;

	bool ignore_suspend;

	bool regulator_is_enabled;
	struct regulator *enable_regulator;
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
	u32 action = new_ppm < 0 ? PLL_FASTER : PLL_SLOWER;

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

static void set_amp_mute(struct ameba_priv *priv, bool mute)
{
	if (priv->amp_mute_gpio)
		gpiod_set_value_cansleep(priv->amp_mute_gpio, mute);
}

static void set_hp_mute(struct ameba_priv *priv, bool mute)
{
	if (priv->hp_mute_gpio)
		gpiod_set_value_cansleep(priv->hp_mute_gpio, mute);
}

static int set_regulator_enable(struct ameba_priv *priv, bool enable)
{
	int ret = 0;

	if (!priv->enable_regulator)
		return 0;

	if (priv->regulator_is_enabled == enable)
		return 0;

	if (enable)
		ret = regulator_enable(priv->enable_regulator);
	else
		ret = regulator_disable(priv->enable_regulator);

	if (!ret)
		priv->regulator_is_enabled = enable;

	return ret;
}

static const struct snd_soc_dapm_widget ameba_dapm_widgets[] = {
	// Just to suppress the error message from the kernel about unexistent dapm route
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

static void ameba_mute_work_handler(struct work_struct *work)
{
	struct ameba_priv *priv = container_of(work, struct ameba_priv, mute_work);

	if (!atomic_read(&priv->playing)) {
		set_amp_mute(priv, true);
		set_hp_mute(priv, true);
	} else {
		if (atomic_read(&priv->hp_jack_inserted)) {
			set_amp_mute(priv, true);
			set_hp_mute(priv, false);
		} else {
			set_hp_mute(priv, true);
			set_amp_mute(priv, false);
		}
	}
}

static int ameba_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->card->dev;

	return pm_runtime_resume_and_get(dev);
}

static void ameba_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->card->dev;

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_sync_autosuspend(dev);
}

static int ameba_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct ameba_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	// Re-set the PLLs to 0
	priv->cur_pll_ppm = 0;
	snd_soc_ameba_set_pll_ppm(priv->cur_pll_ppm);

	if (priv->drift_kcontrol) {
		snd_ctl_notify(rtd->card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE, &priv->drift_kcontrol->id);
	}

	return 0;
}

static int ameba_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct ameba_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	// nothing to do on capture for now
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		return 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		atomic_set(&priv->playing, true);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		atomic_set(&priv->playing, false);
		break;
	}

	// This is an atomic context, and since the gpio is managed through IPC, we need to do this async
	schedule_work(&priv->mute_work);
	return 0;
}

static struct snd_soc_ops ameba_ops = {
	.startup = ameba_startup,
	.shutdown = ameba_shutdown,
	.hw_params = ameba_hw_params,
	.trigger = ameba_trigger,
};

static int hp_jack_event(struct notifier_block *nb, unsigned long event,
	void *data)
{
	struct snd_soc_jack *jack = data;
	struct ameba_priv *priv = snd_soc_card_get_drvdata(jack->card);

	atomic_set(&priv->hp_jack_inserted, (event & SND_JACK_HEADPHONE));
	schedule_work(&priv->mute_work);

	return NOTIFY_OK;
}

static struct notifier_block hp_jack_nb = {
	.notifier_call = hp_jack_event,
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
		.dai_fmt = SND_SOC_DAI_FORMAT_I2S,
		SND_SOC_DAILINK_REG(aif3),
	},
	{
		.name = "codec AIF4",
		.stream_name = "SPORT3_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_I2S,
		SND_SOC_DAILINK_REG(aif4),
	},
};

static struct snd_soc_dai_link ameba_dai_digital_only[] = {
	{
		.name = "codec AIF3",
		.stream_name = "SPORT2_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_I2S,
		SND_SOC_DAILINK_REG(aif3),
	},
	{
		.name = "codec AIF4",
		.stream_name = "SPORT3_I2S_Dai",
		.ops = &ameba_ops,
		.dai_fmt = SND_SOC_DAI_FORMAT_I2S,
		SND_SOC_DAILINK_REG(aif4),
	},
};

static ssize_t ameba_ignore_suspend_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);

	return scnprintf(buf, PAGE_SIZE, "%d\n", priv->ignore_suspend);
}

static int ameba_set_ignore_suspend(struct snd_soc_card *card,
					 bool ignore_suspend)
{
	int ret = 0;
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);
	struct device *dev = card->dev;
	struct snd_soc_component *component;

	if (priv->ignore_suspend == ignore_suspend)
		return 0;

	priv->ignore_suspend = ignore_suspend;

	dev_info(dev, "%s suspend, runtime %s components\n",
		 ignore_suspend ? "ignoring" : "not ignoring",
		 !ignore_suspend ? "releasing" : "acquiring");

	ret = priv->ignore_suspend ? pm_runtime_get_sync(dev) : pm_runtime_put_sync(dev);

	// We achieve the "ignore suspend" behavior by increasing the reference counter of all
	// components, by using `pm_runtime_get_sync()`. To enable the suspend behavior again
	// we just decrease the reference counter with `pm_runtime_put_sync()`.
	for_each_card_components(card, component) {
		if (!pm_runtime_enabled(component->dev)) {
			dev_dbg(component->dev, "Runtime power management is disabled for this component, skipping!");
			continue;
		}
		ret = priv->ignore_suspend ? pm_runtime_get_sync(component->dev) : pm_runtime_put_sync(component->dev);
		if (ret < 0 && ret != -ENOSYS) {
			dev_err(component->dev, "%s() failed: %d\n", priv->ignore_suspend ? "pm_runtime_get_sync" : "pm_runtime_put_sync", ret);
			break;
		}
	}

	return ret;
}

static ssize_t ameba_ignore_suspend_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	int ret;
	bool ignore_suspend = true;

	ret = strtobool(buf, &ignore_suspend);
	if (ret < 0)
		return ret;

	ret = ameba_set_ignore_suspend(card, ignore_suspend);
	if (ret < 0) {
		dev_err(dev, "failed to set ignore_suspend state: %d\n", ret);
		return ret;
	}

	return count;
}
static DEVICE_ATTR(ignore_suspend, 0644, ameba_ignore_suspend_show,
		   ameba_ignore_suspend_store);

static int ameba_card_late_probe(struct snd_soc_card *card)
{
	int ret = 0;

	// We can only call `ameba_set_ignore_suspend()` in the `late_probe()`
	// after `devm_snd_soc_register_card()` has pupulated the `component_dev_list`
	// in the `snd_soc_card` struct.
	ret = ameba_set_ignore_suspend(card, true);
	if (ret < 0) {
		dev_err(card->dev, "failed to set initial ignore_suspend state: %d\n", ret);
		return ret;
	}

	ret = device_create_file(card->dev, &dev_attr_ignore_suspend);
	if (ret < 0) {
		dev_err(card->dev, "failed to create ignore_suspend sysfs file: %d\n", ret);
		return ret;
	}

	return 0;
}

static int ameba_card_probe(struct snd_soc_card *card)
{
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);
	struct device_node *np = card->dev->of_node;

	priv->amp_mute_gpio = devm_gpiod_get_optional(card->dev, "mute", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->amp_mute_gpio))
		return dev_err_probe(card->dev, PTR_ERR(priv->amp_mute_gpio), "Failed to get mute gpio\n");

	priv->hp_mute_gpio = devm_gpiod_get_optional(card->dev, "hp_mute", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->hp_mute_gpio))
		return dev_err_probe(card->dev, PTR_ERR(priv->hp_mute_gpio), "Failed to get HP mute gpio\n");

	if (of_property_read_bool(np, "hp-det-gpio")) {
		int ret = asoc_simple_init_jack(card, &priv->hp_jack, 1, NULL, "Headphone Jack");
		if (ret < 0) {
			dev_err(card->dev, "Failed to init HP detect gpio: %d\n", ret);
			return ret;
		}
		snd_soc_jack_notifier_register(&priv->hp_jack.jack, &hp_jack_nb);
	}

	priv->enable_regulator = devm_regulator_get_exclusive(card->dev, "amp");
	if (IS_ERR(priv->enable_regulator))
		return dev_err_probe(card->dev, PTR_ERR(priv->enable_regulator), "Failed to get regulator\n");
	priv->regulator_is_enabled = regulator_is_enabled(priv->enable_regulator);

	priv->drift_kcontrol = snd_soc_card_get_kcontrol(card, KCONTROL_DRIFT_COMPENSATOR_NAME);

	return 0;
}

static int ameba_runtime_suspend(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);
	return set_regulator_enable(priv, false);
}

static int ameba_runtime_resume(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ameba_priv *priv = snd_soc_card_get_drvdata(card);
	int ret = 0;
	ret = set_regulator_enable(priv, true);
	msleep(50);
	return ret;
}

static const struct dev_pm_ops ameba_pm_ops = {
	/* ASoC-specific system sleep handlers */
	.suspend = snd_soc_suspend,
	.resume = snd_soc_resume,
	.freeze = snd_soc_suspend,
	.thaw = snd_soc_resume,
	.poweroff = snd_soc_poweroff,
	.restore = snd_soc_resume,
	/* Custom runtime PM handlers */
	.runtime_suspend = ameba_runtime_suspend,
	.runtime_resume = ameba_runtime_resume,
};


static struct snd_soc_card ameba_snd = {
	.name = "Ameba-snd",
	.owner = THIS_MODULE,
	.controls = snd_soc_ameba_controls,
	.num_controls = ARRAY_SIZE(snd_soc_ameba_controls),
	.probe = ameba_card_probe,
	.late_probe = ameba_card_late_probe,
	.dapm_widgets = ameba_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(ameba_dapm_widgets),
};

/*
 * This function allows to change the codecs for the AIF3 (I2S2) and AIF4 (I2S3) interfaces
 * by parsing the `sue,audio-codec` device-tree property.
 * If this property is not specified or it only contains one entry then the dummy codec
 * will be kept as specified above by using `SND_SOC_DAILINK_DEFS()`.
 */
static int sue_parse_digital_audio_codecs(struct snd_soc_card *card)
{
	struct snd_soc_dai_link_component * const per_aif_codecs[2] = { aif3_codecs, aif4_codecs };
	struct device_node *np = card->dev->of_node;
	int num_sue_codecs, ret, i;

	num_sue_codecs = of_count_phandle_with_args(np, "sue,audio-codec", "#sound-dai-cells");
	if (num_sue_codecs < 0 && num_sue_codecs != -ENOENT) {
		dev_err(card->dev, "failed to parse sue,audio-codec node: %d\n", num_sue_codecs);
		return ret;
	}

	if (num_sue_codecs > 2) {
		dev_warn(card->dev, "sue,audio-codec: excess entries ignored (max 2)\n");
		num_sue_codecs = 2;
	}

	for (i = 0; i < num_sue_codecs; i++) {
		struct of_phandle_args args;
		struct snd_soc_dai_link_component *comp = &per_aif_codecs[i][0];

		ret = of_parse_phandle_with_args(np, "sue,audio-codec", "#sound-dai-cells", i, &args);
		if (ret < 0) {
			/*
			 * If there was a sentinel value specified (<0>) then we get -ENOENT, so
			 * let's skip the codec and do nothing as we statically already defined a
			 * dummy codec.
			 */
			if (ret == -ENOENT)
				continue;

			dev_err(card->dev, "failed to parse sue,audio-codec[%d]: %d\n", i, ret);
			return ret;
		}

		comp->name = NULL;
		comp->of_node = args.np;

		ret = snd_soc_get_dai_name(&args, &comp->dai_name);
		of_node_put(args.np);

		if (ret < 0) {
			dev_err(card->dev, "failed to get dai_name for codec sue,audio-codec %d: %d\n", i, ret);
			return ret;
		}

	}

	return 0;
}

static int ameba_audio_probe(struct platform_device *pdev)
{
	int ret;
	struct ameba_priv *priv;
	struct snd_soc_card *card = &ameba_snd;
	struct device_node *np = pdev->dev.of_node;
	bool disable_analog_links;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	INIT_WORK(&priv->mute_work, ameba_mute_work_handler);

	snd_soc_card_set_drvdata(card, priv);

	disable_analog_links = of_property_read_bool(np, "sue,disable-analog-links");

	card->dev = &pdev->dev;

	if (disable_analog_links) {
		card->dai_link = ameba_dai_digital_only;
		card->num_links = ARRAY_SIZE(ameba_dai_digital_only);
	} else {
		card->dai_link = ameba_dai;
		card->num_links = ARRAY_SIZE(ameba_dai);
	}

	ret = sue_parse_digital_audio_codecs(card);
	if (ret < 0) {
		dev_err(card->dev, "failed to parse SUE audio codecs: %d\n", ret);
		return ret;
	}

	/* Enable runtime PM for this device */
	pm_runtime_set_autosuspend_delay(&pdev->dev, 1000);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	return ret;

	return 0;
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
		.pm	= &ameba_pm_ops,
	},
	.probe		= ameba_audio_probe,
};

module_platform_driver(ameba_audio_driver);

MODULE_DESCRIPTION("Realtek Ameba ALSA driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Realtek Corporation");
