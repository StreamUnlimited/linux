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

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/of_reserved_mem.h>
#include <video/display_timing.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_gem_cma_helper.h>
//#include <drm/drm_plane_helper.h>
//#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>
#include <drm/drm_gem_framebuffer_helper.h>

#include "ameba_drm_drv.h"
#include "ameba_drm_util.h"

#define CHECK_PARAM_RETURN_VALUE(a,b)   {if(0 == (u32)a){printk(KERN_DEBUG" %s-%d return !\n",__func__,__LINE__); return b;}}
#define CHECK_PARAM_RETURN(a)           {if(0 == (u32)a){printk(KERN_DEBUG" %s-%d return !\n",__func__,__LINE__); return ;}}


struct ameba_buf_struct {
	u32 layer_address;

	//used to copy window from fb to show
	bool  use_sec_buffer ;
	void *sec_vaddr;
	dma_addr_t sec_paddr;
	size_t size ;
};
static struct ameba_buf_struct sec_layer_info[LCDC_LAYER_MAX_NUM]={0,};

struct lcdc_hw_ctx_type {

	//lcdc ctl register and struct instance
	LCDC_InitTypeDef lcdc_initstancd;
	void __iomem *reg_base_addr;

	u32 ldcd_writeback_buffer_length;
	u32 ldcd_writeback_buffer ;
	dma_addr_t dump_paddr;

	int irq;

	struct clk *clock;

	///point to drv->private->crtc info , defined in ameba_drm_drv.c
	struct drm_crtc *crtc;
};

void lcdc_underflow_reset(void *data)
{
	struct lcdc_hw_ctx_type *lcdc_ctx = (struct lcdc_hw_ctx_type *)data ;
	void __iomem* plcdc_reg = lcdc_ctx->reg_base_addr;

	//todo
	LCDC_Cmd(plcdc_reg, DISABLE);
	printk(KERN_DEBUG"Reset lcdc...\n");

	LCDC_Cmd(plcdc_reg, ENABLE);
	while (!LCDC_CheckLCDCReady(plcdc_reg));
}


///defined in drm_fourcc.c :: drm_mode_legacy_fb_format
static const struct ameba_format ameba_formats[] = {
	/* 16bpp RGB: */
	{ DRM_FORMAT_RGB565, LCDC_LAYER_IMG_FORMAT_RGB565 },
	/* 32bpp [A]RGB: */
	{ DRM_FORMAT_ARGB8888, LCDC_LAYER_IMG_FORMAT_ARGB8888 },
	/* 16bpp ARGB: */
	{ DRM_FORMAT_XRGB1555, LCDC_LAYER_IMG_FORMAT_ARGB1555 },
	/* 24bpp RGB: */
	{ DRM_FORMAT_RGB888, LCDC_LAYER_IMG_FORMAT_RGB888 },
};
static const u32 ameba_channel_formats[] = {
	DRM_FORMAT_RGB565,
	DRM_FORMAT_ARGB8888,

	DRM_FORMAT_XRGB1555,
	DRM_FORMAT_RGB888
};
//get bytes from pixformat
static u32 getBppFromFormat(u32 pixformat)
{
	u32 bpp = 4 ;

	switch (pixformat) {
	case LCDC_LAYER_IMG_FORMAT_ARGB1555:
	case LCDC_LAYER_IMG_FORMAT_RGB565:
		bpp = 2 ;
		break;
	case LCDC_LAYER_IMG_FORMAT_RGB888:
		bpp = 3;
		break;
	case LCDC_LAYER_IMG_FORMAT_ARGB8888:
	default:
		bpp = 4;
		break;
	}
	return bpp ;
}

/* convert from fourcc format to lcdc format */
static u32 ameba_get_format(u32 pixel_format)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(ameba_formats); i++)
		if (ameba_formats[i].pixel_format == pixel_format) {
			return ameba_formats[i].hw_format;
		}

	/* not found */
	DRM_ERROR("Not found pixel format!!fourcc_format= 0x%x\n", pixel_format);
	return LCDC_LAYER_IMG_FORMAT_NOT_SUPPORT;
}
static bool ameba_crtc_mode_fixup(struct drm_crtc *crtc,
								  const struct drm_display_mode *mode,
								  struct drm_display_mode *adjusted_mode)
{
	return true;
}

//while video frame done, should emit this event
//todo @20210917
static int ameba_crtc_enable_vblank(struct drm_crtc *crtc)
{
	struct ameba_crtc *kcrtc  ;
	struct lcdc_hw_ctx_type *lcdc_ctx ;
	DRM_LCDC_IN();
#ifdef ENABLE_LCDC_CTL
	CHECK_PARAM_RETURN_VALUE(crtc, 0);
	kcrtc = to_ameba_crtc(crtc);
	CHECK_PARAM_RETURN_VALUE(kcrtc, 0);
	lcdc_ctx = kcrtc->lcdc_hw_ctx;
	CHECK_PARAM_RETURN_VALUE(lcdc_ctx, 0);
	ameba_lcdc_irq_enable(lcdc_ctx->reg_base_addr, LCDC_BIT_LCD_LIN_INTEN, ENABLE);
	ameba_lcdc_enable_SHW(lcdc_ctx->reg_base_addr);
#endif

	DRM_LCDC_OUT();
	return 0 ;
}

