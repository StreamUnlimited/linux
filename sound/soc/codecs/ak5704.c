/*
 * ak5704.c  --  audio driver for AK5704 DAC
 *
 * Copyright (C) 2020 StreamUnlimited Engineering GmbH
 *
 * Authors:
 * Quentin Schulz <quentin.schulz@streamunlimited.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

struct ak5704_priv {
	struct regmap *regmap;
	struct device *dev;
};

#define AK5704_FLOW_CTRL	0x00
#define AK5704_FLOW_CTRL_SDTO2_EN	BIT(6)

#define AK5704_PWR_MGMT1	0x01
#define AK5704_PWR_MGMT1_PLL_UP		BIT(6)

#define AK5704_PWR_MGMT2	0x02
#define AK5704_PWR_MGMT2_MIC1A_UP	BIT(4)
#define AK5704_PWR_MGMT2_MIC1B_UP	BIT(5)
#define AK5704_PWR_MGMT2_MIC2A_UP	BIT(6)
#define AK5704_PWR_MGMT2_MIC2B_UP	BIT(7)

#define AK5704_DMIC_CONF	0x07
#define AK5704_DMIC_CONF_MIC1_DIGITAL	BIT(0)
#define AK5704_DMIC_CONF_MIC1_DCLKE	BIT(1)
#define AK5704_DMIC_CONF_MIC1_DCLKP	BIT(2)
#define AK5704_DMIC_CONF_MIC2_DIGITAL	BIT(4)
#define AK5704_DMIC_CONF_MIC2_DCLKE	BIT(5)
#define AK5704_DMIC_CONF_MIC2_DCLKP	BIT(6)

#define AK5704_CLK_MODE_SEL	0x08
#define AK5704_CLK_MODE_SEL_CM_SHIFT	4
#define AK5704_CLK_MODE_SEL_CM_MASK	GENMASK(5, 4)
#define AK5704_CLK_MODE_SEL_CM(x)	(((x) << AK5704_CLK_MODE_SEL_CM_SHIFT) & AK5704_CLK_MODE_SEL_CM_MASK)
#define AK5704_CLK_MODE_SEL_FS_MASK	GENMASK(3, 0)
#define AK5704_CLK_MODE_SEL_FS(x)	((x) & AK5704_CLK_MODE_SEL_FS_MASK)

#define AK5704_CLK_SRC		0x09
#define AK5704_CLK_SRC_PLL_BCLK		BIT(1)

#define AK5704_PLD_H		0x0a
#define AK5704_PLD_L		0x0b
#define AK5704_PLM_H		0x0c
#define AK5704_PLM_L		0x0d

struct ak5704_pllclk {
	unsigned int	rate;
	u16		ref_div;
	u16		feedback_div;
	u8		fs;
};

/*
 * PLL table extracted from Table 14. (p.36) in datasheet for BCLK at 64fs (2
 * slots of 32fs).
 */
static struct ak5704_pllclk ak5704_pllclks[] = {
	{   8000, 0x0000, 0x0077, 0x0 },
	{  11025, 0x0001, 0x009f, 0x1 },
	{  12000, 0x0001, 0x009f, 0x2 },
	{  16000, 0x0001, 0x0077, 0x4 },
	{  22050, 0x0003, 0x009f, 0x5 },
	{  24000, 0x0003, 0x009f, 0x6 },
	{  32000, 0x0003, 0x0077, 0x8 },
	{  44100, 0x0007, 0x009f, 0x9 },
	{  48000, 0x0007, 0x009f, 0xa },
	{  88200, 0x000f, 0x009f, 0xc },
	{  96000, 0x000f, 0x009f, 0xd },
	{ 176400, 0x001f, 0x009f, 0xe },
	{ 192000, 0x001f, 0x009f, 0xf },
};

/*
 * Microphones hooked to AK5704 on SK1955 4mic board (IM69D130V01XTSA1) accept
 * clock rates between 0.4 and 3.3MHz, making them unusable with >48k rates
 * since BCLK is fixed to 64fs. For some reason, 8k also does not work with
 * those microphones.
 *
 * Constraints are used instead of SNDRV_PCM_RATE_* in
 * snd_soc_dai_driver.capture.rates because 12KHz and 24KHz rates don't have a
 * SNDRV_PCM_RATE_ define.
 */
static const unsigned int rates11_48[] = {
	11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000,
};

static const struct snd_pcm_hw_constraint_list constraints11_48 = {
	.count	= ARRAY_SIZE(rates11_48),
	.list	= rates11_48,
};

static int ak5704_startup(struct snd_pcm_substream *substream,
			  struct snd_soc_dai *dai)
{
	snd_pcm_hw_constraint_list(substream->runtime, 0,
				   SNDRV_PCM_HW_PARAM_RATE,
				   &constraints11_48);

	return 0;
}

