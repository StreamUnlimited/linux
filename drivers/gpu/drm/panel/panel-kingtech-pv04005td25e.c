// SPDX-License-Identifier: GPL-2.0
/*
 * Kingtech PV04005TD25E MIPI-DSI panel driver
 *
 * Copyright 2019 StreamUnlimited
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/backlight.h>
#include <linux/iopoll.h>
#include <linux/media-bus-format.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <video/display_timing.h>
#include <video/of_display_timing.h>
#include <video/videomode.h>
#include <video/mipi_display.h>

#include <drm/drm_print.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

struct kingtech_panel {
	struct mipi_dsi_device *dsi;
	struct drm_panel panel;
	struct gpio_desc *reset_gpio;
	struct backlight_device *backlight;
};

static inline struct kingtech_panel *to_kingtech_panel(struct drm_panel *panel)
{
	return container_of(panel, struct kingtech_panel, panel);
}

#define KINGTECH_DSI(__kingtech, __seq...)			\
	{							\
		const u8 d[] = { __seq };			\
		mipi_dsi_dcs_write_buffer(__kingtech->dsi,	\
			d, ARRAY_SIZE(d));			\
	}

static int kingtech_disable(struct drm_panel *panel)
{
	struct kingtech_panel *kingtech = to_kingtech_panel(panel);

	if (kingtech->backlight)
		backlight_disable(kingtech->backlight);

	KINGTECH_DSI(kingtech, MIPI_DCS_SET_DISPLAY_OFF);

	return 0;
}

static int kingtech_unprepare(struct drm_panel *panel)
{
	struct kingtech_panel *kingtech = to_kingtech_panel(panel);

	gpiod_set_value(kingtech->reset_gpio, 1);
	msleep(150);

	return 0;
}

static int kingtech_prepare(struct drm_panel *panel)
{
	struct kingtech_panel *kingtech = to_kingtech_panel(panel);

	/*
	 * We only do a reset here, sending commands is not possible
	 * because the LCD interface is not enabled to the DSI bridge
	 * so the transfers will stall. Instead we will use the enable
	 * callback below to send the initialization DSI, at the point
	 * of the enable callback, the LCD interface will be enabled.
	 */
	gpiod_set_value(kingtech->reset_gpio, 1);
	msleep(100);
	gpiod_set_value(kingtech->reset_gpio, 0);
	msleep(200);

	return 0;
}