static void ameba_crtc_disable_vblank(struct drm_crtc *crtc)
{
	struct ameba_crtc *kcrtc  ;
	struct lcdc_hw_ctx_type *lcdc_ctx ;
	DRM_LCDC_IN();

#ifdef ENABLE_LCDC_CTL
	CHECK_PARAM_RETURN(crtc);
	kcrtc = to_ameba_crtc(crtc);
	CHECK_PARAM_RETURN(kcrtc);
	lcdc_ctx = kcrtc->lcdc_hw_ctx;
	CHECK_PARAM_RETURN(lcdc_ctx);
	ameba_lcdc_irq_enable(lcdc_ctx->reg_base_addr, LCDC_BIT_LCD_LIN_INTEN, DISABLE);
	ameba_lcdc_enable_SHW(lcdc_ctx->reg_base_addr);
#endif

	DRM_LCDC_OUT();
}

static irqreturn_t ameba_irq_handler(int irq, void *data)
{
	/* vblank irq */
	struct drm_crtc *crtc = (struct drm_crtc *)data ;
	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
	struct lcdc_hw_ctx_type *lcdc_ctx = kcrtc->lcdc_hw_ctx;
	u32 irq_status = ameba_lcdc_get_irqstatus(lcdc_ctx->reg_base_addr) ;
	//DRM_TEST_PRINTK("here %s-%d [irq_status=0x%x-0x%x]\n",__func__,__LINE__,irq_status,LCDC_BIT_LCD_LIN_INTS);
	if (irq_status & LCDC_BIT_LCD_LIN_INTS) {
		ameba_lcdc_clean_irqstatus(lcdc_ctx->reg_base_addr, LCDC_BIT_LCD_LIN_INTS);

		///send out vblank msg
		drm_crtc_handle_vblank(crtc);
	}

	if (irq_status & LCDC_BIT_DMA_UN_INTS) {
		drm_underflow_flag_add(1);
		if (drm_get_underflow_flag() == 1) {
			printk(KERN_DEBUG"DMA Under-----\n");
			mipi_dsi_underflow_reset();
		}
	}

	return IRQ_HANDLED;
}

static void ameba_display_enable(struct lcdc_hw_ctx_type *ctx)
{
	void __iomem *base ;
	DRM_LCDC_IN();
#ifdef ENABLE_LCDC_CTL
	CHECK_PARAM_RETURN(ctx);
	base = ctx->reg_base_addr;
	CHECK_PARAM_RETURN(base);
	ameba_lcdc_enable(base, ENABLE);
	//ameba_lcdc_config_setvalid(base, &(ctx->lcdc_initstancd));
	//DRM_TEST_PRINTK("[LCDC]here lcdc enable success  %s-%d \n", __func__, __LINE__);
	ameba_lcdc_enable_SHW(base);
#endif
	DRM_LCDC_OUT();
}
static void ameba_display_disable(struct lcdc_hw_ctx_type *ctx)
{
	/*
		should disable mipi first , then disable lcdc
	*/
	void __iomem *base ;
	DRM_LCDC_IN();
	CHECK_PARAM_RETURN(ctx);
	base = ctx->reg_base_addr;
	CHECK_PARAM_RETURN(base);
	ameba_lcdc_enable(base, DISABLE);
	ameba_lcdc_enable_SHW(base);
	DRM_LCDC_OUT();
}

static inline void ameba_dump_regs(struct lcdc_hw_ctx_type *ctx)
{
#if  0
	return ;
#else
#ifdef ENABLE_LCDC_CTL
	LcdcDumpRegValue(ctx->reg_base_addr, "ameba_dump_regs");
#endif
#endif
}

static void ameba_crtc_atomic_enable(struct drm_crtc *crtc,
									 struct drm_crtc_state *old_state)
{
	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
	struct lcdc_hw_ctx_type *ctx = kcrtc->lcdc_hw_ctx;
	DRM_LCDC_IN();
	if (kcrtc->enable) {
		return;
	}

	ameba_display_enable(ctx);
	ameba_dump_regs(ctx);
	drm_crtc_vblank_on(crtc);
	kcrtc->enable = true;
	DRM_LCDC_OUT();
}

static void ameba_crtc_atomic_disable(struct drm_crtc *crtc,
									  struct drm_crtc_state *old_state)
{
	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
	struct lcdc_hw_ctx_type *ctx = kcrtc->lcdc_hw_ctx;
	DRM_LCDC_IN();
	if (!kcrtc->enable) {
		DRM_LCDC_OUT();
		return;
	}

	drm_crtc_vblank_off(crtc);
	ameba_display_disable(ctx);
	kcrtc->enable = false;
	DRM_LCDC_OUT();
}
/*
	This callback is used to update the display mode of a CRTC without
	* changing anything of the primary plane configuration
*/
static void ameba_crtc_mode_set_nofb(struct drm_crtc *crtc)
{
//	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
//	struct lcdc_hw_ctx_type *lcdc_ctx = kcrtc->lcdc_hw_ctx;

	DRM_LCDC_IN();
	DRM_LCDC_OUT();
}
/*
static void ameba_crtc_atomic_begin(struct drm_crtc *crtc,
				  struct drm_crtc_state *old_state)
{
	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
	struct lcdc_hw_ctx_type *lcdc_ctx = kcrtc->lcdc_hw_ctx;
	DRM_LCDC_IN();
	DRM_LCDC_OUT();
}
*/
static void ameba_crtc_atomic_flush(struct drm_crtc *crtc,
									struct drm_crtc_state *old_state)

