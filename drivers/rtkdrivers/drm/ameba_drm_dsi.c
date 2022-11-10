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

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_device.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_of.h>
#include <drm/drm_print.h>
#include <drm/drm_probe_helper.h>

#include "ameba_drm_base.h"
#include "ameba_mipi.h"
#include "ameba_drm_drv.h"
#include "ameba_drm_util.h"


#ifndef ENABLE_LCDC_CTL
void MIPI_DSI_Pat_Gen(MIPI_TypeDef *MIPIx, u8 NewState, u8 pg_type, u32 user_color);
#endif
static irqreturn_t mipi_dsi_irq_handler(int irq, void *data);

/*
*	below is drm dsi struct
*/
#define encoder_to_dsi(encoder) container_of(encoder, struct ameba_hw_dsi, encoder)

struct ameba_hw_dsi {
	struct drm_encoder encoder;
	struct drm_connector connector;
	struct drm_display_mode cur_mode;

	//mipi dsi register  & data struction
	MIPI_InitTypeDef dsi_ctx;
	void __iomem *mipi_reg;

	bool enable;
	//enum mipi_dsi_pixel_format format;
	//unsigned long mode_flags;

	int irq;
	int gpio;

	//display init flag
	u32 ST7701S_Init_Done_g ;
	u32 ST7701S_Send_cmd_g ;
};

static irqreturn_t mipi_dsi_underflow_irq(int irq, void *data)
{
	struct device *dev = (struct device *)drm_get_mipidsi_param2(data);
	struct ameba_hw_dsi *dsi = dev_get_drvdata(dev);
	void __iomem* pmipi_reg = dsi->mipi_reg ;

	u32 reg_val = MIPI_DSI_INTS_ACPU_Get(pmipi_reg);

	if (reg_val) {
		drm_underflow_flag_reset();
		MIPI_DSI_INT_Config(pmipi_reg, DISABLE, DISABLE, DISABLE);
		MIPI_DSI_INTS_ACPU_Clr(pmipi_reg, reg_val);

		/*reset the LCDC*/
		lcdc_underflow_reset(drm_get_lcdc_param());

		MIPI_DSI_Mode_Switch(pmipi_reg, ENABLE);
	}

	return IRQ_HANDLED; 
}

void mipi_dsi_underflow_reset()
{
#if  0
	InterruptRegister((IRQ_FUN)MipiDsi_UnderFlowReset, MIPI_DSI_IRQ, (u32)MIPI, 5);
	InterruptEn(MIPI_DSI_IRQ, 5);

	MIPI_DSI_INTS_ACPU_Clr(MIPI, MIPI_DSI_INTS_ACPU_Get(MIPI));
	MIPI_DSI_Mode_Switch(MIPI, DISABLE);
	MIPI_DSI_INT_Config(MIPI, ENABLE, ENABLE, ENABLE);
#endif
	int ret;
	struct device *dev = (struct device *)(drm_get_mipidsi_param());
	struct ameba_hw_dsi *dsi = dev_get_drvdata(dev);
	void __iomem* pmipi_reg = dsi->mipi_reg ;

	if(drm_get_dsi_flag() == 0)
	{
		disable_irq(dsi->irq);
		ret = devm_request_irq(dev,dsi->irq, mipi_dsi_underflow_irq, 0, dev_name(dev), drm_get_param_handle());
		if (ret)
		{
			DRM_ERROR("failed to set irq \n");
			return ;
		}
		drm_set_dsi_flag(1) ;
		enable_irq(dsi->irq);
	}

	MIPI_DSI_INTS_ACPU_Clr(pmipi_reg, MIPI_DSI_INTS_ACPU_Get(pmipi_reg));
	MIPI_DSI_Mode_Switch(pmipi_reg, DISABLE);
	MIPI_DSI_INT_Config(pmipi_reg, ENABLE, ENABLE, ENABLE);
}

