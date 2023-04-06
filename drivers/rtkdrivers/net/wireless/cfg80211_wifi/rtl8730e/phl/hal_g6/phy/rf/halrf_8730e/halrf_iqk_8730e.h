/******************************************************************************
 *
 * Copyright(c) 2007 - 2021  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALRF_IQK_8730EH__
#define __HALRF_IQK_8730EH__
#ifdef RF_8730E_SUPPORT

#define iqk_version_8730e 0xc
#define ss_8730e 1
#define IQK_THR_ReK 0x20

void iqk_init_8730e(struct rf_info *rf);
void iqk_get_ch_info_8730e(struct rf_info *rf, enum phl_phy_idx phy, u8 path);
void iqk_macbb_setting_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void iqk_preset_8730e(struct rf_info *rf, u8 path);
void iqk_start_iqk_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void iqk_restore_8730e(struct rf_info *rf, u8 path);
void iqk_afebb_restore_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void halrf_iqk_track_8730e(struct rf_info *rf);
bool halrf_iqk_get_ther_rek_8730e(struct rf_info *rf);
u32 halrf_get_iqk_ver_8730e(void);

#endif
#endif /*  __HALRF_IQK_8730EH__ */

