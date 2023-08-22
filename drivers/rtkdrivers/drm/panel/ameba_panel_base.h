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

#ifndef _AMEBAD2_DRM_PANEL_BASE_H_
#define _AMEBAD2_DRM_PANEL_BASE_H_

#include "../ameba_drm_base.h"

typedef struct LCM_setting_table {
	u8                      cmd;
	u8                      count;
	u8                      para_list[128];
} LCM_setting_table_t;

//make sure below cmd is not conflict with the cmd that send to DSI
#define REGFLAG_DELAY                           0xFC        /* RUN DELAY */
#define REGFLAG_END_OF_TABLE                    0xFD        /* END OF REGISTERS MARKER */

struct ameba_drm_panel_struct {
	void                    *panel_priv;
	LCM_setting_table_t     *init_table;
};

#endif  /*_AMEBAD2_DRM_PANEL_BASE_H_*/
