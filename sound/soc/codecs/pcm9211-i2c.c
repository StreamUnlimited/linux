/*
 * PCM9211 codec i2c driver
 *
 * Copyright (C) 2017 jusst technologies GmbH / jusst.engineering
 *
 * Author: Julian Scheel <julian@jusst.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/module.h>

#include <sound/soc.h>

#include "pcm9211.h"

static int pcm9211_i2c_probe(struct i2c_client *i2c)
{
	struct regmap *regmap;

	regmap = devm_regmap_init_i2c(i2c, &pcm9211_regmap);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	return pcm9211_probe(&i2c->dev, regmap);
}

static int pcm9211_i2c_remove(struct i2c_client *i2c)
{
	pcm9211_remove(&i2c->dev);
	return 0;
}

static const struct i2c_device_id pcm9211_i2c_id[] = {
	{ "pcm9211", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcm9211_i2c_id);

static const struct of_device_id pcm9211_of_match[] = {
	{ .compatible = "ti,pcm9211", },
	{ }
};
MODULE_DEVICE_TABLE(of, pcm9211_of_match);

static struct i2c_driver pcm9211_i2c_driver = {
	.probe_new = pcm9211_i2c_probe,
	.remove = pcm9211_i2c_remove,
	.id_table = pcm9211_i2c_id,
	.driver = {
		.name = "pcm9211",
		.of_match_table = pcm9211_of_match,
		.pm = &pcm9211_pm_ops,
	},
};
module_i2c_driver(pcm9211_i2c_driver);

MODULE_DESCRIPTION("PCM9211 I2C codec driver");
MODULE_AUTHOR("Julian Scheel <julian@jusst.de>");
MODULE_LICENSE("GPL v2");
