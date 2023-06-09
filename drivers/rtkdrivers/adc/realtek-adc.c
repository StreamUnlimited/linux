// SPDX-License-Identifier: GPL-2.0
/*
 * This file is part of realtek ADC driver
 *
 * Copyright (C) 2021, Realtek - All Rights Reserved
 */

#include <linux/delay.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/mfd/rtk-timer.h>

#include "realtek-adc.h"

/* adc configuration data */
static const struct realtek_adc_info realtek_adc_cfg = {
	.clk_div = 3,
	.rx_level = 0,
	.chid_en = 0,
	.special_ch = 0xFF,
	.timer_idx = 4,
	.period = 100000000,	//unit: ns
};

/**
  * realtek_adc_cmd - Enable or Disable the ADC peripheral.
  * @param  adc: ADC driver data.
  * @param  state: state of the ADC.
  *   			This parameter can be: true or false.
  * @retval None
  */
void realtek_adc_cmd(struct realtek_adc_data *adc, bool state)
{
	u32 reg_value;

	reg_value = readl(adc->base + RTK_ADC_CONF);
	if (state) {
		reg_value |= ADC_BIT_ENABLE;
	} else {
		reg_value &= ~ADC_BIT_ENABLE;
		// clear fifo
		writel(1, adc->base + RTK_ADC_CLR_FIFO);
		writel(0, adc->base + RTK_ADC_CLR_FIFO);
	}
	writel(reg_value, adc->base + RTK_ADC_CONF);
}

/**
  * realtek_adc_clear_fifo - Clear ADC FIFO.
  * @param  adc: ADC driver data.
  * @retval None
  */
void realtek_adc_clear_fifo(struct realtek_adc_data *adc)
{
	writel(1, adc->base + RTK_ADC_CLR_FIFO);
	writel(0, adc->base + RTK_ADC_CLR_FIFO);
}

/**
  * realtek_adc_int_config - ENABLE/DISABLE  the ADC interrupt bits.
  * @param  adc: ADC driver data.
  * @param  ADC_IT: specifies the ADC interrupt to be setup.
  *          This parameter can be one or combinations of the following values:
  *            @arg ADC_BIT_IT_COMP_CH9_EN:	ADC channel 9 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH8_EN:	ADC channel 8 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH7_EN:	ADC channel 7 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH6_EN:	ADC channel 6 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH5_EN:	ADC channel 5 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH4_EN:	ADC channel 4 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH3_EN:	ADC channel 3 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH2_EN:	ADC channel 2 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH1_EN:	ADC channel 1 compare interrupt
  *            @arg ADC_BIT_IT_COMP_CH0_EN:	ADC channel 0 compare interrupt
  *            @arg ADC_BIT_IT_ERR_EN:			ADC error state interrupt
  *            @arg ADC_BIT_IT_DAT_OVW_EN:	ADC data overwritten interrupt
  *            @arg ADC_BIT_IT_FIFO_EMPTY_EN:	ADC FIFO empty interrupt
  *            @arg ADC_BIT_IT_FIFO_OVER_EN:	ADC FIFO overflow interrupt
  *            @arg ADC_BIT_IT_FIFO_FULL_EN:	ADC FIFO full interrupt
  *            @arg ADC_BIT_IT_CHCV_END_EN:	ADC particular channel conversion done interrupt
  *            @arg ADC_BIT_IT_CV_END_EN:		ADC conversion end interrupt
  *            @arg ADC_BIT_IT_CVLIST_END_EN:	ADC conversion list end interrupt
  * @param  state: true/false.
  * @retval  None
  */
static void realtek_adc_int_config(struct realtek_adc_data *adc, u32 ADC_IT, bool state)
{
	u32 reg_value;

	reg_value = readl(adc->base + RTK_ADC_INTR_CTRL);
	if (state) {
		reg_value |= ADC_IT;
	} else {
		reg_value &= ~ADC_IT;
	}
	writel(reg_value, adc->base + RTK_ADC_INTR_CTRL);
}

/**
  * realtek_adc_readable - Detemine ADC FIFO is empty or not.
  * @param  adc: ADC driver data.
  * @retval ADC FIFO is empty or not:
  *        - 0: Not Empty
  *        - 1: Empty
  */
