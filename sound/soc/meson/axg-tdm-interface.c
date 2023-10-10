// SPDX-License-Identifier: (GPL-2.0 OR MIT)
//
// Copyright (c) 2018 BayLibre, SAS.
// Author: Jerome Brunet <jbrunet@baylibre.com>

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>

#include "axg-tdm.h"

/* Maximum bit clock frequency according the datasheets */
#define MAX_SCLK 100000000 /* Hz */

enum {
	TDM_IFACE_PAD,
	TDM_IFACE_LOOPBACK,
};

static unsigned int axg_tdm_slots_total(u32 *mask)
{
	unsigned int slots = 0;
	int i;

	if (!mask)
		return 0;

	/* Count the total number of slots provided by all 4 lanes */
	for (i = 0; i < AXG_TDM_NUM_LANES; i++)
		slots += hweight32(mask[i]);

	return slots;
}

int axg_tdm_set_tdm_slots(struct snd_soc_dai *dai, u32 *tx_mask,
			  u32 *rx_mask, unsigned int slots,
			  unsigned int slot_width)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	struct axg_tdm_stream *tx = snd_soc_dai_dma_data_get_playback(dai);
	struct axg_tdm_stream *rx = snd_soc_dai_dma_data_get_capture(dai);
	unsigned int tx_slots, rx_slots;
	unsigned int fmt = 0;

	tx_slots = axg_tdm_slots_total(tx_mask);
	rx_slots = axg_tdm_slots_total(rx_mask);

	/* We should at least have a slot for a valid interface */
	if (!tx_slots && !rx_slots) {
		dev_err(dai->dev, "interface has no slot\n");
		return -EINVAL;
	}

	iface->slots = slots;

	switch (slot_width) {
	case 0:
		slot_width = 32;
		fallthrough;
	case 32:
		fmt |= SNDRV_PCM_FMTBIT_S32_LE;
		fallthrough;
	case 24:
		fmt |= SNDRV_PCM_FMTBIT_S24_LE;
		fmt |= SNDRV_PCM_FMTBIT_S20_LE;
		fallthrough;
	case 16:
		fmt |= SNDRV_PCM_FMTBIT_S16_LE;
		fallthrough;
	case 8:
		fmt |= SNDRV_PCM_FMTBIT_S8;
		break;
	default:
		dev_err(dai->dev, "unsupported slot width: %d\n", slot_width);
		return -EINVAL;
	}

	iface->slot_width = slot_width;

	/* Amend the dai driver and let dpcm merge do its job */
	if (tx) {
		tx->mask = tx_mask;
		dai->driver->playback.channels_max = tx_slots;
		dai->driver->playback.formats = fmt;
	}

	if (rx) {
		rx->mask = rx_mask;
		dai->driver->capture.channels_max = rx_slots;
		dai->driver->capture.formats = fmt;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axg_tdm_set_tdm_slots);

static int axg_tdm_iface_enable_clk(struct axg_tdm_iface *iface)
{
	int ret = 0;
	ret = clk_prepare_enable(iface->mclk);
	if (ret)
		goto err_mclk;
	ret = clk_prepare_enable(iface->sclk);
	if (ret)
		goto err_sclk;
	ret = clk_prepare_enable(iface->lrclk);
	if (ret)
		goto err_lrclk;

	return 0;

err_lrclk:
	clk_disable_unprepare(iface->sclk);
err_sclk:
	clk_disable_unprepare(iface->mclk);
err_mclk:
	return ret;
}

static int axg_tdm_iface_disable_clk(struct axg_tdm_iface *iface)
{
	clk_disable_unprepare(iface->lrclk);
	clk_disable_unprepare(iface->sclk);
	clk_disable_unprepare(iface->mclk);
	return 0;
}

int axg_tdm_iface_init_clk(struct axg_tdm_iface *iface)
{
	int ret = 0;
	/*
	* Always set the BCLK to MCLK/8 and the WCLK to BCLK/64 for the
	* early clocks.
	*/
	ret = clk_set_rate(iface->sclk, iface->mclk_rate/8);
	if (ret)
		return ret;
	ret = clk_set_rate(iface->lrclk, iface->mclk_rate/(64*8));
	if (ret)
		return ret;
	return clk_set_duty_cycle(iface->lrclk, 1, 2);
}
EXPORT_SYMBOL_GPL(axg_tdm_iface_init_clk);

static ssize_t axg_tdm_iface_ignore_suspend_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct axg_tdm_iface *iface = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", iface->ignore_suspend);
}

