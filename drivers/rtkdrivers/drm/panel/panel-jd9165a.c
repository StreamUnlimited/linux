// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek Panel support
*
* MIPI-DSI jd9165a panel driver. This is a 1024 * 600
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <drm/drm_modes.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>
#include <drm/drm_drv.h>
#include <video/mipi_display.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

#include "ameba_panel_base.h"
#include "ameba_panel_priv.h"

struct jd9165a {
	int gpio;
};

/*
 * The timings are not very helpful as the display is used in
 * command mode.
 */
static struct drm_display_mode jd9165a_mode = {
	.clock = 43465,
	.hdisplay = 1024,
	.hsync_start = 1184,
	.hsync_end = 1208,
	.htotal = 1344,
	.vdisplay = 600,
	.vsync_start = 612,
	.vsync_end = 614,
	.vtotal = 635,
};

static LCM_setting_table_t jd9165a_initialization[] = {/* DCS Write Long */
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x00}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 5, {0xF7, 0x49, 0x61, 0x02, 0x00}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x01}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x04, 0x0C}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x05, 0x00}}, //05=06(xhs)
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x06, 0x00}}, //06=80(xhs)
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x0B, 0x11}}, //0x13=4lanes，0x12=3lanes，0x11=2lanes，0x10=1 lanes
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x17, 0x00}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x1F, 0x05}}, //add hs_settle time
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x23, 0x00}}, //add //close gas
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x25, 0x19}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x28, 0x18}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x29, 0xFC}}, //revcom
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x2A, 0x00}}, //revcom
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x2B, 0xFC}}, //vcom
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x2C, 0x00}}, //vcom
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x02}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x01, 0x22}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x03, 0x12}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x04, 0x00}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x05, 0x64}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x0A, 0x08}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x0B, 0x0A, 0x1A, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x06, 0x08, 0x1F, 0x1D}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x0D, 0x16, 0x1B, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x07, 0x09, 0x1E, 0x1C}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x0E, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x0F, 0x16, 0x1B, 0x0D, 0x0B, 0x0D, 0x11, 0x10, 0x1C, 0x1E, 0x09, 0x07}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x10, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x11, 0x0A, 0x1A, 0x0D, 0x0B, 0x0D, 0x11, 0x10, 0x1D, 0x1F, 0x08, 0x06}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 12, {0x12, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 5, {0x14, 0x00, 0x00, 0x00, 0x00}},  //CKV_OFF
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x18, 0x99}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x06}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 15, {0x12, 0x36, 0x2B, 0x2C, 0x3A, 0x33, 0x30, 0x30, 0x2E, 0x2B, 0x1B, 0x2A, 0x20, 0x16, 0x29}},
	{MIPI_DSI_GENERIC_LONG_WRITE, 15, {0x13, 0x36, 0x2B, 0x2C, 0x3A, 0x33, 0x30, 0x30, 0x2E, 0x2B, 0x1B, 0x2A, 0x20, 0x16, 0x29}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x0A}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x02, 0x4F}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x0B, 0x40}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x12, 0x3E}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x13, 0x78}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x0D}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x0D, 0x04}}, //0x0C, 0x04
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x10, 0x0C}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x11, 0x0C}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x12, 0x0C}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x13, 0x0C}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0x30, 0x00}},
	{MIPI_DSI_DCS_SHORT_WRITE, 1, {0x11}},
	{REGFLAG_DELAY, 120, {}},
	{MIPI_DSI_DCS_SHORT_WRITE, 1, {0x29}},
	{REGFLAG_DELAY, 20, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}},
} ;

static int dsi_gpio_reset(int iod)
{
	int req_status;
	int set_direct_status;
	int gpio_index = iod ;

	req_status = gpio_request(gpio_index, NULL);
	if (req_status != 0) {
		DRM_ERROR("Gpio request fail!\n");
		return -EINVAL;
	}

	set_direct_status = gpio_direction_output(gpio_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		DRM_ERROR("Set gpio direction output fail!\n");
		return -EINVAL;
	}

	/* to prevent electric leakage */
	gpio_set_value(iod,1);
	mdelay(10);
	gpio_set_value(iod,0);
	mdelay(15);
	gpio_set_value(iod,1);
	gpio_free(gpio_index);
	mdelay(120);
	return  0 ;
}