static void dsi_gpio_set_pin(int gpio_index,u8 Newstatus)
{
	//int old,now;
	//old = gpio_get_value(gpio_index);
	gpio_set_value(gpio_index, Newstatus);
	//now = gpio_get_value(gpio_index);
//	pr_warn("gpio data ,old=%d,want=%d,now=%d\n",old,Newstatus,now);
}
static int dsi_gpio_reset(int iod)
{	
	int gpio_index = iod ;
	int req_status;
	int set_direct_status;
		
//	pr_info("gpio output test start:%d\n", iod);
	req_status = gpio_request(gpio_index, NULL);
	if (req_status != 0) {
		pr_warn("gpio request failed!\n");
		return -EINVAL;
	}

	set_direct_status = gpio_direction_output(gpio_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed\n");
		return -EINVAL;
	}

	/* to prevent electric leakage */
	dsi_gpio_set_pin(iod,1);
	mdelay(10);
	dsi_gpio_set_pin(iod,0);
	mdelay(10);
	dsi_gpio_set_pin(iod,1);
	gpio_free(gpio_index);

	mdelay(120);

	return  0 ;
}

/*
*	support two differete mode : cmd/vo
*/
/*
static void dsi_set_video_mode(MIPI_InitTypeDef *pdsi_ctx, unsigned long flags)
{
	DRM_MIPI_IN();
	if(pdsi_ctx)
		pdsi_ctx->MIPI_VideoNCmdMode = true;
	
	printk(KERN_DEBUG"MIPI video mode!\n");
	DRM_MIPI_OUT();
}
*/


static int dsi_mipi_init(struct device *dev, struct ameba_hw_dsi *dsi)
{
	int ret;
	void *pdev_id ;
	MIPI_InitTypeDef *pdsi_ctx = &(dsi->dsi_ctx);
	void __iomem* pmipi_reg = dsi->mipi_reg ;
	DRM_MIPI_IN();
#ifdef ENABLE_MIPI_CTL
	//TODO  maybe should put this in a task , to cut the timecost when kernel start @20211022
	
	// reset Core 
	MIPI_StructInit(pdsi_ctx);
	MIPI_InitStruct_Config(pdsi_ctx);
	MIPI_DPHY_init(pmipi_reg, pdsi_ctx);
	
	ret = dsi_gpio_reset(dsi->gpio);
	if (ret) {
		DRM_ERROR("Failed to set dis spio\n");
		return ret ;
	}

	pdev_id = drm_get_param_handle();
	//register MIPI ISR
	ret = devm_request_irq(dev,dsi->irq, mipi_dsi_irq_handler,0,dev_name(dev), pdev_id);
	if (ret)
	{
		DRM_ERROR("failed to do devm_request_irq \n");
		return -ENODEV;
	}
	//set command to dsi
	MipiDsi_ST7701S_push_table(pmipi_reg,pdsi_ctx,&(dsi->ST7701S_Init_Done_g),&(dsi->ST7701S_Send_cmd_g));
	//unregister MIPI ISR
	devm_free_irq(dev, dsi->irq, pdev_id);

#ifndef ENABLE_LCDC_CTL
	MIPI_DSI_Pat_Gen(pmipi_reg, ENABLE, 0, 0xFFFF00);
	MIPI_DSI_Mode_Switch(pmipi_reg, ENABLE);
#endif
#endif
	DRM_MIPI_OUT();

	return  0 ;
}
/*
* This callback should be used to disable the encoder. With the atomic
* drivers it is called before this encoder's CRTC has been shut off
* using their own &drm_crtc_helper_funcs.disable hook.
*/
static void dsi_encoder_disable(struct drm_encoder *encoder)
{
	void __iomem* pmipi_reg ;
	struct ameba_hw_dsi *dsi = encoder_to_dsi(encoder);
	DRM_MIPI_IN();
	if (NULL== dsi || !dsi->enable)
		return;

#ifdef ENABLE_MIPI_CTL	
	pmipi_reg = dsi->mipi_reg ;
	MIPI_DPHY_Reset(pmipi_reg);
#endif	
	dsi->enable = false;

	DRM_MIPI_OUT();
}