int axg_tdm_iface_set_ignore_suspend(struct axg_tdm_iface *iface, bool ignore_suspend)
{
	if (iface->ignore_suspend == ignore_suspend)
		return 0;
	iface->ignore_suspend = ignore_suspend;

	if(iface->ignore_suspend)
		return axg_tdm_iface_enable_clk(iface);
	else
		return axg_tdm_iface_disable_clk(iface);
}
EXPORT_SYMBOL_GPL(axg_tdm_iface_set_ignore_suspend);

static ssize_t axg_tdm_iface_ignore_suspend_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct axg_tdm_iface *iface = dev_get_drvdata(dev);
	int ret;
	bool ignore_suspend;

	ret = strtobool(buf, &ignore_suspend);
	if (ret < 0)
		return ret;

	if (iface->ignore_suspend == ignore_suspend)
		return count;

	ret = axg_tdm_iface_set_ignore_suspend(iface, ignore_suspend);
	if (ret < 0) {
		dev_err(dev, "failed to set ignore_suspend state: %d\n", ret);
		return ret;
	}

	if (ignore_suspend)
		dev_info(dev, "ignoring suspend, runtime acquiring components\n");
	else
		dev_info(dev, "not ignoring suspend, runtime releasing components\n");

	iface->ignore_suspend = ignore_suspend;

	return count;
}
static DEVICE_ATTR(ignore_suspend, 0644, axg_tdm_iface_ignore_suspend_show, axg_tdm_iface_ignore_suspend_store);

static int axg_tdm_iface_set_sysclk(struct snd_soc_dai *dai, int clk_id,
				    unsigned int freq, int dir)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	int ret = -ENOTSUPP;

	if (dir == SND_SOC_CLOCK_OUT && clk_id == 0) {
		if (!iface->mclk) {
			dev_warn(dai->dev, "master clock not provided\n");
		} else {
			int sign;
			unsigned int comp;

			/* save nominal sysclk for drift_comp calculation */
			iface->sysclk_nominal = freq;

			sign = (iface->drift_comp_value < 0) ? -1 : 1;
			comp = DIV_ROUND_CLOSEST_ULL((u64)freq * abs(iface->drift_comp_value),
					1000000UL); /* compensation is in ppm */
			freq -= sign * comp;
			pr_debug("axg_tdm_iface_set_sysclk nominal_freq = %d, freq = %d\n",
				iface->sysclk_nominal, freq);
			
			ret = clk_set_rate(iface->mclk, freq);
			if (!ret)
				iface->mclk_rate = freq;
			pr_debug("axg_tdm_iface_set_sysclk actual_freq = %ld\n", iface->mclk_rate);
		}
	}

	return ret;
}

static int axg_tdm_iface_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);

	switch (fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) {
	case SND_SOC_DAIFMT_BP_FP:
		if (!iface->mclk) {
			dev_err(dai->dev, "cpu clock master: mclk missing\n");
			return -ENODEV;
		}
		break;

	case SND_SOC_DAIFMT_BC_FC:
		break;

	case SND_SOC_DAIFMT_BP_FC:
	case SND_SOC_DAIFMT_BC_FP:
		dev_err(dai->dev, "only CBS_CFS and CBM_CFM are supported\n");
		fallthrough;
	default:
		return -EINVAL;
	}

	iface->fmt = fmt;
	return 0;
}

static int axg_tdm_iface_startup(struct snd_pcm_substream *substream,
				 struct snd_soc_dai *dai)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	struct axg_tdm_stream *ts =
		snd_soc_dai_get_dma_data(dai, substream);
	int ret;

	if (!axg_tdm_slots_total(ts->mask)) {
		dev_err(dai->dev, "interface has not slots\n");
		return -EINVAL;
	}

	if (snd_soc_component_active(dai->component)) {
		/* Apply component wide rate symmetry */
		ret = snd_pcm_hw_constraint_single(substream->runtime,
						   SNDRV_PCM_HW_PARAM_RATE,
						   iface->rate);

	} else {
		/* Limit rate according to the slot number and width */
		unsigned int max_rate =
			MAX_SCLK / (iface->slots * iface->slot_width);
		ret = snd_pcm_hw_constraint_minmax(substream->runtime,
						   SNDRV_PCM_HW_PARAM_RATE,
						   0, max_rate);
	}
	axg_tdm_iface_enable_clk(iface);

	/*
	* Make sure, that the buffer size is always even (aligned to 2 bytes)
	* to avoid problems with audio pointer calculation!
	*/
	ret |= snd_pcm_hw_constraint_step(substream->runtime, 0,
		SNDRV_PCM_HW_PARAM_BUFFER_SIZE, 2);

	if (ret < 0)
		dev_err(dai->dev, "can't set iface rate constraint\n");
	else
		ret = 0;

	return ret;
}