static int kingtech_enable(struct drm_panel *panel)
{
	struct kingtech_panel *kingtech = to_kingtech_panel(panel);

	/*
	 * This initialization sequence is taken 1:1 from the code we received via email
	 * (`PV04005TD25E Initial code.txt`), it was just transformed to use the KINGTECH_DSI() macro.
	 * Inline comments were kept for reference.
	 */

	//---------------------------------------Bank0 Setting-------------------------------------------------//
	//------------------------------------Display Control setting----------------------------------------------//
	KINGTECH_DSI(kingtech, 0xFF, 0x77, 0x01, 0x00, 0x00, 0x10);
	KINGTECH_DSI(kingtech, 0xC0, 0x63, 0x00);
	KINGTECH_DSI(kingtech, 0xC1, 0x11, 0x02);
	KINGTECH_DSI(kingtech, 0xC2, 0x31, 0x08);
	KINGTECH_DSI(kingtech, 0xCC, 0x10);
	KINGTECH_DSI(kingtech, 0xB0, 0x40, 0x01, 0x46, 0x0D, 0x13, 0x09, 0x05, 0x09, 0x09, 0x1B, 0x07, 0x15, 0x12, 0x4C, 0x10, 0xC8);
	KINGTECH_DSI(kingtech, 0xB1, 0x40, 0x02, 0x86, 0x0D, 0x13, 0x09, 0x05, 0x09, 0x09, 0x1F, 0x07, 0x15, 0x12, 0x15, 0x19, 0x08);
	//---------------------------------------End Gamma Setting----------------------------------------------//
	//------------------------------------End Display Control setting----------------------------------------//
	//-----------------------------------------Bank0 Setting End---------------------------------------------//
	//-------------------------------------------Bank1 Setting---------------------------------------------------//
	//-------------------------------- Power Control Registers Initial --------------------------------------//
	KINGTECH_DSI(kingtech, 0xFF, 0x77, 0x01, 0x00, 0x00, 0x11);
	KINGTECH_DSI(kingtech, 0xB0, 0x50);
	//-------------------------------------------Vcom Setting---------------------------------------------------//
	KINGTECH_DSI(kingtech, 0xB1, 0x68);
	//-----------------------------------------End Vcom Setting-----------------------------------------------//
	KINGTECH_DSI(kingtech, 0xB2, 0x07);
	KINGTECH_DSI(kingtech, 0xB3, 0x80);
	KINGTECH_DSI(kingtech, 0xB5, 0x47);
	KINGTECH_DSI(kingtech, 0xB7, 0x85);
	KINGTECH_DSI(kingtech, 0xB8, 0x21);
	KINGTECH_DSI(kingtech, 0xB9, 0x10);
	KINGTECH_DSI(kingtech, 0xC1, 0x78);
	KINGTECH_DSI(kingtech, 0xC2, 0x78);
	KINGTECH_DSI(kingtech, 0xD0, 0x88);
	//---------------------------------End Power Control Registers Initial -------------------------------//
	msleep(100);
	//---------------------------------------------GIP Setting----------------------------------------------------//
	KINGTECH_DSI(kingtech, 0xE0, 0x00, 0x00, 0x02);
	KINGTECH_DSI(kingtech, 0xE1, 0x08, 0x00, 0x0A, 0x00, 0x07, 0x00, 0x09, 0x00, 0x00, 0x33, 0x33);
	KINGTECH_DSI(kingtech, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	KINGTECH_DSI(kingtech, 0xE3, 0x00, 0x00, 0x33, 0x33);
	KINGTECH_DSI(kingtech, 0xE4, 0x44, 0x44);
	KINGTECH_DSI(kingtech, 0xE5, 0x0E, 0x2D, 0xA0, 0xA0, 0x10, 0x2D, 0xA0, 0xA0, 0x0A, 0x2D, 0xA0, 0xA0, 0x0C, 0x2D, 0xA0, 0xA0);
	KINGTECH_DSI(kingtech, 0xE6, 0x00, 0x00, 0x33, 0x33);
	KINGTECH_DSI(kingtech, 0xE7, 0x44, 0x44);
	KINGTECH_DSI(kingtech, 0xE8, 0x0D, 0x2D, 0xA0, 0xA0, 0x0F, 0x2D, 0xA0, 0xA0, 0x09, 0x2D, 0xA0, 0xA0, 0x0B, 0x2D, 0xA0, 0xA0);
	KINGTECH_DSI(kingtech, 0xEB, 0x02, 0x01, 0xE4, 0xE4, 0x44, 0x00, 0x40);
	KINGTECH_DSI(kingtech, 0xEC, 0x02, 0x01);
	KINGTECH_DSI(kingtech, 0xED, 0xAB, 0x89, 0x76, 0x54, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x45, 0x67, 0x98, 0xBA);
	//--------------------------------------------End GIP Setting-----------------------------------------------//
	//------------------------------ Power Control Registers Initial End-----------------------------------//
	//------------------------------------------Bank1 Setting----------------------------------------------------//
	KINGTECH_DSI(kingtech, 0xFF, 0x77, 0x01, 0x00, 0x00, 0x00);
	KINGTECH_DSI(kingtech, 0x11);
	msleep(120);
	KINGTECH_DSI(kingtech, MIPI_DCS_SET_DISPLAY_ON);
	msleep(10);

	if (kingtech->backlight)
		backlight_enable(kingtech->backlight);

	return 0;
}

static const struct drm_display_mode default_mode = {
		.clock		= 20000,

		.hdisplay	= 480,
		.hsync_start	= 480 + 22,
		.hsync_end	= 480 + 22 + 20,
		.htotal		= 480 + 22 + 20 + 22,

		.vdisplay	= 800,
		.vsync_start	= 800 + 40,
		.vsync_end	= 800 + 40 + 5,
		.vtotal		= 800 + 40 + 5 + 40,

		.width_mm	= 52,
		.height_mm	= 86,

		.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const u32 bus_formats[] = {
	MEDIA_BUS_FMT_RGB888_1X24,
};

static int kingtech_get_modes(struct drm_panel *drm_panel,
			     struct drm_connector *connector)
{
	int ret;
	struct drm_display_mode *mode;

	dev_dbg(drm_panel->dev, "called %s\n", __func__);

	mode = drm_mode_duplicate(connector->dev, &default_mode);
	if (!mode) {
		DRM_DEV_ERROR(drm_panel->dev,
			      "failed to add mode %ux%ux@%u\n",
			      default_mode.hdisplay, default_mode.vdisplay,
			      drm_mode_vrefresh(&default_mode));
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	drm_mode_probed_add(connector, mode);

	ret = drm_display_info_set_bus_formats(&connector->display_info,
		bus_formats, ARRAY_SIZE(bus_formats));
	if (ret) {
		drm_mode_destroy(connector->dev, mode);
		return ret;
	}

	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	return 1;
}

static const struct drm_panel_funcs kingtech_panel_funcs = {
	.disable	= kingtech_disable,
	.unprepare	= kingtech_unprepare,
	.prepare	= kingtech_prepare,
	.enable		= kingtech_enable,
	.get_modes	= kingtech_get_modes,
};

static int kingtech_panel_probe(struct mipi_dsi_device *dsi)
{
	struct kingtech_panel *kingtech;
	struct device_node *np;
	int ret;

	kingtech = devm_kzalloc(&dsi->dev, sizeof(*kingtech), GFP_KERNEL);
	if (!kingtech)
		return -ENOMEM;

	kingtech->reset_gpio = devm_gpiod_get(&dsi->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(kingtech->reset_gpio)) {
		DRM_DEV_ERROR(&dsi->dev, "failed to get reset GPIO\n");
		return PTR_ERR(kingtech->reset_gpio);
	}

	np = of_parse_phandle(dsi->dev.of_node, "backlight", 0);
	if (np) {
		kingtech->backlight = of_find_backlight_by_node(np);
		of_node_put(np);

		if (!kingtech->backlight)
			return -EPROBE_DEFER;
	}

	kingtech->dsi = dsi;

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_LPM;
	mipi_dsi_set_drvdata(dsi, kingtech);

	drm_panel_init(&kingtech->panel, &dsi->dev, &kingtech_panel_funcs, DRM_MODE_CONNECTOR_DPI);

	kingtech->panel.funcs = &kingtech_panel_funcs;
	kingtech->panel.dev = &dsi->dev;

	drm_panel_add(&kingtech->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		goto err_put_backlight;

	return 0;

err_put_backlight:
	if (kingtech->backlight)
		put_device(&kingtech->backlight->dev);

	return ret;
}

static void kingtech_panel_remove(struct mipi_dsi_device *dsi)
{
	struct kingtech_panel *kingtech = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(kingtech->dsi);
	drm_panel_remove(&kingtech->panel);

	if (kingtech->backlight)
		put_device(&kingtech->backlight->dev);
}


static const struct of_device_id kingtech_of_match[] = {
	{ .compatible = "kingtech,pv04005td25e" },
	{ }
};
MODULE_DEVICE_TABLE(of, kingtech_of_match);

static struct mipi_dsi_driver kingtech_panel_driver = {
	.driver = {
		.name = "panel-kingtech-pv04005td25e",
		.of_match_table = kingtech_of_match,
	},
	.probe = kingtech_panel_probe,
	.remove = kingtech_panel_remove,
};
module_mipi_dsi_driver(kingtech_panel_driver);

MODULE_AUTHOR("Martin Pietryka <martin.pietryka@streamunlimited.com>");
MODULE_DESCRIPTION("Kingtech PV04005TD25E panel driver");
MODULE_LICENSE("GPL v2");
