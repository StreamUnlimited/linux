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

#define ENABLE_LCDC_CTL (1)
//#undef ENABLE_LCDC_CTL

#define ENABLE_MIPI_CTL (1)
//#undef ENABLE_MIPI_CTL


#define  LCDC_LAYER_IMG_FORMAT_NOT_SUPPORT (LCDC_LAYER_IMG_FORMAT_ARGB8666+1)

#define to_ameba_crtc(crtc) container_of(crtc, struct ameba_crtc, base)

#define to_ameba_plane(plane) container_of(plane, struct ameba_plane, base)

/* ameba-format translate table */
struct ameba_format {
	u32 pixel_format;
	u32 hw_format;
};

struct ameba_crtc {
	struct drm_crtc base;
	//struct lcdc_hw_ctx_type 
	void *lcdc_hw_ctx;
	bool enable;

	//vblink issue
	struct hrtimer vblank_hrtimer;
	ktime_t period_ns;	
	/* protects concurrent access to composer */
//	spinlock_t lock;
};

struct ameba_plane {
	struct drm_plane base;
	//struct lcdc_hw_ctx_type 
	void *lcdc_hw_ctx;
	u32 ch;///layer index

//	bool enable;
};

/* display controller init/cleanup ops */
struct ameba_drm_data {
	const u32 *channel_formats;
	u32 channel_formats_cnt;
	int config_max_width;
	int config_max_height;
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
	void (*cleanup_hw_ctx)(void *hw_ctx);
};

//
void* drm_get_param_handle(void);
bool drm_get_dsi_flag(void);
bool drm_set_dsi_flag(bool);
void drm_underflow_flag_add(unsigned char value);
void drm_underflow_flag_reset(void);
unsigned char drm_get_underflow_flag(void);

//mipi
void drm_set_mipidsi_param(void *data);
void* drm_get_mipidsi_param(void);
void* drm_get_mipidsi_param2(void* data);
void mipi_dsi_underflow_reset(void);

//lcdc
void drm_set_lcdc_param(void *data);
void* drm_get_lcdc_param(void);
void lcdc_underflow_reset(void *data);


#endif /* __KIRIN_DRM_DRV_H__ */
