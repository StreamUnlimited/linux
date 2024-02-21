// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek DRM support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_device.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_of.h>
#include <drm/drm_print.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_panel.h>

#include "panel/ameba_panel_base.h"
#include "ameba_drm_base.h"
#include "ameba_mipi.h"
#include "ameba_drm_drv.h"
#include "ameba_drm_comm.h"


#define LSYS_MASK_CKD_MIPI                         ((u32)0x0000003F << 24)          /*!<R/WPD 6'd14  mipi vo clock divider, it is based on nppll clock Max timing signoff clock is 66.7M . Default is 800/12 = 66.7 divider by this value + 1 */
#define LSYS_CKD_MIPI(x)                           ((u32)(((x) & 0x0000003F) << 24))
#define LSYS_GET_CKD_MIPI(x)                       ((u32)(((x >> 24) & 0x0000003F)))
#define REG_LSYS_CKD_GRP0                           0x28


#ifndef ENABLE_LCDC_CTL
void MIPI_DSI_Pat_Gen(MIPI_TypeDef *mipireg, u8 new_state, u8 pg_type, u32 user_color);
#endif

/*
*	below is mipi dsi struct
*/
#define encoder_to_dsi(param)           container_of(param, struct ameba_hw_dsi, encoder)
#define connector_to_dsi(param)         container_of(param, struct ameba_hw_dsi, connector)
#define host_to_dsi(param)              container_of(param, struct ameba_hw_dsi, dsi_host)

struct ameba_hw_dsi {
	void __iomem *mipi_reg;	//put in first place
	struct device *dsidev;

	struct ameba_drm_struct *ameba_struct;

	struct drm_encoder encoder;
	struct drm_connector connector;
	struct device *paneldev;

	//panel driver issue
	struct drm_panel *panel;
	struct mipi_dsi_host dsi_host; 

	//mipi dsi register  & data struction
	MIPI_InitTypeDef dsi_ctx;
	void __iomem *lsys_aip_ctrl1;

	u32 mipi_ckd;
	void __iomem *sysctl_base_lp;//vo clk div

	bool enable;
	bool dsi_init;

	int irq;  /* cmd replay,underflow reset issue */

	u32 dsi_tx_done;//command tx done
	u32 dsi_rx_cmd;//it is read command

	u32 dsi_debug;
};

//debug 
static ssize_t dsi_debug_show(struct device *dev, struct device_attribute*attr, char *buf)
{
	struct ameba_hw_dsi         *dsi = dev_get_drvdata(dev);
	return sprintf(buf, "dsi_debug=%d\n", dsi->dsi_debug);
}
static ssize_t dsi_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct ameba_hw_dsi         *dsi = dev_get_drvdata(dev);
	u32 tmp = 0 ;
	char c;
	size_t tmp_count = count;
	char* pbuf = (char*)buf;

	while(*pbuf != 0 && tmp_count > 1)
	{
		c = *pbuf;
		if((c<'0')||(c>'9'))
			break;
		tmp = tmp * 10 + (c - '0');
		pbuf++;
		tmp_count--;
	}

	dsi->dsi_debug = tmp;
    return count;
}

static DEVICE_ATTR(dsi_debug, S_IWUSR |S_IRUGO, dsi_debug_show, dsi_debug_store);

static struct attribute *dsi_debug_attrs[] = {
        &dev_attr_dsi_debug.attr,
        NULL
};

static const struct attribute_group dsi_debug_attr_grp = {
        .attrs = dsi_debug_attrs,
};

/* underflow issue */
static irqreturn_t mipi_dsi_underflow_irq(int irq, void *data)
{
	struct ameba_hw_dsi      *dsi = (struct ameba_hw_dsi*)data;
	struct device            *dev = dsi->dsidev;
	void __iomem             *pmipi_reg = dsi->mipi_reg;
	struct ameba_drm_struct  *ameba_struct = dsi->ameba_struct;
	struct ameba_drm_reg_t   *lcdc_ctx = (struct ameba_drm_reg_t *)ameba_struct->lcdc_hw_ctx;
	void __iomem             *plcdc_reg = lcdc_ctx->reg;

	u32 reg_val = MIPI_DSI_INTS_ACPU_Get(pmipi_reg);
	MIPI_DSI_INTS_ACPU_Clr(pmipi_reg, reg_val);

	if (reg_val) {
		DRM_DEBUG_DRIVER("Underflow count(%d)reg(0x%x)\n",
			ameba_struct->under_flow_count,readl((void*)(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0)));

		MIPI_DSI_INT_Config(pmipi_reg, DISABLE, DISABLE, DISABLE);

		/*reset the LCDC*/
		ameba_lcdc_reenable(plcdc_reg);
		ameba_struct->under_flow_flag = 0;

		MIPI_DSI_Mode_Switch(pmipi_reg, ENABLE);

		if(dsi->dsi_debug){
			MipiDumpRegValue(dev, pmipi_reg);
		}
	}

	return IRQ_HANDLED; 
}


