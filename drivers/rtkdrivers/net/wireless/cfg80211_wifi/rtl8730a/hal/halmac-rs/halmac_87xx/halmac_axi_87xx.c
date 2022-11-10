/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "halmac_axi_87xx.h"
#include "halmac_efuse_87xx.h"

#if (HALMAC_87XX_SUPPORT && HALMAC_AXI_SUPPORT)


/**
 * init_axi_cfg_87xx() -  init PCIe
 * @adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
init_axi_cfg_87xx(struct halmac_adapter *adapter) {

	return HALMAC_RET_SUCCESS;
}

/**
 * deinit_axi_cfg_87xx() - deinit AXI
 * @adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
deinit_axi_cfg_87xx(struct halmac_adapter *adapter) {
	return HALMAC_RET_SUCCESS;
}

/**
 * cfg_axi_rx_agg_87xx() - config rx aggregation
 * @adapter : the adapter of halmac
 * @halmac_rx_agg_mode
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
cfg_axi_rx_agg_87xx(struct halmac_adapter *adapter,
		    struct halmac_rxagg_cfg *cfg) {
	return HALMAC_RET_SUCCESS;
}

/**
 * reg_r8_axi_87xx() - read 1byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
u8
reg_r8_axi_87xx(struct halmac_adapter *adapter, u32 offset)
{
	return PLTFM_REG_R8(offset);
}

u8
sys_reg_r8_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset)
{
	return PLTFM_SYS_REG_R8(base, offset);
}

/**
 * reg_w8_axi_87xx() - write 1byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * @value : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
reg_w8_axi_87xx(struct halmac_adapter *adapter, u32 offset, u8 value) {
	PLTFM_REG_W8(offset, value);

	return HALMAC_RET_SUCCESS;
}

enum halmac_ret_status
sys_reg_w8_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u8 value) {
	PLTFM_SYS_REG_W8(base, offset, value);

	return HALMAC_RET_SUCCESS;
}


/**
 * reg_r16_axi_87xx() - read 2byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
u16
reg_r16_axi_87xx(struct halmac_adapter *adapter, u32 offset)
{
	return PLTFM_REG_R16(offset);
}

u16
sys_reg_r16_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset)
{
	return PLTFM_SYS_REG_R16(base, offset);
}

/**
 * reg_w16_axi_87xx() - write 2byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * @value : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
reg_w16_axi_87xx(struct halmac_adapter *adapter, u32 offset, u16 value) {
	PLTFM_REG_W16(offset, value);

	return HALMAC_RET_SUCCESS;
}

enum halmac_ret_status
sys_reg_w16_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u16 value) {
	PLTFM_SYS_REG_W16(base, offset, value);

	return HALMAC_RET_SUCCESS;
}

/**
 * reg_r32_axi_87xx() - read 4byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
u32
reg_r32_axi_87xx(struct halmac_adapter *adapter, u32 offset)
{
	return PLTFM_REG_R32(offset);
}

u32
sys_reg_r32_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset)
{
	return PLTFM_SYS_REG_R32(base, offset);
}

/**
 * reg_w32_axi_87xx() - write 4byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * @value : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
reg_w32_axi_87xx(struct halmac_adapter *adapter, u32 offset, u32 value) {
	PLTFM_REG_W32(offset, value);

	return HALMAC_RET_SUCCESS;
}

enum halmac_ret_status
sys_reg_w32_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u32 value) {
	PLTFM_SYS_REG_W32(base, offset, value);

	return HALMAC_RET_SUCCESS;
}

/**
 * cfg_txagg_axi_align_87xx() -config sdio bus tx agg alignment
 * @adapter : the adapter of halmac
 * @enable : function enable(1)/disable(0)
 * @align_size : sdio bus tx agg alignment size (2^n, n = 3~11)
 * Author : Soar Tu
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
cfg_txagg_axi_align_87xx(struct halmac_adapter *adapter, u8 enable,
			 u16 align_size) {
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * tx_allowed_axi_87xx() - check tx status
 * @adapter : the adapter of halmac
 * @buf : tx packet, include txdesc
 * @size : tx packet size, include txdesc
 * Author : Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
tx_allowed_axi_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size) {
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * axi_indirect_reg_r32_87xx() - read MAC reg by SDIO reg
 * @adapter : the adapter of halmac
 * @offset : register offset
 * Author : Soar
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
u32
axi_indirect_reg_r32_87xx(struct halmac_adapter *adapter, u32 offset)
{
	return 0xFFFFFFFF;
}

/**
 * axi_reg_rn_87xx() - read n byte register
 * @adapter : the adapter of halmac
 * @offset : register offset
 * @size : register value size
 * @value : register value
 * Author : Soar
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
axi_reg_rn_87xx(struct halmac_adapter *adapter, u32 offset, u32 size,
		u8 *value) {
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * set_axi_bulkout_num_87xx() - inform bulk-out num
 * @adapter : the adapter of halmac
 * @num : usb bulk-out number
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
set_axi_bulkout_num_87xx(struct halmac_adapter *adapter, u8 num) {
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * get_axi_tx_addr_87xx() - get CMD53 addr for the TX packet
 * @adapter : the adapter of halmac
 * @buf : tx packet, include txdesc
 * @size : tx packet size
 * @cmd53_addr : cmd53 addr value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
get_axi_tx_addr_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size,
		     u32 *cmd53_addr) {
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * get_axi_bulkout_id_87xx() - get bulk out id for the TX packet
 * @adapter : the adapter of halmac
 * @buf : tx packet, include txdesc
 * @size : tx packet size
 * @id : usb bulk-out id
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
get_axi_bulkout_id_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size,
			u8 *id) {
	return HALMAC_RET_NOT_SUPPORT;
}

enum halmac_ret_status
trxdma_check_idle_87xx(struct halmac_adapter *adapter) {
	u32 cnt = 0;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;

	/* Stop Tx & Rx DMA */
	HALMAC_REG_W32_SET(REG_RXPKT_NUM, BIT(18));
	HALMAC_REG_W16_SET(REG_AXI_CTRL, ~(BIT(15) | BIT(8)));

	/* Stop FW */
	HALMAC_REG_W32_CLR(REG_WL_FUNC_EN, BIT_FEN_WLMCU);

	/* Check Tx DMA is idle */
	cnt = 20;
	while ((HALMAC_REG_R8(REG_TXDMA_STATUS) & BIT_SFF_UDN) == BIT_SFF_UDN)
	{
		PLTFM_DELAY_US(10);
		cnt--;
		if (cnt == 0) {
			PLTFM_MSG_ERR("[ERR]Chk tx idle\n");
			return HALMAC_RET_POWER_OFF_FAIL;
		}
	}

	/* Check Rx DMA is idle */
	cnt = 20;
	while ((HALMAC_REG_R32(REG_RXPKT_NUM) & BIT(17)) != BIT(17))
	{
		PLTFM_DELAY_US(10);
		cnt--;
		if (cnt == 0) {
			PLTFM_MSG_ERR("[ERR]Chk rx idle\n");
			return HALMAC_RET_POWER_OFF_FAIL;
		}
	}

	return HALMAC_RET_SUCCESS;
}

enum halmac_ret_status
en_ref_autok_axi_87xx(struct halmac_adapter *adapter, u8 en) {
	if (en == 1)
	{
		adapter->axi_refautok_en = 1;
	} else
	{
		adapter->axi_refautok_en = 0;
	}
	return HALMAC_RET_SUCCESS;
}

#endif /* HALMAC_87XX_SUPPORT */
