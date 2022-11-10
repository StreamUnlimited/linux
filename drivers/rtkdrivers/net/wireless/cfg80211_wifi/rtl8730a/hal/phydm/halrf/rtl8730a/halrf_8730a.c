/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
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

#include "mp_precomp.h"
#if (DM_ODM_SUPPORT_TYPE == 0x08)
#if RT_PLATFORM == PLATFORM_MACOSX
#include "phydm_precomp.h"
#else
#include "../phydm_precomp.h"
#endif
#else
#include "../../phydm_precomp.h"
#endif

#if (RTL8730A_SUPPORT == 1)

u8 halrf_thermal_read_8730a(void *dm_void,	u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_rf_reg(dm, path, RF_0x42, BIT(19), 0x1);
	odm_set_rf_reg(dm, path, RF_0x42, BIT(19), 0x0);
	odm_set_rf_reg(dm, path, RF_0x42, BIT(19), 0x1);

	ODM_delay_us(15);

	return (u8)odm_get_rf_reg(dm, path, RF_0x42, 0x0007e);
}

void _halrf_dbg_dump_rf_reg_8730a(struct dm_struct *dm)
{
	u32 addr = 0;
	u32 reg = 0, reg1 = 0, reg2 = 0, reg3 = 0;

	RF_DBG(dm, DBG_RF_RFK,
	       "[RFK] ************* DUMP RFREG *************\n");

	RF_DBG(dm, DBG_RF_RFK, "[RFK] S0 RF reg\n");
	for (addr = 0x0; addr <= 0xFF; addr += 4) {
		reg = odm_get_rf_reg(dm, RF_PATH_A, addr, RFREG_MASK);
		reg1 = odm_get_rf_reg(dm, RF_PATH_A, addr + 1, RFREG_MASK);
		reg2 = odm_get_rf_reg(dm, RF_PATH_A, addr + 2, RFREG_MASK);
		reg3 = odm_get_rf_reg(dm, RF_PATH_A, addr + 3, RFREG_MASK);
		RF_DBG(dm, DBG_RF_RFK,
		       "[RFK] 0x%02x  0x%05x, 0x%05x, 0x%05x, 0x%05x\n",
		       addr, reg, reg1, reg2, reg3);
	}
}

void _halrf_dump_rfk_reg_8730a(struct dm_struct *dm)
{
	u32 addr = 0;
	u16 page[] = {0x1b00, 0x1c00, 0x3a00, 0x4200, 0x4300};
	u8 i, len = sizeof(page) / sizeof(page[0]);

	RF_DBG(dm, DBG_RF_RFK,
	       "[RFK] ************* DUMP PHYREG *************\n");

	for (i = 0; i < len; i++) {
		RF_DBG(dm, DBG_RF_RFK,
		       "\n[RFK] *************Page %x *************\n", page[i]);
		for (addr = page[i]; addr < page[i] + 0xff; addr += 0x10) {
			RF_DBG(dm, DBG_RF_RFK, "[RFK] 0x%04x 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
			       addr, odm_get_bb_reg(dm, addr, MASKDWORD),
			       odm_get_bb_reg(dm, addr + 4, MASKDWORD),
			       odm_get_bb_reg(dm, addr + 8, MASKDWORD),
			       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
		}
	}
#if 0
	RF_DBG(dm, DBG_RF_RFK,
	       "[RFK] *************Page 1b *************\n");
	for (addr = 0x1b00; addr < 0x1bff; addr += 0x10) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x%04x 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		       addr, odm_get_bb_reg(dm, addr, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 4, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 8, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
	}

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] *************Page 3a *************\n");
	for (addr = 0x3a00; addr < 0x3aff; addr += 0x10) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x%04x 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		       addr, odm_get_bb_reg(dm, addr, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 4, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 8, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
	}
	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] *************Page 42 *************\n");
	for (addr = 0x4200; addr < 0x42ff; addr += 0x10) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x%04x 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		       addr, odm_get_bb_reg(dm, addr, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 4, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 8, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
	}

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] *************Page 43 *************\n");
	for (addr = 0x4300; addr < 0x43ff; addr += 0x10) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x%04x 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		       addr, odm_get_bb_reg(dm, addr, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 4, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 8, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
	}
#endif
	RF_DBG(dm, DBG_RF_RFK,
	       "[RFK] ************* DUMP PHYREG END*************\n");
}

void halrf_rf_lna_setting_8730a(struct dm_struct *dm_void,
				enum halrf_lna_set type)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u8 path = 0x0;