static void MipiDsiReadCMD_Decode(struct device *dev, MIPI_TypeDef *MIPIx, u8 rcmd_idx)
{
	u32                rcmd_val, payload_len, addr, word0, word1;
	u8                 data_id, byte0, byte1;

	rcmd_val = MIPI_DSI_CMD_Rxcv_CMD(MIPIx, rcmd_idx);
	if (rcmd_val & (MIPI_BIT_RCMDx_CRC_CHK_ERR | MIPI_BIT_RCMDx_ECC_CHK_ERR | MIPI_BIT_RCMDx_ECC_ERR_FIX)) {
		DRM_DEBUG_DRIVER("RCMD[%d] Error\n", rcmd_idx + 1);
	}

	if (rcmd_val & MIPI_BIT_RCMDx_ECC_NO_ERR) {
		data_id = MIPI_GET_RCMDx_DATAID(rcmd_val);
		byte0 = MIPI_GET_RCMDx_BYTE0(rcmd_val);
		byte1 = MIPI_GET_RCMDx_BYTE1(rcmd_val);
		DRM_DEBUG_DRIVER("RCMD[%d]:\n", rcmd_idx + 1);
		DRM_DEBUG_DRIVER("DataID: 0x%02x, BYTE0: 0x%02x, BYTE1: 0x%02x\n", data_id, byte0, byte1);

		/* Peripheral to Processor Transactions Long Packet is 0x1A or 0x1C */
		if (MIPI_LPRX_IS_LONGRead(data_id)) {
			payload_len = (byte1 << 8) + byte0;
		} else {
			payload_len = 0;
		}
		DRM_DEBUG_DRIVER("Payloadlen(%d)\n", payload_len);
		/* the addr payload_len 1 ~ 8 is 0 */
		for (addr = 0; addr < (payload_len + 7) / 8; addr++) {
			MIPI_DSI_CMD_LongPkt_MemQWordRW(MIPIx, addr, &word0, &word1, TRUE);
			DRM_DEBUG_DRIVER("Addr[%x]: 0x%08x 0x%08x\n", addr * 8, word0, word1);
		}
	}
}