{
	struct ameba_crtc *kcrtc = to_ameba_crtc(crtc);
	struct lcdc_hw_ctx_type *lcdc_ctx = kcrtc->lcdc_hw_ctx;

	struct drm_pending_vblank_event *event = crtc->state->event;
	DRM_LCDC_IN();

	/* only crtc is enabled regs take effect */
	if (kcrtc->enable) {
		/* flush lcdc registers */
		ameba_lcdc_enable(lcdc_ctx->reg_base_addr, ENABLE);
		//ameba_display_enable(lcdc_ctx);

		ameba_dump_regs(lcdc_ctx);
	}
	DRM_LCDC_TEST();
	if (event) {
		crtc->state->event = NULL;

		DRM_TEST_PRINTK("here %s-%d [type=0x%x]\n", __func__, __LINE__, event->event.base.type);
		spin_lock_irq(&crtc->dev->event_lock);
		if (drm_crtc_vblank_get(crtc) == 0) {
			DRM_TEST_PRINTK("here %s-%d [send vblank event]\n", __func__, __LINE__);
			drm_crtc_arm_vblank_event(crtc, event);
		} else {
			DRM_TEST_PRINTK("here %s-%d [send vblank event]\n", __func__, __LINE__);
			drm_crtc_send_vblank_event(crtc, event);
		}
		spin_unlock_irq(&crtc->dev->event_lock);
	}

	DRM_LCDC_OUT();
}

static int ameba_drm_atomic_helper_set_config(struct drm_mode_set *set,
		struct drm_modeset_acquire_ctx *ctx)
{
	int ret = 0;

	DRM_LCDC_IN();
	ret = drm_atomic_helper_set_config(set, ctx);
	DRM_LCDC_OUT();

	return ret ;
}

static const struct drm_crtc_helper_funcs ameba_crtc_helper_funcs = {
	.mode_fixup	= ameba_crtc_mode_fixup,//done
	.mode_set_nofb	= ameba_crtc_mode_set_nofb,//done
	//.atomic_begin	= ameba_crtc_atomic_begin,
	.atomic_flush	= ameba_crtc_atomic_flush,//done
	.atomic_enable	= ameba_crtc_atomic_enable,//done
	.atomic_disable	= ameba_crtc_atomic_disable,//done
};

static const struct drm_crtc_funcs ameba_crtc_funcs = {
	.destroy	= drm_crtc_cleanup,

	//set config  drmModeSetCrtc will call this function,
	//it make all the link connect success
	.set_config	= ameba_drm_atomic_helper_set_config,
	//drmModePageFlip will call this function
	.page_flip	= drm_atomic_helper_page_flip,

	.reset		= drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state	= drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_crtc_destroy_state,

	//todo support vblank
	.enable_vblank	= ameba_crtc_enable_vblank,//
	.disable_vblank	= ameba_crtc_disable_vblank,//
};

static int ameba_plane_atomic_check(struct drm_plane *plane,
									struct drm_plane_state *state)
{
	struct drm_framebuffer *fb = state->fb;
	struct drm_crtc *crtc = state->crtc;
	struct drm_crtc_state *crtc_state;
	u32 src_x, src_y, src_w, src_h, crtc_w, crtc_h, fmt;
	int crtc_x, crtc_y ;
	DRM_LCDC_IN();
#ifdef ENABLE_LCDC_CTL
	src_x = state->src_x >> 16;
	src_y = state->src_y >> 16;
	src_w = state->src_w >> 16;
	src_h = state->src_h >> 16;
	crtc_x = state->crtc_x;
	crtc_y = state->crtc_y;
	crtc_w = state->crtc_w;
	crtc_h = state->crtc_h;

	if (!crtc || !fb) {
		DRM_LCDC_OUT();
		return 0;
	}
	//printk("fb = 0x%x crtc = 0x%x \n", fb, crtc);
	DRM_LCDC_TEST();
	fmt = ameba_get_format(fb->format->format);
	if (fmt >= LCDC_LAYER_IMG_FORMAT_NOT_SUPPORT) {
		return -EINVAL;
	}
	DRM_LCDC_TEST();
	crtc_state = drm_atomic_get_crtc_state(state->state, crtc);
	if (IS_ERR(crtc_state)) {
		return PTR_ERR(crtc_state);
	}
	DRM_LCDC_TEST();
	if (src_w != crtc_w || src_h != crtc_h) {
		return -EINVAL;
	}

	if (src_x + src_w > fb->width ||
		src_y + src_h > fb->height) {
		return -EINVAL;
	}

	if (crtc_x < 0 || crtc_y < 0) {
		return -EINVAL;
	}

	if (crtc_x + crtc_w > crtc_state->adjusted_mode.hdisplay ||
		crtc_y + crtc_h > crtc_state->adjusted_mode.vdisplay) {
		return -EINVAL;
	}
#endif
	DRM_LCDC_OUT();
	return 0;
}

