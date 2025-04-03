/*
 * PCM9211 codec driver
 *
 * Copyright (C) 2017 jusst technologies GmbH / jusst.engineering
 * Copyright (C) 2025 StreamUnlimited GmbH
 *
 * Author: Julian Scheel <julian@jusst.de>
 * Author: Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/gpio/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/workqueue.h>

#include <sound/control.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#include "pcm9211.h"

#define PCM9211_DIT_MCLK_48k	24576000
#define PCM9211_DIT_MCLK_44k1	22579200

#define PCM9211_MAX_SYSCLK	24576000
#define PCM9211_DAI_MAIN	0
#define PCM9211_DAI_AUX		1
#define PCM9211_DAI_DIT		2

/*
 * Currently both interrupts are hardcoded:
 * - INT0/IRQ0: Used to detect errors or non-PCM on the SPDIF input (DIR)
 * - INT1/IRQ1: Used for the ADC threshold detection
 */
#define PCM9211_SPDIF_ERROR_MONITOR_PERIOD_MS	50
#define PCM9211_SPDIF_ERROR_IRQ_MASK		(PCM9211_INT0_MERROR0_MASK | PCM9211_INT0_MNPCM0_MASK)

#define PCM9211_ADC_DETECT_PERIOD_MS		50

#define PCM9211_SPDIF_RATE_DETECT_PERIOD_MS	25

struct pcm9211_priv {
	struct snd_pcm_hw_constraint_list rate_constraints;

	struct snd_soc_component *component;
	struct gpio_desc *reset;
	struct regmap *regmap;
	struct device *dev;
	struct clk *xti;

	unsigned int dai_format;
	unsigned int adc_rate;
	unsigned long sysclk;

	struct delayed_work rate_detect_work;
	struct delayed_work dir_status_monitor_work;
	struct delayed_work adc_detect_work;

	// Lock for irq0_masked and irq1_masked
	spinlock_t irq_lock;
	bool irq0_masked, irq1_masked;
	int irq0, irq1;

	// Lock for dir_error, dir_npcm, adc_detected and dir_rate
	spinlock_t status_lock;
	bool dir_error;
	bool dir_npcm;
	bool adc_detected;
	// Detect DIR rate as index value into `biphase_rate_values[]`
	unsigned int dir_rate;
};

static const struct regmap_range pcm9211_reg_rd_range[] = {
	regmap_reg_range(PCM9211_ERR_OUT, PCM9211_PD_BUF1),
	regmap_reg_range(PCM9211_SYS_RESET, PCM9211_SYS_RESET),
	regmap_reg_range(PCM9211_ADC_CTRL1, PCM9211_ADC_CTRL1),
	regmap_reg_range(PCM9211_ADC_L_CH_ATT, PCM9211_ADC_CTRL3),
	regmap_reg_range(PCM9211_DIR_STATUS1, PCM9211_DIT_STATUS6),
	regmap_reg_range(PCM9211_MAIN_AUX_MUTE, PCM9211_MPIO_C_DATA_IN),
};

static const struct regmap_access_table pcm9211_reg_rd_table = {
	.yes_ranges = pcm9211_reg_rd_range,
	.n_yes_ranges = ARRAY_SIZE(pcm9211_reg_rd_range),
};

static const struct regmap_range pcm9211_reg_wr_range[] = {
	regmap_reg_range(PCM9211_ERR_OUT, PCM9211_INT1_CAUSE),
	regmap_reg_range(PCM9211_INT_POLARITY, PCM9211_FS_CALC_TARGET),
	regmap_reg_range(PCM9211_SYS_RESET, PCM9211_SYS_RESET),
	regmap_reg_range(PCM9211_ADC_CTRL1, PCM9211_ADC_CTRL1),
	regmap_reg_range(PCM9211_ADC_L_CH_ATT, PCM9211_ADC_CTRL3),
	regmap_reg_range(PCM9211_DIT_CTRL1, PCM9211_DIT_STATUS6),
	regmap_reg_range(PCM9211_MAIN_AUX_MUTE, PCM9211_MPIO_C_DATA_OUT),
};

static const struct regmap_access_table pcm9211_reg_wr_table = {
	.yes_ranges = pcm9211_reg_wr_range,
	.n_yes_ranges = ARRAY_SIZE(pcm9211_reg_wr_range),
};

static const struct regmap_range pcm9211_reg_volatile_range[] = {
	regmap_reg_range(PCM9211_INT0_OUT, PCM9211_INT1_OUT),
	regmap_reg_range(PCM9211_BIPHASE_INFO, PCM9211_PD_BUF1),
	regmap_reg_range(PCM9211_SYS_RESET, PCM9211_SYS_RESET),
	regmap_reg_range(PCM9211_DIR_STATUS1, PCM9211_DIR_STATUS6),
	regmap_reg_range(PCM9211_MPIO_A_B_DATA_IN, PCM9211_MPIO_C_DATA_IN),
};

static const struct regmap_access_table pcm9211_reg_volatile_table = {
	.yes_ranges = pcm9211_reg_volatile_range,
	.n_yes_ranges = ARRAY_SIZE(pcm9211_reg_volatile_range),
};

