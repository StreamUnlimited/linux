/*
 * Copyright (c) 2021 Realtek, LLC.
 * All rights reserved.
 *
 * Licensed under the Realtek License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License from Realtek
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/of_platform.h>
#include <linux/component.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_gem_cma_helper.h>
//#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_of.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>

#include "ameba_drm_drv.h"

extern struct ameba_drm_data lcdc_driver_data;



/*
	mipi dsi underflow issue
*/
struct ameba_lcdc_param_struct {
	//used for underflow
	u32 under_flow_count ;
	volatile u32 under_flow_flag ;
	void *lcdc_hw_ctx ; 	//struct lcdc_hw_ctx_type handle
	void *dsi_local ;  		//dsi dev handle
};
static struct ameba_lcdc_param_struct lcdcUndFlwInstance={0,0,NULL,NULL} ;

//struct ameba_lcdc_param_struct
void* drm_get_param_handle(void)
{
	return &lcdcUndFlwInstance ;
}
void drm_underflow_flag_add(unsigned char value)
{
	lcdcUndFlwInstance.under_flow_flag += value ;
	lcdcUndFlwInstance.under_flow_count += value ;
}
void drm_underflow_flag_reset()
{
	lcdcUndFlwInstance.under_flow_flag = 0 ;
}
u32 drm_get_underflow_flag(void)
{
	return lcdcUndFlwInstance.under_flow_flag;
}

//mipi
void drm_set_mipidsi_param(void *data)
{
	lcdcUndFlwInstance.dsi_local = data;
}
void* drm_get_mipidsi_param(void)
{
	return lcdcUndFlwInstance.dsi_local ;
}
void* drm_get_mipidsi_param2(void* data)
{
	struct ameba_lcdc_param_struct *phandle = (struct ameba_lcdc_param_struct*)data;
	return phandle->dsi_local ;
}
//lcdc
void drm_set_lcdc_param(void *data)
{
	lcdcUndFlwInstance.lcdc_hw_ctx = data;
}
void* drm_get_lcdc_param(void)
{
	return lcdcUndFlwInstance.lcdc_hw_ctx ;
}


static int ameba_drm_crtc_init(struct drm_device *dev, struct drm_crtc *crtc,
							   struct drm_plane *plane,
							   const struct ameba_drm_data *driver_data)
{
	struct device_node *port;
	int ret;

	/* set crtc port so that
	 * drm_of_find_possible_crtcs call works
	 */
	port = of_get_child_by_name(dev->dev->of_node, "port");
	if (!port) {
		DRM_DEV_ERROR(dev->dev, "no port node found in %pOF\n", dev->dev->of_node);
		return -EINVAL;
	}
	of_node_put(port);
	crtc->port = port;

	ret = drm_crtc_init_with_planes(dev, crtc, plane, NULL,
									driver_data->crtc_funcs, NULL);
	if (ret) {
		DRM_DEV_ERROR(dev->dev, "failed to init crtc.\n");
		return ret;
	}

	drm_crtc_helper_add(crtc, driver_data->crtc_helper_funcs);

	return 0;
}

static int ameba_drm_plane_init(struct drm_device *dev, struct drm_plane *plane,
								enum drm_plane_type type,
								const struct ameba_drm_data *data)
{
	int ret = 0;

	ret = drm_universal_plane_init(dev, plane, 0xff, data->plane_funcs,
								   data->channel_formats,
								   data->channel_formats_cnt,
								   NULL, type, NULL);
	if (ret) {
		DRM_DEV_ERROR(dev->dev, "fail to init plane\n");
		return ret;
	}


	drm_plane_helper_add(plane, data->plane_helper_funcs);

	return 0;
}

static void ameba_drm_private_cleanup(struct drm_device *dev)
{
	struct ameba_drm_data 		*data;
	struct ameba_drm_private 	*ameba_priv = dev->dev_private;
	struct platform_device		*pdev = to_platform_device(dev->dev);

	data = (struct ameba_drm_data *)of_device_get_match_data(dev->dev);
	if (data->cleanup_hw_ctx) {
		data->cleanup_hw_ctx(pdev, ameba_priv->lcdc_hw_ctx);
	}

	devm_kfree(dev->dev, ameba_priv);

	dev->dev_private = NULL;
}