static void axg_tdm_iface_shutdown(struct snd_pcm_substream *substream,
				 struct snd_soc_dai *dai)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	axg_tdm_iface_disable_clk(iface);
}

static int axg_tdm_iface_set_stream(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *params,
				    struct snd_soc_dai *dai)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	struct axg_tdm_stream *ts = snd_soc_dai_get_dma_data(dai, substream);
	unsigned int channels = params_channels(params);
	unsigned int width = params_width(params);

	/* Save rate and sample_bits for component symmetry */
	iface->rate = params_rate(params);

	/* Make sure this interface can cope with the stream */
	if (axg_tdm_slots_total(ts->mask) < channels) {
		dev_err(dai->dev, "not enough slots for channels\n");
		return -EINVAL;
	}

	if (iface->slot_width < width) {
		dev_err(dai->dev, "incompatible slots width for stream\n");
		return -EINVAL;
	}

	/* Save the parameter for tdmout/tdmin widgets */
	ts->physical_width = params_physical_width(params);
	ts->width = params_width(params);
	ts->channels = params_channels(params);

	return 0;
}

static int axg_tdm_iface_set_lrclk(struct snd_soc_dai *dai,
				   struct snd_pcm_hw_params *params)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	unsigned int ratio_num;
	int ret;

	ret = clk_set_rate(iface->lrclk, params_rate(params));
	if (ret) {
		dev_err(dai->dev, "setting sample clock failed: %d\n", ret);
		return ret;
	}

	switch (iface->fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
	case SND_SOC_DAIFMT_RIGHT_J:
		/* 50% duty cycle ratio */
		ratio_num = 1;
		break;

	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		/*
		 * A zero duty cycle ratio will result in setting the mininum
		 * ratio possible which, for this clock, is 1 cycle of the
		 * parent bclk clock high and the rest low, This is exactly
		 * what we want here.
		 */
		ratio_num = 0;
		break;

	default:
		return -EINVAL;
	}

	ret = clk_set_duty_cycle(iface->lrclk, ratio_num, 2);
	if (ret) {
		dev_err(dai->dev,
			"setting sample clock duty cycle failed: %d\n", ret);
		return ret;
	}

	/* Set sample clock inversion */
	ret = clk_set_phase(iface->lrclk,
			    axg_tdm_lrclk_invert(iface->fmt) ? 180 : 0);
	if (ret) {
		dev_err(dai->dev,
			"setting sample clock phase failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static int axg_tdm_iface_set_sclk(struct snd_soc_dai *dai,
				  struct snd_pcm_hw_params *params)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	unsigned long srate;
	int ret;

	srate = iface->slots * iface->slot_width * params_rate(params);

	if (!iface->mclk_rate) {
		/* If no specific mclk is requested, default to bit clock * 2 */
		clk_set_rate(iface->mclk, 2 * srate);
	}

	ret = clk_set_rate(iface->sclk, srate);
	if (ret) {
		dev_err(dai->dev, "setting bit clock failed: %d\n", ret);
		return ret;
	}

	/* Set the bit clock inversion */
	ret = clk_set_phase(iface->sclk,
			    axg_tdm_sclk_invert(iface->fmt) ? 0 : 180);
	if (ret) {
		dev_err(dai->dev, "setting bit clock phase failed: %d\n", ret);
		return ret;
	}

	return ret;
}

static int axg_tdm_iface_hw_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *params,
				   struct snd_soc_dai *dai)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	int ret;

	switch (iface->fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
	case SND_SOC_DAIFMT_RIGHT_J:
		if (iface->slots > 2) {
			dev_err(dai->dev, "bad slot number for format: %d\n",
				iface->slots);
			return -EINVAL;
		}
		break;

	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		break;

	default:
		dev_err(dai->dev, "unsupported dai format\n");
		return -EINVAL;
	}

	ret = axg_tdm_iface_set_stream(substream, params, dai);
	if (ret)
		return ret;

	if ((iface->fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) ==
	    SND_SOC_DAIFMT_BP_FP) {
		ret = axg_tdm_iface_set_sclk(dai, params);
		if (ret)
			return ret;

		ret = axg_tdm_iface_set_lrclk(dai, params);
		if (ret)
			return ret;
	}

	return 0;
}

