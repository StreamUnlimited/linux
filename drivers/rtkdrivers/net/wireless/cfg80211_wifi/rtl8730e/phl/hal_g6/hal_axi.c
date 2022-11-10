/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _HAL_AXI_C_
#include "hal_headers.h"

#ifdef CONFIG_AXI_HCI
static u8 axi_read8(struct rtw_hal_com_t *hal, u32 addr)
{
	return _os_read8_axi(hal->drv_priv, addr);
}
static u16 axi_read16(struct rtw_hal_com_t *hal, u32 addr)
{
	return _os_read16_axi(hal->drv_priv, addr);
}
static u32 axi_read32(struct rtw_hal_com_t *hal, u32 addr)
{
	return _os_read32_axi(hal->drv_priv, addr);
}

static int axi_write8(struct rtw_hal_com_t *hal, u32 addr, u8 val)
{
	return _os_write8_axi(hal->drv_priv, addr, val);
}
static int axi_write16(struct rtw_hal_com_t *hal, u32 addr, u16 val)
{
	return _os_write16_axi(hal->drv_priv, addr, val);
}
static int axi_write32(struct rtw_hal_com_t *hal, u32 addr, u32 val)
{
	return _os_write32_axi(hal->drv_priv, addr, val);
}

static u8 axi_sys_read8(struct rtw_hal_com_t *hal, u32 base, u32 addr)
{
	return _os_sys_read8_axi(hal->drv_priv, base, addr);
}
static u16 axi_sys_read16(struct rtw_hal_com_t *hal, u32 base, u32 addr)
{
	return _os_sys_read16_axi(hal->drv_priv, base, addr);
}
static u32 axi_sys_read32(struct rtw_hal_com_t *hal, u32 base, u32 addr)
{
	return _os_sys_read32_axi(hal->drv_priv, base, addr);
}

static int axi_sys_write8(struct rtw_hal_com_t *hal, u32 base, u32 addr, u8 val)
{
	return _os_sys_write8_axi(hal->drv_priv, base, addr, val);
}
static int axi_sys_write16(struct rtw_hal_com_t *hal, u32 base, u32 addr, u16 val)
{
	return _os_sys_write16_axi(hal->drv_priv, base, addr, val);
}
static int axi_sys_write32(struct rtw_hal_com_t *hal, u32 base, u32 addr, u32 val)
{
	return _os_sys_write32_axi(hal->drv_priv, base, addr, val);
}

#ifdef RTW_WKARD_BUS_WRITE
static int axi_write_post_cfg(struct rtw_hal_com_t *hal, u32 addr, u32 val)
{
	struct hal_info_t	*hal_info = hal->hal_priv;
	struct hal_ops_t	*hal_ops = hal_get_ops(hal_info);

	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	if (NULL != hal_ops->write_reg_post_cfg) {
		hal_status = hal_ops->write_reg_post_cfg(hal_info, addr, val);
	}

	return hal_status;
}
#endif
void hal_axi_set_io_ops(struct rtw_hal_com_t *hal, struct hal_io_ops *pops)
{
	/*_rtw_memset((u8 *)pops, 0, sizeof(struct hal_io_ops));*/
	_os_mem_set(hal->drv_priv, (u8 *)pops, 0, sizeof(struct hal_io_ops));

	pops->_read8 = &axi_read8;
	pops->_read16 = &axi_read16;
	pops->_read32 = &axi_read32;

	pops->_sys_read8 = &axi_sys_read8;
	pops->_sys_read16 = &axi_sys_read16;
	pops->_sys_read32 = &axi_sys_read32;

	pops->_write8 = &axi_write8;
	pops->_write16 = &axi_write16;
	pops->_write32 = &axi_write32;

	pops->_sys_write8 = &axi_sys_write8;
	pops->_sys_write16 = &axi_sys_write16;
	pops->_sys_write32 = &axi_sys_write32;

#ifdef RTW_WKARD_BUS_WRITE
	pops->_write_post_cfg = &axi_write_post_cfg;
#endif
}

void rtw_hal_clear_bdidx(void *hal)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	FUNCIN();

	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mac_clr_bdidx(hal_info)) {
		PHL_ERR("%s failure \n", __func__);
	}
}


void rtw_hal_cfg_dma_io(void *hal, u8 en)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : enable %u.\n", __func__, en);

	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mac_cfg_dma_io(hal_info, en)) {
		PHL_ERR("%s failure \n", __func__);
	}
}

#ifdef RTW_WKARD_DYNAMIC_LTR
enum rtw_hal_status
rtw_hal_ltr_en_hw_mode(void *hal, bool hw_mode) {
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	status = rtw_hal_mac_ltr_set_pcie(hal_info,
					  hw_mode ? RTW_PCIE_BUS_FUNC_ENABLE : RTW_PCIE_BUS_FUNC_DISABLE,
					  0, 0, 0, 0);

	if (status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s : fail\n", __func__);
		return status;
	}

	return status;
}

#endif
#endif /*CONFIG_AXI_HCI*/