static int ameba_drm_private_init(struct drm_device *dev,
								  const struct ameba_drm_data *driver_data)
{
	u32 ch;
	int ret;
	void *ctx;
	enum drm_plane_type 		type;
	struct drm_plane 			*prim_plane;
	struct ameba_drm_private 	*ameba_priv;
	struct platform_device 		*pdev = to_platform_device(dev->dev);

	ameba_priv = devm_kzalloc(dev->dev, sizeof(*ameba_priv), GFP_KERNEL);
	if (!ameba_priv) {
		DRM_DEV_ERROR(dev->dev, "failed to alloc ameba_drm_private\n");
		return -ENOMEM;
	}

	ctx = driver_data->alloc_hw_ctx(pdev, &ameba_priv->crtc.base);
	if (IS_ERR(ctx)) {
		DRM_DEV_ERROR(dev->dev, "failed to initialize ameba_priv hw ctx\n");
		return -EINVAL;
	}
	ameba_priv->lcdc_hw_ctx = ctx;

	/*
	 * plane init, support all plane info
	 */
	for (ch = 0; ch < driver_data->num_planes; ch++) {
		if (ch == driver_data->prim_plane) {
			type = DRM_PLANE_TYPE_PRIMARY;
		} else {
			type = DRM_PLANE_TYPE_OVERLAY;
		}
		ret = ameba_drm_plane_init(dev, &ameba_priv->planes[ch].base,
								   type, driver_data);
		if (ret) {
			return ret;
		}
		ameba_priv->planes[ch].ch = ch;
		ameba_priv->planes[ch].lcdc_hw_ctx = ctx;
	}

	/* crtc init */
	prim_plane = &ameba_priv->planes[driver_data->prim_plane].base;
	ret = ameba_drm_crtc_init(dev, &ameba_priv->crtc.base,
							  prim_plane, driver_data);
	if (ret) {
		return ret;
	}
	ameba_priv->crtc.lcdc_hw_ctx = ctx;
	ameba_priv->crtc.enable = false ;

	dev->dev_private = ameba_priv;

	return 0;
}

static int ameba_drm_kms_init(struct drm_device *dev,
							  const struct ameba_drm_data *driver_data)
{
	int ret;

	/* dev->mode_config initialization */
	drm_mode_config_init(dev);
	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;
	dev->mode_config.funcs = driver_data->mode_config_funcs;

	/* display controller init */
	ret = ameba_drm_private_init(dev, driver_data);
	if (ret) {
		goto err_mode_config_cleanup;
	}

	//update the display size
	dev->mode_config.max_width = ameba_drm_get_display_width();
	dev->mode_config.max_height = ameba_drm_get_display_height();

	/* bind and init sub drivers */
	ret = component_bind_all(dev->dev, dev);
	if (ret) {
		DRM_DEV_ERROR(dev->dev, "failed to bind all component.\n");
		goto err_private_cleanup;
	}

	/* vblank init */
	ret = drm_vblank_init(dev, dev->mode_config.num_crtc);
	if (ret) {
		DRM_DEV_ERROR(dev->dev, "failed to initialize vblank.\n");
		goto err_unbind_all;
	}
	/* with irq_enabled = true, we can use the vblank feature. */
	dev->irq_enabled = true;

	/* reset all the states of crtc/plane/encoder/connector */
	drm_mode_config_reset(dev);

	/* init kms poll for handling hpd */
	drm_kms_helper_poll_init(dev);

	return 0;

err_unbind_all:
	component_unbind_all(dev->dev, dev);
err_private_cleanup:
	ameba_drm_private_cleanup(dev);
err_mode_config_cleanup:
	drm_mode_config_cleanup(dev);

	return ret;
}