u32 realtek_adc_readable(struct realtek_adc_data *adc)
{
	u32 Status = readl(adc->base + RTK_ADC_BUSY_STS);
	u32 Readable = (((Status & ADC_BIT_FIFO_EMPTY) == 0) ? 1 : 0);

	return Readable;
}

/**
  * realtek_adc_swtrig_cmd - Control the ADC module to do a conversion. Used as a start-convert event which is controlled by software.
  * @param  adc: ADC driver data.
  * @param  state: can be one of the following value:
  *			@arg true: Enable the analog module and analog mux. And then start a new channel conversion.
  *			@arg false:  Disable the analog module and analog mux.
  * @retval  None.
  * @note  1. Every time this bit is set to 1, ADC module would switch to a new channel and do one conversion.
  *			    Every time a conversion is done, software MUST clear this bit manually.
  *		  2. Used in Sotfware Trigger Mode
  */
static void realtek_adc_swtrig_cmd(struct realtek_adc_data *adc, bool state)
{
	u32 div;
	u8 sync_time[4] = {12, 16, 32, 64};

	div = readl(adc->base + RTK_ADC_CLK_DIV);
	if (state) {
		writel(ADC_BIT_SW_TRIG, adc->base + RTK_ADC_SW_TRIG);
	} else {
		writel(0, adc->base + RTK_ADC_SW_TRIG);
	}

	/* Wait 2 clock to sync signal */
	udelay(sync_time[div]);
}

/**
  * realtek_adc_auto_cmd - Controls the automatic channel switch enabled or disabled.
  * @param  adc: ADC driver data.
  * @param  state: can be one of the following value:
  *		@arg true: Enable the automatic channel switch.
  *			When setting this bit, an automatic channel switch starts from the first channel in the channel switch list.
  *		@arg false:  Disable the automatic channel switch.
  *			If an automatic channel switch is in progess, writing 0 will terminate the automatic channel switch.
  * @retval  None.
  * @note  Used in Automatic Mode
  */
static void realtek_adc_auto_cmd(struct realtek_adc_data *adc, bool state)
{
	u32 div;
	u8 sync_time[4] = {12, 16, 32, 64};

	div = readl(adc->base + RTK_ADC_CLK_DIV);
	if (state) {
		writel(ADC_BIT_AUTO_CSW_EN, adc->base + RTK_ADC_AUTO_CSW_CTRL);
	} else {
		writel(0, adc->base + RTK_ADC_AUTO_CSW_CTRL);
	}

	/* Wait 2 clock to sync signal */
	udelay(sync_time[div]);
}

/**
  * realtek_adc_timtrig_cmd - Initialize the trigger timer when in ADC Timer-Trigger Mode.
  * @param  adc: ADC driver data.
  * @param  Tim_Idx: The timer index would be used to make ADC module do a conversion.
  * @param  PeriodNs: Indicate the period of trigger timer.
  * @param  state: can be one of the following value:
  *			@arg true: Enable the ADC timer trigger mode.
  *			@arg false: Disable the ADC timer trigger mode.
  * @retval  None.
  * @note  Used in Timer-Trigger Mode
  */
static void realtek_adc_timtrig_cmd(struct realtek_adc_data *adc, u8 Tim_Idx, u64 PeriodNs, bool state)
{
	writel(Tim_Idx, adc->base + RTK_ADC_EXT_TRIG_TIMER_SEL);

	if (state) {
		rtk_gtimer_init(Tim_Idx, PeriodNs, NULL, NULL);
		rtk_gtimer_start(Tim_Idx, state);
	} else {
		rtk_gtimer_start(Tim_Idx, state);
		// free timer irq outside IRQ Context
	}
}

/**
 * realtek_adc_single_read() - Performs adc conversion
 * @indio_dev: IIO device
 * @chan: IIO channel
 * @res: conversion result
 */