static const struct reg_default pcm9211_reg_defaults[] = {
	{ PCM9211_ERR_OUT, 0x00 },
	{ PCM9211_DIR_INITIAL1, 0x00 },
	{ PCM9211_DIR_INITIAL2, 0x01 },
	{ PCM9211_DIR_INITIAL3, 0x04 },
	{ PCM9211_OSC_CTRL, 0x00 },
	{ PCM9211_ERR_CAUSE, 0x01 },
	{ PCM9211_AUTO_SEL_CAUSE, 0x01 },
	{ PCM9211_DIR_FS_RANGE, 0x00 },
	{ PCM9211_NON_PCM_DEF, 0x03 },
	{ PCM9211_DTS_CD_LD, 0x0c },
	{ PCM9211_INT0_CAUSE, 0xff },
	{ PCM9211_INT1_CAUSE, 0xff },
	{ PCM9211_INT_POLARITY, 0x00 },
	{ PCM9211_DIR_OUT_FMT, 0x04 },
	{ PCM9211_DIR_RSCLK_RATIO, 0x02 },
	{ PCM9211_XTI_SCLK_FREQ, 0x1a },
	{ PCM9211_DIR_SOURCE_BIT2, 0x22 },
	{ PCM9211_XTI_SOURCE_BIT2, 0x22 },
	{ PCM9211_DIR_INP_BIPHASE, 0xc2 },
	{ PCM9211_RECOUT0_BIPHASE, 0x02 },
	{ PCM9211_RECOUT1_BIPHASE, 0x02 },
	{ PCM9211_FS_CALC_TARGET, 0x00 },
	{ PCM9211_SYS_RESET, 0x40 },
	{ PCM9211_ADC_CTRL1, 0x02 },
	{ PCM9211_ADC_L_CH_ATT, 0xd7 },
	{ PCM9211_ADC_R_CH_ATT, 0xd7 },
	{ PCM9211_ADC_CTRL2, 0x00 },
	{ PCM9211_ADC_CTRL3, 0x00 },
	{ PCM9211_DIR_STATUS1, 0x04 },
	{ PCM9211_DIR_STATUS2, 0x00 },
	{ PCM9211_DIR_STATUS3, 0x00 },
	{ PCM9211_DIR_STATUS4, 0x00 },
	{ PCM9211_DIR_STATUS5, 0x00 },
	{ PCM9211_DIR_STATUS6, 0x00 },
	{ PCM9211_DIT_CTRL1, 0x44 },
	{ PCM9211_DIT_CTRL2, 0x10 },
	{ PCM9211_DIT_CTRL3, 0x00 },
	{ PCM9211_MAIN_AUX_MUTE, 0x00 },
	{ PCM9211_MAIN_OUT_SOURCE, 0x00 },
	{ PCM9211_AUX_OUT_SOURCE, 0x00 },
	{ PCM9211_MPIO_B_MAIN_HIZ, 0x00 },
	{ PCM9211_MPIO_C_MPIO_A_HIZ, 0x0f },
	{ PCM9211_MPIO_GROUP, 0x40 },
	{ PCM9211_MPIO_A_FLAGS, 0x00 },
	{ PCM9211_MPIO_B_MPIO_C_FLAGS, 0x00 },
	{ PCM9211_MPIO_A1_A0_OUT_FLAG, 0x00 },
	{ PCM9211_MPIO_A3_A2_OUT_FLAG, 0x00 },
	{ PCM9211_MPIO_B1_B0_OUT_FLAG, 0x00 },
	{ PCM9211_MPIO_B3_B2_OUT_FLAG, 0x00 },
	{ PCM9211_MPIO_C1_C0_OUT_FLAG, 0x00 },
	{ PCM9211_MPIO_C3_C2_OUT_FLAG, 0x00 },
	{ PCM9211_MPO_1_0_FUNC, 0x3d },
	{ PCM9211_MPIO_A_B_DIR, 0x00 },
	{ PCM9211_MPIO_C_DIR, 0x00 },
	{ PCM9211_MPIO_A_B_DATA_OUT, 0x00 },
	{ PCM9211_MPIO_C_DATA_OUT, 0x00 },
};

const struct regmap_config pcm9211_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = PCM9211_MPIO_C_DATA_IN,
	.wr_table = &pcm9211_reg_wr_table,
	.rd_table = &pcm9211_reg_rd_table,
	.volatile_table = &pcm9211_reg_volatile_table,
	.reg_defaults = pcm9211_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(pcm9211_reg_defaults),
	.cache_type = REGCACHE_RBTREE,
};
EXPORT_SYMBOL_GPL(pcm9211_regmap);

static const u32 adc_rates[] = { 48000, 96000 };
static const struct snd_pcm_hw_constraint_list adc_rate_constraints = {
	.count = ARRAY_SIZE(adc_rates),
	.list = adc_rates,
};


static const int biphase_rate_values[] = {
	0, 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100,
	48000, 64000, 88200, 96000, 128000, 176400, 192000
};

static const char * const biphase_rate_text[] = {
	"N/A", "8000", "11025", "12000", "16000", "22050", "24000", "32000", "44100",
	"48000", "64000", "88200", "96000", "128000", "176400", "192000"
};
SOC_ENUM_SINGLE_EXT_DECL(biphase_rate, biphase_rate_text);

static const unsigned int pcm9211_sck_ratios[] = { 1, 2, 4, 8 };
static const unsigned int pcm9211_bck_ratios[] = { 2, 4, 8, 16 };
static const unsigned int pcm9211_lrck_ratios[] = { 128, 256, 512, 1024 };
#define PCM9211_NUM_SCK_RATIOS ARRAY_SIZE(pcm9211_sck_ratios)
#define PCM9211_NUM_BCK_RATIOS ARRAY_SIZE(pcm9211_bck_ratios)
#define PCM9211_NUM_LRCK_RATIOS ARRAY_SIZE(pcm9211_lrck_ratios)

static int pcm9211_get_output_port(struct device *dev, int dai_id)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	unsigned int port;
	unsigned int val;
	int reg = 0;
	int ret;

	switch (dai_id) {
	case PCM9211_DAI_MAIN:
		reg = PCM9211_MAIN_OUT_SOURCE;
		break;
	case PCM9211_DAI_AUX:
		reg = PCM9211_AUX_OUT_SOURCE;
		break;
	default:
		return -EINVAL;
	}

	ret = regmap_read(priv->regmap, reg, &val);
	if (ret) {
		dev_err(dev, "Failed to read selected source: %d\n", ret);
		return ret;
	}

	port = (val & PCM9211_MOPSRC_MASK) >> PCM9211_MOPSRC_SHIFT;
	if (port == PCM9211_MOSRC_AUTO) {
		ret = regmap_read(priv->regmap, PCM9211_BIPHASE_INFO, &val);
		if (ret) {
			dev_err(dev, "Failed to read biphase information: %d\n",
					ret);
			return ret;
		}

		/* Assumes that Sampling Frequency Status calculation
		 * corresponds with DIR Lock, which seems to to be exposed to
		 * any register directly
		 */
		if ((val & PCM9211_BIPHASE_SFSST_MASK) == 0)
			port = PCM9211_MOSRC_DIR;
		else
			port = PCM9211_MOSRC_ADC;
	}

	return port;
}

static int pcm9211_dir_rate_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	unsigned long flags;

	spin_lock_irqsave(&priv->status_lock, flags);
	ucontrol->value.enumerated.item[0] = priv->dir_rate;
	spin_unlock_irqrestore(&priv->status_lock, flags);

	return 0;
}

static int pcm9211_reset(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	int ret;

	/* Use reset gpio if available, otherwise soft-reset */
	if (priv->reset) {
		gpiod_set_value_cansleep(priv->reset, 1);
		usleep_range(100, 250);
		gpiod_set_value_cansleep(priv->reset, 0);
		usleep_range(100, 250);
	} else {
		ret = regmap_update_bits(priv->regmap, PCM9211_SYS_RESET,
				PCM9211_SYS_RESET_MRST, 0);
		if (ret) {
			dev_err(dev, "Could not reset device: %d\n", ret);
			return ret;
		}
		usleep_range(10000, 15000);
	}

	regcache_mark_dirty(priv->regmap);

	return 0;
}

static void pcm9211_snd_ctl_notify(struct device *dev, const char *ctl_name)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	struct snd_kcontrol *ctl;

	if (priv->component == NULL)
		return;

	ctl = snd_soc_card_get_kcontrol(priv->component->card, ctl_name);
	if (ctl == NULL)
		return;

	snd_ctl_notify(priv->component->card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE, &ctl->id);
}

