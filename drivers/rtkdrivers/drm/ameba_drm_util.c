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
#include <linux/slab.h>
#include <linux/delay.h>
#include <video/mipi_display.h>

#include "ameba_drm_util.h"

#define MIPI_DSI_RTNI		2//4
#define MIPI_DSI_HSA		4
#define MIPI_DSI_HBP		30
#define MIPI_DSI_HFP		30

#define MIPI_DSI_VSA		5
#define MIPI_DSI_VBP		20
#define MIPI_DSI_VFP		15

#define Mhz			1000000UL
#define T_LPX		5
#define T_HS_PREP	6
#define T_HS_TRAIL	8
#define T_HS_EXIT	7
#define T_HS_ZERO	10

#define REGFLAG_DELAY						0xFC
#define REGFLAG_END_OF_TABLE				0xFD	// END OF REGISTERS MARKER

#define DUMP_REG(a,b)   b,readl(a + b)

/*
	struct define
*/
typedef struct {
	u8 alpha;
	u8 blue;
	u8 green;
	u8 red;
} rgb888_t;
typedef struct LCM_setting_table {
	u8 cmd;
	u8 count;
	u8 para_list[128];
} LCM_setting_table_t;

static LCM_setting_table_t lcm_initialization_setting[] = {/* DCS Write Long */
	/* ST7701S Reset Sequence */
	/* LCD_Nreset (1); Delayms (1); */
	/* LCD_Nreset (0); Delayms (1); */
	/* LCD_Nreset (1); Delayms (120); */
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},/* Delayms (120) */

	/* Bank0 Setting */
	/* Display Control setting */
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x10}},
	{0xC0, 2, {0x63, 0x00}},
	{0xC1, 2, {0x0C, 0x02}},
	{0xC2, 2, {0x31, 0x08}},
	{0xCC, 1, {0x10}},

	/* Gamma Cluster Setting */
	{0xB0, 16, {0x40, 0x02, 0x87, 0x0E, 0x15, 0x0A, 0x03, 0x0A, 0x0A, 0x18, 0x08, 0x16, 0x13, 0x07, 0x09, 0x19}},
	{0xB1, 16, {0x40, 0x01, 0x86, 0x0D, 0x13, 0x09, 0x03, 0x0A, 0x09, 0x1C, 0x09, 0x15, 0x13, 0x91, 0x16, 0x19}},
	/* End Gamma Setting */
	/* End Display Control setting */
	/* End Bank0 Setting */

	/* Bank1 Setting */
	/* Power Control Registers Initial */
	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}},
	{0xB0, 1, {0x4D}},

	/* Vcom Setting */
	{0xB1, 1, {0x64}},
	/* End End Vcom Setting */

	{0xB2, 1, {0x07}},
	{0xB3, 1, {0x80}},
	{0xB5, 1, {0x47}},
	{0xB7, 1, {0x85}},
	{0xB8, 1, {0x21}},
	{0xB9, 1, {0x10}},
	{0xC1, 1, {0x78}},
	{0xC2, 1, {0x78}},
	{0xD0, 1, {0x88}},
	/* End Power Control Registers Initial */
	{REGFLAG_DELAY, 100, {}},/* Delayms (100) */
	/* GIP Setting */
	{0xE0, 3, {0x00, 0x84, 0x02}},
	{0xE1, 11, {0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20}},
	{0xE2, 13, {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xE3, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE4, 2, {0x44, 0x44}},
	{0xE5, 16, {0x09, 0x31, 0xBE, 0xA0, 0x0B, 0x31, 0xBE, 0xA0, 0x05, 0x31, 0xBE, 0xA0, 0x07, 0x31, 0xBE, 0xA0}},
	{0xE6, 4, {0x00, 0x00, 0x33, 0x33}},
	{0xE7, 2, {0x44, 0x44}},
	{0xE8, 16, {0x08, 0x31, 0xBE, 0xA0, 0x0A, 0x31, 0xBE, 0xA0, 0x04, 0x31, 0xBE, 0xA0, 0x06, 0x31, 0xBE, 0xA0}},
	{0xEA, 16, {0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00}},
	{0xEB, 7, {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xEC, 2, {0x02, 0x00}},
	{0xED, 16, {0xF5, 0x47, 0x6F, 0x0B, 0x8F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xF8, 0xB0, 0xF6, 0x74, 0x5F}},
	{0xEF, 12, {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
	/* End GIP Setting */

	{0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}},
	{0x29, 0, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
} ;

/*
*	lcdc apis
*/
static void LcdcInitConfig(LCDC_InitTypeDef *plcdc_initstruct, u8 *imgbuffer, u32 bgcolor, u16 widht, u16 height)
{
	u8 idx ;
	rgb888_t bg_color;
	if (NULL == plcdc_initstruct) {
		return ;
	}
	LCDC_StructInit(plcdc_initstruct);
	plcdc_initstruct->LCDC_ImageWidth = widht;
	plcdc_initstruct->LCDC_ImageHeight = height;

	*(u32 *)&bg_color = bgcolor;
	plcdc_initstruct->LCDC_BgColorRed = bg_color.red;
	plcdc_initstruct->LCDC_BgColorGreen = bg_color.green;
	plcdc_initstruct->LCDC_BgColorBlue = bg_color.blue;

	for (idx = 0; idx < LCDC_LAYER_MAX_NUM; idx++) {
		plcdc_initstruct->layerx[idx].LCDC_LayerEn = DISABLE;
		plcdc_initstruct->layerx[idx].LCDC_LayerImgFormat = LCDC_LAYER_IMG_FORMAT_ARGB8888;//default value
		plcdc_initstruct->layerx[idx].LCDC_LayerImgBaseAddr = (u32)imgbuffer;
		plcdc_initstruct->layerx[idx].LCDC_LayerHorizontalStart = 1;/*1-based*/
		plcdc_initstruct->layerx[idx].LCDC_LayerHorizontalStop = widht;
		plcdc_initstruct->layerx[idx].LCDC_LayerVerticalStart = 1;/*1-based*/
		plcdc_initstruct->layerx[idx].LCDC_LayerVerticalStop = height;
	}
}

//lcdc global register APIs
void LcdcDumpRegValue(void __iomem *address, const char *filename)
{
	void __iomem  *pLCDCx =  address;
	void __iomem  *LCDC_Layerx;
	u32 idx ;

	/*global register*/
	printk(KERN_NOTICE"[%s]Dump lcdc register value baseaddr : 0x%08x\n", filename, (u32)pLCDCx);
	
	printk(KERN_NOTICE"pLCDCx CTRL[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_CTRL_OFFSET));
	printk(KERN_NOTICE"pLCDCx PLANE_SIZE[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_PLANE_SIZE_OFFSET));
	printk(KERN_NOTICE"pLCDCx UNDFLW_CFG[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_UNDFLW_CFG_OFFSET));
	printk(KERN_NOTICE"pLCDCx DMA_MODE_CFG[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_DMA_MODE_CFG_OFFSET));
	printk(KERN_NOTICE"pLCDCx SHW_RLD_CFG[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_SHW_RLD_CFG_OFFSET));
	printk(KERN_NOTICE"pLCDCx BKG_COLOR[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_BKG_COLOR_OFFSET));
//	printk(KERN_NOTICE"pLCDCx->LCDC_SEC_DEST_ADDR = 0x%08x\n", readl(pLCDCx+LCDC_SEC_DEST_ADDR_OFFSET));
	printk(KERN_NOTICE"pLCDCx DEBUG_STATUS[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_DEBUG_STATUS_OFFSET));

	//interrupt related register
	printk(KERN_NOTICE"pLCDCx IRQ_EN[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_IRQ_EN_OFFSET));
	printk(KERN_NOTICE"pLCDCx IRQ_STATUS[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_IRQ_STATUS_OFFSET));
	printk(KERN_NOTICE"pLCDCx IRQ_RAW[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_IRQ_RAW_OFFSET));
	printk(KERN_NOTICE"pLCDCx LINE_INT_POS[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_LINE_INT_POS_OFFSET));
	printk(KERN_NOTICE"pLCDCx CUR_POS_STATUS[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_CUR_POS_STATUS_OFFSET));
	printk(KERN_NOTICE"pLCDCx STATUS[0x%x] = 0x%08x\n", DUMP_REG(pLCDCx , LCDC_STATUS_OFFSET));
	printk(KERN_NOTICE"\n");

	for (idx = 0; idx < LCDC_LAYER_MAX_NUM; idx++) {
		switch (idx) {
		case LCDC_LAYER_LAYER1:
			LCDC_Layerx = pLCDCx + LCDC_LAYER1_BASEADDRESS_OFFSET ;
			break;
		case LCDC_LAYER_LAYER2:
			LCDC_Layerx = pLCDCx + LCDC_LAYER2_BASEADDRESS_OFFSET ;
			break;
		case LCDC_LAYER_LAYER3:
		default:
			LCDC_Layerx = pLCDCx + LCDC_LAYER3_BASEADDRESS_OFFSET ;
			break;
		}
		/*Layerx related register*/
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_CTRL = 0x%08x\n", idx, readl(LCDC_Layerx + LCDC_LAYERx_CTRL_OFFSET));
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_BASE_ADDR = 0x%08x\n", idx, readl(LCDC_Layerx + LCDC_LAYERx_BASE_ADDR_OFFSET));
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_WIN_XPOS = 0x%08x\n", idx, readl(LCDC_Layerx + LCDC_LAYERx_WIN_XPOS_OFFSET));
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_WIN_YPOS = 0x%08x\n", idx, readl(LCDC_Layerx + LCDC_LAYERx_WIN_YPOS_OFFSET));
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_COLOR_KEY = 0x%08x\n", idx, readl(LCDC_Layerx+LCDC_LAYERx_COLOR_KEY_OFFSET));
		printk(KERN_NOTICE"pLCDCx->LCDC_LAYER%x_ALPHA = 0x%08x\n", idx, readl(LCDC_Layerx+LCDC_LAYERx_ALPHA_OFFSET));
		printk(KERN_NOTICE"\n");
	}
}
u32 ameba_lcdc_reg_read(void __iomem *address)
{
	return readl(address);
}
void ameba_lcdc_reg_write(void __iomem *address,u32 Value32)
{
	writel(Value32,address);
}

void ameba_lcdc_enable(void __iomem *address, u32 NewState)
{
	if (DISABLE == NewState) {
		LCDC_Cmd(address, DISABLE);
	} else {
		/*enable the LCDC*/
		LCDC_Cmd(address, ENABLE);
		while (!LCDC_CheckLCDCReady(address));
	}
}
u32 ameba_lcdc_get_irqstatus(void __iomem *address)
{
	return LCDC_GetINTStatus(address);
}
void ameba_lcdc_clean_irqstatus(void __iomem *address, u32 irq)
{
	LCDC_ClearINT(address, irq);
}

//reset lcdc struct
void ameba_lcdc_reset_config(LCDC_InitTypeDef *LCDC_InitStruct,u16 widht, u16 height,u32 bgcolor)
{
	if (NULL == LCDC_InitStruct) {
		return ;
	}
	//LCDC_StructInit(LCDC_InitStruct);
	LcdcInitConfig(LCDC_InitStruct, NULL, bgcolor,widht, height);
}
void ameba_lcdc_set_planesize(LCDC_InitTypeDef *LCDC_InitStruct, u16 widht, u16 height)
{
	if (NULL == LCDC_InitStruct) {
		return ;
	}
	LCDC_InitStruct->LCDC_ImageWidth = widht;
	LCDC_InitStruct->LCDC_ImageHeight = height;
}
void ameba_lcdc_set_background_color(LCDC_InitTypeDef *LCDC_InitStruct, u32 bgcolor)
{
	rgb888_t bg_color;
	if (NULL == LCDC_InitStruct) {
		return ;
	}

	*(u32 *)&bg_color = bgcolor;
	LCDC_InitStruct->LCDC_BgColorRed = bg_color.red;
	LCDC_InitStruct->LCDC_BgColorGreen = bg_color.green;
	LCDC_InitStruct->LCDC_BgColorBlue = bg_color.blue;
}
void ameba_lcdc_config_setvalid(void __iomem *address, LCDC_InitTypeDef *LCDC_InitStruct)
{
	if (NULL == LCDC_InitStruct) {
		return ;
	}
	LCDC_Init(address, LCDC_InitStruct);
}
void ameba_lcdc_enable_SHW(void __iomem *address)
{
	LCDC_TrigerSHWReload(address);
}

//dma
void ameba_lcdc_dma_config_bustsize(void __iomem *address, u32 size)
{
	LCDC_DMAModeConfig(address, size);
}
void ameba_lcdc_dma_config_keeplastFrm(void __iomem *address, u32 DmaUnFlwMode, u32 showData)
{
	LCDC_DMAUnderFlowConfig(address, DmaUnFlwMode, showData);
}
void ameba_lcdc_dma_debug_config(void __iomem *address, u32 writeBackFlag, u32 ImgDestAddr)
{
	LCDC_DMADebugConfig(address, writeBackFlag, ImgDestAddr);
}
void ameba_lcdc_dma_get_unint_cnt(void __iomem *address, u32 *DmaUnIntCnt)
{
	LCDC_GetDmaUnINTCnt(address, DmaUnIntCnt);
}


//lcdc irq issue
void ameba_lcdc_irq_enable(void __iomem *address, u32 LCDC_IT, u32 NewState)
{
	LCDC_INTConfig(address, LCDC_IT, NewState);
}
void ameba_lcdc_irq_linepos(void __iomem *address, u32 LineNum)
{
	LCDC_LineINTPosConfig(address, LineNum);
}
void ameba_lcdc_irq_config(void __iomem *address, u32 intType, u32 NewState)
{
	LCDC_INTConfig(address, intType, NewState);
}


///layer
void ameba_lcdc_update_layer_reg(void __iomem *address, u8 layid, LCDC_LayerConfigTypeDef *EachLayer)
{
	LCDC_LayerConfig(address, layid, EachLayer);
}
void ameba_lcdc_layer_enable(LCDC_InitTypeDef *LCDC_InitStruct, u8 layid, u8 able)
{
	if (NULL == LCDC_InitStruct || layid >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[layid].LCDC_LayerEn = able;
}
void ameba_lcdc_layer_imgfmt(LCDC_InitTypeDef *LCDC_InitStruct, u8 layid, u32 fmt)
{
	if (NULL == LCDC_InitStruct || layid >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[layid].LCDC_LayerImgFormat = fmt;
}
void ameba_lcdc_layer_imgaddress(LCDC_InitTypeDef *LCDC_InitStruct, u8 layid, u32 imgaddress)
{
	if (NULL == LCDC_InitStruct || layid >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[layid].LCDC_LayerImgBaseAddr = imgaddress;
}
void ameba_lcdc_layer_pos(LCDC_InitTypeDef *LCDC_InitStruct, u8 idx, u16 start_x, u16 stop_x, u16 start_y, u16 stop_y)
{
	if (NULL == LCDC_InitStruct || idx >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[idx].LCDC_LayerHorizontalStart = start_x;/*1-based*/
	LCDC_InitStruct->layerx[idx].LCDC_LayerHorizontalStop = stop_x;
	LCDC_InitStruct->layerx[idx].LCDC_LayerVerticalStart = start_y;/*1-based*/
	LCDC_InitStruct->layerx[idx].LCDC_LayerVerticalStop = stop_y;
}
void ameba_lcdc_layer_colorkey_enable(LCDC_InitTypeDef *LCDC_InitStruct, u8 idx, u8 able)
{
	if (NULL == LCDC_InitStruct || idx >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[idx].LCDC_LayerColorKeyingEn = able;
}
void ameba_lcdc_layer_colorkey_value(LCDC_InitTypeDef *LCDC_InitStruct, u8 idx, u32 colorkey)
{
	if (NULL == LCDC_InitStruct || idx >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[idx].LCDC_LayerColorKeyingVal = colorkey;
}
void ameba_lcdc_layer_blend_value(LCDC_InitTypeDef *LCDC_InitStruct, u8 idx, u8 blend)
{
	if (NULL == LCDC_InitStruct || idx >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[idx].LCDC_LayerBlendConfig = blend;
}
void ameba_lcdc_layer_alpha_value(LCDC_InitTypeDef *LCDC_InitStruct, u8 idx, u8 alpha)
{
	if (NULL == LCDC_InitStruct || idx >= LCDC_LAYER_MAX_NUM) {
		return ;
	}
	LCDC_InitStruct->layerx[idx].LCDC_LayerConstAlpha = alpha;
}



/*
*	mipi dsi apis
*/
void DelayUs(u32 time)
{
	ndelay(1000 * time) ;
}
void MIPI_InitStruct_Config(MIPI_InitTypeDef *MIPI_InitStruct,u32 width,u32 height,u32 framerate, u32 *mipi_ckd)
{
	u32 vtotal, htotal_bits, bit_per_pixel, overhead_cycles, overhead_bits, total_bits;
	u32 MIPI_HACT_g = width;
	u32 MIPI_VACT_g = height;
	u32 vo_freq, vo_totalx, vo_totaly, mipi_div;

	switch (MIPI_InitStruct->MIPI_VideoDataFormat) {
	case MIPI_VIDEO_DATA_FORMAT_RGB565:
		bit_per_pixel = 16;
		break;
	case MIPI_VIDEO_DATA_FORMAT_RGB666_PACKED:
		bit_per_pixel = 18;
		break;
	case MIPI_VIDEO_DATA_FORMAT_RGB666_LOOSELY:
	case MIPI_VIDEO_DATA_FORMAT_RGB888:
	default:
		bit_per_pixel = 24;
		break;
	}

	MIPI_InitStruct->MIPI_LaneNum = 2;
	MIPI_InitStruct->MIPI_FrameRate = framerate;

	MIPI_InitStruct->MIPI_HSA = MIPI_DSI_HSA * bit_per_pixel / 8 ;//- 10; /* here the unit is pixel but not us */
	if (MIPI_InitStruct->MIPI_VideoModeInterface == MIPI_VIDEO_NON_BURST_MODE_WITH_SYNC_PULSES) {
		MIPI_InitStruct->MIPI_HBP = MIPI_DSI_HBP * bit_per_pixel / 8 ;//- 10;
	} else {
		MIPI_InitStruct->MIPI_HBP = (MIPI_DSI_HSA + MIPI_DSI_HBP) * bit_per_pixel / 8 ;//-10 ;
	}

	MIPI_InitStruct->MIPI_HACT = MIPI_HACT_g;
	MIPI_InitStruct->MIPI_HFP = MIPI_DSI_HFP * bit_per_pixel / 8 ;//-12;

	MIPI_InitStruct->MIPI_VSA = MIPI_DSI_VSA;
	MIPI_InitStruct->MIPI_VBP = MIPI_DSI_VBP;
	MIPI_InitStruct->MIPI_VACT = MIPI_VACT_g;
	MIPI_InitStruct->MIPI_VFP = MIPI_DSI_VFP;

	/*DataLaneFreq * LaneNum = FrameRate * (VSA+VBP+VACT+VFP) * (HSA+HBP+HACT+HFP) * PixelFromat*/
	vtotal = MIPI_InitStruct->MIPI_VSA + MIPI_InitStruct->MIPI_VBP + MIPI_InitStruct->MIPI_VACT + MIPI_InitStruct->MIPI_VFP;
	htotal_bits = (MIPI_DSI_HSA + MIPI_DSI_HBP + MIPI_InitStruct->MIPI_HACT + MIPI_DSI_HFP) * bit_per_pixel;
	overhead_cycles = T_LPX + T_HS_PREP + T_HS_ZERO + T_HS_TRAIL + T_HS_EXIT;
	overhead_bits = overhead_cycles * MIPI_InitStruct->MIPI_LaneNum * 8;
	total_bits = htotal_bits + overhead_bits;

	MIPI_InitStruct->MIPI_VideDataLaneFreq = MIPI_InitStruct->MIPI_FrameRate * total_bits * vtotal / MIPI_InitStruct->MIPI_LaneNum / Mhz + 20;

	MIPI_InitStruct->MIPI_LineTime = (MIPI_InitStruct->MIPI_VideDataLaneFreq * Mhz) / 8 / MIPI_InitStruct->MIPI_FrameRate / vtotal;
	MIPI_InitStruct->MIPI_BllpLen = MIPI_InitStruct->MIPI_LineTime / MIPI_InitStruct->MIPI_LaneNum ;

	if (MIPI_DSI_HSA + MIPI_DSI_HBP + MIPI_HACT_g + MIPI_DSI_HFP < (512 + MIPI_DSI_RTNI * 16)) {
		printk(KERN_WARNING"!!ERROR!!, LCM NOT SUPPORT\n");
	}

	if (MIPI_InitStruct->MIPI_LineTime * MIPI_InitStruct->MIPI_LaneNum < total_bits / 8) {
		printk(KERN_WARNING"!!ERROR!!, LINE TIME TOO SHORT!\n");
	}
	
	//vo frequency , //output format is RGB888
	vo_totalx = MIPI_DSI_HSA + MIPI_DSI_HBP + MIPI_DSI_HFP + width;
	vo_totaly = MIPI_DSI_VSA + MIPI_DSI_VBP + MIPI_DSI_VFP + height;
	vo_freq = vo_totalx * vo_totaly * MIPI_InitStruct->MIPI_FrameRate * 24 / 24 / Mhz + 4;
	//assert_param(vo_freq < 67);

	mipi_div = 1000 / vo_freq - 1;//shall get NPPLL frequency 
	if(mipi_ckd)
	{
		*mipi_ckd = mipi_div;
	}

	printk(KERN_INFO"DataLaneFreq: %d, LineTime: %d, vo_freq: %d[Mhz], mipi_ckd: %d\n", 
		MIPI_InitStruct->MIPI_VideDataLaneFreq, MIPI_InitStruct->MIPI_LineTime, vo_freq, mipi_div);
}

static void MipiDsi_Send_DCS_Cmd(void __iomem *MIPIx, u8 cmd, u8 payload_len, u8 *para_list)
{
	u32 word0, word1, addr, idx;
	u8 cmd_addr[128];

	if (payload_len == 0) {
		MIPI_DSI_CMD_Send(MIPIx, MIPI_DSI_DCS_SHORT_WRITE, cmd, 0);
		return;
	} else if (payload_len == 1) {
		MIPI_DSI_CMD_Send(MIPIx, MIPI_DSI_DCS_SHORT_WRITE_PARAM, cmd, para_list[0]);
		return;
	}

	cmd_addr[0] = cmd;
	for (idx = 0; idx < payload_len; idx++) {
		cmd_addr[idx + 1 ] = para_list[idx];
	}

	payload_len = payload_len + 1;

	/* the addr payload_len 1 ~ 8 is 0 */
	for (addr = 0; addr < (u32)(payload_len + 7) / 8; addr++) {
		idx = addr * 8;
		word0 = (cmd_addr[idx + 3] << 24) + (cmd_addr[idx + 2] << 16) + (cmd_addr[idx + 1] << 8) + cmd_addr[idx + 0];
		word1 = (cmd_addr[idx + 7] << 24) + (cmd_addr[idx + 6] << 16) + (cmd_addr[idx + 5] << 8) + cmd_addr[idx + 4];

		MIPI_DSI_CMD_LongPkt_MemQWordRW(MIPIx, addr, &word0, &word1, false);
	}
	MIPI_DSI_CMD_Send(MIPIx, MIPI_DSI_DCS_LONG_WRITE, payload_len, 0);
}

static void MipiDsi_Send_Cmd(void __iomem *MIPIx, LCM_setting_table_t *table, u32 *initdone)
{
	static u8 send_cmd_idx_s = 0;
	u32 payload_len;
	u8 cmd, send_flag = false;
	u8 *cmd_addr;

	while (1) {
		cmd = table[send_cmd_idx_s].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			mdelay(table[send_cmd_idx_s].count);
			break;
		case REGFLAG_END_OF_TABLE:
			printk(KERN_DEBUG"CMD Send Done\n");
			*initdone = 1;
			return;
		default:
			if (send_flag) {
				return;
			}
			cmd_addr = table[send_cmd_idx_s].para_list;
			payload_len = table[send_cmd_idx_s].count;

			MipiDsi_Send_DCS_Cmd(MIPIx, cmd, payload_len, cmd_addr);

			send_flag = true;
		}
		send_cmd_idx_s++;
	}
}

void MipiDsi_Do_Init(void __iomem *MIPIx, MIPI_InitTypeDef *MIPI_InitStruct,  u32 *sendcmd)
{
	u32 initdone = 0 ;
	*sendcmd = true;

	MIPI_DSI_TO1_Set(MIPIx, DISABLE, 0);
	MIPI_DSI_TO2_Set(MIPIx, ENABLE, 0x7FFFFFFF);
	MIPI_DSI_TO3_Set(MIPIx, DISABLE, 0);

	//enable ir ,get the cmd send finish issue
	MIPI_DSI_INT_Config(MIPIx, DISABLE, ENABLE, false);
	MIPI_DSI_init(MIPIx, MIPI_InitStruct);

	while (1) {
		if (*sendcmd) {
			*sendcmd = 0;

			if (!(initdone)) {
				MipiDsi_Send_Cmd(MIPIx, lcm_initialization_setting, &initdone);
				mdelay(1);
			} else {
				break;
			}
		} else {
			mdelay(1);
		}
	}

	MIPI_DSI_INT_Config(MIPIx, DISABLE, DISABLE, false);
}

void MipiDumpRegValue(void __iomem *address)
{
	void __iomem *MIPIx = address;

	/*global register*/
	printk(KERN_NOTICE"Dump mipi register value baseaddr : 0x%08x\n", (u32)MIPIx);	
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_MAIN_CTRL_OFFSET,readl(MIPIx + MIPI_MAIN_CTRL_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_INTE_OFFSET,readl(MIPIx + MIPI_INTE_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_INTS_ACPU_OFFSET,readl(MIPIx + MIPI_INTS_ACPU_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_PAT_GEN_OFFSET,readl(MIPIx + MIPI_PAT_GEN_OFFSET));
	printk(KERN_NOTICE"\n");

	/*Dphy register*/
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_CLOCK_GEN_OFFSET,readl(MIPIx + MIPI_CLOCK_GEN_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_WATCHDOG_OFFSET,readl(MIPIx + MIPI_WATCHDOG_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_DF_OFFSET,readl(MIPIx + MIPI_DF_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_SSC2_OFFSET,readl(MIPIx + MIPI_SSC2_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_SSC3_OFFSET,readl(MIPIx + MIPI_SSC3_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_MPLL_OFFSET,readl(MIPIx + MIPI_MPLL_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_ESCAPE_TX_DATA_1_OFFSET,readl(MIPIx + MIPI_ESCAPE_TX_DATA_1_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_ESCAPE_TX_DATA_2_OFFSET,readl(MIPIx + MIPI_ESCAPE_TX_DATA_2_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_ESCAPE_TX_DATA_3_OFFSET,readl(MIPIx + MIPI_ESCAPE_TX_DATA_3_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_ESCAPE_TX_CLK_0_OFFSET,readl(MIPIx + MIPI_ESCAPE_TX_CLK_0_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_ESCAPE_TX_DATA_6_OFFSET,readl(MIPIx + MIPI_ESCAPE_TX_DATA_6_OFFSET));
	printk(KERN_NOTICE"\n");

	/*DSI register*/
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC0_OFFSET,readl(MIPIx + MIPI_TC0_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC1_OFFSET,readl(MIPIx + MIPI_TC1_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC2_OFFSET,readl(MIPIx + MIPI_TC2_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC3_OFFSET,readl(MIPIx + MIPI_TC3_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC4_OFFSET,readl(MIPIx + MIPI_TC4_OFFSET));
	printk(KERN_NOTICE"MIPIx[0x%x] = 0x%08x\n", MIPI_TC5_OFFSET,readl(MIPIx + MIPI_TC5_OFFSET));
	printk(KERN_NOTICE"\n");
}

//afdsads