static int realtek_adc_single_read(struct iio_dev *indio_dev,
								   const struct iio_chan_spec *chan,
								   int *res)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);
	long timeout;
	int ret;
	unsigned long flags;
	u32 reg_value;

	spin_lock_irqsave(&adc->lock, flags);

	realtek_adc_cmd(adc, false);
	//set adc channel list len
	reg_value = readl(adc->base + RTK_ADC_CONF);
	reg_value &= ~(ADC_MASK_CVLIST_LEN);
	writel(reg_value, adc->base + RTK_ADC_CONF);
	// set channel switch list
	writel(chan->channel, adc->base + RTK_ADC_CHSW_LIST_0);
	realtek_adc_cmd(adc, true);

	realtek_adc_clear_fifo(adc);

	if (adc->mode == ADC_SW_TRI_MODE) {
		realtek_adc_swtrig_cmd(adc, true);
		while (realtek_adc_readable(adc) == 0);
		realtek_adc_swtrig_cmd(adc, false);
		*res = ADC_GET_DATA_GLOBAL(readl(adc->base + RTK_ADC_DATA_GLOBAL));
		ret = IIO_VAL_INT;
	} else if (adc->mode == ADC_AUTO_MODE) {
		realtek_adc_auto_cmd(adc, true);
		while (realtek_adc_readable(adc) == 0);
		realtek_adc_auto_cmd(adc, false);
		*res = ADC_GET_DATA_GLOBAL(readl(adc->base + RTK_ADC_DATA_GLOBAL));
		ret = IIO_VAL_INT;
	} else if (adc->mode == ADC_TIM_TRI_MODE) {
		reinit_completion(&adc->completion);
		realtek_adc_int_config(adc, ADC_BIT_IT_CV_END_EN, true);
		realtek_adc_timtrig_cmd(adc, realtek_adc_cfg.timer_idx, realtek_adc_cfg.period, true);
		timeout = wait_for_completion_interruptible_timeout(&adc->completion, msecs_to_jiffies(100));
		if (timeout == 0) {
			ret = -ETIMEDOUT;
		} else if (timeout < 0) {
			ret = timeout;
		} else {
			*res = (int) adc->buffer[0];
			ret = IIO_VAL_INT;
		}
		realtek_adc_int_config(adc, (ADC_BIT_IT_CV_END_EN), false);
		rtk_gtimer_deinit(realtek_adc_cfg.timer_idx);
	}

	spin_unlock_irqrestore(&adc->lock, flags);
	return ret;
}

static int realtek_adc_read_raw(struct iio_dev *indio_dev,
								struct iio_chan_spec const *chan,
								int *val, int *val2, long mask)
{
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = iio_device_claim_direct_mode(indio_dev);
		if (ret) {
			return ret;
		}
		if (chan->type == IIO_VOLTAGE) {
			ret = realtek_adc_single_read(indio_dev, chan, val);
		} else {
			ret = -EINVAL;
		}
		iio_device_release_direct_mode(indio_dev);
		return ret;

	case IIO_CHAN_INFO_SCALE:
		if (chan->differential) {
			*val = 3300 * 2;
			*val2 = chan->scan_type.realbits;
		} else {
			if (chan->channel == 6) {
				*val = 5000;
			} else {
				*val = 3300;
			}
			*val2 = chan->scan_type.realbits;
		}
		return IIO_VAL_FRACTIONAL_LOG2;

	case IIO_CHAN_INFO_OFFSET:
		if (chan->differential)
			/* ADC_full_scale / 2 */
		{
			*val = -((1 << chan->scan_type.realbits) / 2);
		} else {
			*val = 0;
		}
		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static int realtek_adc_update_scan_mode(struct iio_dev *indio_dev,
										const unsigned long *scan_mask)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);
	unsigned long flags;
	const struct iio_chan_spec *chan;
	u32 reg_value, bit = 0;
	u32 i = 0;

	spin_lock_irqsave(&adc->lock, flags);

	adc->num_conv = bitmap_weight(scan_mask, indio_dev->masklength);

	realtek_adc_cmd(adc, false);

	for_each_set_bit(bit, scan_mask, indio_dev->masklength) {
		chan = indio_dev->channels + bit;
		i++;
		if (i > RTK_ADC_CH_NUM) {
			spin_unlock_irqrestore(&adc->lock, flags);
			return -EINVAL;
		}

		dev_info(&indio_dev->dev, "%s chan %d to seq%d\n", __func__, chan->channel, i);

		if (i > 8) {
			reg_value = readl(adc->base + RTK_ADC_CHSW_LIST_1);
			reg_value &= ~(ADC_MASK_CHSW << ADC_SHIFT_CHSW1((i - 1)));
			reg_value |= (u32)(chan->channel << ADC_SHIFT_CHSW1((i - 1)));
			writel(reg_value, adc->base + RTK_ADC_CHSW_LIST_1);
		} else {
			reg_value = readl(adc->base + RTK_ADC_CHSW_LIST_0);
			reg_value &= ~(ADC_MASK_CHSW << ADC_SHIFT_CHSW0((i - 1)));
			reg_value |= (u32)(chan->channel << ADC_SHIFT_CHSW0((i - 1)));
			writel(reg_value, adc->base + RTK_ADC_CHSW_LIST_0);
		}
	}

	if (!i) {
		spin_unlock_irqrestore(&adc->lock, flags);
		return -EINVAL;
	}

	//set adc mode and channel list len
	reg_value = readl(adc->base + RTK_ADC_CONF);
	reg_value &= ~(ADC_MASK_OP_MOD | ADC_MASK_CVLIST_LEN);
	reg_value |= ADC_OP_MOD(ADC_AUTO_MODE) | ADC_CVLIST_LEN((i - 1));
	writel(reg_value, adc->base + RTK_ADC_CONF);
	realtek_adc_cmd(adc, true);

	spin_unlock_irqrestore(&adc->lock, flags);

	return 0;
}