static int axg_tdm_iface_trigger(struct snd_pcm_substream *substream,
				 int cmd,
				 struct snd_soc_dai *dai)
{
	struct axg_tdm_stream *ts =
		snd_soc_dai_get_dma_data(dai, substream);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		axg_tdm_stream_start(ts);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_STOP:
		axg_tdm_stream_stop(ts);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int axg_tdm_iface_drift_info(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->value.integer.min = -500; /* +/- 500ppm */
	uinfo->value.integer.max = 500;
	uinfo->count = 1;

	return 0;
}

static int axg_tdm_iface_drift_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dai *cpu_dai = snd_kcontrol_chip(kcontrol);
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(cpu_dai);

	ucontrol->value.integer.value[0] = iface->drift_comp_value;

	return 0;
}

static int axg_tdm_iface_drift_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dai *cpu_dai = snd_kcontrol_chip(kcontrol);
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(cpu_dai);

	if (ucontrol->value.integer.value[0] == iface->drift_comp_value)
		return 0;

	iface->drift_comp_value = ucontrol->value.integer.value[0];

	if (iface->sysclk_nominal == 0)
		return 0;

	return snd_soc_dai_set_sysclk(cpu_dai, 0, iface->sysclk_nominal,
			SND_SOC_CLOCK_OUT);
}

static int axg_tdm_iface_remove_dai(struct snd_soc_dai *dai)
{
	int stream;

	for_each_pcm_streams(stream) {
		struct axg_tdm_stream *ts = snd_soc_dai_dma_data_get(dai, stream);

		if (ts)
			axg_tdm_stream_free(ts);
	}

	return 0;
}

static int axg_tdm_iface_probe_dai(struct snd_soc_dai *dai)
{
	struct axg_tdm_iface *iface = snd_soc_dai_get_drvdata(dai);
	int stream;

	for_each_pcm_streams(stream) {
		struct axg_tdm_stream *ts;

		if (!snd_soc_dai_get_widget(dai, stream))
			continue;

		ts = axg_tdm_stream_alloc(iface);
		if (!ts) {
			axg_tdm_iface_remove_dai(dai);
			return -ENOMEM;
		}
		snd_soc_dai_dma_data_set(dai, stream, ts);
	}

	/*
	 * Only add the drift compensator control if it was exlicitly requested
	 * for the TDM interface. The reason that we do not want to have this
	 * control on all interfaces since it could happen that two or more interfaces
	 * might share the same parent clock, so multiple dirft compensators would
	 * be controlling the same PLL.
	 */
	if (iface->enable_drift_compensator) {
		char name[128] = "Drift compensator ";
		struct snd_kcontrol_new tdm_control = {
			.name	= name,
			.iface	= SNDRV_CTL_ELEM_IFACE_MIXER,
			.info	= axg_tdm_iface_drift_info,
			.get	= axg_tdm_iface_drift_get,
			.put	= axg_tdm_iface_drift_put,
		};
		strncat(name, dai->component->name_prefix, ARRAY_SIZE(name));
		snd_soc_add_dai_controls(dai, &tdm_control, 1);
	}

	return 0;
}

static const struct snd_soc_dai_ops axg_tdm_iface_ops = {
	.probe		= axg_tdm_iface_probe_dai,
	.remove		= axg_tdm_iface_remove_dai,
	.set_sysclk	= axg_tdm_iface_set_sysclk,
	.set_fmt	= axg_tdm_iface_set_fmt,
	.startup	= axg_tdm_iface_startup,
	.shutdown	= axg_tdm_iface_shutdown,
	.hw_params	= axg_tdm_iface_hw_params,
	.trigger	= axg_tdm_iface_trigger,
};

/* TDM Backend DAIs */
static const struct snd_soc_dai_driver axg_tdm_iface_dai_drv[] = {
	[TDM_IFACE_PAD] = {
		.name = "TDM Pad",
		.playback = {
			.stream_name	= "Playback",
			.channels_min	= 1,
			.channels_max	= AXG_TDM_CHANNEL_MAX,
			.rates		= AXG_TDM_RATES,
			.formats	= AXG_TDM_FORMATS,
		},
		.capture = {
			.stream_name	= "Capture",
			.channels_min	= 1,
			.channels_max	= AXG_TDM_CHANNEL_MAX,
			.rates		= AXG_TDM_RATES,
			.formats	= AXG_TDM_FORMATS,
		},
		.id = TDM_IFACE_PAD,
		.ops = &axg_tdm_iface_ops,
	},
	[TDM_IFACE_LOOPBACK] = {
		.name = "TDM Loopback",
		.capture = {
			.stream_name	= "Loopback",
			.channels_min	= 1,
			.channels_max	= AXG_TDM_CHANNEL_MAX,
			.rates		= AXG_TDM_RATES,
			.formats	= AXG_TDM_FORMATS,
		},
		.id = TDM_IFACE_LOOPBACK,
		.ops = &axg_tdm_iface_ops,
	},
};

