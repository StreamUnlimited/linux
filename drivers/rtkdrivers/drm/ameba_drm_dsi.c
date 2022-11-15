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
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
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
#define encoder_to_dsi(encoder) container_of(encoder, struct ameba_hw_dsi, encoder)

struct ameba_hw_dsi {
	struct drm_encoder encoder;
	struct drm_connector connector;
	struct drm_display_mode cur_mode;

	//mipi dsi register  & data struction
	MIPI_InitTypeDef dsi_ctx;
	void __iomem *mipi_reg;

	u32 mipi_ckd;
	void __iomem *sysctl_base_lp;//vo clk div

	bool enable;

	int irq;
	int gpio;

	u32 dsi_send_cmd;//command tx done

	u32 dsi_debug;
};

static ssize_t dsi_debug_show(struct device *dev, struct device_attribute*attr, char *buf)
{
	struct ameba_hw_dsi 		*dsi = dev_get_drvdata(dev);
	return sprintf(buf, "dsi_debug=%d\n", dsi->dsi_debug);
}
static ssize_t dsi_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct ameba_hw_dsi 		*dsi = dev_get_drvdata(dev);
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


static irqreturn_t mipi_dsi_underflow_irq(int irq, void *data)
{
	struct device *dev = (struct device *)drm_get_mipidsi_param2(data);
	struct ameba_hw_dsi *dsi = dev_get_drvdata(dev);
	void __iomem *pmipi_reg = dsi->mipi_reg ;
	
	u32 reg_val = MIPI_DSI_INTS_ACPU_Get(pmipi_reg);
	MIPI_DSI_INTS_ACPU_Clr(pmipi_reg, reg_val);

	if (reg_val) {
		DRM_DEV_INFO(dev, "count=%d[0x%x]\n",
			drm_get_underflow_flag(),readl((void*)(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0)));

		MIPI_DSI_INT_Config(pmipi_reg, DISABLE, DISABLE, DISABLE);

		/*reset the LCDC*/
		lcdc_underflow_reset(drm_get_lcdc_param());
		drm_underflow_flag_reset();

		MIPI_DSI_Mode_Switch(pmipi_reg, ENABLE);

		if(dsi->dsi_debug){
			MipiDumpRegValue(pmipi_reg);
		}
	}

	return IRQ_HANDLED; 
}

void mipi_dsi_underflow_reset()
{
	struct device 		*dev = (struct device *)(drm_get_mipidsi_param());
	struct ameba_hw_dsi *dsi = dev_get_drvdata(dev);
	void __iomem		*pmipi_reg = dsi->mipi_reg ;

	MIPI_DSI_INTS_ACPU_Clr(pmipi_reg, MIPI_DSI_INTS_ACPU_Get(pmipi_reg));
	MIPI_DSI_INT_Config(pmipi_reg, ENABLE, DISABLE, ENABLE);//enable acpu, disable scpu
	MIPI_DSI_Mode_Switch(pmipi_reg, DISABLE);
}

static irqreturn_t mipi_dsi_irq_handler(int irq, void *data)
{
	u32 reg_val, reg_val2, reg_dphy_err,Value32;
	struct device 			*dev = (struct device *)drm_get_mipidsi_param2(data);
	struct ameba_hw_dsi 	*mipi_dsi = dev_get_drvdata(dev);
	void __iomem			*mipi_reg = mipi_dsi->mipi_reg ;

	reg_val = MIPI_DSI_INTS_Get(mipi_reg);
	MIPI_DSI_INTS_Clr(mipi_reg, reg_val);

	reg_val2 = MIPI_DSI_INTS_ACPU_Get(mipi_reg);
	MIPI_DSI_INTS_ACPU_Clr(mipi_reg, reg_val2);

	if (reg_val & MIPI_BIT_CMD_TXDONE) { //command tx done
		reg_val &= ~MIPI_BIT_CMD_TXDONE;
		mipi_dsi->dsi_send_cmd = 1;
	}

	if (reg_val & MIPI_BIT_ERROR) {
		reg_dphy_err = readl(mipi_reg+MIPI_DPHY_ERR_OFFSET);
		writel(reg_dphy_err,mipi_reg+MIPI_DPHY_ERR_OFFSET) ;
		printk(KERN_WARNING"LPTX ISSUE: 0x%x, DPHY ISSUE: 0x%x\n", reg_val, reg_dphy_err);

		Value32 = readl(mipi_reg+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET) ;
		if (Value32 & MIPI_MASK_DETECT_ENABLE) {
			Value32 &= ~MIPI_MASK_DETECT_ENABLE;
			writel(Value32,mipi_reg+MIPI_CONTENTION_DETECTOR_AND_STOPSTATE_DT_OFFSET);

			writel(reg_dphy_err,mipi_reg+MIPI_DPHY_ERR_OFFSET);
			MIPI_DSI_INTS_Clr(mipi_reg, MIPI_BIT_ERROR);
			printk(KERN_WARNING"LPTX ISSUE CLR: 0x%x, DPHY: 0x%x\n", readl(mipi_reg+MIPI_INTS_OFFSET), readl(mipi_reg+MIPI_DPHY_ERR_OFFSET));
		}

		if (readl(mipi_reg+MIPI_DPHY_ERR_OFFSET) == reg_dphy_err) {
			printk(KERN_WARNING"LPTX Still Error\n");
			MIPI_DSI_INT_Config(mipi_reg, ENABLE, DISABLE, false);
		}
		reg_val &= ~MIPI_BIT_ERROR;
	}

	if (reg_val) {
		printk(KERN_DEBUG"LPTX Error Occur: 0x%x\n", reg_val);
	}

	if (reg_val2) {
		printk(KERN_DEBUG"ACPU interrupt Occur: 0x%x\n", reg_val2);
	}

	return IRQ_HANDLED;
}

