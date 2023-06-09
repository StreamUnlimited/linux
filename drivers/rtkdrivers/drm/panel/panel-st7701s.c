// SPDX-License-Identifier: GPL-2.0+
/*
 * MIPI-DSI st7701s panel driver. This is a 480*800
 */

#include <drm/drm_modes.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>
#include <video/mipi_display.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/module.h>

#include "ameba_panel_base.h"

struct st7701s {
	int                     gpio;
	struct device           *dev;
	struct drm_panel        panel;
};

/*
 * The timings are not very helpful as the display is used in
 * command mode.
 */
static const struct drm_display_mode st7701s_mode = {
	/* HS clock, (htotal*vtotal*vrefresh)/1000 */
	.clock = 30720,
	.hdisplay = 480,
	.hsync_start = 481,
	.hsync_end = 484,
	.htotal = 500,
	.vdisplay = 800,
	.vsync_start = 824,
	.vsync_end = 896,
	.vtotal = 1024,

	.vrefresh = 60,
};

static LCM_setting_table_t lcm_initialization_setting[] = {/* DCS Write Long */
	/* ST7701S Reset Sequence */
	/* LCD_Nreset (1); Delayms (1); */
	/* LCD_Nreset (0); Delayms (1); */
	/* LCD_Nreset (1); Delayms (120); */
	{MIPI_DSI_DCS_SHORT_WRITE, 1, {0x11, 0x00}},
	{REGFLAG_DELAY, 120, {}},/* Delayms (120) */

	/* Bank0 Setting */
	/* Display Control setting */
	{MIPI_DSI_DCS_LONG_WRITE, 6, {0xFF, 0x77, 0x01, 0x00, 0x00, 0x10}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xC0, 0x63, 0x00}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xC1, 0x0C, 0x02}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xC2, 0x31, 0x08}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xCC, 0x10}},

	/* Gamma Cluster Setting */
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xB0, 0x40, 0x02, 0x87, 0x0E, 0x15, 0x0A, 0x03, 0x0A, 0x0A, 0x18, 0x08, 0x16, 0x13, 0x07, 0x09, 0x19}},
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xB1, 0x40, 0x01, 0x86, 0x0D, 0x13, 0x09, 0x03, 0x0A, 0x09, 0x1C, 0x09, 0x15, 0x13, 0x91, 0x16, 0x19}},
	/* End Gamma Setting */
	/* End Display Control setting */
	/* End Bank0 Setting */

	/* Bank1 Setting */
	/* Power Control Registers Initial */
	{MIPI_DSI_DCS_LONG_WRITE, 6, {0xFF, 0x77, 0x01, 0x00, 0x00, 0x11}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB0, 0x4D}},

	/* Vcom Setting */
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB1, 0x64}},
	/* End End Vcom Setting */

	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB2, 0x07}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB3, 0x80}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB5, 0x47}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB7, 0x85}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB8, 0x21}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xB9, 0x10}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xC1, 0x78}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xC2, 0x78}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 2, {0xD0, 0x88}},
	/* End Power Control Registers Initial */
	{REGFLAG_DELAY, 100, {}},/* Delayms (100) */
	/* GIP Setting */
	{MIPI_DSI_DCS_LONG_WRITE, 4, {0xE0, 0x00, 0x84, 0x02}},
	{MIPI_DSI_DCS_LONG_WRITE, 12, {0xE1, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20}},
	{MIPI_DSI_DCS_LONG_WRITE, 14, {0xE2, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{MIPI_DSI_DCS_LONG_WRITE, 5, {0xE3, 0x00, 0x00, 0x33, 0x33}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xE4, 0x44, 0x44}},
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xE5, 0x09, 0x31, 0xBE, 0xA0, 0x0B, 0x31, 0xBE, 0xA0, 0x05, 0x31, 0xBE, 0xA0, 0x07, 0x31, 0xBE, 0xA0}},
	{MIPI_DSI_DCS_LONG_WRITE, 5, {0xE6, 0x00, 0x00, 0x33, 0x33}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xE7, 0x44, 0x44}},
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xE8, 0x08, 0x31, 0xBE, 0xA0, 0x0A, 0x31, 0xBE, 0xA0, 0x04, 0x31, 0xBE, 0xA0, 0x06, 0x31, 0xBE, 0xA0}},
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xEA, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00}},
	{MIPI_DSI_DCS_LONG_WRITE, 8, {0xEB, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{MIPI_DSI_DCS_LONG_WRITE, 3, {0xEC, 0x02, 0x00}},
	{MIPI_DSI_DCS_LONG_WRITE, 17, {0xED, 0xF5, 0x47, 0x6F, 0x0B, 0x8F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xF8, 0xB0, 0xF6, 0x74, 0x5F}},
	{MIPI_DSI_DCS_LONG_WRITE, 13, {0xEF, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
	/* End GIP Setting */

	{MIPI_DSI_DCS_LONG_WRITE, 6, {0xFF, 0x77, 0x01, 0x00, 0x00, 0x00}},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM, 1, {0x29, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
} ;

static inline struct st7701s *panel_to_st7701s(struct drm_panel *panel)
{
	return container_of(panel, struct st7701s, panel);
}

static inline void dsi_gpio_set_pin(int gpio_index,u8 Newstatus)
{
	gpio_set_value(gpio_index, Newstatus);
}
static int dsi_gpio_reset(int iod)
{
	int                 req_status;
	int                 set_direct_status;
	int                 gpio_index = iod ;

	req_status = gpio_request(gpio_index, NULL);
	if (req_status != 0) {
		DRM_WARN("gpio request failed!\n");
		return -EINVAL;
	}

	set_direct_status = gpio_direction_output(gpio_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		DRM_WARN("set gpio direction output failed\n");
		return -EINVAL;
	}

	/* to prevent electric leakage */
	dsi_gpio_set_pin(iod,1);
	mdelay(10);
	dsi_gpio_set_pin(iod,0);
	mdelay(10);
	dsi_gpio_set_pin(iod,1);
	gpio_free(gpio_index);

	mdelay(120);

	return  0 ;
}

static int st7701s_enable(struct drm_panel *panel)
{
	struct st7701s      *handle = panel_to_st7701s(panel);
	struct device       *dev = handle->dev;
	int ret;

	ret = dsi_gpio_reset(handle->gpio);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to set dis spio\n");
		return ret ;
	}

	return 0;
}

static int st7701s_disable(struct drm_panel *panel)
{
	(void)panel;
	return 0;
}

static int st7701s_get_modes(struct drm_panel *panel)
{
	struct drm_connector        *connector = panel->connector;
	struct drm_display_mode     *mode;

	mode = drm_mode_duplicate(panel->drm, &st7701s_mode);
	if (!mode) {
		DRM_ERROR("bad mode or failed to add mode\n");
		return -EINVAL;
	}
	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	drm_mode_probed_add(connector, mode);

	return 1; /* Number of modes */
}

static const struct drm_panel_funcs st7701s_drm_funcs = {
	.disable = st7701s_disable,
	.enable = st7701s_enable,
	.get_modes = st7701s_get_modes,
};

static int st7701s_probe(struct mipi_dsi_device *dsi)
{
	struct device                   *dev = &dsi->dev;
	struct device_node              *np = dev->of_node;
	enum of_gpio_flags              flags;
	struct ameba_drm_panel_struct   *ameba_panel;
	struct st7701s                  *priv_data;
	int                             ret;

	ameba_panel = devm_kzalloc(dev, sizeof(struct ameba_drm_panel_struct), GFP_KERNEL);
	if (!ameba_panel)
		return -ENOMEM;

	priv_data = devm_kzalloc(dev, sizeof(struct st7701s), GFP_KERNEL);
	if (!priv_data)
		return -ENOMEM;

	ameba_panel->panel_priv = priv_data;
	ameba_panel->init_table = lcm_initialization_setting;

	mipi_dsi_set_drvdata(dsi, ameba_panel);
	priv_data->dev = dev;

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;

	//gpio
	priv_data->gpio = of_get_named_gpio_flags(np, "mipi-gpios", 0, &flags);
	if (!gpio_is_valid(priv_data->gpio)) {
		DRM_DEV_ERROR(dev, "drm mipi dis node failed to get mipi-gpios\n");
		return -ENODEV;
	}

	/*
	 * This display uses command mode so no MIPI_DSI_MODE_VIDEO
	 * or MIPI_DSI_MODE_VIDEO_SYNC_PULSE
	 *
	 * As we only send commands we do not need to be continuously
	 * clocked.
	 */
	dsi->mode_flags = MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_EOT_PACKET;

	drm_panel_init(&priv_data->panel);
	priv_data->panel.dev = dev;
	priv_data->panel.funcs = &st7701s_drm_funcs;

	ret = drm_panel_add(&priv_data->panel);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		drm_panel_remove(&priv_data->panel);

	return ret;
}

static int st7701s_remove(struct mipi_dsi_device *dsi)
{
	struct ameba_drm_panel_struct       *ameba_panel = mipi_dsi_get_drvdata(dsi);
	struct st7701s                      *handle = ameba_panel->panel_priv;

	mipi_dsi_detach(dsi);
	drm_panel_remove(&handle->panel);

	return 0;
}

static const struct of_device_id st7701s_of_match[] = {
	{ .compatible = "realtek,st7701s" },
	{ }
};
MODULE_DEVICE_TABLE(of, st7701s_of_match);

static struct mipi_dsi_driver st7701s_driver = {
	.probe = st7701s_probe,
	.remove = st7701s_remove,
	.driver = {
		.name = "panel-st7701s",
		.of_match_table = st7701s_of_match,
	},
};
module_mipi_dsi_driver(st7701s_driver);

MODULE_AUTHOR("Chunlin.Yi <chunlin.yi@realsil.com.cn>");
MODULE_DESCRIPTION("realtek AmebaD2 SoCs' Panel driver");
MODULE_LICENSE("GPL v2");