//command mode 
static irqreturn_t mipi_dsi_irq_handler(int irq, void *data)
{
	u32                     reg_val, reg_val2, reg_dphy_err,Value32;
	struct ameba_hw_dsi     *mipi_dsi = (struct ameba_hw_dsi*)data;
	struct device           *dev = mipi_dsi->dsidev;
	void __iomem            *mipi_reg = mipi_dsi->mipi_reg ;

	reg_val = MIPI_DSI_INTS_Get(mipi_reg);
	MIPI_DSI_INTS_Clr(mipi_reg, reg_val);

	reg_val2 = MIPI_DSI_INTS_ACPU_Get(mipi_reg);
	MIPI_DSI_INTS_ACPU_Clr(mipi_reg, reg_val2);

	if (reg_val & MIPI_BIT_CMD_TXDONE) { //command tx done
		reg_val &= ~MIPI_BIT_CMD_TXDONE;
		if(mipi_dsi->dsi_rx_cmd == 0) {
			mipi_dsi->dsi_tx_done = 1;
		}
	}

	if(reg_val & (MIPI_BIT_RCMD1|MIPI_BIT_RCMD2|MIPI_BIT_RCMD3 )) {
		/* 1. Read CMD Response */
		if (reg_val & MIPI_BIT_RCMD1) {
			DRM_DEBUG_DRIVER("MIPI RCMD1\n");
			MipiDsiReadCMD_Decode(dev, mipi_reg, 0);
		}
		/* 2. error report packet */
		if (reg_val & MIPI_BIT_RCMD2) {
			DRM_DEBUG_DRIVER("MIPI RCMD2\n");
			MipiDsiReadCMD_Decode(dev, mipi_reg, 1);
		}
		/* 3. eotp */
		if (reg_val & MIPI_BIT_RCMD3) {
			DRM_DEBUG_DRIVER("MIPI RCMD3\n");
			MipiDsiReadCMD_Decode(dev, mipi_reg, 2);
		}
		mipi_dsi->dsi_tx_done = 1;
	}

	if (reg_val & MIPI_BIT_ERROR) {
		reg_dphy_err = readl(mipi_reg+MIPI_DPHY_ERR_OFFSET);
		writel(reg_dphy_err,mipi_reg+MIPI_DPHY_ERR_OFFSET) ;
		DRM_DEBUG_DRIVER("LPTX ISSUE: 0x%x, DPHY ISSUE: 0x%x\n", reg_val, reg_dphy_err);

		Value32 = readl(mipi_reg+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET) ;
		if (Value32 & MIPI_MASK_DETECT_ENABLE) {
			Value32 &= ~MIPI_MASK_DETECT_ENABLE;
			writel(Value32,mipi_reg+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET);

			writel(reg_dphy_err,mipi_reg+MIPI_DPHY_ERR_OFFSET);
			MIPI_DSI_INTS_Clr(mipi_reg, MIPI_BIT_ERROR);
			DRM_DEBUG_DRIVER("LPTX ISSUE CLR: 0x%x, DPHY: 0x%x\n", readl(mipi_reg+MIPI_INTS_OFFSET), readl(mipi_reg+MIPI_DPHY_ERR_OFFSET));
		}

		if (readl(mipi_reg+MIPI_DPHY_ERR_OFFSET) == reg_dphy_err) {
			DRM_DEBUG_DRIVER("LPTX Still Error\n");
			MIPI_DSI_INT_Config(mipi_reg, ENABLE, DISABLE, false);
		}
		reg_val &= ~MIPI_BIT_ERROR;
	}

	if (reg_val) {
		DRM_ERROR("LPTX Error Occur: 0x%x\n", reg_val);
	}

	if (reg_val2) {
		DRM_DEBUG_DRIVER("ACPU interrupt Occur: 0x%x\n", reg_val2);
	}

	return IRQ_HANDLED;
}
static void dsi_mipi_init_pre(struct device *dev, struct ameba_hw_dsi *dsi)
{
	u32 reg_value, reg_tmp;
	struct ameba_drm_struct          *ameba_struct = dsi->ameba_struct;

	MIPI_InitTypeDef                 *pdsi_ctx = &(dsi->dsi_ctx);
	void __iomem                     *pmipi_reg = dsi->mipi_reg ;
	void __iomem                     *lsys_aip_ctrl1 = dsi->lsys_aip_ctrl1;

	// reset mipi core
	MIPI_StructInit(pdsi_ctx);
	MIPI_InitStruct_Config(dev,pdsi_ctx,ameba_struct->display_width,ameba_struct->display_height,ameba_struct->display_framerate,&(dsi->mipi_ckd));
	MIPI_BG_CMD(lsys_aip_ctrl1, ENABLE);//bandgap issue
	MIPI_DPHY_init(dev, pmipi_reg, pdsi_ctx);

	//set the vo div
	reg_value = ameba_lcdc_reg_read(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0);
	reg_tmp = reg_value;
	reg_value &= ~LSYS_MASK_CKD_MIPI;
	reg_value |= LSYS_CKD_MIPI(dsi->mipi_ckd);
	ameba_lcdc_reg_write(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0, reg_value);
	DRM_DEBUG_DRIVER("Reg(0x%x-0x%x)\n", reg_tmp, ameba_lcdc_reg_read(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0));
}
static int dsi_mipi_init(struct device *dev, struct ameba_hw_dsi *dsi)
{
	struct device                   *paneldev;
	struct ameba_drm_panel_struct   *handle;
	LCM_setting_table_t             *table;
	MIPI_InitTypeDef                *pdsi_ctx = &(dsi->dsi_ctx);
	void __iomem                    *pmipi_reg = dsi->mipi_reg ;
	int                             ret;

	//register MIPI ISR
	ret = devm_request_irq(dev,dsi->irq, mipi_dsi_irq_handler,0,dev_name(dev), dsi);
	if (ret)
	{
		DRM_ERROR("Fail to set irq for dsi command \n");
		return -ENODEV;
	}
	//enable_irq(dsi->irq);
	//set command to dsi
	paneldev = dsi->paneldev;
	handle = dev_get_drvdata(paneldev);
	table = handle->init_table;
	MipiDsi_Do_Init(pmipi_reg,pdsi_ctx,&(dsi->dsi_tx_done),&(dsi->dsi_rx_cmd),table);
	//disable_irq(dsi->irq);

	devm_free_irq(dev, dsi->irq, dsi);//unregister dsi ISR

	MIPI_DSI_INT_Config(pmipi_reg,DISABLE,DISABLE, false);

	return 0;
}