#if 0
	for (path = 0x0; path < 2; path++)
		if (type == HALRF_LNA_DISABLE) {
			/*S0*/
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),
				       0x1);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x33,
				       RFREGOFFSETMASK, 0x00003);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3e,
				       RFREGOFFSETMASK, 0x00064);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3f,
				       RFREGOFFSETMASK, 0x0afce);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),
				       0x0);
		} else if (type == HALRF_LNA_ENABLE) {
			/*S0*/
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),
				       0x1);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x33,
				       RFREGOFFSETMASK, 0x00003);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3e,
				       RFREGOFFSETMASK, 0x00064);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3f,
				       RFREGOFFSETMASK, 0x1afce);
			odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),
				       0x0);
		}
#endif
}

void odm_tx_pwr_track_set_pwr8730a(void *dm_void, enum pwrtrack_method method,
				   u8 rf_path, u8 channel_mapped_index)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &dm->rf_calibrate_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	//[TBD]
	return;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "pRF->absolute_ofdm_swing_idx=%d   pRF->remnant_ofdm_swing_idx=%d   pRF->absolute_cck_swing_idx=%d   pRF->remnant_cck_swing_idx=%d   rf_path=%d\n",
	       cali_info->absolute_ofdm_swing_idx[rf_path],
	       cali_info->remnant_ofdm_swing_idx[rf_path],
	       cali_info->absolute_cck_swing_idx[rf_path],
	       cali_info->remnant_cck_swing_idx, rf_path);

	/*use for mp driver clean power tracking status*/
	if (method == CLEAN_MODE) {
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "===> %s method=%d clear power tracking rf_path=%d\n",
		       __func__, method, rf_path);
	} else if (method == BBSWING) {
	} else if (method == MIX_MODE) {
	}
}

void get_delta_swing_table_8730a(void *dm_void,
				 u8 **temperature_up_a,
				 u8 **temperature_down_a,
				 u8 **temperature_up_b,
				 u8 **temperature_down_b)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &dm->rf_calibrate_info;
	u8 channel = *dm->channel;
	u8 tx_rate = phydm_get_tx_rate(dm);

	if (channel >= 1 && channel <= 14) {
		if (IS_CCK_RATE(tx_rate)) {
			*temperature_up_a = cali_info->delta_swing_table_idx_2g_cck_a_p;
			*temperature_down_a = cali_info->delta_swing_table_idx_2g_cck_a_n;
			*temperature_up_b = cali_info->delta_swing_table_idx_2g_cck_b_p;
			*temperature_down_b = cali_info->delta_swing_table_idx_2g_cck_b_n;
		} else {
			*temperature_up_a = cali_info->delta_swing_table_idx_2ga_p;
			*temperature_down_a = cali_info->delta_swing_table_idx_2ga_n;
			*temperature_up_b = cali_info->delta_swing_table_idx_2gb_p;
			*temperature_down_b = cali_info->delta_swing_table_idx_2gb_n;
		}
	}

	if (channel >= 36 && channel <= 64) {
		*temperature_up_a = cali_info->delta_swing_table_idx_5ga_p[0];
		*temperature_down_a = cali_info->delta_swing_table_idx_5ga_n[0];
		*temperature_up_b = cali_info->delta_swing_table_idx_5gb_p[0];
		*temperature_down_b = cali_info->delta_swing_table_idx_5gb_n[0];
	} else if (channel >= 100 && channel <= 144) {
		*temperature_up_a = cali_info->delta_swing_table_idx_5ga_p[1];
		*temperature_down_a = cali_info->delta_swing_table_idx_5ga_n[1];
		*temperature_up_b = cali_info->delta_swing_table_idx_5gb_p[1];
		*temperature_down_b = cali_info->delta_swing_table_idx_5gb_n[1];
	} else if (channel >= 149 && channel <= 177) {
		*temperature_up_a = cali_info->delta_swing_table_idx_5ga_p[2];
		*temperature_down_a = cali_info->delta_swing_table_idx_5ga_n[2];
		*temperature_up_b = cali_info->delta_swing_table_idx_5gb_p[2];
		*temperature_down_b = cali_info->delta_swing_table_idx_5gb_n[2];
	}
}

void _phy_aac_calibrate_8730a(struct dm_struct *dm)
{

}

void _phy_x2_calibrate_8730a(struct dm_struct *dm)
{
	u32 rf_18;

	RF_DBG(dm, DBG_RF_LCK, "[X2K]X2K start!!!!!!!\n");
	RF_DBG(dm, DBG_RF_LCK, "[X2K]X2K end!!!!!!!\n");
}