/*
static struct drm_gem_cma_object *
ameba_drm_gem_cma_create_handle(struct drm_device *drm,
	u32 height,u32 width,u32 bpp)
{
	struct drm_gem_cma_object *cma_obj;

	height=800;
	width=480 ;
	cma_obj = drm_gem_cma_create(drm, height*width*bpp);
	if (IS_ERR(cma_obj))
	  return cma_obj;

	return cma_obj ;
}
*/

/*
* Typicaly, a channel looks like: DMA-->clip-->scale-->ctrans-->compositor
*
* This hook is called in-between the &drm_crtc_helper_funcs.atomic_begin and
* drm_crtc_helper_funcs.atomic_flush callbacks.
*/
static void ameba_plane_atomic_update(struct drm_plane *plane,
									  struct drm_plane_state *old_state)
{
	struct drm_plane_state *state = plane->state;
	u32 fmt;
	struct ameba_plane *kplane;
	struct lcdc_hw_ctx_type  *lcdc_local ;
	LCDC_InitTypeDef *lcdc_tmp ;
	u8 idx;
	int dest_x, dest_y, dest_w, dest_h;
	struct drm_framebuffer *fb ;
	struct drm_gem_cma_object *rk_obj ;
	DRM_LCDC_IN();
#ifdef ENABLE_LCDC_CTL
	CHECK_PARAM_RETURN(state);
	kplane = to_ameba_plane(plane);
	CHECK_PARAM_RETURN(kplane);
	lcdc_local = (struct lcdc_hw_ctx_type *)(kplane->lcdc_hw_ctx);
	CHECK_PARAM_RETURN(lcdc_local);
	lcdc_tmp = &(lcdc_local->lcdc_initstancd) ;
	idx = kplane->ch ;
	fb = state->fb ;

	ameba_dump_regs(lcdc_local);
	CHECK_PARAM_RETURN(fb);
	//rk_obj = drm_fb_cma_get_gem_obj(fb, kplane->ch);
	rk_obj = drm_fb_cma_get_gem_obj(fb, 0);
	CHECK_PARAM_RETURN(rk_obj);

	fmt = ameba_get_format(fb->format->format);
	DRM_TEST_PRINTK("here %s-%d[id=%d,fmt=0x%x/%d,[%d*%d],vaddr=0x%x][%d-%d-%d-%d][%d-%d-%d-%d] \n", __func__, __LINE__,
					idx, fb->format->format, fmt, fb->width, fb->height,
					(u32)rk_obj->vaddr,
					state->crtc_x, state->crtc_y, state->crtc_w, state->crtc_h,
					state->src_x >> 16, state->src_y >> 16, state->src_w >> 16, state->src_h >> 16);

	///check the params , x <  MIPI_DISPLAY_X ; y < MIPI_DISPLAY_Y
	if (state->crtc_x >= MIPI_DISPLAY_X || state->crtc_y >= MIPI_DISPLAY_Y) {
		DRM_WARN("[error happen] %s-%d [start[%d-%d][%d-%d]\n", __func__, __LINE__,
				 state->crtc_x, state->crtc_y, MIPI_DISPLAY_X, MIPI_DISPLAY_Y);
		return ;
	}

	//display from 1~MIPI_DISPLAY_X
	dest_x = state->crtc_x ; //((state->crtc_x==0)?(1):(state->crtc_x));
	dest_y = state->crtc_y ; //((state->crtc_y==0)?(1):(state->crtc_y));

	{
		//check the data valid
		//crtc[] is the plane/layer position,
		//src[] is the source info , surface[0,0,w,h], copy the data from the fb[src_x,src_y,w,h] to [0,0,w,h]
		//src[src.x,src.y,dest.w,dest.h] , dest[crtc.x,crtc.y,dest.w,dest.h]

		//How to get the dest.w/dest.h
		//1.1 src.w > crtc.w , surface.w = crtc.w
		//1.2 src.w < crtc.w , surface.w = src.w
		//2.1 src.h > crtc.h , surface.h = crtc.h
		//2.2 src.h < crtc.h , surface.h = src.h

		//the desc w&h is the common place
		dest_w = ((state->crtc_w > state->src_w >> 16) ? (state->src_w >> 16) : (state->crtc_w));
		if (dest_x + dest_w > MIPI_DISPLAY_X) {
			dest_w = MIPI_DISPLAY_X - dest_x;
		}
		dest_h = ((state->crtc_h > state->src_h >> 16) ? (state->src_h >> 16) : (state->crtc_h));
		if (dest_y + dest_h > MIPI_DISPLAY_Y) {
			dest_h = MIPI_DISPLAY_Y - dest_y;
		}

		DRM_TEST_PRINTK("here %s-%d[%d-%d-%d-%d]fb->width=%d \n", __func__, __LINE__, dest_x, dest_y, dest_w, dest_h, fb->width);

		//from src_x&src_y  to  crtc_x & crtc_y
		//w not match , x not 0 ,
		//if y !=0, maybe just set a offset from the pixmap
		//dest_w != fb->width it is just a window
		//state->src_x!=0 or state->src_y !=0 , it is not from the first pixel
		if (dest_w != fb->width || state->src_x || state->src_y) {
			//should create a buffer, and do copy
			//1.use the global buffer that create in ameba_hw_ctx_alloc
			int y ;
			u32 bpp = getBppFromFormat(fmt);
			//2.do copy
			DRM_TEST_PRINTK("here %s-%d[%d-%d-%d-%d]fb->width=%d/bpp=%d \n", __func__, __LINE__, dest_x, dest_y, dest_w, dest_h, fb->width, bpp);
			for (y = 0; y < dest_h ; ++y) {
				memcpy((uint8_t *)sec_layer_info[idx].sec_vaddr + (y * dest_w * (bpp)),
					   (uint8_t *)rk_obj->vaddr + ((y + state->src_y) * (fb->width) * (bpp)) + ((bpp)*state->src_x),
					   dest_w * bpp);
			}
			sec_layer_info[idx].use_sec_buffer = 1;
		} else {
			sec_layer_info[idx].use_sec_buffer = 0 ;
		}
	}
	//else
	//{
	//	dest_w =state->crtc_w ;
	//	dest_h =state->crtc_h ;
	//	DRM_LCDC_TEST();
	//}