static int dsi_mipi_init_irq(struct device *dev, struct ameba_hw_dsi *dsi)
{
	int                 ret;
#ifndef ENABLE_LCDC_CTL
	void __iomem        *pmipi_reg = dsi->mipi_reg ;
#endif
	ret = devm_request_irq(dev,dsi->irq, mipi_dsi_underflow_irq, 0, dev_name(dev), dsi);
	if (ret)
	{
		DRM_ERROR("Fail to set irq for underflow \n");
		return -ENODEV;
	}

#ifndef ENABLE_LCDC_CTL
	//test dsi issue , will show the color 0xFFFF00
	MIPI_DSI_Pat_Gen(pmipi_reg, ENABLE, 0, 0xFFFF00);
	MIPI_DSI_Mode_Switch(pmipi_reg, ENABLE);
#endif

	return  0 ;
}
/*
* This callback should be used to disable the encoder. With the atomic
* drivers it is called before this encoder's CRTC has been shut off
* using their own &drm_crtc_helper_funcs.disable hook.
*/
static void dsi_encoder_disable(struct drm_encoder *encoder)
{
	void __iomem            *pmipi_reg ;
	struct ameba_hw_dsi     *dsi = encoder_to_dsi(encoder);
	AMEBA_DRM_DEBUG
	if( NULL == dsi || !dsi->enable)
		return ;

	DRM_DEBUG_DRIVER("Dsi disable\n");
	MIPI_DSI_Mode_Switch(dsi->mipi_reg, DISABLE);

	dsi->enable = false;
}
static void dsi_encoder_enable(struct drm_encoder *encoder)
{
	int                      ret ;
	void __iomem             *pmipi_reg ;
	MIPI_InitTypeDef         *pdsi_ctx ;
	struct ameba_hw_dsi      *dsi = encoder_to_dsi(encoder);
	AMEBA_DRM_DEBUG

	if( NULL == dsi || dsi->enable)
		return;

	if(dsi->dsi_init == 0){
		dsi_mipi_init_pre(dsi->dsidev, dsi);

		ret = drm_panel_enable(dsi->panel);
		if (ret) {
			DRM_ERROR("Fail to init mipi dsi\n");
			return;
		}

		ret = dsi_mipi_init(dsi->dsidev, dsi);
		if (ret) {
			DRM_ERROR("Fail to init mipi req\n");
			return;
		}

		ret = dsi_mipi_init_irq(dsi->dsidev, dsi);
		if (ret) {
			DRM_ERROR("Fail to init mipi req\n");
			return;
		}

		dsi->dsi_init = 1;
	}

	pmipi_reg = dsi->mipi_reg;
	pdsi_ctx = &(dsi->dsi_ctx);
	DRM_DEBUG_DRIVER("Dsi enable \n");

	pdsi_ctx->MIPI_VideoNCmdMode = true;
	MIPI_DSI_Mode_Switch(pmipi_reg,ENABLE);

	if(dsi->dsi_debug) {
		MipiDumpRegValue(dsi->dsidev, pmipi_reg);
	}

	dsi->enable = true;
}
static enum drm_mode_status dsi_encoder_mode_valid(struct drm_encoder *encoder,
					const struct drm_display_mode *mode)
{
	return MODE_OK;
}