static inline void dsi_gpio_set_pin(int gpio_index,u8 Newstatus)
{
	gpio_set_value(gpio_index, Newstatus);
}
static int dsi_gpio_reset(int iod)
{	
	int req_status;
	int set_direct_status;
	int gpio_index = iod ;	

	req_status = gpio_request(gpio_index, NULL);
	if (req_status != 0) {
		DRM_WARN("gpio request failed!\n");
		return -EINVAL;
	}

	set_direct_status = gpio_direction_output(gpio_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		DRM_WARN("set gpio direction output failed\n");
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

static int dsi_mipi_init(struct device *dev, struct ameba_hw_dsi *dsi)
{
	int ret;
	void *pdev_id ;
	u32 reg_value, reg_tmp;

	MIPI_InitTypeDef 	*pdsi_ctx = &(dsi->dsi_ctx);
	void __iomem		*pmipi_reg = dsi->mipi_reg ;

	//TODO  maybe should put this in a task , to decrease the timecost when kernel start @20211022	
	// reset Core
	MIPI_StructInit(pdsi_ctx);
	MIPI_InitStruct_Config(pdsi_ctx,ameba_drm_get_display_width(),ameba_drm_get_display_height(),ameba_drm_get_framerate(),&(dsi->mipi_ckd));
	MIPI_DPHY_init(pmipi_reg, pdsi_ctx);

	ret = dsi_gpio_reset(dsi->gpio);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to set dis spio\n");
		return ret ;
	}

	//set the vo div
	reg_value = ameba_lcdc_reg_read(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0);
	reg_tmp = reg_value;
	reg_value &= ~LSYS_MASK_CKD_MIPI;
	reg_value |= LSYS_CKD_MIPI(dsi->mipi_ckd);
	ameba_lcdc_reg_write(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0, reg_value);
	DRM_DEV_INFO(dev, "0x%x-0x%x\n", reg_tmp, ameba_lcdc_reg_read(dsi->sysctl_base_lp + REG_LSYS_CKD_GRP0));

	pdev_id = drm_get_param_handle();
	//register MIPI ISR
	ret = devm_request_irq(dev,dsi->irq, mipi_dsi_irq_handler,0,dev_name(dev), pdev_id);
	if (ret)
	{
		DRM_DEV_ERROR(dev, "failed to set irq for dsi command \n");
		return -ENODEV;
	}
	//enable_irq(dsi->irq);
	MipiDsi_Do_Init(pmipi_reg,pdsi_ctx,&(dsi->dsi_send_cmd));//set command to dsi
	//disable_irq(dsi->irq);

	devm_free_irq(dev, dsi->irq, pdev_id);//unregister dsi ISR
	ret = devm_request_irq(dev,dsi->irq, mipi_dsi_underflow_irq, 0, dev_name(dev), pdev_id);
	if (ret)
	{
		DRM_DEV_ERROR(dev, "failed to set irq for underflow \n");
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
	void __iomem			*pmipi_reg ;
	struct ameba_hw_dsi 	*dsi = encoder_to_dsi(encoder);

	if( NULL == dsi || !dsi->enable)
		return;

	pmipi_reg = dsi->mipi_reg ;
	MIPI_DPHY_Reset(pmipi_reg);

	dsi->enable = false;
}

static void dsi_encoder_enable(struct drm_encoder *encoder)
{
	void __iomem		*pmipi_reg ;
	MIPI_InitTypeDef 	*pdsi_ctx ;
	struct ameba_hw_dsi *dsi = encoder_to_dsi(encoder);

	if( NULL == dsi || dsi->enable)
		return;

	pmipi_reg = dsi->mipi_reg;	
	pdsi_ctx = &(dsi->dsi_ctx);

	pdsi_ctx->MIPI_VideoNCmdMode = true;

	MIPI_DSI_INT_Config(pmipi_reg,DISABLE,DISABLE, false);
	MIPI_DSI_Mode_Switch(pmipi_reg,ENABLE);

	if(dsi->dsi_debug){
		MipiDumpRegValue(pmipi_reg);
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
	struct ameba_hw_dsi *dsi = encoder_to_dsi(encoder);

	drm_mode_copy(&dsi->cur_mode, adj_mode);
}

static int dsi_encoder_atomic_check(struct drm_encoder *encoder,
				    struct drm_crtc_state *crtc_state,
				    struct drm_connector_state *conn_state)
{
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

	crtc_mask = drm_of_find_possible_crtcs(drm_dev, dev->of_node);
	if (!crtc_mask) {
		DRM_DEV_ERROR(dev, "failed to find crtc mask\n");
		return -EINVAL;
	}

	encoder->possible_crtcs = crtc_mask;
	ret = drm_encoder_init(drm_dev, encoder, &ameba_encoder_funcs,
			       DRM_MODE_ENCODER_DSI, NULL);
	if (ret) {
		DRM_DEV_ERROR(dev, "failed to init dsi encoder\n");
		return ret;
	}

	drm_encoder_helper_add(encoder, &ameba_encoder_helper_funcs);

	return 0;
}
static int ameba_conn_get_modes(struct drm_connector *connector)
{
   int count;

   count = drm_add_modes_noedid(connector, ameba_drm_get_display_width(), ameba_drm_get_display_height());
   drm_set_preferred_mode(connector, ameba_drm_get_display_width(), ameba_drm_get_display_height());

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

   //connector init
   ret = drm_connector_init(drm_dev, connector, &ameba_connector_funcs,
				DRM_MODE_CONNECTOR_DSI);
   if (ret) {
	   DRM_DEV_ERROR(dev, "Failed to init connector\n");
	   return 1;
   }
   
   drm_connector_helper_add(connector, &ameba_conn_helper_funcs);
   
   return 0;
}

static int mipi_dsi_bind(struct device *dev, struct device *master, void *data)
{
	struct ameba_hw_dsi 	*dsi = dev_get_drvdata(dev);
	struct drm_device 		*drm_dev = data;
	int ret;

	//init encoder
	ret = ameba_drm_encoder_init(dev, drm_dev, &dsi->encoder);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to init encoder\n");
		goto err_encoder;
	}
	
	//init encoder
	ret = ameba_drm_connect_init(dev, drm_dev, &dsi->connector);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to init connector\n");
		goto err_connector;
	}

	ret = drm_connector_attach_encoder(&dsi->connector, &dsi->encoder);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to attach connector to encoder\n");
		goto err_attach;
	}

	ret = dsi_mipi_init(dev, dsi);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to init mipi dsi\n");
		goto err_mipi_init;
	}

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
}