	//disable all layer TODO issue @20211022
	if (0) {
		u32 DmaUnIntCnt ;
		ameba_lcdc_dma_get_unint_cnt(lcdc_local->reg_base_addr, &DmaUnIntCnt);
		if (DmaUnIntCnt) {
			DRM_TEST_PRINTK("here %s-%d[DmaUnIntCnt=%d] \n", __func__, __LINE__, DmaUnIntCnt);
			//LCDC_DeInit(lcdc_local->reg_base_addr);
			//ameba_dump_regs(lcdc_local);
		}
	}
	DRM_LCDC_TEST();
	ameba_lcdc_layer_enable(lcdc_tmp, idx, ENABLE);
	ameba_lcdc_layer_imgfmt(lcdc_tmp, idx, fmt);
	ameba_lcdc_layer_colorkey_enable(lcdc_tmp, idx, DISABLE);

	if (sec_layer_info[idx].use_sec_buffer) {
		ameba_lcdc_layer_imgaddress(lcdc_tmp, idx, (u32)(sec_layer_info[idx].sec_paddr));
	} else {
		ameba_lcdc_layer_imgaddress(lcdc_tmp, idx, (u32)(rk_obj->paddr));
	}
	sec_layer_info[idx].layer_address = (u32)rk_obj->vaddr ;
	DRM_TEST_PRINTK("here %s-%d[%d-%d-%d-%d] \n", __func__, __LINE__, dest_x, dest_y, dest_w, dest_h);

	if (state->crtc_x <= 0) {
		dest_x = 1 ;
	}
	if (state->crtc_y <= 0) {
		dest_y = 1 ;
	}
	dest_w -= 1;
	dest_h -= 1;

	if (dest_x + dest_w > MIPI_DISPLAY_X) {
		dest_w = MIPI_DISPLAY_X - dest_x;
	}
	if (dest_y + dest_h > MIPI_DISPLAY_Y) {
		dest_h = MIPI_DISPLAY_Y - dest_y;
	}
	DRM_TEST_PRINTK("here %s-%d[%d-%d-%d-%d] \n", __func__, __LINE__, dest_x, dest_y, dest_x + dest_w, dest_y + dest_h);
	ameba_lcdc_layer_pos(lcdc_tmp, idx, dest_x, dest_x + dest_w, dest_y, dest_y + dest_h);
	//ameba_lcdc_layer_pos(lcdc_tmp,idx,1,MIPI_DISPLAY_X,1,MIPI_DISPLAY_Y);

	//ameba_lcdc_config_setvalid(lcdc_local->reg_base_addr, lcdc_tmp);
	ameba_lcdc_update_layer_reg(lcdc_local->reg_base_addr, idx, &(lcdc_tmp->layerx[idx]));
	ameba_lcdc_enable_SHW(lcdc_local->reg_base_addr);

	ameba_dump_regs(lcdc_local);
#endif
	DRM_LCDC_OUT();
}
/*
static void ameba_plane_atomic_disable(struct drm_plane *plane,
				     struct drm_plane_state *old_state)
{
	DRM_LCDC_IN();
#ifdef ENABLE_LCDC_CTL
	struct ameba_plane *kplane = to_ameba_plane(plane);
	struct lcdc_hw_ctx_type*  lcdc_local = (struct lcdc_hw_ctx_type*)(kplane->lcdc_hw_ctx);
	LCDC_InitTypeDef* lcdc_tmp = &(lcdc_local->lcdc_initstancd) ;

	ameba_lcdc_layer_enable(lcdc_tmp,kplane->ch,DISABLE);
	ameba_lcdc_config_setvalid(lcdc_local->reg_base_addr, lcdc_tmp);
#endif
	DRM_LCDC_OUT();
}
*/
static int ameba_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret ;
	DRM_LCDC_IN();
	ret = drm_gem_cma_mmap(filp, vma);
	DRM_LCDC_OUT();
	return ret ;
}


