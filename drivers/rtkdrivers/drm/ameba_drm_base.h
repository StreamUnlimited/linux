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


//dsi issue
#define MIPI_DISPLAY_X	 			(480)
#define MIPI_DISPLAY_Y	 			(800)
#define LDCD_PIXFORMAT_MAX_LEN		(32/8)

#define LCDC_TEST_IMG_BUF_X			(MIPI_DISPLAY_X)
#define LCDC_TEST_IMG_BUF_Y			(MIPI_DISPLAY_Y)
#define LDCD_XRES_MAX				(MIPI_DISPLAY_X)
#define LDCD_YRES_MAX				(MIPI_DISPLAY_Y)


#ifndef LCDC_BACKGROUND   ////BGR
#define LCDC_BACKGROUND			0xFFFFFFFF
#endif

#define DMA_ERROR_BACKGROUND			0x0000FF00


#define assert_param(expr) ((expr) ? (void)0 : printk("assert issue:%s,%d",__func__,__LINE__))

#define AMEBA_LCDC_DEBUG
#undef AMEBA_LCDC_DEBUG

#ifdef AMEBA_LCDC_DEBUG
#include <linux/timex.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

#define DRM_LCDC_IN()       {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][LCDC]enter %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#define DRM_LCDC_TEST()     {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][LCDC]here %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#define DRM_LCDC_OUT()      {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][LCDC]exit  %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#define DRM_MIPI_IN()       {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][MIPI]enter %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#define DRM_MIPI_TEST()     {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][MIPI]here %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#define DRM_MIPI_OUT()      {ktime_t time_start = ktime_get();printk(KERN_DEBUG"[%lld][MIPI]exit  %s-%d \n",ktime_to_us(time_start),__func__,__LINE__);}
#else
#define DRM_LCDC_IN()       {printk(KERN_DEBUG"[LCDC]enter %s-%d \n",__func__,__LINE__);}
#define DRM_LCDC_TEST()     {printk(KERN_DEBUG"[LCDC]here %s-%d \n",__func__,__LINE__);}
#define DRM_LCDC_OUT()      {printk(KERN_DEBUG"[LCDC]exit  %s-%d \n",__func__,__LINE__);}
#define DRM_MIPI_IN()       {printk(KERN_DEBUG"[MIPI]enter %s-%d \n",__func__,__LINE__);}
#define DRM_MIPI_TEST()     {printk(KERN_DEBUG"[MIPI]here %s-%d \n",__func__,__LINE__);}
#define DRM_MIPI_OUT()      {printk(KERN_DEBUG"[MIPI]exit  %s-%d \n",__func__,__LINE__);}
#endif
#define DRM_TEST_PRINTK(...)  printk(KERN_DEBUG __VA_ARGS__)


#endif  /*_AMEBAD2_DRM_BASE_H_*/