static void dsi_encoder_mode_set(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adj_mode)
{
	struct ameba_hw_dsi     *dsi = encoder_to_dsi(encoder);
	AMEBA_DRM_DEBUG

	///valid mode
	if(dsi->ameba_struct)
	{
		dsi->ameba_struct->display_height = adj_mode->vdisplay;
		dsi->ameba_struct->display_width = adj_mode->hdisplay;
		dsi->ameba_struct->display_framerate = adj_mode->vrefresh;
	}
}

static int dsi_encoder_atomic_check(struct drm_encoder *encoder,
				    struct drm_crtc_state *crtc_state,
				    struct drm_connector_state *conn_state)
{
	AMEBA_DRM_DEBUG
	return 0;
}

static const struct drm_encoder_helper_funcs ameba_encoder_helper_funcs = {
	.atomic_check   = dsi_encoder_atomic_check,
	.mode_valid     = dsi_encoder_mode_valid,
	.mode_set       = dsi_encoder_mode_set,	
	.enable         = dsi_encoder_enable,
	.disable        = dsi_encoder_disable
};

static const struct drm_encoder_funcs ameba_encoder_funcs = {
	.destroy = drm_encoder_cleanup,
};

static int ameba_drm_encoder_init(struct device *dev,
			       struct drm_device *drm_dev,
			       struct drm_encoder *encoder)
{
	int              ret;
	u32              crtc_mask  ;

	crtc_mask = drm_of_find_possible_crtcs(drm_dev, dev->of_node);
	if (!crtc_mask) {
		DRM_ERROR("Fail to find crtc mask\n");
		return -EINVAL;
	}

	encoder->possible_crtcs = crtc_mask;
	ret = drm_encoder_init(drm_dev, encoder, &ameba_encoder_funcs,
			       DRM_MODE_ENCODER_DSI, NULL);
	if (ret) {
		DRM_ERROR("Fail to init dsi encoder\n");
		return ret;
	}

	drm_encoder_helper_add(encoder, &ameba_encoder_helper_funcs);

	return 0;
}
static int ameba_conn_get_modes(struct drm_connector *connector)
{
   struct ameba_hw_dsi      *dsi = connector_to_dsi(connector);

   /* Just pass the question to the panel */
   if (dsi->panel)
	   return drm_panel_get_modes(dsi->panel);

   return 0;
}

static const struct drm_connector_funcs ameba_connector_funcs = {
    .fill_modes              = drm_helper_probe_single_connector_modes,
    .destroy                 = drm_connector_cleanup,
    .reset                   = drm_atomic_helper_connector_reset,
    .atomic_destroy_state    = drm_atomic_helper_connector_destroy_state,
    .atomic_duplicate_state  = drm_atomic_helper_connector_duplicate_state,
};
static const struct drm_connector_helper_funcs ameba_connector_helper_funcs = {
   .get_modes	 = ameba_conn_get_modes,
};
static int ameba_drm_connect_init(struct device *dev,
				  struct drm_device *drm_dev,
				  struct drm_connector *connector)
{
   int                    ret;

   //connector init
   ret = drm_connector_init(drm_dev, connector, &ameba_connector_funcs,
				DRM_MODE_CONNECTOR_DSI);
   if (ret) {
	   DRM_ERROR("Fail to init connector\n");
	   return 1;
   }

   drm_connector_helper_add(connector, &ameba_connector_helper_funcs);

   return 0;
}
static int rtk_drm_of_find_panel(const struct device_node *np,
				struct drm_panel **panel)
{
	int                   ret = -EPROBE_DEFER;
	struct device_node    *remote,*drm;

	if (!panel)
		return -EINVAL;
	if (panel)
		*panel = NULL;

	drm = of_graph_get_remote_node(np, 0, 0);  //drm
	if (!drm) {
		return -ENODEV;
	}

	remote = of_graph_get_remote_node(drm,0,1);  //panel
	if (!of_device_is_available(remote)) {
		of_node_put(remote);
		return -ENODEV;
	}

	if (panel) {
		*panel = of_drm_find_panel(remote);
		if (!IS_ERR(*panel)) {
			ret = 0;
		}
		else {
			*panel = NULL;
		}
	}