/**
 * realtek_adc_debugfs_reg_access - read or write register value
 *
 * To read a value from an ADC register:
 *   echo [ADC reg offset] > direct_reg_access
 *   cat direct_reg_access
 *
 * To write a value in a ADC register:
 *   echo [ADC_reg_offset] [value] > direct_reg_access
 */
static int realtek_adc_debugfs_reg_access(struct iio_dev *indio_dev,
		unsigned reg, unsigned writeval,
		unsigned *readval)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);

	if (!readval) {
		writel(writeval, adc->base + reg);
	} else {
		*readval = readl(adc->base + reg);
	}
	return 0;
}

static int realtek_adc_of_xlate(struct iio_dev *indio_dev,
								const struct of_phandle_args *iiospec)
{
	int i;

	for (i = 0; i < indio_dev->num_channels; i++)
		if (indio_dev->channels[i].channel == iiospec->args[0]) {
			return i;
		}

	return -EINVAL;
}

static const struct iio_info realtek_adc_iio_info = {
	.read_raw = realtek_adc_read_raw,
	.update_scan_mode = realtek_adc_update_scan_mode,
	.debugfs_reg_access = realtek_adc_debugfs_reg_access,
	.of_xlate = realtek_adc_of_xlate,
};

static irqreturn_t realtek_adc_isr(int irq, void *data)
{
	struct realtek_adc_data *adc = data;
	struct iio_dev *indio_dev = iio_priv_to_dev(adc);
	u32 status = readl(adc->base + RTK_ADC_INTR_STS);
	u32 cnt;

	if (status & ADC_BIT_CV_END_STS) {
		adc->buffer[0] = ADC_GET_DATA_GLOBAL(readl(adc->base + RTK_ADC_DATA_GLOBAL));
		realtek_adc_timtrig_cmd(adc, realtek_adc_cfg.timer_idx, 0, false);
		complete(&adc->completion);
		writel(status, adc->base + RTK_ADC_INTR_STS);
		return IRQ_HANDLED;
	} else if (status & ADC_BIT_FIFO_FULL_STS) {
		if (iio_buffer_enabled(indio_dev)) {
			cnt = ADC_GET_FLR(readl(adc->base + RTK_ADC_FLR));
			while (cnt--) {
				adc->buffer[adc->buf_index] = ADC_GET_DATA_GLOBAL(readl(adc->base + RTK_ADC_DATA_GLOBAL));
				adc->buf_index++;
				if (RTK_ADC_BUF_SIZE <= adc->buf_index) {
					break;
				}
			}

			if (adc->buf_index >= RTK_ADC_BUF_SIZE) {
				realtek_adc_int_config(adc, (ADC_BIT_IT_FIFO_OVER_EN | ADC_BIT_IT_FIFO_FULL_EN), false);
				iio_trigger_poll(indio_dev->trig);
			}
		}

		return IRQ_HANDLED;
	} else if (status & ADC_BIT_FIFO_OVER_STS) {
		dev_dbg(&indio_dev->dev, "fifo overflow\n");
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static void realtek_adc_hw_init(struct iio_dev *indio_dev)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);
	u32 reg_value, i;

	realtek_adc_cmd(adc, false);
	// disable interrupt, clear all interrupts
	writel(0, adc->base + RTK_ADC_INTR_CTRL);
	writel(0x3FFFF, adc->base + RTK_ADC_INTR_STS);
	// set clock divider
	writel(ADC_CLK_DIV(realtek_adc_cfg.clk_div), adc->base + RTK_ADC_CLK_DIV);
	//set adc mode and channel list len
	reg_value = readl(adc->base + RTK_ADC_CONF);
	reg_value &= ~ADC_MASK_OP_MOD;
	reg_value |= ADC_OP_MOD(adc->mode);
	writel(reg_value, adc->base + RTK_ADC_CONF);
	// set adc input type
	reg_value = 0;
	for (i = 0; i < indio_dev->num_channels; i++) {
		if (indio_dev->channels[i].differential == 1) {
			reg_value |= ADC_DIFFERENTIAL_CH(indio_dev->channels[i].channel);
		}
	}
	writel(reg_value, adc->base + RTK_ADC_IN_TYPE);
	// set particular channel
	if (realtek_adc_cfg.special_ch < RTK_ADC_CH_NUM) {
		writel(ADC_BIT_IT_CHCV_END_EN, adc->base + RTK_ADC_INTR_CTRL);
		writel(realtek_adc_cfg.special_ch, adc->base + RTK_ADC_IT_CHNO_CON);
	}
	// set FIFO full level
	writel(realtek_adc_cfg.rx_level, adc->base + RTK_ADC_FULL_LVL);
	// set channel ID included in data or not
	if (realtek_adc_cfg.chid_en) {
		reg_value = readl(adc->base + RTK_ADC_DELAY_CNT);
		reg_value |= ADC_BIT_CH_UNMASK;
		writel(reg_value, adc->base + RTK_ADC_DELAY_CNT);
	}
	realtek_adc_cmd(adc, true);
}