static void dsi_encoder_enable(struct drm_encoder *encoder)
{
	void __iomem* pmipi_reg ;
	MIPI_InitTypeDef *pdsi_ctx ;
	struct ameba_hw_dsi *dsi = encoder_to_dsi(encoder);
	DRM_MIPI_IN();
	if (NULL==dsi || dsi->enable)
		return;
	DRM_MIPI_TEST();
	pmipi_reg = dsi->mipi_reg;	
	pdsi_ctx = &(dsi->dsi_ctx);

#ifdef ENABLE_MIPI_CTL
	MIPI_StructInit(pdsi_ctx);
	MIPI_InitStruct_Config(pdsi_ctx);
	pdsi_ctx->MIPI_VideoNCmdMode = true;
	MIPI_Init(pmipi_reg,pdsi_ctx);
	DRM_TEST_PRINTK("[MIPI]here before %s-%d[0x%x] \n",__func__,__LINE__,readl(pmipi_reg+MIPI_MAIN_CTRL_OFFSET));

	MIPI_DSI_INT_Config(pmipi_reg,DISABLE,DISABLE, false);

	//test dis issue , will show the color 0xff0000
	//MIPI_DSI_Pat_Gen(pmipi_reg,ENABLE,7,0xFF0000);
	MIPI_DSI_Mode_Switch(pmipi_reg,ENABLE);
	DRM_TEST_PRINTK("[MIPI] dsi enable, here after %s-%d[0x%x] \n",__func__,__LINE__,readl(pmipi_reg+MIPI_MAIN_CTRL_OFFSET));
	MipiDumpRegValue(pmipi_reg);
#endif

	dsi->enable = true;

	DRM_MIPI_OUT();
}

static enum drm_mode_status dsi_encoder_mode_valid(struct drm_encoder *encoder,
					const struct drm_display_mode *mode)
{	
	DRM_MIPI_IN();
	DRM_MIPI_OUT();
	return MODE_OK;
}

static void dsi_encoder_mode_set(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adj_mode)
{
	struct ameba_hw_dsi *dsi = encoder_to_dsi(encoder);
	DRM_MIPI_IN();
	drm_mode_copy(&dsi->cur_mode, adj_mode);
	DRM_MIPI_OUT();
}

static int dsi_encoder_atomic_check(struct drm_encoder *encoder,
				    struct drm_crtc_state *crtc_state,
				    struct drm_connector_state *conn_state)
{
	/* do nothing */
	DRM_MIPI_IN();
	DRM_MIPI_OUT();
	return 0;
}

static const struct drm_encoder_helper_funcs ameba_encoder_helper_funcs = {
	.atomic_check	= dsi_encoder_atomic_check,
	.mode_valid	= dsi_encoder_mode_valid,
	.mode_set	= dsi_encoder_mode_set,
	
	.enable		= dsi_encoder_enable,
	.disable	= dsi_encoder_disable
};

static const struct drm_encoder_funcs ameba_encoder_funcs = {
	.destroy = drm_encoder_cleanup,
};

static int ameba_drm_encoder_init(struct device *dev,
			       struct drm_device *drm_dev,
			       struct drm_encoder *encoder)
{
	int ret;
	u32 crtc_mask  ;
	DRM_MIPI_IN();

	crtc_mask = drm_of_find_possible_crtcs(drm_dev, dev->of_node);
	if (!crtc_mask) {
		DRM_ERROR("failed to find crtc mask\n");
		return -EINVAL;
	}

	encoder->possible_crtcs = crtc_mask;
	ret = drm_encoder_init(drm_dev, encoder, &ameba_encoder_funcs,
			       DRM_MODE_ENCODER_DSI, NULL);
	if (ret) {
		DRM_ERROR("failed to init dsi encoder\n");
		return ret;
	}

	drm_encoder_helper_add(encoder, &ameba_encoder_helper_funcs);

