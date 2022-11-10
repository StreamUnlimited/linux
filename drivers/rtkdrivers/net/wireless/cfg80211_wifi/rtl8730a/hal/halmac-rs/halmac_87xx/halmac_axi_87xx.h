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

#ifndef _HALMAC_AXI_87XX_H_
#define _HALMAC_AXI_87XX_H_

#include "../halmac_api.h"

#if (HALMAC_87XX_SUPPORT && HALMAC_AXI_SUPPORT)

enum halmac_ret_status
init_axi_cfg_87xx(struct halmac_adapter *adapter);

enum halmac_ret_status
deinit_axi_cfg_87xx(struct halmac_adapter *adapter);

enum halmac_ret_status
cfg_axi_rx_agg_87xx(struct halmac_adapter *adapter,
		    struct halmac_rxagg_cfg *cfg);

u8
reg_r8_axi_87xx(struct halmac_adapter *adapter, u32 offset);

enum halmac_ret_status
reg_w8_axi_87xx(struct halmac_adapter *adapter, u32 offset, u8 value);

u16
reg_r16_axi_87xx(struct halmac_adapter *adapter, u32 offset);

enum halmac_ret_status
reg_w16_axi_87xx(struct halmac_adapter *adapter, u32 offset, u16 value);

u32
reg_r32_axi_87xx(struct halmac_adapter *adapter, u32 offset);

enum halmac_ret_status
reg_w32_axi_87xx(struct halmac_adapter *adapter, u32 offset, u32 value);

u8
sys_reg_r8_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset);

enum halmac_ret_status
sys_reg_w8_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u8 value);

u16
sys_reg_r16_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset);

enum halmac_ret_status
sys_reg_w16_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u16 value);

u32
sys_reg_r32_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset);

enum halmac_ret_status
sys_reg_w32_axi_87xx(struct halmac_adapter *adapter, u32 base, u32 offset, u32 value);

enum halmac_ret_status
cfg_txagg_axi_align_87xx(struct halmac_adapter *adapter, u8 enable,
			 u16 align_size);

enum halmac_ret_status
tx_allowed_axi_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size);

u32
axi_indirect_reg_r32_87xx(struct halmac_adapter *adapter, u32 offset);

enum halmac_ret_status
axi_reg_rn_87xx(struct halmac_adapter *adapter, u32 offset, u32 size,
		u8 *value);

enum halmac_ret_status
set_axi_bulkout_num_87xx(struct halmac_adapter *adapter, u8 num);

enum halmac_ret_status
get_axi_tx_addr_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size,
		     u32 *cmd53_addr);

enum halmac_ret_status
get_axi_bulkout_id_87xx(struct halmac_adapter *adapter, u8 *buf, u32 size,
			u8 *id);

enum halmac_ret_status
trxdma_check_idle_87xx(struct halmac_adapter *adapter);

enum halmac_ret_status
en_ref_autok_axi_87xx(struct halmac_adapter *dapter, u8 en);

#endif /* HALMAC_87XX_SUPPORT */

#endif/* _HALMAC_AXI_87XX_H_ */
