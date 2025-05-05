// SPDX-License-Identifier: GPL-2.0-only
/*
* SUE HSMP State controller
*
* Copyright (C) 2024, StreamUnlimited GmbH. All rights reserved.
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/leds.h>
#include <linux/suspend.h>

#include <misc/sue-hsmp-common.h>

struct rgb_led_data {
	struct led_classdev red_led;
	struct led_classdev green_led;
	struct led_classdev blue_led;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct state_controller_data {
	struct regmap *hsmp_regmap;
	struct rgb_led_data rgb_led_data;
#ifdef CONFIG_PM
	enum hsmp_power_state prev_power_state;
#endif
};

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct state_controller_data *data = dev_get_drvdata(dev);
	uint32_t value;
	int i, ret;

	ret = regmap_read(data->hsmp_regmap, HSMP_POWER_STATE_REG, &value);
	if (ret) {
		dev_err(dev, "Failed to read state\n");
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(hsmp_power_state_names); ++i) {
		if (hsmp_power_state_names[i].value == value)
			return sprintf(buf, "%s\n", hsmp_power_state_names[i].name);
	}

	dev_err(dev, "Got unknown HSMP state\n");
	return -EINVAL;
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct state_controller_data *data = dev_get_drvdata(dev);
	uint32_t new_state;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(hsmp_power_state_names); ++i) {
		if (strncmp(buf, hsmp_power_state_names[i].name,
			    strlen(hsmp_power_state_names[i].name)) == 0) {
			new_state = hsmp_power_state_names[i].value;
			ret = regmap_write(data->hsmp_regmap, HSMP_POWER_STATE_REG, new_state);
			if (ret) {
				dev_err(dev, "Failed to set state\n");
				return ret;
			}
			return count;
		}
	}
	return -EINVAL;
}

static DEVICE_ATTR_RW(state);

static ssize_t available_states_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	ssize_t count = 0;
	int i;
	for (i = 0; i < ARRAY_SIZE(hsmp_power_state_names); ++i) {
		count += sprintf(buf + count, "%s\n",
				 hsmp_power_state_names[i].name);
	}
	return count;
}

static DEVICE_ATTR_RO(available_states);

static struct attribute *state_controller_attrs[] = {
	&dev_attr_state.attr,
	&dev_attr_available_states.attr,
	NULL,
};

static const struct attribute_group state_controller_attr_group = {
	.attrs = state_controller_attrs,
};

static void set_rgb_data(struct regmap *hsmp_regmap, uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t brightness = (red << 24) | (green << 16) | (blue << 8);
	regmap_write(hsmp_regmap, HSMP_RGB_LED_REG, brightness);
}

static void set_brightness_rgb(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	struct state_controller_data *data = dev_get_drvdata(led_cdev->dev->parent);
	struct rgb_led_data *led_data = &data->rgb_led_data;

	if (led_cdev == &led_data->red_led) {
		led_data->red = brightness;
	} else if (led_cdev == &led_data->green_led) {
		led_data->green = brightness;
	} else if (led_cdev == &led_data->blue_led) {
		led_data->blue = brightness;
	}

	set_rgb_data(data->hsmp_regmap, led_data->red, led_data->green, led_data->blue);
}

#ifdef CONFIG_PM
static int sue_hsmp_state_controller_suspend(struct device *dev)
{
	struct state_controller_data *data = dev_get_drvdata(dev);
	uint32_t value;
	int ret;

	if (pm_suspend_target_state == PM_SUSPEND_CG || pm_suspend_target_state == PM_SUSPEND_PG) {
		ret = regmap_read(data->hsmp_regmap, HSMP_POWER_STATE_REG, &value);
		if (ret) {
			dev_err(dev, "Failed to save state\n");
			return ret;
		}
		ret = regmap_write(data->hsmp_regmap, HSMP_POWER_STATE_REG, STATE_SUSPEND_TO_RAM);
		if (ret) {
			dev_err(dev, "Failed to set state to STATE_SUSPEND_TO_RAM\n");
			return ret;
		}
		data->prev_power_state = value;
	}
	return 0;
}

static int sue_hsmp_state_controller_resume(struct device *dev)
{
	struct state_controller_data *data = dev_get_drvdata(dev);
	int ret;

	if (pm_suspend_target_state == PM_SUSPEND_CG || pm_suspend_target_state == PM_SUSPEND_PG) {
		ret = regmap_write(data->hsmp_regmap, HSMP_POWER_STATE_REG, data->prev_power_state);
		if (ret) {
			dev_err(dev, "Failed to restore state after resume\n");
			return ret;
		}
	}

	return 0;
}

static const struct dev_pm_ops sue_hsmp_state_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sue_hsmp_state_controller_suspend, sue_hsmp_state_controller_resume)
};
#endif

static int sue_hsmp_state_controller_probe(struct platform_device *pdev)
{
	struct state_controller_data *data;
	int ret = 0;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->hsmp_regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!data->hsmp_regmap) {
		dev_err(&pdev->dev, "Failed to get regmap from parent\n");
		return -EPROBE_DEFER;
	}

	ret = regmap_write(data->hsmp_regmap, HSMP_POWER_STATE_REG, STATE_KERNEL);
	if (ret) {
		dev_err(&pdev->dev, "Failed to set hsmp state\n");
		return ret;
	}

	data->rgb_led_data.red_led.name = "state_rgb_led:red";
	data->rgb_led_data.red_led.brightness_set = set_brightness_rgb;

	data->rgb_led_data.green_led.name = "state_rgb_led:green";
	data->rgb_led_data.green_led.brightness_set = set_brightness_rgb;

	data->rgb_led_data.blue_led.name = "state_rgb_led:blue";
	data->rgb_led_data.blue_led.brightness_set = set_brightness_rgb;

	dev_set_drvdata(&pdev->dev, data);

	ret = devm_led_classdev_register(&pdev->dev, &data->rgb_led_data.red_led);
	if (ret)
		return ret;

	ret = devm_led_classdev_register(&pdev->dev, &data->rgb_led_data.green_led);
	if (ret)
		return ret;

	ret = devm_led_classdev_register(&pdev->dev, &data->rgb_led_data.blue_led);
	if (ret)
		return ret;

	ret = sysfs_create_group(&pdev->dev.kobj, &state_controller_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "Failed to create sysfs group\n");
		return ret;
	}

	return 0;
}

static int sue_hsmp_state_controller_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &state_controller_attr_group);

	return 0;
}

static const struct of_device_id sue_hsmp_state_controller_of_match[] = {
	{ .compatible = "sue,hsmp-state-controller", },
	{},
};

MODULE_DEVICE_TABLE(of, sue_hsmp_state_controller_of_match);

static struct platform_driver sue_hsmp_state_controller = {
	.driver = {
		.name = "sue-hsmp-state-controller",
#ifdef CONFIG_PM
		.pm = &sue_hsmp_state_pm_ops,
#endif
		.of_match_table = sue_hsmp_state_controller_of_match,
	},
	.probe = sue_hsmp_state_controller_probe,
	.remove = sue_hsmp_state_controller_remove,
};

module_platform_driver(sue_hsmp_state_controller);

MODULE_AUTHOR("Peter Suti <peter.suti@streamunlimited.com>");
MODULE_DESCRIPTION("Driver for controlling HSMP device state from userspace");
MODULE_LICENSE("GPL v2");