	DRM_MIPI_OUT();
	return 0;
}
static int ameba_conn_get_modes(struct drm_connector *connector)
{
   int count;
   DRM_MIPI_IN();
   count = drm_add_modes_noedid(connector, LDCD_XRES_MAX, LDCD_YRES_MAX);
   drm_set_preferred_mode(connector, LCDC_TEST_IMG_BUF_X, LCDC_TEST_IMG_BUF_Y);
   DRM_TEST_PRINTK("[MIPI]here %s-%d [count=%d]\n",__func__,__LINE__,count);
   DRM_MIPI_OUT();
   return count;
}
static const struct drm_connector_funcs ameba_connector_funcs = {
   .fill_modes = drm_helper_probe_single_connector_modes,
   .destroy = drm_connector_cleanup,
   .reset = drm_atomic_helper_connector_reset,
   .atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
   .atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};
static const struct drm_connector_helper_funcs ameba_conn_helper_funcs = {
   .get_modes	 = ameba_conn_get_modes,
};

static int ameba_drm_connect_init(struct device *dev,
				  struct drm_device *drm_dev,
				  struct drm_connector *connector)
{
   int ret;
   DRM_MIPI_IN();

   //connector init
   ret = drm_connector_init(drm_dev, connector, &ameba_connector_funcs,
				DRM_MODE_CONNECTOR_DSI);
   if (ret) {
	   DRM_ERROR("Failed to init connector\n");
	   return 1;
   }
   
   drm_connector_helper_add(connector, &ameba_conn_helper_funcs);
   
   DRM_MIPI_OUT();
   return 0;
}

static int mipi_dsi_bind(struct device *dev, struct device *master, void *data)
{
	struct ameba_hw_dsi *dsi = dev_get_drvdata(dev);
	struct drm_device *drm_dev = data;
	int ret;
	DRM_MIPI_IN();

	//init encoder
	ret = ameba_drm_encoder_init(dev, drm_dev, &dsi->encoder);
	if (ret) {
		DRM_ERROR("Failed to init encoder\n");
		goto err_encoder;
	}
	
	//init encoder
	ret = ameba_drm_connect_init(dev, drm_dev, &dsi->connector);
	if (ret) {
		DRM_ERROR("Failed to init connector\n");
		goto err_connector;
	}

	ret = drm_connector_attach_encoder(&dsi->connector, &dsi->encoder);
	if (ret) {
		DRM_ERROR("Failed to attach connector to encoder\n");
		goto err_attach;
	}

	ret = dsi_mipi_init(dev, dsi);
	if (ret) {
		DRM_ERROR("Failed to init mipi dsi\n");
		goto err_mipi_init;
	}

	DRM_MIPI_OUT();
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
	// to do
	DRM_MIPI_IN();
}