static void pcm9211_update_biphase_info(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	struct snd_kcontrol *ctl;
	unsigned int val = 0, rate;
	unsigned long flags;
	int ret;

	ret = regmap_read(priv->regmap, PCM9211_BIPHASE_INFO, &val);
	if (ret) {
		dev_dbg(dev, "Failed to read biphase information: %d\n", ret);
		return;
	}

	/*
	 * Set the rate to 0 (N/A) if the `SFSST` bit is set which indicates
	 * that the signal is unlocked or the frequency calculation is in progress.
	 */
	if (!(val & PCM9211_BIPHASE_SFSST_MASK))
		rate = (val & PCM9211_BIPHASE_SFSOUT_MASK) >> PCM9211_BIPHASE_SFSOUT_SHIFT;
	else
		rate = 0;

	if (rate != priv->dir_rate) {

		spin_lock_irqsave(&priv->status_lock, flags);
		priv->dir_rate = rate;
		spin_unlock_irqrestore(&priv->status_lock, flags);

		pcm9211_snd_ctl_notify(dev, "DIR Sample Rate");

#if 0
		/*
		 * TODO: Check if the following is still needed, see https://jira.streamunlimited.com/browse/SPLATEAMP-322
		 * If the detected rate is 0 and previously we had a rate, that means we have lost
		 * the lock, perform a reset of the PCM9211 just in case. This fixes an issue where
		 * sometimes the PCM9211 will get in a state switching between locked and unlocked
		 * even though a valid signal is present.
		 */
		if (priv->dir_rate == 0) {
			pcm9211_reset(dev);
			regcache_mark_dirty(priv->regmap);
			regcache_sync(priv->regmap);
		}
#endif

		dev_dbg(dev, "DIR sampling rate changed to: %d\n", biphase_rate_values[rate]);
	}
}

static void pcm9211_update_dir_status(struct device *dev, u8 int_cause)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	struct snd_kcontrol *ctl;
	unsigned long flags;

	bool new_error_state = int_cause & PCM9211_INT0_MERROR0_MASK;
	bool new_npcm_state = int_cause & PCM9211_INT0_MNPCM0_MASK;

	if (priv->dir_error != new_error_state) {

		spin_lock_irqsave(&priv->status_lock, flags);
		priv->dir_error = new_error_state;
		spin_unlock_irqrestore(&priv->status_lock, flags);

		pcm9211_snd_ctl_notify(dev, "DIR Error");

		dev_info(dev, "Updated DIR error state: %d\n", priv->dir_error);
	}

	if (priv->dir_npcm != new_npcm_state) {
		spin_lock_irqsave(&priv->status_lock, flags);
		priv->dir_npcm = new_npcm_state;
		spin_unlock_irqrestore(&priv->status_lock, flags);

		pcm9211_snd_ctl_notify(dev, "DIR Non-PCM");

		dev_info(dev, "Updated DIR NPCM state: %d\n", priv->dir_npcm);
	}
}

static void pcm9211_dir_status_monitor_work(struct work_struct *work)
{
	unsigned long flags;
	struct pcm9211_priv *priv = container_of(work, struct pcm9211_priv, dir_status_monitor_work.work);
	struct device *dev = priv->dev;
	u32 int_cause = 0;
	int ret;

	ret = regmap_read(priv->regmap, PCM9211_INT0_OUT, &int_cause);
	if (ret < 0) {
		dev_err(dev, "Failed to read int0 output: %d\n", ret);
		goto out_resched;
	}

	pcm9211_update_dir_status(dev, int_cause);

	if (int_cause & PCM9211_SPDIF_ERROR_IRQ_MASK)
		goto out_resched;

	// If we arrive here, both conditions (ERROR, NPCM) are cleared, enable interrupts again
	spin_lock_irqsave(&priv->irq_lock, flags);
	if (priv->irq0_masked) {
		enable_irq(priv->irq0);
		priv->irq0_masked = false;
	}
	spin_unlock_irqrestore(&priv->irq_lock, flags);

	return;

out_resched:
	schedule_delayed_work(&priv->dir_status_monitor_work, msecs_to_jiffies(PCM9211_SPDIF_ERROR_MONITOR_PERIOD_MS));
}


static irqreturn_t pcm9211_irq0(int irq, void *data)
{
	unsigned long flags;
	struct pcm9211_priv *priv = data;
	struct device *dev = priv->dev;
	u32 int_cause = 0;
	int ret;

	ret = regmap_read(priv->regmap, PCM9211_INT0_OUT, &int_cause);
	if (ret < 0) {
		dev_err(dev, "Failed to read int0 output: %d\n", ret);
		return IRQ_HANDLED;
	}

	if (int_cause & PCM9211_SPDIF_ERROR_IRQ_MASK) {
		spin_lock_irqsave(&priv->irq_lock, flags);
		if (!priv->irq0_masked) {
			disable_irq_nosync(priv->irq0);
			priv->irq0_masked = true;

			schedule_delayed_work(&priv->dir_status_monitor_work, msecs_to_jiffies(PCM9211_SPDIF_ERROR_MONITOR_PERIOD_MS));
		}
		spin_unlock_irqrestore(&priv->irq_lock, flags);
	}

	pcm9211_update_dir_status(dev, int_cause);

	return IRQ_HANDLED;
}

static void pcm9211_update_adc_detect_state(struct device *dev, u8 int_cause)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	struct snd_kcontrol *ctl;
	unsigned long flags;

	bool new_adc_detect_state = int_cause & PCM9211_INT1_MADLVL1_MASK;

	if (priv->adc_detected != new_adc_detect_state) {
		spin_lock_irqsave(&priv->status_lock, flags);
		priv->adc_detected = new_adc_detect_state;
		spin_unlock_irqrestore(&priv->status_lock, flags);

		pcm9211_snd_ctl_notify(dev, "ADC Detected");

		dev_dbg(dev, "Updated ADC detected state: %d\n", priv->adc_detected);
	}
}

static void pcm9211_adc_detect_work(struct work_struct *work)
{
	unsigned long flags;
	struct pcm9211_priv *priv = container_of(work, struct pcm9211_priv, adc_detect_work.work);
	struct device *dev = priv->dev;
	u32 int_cause = 0;
	int ret;

	ret = regmap_read(priv->regmap, PCM9211_INT1_OUT, &int_cause);
	if (ret < 0) {
		dev_err(dev, "Failed to read int1 output: %d\n", ret);
		goto out_resched;
	}

	pcm9211_update_adc_detect_state(dev, int_cause);

	if (int_cause & PCM9211_INT1_MADLVL1_MASK)
		goto out_resched;

	// If we arrive here, the ADC detect condition is cleared, enable interrupts again
	spin_lock_irqsave(&priv->irq_lock, flags);
	if (priv->irq1_masked) {
		enable_irq(priv->irq1);
		priv->irq1_masked = false;
	}
	spin_unlock_irqrestore(&priv->irq_lock, flags);

	return;

out_resched:
	schedule_delayed_work(&priv->adc_detect_work, msecs_to_jiffies(PCM9211_ADC_DETECT_PERIOD_MS));
}