static int ak5704_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct ak5704_pllclk *pllclk = NULL;
	unsigned int i;
	u8 pwr_mgmt2 = 0, dmic = 0;

	for (i = 0; i < ARRAY_SIZE(ak5704_pllclks); i++) {
		if (ak5704_pllclks[i].rate == params_rate(params)) {
			pllclk = &ak5704_pllclks[i];
			break;
		}
	}

	if (!pllclk) {
		return -EINVAL;
	}

	switch (params_channels(params)) {
	case 4:
		pwr_mgmt2 |= AK5704_PWR_MGMT2_MIC2B_UP;
		/* fallthrough */
	case 3:
		pwr_mgmt2 |= AK5704_PWR_MGMT2_MIC2A_UP;
		dmic |= AK5704_DMIC_CONF_MIC2_DIGITAL | AK5704_DMIC_CONF_MIC2_DCLKE;
		/* fallthrough */
	case 2:
		pwr_mgmt2 |= AK5704_PWR_MGMT2_MIC1B_UP;
		/* fallthrough */
	case 1:
		pwr_mgmt2 |= AK5704_PWR_MGMT2_MIC1A_UP;
		dmic |= AK5704_DMIC_CONF_MIC1_DIGITAL | AK5704_DMIC_CONF_MIC1_DCLKE;
		break;
	default:
		return -EINVAL;
	}

	snd_soc_component_write(component, AK5704_DMIC_CONF, dmic);
	snd_soc_component_write(component, AK5704_PWR_MGMT2, pwr_mgmt2);

	snd_soc_component_write(component, AK5704_PLD_H, pllclk->ref_div >> 8);
	snd_soc_component_write(component, AK5704_PLD_L, pllclk->ref_div & GENMASK(7, 0));
	snd_soc_component_write(component, AK5704_PLM_H, pllclk->feedback_div >> 8);
	snd_soc_component_write(component, AK5704_PLM_L, pllclk->feedback_div & GENMASK(7, 0));

	snd_soc_component_write(component, AK5704_CLK_MODE_SEL, AK5704_CLK_MODE_SEL_FS(pllclk->fs));

	return 0;
}

static struct snd_soc_dai_ops ak5704_dai_ops = {
	.hw_params	= ak5704_hw_params,
	.startup	= ak5704_startup,
};

/* DAI */
static struct snd_soc_dai_driver ak5704_dai = {
	.name = "ak5704-aif",
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 4,
		/*
		 * Use SNDRV_PCM_RATE_KNOT because 12Khz and 24KHz and other
		 * >48KHz rates don't have their own SNDRV_PCM_RATE_* define.
		 */
		.rates = SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &ak5704_dai_ops,
};

/*
 * Enabling the PLL, second I2S lane and using BCLK as base clock for the
 * internal PLL requires some delay (~5s) before the AK5704 output something
 * valuable.
 *
 * Doing it in the component probe is fine because this is done during boot and not
 * when the alsa device is opened.
 */
static int ak5704_component_probe(struct snd_soc_component *component)
{
	snd_soc_component_write(component, AK5704_FLOW_CTRL, AK5704_FLOW_CTRL_SDTO2_EN);

	snd_soc_component_write(component, AK5704_CLK_SRC, AK5704_CLK_SRC_PLL_BCLK);

	snd_soc_component_write(component, AK5704_PWR_MGMT1, AK5704_PWR_MGMT1_PLL_UP);

	return 0;
}

/* i2c control interface */
struct snd_soc_component_driver soc_component_dev_ak5704 = {
	.probe = ak5704_component_probe,
};

#define AK5704_VALPF_COEFF_B_L	0x46

const struct regmap_config ak5704_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = AK5704_VALPF_COEFF_B_L,
};
EXPORT_SYMBOL_GPL(ak5704_regmap_config);

int ak5704_probe(struct i2c_client* i2c, const struct i2c_device_id * dev_id)
{
	struct ak5704_priv *ak5704;
	struct device *dev = &i2c->dev;
	int ret;

	ak5704 = devm_kzalloc(dev, sizeof(*ak5704), GFP_KERNEL);
	if (!ak5704)
		return -ENOMEM;

	ak5704->regmap = devm_regmap_init_i2c(i2c, &ak5704_regmap_config);
	if (IS_ERR(ak5704->regmap))
		return PTR_ERR(ak5704->regmap);

	dev_set_drvdata(dev, ak5704);

	ak5704->dev = dev;

	i2c_set_clientdata(i2c, ak5704);

	ret = devm_snd_soc_register_component(dev, &soc_component_dev_ak5704,
					      &ak5704_dai, 1);
	if (ret < 0) {
		dev_err(dev, "Failed to register COMPONENT: %d\n", ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(ak5704_probe);

static const struct i2c_device_id ak5704_i2c_id[] = {
	{ "ak5704", 0 },
	{ /* sentinel */ }
};

static const struct of_device_id ak5704_of_match[] = {
	{ .compatible = "asahi-kasei,ak5704", },
	{ /* sentinel */ }
};

static struct i2c_driver ak5704_i2c_driver = {
	.driver = {
		.name = "ak5704",
		.of_match_table = ak5704_of_match,
	},
	.probe = ak5704_probe,
	.id_table = ak5704_i2c_id,
};
module_i2c_driver(ak5704_i2c_driver);

MODULE_AUTHOR("Quentin Schulz <quentin.schulz@streamunlimited.com>");
MODULE_DESCRIPTION("ASoC AK5704 DAC driver");
MODULE_LICENSE("GPL");
