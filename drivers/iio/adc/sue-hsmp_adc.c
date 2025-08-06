#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/stringify.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

#include <misc/sue-hsmp-common.h>

struct sue_hsmp_adc {
	struct regmap *hsmp_regmap;
	struct iio_dev *indio_dev;
};

static int sue_hsmp_adc_read_raw(struct iio_dev *indio_dev,
							struct iio_chan_spec const *chan,
							int *val, int *val2,
							long mask)
{
	struct sue_hsmp_adc *adc = iio_priv(indio_dev);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		uint32_t data = STATE_ERROR;
		int ret;

		ret = regmap_write(adc->hsmp_regmap, HSMP_ADC_CHANNEL_SELECT_REG, chan->channel);
		if (ret < 0) {
			dev_err(&indio_dev->dev, "Failed to select ADC channel: %d!\n", ret);
			return ret;
		}

		ret = regmap_read(adc->hsmp_regmap, HSMP_ADC_VALUE_REG, &data);
		if (ret < 0) {
			dev_err(&indio_dev->dev, "Failed to read ADC value: %d!\n", ret);
			return ret;
		}

		if (data == STATE_ERROR) {
			dev_err(&indio_dev->dev, "Failed to read ADC value: Timeout!\n");
			return -EIO;
		}

		*val = data;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		// return scale of 1, because raw value is already the correct one, no need to scale it.
		*val = 1;
		*val2 = 0;
		return IIO_VAL_INT_PLUS_MICRO;
	case IIO_CHAN_INFO_OFFSET:
		*val = 0;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

#define SUE_HSMP_CHANNEL(chan_idx) { \
	.type = IIO_VOLTAGE, \
	.indexed = 1, \
	.channel = (chan_idx), \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | \
							BIT(IIO_CHAN_INFO_SCALE) | \
							BIT(IIO_CHAN_INFO_OFFSET), \
	.datasheet_name = "ch" __stringify(chan_idx), \
	.scan_type = { \
		.sign = 'u', \
		.realbits = 12, \
	}, \
}

static const struct iio_chan_spec sue_hsmp_adc_channels[] = {
	SUE_HSMP_CHANNEL(0),
	SUE_HSMP_CHANNEL(1),
	SUE_HSMP_CHANNEL(2),
	SUE_HSMP_CHANNEL(3),
	SUE_HSMP_CHANNEL(4),
	SUE_HSMP_CHANNEL(5),
	SUE_HSMP_CHANNEL(6),
};

static const struct iio_info sue_hsmp_adc_iio_info = {
	.read_raw = &sue_hsmp_adc_read_raw,
};

static int sue_hsmp_adc_probe(struct platform_device *pdev)
{
	struct sue_hsmp_adc *adc;
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*adc));
	if (!indio_dev) {
		return -ENOMEM;
	}

	adc = iio_priv(indio_dev);

	adc->hsmp_regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!adc->hsmp_regmap) {
		dev_err(&pdev->dev, "Failed to get regmap from parent\n");
		return -ENODEV;
	}

	adc->indio_dev = indio_dev;

	indio_dev->name = dev_name(&pdev->dev);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->dev.of_node = pdev->dev.of_node;
	indio_dev->info = &sue_hsmp_adc_iio_info;
	indio_dev->channels = sue_hsmp_adc_channels;
	indio_dev->num_channels = ARRAY_SIZE(sue_hsmp_adc_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	platform_set_drvdata(pdev, adc);

	return iio_device_register(indio_dev);
}

static const struct of_device_id sue_hsmp_adc_of_match[] = {
	{ .compatible = "sue,hsmp-adc", },
	{ }
};
MODULE_DEVICE_TABLE(of, sue_hsmp_adc_of_match);

static struct platform_driver sue_hsmp_adc_driver = {
	.probe  = sue_hsmp_adc_probe,
	.driver = {
		.name = "sue-hsmp-adc",
		.of_match_table = sue_hsmp_adc_of_match,
	},
};

module_platform_driver(sue_hsmp_adc_driver);

MODULE_AUTHOR("Peter Suti <peter.suti@streamunlimited.com>");
MODULE_DESCRIPTION("ADC driver using the SUE HSMP protocol");
MODULE_LICENSE("GPL v2");