static irqreturn_t pcm9211_irq1(int irq, void *data)
{
	unsigned long flags;
	struct pcm9211_priv *priv = data;
	struct device *dev = priv->dev;
	u32 int_cause = 0;
	int ret;

	ret = regmap_read(priv->regmap, PCM9211_INT1_OUT, &int_cause);
	if (ret < 0) {
		dev_err(dev, "Failed to read int1 output: %d\n", ret);
		return IRQ_HANDLED;
	}

	if (int_cause & PCM9211_INT1_MADLVL1_MASK) {
		spin_lock_irqsave(&priv->irq_lock, flags);
		if (!priv->irq1_masked) {
			disable_irq_nosync(priv->irq1);
			priv->irq1_masked = true;

			schedule_delayed_work(&priv->adc_detect_work, msecs_to_jiffies(PCM9211_ADC_DETECT_PERIOD_MS));
		}
		spin_unlock_irqrestore(&priv->irq_lock, flags);
	}

	pcm9211_update_adc_detect_state(dev, int_cause);

	return IRQ_HANDLED;
}

static int pcm9211_put_dir_mux(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret;

	ret = snd_soc_dapm_put_enum_double(kcontrol, ucontrol);

	return ret;
}

static int pcm9211_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct pcm9211_priv *priv = snd_soc_dai_get_drvdata(dai);
	struct snd_soc_component *component = priv->component;
	struct device *dev = component->dev;
	struct snd_soc_dai *other_dai;
	int port;

	port = pcm9211_get_output_port(dev, dai->id);
	if (port < 0) {
		dev_err(dev, "Failed to read selected port: %d\n", port);
		return port;
	}

	if (port == PCM9211_MOSRC_ADC) {
		dev_info(dev, "ADC capture on DAI %d\n", dai->id);
		/* Check if other DAI uses ADC, if so limit available rates */
		list_for_each_entry(other_dai, &component->dai_list, list) {
			if (!other_dai->capture_active)
				continue;

			if (pcm9211_get_output_port(dev, other_dai->id) != port)
				continue;

			priv->rate_constraints.count = 1;
			priv->rate_constraints.list = &priv->adc_rate;
			priv->rate_constraints.mask = 0;

			dev_info(dev, "Active ADC rate is %d Hz\n",
					priv->adc_rate);

			return snd_pcm_hw_constraint_list(substream->runtime,
					0, SNDRV_PCM_HW_PARAM_RATE,
					&priv->rate_constraints);
		}

		return snd_pcm_hw_constraint_list(substream->runtime,
				0, SNDRV_PCM_HW_PARAM_RATE,
				&adc_rate_constraints);
	} else if (port == PCM9211_MOSRC_DIR) {
		unsigned long flags;
		unsigned int dir_rate;

		spin_lock_irqsave(&priv->status_lock, flags);
		dir_rate = priv->dir_rate;
		spin_unlock_irqrestore(&priv->status_lock, flags);

		dev_info(dev, "DIR capture on DAI %d, detected biphase rate is %d Hz\n",
				dai->id, biphase_rate_values[dir_rate]);

		if (dir_rate == 0) {
			/* rate is invalid */
			return -EIO;
		} else {
			/* sample rate detector is locked */
			priv->rate_constraints.count = 1;
			priv->rate_constraints.list = &biphase_rate_values[dir_rate];
			priv->rate_constraints.mask = 0;

			return snd_pcm_hw_constraint_list(substream->runtime,
					0, SNDRV_PCM_HW_PARAM_RATE,
					&priv->rate_constraints);
		}
	} else { /* AUXIN# */
		dev_info(dev, "AUXIN%d capture on DAI %d\n",
				port - PCM9211_MOSRC_AUXIN0, dai->id);

		/* TODO: we'd need to forward the call to the DAI connected to AUXIN#
		 * which is tough to figure out, so just assume the caller knows what it's doing */

		return 0;
	}
}

static int pcm9211_set_dai_sysclk(struct snd_soc_dai *dai, int clk_id,
		unsigned int freq, int dir)
{
	struct snd_soc_component *component = dai->component;
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	int ret;

	if (!priv->xti)
		return 0;

	if (freq > PCM9211_MAX_SYSCLK) {
		dev_err(dev, "System clock greater %d is not supported\n",
				PCM9211_MAX_SYSCLK);
		return -EINVAL;
	}

	ret = clk_set_rate(priv->xti, freq);
	if (ret)
		return ret;

	priv->sysclk = freq;

	return 0;
}

static int pcm9211_set_dai_fmt(struct snd_soc_dai *dai,
		unsigned int format)
{
	struct snd_soc_component *component = dai->component;
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	u32 adfmt, dirfmt;
	int ret;

	if (priv->dai_format != 0 && priv->dai_format != format) {
		dev_err(dev, "Can not use different DAI formats for DAI links.\n");
		return -EINVAL;
	}

	/* Configure format for ADC and DIR block, if main output source is
	 * set to AUTO the output port may switch between them at any time
	 */
	switch (format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		adfmt = PCM9211_ADFMT_I2S;
		dirfmt = PCM9211_DIR_FMT_I2S;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		adfmt = PCM9211_ADFMT_RIGHT_J;
		dirfmt = PCM9211_DIR_FMT_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		adfmt = PCM9211_ADFMT_LEFT_J;
		dirfmt = PCM9211_DIR_FMT_LEFT_J;
		break;
	default:
		dev_err(dev, "Unsupported DAI format\n");
		return -EINVAL;
	}

	ret = regmap_update_bits(priv->regmap, PCM9211_ADC_CTRL2,
			PCM9211_ADFMT_MASK, adfmt << PCM9211_ADFMT_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to update ADC format: %d\n", ret);
		return ret;
	}

	ret = regmap_update_bits(priv->regmap, PCM9211_DIR_OUT_FMT,
			PCM9211_DIR_FMT_MASK, dirfmt << PCM9211_DIR_FMT_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to update ADC format: %d\n", ret);
		return ret;
	}

	priv->dai_format = format;

	return 0;
}