static const struct vm_operations_struct ameba_drm_gem_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};
static const struct file_operations ameba_drm_file_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.mmap		= ameba_drm_gem_mmap,//done
	.unlocked_ioctl = drm_ioctl,
	.compat_ioctl	= drm_compat_ioctl,
	.poll		= drm_poll,
	.read		= drm_read,
	.llseek 	= no_llseek,
	.release	= drm_release,
};
/*
static void  ameba_drm_release(struct drm_device *dev)
{
	struct vkms_device *vkms = container_of(dev, struct vkms_device, drm);

	platform_device_unregister(vkms->platform);
	drm_atomic_helper_shutdown(&vkms->drm);
	drm_mode_config_cleanup(&vkms->drm);
	drm_dev_fini(&vkms->drm);
	destroy_workqueue(vkms->output.composer_workq);
}
int ameba_drm_dumb_create(struct drm_file *file, struct drm_device *dev,
		     struct drm_mode_create_dumb *args)
{
	int ret ;
	DRM_LCDC_IN();

	ret = drm_gem_cma_dumb_create_internal(file, dev, args);

	DRM_LCDC_OUT();

	return PTR_ERR_OR_ZERO(ret);
}
*/
void ameba_drm_gem_free_object(struct drm_gem_object *gem_obj)
{
	DRM_LCDC_IN();
	if (1) { //for test
		struct drm_gem_cma_object *cma_obj;
		u8 i = 0 ;
		cma_obj = to_drm_gem_cma_obj(gem_obj);
		DRM_TEST_PRINTK("here %s-%d [v=0x%x / p=0x%x] \n", __func__, __LINE__, (u32)cma_obj->vaddr, (u32)cma_obj->paddr);
#ifdef ENABLE_LCDC_CTL
		for (i = 0 ; i < LCDC_LAYER_MAX_NUM ; i ++) {
			if (cma_obj->vaddr && ((u32)cma_obj->vaddr) == sec_layer_info[i].layer_address) {
				struct lcdc_hw_ctx_type  *lcdc_local = (struct lcdc_hw_ctx_type *)drm_get_mipidsi_param();
				LCDC_InitTypeDef *lcdc_tmp = &(lcdc_local->lcdc_initstancd) ;
				sec_layer_info[i].layer_address = 0 ;
				ameba_lcdc_layer_enable(lcdc_tmp, i, DISABLE);
				//ameba_lcdc_config_setvalid(lcdc_local->reg_base_addr, lcdc_tmp);
				ameba_lcdc_update_layer_reg(lcdc_local->reg_base_addr, i, &(lcdc_tmp->layerx[i]));
				ameba_lcdc_enable_SHW(lcdc_local->reg_base_addr);
				DRM_TEST_PRINTK("here %s-%d [destory %d layer] \n", __func__, __LINE__, i);
				/*
					fps=60 , shadow will be enable in 16.7ms,
					sleep 50ms ,lcdc will not use the buffer anymore
					it is safe to release the mem
				*/
				msleep(50);
				//if(lcdcInstance.sec_paddr){
				//drm_gem_cma_free_object(&(lcdcInstance.sec_cma_obj->base));
				//	lcdcInstance.sec_paddr = NULL ;
				//}
				LcdcDumpRegValue(lcdc_local->reg_base_addr, "ameba_drm_gem_free_object");
			}
		}
#endif
	}
	drm_gem_cma_free_object(gem_obj);

	DRM_LCDC_OUT();
}
static bool  ameba_drm_get_vblank_timestamp(struct drm_device *dev, unsigned int pipe,
									 int *max_error, ktime_t *vblank_time,
									 bool in_vblank_irq)
{
#if 0
	struct vkms_device *vkmsdev = drm_device_to_vkms_device(dev);
	struct vkms_output *output = &vkmsdev->output;
	struct drm_vblank_crtc *vblank = &dev->vblank[pipe];

	*vblank_time = output->vblank_hrtimer.node.expires;

	if (WARN_ON(*vblank_time == vblank->time)) {
		return true;
	}

	/*
	 * To prevent races we roll the hrtimer forward before we do any
	 * interrupt processing - this is how real hw works (the interrupt is
	 * only generated after all the vblank registers are updated) and what
	 * the vblank core expects. Therefore we need to always correct the
	 * timestampe by one frame.
	 */
	*vblank_time -= output->period_ns;
#endif
	return false;
}

/*
* @plane: plane object to update
* @crtc: owning CRTC of owning plane
* @fb: framebuffer to flip onto plane
* @crtc_x: x offset of primary plane on crtc
* @crtc_y: y offset of primary plane on crtc
* @crtc_w: width of primary plane rectangle on crtc
* @crtc_h: height of primary plane rectangle on crtc
* @src_x: x offset of @fb for panning
* @src_y: y offset of @fb for panning
* @src_w: width of source rectangle in @fb
* @src_h: height of source rectangle in @fb
* @ctx: lock acquire context
*
*/
static int ameba_drm_update_plane(struct drm_plane *plane,
								  struct drm_crtc *crtc,
								  struct drm_framebuffer *fb,
								  int crtc_x, int crtc_y,
								  unsigned int crtc_w, unsigned int crtc_h,
								  uint32_t src_x, uint32_t src_y,
								  uint32_t src_w, uint32_t src_h,
								  struct drm_modeset_acquire_ctx *ctx)
{
	return drm_atomic_helper_update_plane(plane, crtc, fb, crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h, ctx);
}