static int compare_of(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static int ameba_drm_kms_cleanup(struct drm_device *dev)
{
	drm_kms_helper_poll_fini(dev);

	ameba_drm_private_cleanup(dev);

	drm_mode_config_cleanup(dev);

	return 0;
}

static int ameba_drm_connectors_register(struct drm_device *dev)
{
	int ret;
	struct drm_connector *connector = NULL;
	struct drm_connector *failed_connector;
	struct drm_connector_list_iter conn_iter;

	mutex_lock(&dev->mode_config.mutex);
	drm_connector_list_iter_begin(dev, &conn_iter);

	drm_for_each_connector_iter(connector, &conn_iter) {
		ret = drm_connector_register(connector);
		if (ret) {
			failed_connector = connector;
			goto err;
		}
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	return 0;

err:
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (failed_connector == connector) {
			break;
		}
		drm_connector_unregister(connector);
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	return ret;
}

static int ameba_drm_bind(struct device *dev)
{
	struct ameba_drm_data *driver_data;
	struct drm_device *drm_dev;
	int ret;

	driver_data = (struct ameba_drm_data *)of_device_get_match_data(dev);
	if (!driver_data) {
		return -EINVAL;
	}

	//will call drm_dev_init
	drm_dev = drm_dev_alloc(driver_data->driver, dev);
	if (IS_ERR(drm_dev)) {
		return PTR_ERR(drm_dev);
	}
	dev_set_drvdata(dev, drm_dev);

	/* display controller init */
	ret = ameba_drm_kms_init(drm_dev, driver_data);
	if (ret) {
		goto err_drm_dev_put;
	}

	//register the device
	ret = drm_dev_register(drm_dev, 0);
	if (ret) {
		goto err_kms_cleanup;
	}

	/* connectors should be registered after drm device register */
	if (driver_data->register_connects) {
		ret = ameba_drm_connectors_register(drm_dev);
		if (ret) {
			goto err_drm_dev_unregister;
		}
	}

	return 0;

err_drm_dev_unregister:
	drm_dev_unregister(drm_dev);
err_kms_cleanup:
	ameba_drm_kms_cleanup(drm_dev);
err_drm_dev_put:
	drm_dev_put(drm_dev);

	return ret;
}

static void ameba_drm_unbind(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);

	drm_dev_unregister(drm_dev);

	ameba_drm_kms_cleanup(drm_dev);

	drm_dev_put(drm_dev);

}

static const struct component_master_ops ameba_drm_ops = {
	.bind = ameba_drm_bind,
	.unbind = ameba_drm_unbind,
};

static int ameba_drm_platform_probe(struct platform_device *pdev)
{
	struct device_node 		*remote;
	struct component_match 	*match = NULL;
	struct device 			*dev = &pdev->dev;
	struct device_node 		*np = dev->of_node;

	remote = of_graph_get_remote_node(np, 0, 0);
	if (!remote) {
		return -ENODEV;
	}

	drm_of_component_match_add(dev, &match, compare_of, remote);
	of_node_put(remote);

	return component_master_add_with_match(dev, &ameba_drm_ops, match);
}

static int ameba_drm_platform_remove(struct platform_device *pdev)
{
	component_master_del(&pdev->dev, &ameba_drm_ops);

	return 0;
}

static const struct of_device_id ameba_drm_dt_ids[] = {
	{
		.compatible = "realtek,amebad2-drm",
		.data = (void *) &lcdc_driver_data,
	},
	{ /* end node */ },
};
MODULE_DEVICE_TABLE(of, ameba_drm_dt_ids);

static struct platform_driver ameba_drm_platform_driver = {
	.probe = ameba_drm_platform_probe,
	.remove = ameba_drm_platform_remove,
	.driver = {
		.name = "amebad2-drm",
		.of_match_table = ameba_drm_dt_ids,
	},
};

module_platform_driver(ameba_drm_platform_driver);

MODULE_AUTHOR("Chunlin.Yi <chunlin.yi@realsil.com.cn>");
MODULE_DESCRIPTION("realtek AmebaD2 SoCs' DRM driver");
MODULE_LICENSE("GPL v2");