static void realtek_adc_hw_deinit(struct iio_dev *indio_dev)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);

	writel(0, adc->base + RTK_ADC_INTR_CTRL);
	writel(0x3FFFF, adc->base + RTK_ADC_INTR_STS);
	realtek_adc_cmd(adc, false);
}

static void realtek_adc_channel_cfg(struct iio_dev *indio_dev,
									struct iio_chan_spec *chan, u32 vinp,
									u32 vinn, int scan_index, bool differential)
{
	char name[10];

	chan->type = IIO_VOLTAGE;
	chan->channel = vinp;
	if (differential) {
		chan->differential = 1;
		chan->channel2 = vinn;
		snprintf(name, 10, "in%d-in%d", vinp, vinn);
	} else {
		snprintf(name, 10, "in%d", vinp);
	}
	chan->datasheet_name = name;
	chan->scan_index = scan_index;
	chan->indexed = 1;
	chan->info_mask_separate = BIT(IIO_CHAN_INFO_RAW);
	chan->info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET);
	chan->scan_type.sign = 'u';
	chan->scan_type.realbits = 12;
	chan->scan_type.storagebits = 16;
}

static int realtek_adc_init_channel(struct iio_dev *indio_dev)
{
	struct device_node *node = indio_dev->dev.of_node;
	struct realtek_adc_diff_channel diff[RTK_ADC_CH_NUM];
	struct property *prop = NULL;
	const __be32 *cur = NULL;
	struct iio_chan_spec *channels;
	int scan_index = 0, ret, i;
	u32 val = 0, num_channels = 0, num_diff = 0;

	ret = of_property_count_u32_elems(node, "rtk,adc-channels");
	if (ret > RTK_ADC_CH_NUM) {
		dev_err(&indio_dev->dev, "Bad adc-channels?\n");
		return -EINVAL;
	} else if (ret > 0) {
		num_channels += ret;
	}

	ret = of_property_count_elems_of_size(node, "rtk,adc-diff-channels",
										  sizeof(*diff));
	if (ret > RTK_ADC_CH_NUM) {
		dev_err(&indio_dev->dev, "Bad adc-diff-channels?\n");
		return -EINVAL;
	} else if (ret > 0) {
		int size = ret * sizeof(*diff) / sizeof(u32);

		num_diff = ret;
		num_channels += ret;
		ret = of_property_read_u32_array(node, "rtk,adc-diff-channels",
										 (u32 *)diff, size);
		if (ret) {
			return ret;
		}
	}

	if (!num_channels) {
		dev_err(&indio_dev->dev, "No channels configured\n");
		return -ENODATA;
	}

	channels = devm_kcalloc(&indio_dev->dev, num_channels,
							sizeof(struct iio_chan_spec), GFP_KERNEL);
	if (!channels) {
		return -ENOMEM;
	}

	of_property_for_each_u32(node, "rtk,adc-channels", prop, cur, val) {
		if (val >= RTK_ADC_CH_NUM) {
			dev_err(&indio_dev->dev, "Invalid channel %d\n", val);
			return -EINVAL;
		}

		/* Channel can't be configured both as single-ended & diff */
		for (i = 0; i < num_diff; i++) {
			if (val == diff[i].vinp) {
				dev_err(&indio_dev->dev,
						"channel %d miss-configured\n",	val);
				return -EINVAL;
			}
		}
		realtek_adc_channel_cfg(indio_dev, &channels[scan_index], val, 0, scan_index, false);
		scan_index++;
	}

	for (i = 0; i < num_diff; i++) {
		if (diff[i].vinp >= RTK_ADC_CH_NUM || diff[i].vinn >= RTK_ADC_CH_NUM) {
			dev_err(&indio_dev->dev, "Invalid channel in%d-in%d\n",
					diff[i].vinp, diff[i].vinn);
			return -EINVAL;
		}
		realtek_adc_channel_cfg(indio_dev, &channels[scan_index], diff[i].vinp, diff[i].vinn, scan_index, true);
		scan_index++;
	}

	indio_dev->num_channels = scan_index;
	indio_dev->channels = channels;

	return 0;
}