//plane
static const struct drm_plane_helper_funcs ameba_plane_helper_funcs = {
	.atomic_check = ameba_plane_atomic_check,//done
	.atomic_update = ameba_plane_atomic_update,//done
//	.atomic_disable = ameba_plane_atomic_disable,//done
};

static struct drm_plane_funcs ameba_plane_funcs = {
	//drmModeSetPlane will call this hook function
	.update_plane	= ameba_drm_update_plane,//done

	.disable_plane	= drm_atomic_helper_disable_plane,
	.destroy = drm_plane_cleanup,
	.reset = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};

static void *ameba_hw_ctx_alloc(struct platform_device *pdev,
								struct drm_crtc *crtc)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct lcdc_hw_ctx_type *lcdc_ctx = NULL;
	int ret;
	DRM_LCDC_IN();
	lcdc_ctx = devm_kzalloc(dev, sizeof(*lcdc_ctx), GFP_KERNEL);
	if (!lcdc_ctx) {
		DRM_ERROR("failed to alloc lcdc_hw_ctx\n");
		return ERR_PTR(-ENOMEM);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	lcdc_ctx->reg_base_addr = devm_ioremap_resource(&pdev->dev, res);
	if (0 == (lcdc_ctx->reg_base_addr)) {
		DRM_ERROR("failed to remap lcdc io base\n");
		return ERR_PTR(-ENODEV);
	}

	/* get RTK_CKE_LCDCMIPI  e:enable */
	lcdc_ctx->clock = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(lcdc_ctx->clock)) {
		dev_err(&pdev->dev, "Fail to get clock CKE_LCDCMIPI %d\n", __LINE__);
		return ERR_PTR(-ENODEV);
	}

	/* enable RTK_CKE_LCDCMIPI clock */
	ret = clk_prepare_enable(lcdc_ctx->clock);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to enable clock CKE_LCDCMIPI %d\n", ret);
		return ERR_PTR(-ENODEV);
	}

	lcdc_ctx->irq = platform_get_irq(pdev, 0);
	if (lcdc_ctx->irq < 0) {
		DRM_ERROR("failed to get irq\n");
		return ERR_PTR(-ENODEV);
	}
	/*
		irq init
	*/
	ret = devm_request_irq(dev, lcdc_ctx->irq, ameba_irq_handler, 0, dev_name(&pdev->dev), crtc);
	if (ret) {
		return ERR_PTR(-EIO);
	}

	/* Initialize reserved memory resources */
	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "Could not get reserved memory\n");
		return ERR_PTR(-ENODEV);
	}
/*
	lcdc_ctx->ldcd_writeback_buffer_length = (MIPI_DISPLAY_X * MIPI_DISPLAY_Y * LDCD_PIXFORMAT_MAX_LEN) ;
	//lcdc_ctx->ldcd_writeback_buffer = dma_alloc_coherent(&pdev->dev, lcdc_ctx->ldcd_writeback_buffer_length, &(lcdc_ctx->dump_paddr), GFP_KERNEL);
	lcdc_ctx->ldcd_writeback_buffer = 0 ;
	if (lcdc_ctx->ldcd_writeback_buffer) {
		dev_info(&pdev->dev, "Allocated coherent memory, vaddr: 0x%0llX, paddr: 0x%0llX[0x%x]\n",
				 (u64)lcdc_ctx->ldcd_writeback_buffer, (u64)lcdc_ctx->dump_paddr, (u32)lcdc_ctx->ldcd_writeback_buffer_length);
		memset((void *)lcdc_ctx->ldcd_writeback_buffer, 0x00, lcdc_ctx->ldcd_writeback_buffer_length);
	} else {
		lcdc_ctx->ldcd_writeback_buffer_length = 0 ;
	}
*/
	lcdc_ctx->ldcd_writeback_buffer = 0 ;
	lcdc_ctx->ldcd_writeback_buffer_length = 0 ;
	lcdc_ctx->crtc = crtc;

	//init the lcdc info
	//right now not set the buffer address
	ameba_lcdc_reset_config(&(lcdc_ctx->lcdc_initstancd));
	ameba_lcdc_set_planesize(&(lcdc_ctx->lcdc_initstancd), MIPI_DISPLAY_X, MIPI_DISPLAY_Y);
	ameba_lcdc_set_background_color(&(lcdc_ctx->lcdc_initstancd), LCDC_BACKGROUND);

	//dma read
	ameba_lcdc_dma_config_bustsize(lcdc_ctx->reg_base_addr, LCDC_LAYER_BURSTSIZE_4X64BYTES);
	//ToDo show last data, should set to 0 , 1 means errorData
	ameba_lcdc_dma_config_keeplastFrm(lcdc_ctx->reg_base_addr, 1, DMA_ERROR_BACKGROUND);

	/*line number*/
	ameba_lcdc_irq_linepos(lcdc_ctx->reg_base_addr, MIPI_DISPLAY_Y * 4 / 5);
	/*enbale LCDC LCDC_BIT_LCD_LIN_INTEN interrupt*/
	ameba_lcdc_irq_config(lcdc_ctx->reg_base_addr, LCDC_BIT_LCD_LIN_INTEN, ENABLE);

	//LCDC_Init(ctx->reg_base_addr, &(ctx->lcdc_initstancd));
	if (lcdc_ctx->ldcd_writeback_buffer) {
		ameba_lcdc_dma_debug_config(lcdc_ctx->reg_base_addr, LCDC_DMA_OUT_ENABLE, (u32)lcdc_ctx->ldcd_writeback_buffer);
	} else {
		ameba_lcdc_dma_debug_config(lcdc_ctx->reg_base_addr, LCDC_DMA_OUT_DISABLE, 0);
	}

	//set valid , disable all display , it should not display anything right now
	ameba_lcdc_config_setvalid(lcdc_ctx->reg_base_addr, &(lcdc_ctx->lcdc_initstancd));

	for (ret = 0 ; ret < LCDC_LAYER_MAX_NUM; ret ++) {
		struct ameba_buf_struct * layinfo = &(sec_layer_info[ret]);
		layinfo->layer_address = 0;
		layinfo->use_sec_buffer = 0 ;
		layinfo->size = round_up(MIPI_DISPLAY_X * MIPI_DISPLAY_Y * 4, PAGE_SIZE);
		layinfo->sec_vaddr = dma_alloc_coherent(&pdev->dev, layinfo->size, &(layinfo->sec_paddr), GFP_KERNEL);
	}
	drm_set_lcdc_param(lcdc_ctx);

	DRM_LCDC_OUT();
	return lcdc_ctx;
}