static int axg_tdm_iface_set_bias_level(struct snd_soc_component *component,
					enum snd_soc_bias_level level)
{
	struct axg_tdm_iface *iface = snd_soc_component_get_drvdata(component);
	enum snd_soc_bias_level now =
		snd_soc_component_get_bias_level(component);
	int ret = 0;

	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (now == SND_SOC_BIAS_STANDBY)
			ret = clk_prepare_enable(iface->mclk);
		break;

	case SND_SOC_BIAS_STANDBY:
		if (now == SND_SOC_BIAS_PREPARE)
			clk_disable_unprepare(iface->mclk);
		break;

	case SND_SOC_BIAS_OFF:
	case SND_SOC_BIAS_ON:
		break;
	}

	return ret;
}

static const struct snd_soc_dapm_widget axg_tdm_iface_dapm_widgets[] = {
	SND_SOC_DAPM_SIGGEN("Playback Signal"),
};

static const struct snd_soc_dapm_route axg_tdm_iface_dapm_routes[] = {
	{ "Loopback", NULL, "Playback Signal" },
};

static const struct snd_soc_component_driver axg_tdm_iface_component_drv = {
	.dapm_widgets		= axg_tdm_iface_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(axg_tdm_iface_dapm_widgets),
	.dapm_routes		= axg_tdm_iface_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(axg_tdm_iface_dapm_routes),
	.set_bias_level		= axg_tdm_iface_set_bias_level,
};

static const struct of_device_id axg_tdm_iface_of_match[] = {
	{ .compatible = "amlogic,axg-tdm-iface", },
	{}
};
MODULE_DEVICE_TABLE(of, axg_tdm_iface_of_match);

static int axg_tdm_iface_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct snd_soc_dai_driver *dai_drv;
	struct axg_tdm_iface *iface;
	int i;

	iface = devm_kzalloc(dev, sizeof(*iface), GFP_KERNEL);
	if (!iface)
		return -ENOMEM;
	platform_set_drvdata(pdev, iface);

	/*
	 * Duplicate dai driver: depending on the slot masks configuration
	 * We'll change the number of channel provided by DAI stream, so dpcm
	 * channel merge can be done properly
	 */
	dai_drv = devm_kcalloc(dev, ARRAY_SIZE(axg_tdm_iface_dai_drv),
			       sizeof(*dai_drv), GFP_KERNEL);
	if (!dai_drv)
		return -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(axg_tdm_iface_dai_drv); i++)
		memcpy(&dai_drv[i], &axg_tdm_iface_dai_drv[i],
		       sizeof(*dai_drv));

	/* Bit clock provided on the pad */
	iface->sclk = devm_clk_get(dev, "sclk");
	if (IS_ERR(iface->sclk))
		return dev_err_probe(dev, PTR_ERR(iface->sclk), "failed to get sclk\n");

	/* Sample clock provided on the pad */
	iface->lrclk = devm_clk_get(dev, "lrclk");
	if (IS_ERR(iface->lrclk))
		return dev_err_probe(dev, PTR_ERR(iface->lrclk), "failed to get lrclk\n");

	iface->enable_drift_compensator = of_property_read_bool(dev->of_node, "enable-drift-compensator");

	/*
	 * mclk maybe be missing when the cpu dai is in slave mode and
	 * the codec does not require it to provide a master clock.
	 * At this point, ignore the error if mclk is missing. We'll
	 * throw an error if the cpu dai is master and mclk is missing
	 */
	iface->mclk = devm_clk_get_optional(dev, "mclk");
	if (IS_ERR(iface->mclk))
		return dev_err_probe(dev, PTR_ERR(iface->mclk), "failed to get mclk\n");

	if (device_create_file(dev, &dev_attr_ignore_suspend) < 0)
		dev_err(dev, "failed to create ignore_suspend sysfs file\n");

	return devm_snd_soc_register_component(dev,
					&axg_tdm_iface_component_drv, dai_drv,
					ARRAY_SIZE(axg_tdm_iface_dai_drv));
}

static struct platform_driver axg_tdm_iface_pdrv = {
	.probe = axg_tdm_iface_probe,
	.driver = {
		.name = "axg-tdm-iface",
		.of_match_table = axg_tdm_iface_of_match,
	},
};
module_platform_driver(axg_tdm_iface_pdrv);

MODULE_DESCRIPTION("Amlogic AXG TDM interface driver");
MODULE_AUTHOR("Jerome Brunet <jbrunet@baylibre.com>");
MODULE_LICENSE("GPL v2");
