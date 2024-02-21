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

struct ameba_drm_reg_t {
	void __iomem *reg;
};

//drv main struct
struct ameba_drm_struct {
	struct device           *dev;
	struct drm_device       *drm;
	struct device           *dsi_dev;       /* dsi dev handle,used for handle underflow */

	void                    *ameba_drm_priv;
	struct drm_crtc         *crtc;
	//struct drm_atomic_state *state;

	//struct drm_panel *panel;
	//struct mipi_dsi_device *mdsi;

	u32                     display_width ;
	u32                     display_height ;
	u32                     display_framerate;

	u32                     under_flow_count;   /* total underflow count */
	volatile u32            under_flow_flag;    /* one time underflow count */

	void                    *lcdc_hw_ctx;        /* struct lcdc_hw_ctx_type handle */
};


#endif /* __KIRIN_DRM_DRV_H__ */