void phy_x2_check_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 X2K_BUSY;

	/*X2K*/
	_phy_x2_calibrate_8730a(dm);

	RF_DBG(dm, DBG_RF_LCK, "[X2K]X2K check start!!!!!!!\n");
	RF_DBG(dm, DBG_RF_LCK, "[X2K]X2K check end!!!!!!!\n");
}

/*LCK VERSION:0x0*/
void phy_lc_calibrate_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
#if 0
	_phy_aac_calibrate_8730a(dm);
	_phy_rt_calibrate_8730a(dm);
#endif
}

void configure_txpower_track_8730a(struct txpwrtrack_cfg *config)
{
	config->swing_table_size_cck = TXSCALE_TABLE_SIZE;
	config->swing_table_size_ofdm = TXSCALE_TABLE_SIZE;
	config->threshold_iqk = IQK_THRESHOLD;
	config->threshold_dpk = DPK_THRESHOLD;
	config->average_thermal_num = AVG_THERMAL_NUM_8730A;
	config->rf_path_count = MAX_PATH_NUM_8730A;
	config->thermal_reg_addr = RF_T_METER_8730A;

	config->odm_tx_pwr_track_set_pwr = odm_tx_pwr_track_set_pwr8730a;
	config->do_iqk = do_iqk_8730a;
	config->phy_lc_calibrate = halrf_lck_trigger;
	config->get_delta_swing_table = get_delta_swing_table_8730a;
}

void phy_set_rf_path_switch_8730a(struct dm_struct *dm, boolean is_main)
{
}

void halrf_rfk_handshake_8730a(void *dm_void, boolean is_before_k)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	void *padapter = dm->adapter;

	u8 u1b_tmp, h2c_parameter;
	u16 count;
	u8 path = 0, rfk_type = rf->rfk_type;

	return;

	if (is_before_k) {
		RF_DBG(dm, DBG_RF_RFK,
		       "[RFK] WiFi / BT RFK handshake start!!\n");
		RF_DBG(dm, DBG_RF_RFK,
		       "[RFK] WiFi rfk_type = 0x%x\n", rfk_type);

		if (!rf->is_bt_iqk_timeout) {
			count = 0;
			u1b_tmp = (u8)odm_get_mac_reg(dm, 0xac, BIT(6) | BIT(5));
			while (u1b_tmp != 0 && count < 30000) {
				ODM_delay_us(20);
				u1b_tmp = (u8)odm_get_mac_reg(dm, 0xac, BIT(6) | BIT(5));
				count++;
			}

			if (count >= 30000) {
				RF_DBG(dm, DBG_RF_RFK,
				       "[RFK] Wait BT IQK finish timeout!!\n");
				rf->is_bt_iqk_timeout = true;
			}
		}

		/* Send RFK start H2C cmd*/
		h2c_parameter = 1;
		odm_fill_h2c_cmd(dm, ODM_H2C_WIFI_CALIBRATION, 1, &h2c_parameter);
#if defined(CONFIG_BTCOEX_SUPPORT_BTC_CMN) && defined(CONFIG_BT_COEXIST)
		hal_btcoex_WLRFKNotify(padapter, path, rfk_type, 0);
#endif
		ODM_delay_us(20);
	} else {
		/* Send RFK finish H2C cmd*/
		h2c_parameter = 0;
		odm_fill_h2c_cmd(dm, ODM_H2C_WIFI_CALIBRATION, 1, &h2c_parameter);
#if defined(CONFIG_BTCOEX_SUPPORT_BTC_CMN) && defined(CONFIG_BT_COEXIST)
		hal_btcoex_WLRFKNotify(padapter, path, rfk_type, 1);
#endif
		ODM_delay_us(20);

		RF_DBG(dm, DBG_RF_RFK,
		       "[RFK] WiFi / BT RFK handshake finish!!\n");
	}
}

void btc_set_gnt_wl_bt_8730a(void *dm_void, boolean is_before_k)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (is_before_k) {
	} else {
	}
}

void halrf_rfk_power_save_8730a(void *dm_void, boolean is_power_save)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (is_power_save) { /*default*/
		odm_set_bb_reg(dm, 0x1c64, BIT(30), 0x1);
	} else { /*RFK disable pwrsave*/
		odm_set_bb_reg(dm, 0x1c64, BIT(30), 0x0);
	}
}

#endif /*(RTL8730A_SUPPORT == 0)*/
