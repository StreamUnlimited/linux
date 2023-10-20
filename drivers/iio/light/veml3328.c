#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>

#define VEML3328_DRIVER_NAME	"veml3328"

#define VEML3328_REG_CTRL	0x00
#define VEML3328_REG_CLEAR	0x04
#define VEML3328_REG_RED	0x05
#define VEML3328_REG_GREEN	0x06
#define VEML3328_REG_BLUE	0x07
#define VEML3328_REG_IR		0x08
#define VEML3328_REG_ID		0x0C


struct veml3328_data {
	struct i2c_client *client;
};

#define VEML3328_CHANNEL(__color, __addr) { \
		.type = IIO_INTENSITY, \
		.modified = 1, \
		.channel2 = IIO_MOD_LIGHT_##__color, \
		.address = __addr, \
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW), \
		.scan_type = { \
			.sign = 'u', \
			.realbits = 16, \
			.storagebits = 16, \
			.endianness = IIO_BE, \
		}, \
	}

static const struct iio_chan_spec veml3328_channels[] = {
	VEML3328_CHANNEL(CLEAR, VEML3328_REG_CLEAR),
	VEML3328_CHANNEL(RED, VEML3328_REG_RED),
	VEML3328_CHANNEL(GREEN, VEML3328_REG_GREEN),
	VEML3328_CHANNEL(BLUE, VEML3328_REG_BLUE),
	VEML3328_CHANNEL(IR, VEML3328_REG_IR),
};

static int veml3328_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
	int ret;
	struct veml3328_data *data = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = i2c_smbus_read_word_data(data->client, chan->address);
		if (ret < 0)
			return ret;
		*val = ret;
		return IIO_VAL_INT;
	}
	return -EINVAL;
}

static const struct iio_info veml3328_info = {
	.read_raw = veml3328_read_raw,
};

static int veml3328_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct veml3328_data *data;
	struct iio_dev *indio_dev;
	int ret;

	ret = i2c_smbus_read_byte_data(client, VEML3328_REG_ID);
	if (ret < 0)
		return ret;

	if (ret == 0x28)
		dev_info(&client->dev, "VEML3328 found\n");
	else
		return -ENODEV;

	// Write 0x000 to power on the VEML3328 by setting SD1 and SD0 to 0
	ret = i2c_smbus_write_word_data(client, VEML3328_REG_CTRL, 0x0000);
	if (ret < 0)
		return ret;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (indio_dev == NULL)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &veml3328_info;
	indio_dev->name = VEML3328_DRIVER_NAME;
	indio_dev->channels = veml3328_channels;
	indio_dev->num_channels = ARRAY_SIZE(veml3328_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	ret = devm_iio_device_register(&client->dev, indio_dev);
	if (ret)
		dev_err(&client->dev, "registering iio device failed\n");

	return 0;
}

static const struct of_device_id veml3328_of_match[] = {
	{ .compatible = "vishay,veml3328" },
	{ }
};
MODULE_DEVICE_TABLE(of, veml3328_of_match);

static const struct i2c_device_id veml3328_ids[] = {
	{ "veml3328", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, veml3328_ids);

static struct i2c_driver veml3328_driver = {
	.driver = {
		.name		= VEML3328_DRIVER_NAME,
		.of_match_table	= veml3328_of_match,
	},
	.probe		= veml3328_probe,
	.id_table	= veml3328_ids,
};

module_i2c_driver(veml3328_driver);

MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_DESCRIPTION("Vishay VEML3328 RGBCIR Color Sensor");
MODULE_LICENSE("GPL");