static const struct component_ops mipi_dsi_ops = {
	.bind	= mipi_dsi_bind,
	.unbind	= mipi_dsi_unbind,
};

static int mipi_dsi_parse_dt(struct platform_device *pdev, struct ameba_hw_dsi *dsi)
{
	struct resource 		*res;
	enum of_gpio_flags 		flags;
	struct device 			*dev = &pdev->dev;
	struct device_node 		*np = dev->of_node;
	struct device_node 		*rcc_node;
	struct resource 		rcc_res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dsi->mipi_reg = devm_ioremap_resource(&pdev->dev, res);
	if (0 == dsi->mipi_reg) {
		DRM_DEV_ERROR(dev, "failed to remap dsi io region\n");
		return -ENODEV;
	}

	//irq info
	dsi->irq = platform_get_irq(pdev, 0);
	if (dsi->irq < 0) {
		DRM_DEV_ERROR(dev, "failed to get irq\n");
		return -ENODEV;
	}

	//gpio
	dsi->gpio = of_get_named_gpio_flags(np, "mipi-gpios", 0, &flags);
	if (!gpio_is_valid(dsi->gpio)) {
		pr_err("drm mipi dis node failed to get mipi-gpios\n");
		return -ENODEV;
	}

	rcc_node = of_parse_phandle(dev->of_node, "rtk,lcdc-vo-clock", 0);
	if ( of_address_to_resource(rcc_node, 1, &rcc_res) ) {
		dev_err(&pdev->dev, "get resource for lcdc vo clock error\n");
		return -ENOMEM;
	}

	dsi->sysctl_base_lp = ioremap(rcc_res.start, resource_size(&rcc_res));
	if ( !dsi->sysctl_base_lp ) {
			dev_err(&pdev->dev, "ioremap for lcdc vo clock  error\n");
			return -ENOMEM;
	}

	return 0;
}

static int mipi_dsi_probe(struct platform_device *pdev)
{
	int ret;
	struct ameba_hw_dsi		*data;
	struct device 			*dev = &pdev->dev;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		DRM_DEV_ERROR(dev, "failed to allocate dsi data.\n");
		return -ENOMEM;
	}

	ret = mipi_dsi_parse_dt(pdev, data);
	if (ret)
		return ret;

	data->dsi_debug = 0;
	if ( sysfs_create_group(&pdev->dev.kobj,&dsi_debug_attr_grp) )
		DRM_DEV_INFO(&pdev->dev, "Error creating dsi sysfs entry\n");

	platform_set_drvdata(pdev, data);

	drm_set_mipidsi_param(&pdev->dev);

	return component_add(&pdev->dev, &mipi_dsi_ops);
}

static int mipi_dsi_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &mipi_dsi_ops);

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

