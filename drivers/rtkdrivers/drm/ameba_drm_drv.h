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

#ifndef __KIRIN_DRM_DRV_H__
#define __KIRIN_DRM_DRV_H__

#include <drm/drm_gem.h>
#include "ameba_drm_util.h"


#define  LCDC_LAYER_IMG_FORMAT_NOT_SUPPORT (LCDC_LAYER_IMG_FORMAT_ARGB8666+1)

#define to_ameba_crtc(crtc) container_of(crtc, struct ameba_crtc, base)

#define to_ameba_plane(plane) container_of(plane, struct ameba_plane, base)

/* ameba-format translate table */
struct ameba_format {
	u32 pixel_bpp;
	u32 pixel_format;
	u32 hw_format;
};

struct ameba_crtc {
	struct drm_crtc base;

	void *lcdc_hw_ctx;	//struct lcdc_hw_ctx_type 
	bool enable;
};

struct ameba_plane {
	struct drm_plane base;
	
	void *lcdc_hw_ctx;	//struct lcdc_hw_ctx_type 
	u32 ch;	///layer index
};

//saved in struct drm_device->dev_private
struct ameba_drm_private {
	struct ameba_crtc crtc;
	struct ameba_plane planes[LCDC_LAYER_MAX_NUM];

	void *lcdc_hw_ctx; //struct lcdc_hw_ctx_type handle
};


/* display controller init/cleanup ops */
struct ameba_drm_data {
	const u32 *channel_formats;
	u32 channel_formats_cnt;
	bool register_connects;
	u32 num_planes;
	u32 prim_plane;

	struct drm_driver *driver;
	const struct drm_crtc_helper_funcs *crtc_helper_funcs;
	const struct drm_crtc_funcs *crtc_funcs;
	const struct drm_plane_helper_funcs *plane_helper_funcs;
	const struct drm_plane_funcs  *plane_funcs;
	const struct drm_mode_config_funcs *mode_config_funcs;

	void *(*alloc_hw_ctx)(struct platform_device *pdev,
			      struct drm_crtc *crtc);

	void (*cleanup_hw_ctx)(struct platform_device *pdev, void *hw_ctx);
};

//
void* drm_get_param_handle(void);
void drm_underflow_flag_add(unsigned char value);
void drm_underflow_flag_reset(void);
u32 drm_get_underflow_flag(void);

//mipi
void drm_set_mipidsi_param(void *data);
void* drm_get_mipidsi_param(void);
void* drm_get_mipidsi_param2(void* data);
void mipi_dsi_underflow_reset(void);

//lcdc
void drm_set_lcdc_param(void *data);
void* drm_get_lcdc_param(void);
void lcdc_underflow_reset(void *data);

//dts
int ameba_drm_get_display_width(void);
int ameba_drm_get_display_height(void);
int ameba_drm_get_framerate(void);


#endif /* __KIRIN_DRM_DRV_H__ */