static int pcm9211_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	unsigned int sclk = 1;
	unsigned int rate;
	unsigned int bck;
	unsigned int ratio;
	unsigned int port;
	int ret;
	int i;

	rate = params_rate(params);
	bck = rate * 64;

	port = pcm9211_get_output_port(dev, dai->id);
	if ((port == PCM9211_MOSRC_ADC) && priv->xti) {
		switch (rate) {
		case 48000:
			sclk = 12288000;
			break;
		case 96000:
			sclk = 24576000;
			break;
		default:
			dev_err(dev, "Rate %d unsupported.\n", rate);
			return -EINVAL;
		}

		/* Systemclock setup */
		ratio = priv->sysclk / sclk;
		for (i = 0; i < PCM9211_NUM_SCK_RATIOS; i++) {
			if (pcm9211_sck_ratios[i] == ratio)
				break;
		}
		if (i == PCM9211_NUM_SCK_RATIOS) {
			dev_err(dev, "SCK divider %d is not supported\n",
					ratio);
			return -EINVAL;
		}
		ret = regmap_update_bits(priv->regmap, PCM9211_XTI_SCLK_FREQ,
				PCM9211_XTI_XSCK_MASK,
				i << PCM9211_XTI_XSCK_SHIFT);

		if (ret) {
			dev_err(dev, "Failed to configure SCK divider: %d\n",
					ret);
			return ret;
		}

		/* Bitclock setup */
		ratio = priv->sysclk / bck;
		for (i = 0; i < PCM9211_NUM_BCK_RATIOS; i++) {
			if (pcm9211_bck_ratios[i] == ratio)
				break;
		}
		if (i == PCM9211_NUM_BCK_RATIOS) {
			dev_err(dev, "BCK divider %d is not supported\n",
					ratio);
			return -EINVAL;
		}
		ret = regmap_update_bits(priv->regmap, PCM9211_XTI_SCLK_FREQ,
				PCM9211_XTI_BCK_MASK,
				i << PCM9211_XTI_BCK_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to configure BCK divider: %d\n",
					ret);
			return ret;
		}

		/* Frameclock setup */
		ratio = priv->sysclk / rate;
		for (i = 0; i < PCM9211_NUM_LRCK_RATIOS; i++) {
			if (pcm9211_lrck_ratios[i] == ratio)
				break;
		}
		if (i == PCM9211_NUM_LRCK_RATIOS) {
			dev_err(dev, "LRCK divider %d is not supported\n",
					ratio);
			return -EINVAL;
		}
		ret = regmap_update_bits(priv->regmap, PCM9211_XTI_SCLK_FREQ,
				PCM9211_XTI_LRCK_MASK,
				i << PCM9211_XTI_LRCK_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to configure LRCK divider: %d\n",
					ret);
			return ret;
		}

		priv->adc_rate = rate;
	}

	return 0;
}

