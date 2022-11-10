/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
#include "spatial_reuse.h"

u32 mac_sr_update(struct mac_ax_adapter *adapter,
		  struct mac_ax_sr_info *sr_info,
		  enum mac_ax_band band)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 val8;
	u32 val32, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val8 = MAC_REG_R8(REG_SR_CFG0);
	val8 = (sr_info->sr_en) ? (val8 | BIT_R_SR_EN) : (val8 & ~BIT_R_SR_EN);
	MAC_REG_W8(REG_SR_CFG0, val8);

	val32 = MAC_REG_R32(REG_MACTX_SR);
	val32 = (sr_info->sr_field_v15_allowed) ?
		(val32 | BIT_R_WMAC_SR_VAL15_ALLOW) :
		(val32 & ~BIT_R_WMAC_SR_VAL15_ALLOW);
	MAC_REG_W32(REG_MACTX_SR, val32);

	val32 = MAC_REG_R32(REG_SR_CFG1);
	val32 = BIT_SET_R_SRG_OBSS_PD_MIN(val32, sr_info->srg_obss_pd_min);
	val32 = BIT_SET_R_SRG_OBSS_PD_MAX(val32, sr_info->srg_obss_pd_max);
	val32 = BIT_SET_R_NONSRG_OBSS_PD_MIN(val32, sr_info->non_srg_obss_pd_min);
	val32 = BIT_SET_R_NONSRG_OBSS_PD_MAX(val32, sr_info->non_srg_obss_pd_max);
	MAC_REG_W32(REG_SR_CFG1, val32);

	MAC_REG_W32(REG_BSSCOLOR_BITMAP_L, sr_info->srg_bsscolor_bitmap_0);
	MAC_REG_W32(REG_BSSCOLOR_BITMAP_H, sr_info->srg_bsscolor_bitmap_1);

	MAC_REG_W32(REG_PBSSID_BITMAP_L, sr_info->srg_partbsid_bitmap_0);
	MAC_REG_W32(REG_PBSSID_BITMAP_H, sr_info->srg_partbsid_bitmap_1);

	return MACSUCCESS;
}

u32 spatial_reuse_init(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 val8;
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val8 = MAC_REG_R8(REG_SR_CFG0);
	val8 |= BIT_R_SR_EN;
	MAC_REG_W8(REG_SR_CFG0, val8);

	return MACSUCCESS;
}