	of_node_put(remote);
	return 0;
}
static int ameba_init_panel(struct device *dev, struct ameba_hw_dsi *dsi)
{
	struct drm_panel        *panel = NULL;
//	struct device_node      *child;

	if (!of_get_available_child_count(dev->of_node)) {
		DRM_ERROR("DSI interface not init\n");
		return -EPERM;
	}
#if 0
	/* Look for a panel as a child to this node */
	for_each_available_child_of_node(dev->of_node, child) {
		panel = of_drm_find_panel(child);
		if (IS_ERR(panel)) {
			DRM_ERROR("Fail to find panel (%ld)\n",PTR_ERR(panel));
			panel = NULL;
		}
		else {
			break;
		}
	}
#else
	if( rtk_drm_of_find_panel(dev->of_node,&panel) < 0) {
		DRM_ERROR("Fail to find panel !\n");
		return -ENODEV;
	}
#endif
	if (panel) {
		dsi->panel = panel;
	} else {
		DRM_ERROR("No panel found!\n");
		return -ENODEV;
	}

	dsi->paneldev = panel->dev;

	drm_panel_attach(dsi->panel,&dsi->connector);

	return 0;
}
static int mipi_dsi_bind(struct device *dev, struct device *master, void *data)
{
	struct ameba_hw_dsi         *dsi = dev_get_drvdata(dev);
	struct drm_device           *drm = data;
	struct ameba_drm_struct     *ameba_struct = drm->dev_private;
	int                         ret;
	AMEBA_DRM_DEBUG

	ameba_struct->dsi_dev = dev;
	dsi->ameba_struct = ameba_struct;

	//init encoder
	ret = ameba_drm_encoder_init(dev, drm, &dsi->encoder);
	if (ret) {
		DRM_ERROR("Fail to init encoder\n");
		goto err_encoder;
	}

	//init encoder
	ret = ameba_drm_connect_init(dev, drm, &dsi->connector);
	if (ret) {
		DRM_ERROR("Fail to init connector\n");
		goto err_connector;
	}

	ret = drm_connector_attach_encoder(&dsi->connector, &dsi->encoder);
	if (ret) {
		DRM_ERROR("Fail to attach connector to encoder\n");
		goto err_attach;
	}

	//enable dsi
	ret = ameba_init_panel(dev, dsi);
	if (ret) {
		DRM_ERROR( "Fail to init panel \n");
		goto err_mipi_init;
	}

	DRM_INFO("MIPI DSI Bind Success!\n");

	return 0;

err_mipi_init:
err_attach:
	drm_connector_cleanup(&dsi->connector);

err_connector:
	drm_encoder_cleanup(&dsi->encoder);

err_encoder:

	return ret;
}

static void mipi_dsi_unbind(struct device *dev, struct device *master, void *data)
{
	struct ameba_hw_dsi         *dsi = dev_get_drvdata(dev);
	DRM_INFO("Run MIPI DSI Unbind \n");

	if (dsi->panel) {
		drm_panel_detach(dsi->panel);
		dsi->panel = NULL;
	}
}

static const struct component_ops mipi_dsi_ops = {
	.bind	= mipi_dsi_bind,
	.unbind	= mipi_dsi_unbind,
};