static int set_power_by_i2c(void)
{
    struct i2c_adapter *adapter = i2c_get_adapter(2);
    struct i2c_msg power_msgs[5];
    struct i2c_client *client;
    int ret = -1;
    int retries = 0;
    u8 buf1[] = {0x00, 0x32}; // AVDD=11V
    u8 buf2[] = {0x02, 0x4D}; // VGH=20V
    u8 buf3[] = {0x03, 0x9A}; // VGL=-7V
    u8 buf4[] = {0x05, 0x5C}; // VCOM=3.6V
    u8 buf5[] = {0xFF, 0x80}; // Write to MTP

    struct i2c_board_info info = {
        I2C_BOARD_INFO("panel-power", 0x23)
    };

    if (!adapter) {
        printk(KERN_ERR "Failed to get i2c adapter\n");
        return -ENODEV;
    }

    client = i2c_new_client_device(adapter, &info);
    if (!client) {
        printk(KERN_ERR "Failed to create i2c client\n");
        i2c_put_adapter(adapter);
        return -ENOMEM;
    }

    power_msgs[0].addr = 0x23; power_msgs[0].flags = 0; power_msgs[0].len = 2; power_msgs[0].buf = buf1;
    power_msgs[1].addr = 0x23; power_msgs[1].flags = 0; power_msgs[1].len = 2; power_msgs[1].buf = buf2;
    power_msgs[2].addr = 0x23; power_msgs[2].flags = 0; power_msgs[2].len = 2; power_msgs[2].buf = buf3;
    power_msgs[3].addr = 0x23; power_msgs[3].flags = 0; power_msgs[3].len = 2; power_msgs[3].buf = buf4;
    power_msgs[4].addr = 0x23; power_msgs[4].flags = 0; power_msgs[4].len = 2; power_msgs[4].buf = buf5;

    while (retries < 2) {
        ret = i2c_transfer(client->adapter, power_msgs, 5);
        if (ret == 5) {
            break;
        }
        printk(KERN_WARNING "I2C transfer failed, retrying... (%d)\n", retries + 1);
        retries++;
    }

    if (ret != 5) {
        printk(KERN_ERR "I2C messages sent failed.\n");
        ret = -EIO;
    }

    i2c_unregister_device(client);
    i2c_put_adapter(adapter);

    return ret;
}

static int jd9165a_enable(struct drm_panel *panel)
{
	struct ameba_panel_desc  *desc = panel_to_desc(panel);
	struct jd9165a           *handle = desc->priv;
	struct device            *dev = desc->dev;
	int                      ret;

	/* set power before reset. */
	set_power_by_i2c();

	ret = dsi_gpio_reset(handle->gpio);
	if (ret) {
		DRM_ERROR("Fail to set dis spio\n");
		return ret ;
	}
	return 0;
}

static int jd9165a_disable(struct drm_panel *panel)
{
	(void)panel;
	return 0;
}

static int jd9165a_get_modes(struct drm_panel *panel, struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &jd9165a_mode);
	if (!mode) {
		DRM_ERROR("Bad mode or fail to add mode\n");
		return -EINVAL;
	}
	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	drm_mode_probed_add(connector, mode);
	return 1; /* Number of modes */
}

static int jd9165a_probe(struct device *dev,struct ameba_panel_desc *priv_data)
{
	struct device_node              *np = dev->of_node;
	struct jd9165a                  *jd9165a_data;

	jd9165a_data = devm_kzalloc(dev, sizeof(struct jd9165a), GFP_KERNEL);
	if (!jd9165a_data)
		return -ENOMEM;

	priv_data->priv = jd9165a_data ;

	jd9165a_data->gpio = of_get_named_gpio(np, "mipi-gpios", 0);
	if (!gpio_is_valid(jd9165a_data->gpio)) {
		DRM_ERROR("Panel fail to get mipi-gpios\n");
		return -ENODEV;
	}

	return 0;
}

static int jd9165a_remove(struct device *dev,struct ameba_panel_desc *priv_data)
{
	struct jd9165a      *handle = priv_data->priv;
	AMEBA_DRM_DEBUG();

	gpio_free(handle->gpio);
	return 0;
}

static struct drm_panel_funcs jd9165a_panel_funcs = {
	.disable   = jd9165a_disable,
	.enable    = jd9165a_enable,
	.get_modes = jd9165a_get_modes,
};

struct ameba_panel_desc panel_jd9165a_desc = {
	.dev          = NULL,
	.priv         = NULL,
	.init_table   = jd9165a_initialization,
	.panel_module = &jd9165a_mode,
	.rtk_panel_funcs  = &jd9165a_panel_funcs,

	.init   = jd9165a_probe,
	.deinit = jd9165a_remove,
};
EXPORT_SYMBOL(panel_jd9165a_desc);