static const struct component_ops mipi_dsi_ops = {
	.bind	= mipi_dsi_bind,
	.unbind	= mipi_dsi_unbind,
};
static irqreturn_t mipi_dsi_irq_handler(int irq, void *data)
{
	struct device *dev = (struct device *)drm_get_mipidsi_param2(data);
	struct ameba_hw_dsi *mipi_dsi = dev_get_drvdata(dev);
	void __iomem* MIPIx = mipi_dsi->mipi_reg ;

	u32 reg_val, reg_val2, reg_dphy_err,Value32;

	reg_val = MIPI_DSI_INTS_Get(MIPIx);
	MIPI_DSI_INTS_Clr(MIPIx, reg_val);

	reg_val2 = MIPI_DSI_INTS_ACPU_Get(MIPIx);
	MIPI_DSI_INTS_ACPU_Clr(MIPIx, reg_val2);

	if (reg_val & MIPI_BIT_CMD_TXDONE) { //frame done
		reg_val &= ~MIPI_BIT_CMD_TXDONE;

		mipi_dsi->ST7701S_Send_cmd_g = 1;
	}

	if (reg_val & MIPI_BIT_ERROR) {
		reg_dphy_err = readl(MIPIx+MIPI_DPHY_ERR_OFFSET);
		writel(reg_dphy_err,MIPIx+MIPI_DPHY_ERR_OFFSET) ;
		printk(KERN_DEBUG"LPTX Error: 0x%x, DPHY Error: 0x%x\n", reg_val, reg_dphy_err);

		Value32 = readl(MIPIx+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET) ;
		if (Value32 & MIPI_MASK_DETECT_ENABLE) {
			Value32 &= ~MIPI_MASK_DETECT_ENABLE;
			writel(Value32,MIPIx+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET);

			writel(reg_dphy_err,MIPIx+MIPI_DPHY_ERR_OFFSET);
			MIPI_DSI_INTS_Clr(MIPIx, MIPI_BIT_ERROR);
			printk(KERN_DEBUG"LPTX Error CLR: 0x%x, DPHY: 0x%x\n", readl(MIPIx+MIPI_INTS_OFFSET), readl(MIPIx+MIPI_DPHY_ERR_OFFSET));
		}

		if (readl(MIPIx+MIPI_DPHY_ERR_OFFSET) == reg_dphy_err) {
			printk(KERN_DEBUG"LPTX Still Error\n");
			MIPI_DSI_INT_Config(MIPIx, ENABLE, DISABLE, false);
		}
		reg_val &= ~MIPI_BIT_ERROR;
	}

	if (reg_val) {
		printk(KERN_DEBUG"LPTX Error Occur: 0x%x\n", reg_val);
	}

	if (reg_val2) {
		printk(KERN_DEBUG"#\n");
	}

	return IRQ_HANDLED;
}

static int mipi_dsi_parse_dt(struct platform_device *pdev, struct ameba_hw_dsi *dsi)
{
	struct resource *res;
	enum of_gpio_flags flags;
	struct device_node *np = pdev->dev.of_node;
	DRM_MIPI_IN();

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dsi->mipi_reg = devm_ioremap_resource(&pdev->dev, res);
	if (0 == dsi->mipi_reg) {
		DRM_ERROR("failed to remap dsi io region\n");
		return -ENODEV;
	}

	//irq info
	dsi->irq = platform_get_irq(pdev, 0);
	if (dsi->irq < 0) {
		DRM_ERROR("failed to get irq\n");
		return -ENODEV;
	}

	//gpio
	dsi->gpio = of_get_named_gpio_flags(np, "mipi-gpios", 0, &flags);
	if (!gpio_is_valid(dsi->gpio)) {
		pr_err("drm mipi dis node failed to get mipi-gpios\n");
		return -ENODEV;
	}
	
	DRM_MIPI_OUT();
	return 0;
}

static int mipi_dsi_probe(struct platform_device *pdev)
{
	struct ameba_hw_dsi* data;
	int ret;
	DRM_MIPI_IN();
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		DRM_ERROR("failed to allocate dsi data.\n");
		return -ENOMEM;
	}

	ret = mipi_dsi_parse_dt(pdev, data);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, data);

	drm_set_mipidsi_param(&pdev->dev);
	
	DRM_MIPI_OUT();
	return component_add(&pdev->dev, &mipi_dsi_ops);
}

static int mipi_dsi_remove(struct platform_device *pdev)
{
	DRM_MIPI_IN();
	component_del(&pdev->dev, &mipi_dsi_ops);
	DRM_MIPI_OUT();
	return 0;
}

static const struct of_device_id mipi_dsi_of_match[] = {
	{.compatible = "realtek,amebad2-dsi"},
	{ }
};

static struct platform_driver mipi_dsi_driver = {
	.probe = mipi_dsi_probe,
	.remove = mipi_dsi_remove,
	.driver = {
		.name = "amebad2-dsi",
		.of_match_table = mipi_dsi_of_match,
	},
};

module_platform_driver(mipi_dsi_driver);

MODULE_AUTHOR("Chunlin.Yi <chunlin.yi@realsil.com.cn>");
MODULE_DESCRIPTION("realtek AmebaD2 SoCs' MIPI DSI Controller driver");
MODULE_LICENSE("GPL v2");