static void ameba_hw_ctx_cleanup(void *hw_ctx)
{
	DRM_LCDC_IN();
	DRM_LCDC_OUT();
}
static struct drm_framebuffer *
ameba_user_fb_create(struct drm_device *dev, struct drm_file *file_priv,
					 const struct drm_mode_fb_cmd2 *mode_cmd)
{
	//DRM_TEST_PRINTK("mode_cmd->pixel_format=%d\n",mode_cmd->pixel_format);
	DRM_LCDC_IN();
	if (ameba_get_format(mode_cmd->pixel_format) >= LCDC_LAYER_IMG_FORMAT_NOT_SUPPORT) {
		dev_err(dev->dev, "mode_cmd->pixel_formaterror happen , not support =%x\n", mode_cmd->pixel_format);
		return ERR_PTR(-EINVAL) ;
	}
	return drm_gem_fb_create(dev, file_priv, mode_cmd);
}

static int ameba_drm_atomic_helper_check(struct drm_device *dev,
		struct drm_atomic_state *state)
{
	int ret = 0;
	DRM_LCDC_IN();
	ret = drm_atomic_helper_check(dev, state);
	DRM_LCDC_OUT();
	return ret ;
}
static int ameba_drm_atomic_helper_commit(struct drm_device *dev,
		struct drm_atomic_state *state,
		bool nonblock)
{
	int ret = 0;
	DRM_LCDC_IN();
	ret = drm_atomic_helper_commit(dev, state, nonblock);
	DRM_LCDC_OUT();
	return ret ;
}

static const struct drm_mode_config_funcs ameba_mode_config_funcs = {
	.fb_create = ameba_user_fb_create,
	.atomic_check = ameba_drm_atomic_helper_check,
	.atomic_commit = ameba_drm_atomic_helper_commit,
};

static struct drm_driver ameba_lcdc_driver = {
	.driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,

//	.release		=  ameba_drm_release,
	.fops			= &ameba_drm_file_fops,

	//create gem & buffer
	.dumb_create	= drm_gem_cma_dumb_create_internal,
	.gem_vm_ops		= &ameba_drm_gem_vm_ops,
	.gem_free_object_unlocked = ameba_drm_gem_free_object,
	.get_vblank_timestamp	=  ameba_drm_get_vblank_timestamp,

#if 0  //need prime support : 
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,
	.gem_prime_get_sg_table = drm_gem_cma_prime_get_sg_table,
	.gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
	.gem_prime_vmap = drm_gem_cma_prime_vmap,
	.gem_prime_vunmap = drm_gem_cma_prime_vunmap,
	.gem_prime_mmap = drm_gem_cma_prime_mmap,
#endif

	.name = "amebad2",
	.desc = "Realtek AmebaD2 SoC DRM Driver",
	.date = "20210916",
	.major = 1,
	.minor = 0,
};

struct ameba_drm_data lcdc_driver_data = {
	.register_connects = false,
	.num_planes = LCDC_LAYER_MAX_NUM,
	.prim_plane = LCDC_LAYER_LAYER1,
	.channel_formats = ameba_channel_formats,
	.channel_formats_cnt = ARRAY_SIZE(ameba_channel_formats),
	.config_max_width = LDCD_XRES_MAX,
	.config_max_height = LDCD_YRES_MAX,
	.driver = &ameba_lcdc_driver,
	.crtc_helper_funcs = &ameba_crtc_helper_funcs,
	.crtc_funcs = &ameba_crtc_funcs,
	.plane_helper_funcs = &ameba_plane_helper_funcs,
	.plane_funcs = &ameba_plane_funcs,
	.mode_config_funcs = &ameba_mode_config_funcs,

	.alloc_hw_ctx = ameba_hw_ctx_alloc,
	.cleanup_hw_ctx = ameba_hw_ctx_cleanup,
};