static int mipi_dsi_parse_dt(struct platform_device *pdev, struct ameba_hw_dsi *dsi)
{
	struct resource         *res;
	struct device           *dev = &pdev->dev;
	struct device_node      *rcc_node;
	struct resource         rcc_res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dsi->mipi_reg = devm_ioremap_resource(dev, res);
	if (0 == dsi->mipi_reg) {
		DRM_ERROR("Fail to remap dsi io region\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	dsi->lsys_aip_ctrl1 = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (IS_ERR(dsi->lsys_aip_ctrl1)) {
		DRM_ERROR("Fail to remap sys reg io region\n");
		return -ENODEV;
	}
	//irq info
	dsi->irq = platform_get_irq(pdev, 0);
	if (dsi->irq < 0) {
		DRM_ERROR("Fail to get irq\n");
		return -ENODEV;
	}

	rcc_node = of_parse_phandle(dev->of_node, "rtk,lcdc-vo-clock", 0);
	if ( of_address_to_resource(rcc_node, 1, &rcc_res) ) {
		DRM_ERROR("Get resource for lcdc vo clock error\n");
		return -ENOMEM;
	}

	dsi->sysctl_base_lp = ioremap(rcc_res.start, resource_size(&rcc_res));
	if ( !dsi->sysctl_base_lp ) {
			DRM_ERROR("Ioremap for lcdc vo clock  error\n");
			return -ENOMEM;
	}

	return 0;
}

static int mcde_dsi_host_attach(struct mipi_dsi_host *host,
				struct mipi_dsi_device *mdsi)
{
	AMEBA_DRM_DEBUG
	return 0;
}
static int mcde_dsi_host_detach(struct mipi_dsi_host *host,
				struct mipi_dsi_device *mdsi)
{
	AMEBA_DRM_DEBUG
	return 0;
}
static const struct mipi_dsi_host_ops ameba_dsi_host_ops = {
	.attach = mcde_dsi_host_attach,
	.detach = mcde_dsi_host_detach,
};
static int ameba_host_init(struct device *dev, struct mipi_dsi_host *host)
{
	int     ret;
	if(NULL == host)
		return -EPERM ;

	host->dev = dev;
	host->ops = &ameba_dsi_host_ops;
	ret = mipi_dsi_host_register(host);
	if (ret < 0) {
		DRM_ERROR("Fail to register DSI host: %d\n", ret);
		return ret;
	}

	return 0;
}

static int mipi_dsi_probe(struct platform_device *pdev)
{
	struct ameba_hw_dsi     *dsi;
	struct device           *dev = &pdev->dev;
	int                     ret;

	DRM_DEBUG_DRIVER("Run Dsi Probe!\n");

	dsi = devm_kzalloc(dev, sizeof(*dsi), GFP_KERNEL);
	if (!dsi) {
		DRM_ERROR("Fail to allocate dsi data.\n");
		return -ENOMEM;
	}
	memset(dsi, 0x00,sizeof(*dsi));
	dsi->dsidev = dev;

	ret = mipi_dsi_parse_dt(pdev, dsi);
	if (ret)
		return ret;

	dsi->dsi_debug = 0;
	dsi->dsi_init = 0;
	if ( sysfs_create_group(&pdev->dev.kobj,&dsi_debug_attr_grp) ) {
		DRM_ERROR("Error creating dsi sysfs entry.\n");
	}

	dev_set_drvdata(dev, dsi);

	ret = ameba_host_init(dev, &dsi->dsi_host);
	if (ret) {
		DRM_ERROR("Fail to init host.\n");
		return ret;
	}
	DRM_DEBUG_DRIVER("Run DSI Probe Success\n");

	return component_add(dev, &mipi_dsi_ops);
}

static int mipi_dsi_remove(struct platform_device *pdev)
{
	struct device               *dev = &pdev->dev;
	struct ameba_hw_dsi         *dsi = dev_get_drvdata(dev);
	void __iomem                *lsys_aip_ctrl1 = dsi->lsys_aip_ctrl1;

	DRM_DEBUG_DRIVER("Run Dsi Remove!\n");

	//close irq
	if( dsi && dsi->enable) {
		devm_free_irq(dev, dsi->irq, dsi);
		dsi->enable = false ;
	}
	iounmap(dsi->sysctl_base_lp);

	sysfs_remove_group(&pdev->dev.kobj, &dsi_debug_attr_grp);

	//host deinit
	mipi_dsi_host_unregister(&(dsi->dsi_host));
	dsi->dsi_host.dev = NULL;
	dsi->dsi_host.ops = NULL;

	MIPI_BG_CMD(lsys_aip_ctrl1, DISABLE);//bandgap issue	
	component_del(&pdev->dev, &mipi_dsi_ops);

	return 0;
}

static const struct of_device_id mipi_dsi_of_match[] = {
	{.compatible = "realtek,amebad2-dsi"},
	{ }
};
MODULE_DEVICE_TABLE(of, mipi_dsi_of_match);

static struct platform_driver mipi_dsi_driver_struct = {
	.probe = mipi_dsi_probe,
	.remove = mipi_dsi_remove,
	.driver = {
		.name = "realtek-amebad2-dsi",
		.of_match_table = mipi_dsi_of_match,
	},
};

module_platform_driver(mipi_dsi_driver_struct);

MODULE_DESCRIPTION("Realtek Ameba MIPI DSI driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Realtek Corporation");
