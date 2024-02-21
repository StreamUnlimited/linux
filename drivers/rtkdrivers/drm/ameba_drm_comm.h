// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek DRM support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#ifndef _AMEBAD2_DRM_BASE_COMMON_H_
#define _AMEBAD2_DRM_BASE_COMMON_H_

#include "ameba_drm_base.h"
#include "ameba_lcdc.h"
#include "ameba_mipi.h"


/*
	APIs define
*/
void DelayUs(u32 time);


//lcdc api
/*
	1. lcdc reset 
	2. lcdc update all info  ; such as  plan size/color ; 
	3. update layer details , such as img , format , disable/enable 
	4. do init to update to reg
	5. update dma info 
	6. able/disable dma debug info 
	7. lcdc enable/disable ,
	8. lcdc ir enable/disable
	9. lcdc ir set posi
*/
//lcdc global reg
u32 ameba_lcdc_reg_read(void __iomem *address);
void ameba_lcdc_reg_write(void __iomem *address,u32 Value32);

void LcdcDumpRegValue(struct device *dev, void __iomem * address,const char* filename);

void ameba_lcdc_enable(void __iomem * address,u32 NewState);
u32 ameba_lcdc_get_irqstatus(void __iomem * address);
void ameba_lcdc_clean_irqstatus(void __iomem * address,u32 irq);

void ameba_lcdc_reset_config(LCDC_InitTypeDef *LCDC_InitStruct, u16 widht, u16 height,u32 bgcolor);
void ameba_lcdc_set_planesize(LCDC_InitTypeDef *LCDC_InitStruct,u16 widht,u16 height);
void ameba_lcdc_set_background_color(LCDC_InitTypeDef *LCDC_InitStruct,u32 bgcolor);
void ameba_lcdc_config_setvalid(void __iomem * address,LCDC_InitTypeDef *LCDC_InitStruct);
void ameba_lcdc_enable_SHW(void __iomem * address);

//dma issue
void ameba_lcdc_dma_config_bustsize(void __iomem * address,u32 BurstSize);
void ameba_lcdc_dma_config_keeplastFrm(void __iomem * address,u32 DmaUnFlwMode, u32 showData);
void ameba_lcdc_dma_debug_config(void __iomem * address,u32 writeBackFlag, u32 ImgDestAddr);
void ameba_lcdc_dma_get_unint_cnt(void __iomem * address,u32* DmaUnIntCnt);

//lcdc irq issue
void ameba_lcdc_irq_enable(void __iomem * address,u32 LCDC_IT, u32 NewState);
void ameba_lcdc_irq_linepos(void __iomem * address,u32 LineNum);
void ameba_lcdc_irq_config(void __iomem * address,u32 intType, u32 NewState);


//layer reg
void ameba_lcdc_update_layer_reg(void __iomem * address,u8 layid,LCDC_LayerConfigTypeDef *EachLayer);
void ameba_lcdc_layer_enable(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u8 able);
void ameba_lcdc_layer_imgfmt(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u32 fmt);
void ameba_lcdc_layer_imgaddress(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u32 imgaddress);
void ameba_lcdc_layer_pos(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u16 start_x,u16 stop_x,u16 start_y,u16 stop_y);
void ameba_lcdc_layer_colorkey_enable(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u8 able);
void ameba_lcdc_layer_colorkey_value(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u32 colorkey);
void ameba_lcdc_layer_blend_value(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u8 blend);
void ameba_lcdc_layer_alpha_value(LCDC_InitTypeDef *LCDC_InitStruct,u8 layid,u8 alpha);

void mipi_lcdc_enable(void __iomem *address, u32 NewState);
void ameba_lcdc_reenable(void __iomem* plcdc_reg);

//mipi dis api
void MipiDumpRegValue(struct device *dev, void __iomem * address);
void MipiDsi_Do_Init(void __iomem *MIPIx, MIPI_InitTypeDef *MIPI_InitStruct,u32* txdone,u32 *rxcmd,void *init_table);
void MIPI_InitStruct_Config(struct device *dev, MIPI_InitTypeDef *MIPI_InitStruct,u32 width,u32 height,u32 framerate, u32 *mipi_ckd);
void MIPI_DPHY_Reset(void __iomem *MIPIx);

void mipi_dsi_underflow_reset(void __iomem *pmipi_reg);


#endif  /*_AMEBAD2_DRM_BASE_COMMON_H_*/
