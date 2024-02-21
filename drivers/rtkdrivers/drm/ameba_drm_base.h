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

#ifndef _AMEBAD2_DRM_BASE_H_
#define _AMEBAD2_DRM_BASE_H_

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/io.h>
#include <drm/drm_print.h>


#ifndef ENABLE
#define   ENABLE    1
#endif

#ifndef DISABLE
#define   DISABLE   0
#endif

#define LCDC_MAX_REMOTE_DEV 				(2)

//dsi default value
#define LCDC_KG_COLOR                       0xFFFFFFFF
#define LCDC_UNDFLOW_COLOR                  0xFFFFFFFF

#define LCDC_LAY_COLOR_KEY                  0x00000000
#define LCDC_LAY_BLEND_ALPHA                0xff

//while disable lcdc ctl, mipi will show the pattern
#define ENABLE_LCDC_CTL                     (1)
//#undef ENABLE_LCDC_CTL 

#define UPDATE_DUMP_LDCD_REG                (10)


#define assert_param(expr)                  ((expr) ? (void)0 : printk("[DRM]assert issue:%s,%d",__func__,__LINE__))
#define AMEBA_DRM_DEBUG                     //DRM_INFO("%s Enter %d\n", __func__, __LINE__);

#endif  /*_AMEBAD2_DRM_BASE_H_*/