static int pcm9211_write_pinconfig(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	u8 values[4];
	int val;
	int ret;
	int i;

	ret = of_property_read_u8_array(dev->of_node, "ti,group-function",
			values, 3);
	if (!ret) {
		val = (values[0] << PCM9211_MPASEL_SHIFT &
				PCM9211_MPASEL_MASK) |
			(values[1] << PCM9211_MPBSEL_SHIFT &
				 PCM9211_MPBSEL_MASK) |
			(values[2] << PCM9211_MPCSEL_SHIFT &
				PCM9211_MPCSEL_MASK);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_GROUP, val);
		if (ret) {
			dev_err(dev, "Failed to write mpio group functions: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-a-flags-gpio",
			values, 4);
	if (!ret) {
		/* Write MPIO A flags/gpio selection */
		for (i = 0, val = 0; i < 4; i++)
			val |= (values[i] << PCM9211_MPAxSEL_SHIFT(i)) &
				PCM9211_MPAxSEL_MASK(i);

		ret = regmap_update_bits(priv->regmap, PCM9211_MPIO_A_FLAGS,
				PCM9211_MPAxSEL_MASK(0) |
				PCM9211_MPAxSEL_MASK(1) |
				PCM9211_MPAxSEL_MASK(2), val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_a flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-b-flags-gpio",
			values, 4);
	if (!ret) {
		/* Write MPIO B flags/gpio selection */
		for (i = 0, val = 0; i < 4; i++)
			val |= (values[i] << PCM9211_MPBxSEL_SHIFT(i)) &
				PCM9211_MPBxSEL_MASK(i);

		ret = regmap_update_bits(priv->regmap,
				PCM9211_MPIO_B_MPIO_C_FLAGS,
				PCM9211_MPBxSEL_MASK(0) |
				PCM9211_MPBxSEL_MASK(1) |
				PCM9211_MPBxSEL_MASK(2), val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_a flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-c-flags-gpio",
			values, 4);
	if (!ret) {
		/* Write MPIO B flags/gpio selection */
		for (i = 0, val = 0; i < 4; i++)
			val |= (values[i] << PCM9211_MPCxSEL_SHIFT(i)) &
				PCM9211_MPCxSEL_MASK(i);

		ret = regmap_update_bits(priv->regmap,
				PCM9211_MPIO_B_MPIO_C_FLAGS,
				PCM9211_MPCxSEL_MASK(0) |
				PCM9211_MPCxSEL_MASK(1) |
				PCM9211_MPCxSEL_MASK(2), val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_a flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-a-flag",
			values, 4);
	if (!ret) {
		/* Write MPIO A flag selection */
		for (i = 0, val = 0; i < 2; i++)
			val |= (values[i] <<
					PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_A1_A0_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_a1/0 flags: %d\n",
					ret);
			return ret;
		}

		for (i = 2, val = 0; i < 4; i++)
			val |= (values[i] <<
					PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_A3_A2_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_a3/2 flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-b-flag",
			values, 4);
	if (!ret) {
		/* Write MPIO B flag selection */
		for (i = 0, val = 0; i < 2; i++)
			val |= (values[i] << PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_B1_B0_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_b1/0 flags: %d\n",
					ret);
			return ret;
		}

		for (i = 2, val = 0; i < 4; i++)
			val |= (values[i] << PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_B3_B2_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_b3/2 flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpio-c-flag",
				  values, 4);
	if (!ret) {
		/* Write MPIO C flag selection */
		for (i = 0, val = 0; i < 2; i++)
			val |= (values[i] << PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_C1_C0_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_c1/0 flags: %d\n",
					ret);
			return ret;
		}

		for (i = 2, val = 0; i < 4; i++)
			val |= (values[i] << PCM9211_MPIO_ABCx_FLAG_SHIFT(i)) &
				PCM9211_MPIO_ABCx_FLAG_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_C3_C2_OUT_FLAG,
				val);
		if (ret) {
			dev_err(dev, "Failed to update mpio_c3/2 flags: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,mpo-function",
			values, 2);
	if (!ret) {
		/* Write MPO function selection */
		for (i = 0, val = 0; i < 2; i++)
			val |= (values[i] << PCM9211_MPOxOUT_SHIFT(i)) &
				PCM9211_MPOxOUT_MASK(i);
		ret = regmap_write(priv->regmap, PCM9211_MPO_1_0_FUNC, val);
		if (ret) {
			dev_err(dev, "Failed to update mpo function selection: %d\n",
					ret);
			return ret;
		}
	}

	ret = of_property_read_u8_array(dev->of_node, "ti,hiz-mask", values, 4);
	if (!ret) {
		uint8_t reg = (values[0] << PCM9211_MPIO_A_HIZ_SHIFT & PCM9211_MPIO_A_HIZ_MASK) |
				(values[2] << PCM9211_MPIO_C_HIZ_SHIFT & PCM9211_MPIO_C_HIZ_MASK);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_C_MPIO_A_HIZ, reg);
		if (ret) {
			dev_err(dev, "Failed to update MPIO_A/MPIO_C HiZ configuration: %d\n", ret);
			return ret;
		}

		reg = (values[1] << PCM9211_MPIO_B_HIZ_SHIFT & PCM9211_MPIO_B_HIZ_MASK) |
			(values[3] << PCM9211_MAIN_HIZ_SHIFT & PCM9211_MAIN_HIZ_MASK);
		ret = regmap_write(priv->regmap, PCM9211_MPIO_B_MAIN_HIZ, reg);
		if (ret) {
			dev_err(dev, "Failed to update MPIO_B/MAIN HiZ configuration: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static void pcm9211_rate_detect_work(struct work_struct *work)
{
	struct pcm9211_priv *priv = container_of(work, struct pcm9211_priv, rate_detect_work.work);
	struct device *dev = priv->dev;

	pcm9211_update_biphase_info(dev);

	schedule_delayed_work(&priv->rate_detect_work, msecs_to_jiffies(PCM9211_SPDIF_RATE_DETECT_PERIOD_MS));
}

static int pcm9211_soc_probe(struct snd_soc_component *component)
{
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	u32 val;
	int ret;

	priv->component = component;
	dev_info(dev, "component probe\n");

	if (priv->xti) {
		ret = clk_prepare_enable(priv->xti);
		if (ret) {
			dev_err(dev, "Failed to enable xti clock: %d\n", ret);
			return ret;
		}

		priv->sysclk = clk_get_rate(priv->xti);
		if (priv->sysclk > PCM9211_MAX_SYSCLK) {
			dev_err(dev, "xti clock rate (%lu) exceeds supported max %u\n",
					priv->sysclk, PCM9211_MAX_SYSCLK);

			ret = -EINVAL;
			goto err;
		}
	}

	pcm9211_reset(dev);

	ret = pcm9211_write_pinconfig(dev);
	if (ret)
		goto err;

	if (priv->irq0) {
		INIT_DELAYED_WORK(&priv->dir_status_monitor_work, pcm9211_dir_status_monitor_work);

		ret = devm_request_threaded_irq(dev, priv->irq0, NULL, pcm9211_irq0, IRQF_ONESHOT, "pcm9211-int0", priv);
		if (ret < 0) {
			dev_err(dev, "Failed to request int0: %d\n", ret);
			goto err;
		}

		ret = regmap_update_bits(priv->regmap, PCM9211_ERR_OUT, PCM9211_ERROR_INT0_MASK, PCM9211_ERROR_INT0_MASK);
		if (ret < 0) {
			dev_err(dev, "Failed to set ERROR port to int0: %d\n", ret);
			goto err;
		}

		ret = regmap_update_bits(priv->regmap, PCM9211_INT0_CAUSE, PCM9211_SPDIF_ERROR_IRQ_MASK, 0);
		if (ret < 0) {
			dev_err(dev, "Failed to set int0 cause: %d\n", ret);
			goto err;
		}
	}

	if (priv->irq1) {
		INIT_DELAYED_WORK(&priv->adc_detect_work, pcm9211_adc_detect_work);

		ret = devm_request_threaded_irq(dev, priv->irq1, NULL, pcm9211_irq1, IRQF_ONESHOT, "pcm9211-int1", priv);
		if (ret < 0) {
			dev_err(dev, "Failed to request int1: %d\n", ret);
			goto err;
		}

		ret = regmap_update_bits(priv->regmap, PCM9211_ERR_OUT, PCM9211_NPCM_INT1_MASK, PCM9211_NPCM_INT1_MASK);
		if (ret < 0) {
			dev_err(dev, "Failed to set NPCM port to int1: %d\n", ret);
			goto err;
		}

		ret = regmap_update_bits(priv->regmap, PCM9211_INT1_CAUSE, PCM9211_INT1_MADLVL1_MASK, 0);
		if (ret < 0) {
			dev_err(dev, "Failed to set int1 cause: %d\n", ret);
			goto err;
		}
	}

	INIT_DELAYED_WORK(&priv->rate_detect_work, pcm9211_rate_detect_work);
	schedule_delayed_work(&priv->rate_detect_work, msecs_to_jiffies(PCM9211_SPDIF_RATE_DETECT_PERIOD_MS));

#ifdef CONFIG_PM
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
#endif

	return 0;

err:
	if (priv->xti)
		clk_disable_unprepare(priv->xti);

	return ret;

}

static void pcm9211_soc_remove(struct snd_soc_component *component)
{
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);

	if (priv->xti)
		clk_disable_unprepare(priv->xti);

#ifdef CONFIG_PM
	struct device *dev = component->dev;

	pm_runtime_disable(dev);
#endif
}

/* Simple Controls */
static const DECLARE_TLV_DB_SCALE(pcm9211_adc_tlv, -10050, 50, 1);
static const char *const pcm9211_main_outputs[] = { "AUTO", "DIR", "ADC",
	"AUXIN0", "AUXIN1", "AUXIN2" };
static const struct soc_enum pcm9211_main_sclk_enum =
	SOC_ENUM_SINGLE(PCM9211_MAIN_OUT_SOURCE, 4, 6, pcm9211_main_outputs);
static const struct soc_enum pcm9211_aux_sclk_enum =
	SOC_ENUM_SINGLE(PCM9211_AUX_OUT_SOURCE, 4, 5, pcm9211_main_outputs);
static const char *const pcm9211_adc_clks[] = { "AUTO", "DIR", "XTI",
	"AUXIN0", "AUXIN1", "AUXIN2" };
static const struct soc_enum pcm9211_adc_clk_enum =
	SOC_ENUM_SINGLE(PCM9211_ADC_CTRL1, 0, 6, pcm9211_adc_clks);

static const char *const pcm9211_adc_det_levels[] = { "-12 dB", "-24 dB", "-36 dB", "-48 dB" };
static const struct soc_enum pcm9211_adc_det_level_enum =
	SOC_ENUM_SINGLE(PCM9211_INT_POLARITY, 3, 4, pcm9211_adc_det_levels);

static int pcm9211_spdif_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;

	return 0;
}

static int pcm9211_spdif_capture_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int i;
	uint8_t buffer[6];
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);

	regmap_bulk_read(priv->regmap, PCM9211_DIR_STATUS1, &buffer, ARRAY_SIZE(buffer));

	for (i = 0; i < ARRAY_SIZE(buffer); i++) {
		ucontrol->value.iec958.status[i] = buffer[i];
	}

	return 0;
}

enum pcm9211_ctl_id {
	PCM9211_CTL_DIR_ERROR,
	PCM9211_CTL_DIR_NPCM,
	PCM9211_CTL_ADC_DETECTED,
};


static int pcm9211_bool_status_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	enum pcm9211_ctl_id id = kcontrol->private_value;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&priv->status_lock, flags);
	switch (id) {
		case PCM9211_CTL_DIR_ERROR:
			ucontrol->value.integer.value[0] = priv->dir_error;
			break;
		case PCM9211_CTL_DIR_NPCM:
			ucontrol->value.integer.value[0] = priv->dir_npcm;
			break;
		case PCM9211_CTL_ADC_DETECTED:
			ucontrol->value.integer.value[0] = priv->adc_detected;
			break;
		default:
			dev_warn(priv->dev, "invalid ctl id: %d\n", id);
			ucontrol->value.integer.value[0] = 0;
			ret = -EINVAL;
			break;
	}
	spin_unlock_irqrestore(&priv->status_lock, flags);

	return ret;
}

static const struct snd_kcontrol_new pcm9211_snd_controls[] = {
	SOC_DOUBLE_R_RANGE_TLV("ADC Capture Volume",
			PCM9211_ADC_L_CH_ATT,
			PCM9211_ADC_R_CH_ATT,
			0, 14, 255, 0, pcm9211_adc_tlv),
	SOC_ENUM_EXT("DIR Sample Rate", biphase_rate, pcm9211_dir_rate_get, NULL),
	SOC_SINGLE_BOOL_EXT("DIR Error", PCM9211_CTL_DIR_ERROR, pcm9211_bool_status_get, NULL),
	SOC_SINGLE_BOOL_EXT("DIR Non-PCM", PCM9211_CTL_DIR_NPCM, pcm9211_bool_status_get, NULL),
	SOC_SINGLE_BOOL_EXT("ADC Detected", PCM9211_CTL_ADC_DETECTED, pcm9211_bool_status_get, NULL),
	SOC_ENUM("ADC Detect Level", pcm9211_adc_det_level_enum),

	SOC_ENUM("MAIN SCLK Output Select", pcm9211_main_sclk_enum),
	SOC_ENUM("AUX SCLK Output Select", pcm9211_aux_sclk_enum),
	SOC_ENUM("ADC CLK Select", pcm9211_adc_clk_enum),
	{
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.name = SNDRV_CTL_NAME_IEC958("PCM9211 ", CAPTURE, DEFAULT),
		.access = SNDRV_CTL_ELEM_ACCESS_READ |
			SNDRV_CTL_ELEM_ACCESS_VOLATILE,
		.info = pcm9211_spdif_info,
		.get = pcm9211_spdif_capture_get,
	},
};

/* DAPM Controls */
static const char *const pcm9211_dir_inputs[] = { "RXIN0", "RXIN1", "RXIN2",
	"RXIN3", "RXIN4", "RXIN5", "RXIN6", "RXIN7" };
static const struct soc_enum pcm9211_dir_mux_enum =
	SOC_ENUM_SINGLE(PCM9211_DIR_INP_BIPHASE, 0, 8, pcm9211_dir_inputs);
static const struct snd_kcontrol_new pcm9211_dir_mux_control =
	SOC_DAPM_ENUM_EXT("DIR Input Select", pcm9211_dir_mux_enum,
	snd_soc_dapm_get_enum_double, pcm9211_put_dir_mux);

static const struct soc_enum pcm9211_main_out_enum =
	SOC_ENUM_SINGLE(PCM9211_MAIN_OUT_SOURCE, 0, 6, pcm9211_main_outputs);
static const struct snd_kcontrol_new pcm9211_main_out_control =
	SOC_DAPM_ENUM("MAIN Output Select", pcm9211_main_out_enum);

static const struct soc_enum pcm9211_aux_out_enum =
	SOC_ENUM_SINGLE(PCM9211_AUX_OUT_SOURCE, 0, 5, pcm9211_main_outputs);
static const struct snd_kcontrol_new pcm9211_aux_out_control =
	SOC_DAPM_ENUM("AUX Output Select", pcm9211_aux_out_enum);

/* DAPM widgets */
static const struct snd_soc_dapm_widget pcm9211_dapm_widgets[] = {
	/* Inputs */
	SND_SOC_DAPM_INPUT("RXIN0"),
	SND_SOC_DAPM_INPUT("RXIN1"),
	SND_SOC_DAPM_INPUT("RXIN2"),
	SND_SOC_DAPM_INPUT("RXIN3"),
	SND_SOC_DAPM_INPUT("RXIN4"),
	SND_SOC_DAPM_INPUT("RXIN5"),
	SND_SOC_DAPM_INPUT("RXIN6"),
	SND_SOC_DAPM_INPUT("RXIN7"),
	SND_SOC_DAPM_INPUT("VINL"),
	SND_SOC_DAPM_INPUT("VINR"),

	SND_SOC_DAPM_ADC("ADC", NULL, PCM9211_SYS_RESET,
			PCM9211_SYS_RESET_ADDIS_SHIFT, 1),

	SND_SOC_DAPM_AIF_IN("AUXIN0", NULL, 0, SND_SOC_NOPM, 0, 0),

	/* Power */
	SND_SOC_DAPM_SUPPLY("RXIN0 Coaxial Amplifier", PCM9211_DIR_INP_BIPHASE,
		7, 1, NULL, 0),

	SND_SOC_DAPM_SUPPLY("RXIN1 Coaxial Amplifier", PCM9211_DIR_INP_BIPHASE,
		6, 1, NULL, 0),

	/* Processing */
	SND_SOC_DAPM_AIF_IN("DIR", NULL, 0, PCM9211_SYS_RESET,
			PCM9211_SYS_RESET_RXDIS_SHIFT, 1),
	SND_SOC_DAPM_MIXER("AUTO", SND_SOC_NOPM, 0, 0, NULL, 0),

	/* Internal routing */
	SND_SOC_DAPM_MUX("DIR Input Mux", SND_SOC_NOPM, 0, 0,
			&pcm9211_dir_mux_control),
	SND_SOC_DAPM_MUX("MAIN Output Mux", SND_SOC_NOPM, 0, 0,
			&pcm9211_main_out_control),
	SND_SOC_DAPM_MUX("AUX Output Mux", SND_SOC_NOPM, 0, 0,
			&pcm9211_aux_out_control),

	/* Outputs */
	SND_SOC_DAPM_OUTPUT("MAIN"),
	SND_SOC_DAPM_OUTPUT("AUX"),
};

/* DAPM Routing */
static const struct snd_soc_dapm_route pcm9211_dapm_routes[] = {
	{ "DIR Input Mux", "RXIN0", "RXIN0" },
	{ "DIR Input Mux", "RXIN1", "RXIN1" },
	{ "DIR Input Mux", "RXIN2", "RXIN2" },
	{ "DIR Input Mux", "RXIN3", "RXIN3" },
	{ "DIR Input Mux", "RXIN4", "RXIN4" },
	{ "DIR Input Mux", "RXIN5", "RXIN5" },
	{ "DIR Input Mux", "RXIN6", "RXIN6" },
	{ "DIR Input Mux", "RXIN7", "RXIN7" },

	{ "RXIN0", NULL, "RXIN0 Coaxial Amplifier" },
	{ "RXIN1", NULL, "RXIN1 Coaxial Amplifier" },

	{ "ADC", NULL, "VINL" },
	{ "ADC", NULL, "VINR" },

	{ "DIR", NULL, "DIR Input Mux" },
	{ "AUTO", NULL, "DIR" },
	{ "AUTO", NULL, "ADC" },

	{ "MAIN Output Mux", "AUXIN0", "AUXIN0" },
	{ "MAIN Output Mux", "DIR", "DIR" },
	{ "MAIN Output Mux", "ADC", "ADC" },
	{ "MAIN Output Mux", "AUTO", "AUTO" },

	{ "AUX Output Mux", "DIR", "DIR" },
	{ "AUX Output Mux", "ADC", "ADC" },
	{ "AUX Output Mux", "AUTO", "AUTO" },

	{ "MAIN", NULL, "MAIN Output Mux" },
	{ "AUX", NULL, "AUX Output Mux" },

	{ "MAIN Capture", NULL, "MAIN" },
	{ "AUX Capture", NULL, "AUX" },
};

static struct snd_soc_dai_ops pcm9211_dai_ops = {
	.startup = pcm9211_startup,
	.hw_params = pcm9211_hw_params,
	.set_sysclk = pcm9211_set_dai_sysclk,
	.set_fmt = pcm9211_set_dai_fmt,
};

static int pcm9211_dit_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct pcm9211_priv *priv = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;

	u32 mclk = (params_rate(params) % 8000 == 0) ? PCM9211_DIT_MCLK_48k : PCM9211_DIT_MCLK_44k1;
	u32 div = mclk / params_rate(params);
	u32 txsck = 0;

	dev_info(dev, "DIT mclk: %u, div: %u", mclk, div);

	switch (div) {
		case 128:
			txsck = 0;
			break;
		case 256:
			txsck = 1;
			break;
		case 512:
			txsck = 2;
			break;
		default:
			dev_err(dev, "Unsupported DIT mclk div: %u\n", div);
			return -EINVAL;
	}

	return regmap_update_bits(priv->regmap, PCM9211_DIT_CTRL2, PCM9211_DIT_TXSCK_MASK, txsck << PCM9211_DIT_TXSCK_SHIFT);
}

