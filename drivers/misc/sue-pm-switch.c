// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/pm.h>

struct sue_pm_switch {
	struct regulator *vdd;
};

static int sue_pm_switch_probe(struct platform_device *pdev)
{
	struct sue_pm_switch *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	priv->vdd = devm_regulator_get(&pdev->dev, "vdd");
	if (IS_ERR(priv->vdd)) {
		dev_err(&pdev->dev, "Failed to get VDD regulator: %ld\n", PTR_ERR(priv->vdd));
		return PTR_ERR(priv->vdd);
	}

	return regulator_enable(priv->vdd);
}

static int sue_pm_switch_remove(struct platform_device *pdev)
{
	struct sue_pm_switch *priv = platform_get_drvdata(pdev);

	return regulator_disable(priv->vdd);
}

static void sue_pm_switch_shutdown(struct platform_device *pdev)
{
	struct sue_pm_switch *priv = platform_get_drvdata(pdev);

	regulator_disable(priv->vdd);
}

static int sue_pm_switch_suspend(struct device *dev)
{
	struct sue_pm_switch *priv = dev_get_drvdata(dev);

	return regulator_disable(priv->vdd);
}

static int sue_pm_switch_resume(struct device *dev)
{
	struct sue_pm_switch *priv = dev_get_drvdata(dev);

	return regulator_enable(priv->vdd);
}

static SIMPLE_DEV_PM_OPS(sue_pm_switch_pm_ops, sue_pm_switch_suspend, sue_pm_switch_resume);

static const struct of_device_id sue_pm_switch_of_match[] = {
	{ .compatible = "sue,pm-switch" },
	{ }
};
MODULE_DEVICE_TABLE(of, sue_pm_switch_of_match);

static struct platform_driver sue_pm_switch_driver = {
	.driver = {
		.name = "sue-usb-eth-power",
		.of_match_table = sue_pm_switch_of_match,
		.pm = &sue_pm_switch_pm_ops,
	},
	.probe = sue_pm_switch_probe,
	.remove = sue_pm_switch_remove,
	.shutdown = sue_pm_switch_shutdown,
};
module_platform_driver(sue_pm_switch_driver);

MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_DESCRIPTION("Control Regulator on pm suspend/resume");
MODULE_LICENSE("GPL");
