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

#ifndef _AMEBAD2_DRM_PANEL_PRIV_H_
#define _AMEBAD2_DRM_PANEL_PRIV_H_

struct ameba_panel_desc {
	struct device				*dev;
	LCM_setting_table_t 		*init_table;
	struct drm_display_mode 	*panel_module;
	struct drm_panel			panel;
	void						*priv;

	struct drm_panel_funcs		*rtk_panel_funcs;


	int(*init)(struct device *dev,struct ameba_panel_desc *priv_data);
	int(*deinit)(struct device *dev,struct ameba_panel_desc *priv_data);

};

static inline struct ameba_panel_desc *panel_to_desc(struct drm_panel *panel)
{
	return container_of(panel, struct ameba_panel_desc, panel);
}


#endif  /*_AMEBAD2_DRM_PANEL_PRIV_H_*/