static struct snd_soc_dai_ops pcm9211_dit_dai_ops = {
	.hw_params = pcm9211_dit_hw_params,
};

/* BCLK is always 64 * FS == 32 bit/channel */
#define PCM9211_FORMATS SNDRV_PCM_FMTBIT_S32_LE

struct snd_soc_dai_driver pcm9211_dai[] = {
	{
		.name = "pcm9211-main-hifi",
		.id = PCM9211_DAI_MAIN,
		.capture = {
			.stream_name = "MAIN Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = PCM9211_FORMATS,
		},
		.ops = &pcm9211_dai_ops,
	},
	{
		.name = "pcm9211-aux-hifi",
		.id = PCM9211_DAI_AUX,
		.capture = {
			.stream_name = "AUX Capture",
			.channels_min = 2,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = PCM9211_FORMATS,
		},
		.ops = &pcm9211_dai_ops,
	},
	{
		.name = "pcm9211-dit-hifi",
		.id = PCM9211_DAI_DIT,
		.playback = {
			.stream_name = "DIT Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = PCM9211_FORMATS,
		},
		.ops = &pcm9211_dit_dai_ops,
	},
};

static const struct snd_soc_component_driver pcm9211_driver = {
	.probe = pcm9211_soc_probe,
	.remove = pcm9211_soc_remove,
	.controls = pcm9211_snd_controls,
	.num_controls = ARRAY_SIZE(pcm9211_snd_controls),
	.dapm_widgets = pcm9211_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(pcm9211_dapm_widgets),
	.dapm_routes = pcm9211_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(pcm9211_dapm_routes),
};