static int realtek_adc_buffer_postenable(struct iio_dev *indio_dev)
{
	struct realtek_adc_data *adc = iio_priv(indio_dev);
	int ret;
	unsigned long flags;

	ret = iio_triggered_buffer_postenable(indio_dev);
	if (ret < 0) {
		return ret;
	}

	spin_lock_irqsave(&adc->lock, flags);
	adc->buf_index = 0;
	realtek_adc_clear_fifo(adc);
	writel(1, adc->base + RTK_ADC_RST_LIST);
	writel(0, adc->base + RTK_ADC_RST_LIST);
	writel(0x3FFFF, adc->base + RTK_ADC_INTR_STS);
	realtek_adc_int_config(adc, (ADC_BIT_IT_FIFO_OVER_EN | ADC_BIT_IT_FIFO_FULL_EN), true);
	realtek_adc_auto_cmd(adc, true);
	spin_unlock_irqrestore(&adc->lock, flags);

	return 0;
}

static int realtek_adc_buffer_predisable(struct iio_dev *indio_dev)
{
	int ret;
	struct realtek_adc_data *adc = iio_priv(indio_dev);
	u32 i;
	unsigned long flags;

	ret = iio_triggered_buffer_predisable(indio_dev);
	if (ret < 0) {
		dev_err(&indio_dev->dev, "predisable failed\n");
	}

	spin_lock_irqsave(&adc->lock, flags);
	realtek_adc_auto_cmd(adc, false);

	for (i = 0; i < RTK_ADC_BUF_SIZE; i++) {
		dev_dbg(&indio_dev->dev, "data: %x\n", adc->buffer[i]);
	}

	spin_unlock_irqrestore(&adc->lock, flags);
	return ret;
}

static const struct iio_buffer_setup_ops realtek_adc_buffer_setup_ops = {
	.postenable = &realtek_adc_buffer_postenable,
	.predisable = &realtek_adc_buffer_predisable,
};

