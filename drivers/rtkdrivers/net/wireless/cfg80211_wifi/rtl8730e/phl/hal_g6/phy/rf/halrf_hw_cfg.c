/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
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
#include "halrf_precomp.h"

bool halrf_init_reg_by_hdr(void *rf_void)
{
	bool result = true;

	return result;
}

bool halrf_nctl_init_reg_by_hdr(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;

	bool result = true;
#ifdef RF_8852A_SUPPORT
	if (rf->ic_type == RF_RTL8852A)	{
		halrf_config_8852a_nctl_reg(rf);
	}
#endif
#ifdef RF_8852B_SUPPORT
	if (rf->ic_type == RF_RTL8852B) {
		halrf_config_8852b_nctl_reg(rf);
	}
#endif
#ifdef RF_8832BR_SUPPORT
	if (rf->ic_type == RF_RTL8832BR) {
		halrf_config_8832br_nctl_reg(rf);
	}
#endif
#ifdef RF_8192XB_SUPPORT
	if (rf->ic_type == RF_RTL8192XB) {
		halrf_config_8192xb_nctl_reg(rf);
	}
#endif
#ifdef RF_8852BP_SUPPORT
	if (rf->ic_type == RF_RTL8852BP) {
		halrf_config_8852bp_nctl_reg(rf);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (rf->ic_type == RF_RTL8730E) {
		halrf_config_8730e_nctl_reg(rf);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (rf->ic_type == RF_RTL8720E) {
		halrf_config_8720e_nctl_reg(rf);
	}
#endif
	return result;
}

bool halrf_config_radio_a_reg(void *rf_void, bool is_form_folder,
			      u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_radio_a_reg(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (rf->ic_type == RF_RTL8730E) {
		halrf_config_8730e_radio_a_reg(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (rf->ic_type == RF_RTL8720E) {
		halrf_config_8720e_radio_a_reg(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_radio_b_reg(void *rf_void, bool is_form_folder,
			      u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_radio_b_reg(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_radio_b_reg(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_radio_b_reg(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_radio_b_reg(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_radio_b_reg(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_radio_b_reg(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_store_power_by_rate(void *rf_void,
				      bool is_form_folder, u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_by_rate(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_by_rate(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_by_rate(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_store_power_limit(void *rf_void,
				    bool is_form_folder, u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_limit(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_limit(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_limit(rf, 0);
		halrf_config_8852c_store_power_limit_6g(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_limit(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_limit(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_limit(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_limit(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_limit(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_store_power_limit_ru(void *rf_void,
				       bool is_form_folder, u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_limit_ru(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_limit_ru(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_limit_ru(rf, 0);
		halrf_config_8852c_store_power_limit_ru_6g(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_limit_ru(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_limit_ru(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_limit_ru(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_limit_ru(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_limit_ru(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_store_power_track(void *rf_void,
				    bool is_form_folder, u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_pwr_track(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_pwr_track(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_pwr_track(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_store_xtal_track(void *rf_void,
				   bool is_form_folder, u32 folder_len, u32 *folder_array)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_xtal_track(rf, 0);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_xtal_track(rf, 0);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_xtal_track(rf, 0);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_xtal_track(rf, 0);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_xtal_track(rf, 0);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_xtal_track(rf, 0);
	}
#endif

	return result;
}

bool halrf_config_radio(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_radio_a_reg(rf, phy);
		halrf_config_8852a_radio_b_reg(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_radio_a_reg(rf, phy);
		halrf_config_8852b_radio_b_reg(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_radio_b_reg(rf, phy);
		halrf_config_8852c_radio_a_reg(rf, phy);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_radio_a_reg(rf, phy);
		halrf_config_8832br_radio_b_reg(rf, phy);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_radio_a_reg(rf, phy);
		halrf_config_8192xb_radio_b_reg(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_radio_b_reg(rf, phy);
		halrf_config_8852bp_radio_a_reg(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_radio_a_reg(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_radio_a_reg(rf, phy);
	}
#endif

	return result;
}

bool halrf_config_power_by_rate(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_by_rate(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_by_rate(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_by_rate(rf, phy);
	}
#endif

	return result;
}

bool halrf_config_power_limit(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_limit(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_limit(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_limit(rf, phy);
		halrf_config_8852c_store_power_limit_6g(rf, phy);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_limit(rf, phy);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_limit(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_limit(rf, phy);
		halrf_config_8852bp_store_power_limit_6g(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_limit(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_limit(rf, phy);
	}
#endif

	return result;
}

bool halrf_config_power_limit_ru(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_power_limit_ru(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_power_limit_ru(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_power_limit_ru(rf, phy);
		halrf_config_8852c_store_power_limit_ru_6g(rf, phy);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_power_limit_ru(rf, phy);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_power_limit_ru(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_power_limit_ru(rf, phy);
		halrf_config_8852bp_store_power_limit_ru_6g(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_power_limit_ru(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_power_limit_ru(rf, phy);
	}
#endif

	return result;
}

bool halrf_config_power_track(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8832BR_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8832BR) {
		halrf_config_8832br_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8192XB_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8192XB) {
		halrf_config_8192xb_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_pwr_track(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_pwr_track(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_pwr_track(rf, phy);
	}
#endif

	return result;
}

bool halrf_config_xtal_track(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;
	bool result = true;

#ifdef RF_8852A_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		halrf_config_8852a_store_xtal_track(rf, phy);
	}
#endif

#ifdef RF_8852B_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852B) {
		halrf_config_8852b_store_xtal_track(rf, phy);
	}
#endif

#ifdef RF_8852C_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852C) {
		halrf_config_8852c_store_xtal_track(rf, phy);
	}
#endif

#ifdef RF_8852BP_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8852BP) {
		halrf_config_8852bp_store_xtal_track(rf, phy);
	}
#endif
#ifdef RF_8730E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8730E) {
		halrf_config_8730e_store_xtal_track(rf, phy);
	}
#endif

#ifdef RF_8720E_SUPPORT
	if (hal_com->chip_id == CHIP_WIFI6_8720E) {
		halrf_config_8720e_store_xtal_track(rf, phy);
	}
#endif

	return result;
}

void halrf_config_rf_parameter(void *rf_void, enum phl_phy_idx phy)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	struct rtw_hal_com_t *hal_com = rf->hal_com;

	/*Radio A and B*/
	halrf_config_radio(rf, phy);

	/*Power by rate*/
	halrf_config_power_by_rate(rf, phy);

	/*Power limit*/
	halrf_config_power_limit(rf, phy);

	/*Power limit RU*/
	halrf_config_power_limit_ru(rf, phy);

	/*Power Track*/
	halrf_config_power_track(rf, phy);

	/*Xtal Track*/
	halrf_config_xtal_track(rf, phy);
}

