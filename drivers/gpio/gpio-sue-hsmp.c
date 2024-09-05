#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

#include <misc/sue-hsmp-common.h>

struct sue_hsmp_gpio {
	struct gpio_chip gc;
	struct regmap *hsmp_regmap;
};

static int sue_hsmp_gpio_get_direction(struct gpio_chip *gc, unsigned offset)
{
	return GPIOF_DIR_OUT;
}

static int sue_hsmp_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct sue_hsmp_gpio *gpio = gpiochip_get_data(gc);
	uint32_t data;
	uint32_t mask = BIT(offset);
	regmap_read(gpio->hsmp_regmap, HSMP_GPIO_VAL_REG, &data);
	return (data & mask) != 0;
}

static void sue_hsmp_gpio_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct sue_hsmp_gpio *gpio = gpiochip_get_data(gc);
	uint32_t data = value ? BIT(offset) : 0;
	uint32_t mask = BIT(offset);
	/* Keep in mind that M0 might decide to override your request */
	regmap_update_bits(gpio->hsmp_regmap, HSMP_GPIO_VAL_REG, mask, data);
}

static int sue_hsmp_gpio_probe(struct platform_device *pdev)
{
	struct sue_hsmp_gpio *gpio;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	platform_set_drvdata(pdev, gpio);

	gpio->hsmp_regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!gpio->hsmp_regmap) {
		dev_err(&pdev->dev, "Failed to get regmap from parent\n");
		return -ENODEV;
	}

	gpio->gc.label = dev_name(&pdev->dev);
	gpio->gc.parent = &pdev->dev;
	gpio->gc.owner = THIS_MODULE;
	gpio->gc.base = -1;
	gpio->gc.ngpio = 32;
	gpio->gc.get_direction = sue_hsmp_gpio_get_direction;
	gpio->gc.set = sue_hsmp_gpio_set;
	gpio->gc.get = sue_hsmp_gpio_get;

	return devm_gpiochip_add_data(&pdev->dev, &gpio->gc, gpio);
}

static const struct of_device_id sue_hsmp_gpio_of_match[] = {
	{ .compatible = "sue,hsmp-gpio", },
	{ }
};
MODULE_DEVICE_TABLE(of, sue_hsmp_gpio_of_match);

static struct platform_driver sue_hsmp_gpio_driver = {
	.probe  = sue_hsmp_gpio_probe,
	.driver = {
		.name = "sue-hsmp-gpio",
		.of_match_table = sue_hsmp_gpio_of_match,
	},
};

module_platform_driver(sue_hsmp_gpio_driver);

MODULE_AUTHOR("Peter Suti <peter.suti@streamunlimited.com>");
MODULE_DESCRIPTION("GPIO driver using the SUE HSMP protocol");
MODULE_LICENSE("GPL v2");