int pcm9211_probe(struct device *dev, struct regmap *regmap)
{
	struct pcm9211_priv *priv;
	int ret;
	int i;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	dev_set_drvdata(dev, priv);
	priv->dev = dev;
	priv->regmap = regmap;

	spin_lock_init(&priv->irq_lock);
	spin_lock_init(&priv->status_lock);

	priv->irq0 = of_irq_get_byname(dev->of_node, "int0");
	priv->irq1 = of_irq_get_byname(dev->of_node, "int1");
	dev_info(dev, "irq0: %d, irq1: %d\n", priv->irq0, priv->irq1);

	priv->xti = devm_clk_get(dev, "xti");
	if (IS_ERR(priv->xti)) {
		ret = PTR_ERR(priv->xti);

		if (ret == -EPROBE_DEFER)
			goto err;

		dev_err(dev, "Failed to get clock 'xti': %d\n", ret);
		priv->xti = NULL;
	} else {
		ret = clk_prepare_enable(priv->xti);
		if (ret) {
			dev_err(dev, "Failed to enable xti clock: %d\n", ret);
			goto err;
		}

		priv->sysclk = clk_get_rate(priv->xti);
		if (priv->sysclk > PCM9211_MAX_SYSCLK) {
			dev_err(dev, "xti clock rate (%lu) exceeds supported max %u\n",
					priv->sysclk, PCM9211_MAX_SYSCLK);
			ret = -EINVAL;
			goto err;
		}
	}

	priv->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->reset)) {
		ret = PTR_ERR(priv->reset);
		dev_err(dev, "Failed to get reset gpio: %d\n", ret);
		priv->reset = NULL;
	}

	ret = devm_snd_soc_register_component(dev, &pcm9211_driver, pcm9211_dai,
			ARRAY_SIZE(pcm9211_dai));
	if (ret) {
		dev_err(dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	return 0;

err:
	if (priv->xti)
		clk_disable_unprepare(priv->xti);

	return ret;

}
EXPORT_SYMBOL_GPL(pcm9211_probe);

void pcm9211_remove(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);

	if (priv->xti)
		clk_disable_unprepare(priv->xti);
}
EXPORT_SYMBOL_GPL(pcm9211_remove);

#ifdef CONFIG_PM
static int pcm9211_runtime_resume(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);
	int ret;

	if (priv->xti) {
		ret = clk_prepare_enable(priv->xti);
		if (ret) {
			dev_err(dev, "Failed to enable xti clock: %d\n", ret);
			return ret;
		}
	}

	ret = pcm9211_reset(dev);
	if (ret) {
		dev_err(dev, "Failed to reset device: %d\n", ret);
		goto err;
	}

	regcache_cache_only(priv->regmap, false);
	regcache_mark_dirty(priv->regmap);

	ret = regcache_sync(priv->regmap);
	if (ret) {
		dev_err(dev, "Failed to sync regmap: %d\n", ret);
		goto err;
	}

	schedule_delayed_work(&priv->rate_detect_work, msecs_to_jiffies(PCM9211_SPDIF_RATE_DETECT_PERIOD_MS));
	enable_irq(priv->irq0);
	enable_irq(priv->irq1);

	dev_info(dev, "rt resume\n");

	return 0;

err:
	if (priv->xti)
		clk_disable_unprepare(priv->xti);

	return ret;
}

static int pcm9211_runtime_suspend(struct device *dev)
{
	struct pcm9211_priv *priv = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&priv->rate_detect_work);
	disable_irq_nosync(priv->irq0);
	disable_irq_nosync(priv->irq1);
	regcache_cache_only(priv->regmap, true);
	if (priv->xti)
		clk_disable_unprepare(priv->xti);

	dev_info(dev, "rt suspend\n");

	return 0;
}
#endif

const struct dev_pm_ops pcm9211_pm_ops = {
	SET_RUNTIME_PM_OPS(pcm9211_runtime_suspend, pcm9211_runtime_resume,
			NULL)
};
EXPORT_SYMBOL_GPL(pcm9211_pm_ops);

MODULE_DESCRIPTION("PCM9211 codec driver");
MODULE_AUTHOR("Julian Scheel <julian@jusst.de>");
MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_LICENSE("GPL v2");
