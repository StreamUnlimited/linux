/*
 * AXP15060 regulator driver.
 *
 * Copyright (C) 2019 Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>


#define AXP15060_POWERON_SRC		0x00
#define AXP15060_DATA_BUFFER(m)		(0x04 + (m))
#define AXP15060_PWR_CTRL_1		0x10
#define AXP15060_PWR_CTRL_2		0x11
#define AXP15060_PWR_CTRL_3		0x12
#define AXP15060_DCDC1_V_CTRL		0x13
#define AXP15060_DCDC2_V_CTRL		0x14
#define AXP15060_DCDC3_V_CTRL		0x15
#define AXP15060_DCDC4_V_CTRL		0x16
#define AXP15060_DCDC5_V_CTRL		0x17
#define AXP15060_DCDC6_V_CTRL		0x18
#define AXP15060_ALDO1_V_CTRL		0x19
#define AXP15060_DCDC_MODE_CTRL_1	0x1A
#define AXP15060_DCDC_MODE_CTRL_2	0x1B
#define AXP15060_OUTPUT_MONITOR_CTRL	0x1E
#define AXP15060_IRQ_PWROK_VOFF		0x1F
#define AXP15060_ALDO2_V_CTRL		0x20
#define AXP15060_ALDO3_V_CTRL		0x21
#define AXP15060_ALDO4_V_CTRL		0x22
#define AXP15060_ALDO5_V_CTRL		0x23
#define AXP15060_BLDO1_V_CTRL		0x24
#define AXP15060_BLDO2_V_CTRL		0x25
#define AXP15060_BLDO3_V_CTRL		0x26
#define AXP15060_BLDO4_V_CTRL		0x27
#define AXP15060_BLDO5_V_CTRL		0x28
#define AXP15060_CLDO1_V_CTRL		0x29
#define AXP15060_CLDO2_V_CTRL		0x2A
#define AXP15060_CLDO3_V_CTRL		0x2B
#define AXP15060_CLDO4_GPIO2_CTRL	0x2C
#define AXP15060_CLDO4_V_CTRL		0x2D
#define AXP15060_CPUSLDO_V_CTRL		0x2E
#define AXP15060_PWR_WKUP_CTRL		0x31
#define AXP15060_PWR_DIS_PWR_DWN	0x32
#define AXP15060_PWROK_SET		0x36
#define AXP15060_IRQ_EN_1		0x40
#define AXP15060_IRQ_EN_2		0x41
#define AXP15060_IRQ_STATUS_1		0x48
#define AXP15060_IRQ_STATUS_2		0x49

#define AXP15060_DCDC16_V_CTRL_MASK	0x1F
#define AXP15060_DCDC2345_V_CTRL_MASK	0x7F
#define AXP15060_ALDO_V_CTRL_MASK	0x1F
#define AXP15060_BLDO_V_CTRL_MASK	0x1F
#define AXP15060_CLDO_V_CTRL_MASK	0x1F
#define AXP15060_CPUSLDO_V_CTRL_MASK	0x0F

struct axp15060_data_buffer_attr_info {
	struct device_attribute attr;
	u32 offset;
};

static ssize_t axp15060_data_buffer_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	unsigned int val;
	struct regmap *regmap = dev_get_regmap(dev, NULL);
	struct axp15060_data_buffer_attr_info *attr_info = container_of(attr, struct axp15060_data_buffer_attr_info, attr);

	ret = regmap_read(regmap, AXP15060_DATA_BUFFER(attr_info->offset), &val);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%u\n", val);
}

static ssize_t axp15060_data_buffer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int val;
	struct regmap *regmap = dev_get_regmap(dev, NULL);
	struct axp15060_data_buffer_attr_info *attr_info = container_of(attr, struct axp15060_data_buffer_attr_info, attr);

	if (sscanf(buf, "%u", &val) != 1)
		return -EINVAL;

	ret = regmap_write(regmap, AXP15060_DATA_BUFFER(attr_info->offset), val);
	if (ret < 0)
		return ret;

	return count;
}

#define AXP15060_DATA_BUFFER_ATTR(_name, _offset) { \
	.attr = __ATTR(_name, S_IWUSR | S_IRUGO, axp15060_data_buffer_show, axp15060_data_buffer_store), \
	.offset = _offset, \
}

static struct axp15060_data_buffer_attr_info axp15060_data_buffer_attrs[] = {
	AXP15060_DATA_BUFFER_ATTR(data_buf_0, 0),
	AXP15060_DATA_BUFFER_ATTR(data_buf_1, 1),
	AXP15060_DATA_BUFFER_ATTR(data_buf_2, 2),
	AXP15060_DATA_BUFFER_ATTR(data_buf_3, 3),
};

static struct attribute *axp15060_attribute_list[] = {
	&axp15060_data_buffer_attrs[0].attr.attr,
	&axp15060_data_buffer_attrs[1].attr.attr,
	&axp15060_data_buffer_attrs[2].attr.attr,
	&axp15060_data_buffer_attrs[3].attr.attr,
	NULL
};

static const struct attribute_group axp15060_attribute_group = {
	.name = "axp15060",
	.attrs = axp15060_attribute_list,
};

static const struct regmap_range axp15060_volatile_ranges[] = {
	regmap_reg_range(AXP15060_POWERON_SRC, AXP15060_POWERON_SRC),
	regmap_reg_range(AXP15060_IRQ_STATUS_1, AXP15060_IRQ_STATUS_2),
};

static const struct regmap_access_table axp15060_volatile_table = {
	.yes_ranges	= axp15060_volatile_ranges,
	.n_yes_ranges	= ARRAY_SIZE(axp15060_volatile_ranges),
};

static const struct regmap_config axp15060_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.volatile_table	= &axp15060_volatile_table,
	.max_register	= AXP15060_IRQ_STATUS_2,
	.cache_type	= REGCACHE_RBTREE,
};

#define AXP15060_DCDC234_NUM_VOLTAGES 88
static const struct linear_range axp15060_dcdc234_ranges[] = {
	REGULATOR_LINEAR_RANGE(500 * 1000,   0,  70, 10 * 1000),
	REGULATOR_LINEAR_RANGE(1220 * 1000,   71,  87, 20 * 1000),
};

#define AXP15060_DCDC5_NUM_VOLTAGES 69
static const struct linear_range axp15060_dcdc5_ranges[] = {
	REGULATOR_LINEAR_RANGE(800 * 1000,   0,  32, 10 * 1000),
	REGULATOR_LINEAR_RANGE(1140 * 1000,   33,  68, 20 * 1000),
};

static const struct regulator_ops axp15060_ops = {
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.list_voltage		= regulator_list_voltage_linear,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static const struct regulator_ops axp15060_ops_range = {
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.list_voltage		= regulator_list_voltage_linear_range,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};


#define AXP15060_REG(_id, _match, _min, _max, _step, _vreg, _vmask, _ereg, _emask) \
	{									\
		.name		= (_match),					\
		.of_match	= of_match_ptr(_match),				\
		.regulators_node = of_match_ptr("regulators"),			\
		.type		= REGULATOR_VOLTAGE,				\
		.id		= _id,						\
		.n_voltages	= (((_max) - (_min)) / (_step) + 1),		\
		.owner		= THIS_MODULE,					\
		.min_uV		= (_min) * 1000,				\
		.uV_step	= (_step) * 1000,				\
		.vsel_reg	= (_vreg),					\
		.vsel_mask	= (_vmask),					\
		.enable_reg	= (_ereg),					\
		.enable_mask	= (_emask),					\
		.ops		= &axp15060_ops,					\
	}

#define AXP15060_REG_RANGE(_id, _match, _ranges, _num_voltages, _vreg, _vmask, _ereg, _emask) \
	{									\
		.name		= (_match),					\
		.of_match	= of_match_ptr(_match),				\
		.regulators_node = of_match_ptr("regulators"),			\
		.type		= REGULATOR_VOLTAGE,				\
		.id		= _id,						\
		.owner		= THIS_MODULE,					\
		.linear_ranges	= _ranges,					\
		.n_linear_ranges = ARRAY_SIZE(_ranges),				\
		.n_voltages	= (_num_voltages),				\
		.vsel_reg	= (_vreg),					\
		.vsel_mask	= (_vmask),					\
		.enable_reg	= (_ereg),					\
		.enable_mask	= (_emask),					\
		.ops		= &axp15060_ops_range,				\
	}

enum axp15060_regulators
{
	DCDC1 = 0,
	DCDC2, DCDC3, DCDC4, DCDC5, DCDC6,
	ALDO1, ALDO2, ALDO3, ALDO4, ALDO5,
	BLDO1, BLDO2, BLDO3, BLDO4, BLDO5,
	CLDO1, CLDO2, CLDO3, CLDO4,
	CPUSLDO,
};

static const struct regulator_desc axp15060_regulators[] = {
	AXP15060_REG(DCDC1, "dcdc1", 1500, 3400, 100, AXP15060_DCDC1_V_CTRL, AXP15060_DCDC16_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 0)),

	AXP15060_REG_RANGE(DCDC2, "dcdc2", axp15060_dcdc234_ranges, AXP15060_DCDC234_NUM_VOLTAGES,
				AXP15060_DCDC2_V_CTRL, AXP15060_DCDC2345_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 1)),
	AXP15060_REG_RANGE(DCDC3, "dcdc3", axp15060_dcdc234_ranges, AXP15060_DCDC234_NUM_VOLTAGES,
				AXP15060_DCDC3_V_CTRL, AXP15060_DCDC2345_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 2)),
	AXP15060_REG_RANGE(DCDC4, "dcdc4", axp15060_dcdc234_ranges, AXP15060_DCDC234_NUM_VOLTAGES,
				AXP15060_DCDC4_V_CTRL, AXP15060_DCDC2345_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 3)),
	AXP15060_REG_RANGE(DCDC5, "dcdc5", axp15060_dcdc5_ranges, AXP15060_DCDC5_NUM_VOLTAGES,
				AXP15060_DCDC5_V_CTRL, AXP15060_DCDC2345_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 4)),

	AXP15060_REG(DCDC6, "dcdc6", 500, 3400, 100, AXP15060_DCDC1_V_CTRL, AXP15060_DCDC16_V_CTRL_MASK, AXP15060_PWR_CTRL_1, (1 << 5)),

	AXP15060_REG(ALDO1, "aldo1", 700, 3300, 100, AXP15060_ALDO1_V_CTRL, AXP15060_ALDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 0)),
	AXP15060_REG(ALDO2, "aldo2", 700, 3300, 100, AXP15060_ALDO2_V_CTRL, AXP15060_ALDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 1)),
	AXP15060_REG(ALDO3, "aldo3", 700, 3300, 100, AXP15060_ALDO3_V_CTRL, AXP15060_ALDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 2)),
	AXP15060_REG(ALDO4, "aldo4", 700, 3300, 100, AXP15060_ALDO4_V_CTRL, AXP15060_ALDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 3)),
	AXP15060_REG(ALDO5, "aldo5", 700, 3300, 100, AXP15060_ALDO5_V_CTRL, AXP15060_ALDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 4)),

	AXP15060_REG(BLDO1, "bldo1", 700, 3300, 100, AXP15060_BLDO1_V_CTRL, AXP15060_BLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 5)),
	AXP15060_REG(BLDO2, "bldo2", 700, 3300, 100, AXP15060_BLDO2_V_CTRL, AXP15060_BLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 6)),
	AXP15060_REG(BLDO3, "bldo3", 700, 3300, 100, AXP15060_BLDO3_V_CTRL, AXP15060_BLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_2, (1 << 7)),
	AXP15060_REG(BLDO4, "bldo4", 700, 3300, 100, AXP15060_BLDO4_V_CTRL, AXP15060_BLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 0)),
	AXP15060_REG(BLDO5, "bldo5", 700, 3300, 100, AXP15060_BLDO5_V_CTRL, AXP15060_BLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 1)),

	AXP15060_REG(CLDO1, "cldo1", 700, 3300, 100, AXP15060_CLDO1_V_CTRL, AXP15060_CLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 2)),
	AXP15060_REG(CLDO2, "cldo2", 700, 3300, 100, AXP15060_CLDO2_V_CTRL, AXP15060_CLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 3)),
	AXP15060_REG(CLDO3, "cldo3", 700, 3300, 100, AXP15060_CLDO3_V_CTRL, AXP15060_CLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 4)),
	AXP15060_REG(CLDO4, "cldo4", 700, 3300, 100, AXP15060_CLDO4_V_CTRL, AXP15060_CLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 5)),

	AXP15060_REG(CPUSLDO, "cpusldo", 700, 1400, 50, AXP15060_CPUSLDO_V_CTRL, AXP15060_CPUSLDO_V_CTRL_MASK, AXP15060_PWR_CTRL_3, (1 << 6)),
};

static int axp15060_i2c_probe(struct i2c_client *client)
{
	struct regulator_config config = { };
	struct regmap *regmap;
	int i;

	regmap = devm_regmap_init_i2c(client, &axp15060_regmap_config);

	config.dev = &client->dev;
	config.driver_data = NULL;
	config.regmap = regmap;

	for (i = 0; i < ARRAY_SIZE(axp15060_regulators); i++) {
		struct regulator_dev *rdev;

		rdev = devm_regulator_register(&client->dev, &axp15060_regulators[i], &config);
		if (IS_ERR(rdev)) {
			dev_err(&client->dev, "failed to register %s regulator\n", axp15060_regulators[i].name);
			return PTR_ERR(rdev);
		}
	}

	return devm_device_add_group(&client->dev, &axp15060_attribute_group);
}

static const struct of_device_id axp15060_of_match[] = {
	{ .compatible = "x-powers,axp15060" },
	{ },
};
MODULE_DEVICE_TABLE(of, axp15060_of_match);

static struct i2c_driver axp15060_i2c_driver = {
	.driver = {
		.name	= "axp15060-i2c",
		.of_match_table	= of_match_ptr(axp15060_of_match),
	},
	.probe		= axp15060_i2c_probe,
};
module_i2c_driver(axp15060_i2c_driver);

MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_DESCRIPTION("AXP15060 PMIC driver");
MODULE_LICENSE("GPL v2");