static irqreturn_t realtek_adc_trigger_handler(int irq, void *private)
{
	struct iio_poll_func *pf = private;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct realtek_adc_data *adc = iio_priv(indio_dev);

	dev_dbg(&indio_dev->dev, "%s(irq=%d)\n", __func__, irq);

	adc->buf_index = 0;

	iio_push_to_buffers_with_timestamp(indio_dev, adc->buffer, pf->timestamp);

	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

static int realtek_adc_probe(struct platform_device *pdev)
{
	struct iio_dev *indio_dev;
	struct realtek_adc_data *adc;
	int ret;
	struct resource *res;

	if (!pdev->dev.of_node) {
		return -ENODEV;
	}

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*adc));
	if (!indio_dev) {
		return -ENOMEM;
	}

	adc = iio_priv(indio_dev);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	adc->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(adc->base)) {
		return PTR_ERR(adc->base);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	adc->comp_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(adc->base)) {
		return PTR_ERR(adc->base);
	}

	spin_lock_init(&adc->lock);
	init_completion(&adc->completion);
	indio_dev->name = dev_name(&pdev->dev);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->dev.of_node = pdev->dev.of_node;
	indio_dev->info = &realtek_adc_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;

	platform_set_drvdata(pdev, adc);

	if (of_property_read_u32(pdev->dev.of_node, "rtk,adc-mode", &adc->mode)) {
		dev_err(&pdev->dev, "adc mode property not found\n");
		return -EINVAL;
	}

	adc->adc_clk = devm_clk_get(&pdev->dev, "rtk_adc_clk");
	if (IS_ERR(adc->adc_clk)) {
		dev_err(&pdev->dev, "Fail to get adc clock\n");
		return PTR_ERR(adc->adc_clk);
	}

	adc->ctc_clk = devm_clk_get(&pdev->dev, "rtk_ctc_clk");
	if (IS_ERR(adc->ctc_clk)) {
		dev_err(&pdev->dev, "Fail to get ctc clock\n");
		return PTR_ERR(adc->ctc_clk);
	}

	ret = clk_prepare_enable(adc->adc_clk);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to enable adc clock %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(adc->ctc_clk);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to enable ctc clock %d\n", ret);
		goto clk_fail;
	}

	if (adc->mode != ADC_COMP_ASSIST_MODE) {
		adc->irq = platform_get_irq(pdev, 0);
		if (adc->irq < 0) {
			return adc->irq;
		}

		ret = devm_request_irq(&pdev->dev, adc->irq, realtek_adc_isr, 0, pdev->name, adc);
		if (ret) {
			dev_err(&pdev->dev, "failed to request IRQ\n");
			return ret;
		}

		ret = realtek_adc_init_channel(indio_dev);
		if (ret < 0) {
			return ret;
		}

		ret = iio_triggered_buffer_setup(indio_dev, &iio_pollfunc_store_time, &realtek_adc_trigger_handler,
										 &realtek_adc_buffer_setup_ops);
		if (ret) {
			dev_err(&pdev->dev, "buffer setup failed\n");
			return ret;
		}

		realtek_adc_hw_init(indio_dev);
	}
	ret = iio_device_register(indio_dev);
	if (ret) {
		dev_err(&pdev->dev, "iio dev register failed\n");
		goto err_fail;
	}

	if (adc->mode == ADC_COMP_ASSIST_MODE) {
		ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to populate DT children\n");
			goto err_dev_register;
		}
	}

	return 0;

err_dev_register:
	iio_device_unregister(indio_dev);
err_fail:
	realtek_adc_hw_deinit(indio_dev);
	clk_disable_unprepare(adc->ctc_clk);
clk_fail:
	clk_disable_unprepare(adc->adc_clk);
	return ret;
}

static int realtek_adc_remove(struct platform_device *pdev)
{
	struct realtek_adc_data *adc = platform_get_drvdata(pdev);
	struct iio_dev *indio_dev = iio_priv_to_dev(adc);

	clk_disable_unprepare(adc->adc_clk);
	clk_disable_unprepare(adc->ctc_clk);
	iio_device_unregister(indio_dev);
	realtek_adc_hw_deinit(indio_dev);

	return 0;
}

static const struct of_device_id realtek_adc_match[] = {
	{.compatible = "realtek,amebad2-adc",},
};
MODULE_DEVICE_TABLE(of, realtek_adc_match);

static struct platform_driver realtek_adc_driver = {
	.probe	= realtek_adc_probe,
	.remove	= realtek_adc_remove,
	.driver = {
		.name = "realtek-amebad2-adc",
		.of_match_table = of_match_ptr(realtek_adc_match),
	},
};

builtin_platform_driver(realtek_adc_driver);

MODULE_DESCRIPTION("AmebaD2 realtek_adc_data driver");
MODULE_LICENSE("GPL");
