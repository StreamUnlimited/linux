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
#include "../halrf_precomp.h"
#include "mac/mac_def.h"
#ifdef RF_8730E_SUPPORT
bool halrf_bw_setting_8730e(struct rf_info *rf, enum rf_path path, enum channel_width bw, bool is_dav)
{
	u32 rf_reg18 = 0;
	u32 reg_reg18_addr = 0x0;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);
	if (is_dav) {
		reg_reg18_addr = 0x18;
	} else {
		reg_reg18_addr = 0x10018;
	}

	rf_reg18 = halrf_rrf(rf, path, reg_reg18_addr, MASKRF);
	/*==== [Error handling] ====*/
	if (rf_reg18 == INVALID_RF_DATA) {
		RF_DBG(rf, DBG_RF_RFK, "[RFK]Invalid RF_0x18 for Path-%d\n", path);
		return false;
	}
	rf_reg18 &= ~(BIT(11) | BIT(10));
	/*==== [Switch bandwidth] ====*/
	switch (bw) {
	case CHANNEL_WIDTH_5:
	case CHANNEL_WIDTH_10:
	case CHANNEL_WIDTH_20:
		/*RF bandwidth */
		rf_reg18 |= (BIT(11) | BIT(10));
		break;
	case CHANNEL_WIDTH_40:
		/*RF bandwidth */
		rf_reg18 |= BIT(11);
		break;
	case CHANNEL_WIDTH_80:
		/*RF bandwidth */
		rf_reg18 |= BIT(10);
		break;
	default:
		RF_DBG(rf, DBG_RF_RFK, "[RFK]Fail to set CH\n");
	}

	/*==== [Write RF register] ====*/
	halrf_wrf(rf, path, reg_reg18_addr, MASKRF, rf_reg18);
	RF_DBG(rf, DBG_RF_RFK, "[RFK] set %x at path%d, %x =0x%x\n", bw, path, reg_reg18_addr, halrf_rrf(rf, path, reg_reg18_addr, MASKRF));
	return true;
}

bool halrf_ctrl_bw_8730e(struct rf_info *rf, enum channel_width bw)
{
	bool is_dav;
	//RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	/*==== Error handling ====*/
	if (bw >= CHANNEL_WIDTH_MAX) {
		RF_DBG(rf, DBG_RF_RFK, "[RFK]Fail to switch bw(bw:%d)\n", bw);
		return false;
	}

	//DAV
	is_dav = true;
	halrf_bw_setting_8730e(rf, RF_PATH_A, bw, is_dav);
	//DDV
	is_dav = false;
	halrf_bw_setting_8730e(rf, RF_PATH_A, bw, is_dav);

	//RF_DBG(rf, DBG_RF_RFK, "[RFK] BW: %d\n", bw);
	//RF_DBG(rf, DBG_RF_RFK, "[RFK] 0x18 = 0x%x\n",halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF));

	return true;
}

bool halrf_ch_setting_8730e(struct rf_info *rf, enum rf_path path, u8 central_ch,
			    enum band_type band, bool is_dav)
{
	u32 rf_reg18 = 0;
	u32 reg_reg18_addr = 0x0;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	if (is_dav) {
		reg_reg18_addr = 0x18;
	} else {
		reg_reg18_addr = 0x10018;
	}

	rf_reg18 = halrf_rrf(rf, path, reg_reg18_addr, MASKRF);
	/*==== [Error handling] ====*/
	if (rf_reg18 == INVALID_RF_DATA) {
		RF_DBG(rf, DBG_RF_RFK, "[RFK]Invalid RF_0x18 for Path-%d\n", path);
		return false;
	}
	//*is_2g_ch = (central_ch <= 14) ? true : false;
	/*==== [Set RF Reg 0x18] ====*/
	rf_reg18 &= ~0x303ff; /*[17:16],[9:8],[7:0]*/
	rf_reg18 |= central_ch; /* Channel*/
	/*==== [5G Setting] ====*/
	if (band == BAND_ON_5G) {
		rf_reg18 |= (BIT(16) | BIT(8));
	}
	halrf_wrf(rf, path, reg_reg18_addr, MASKRF, rf_reg18);
	halrf_delay_us(rf, 100);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 0);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 1);
	RF_DBG(rf, DBG_RF_RFK, "[RFK]CH: %d for Path-%d, reg0x%x = 0x%x\n", central_ch, path, reg_reg18_addr, halrf_rrf(rf, path, reg_reg18_addr, MASKRF));
	return true;
}

bool halrf_ctrl_ch_8730e(struct rf_info *rf, u8 central_ch, enum band_type band)
{
	bool is_dav;

	//RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	/*==== Error handling ====*/
	if ((central_ch > 14 && central_ch < 36) ||
	    (central_ch > 64 && central_ch < 100) ||
	    (central_ch > 144 && central_ch < 149) ||
	    central_ch > 177) {
		RF_DBG(rf, DBG_RF_RFK, "[RFK]Invalid CH:%d \n", central_ch);
		return false;
	}
	//DAV
	is_dav = true;
	halrf_ch_setting_8730e(rf, RF_PATH_A, central_ch, band, is_dav);
	//DDV
	is_dav = false;
	halrf_ch_setting_8730e(rf, RF_PATH_A, central_ch, band, is_dav);
	//RF_DBG(rf, DBG_RF_RFK, "[RFK] CH: %d\n", central_ch);
	return true;
}

void halrf_set_lo_8730e(struct rf_info *rf, bool is_on, enum rf_path path)
{
	if (is_on) {
		halrf_rf_direct_cntrl_8730e(rf, path, false);
		halrf_wrf(rf, path, 0x0, MASKRFMODE, 0x2);
		halrf_wrf(rf, path, 0x58, BIT(1), 0x1);
		halrf_wrf(rf, path, 0xdf, 0x90, 0x3);
		halrf_wrf(rf, path, 0x56, 0x1c00, 0x1);
		halrf_wrf(rf, path, 0x56, 0x3e0, 0x4);
	} else {
		halrf_wrf(rf, path, 0x58, BIT(1), 0x0);
		halrf_rf_direct_cntrl_8730e(rf, path, true);
		halrf_wrf(rf, path, 0xdf, 0x90, 0x0);
	}
}

void halrf_rf_direct_cntrl_8730e(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb) {
		halrf_wrf(rf, path, 0x5, BIT(0), 0x1);
	} else {
		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
	}
}

void halrf_drf_direct_cntrl_8730e(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb) {
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x1);
	} else {
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x0);
	}
}


void halrf_lo_test_8730e(struct rf_info *rf, bool is_on, enum rf_path path)
{
	halrf_set_lo_8730e(rf, is_on, RF_PATH_A);
}

u8 halrf_kpath_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx)
{

	//RF_DBG(rf, DBG_RF_RFK, "[RFK]dbcc_en: %x,  PHY%d\n", rf->hal_com->dbcc_en, phy_idx);

	return RF_A;
}

void halrf_cca_disable_8730e(struct rf_info *rf, enum phl_phy_idx phy, bool is_dis_cca)
{
	struct rtw_hal_com_t *hal_i = rf->hal_com;

	if (is_dis_cca) {
		//PD Disable
		halrf_wreg(rf, 0x2320, BIT(0), 0x1);
		halrf_wreg(rf, 0xc3c, BIT(9), 0x1);
		halrf_wreg(rf, 0x4730, BIT(31), 0x0); //disable AGC
	} else {
		if (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_24G) {
			halrf_wreg(rf, 0x2320, BIT(0), 0x0);
		}
		halrf_wreg(rf, 0xc3c, BIT(9), 0x0);//
		halrf_wreg(rf, 0x4730, BIT(31), 0x1); //r_AGC_en
	}

}

void halrf_spur_compensation_8730e(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct	rtw_hal_com_t *hal_i = rf->hal_com;
	u32	sys_afe, sys_xtal, rf_reg5;
	u32	data[3], datamax[6], mindata, spur_level;
	u8	phase_val[6] = {0, 1, 3, 4, 6, 7};
	u8	i, j, k, t = 0;

	//halrf_cca_disable_8730e(rf, phy, true);

	if (rf->hal_com->band[phy].cur_chandef.center_ch == 13) {  //2G
		s32	psd_loc[3] = {0x3F, 0x40, 0x41};
		rf_reg5 = halrf_rrf(rf, RF_PATH_A, 0x5, MASKRF);

		sys_xtal = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, XTAL_ANAPAR_XTAL_OFF_0);
		sys_xtal = sys_xtal & 0xFFFF99FF;
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, XTAL_ANAPAR_XTAL_OFF_0, sys_xtal | 0x2200);

		sys_afe = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0);
		sys_afe = sys_afe & 0xFFFFFFF7;
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);

		halrf_wrf(rf, RF_PATH_A, 0x5, BIT(0), 0x0);
		halrf_wrf(rf, RF_PATH_A, 0x0, MASKRF, 0x33BE0);
		halrf_psd_init(rf, phy, RF_PATH_A, 0, 32, 1280);

		for (i = 0; i < 6; i++) {
			sys_afe = (sys_afe & 0xFFFFFF87) | (phase_val[i] << 4);
			hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);
			halrf_delay_us(rf, 10);
			for (k = 0; k < 3; k++) {
				data[k] = halrf_psd_get_point_data(rf, phy, psd_loc[k]);
			}
			datamax[i] = (data[0] > data[1]) ? (data[0]) : (data[1]);
			datamax[i] = (datamax[i] > data[2]) ? (datamax[i]) : (data[2]);
			//datamax[i] = MAX_2(MAX_2(data[0], data[1]), data[2]);
			RF_DBG(rf, DBG_RF_RFK, "[RF spur]max_psd[%d]= %x\n", i, datamax[i]);
		}
		halrf_psd_restore(rf, phy);
		halrf_wrf(rf, RF_PATH_A, 0x5, MASKRF, rf_reg5);
		mindata = datamax[0];
		for (j = 1; j < 6; j++) {
			mindata = ((mindata) < (datamax[j])) ? (mindata) : (datamax[j]);
			//mindata = MIN_2(mindata, datamax[j]);
		}
		for (i = 0; i < 6; i++) {
			if (mindata == datamax[i]) {
				t = phase_val[i];
				RF_DBG(rf, DBG_RF_RFK, "[RF spur]t= %d, min_spur = %x\n", t, mindata);
			}
		}
		sys_afe = (sys_afe & 0xFFFFFF87) | (t << 4);
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);
		//sys_afe = WLAFE_BASE->WLAFE_ANAPAR_MAC_0;
	} else if (rf->hal_com->band[phy].cur_chandef.center_ch == 14) {
		s32	psd_loc[3] = {-31, -32, -33};
		rf_reg5 = halrf_rrf(rf, RF_PATH_A, 0x5, MASKRF);

		sys_xtal = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, XTAL_ANAPAR_XTAL_OFF_0);
		sys_xtal = sys_xtal & 0xFFFF99FF;
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, XTAL_ANAPAR_XTAL_OFF_0, sys_xtal | 0x2200);

		sys_afe = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0);
		sys_afe = sys_afe & 0xFFFFFFF7;
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);

		halrf_wrf(rf, RF_PATH_A, 0x5, BIT(0), 0x0);
		halrf_wrf(rf, RF_PATH_A, 0x0, MASKRF, 0x33BE0);
		halrf_psd_init(rf, phy, RF_PATH_A, 0, 32, 1280);

		for (i = 0; i < 6; i++) {
			sys_afe = (sys_afe & 0xFFFFFF87) | (phase_val[i] << 4);
			hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);
			halrf_delay_us(rf, 10);
			for (k = 0; k < 3; k++) {
				data[k] = halrf_psd_get_point_data(rf, phy, psd_loc[k]);
			}
			datamax[i] = (data[0] > data[1]) ? (data[0]) : (data[1]);
			datamax[i] = (datamax[i] > data[2]) ? (datamax[i]) : (data[2]);
			//datamax[i] = MAX_2(MAX_2(data[0], data[1]), data[2]);
			RF_DBG(rf, DBG_RF_RFK, "[RF spur]max_psd[%d]= %x\n", i, datamax[i]);
		}
		halrf_psd_restore(rf, phy);
		halrf_wrf(rf, RF_PATH_A, 0x5, MASKRF, rf_reg5);
		mindata = datamax[0];
		for (j = 1; j < 6; j++) {
			mindata = ((mindata) < (datamax[j])) ? (mindata) : (datamax[j]);
			//mindata = MIN_2(mindata, datamax[j]);
		}
		for (i = 0; i < 6; i++) {
			if (mindata == datamax[i]) {
				t = phase_val[i];
				RF_DBG(rf, DBG_RF_RFK, "[RF spur]t= %d, min_spur = %x\n", t, mindata);
			}
		}
		sys_afe = (sys_afe & 0xFFFFFF87) | (t << 4);
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);
		//sys_afe = WLAFE_BASE->WLAFE_ANAPAR_MAC_0;
	} else {
		sys_afe = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0);
		sys_afe = (sys_afe & 0xFFFFFF87) | BIT(3);
		hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0, sys_afe);

	}
	halrf_delay_us(rf, 100);
	//halrf_cca_disable_8730e(rf, phy, false);
	RF_DBG(rf, DBG_RF_RFK, "[RF spur]0x42008944= %x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_0));
	RF_DBG(rf, DBG_RF_RFK, "[RF spur]RF0 = 0x%x\n", halrf_rrf(rf, RF_PATH_A, 0x0, MASKRF));
}

void halrf_set_rx_path_8730e(struct rf_info *rf, enum phl_phy_idx phy, bool rx_path)
{
	struct rtw_hal_com_t *hal_i = rf->hal_com;

	if (rx_path) {	//1 aux
		halrf_wrf(rf, RF_PATH_A, 0x02, BIT(18), 0x1);
		halrf_wrf(rf, RF_PATH_A, 0x8B, BIT(17), 0x1);
		halrf_wrf(rf, RF_PATH_A, 0x02, BIT(17), 0x1);
		halrf_wrf(rf, RF_PATH_A, 0x87, BIT(0), 0x1);
		/*
			if(rf->hal_com->band[phy].cur_chandef.band) { //5G
				halrf_wrf(rf, RF_PATH_A, 0x02, BIT(18), 0x1);
				halrf_wrf(rf, RF_PATH_A, 0x8B, BIT(17), 0x1);
			} else {				      //2G
				halrf_wrf(rf, RF_PATH_A, 0x02, BIT(17), 0x1);
				halrf_wrf(rf, RF_PATH_A, 0x87, BIT(0), 0x1);
			}
		*/
	} else {  //0 main
		halrf_wrf(rf, RF_PATH_A, 0x02, BIT(18), 0x0);
		halrf_wrf(rf, RF_PATH_A, 0x8B, BIT(17), 0x0);
		halrf_wrf(rf, RF_PATH_A, 0x02, BIT(17), 0x0);
		halrf_wrf(rf, RF_PATH_A, 0x87, BIT(0), 0x0);
		/*
			if(rf->hal_com->band[phy].cur_chandef.band) { //5G
				halrf_wrf(rf, RF_PATH_A, 0x02, BIT(18), 0x0);
				halrf_wrf(rf, RF_PATH_A, 0x8B, BIT(17), 0x0);
			} else {				      //2G
				halrf_wrf(rf, RF_PATH_A, 0x02, BIT(17), 0x0);
				halrf_wrf(rf, RF_PATH_A, 0x87, BIT(0), 0x0);
			}
		*/
	}
}

void _rx_dck_info_8730e(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	rx_dck->is_afe = is_afe;
	rx_dck->loc[path].cur_band = rf->hal_com->band[phy].cur_chandef.band;
	rx_dck->loc[path].cur_bw = rf->hal_com->band[phy].cur_chandef.bw;
	rx_dck->loc[path].cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ==== S%d RX DCK (%s / CH%d / %s)====\n", path,
	       rx_dck->loc[path].cur_band == 0 ? "2G" :
	       (rx_dck->loc[path].cur_band == 1 ? "5G" : "6G"),
	       rx_dck->loc[path].cur_ch,
	       rx_dck->loc[path].cur_bw == 0 ? "20M" :
	       (rx_dck->loc[path].cur_bw == 1 ? "40M" : "80M"));
}

void halrf_set_rx_dck_8730e(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;
	u8 i = 0;
	u32	rf_reg5, rf_9F, rx_G7_dck, iqk_G7_dck, rf_bk;
	u8	diff_I, diff_Q, a, b;
	u32	rx_dck_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	rx_dck->is_afe = is_afe;
	rx_dck->loc[path].cur_band = rf->hal_com->band[phy].cur_chandef.band;
	rx_dck->loc[path].cur_bw = rf->hal_com->band[phy].cur_chandef.bw;
	rx_dck->loc[path].cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ==== S%d RX DCK (%s / CH%d / %s)====\n", path,
	       rx_dck->loc[path].cur_band == 0 ? "2G" : "5G",
	       rx_dck->loc[path].cur_ch,
	       rx_dck->loc[path].cur_bw == 0 ? "20M" :
	       (rx_dck->loc[path].cur_bw == 1 ? "40M" : "80M"));

	rx_dck->ther_rxdck[path] = halrf_get_thermal_8730e(rf, path);

	if (rx_dck->loc[path].cur_band) {  //A mode
		rf_reg5 = halrf_rrf(rf, path, 0x5, MASKRF);
		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
//		halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);
		halrf_wrf(rf, path, 0x00, MASKRF, 0x333e0);
		/*RXBB DCK*/
		halrf_wrf(rf, path, 0x18, 0x30000, 0x1);
		halrf_wrf(rf, path, 0x9B, MASKRF, 0x50FE);
		halrf_wrf(rf, path, 0x9C, 0x1C, 0x2);

		halrf_wrf(rf, path, 0x81, MASKRF, 0x6F040);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x2F040);
		/*Need Add Delay time 70u after RXBB DCK, delay 100us*/
		i = 0;
		while ((halrf_rrf(rf, path, 0xDA, BIT(1)) == 0x0) && (i < 20)) {
			halrf_delay_us(rf, 10);
			i++;
		}
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RXBB RF 0xDA = 0x%x, delay %d us\n",
		       halrf_rrf(rf, path, 0xDA, MASKRF), (i * 10));
		/*Amode only 1 time TIA DCK (A cut)*/
		halrf_wrf(rf, path, 0x9C, 0x1C, 0x7);
		halrf_wrf(rf, path, 0x9B, MASKRF, 0x50FE); //0xA0FE
		/*
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9C = 0x%x\n",
			halrf_rrf(rf, path, 0x9C, MASKRF));
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9B = 0x%x\n",
			halrf_rrf(rf, path, 0x9B, MASKRF));
		*/
		halrf_wrf(rf, path, 0x81, MASKRF, 0x5F840);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x1F840);
		/* Need Add Delay time160u after TIA DCK, delay 200us*/
		i = 0;
		while ((halrf_rrf(rf, path, 0xDA, BIT(0)) == 0x0) && (i < 100)) {
			halrf_delay_us(rf, 10);
			i++;
		}
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RX RF 0xDA = 0x%x, delay %d us\n",
		       halrf_rrf(rf, path, 0xDA, MASKRF), (i * 10));

		halrf_wrf(rf, path, 0xC8, MASKRF, 0x17000);
		rx_G7_dck = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x16800);
		halrf_wrf(rf, path, 0xC9, MASKRF, rx_G7_dck);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x0);

		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RF 0x9C = 0x%x, 0x9D = 0x%x\n",
		       halrf_rrf(rf, path, 0x9C, MASKRF),
		       halrf_rrf(rf, path, 0x9D, MASKRF));
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RF 0x9C = 0x%x, 0x9D = 0x%x\n",
		       ((halrf_rrf(rf, path, 0x9C, MASKRF) & 0xFC000) >> 14),
		       ((halrf_rrf(rf, path, 0x9D, MASKRF) & 0xFC000) >> 14));
		rf_9F = halrf_rrf(rf, path, 0x9F, MASKRF);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9F = 0x%x\n", rf_9F);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]WL RF 0x9F[19:18] = %x, 0x9F[17:16] = %x, RF 0x9F[15:8] = %x, 0x9F[7:0] = %x\n",
		       ((rf_9F & 0xC0000) >> 18), ((rf_9F & 0x30000) >> 16),
		       ((rf_9F & 0xFF00) >> 8), (rf_9F & 0xFF));
		halrf_wrf(rf, path, 0x5, MASKRF, rf_reg5);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK]WL RFC ADCK over!\n");
	} else {
		rf_reg5 = halrf_rrf(rf, path, 0x5, MASKRF);
		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
		//	halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);
		halrf_wrf(rf, path, 0x00, MASKRF, 0x333e0);
		halrf_wrf(rf, path, 0x18, 0x30000, 0x0);
		/*RXBB DCK*/
		halrf_wrf(rf, path, 0x9B, MASKRF, 0x50FE);
		halrf_wrf(rf, path, 0x9C, 0x1C, 0x2);

		halrf_wrf(rf, path, 0x81, MASKRF, 0x6F040);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x2F040);
		/*Need Add Delay time 70u after RXBB DCK, delay 100us*/
		i = 0;
		while ((halrf_rrf(rf, path, 0xDA, BIT(1)) == 0x0) && (i < 20)) {
			halrf_delay_us(rf, 10);
			i++;
		}
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RXBB RF 0xDA = 0x%x, delay %d us\n",
		       halrf_rrf(rf, path, 0xDA, MASKRF), (i * 10));
		/*RXmode 1st TIA DCK*/
		halrf_wrf(rf, path, 0x9C, 0x1C, 0x5);  //7
		halrf_wrf(rf, path, 0x9B, MASKRF, 0x50FE); //0xA0FE
		/*
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9C = 0x%x\n",
			halrf_rrf(rf, path, 0x9C, MASKRF));
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9B = 0x%x\n",
			halrf_rrf(rf, path, 0x9B, MASKRF));
		*/
		halrf_wrf(rf, path, 0x81, MASKRF, 0x5F840);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x1F840);
		/* Need Add Delay time160u after TIA DCK, delay 200us*/
		i = 0;
		while ((halrf_rrf(rf, path, 0xDA, BIT(0)) == 0x0) && (i < 100)) {
			halrf_delay_us(rf, 10);
			i++;
		}
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RX RF 0xDA = 0x%x, delay %d us\n",
		       halrf_rrf(rf, path, 0xDA, MASKRF), (i * 10));

		halrf_wrf(rf, path, 0xC8, MASKRF, 0x47000);
		rx_G7_dck = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x46800);
		halrf_wrf(rf, path, 0xC9, MASKRF, rx_G7_dck);

		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA] 1st RX G7[19:12] = 0x%x, G7_dck[9:2] = 0x%x\n",
		       (rx_G7_dck & 0xFF000) >> 12, (rx_G7_dck & 0x3FC) >> 2);

		halrf_wrf(rf, path, 0xC8, MASKRF, 0x47000);
		rx_dck_val[7] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x46000);
		rx_dck_val[6] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x45000);
		rx_dck_val[5] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x44000);
		rx_dck_val[4] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x43000);
		rx_dck_val[3] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x42000);
		rx_dck_val[2] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x41000);
		rx_dck_val[1] = halrf_rrf(rf, path, 0xCC, MASKRF);
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x40000);
		rx_dck_val[0] = halrf_rrf(rf, path, 0xCC, MASKRF);

		/*RXIQK mode 2nd TIA  DCK*/
		halrf_wrf(rf, path, 0x00, 0xF0000, 0x7);
		halrf_wrf(rf, path, 0x87, BIT(11), 0x1);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x5F840);
		halrf_wrf(rf, path, 0x81, MASKRF, 0x1F840);
		/* Need Add Delay time160u after TIA DCK, delay 200us*/
		i = 0;
		while ((halrf_rrf(rf, path, 0xDA, BIT(0)) == 0x0) && (i < 100)) {
			halrf_delay_us(rf, 10);
			i++;
		}
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RX RF 0xDA = 0x%x, delay %d us\n",
		       halrf_rrf(rf, path, 0xDA, MASKRF), (i * 10));
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x47000);
		iqk_G7_dck = halrf_rrf(rf, path, 0xCC, MASKRF);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA] 2nd IQK G7[19:12] = 0x%x, G7_dck[9:2] = 0x%x\n",
		       (iqk_G7_dck & 0xFF000) >> 12, (iqk_G7_dck & 0x3FC) >> 2);

		/*compare rx_G7_dck and iqk_G7_dck, abs<3 use rx mode*/
		a = (rx_G7_dck & 0xFF000) >> 12;
		b = (iqk_G7_dck & 0xFF000) >> 12;
		if (a >= b) {
			diff_Q = a - b;
		} else {
			diff_Q = b - a;
		}

		a = (rx_G7_dck & 0x3FC) >> 2;
		b = (iqk_G7_dck & 0x3FC) >> 2;
		if (a >= b) {
			diff_I = a - b;
		} else {
			diff_I = b - a;
		}

		if ((diff_Q <= 3) && (diff_I <= 3)) {
			RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK]WL use RX normal value!\n");
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x47800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[7]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x46800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[6]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x45800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[5]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x44800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[4]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x43800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[3]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x42800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[2]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x41800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[1]);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x40800);
			halrf_wrf(rf, path, 0xC9, MASKRF, rx_dck_val[0]);
		} else {
			RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK]WL use RXIQK value!\n");
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x47000);
			iqk_G7_dck = halrf_rrf(rf, path, 0xCC, MASKRF);
			halrf_wrf(rf, path, 0xC8, MASKRF, 0x46800);
			halrf_wrf(rf, path, 0xC9, MASKRF, iqk_G7_dck);
		}
		halrf_wrf(rf, path, 0xC8, MASKRF, 0x0);
		halrf_wrf(rf, path, 0x87, BIT(11), 0x0);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RF 0x9C = 0x%x, 0x9D = 0x%x\n",
		       halrf_rrf(rf, path, 0x9C, MASKRF),
		       halrf_rrf(rf, path, 0x9D, MASKRF));
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][RXBB]RF 0x9C = 0x%x, 0x9D = 0x%x\n",
		       ((halrf_rrf(rf, path, 0x9C, MASKRF) & 0xFC000) >> 14),
		       ((halrf_rrf(rf, path, 0x9D, MASKRF) & 0xFC000) >> 14));
		rf_9F = halrf_rrf(rf, path, 0x9F, MASKRF);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]RF 0x9F = 0x%x\n", rf_9F);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK][TIA]WL RF 0x9F[19:18] = %x, 0x9F[17:16] = %x, RF 0x9F[15:8] = %x, 0x9F[7:0] = %x\n",
		       ((rf_9F & 0xC0000) >> 18), ((rf_9F & 0x30000) >> 16),
		       ((rf_9F & 0xFF00) >> 8), (rf_9F & 0xFF));
		halrf_wrf(rf, path, 0x5, MASKRF, rf_reg5);
		RF_DBG(rf, DBG_RF_RXDCK, "[RFC DCK]WL RFC ADCK over!\n");
	}
}

void halrf_rxdck_track_8730e(struct rf_info *rf)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] S0 delta_ther = %d (0x%x / 0x%x)\n",
	       rf->cur_ther_s0 - rx_dck->ther_rxdck[RF_PATH_A],
	       rf->cur_ther_s0, rx_dck->ther_rxdck[RF_PATH_A]);

	if (HALRF_ABS(rf->cur_ther_s0, rx_dck->ther_rxdck[RF_PATH_A]) >= 23) { //8
		halrf_rx_dck_trigger(rf, HW_PHY_0, true);
	}
}

bool halrf_rx_dck_check_8730e(struct rf_info *rf, enum rf_path path)
{
	u8 addr;
	bool is_fail = false;

	if (halrf_rreg(rf, 0xc400 + path * 0x1000, 0xF0000) == 0x0) {
		return is_fail = true;
	} else if (halrf_rreg(rf, 0xc400 + path * 0x1000, 0x0F000) == 0x0) {
		return is_fail = true;
	} else if (halrf_rreg(rf, 0xc440 + path * 0x1000, 0xF0000) == 0x0) {
		return is_fail = true;
	} else if (halrf_rreg(rf, 0xc440 + path * 0x1000, 0x0F000) == 0x0) {
		return is_fail = true;
	} else {
		for (addr = 0x0; addr < 0x20; addr++) {
			if (halrf_rreg(rf, 0xc400 + path * 0x1000 + addr * 4, 0x00FC0) == 0x0) {
				return is_fail = true;
			}
		}

		for (addr = 0x0; addr < 0x20; addr++) {
			if (halrf_rreg(rf, 0xc400 + path * 0x1000 + addr * 4, 0x0003F) == 0x0) {
				return is_fail = true;
			}
		}
	}

	return is_fail;
}

void halrf_rx_dck_8730e(struct rf_info *rf, enum phl_phy_idx phy, bool is_afe)
{
	u8 path;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ****** RXDCK Start (Ver: 0x%x, Cv: %d) ******\n",
	       RXDCK_VER_8730E, rf->hal_com->cv);
	for (path = 0; path < 1; path++) {
		if (rf->is_tssi_mode[path]) {
			halrf_wreg(rf, 0x15818, BIT(30), 0x1);        /*TSSI pause*/
		}

		halrf_set_rx_dck_8730e(rf, phy, path, is_afe);

		if (rf->is_tssi_mode[path]) {
			halrf_wreg(rf, 0x15818, BIT(30), 0x0);        /*TSSI resume*/
		}
	}
}

void halrf_rx_dck_onoff_8730e(struct rf_info *rf, bool is_enable)
{
	u8 path;

	for (path = 0; path < 2; path++) {
		halrf_wrf(rf, path, 0x93, BIT(0), !is_enable);
		if (!is_enable) {
			halrf_wrf(rf, path, 0x92, 0xFFC00, 0x220); /*[19:10]*/
			halrf_wrf(rf, path, 0x93, 0xFFC00, 0x220); /*[19:10]*/
		}
	}
}

void halrf_rck_8730e(struct rf_info *rf)
{
	u8 cnt = 0;
	u32 rf_reg5;
	u32 rck_val = 0;

	return;
	RF_DBG(rf, DBG_RF_RFK, "[RCK] ====== S%d RCK ======\n", RF_PATH_A);

	rf_reg5 = halrf_rrf(rf, RF_PATH_A, 0x5, MASKRF);

	halrf_wrf(rf, RF_PATH_A, 0x5, BIT(0), 0x0);
	halrf_wrf(rf, RF_PATH_A, 0x0, MASKRFMODE, RF_RX);

	RF_DBG(rf, DBG_RF_RFK, "[RCK] RF0x00 = 0x%05x\n", halrf_rrf(rf, RF_PATH_A, 0x00, MASKRF));

	/*RCK trigger*/
	halrf_wrf(rf, RF_PATH_A, 0x1b, MASKRF, 0x00240);

	while ((halrf_rrf(rf, RF_PATH_A, 0x1c, BIT(3)) == 0x00) && (cnt < 10)) {
		halrf_delay_us(rf, 2);
		cnt++;
	}

	rck_val = halrf_rrf(rf, RF_PATH_A, 0x1b, 0x07C00); /*[14:10]*/

	RF_DBG(rf, DBG_RF_RFK, "[RCK] rck_val = 0x%x, count = %d\n", rck_val, cnt);

	halrf_wrf(rf, RF_PATH_A, 0x1b, MASKRF, rck_val);

	halrf_wrf(rf, RF_PATH_A, 0x5, MASKRF, rf_reg5);

	RF_DBG(rf, DBG_RF_RFK, "[RCK] RF 0x1b = 0x%x\n",
	       halrf_rrf(rf, RF_PATH_A, 0x1b, MASKRF));
}

void iqk_backup_8730e(struct rf_info *rf, enum rf_path path)
{
	return;
}

void halrf_bf_config_rf_8730e(struct rf_info *rf)
{
	halrf_wrf(rf, RF_PATH_A, 0xef, BIT(19), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x00082);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0x3);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035e7);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0xa);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035f7);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0xb);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035ef);
	halrf_wrf(rf, RF_PATH_A, 0xef, BIT(19), 0x0);

	halrf_wrf(rf, RF_PATH_B, 0xef, BIT(19), 0x1);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0x1);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00020);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0x3);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x80d79);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0xa);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00d7d);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0xb);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00d7b);
	halrf_wrf(rf, RF_PATH_B, 0xef, BIT(19), 0x0);
}

void halrf_set_dpd_backoff_8730e(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 tx_scale, ofdm_bkof, path, kpath;

	return;

	kpath = halrf_kpath_8730e(rf, phy);

	ofdm_bkof = (u8)halrf_rreg(rf, 0x44a0 + (phy << 13), 0x0001F000); /*[16:12]*/
	tx_scale = (u8)halrf_rreg(rf, 0x44a0 + (phy << 13), 0x0000007F); /*[6:0]*/

	if ((ofdm_bkof + tx_scale) >= 44) { /*move dpd backoff to bb, and set dpd backoff to 0*/
		dpk->dpk_gs[phy] = 0x7f;
		for (path = 0; path < DPK_RF_PATH_MAX_8730E; path++) {
			if (kpath & BIT(path)) {
				halrf_wreg(rf, 0x81bc + (path << 8), 0x007FFFFF, 0x7f7f7f); /*[22:0]*/
				RF_DBG(rf, DBG_RF_RFK, "[RFK] Set S%d DPD backoff to 0dB\n", path);
			}
		}
	} else {
		dpk->dpk_gs[phy] = 0x5b;
	}
}

void halrf_dpk_init_8730e(struct rf_info *rf)
{
	halrf_set_dpd_backoff_8730e(rf, HW_PHY_0);
}

void halrf_set_rxbb_bw_8730e(struct rf_info *rf, enum channel_width bw, enum rf_path path)
{
#if 0
	halrf_wrf(rf, path, 0xee, BIT(2), 0x1);
	halrf_wrf(rf, path, 0x33, 0x0001F, 0x12); /*[4:0]*/

	if (bw == CHANNEL_WIDTH_20) {
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x1b);        /*[5:0]*/
	} else if (bw == CHANNEL_WIDTH_40) {
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x13);        /*[5:0]*/
	} else if (bw == CHANNEL_WIDTH_80) {
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0xb);        /*[5:0]*/
	} else {
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x3);        /*[5:0]*/
	}

	RF_DBG(rf, DBG_RF_RFK, "[RFK] set S%d RXBB BW 0x3F = 0x%x\n", path,
	       halrf_rrf(rf, path, 0x3f, 0x0003F));

	halrf_wrf(rf, path, 0xee, BIT(2), 0x0);
#endif
}

void halrf_rxbb_bw_8730e(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw)
{
	u8 kpath, path;

	kpath = halrf_kpath_8730e(rf, phy);

	for (path = 0; path < 1; path++) {
		if ((kpath & BIT(path)) && (rf->pre_rxbb_bw[path] != bw)) {
			halrf_set_rxbb_bw_8730e(rf, bw, path);
			rf->pre_rxbb_bw[path] = bw;
		} else
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] S%d RXBB BW unchanged (pre_bw = 0x%x)\n",
			       path, rf->pre_rxbb_bw[path]);
	}
}

void halrf_disconnect_notify_8730e(struct rf_info *rf, struct rtw_chan_def *chandef)
{

	struct halrf_iqk_info *iqk_info = &rf->iqk;
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	u8 path, ch;

	RF_DBG(rf, DBG_RF_RFK, "[IQK]===>%s\n", __func__);
	/*[IQK disconnect]*/
	for (ch = 0; ch < 2; ch++) {
		for (path = 0; path < KPATH; path++) {
			if (iqk_info->iqk_mcc_ch[ch][path] == chandef->center_ch) {
				iqk_info->iqk_mcc_ch[ch][path] = 0x0;
			}
		}

	}
	/*TXGAPK*/
	for (ch = 0; ch < 2; ch++) {
		if (txgapk_info->txgapk_mcc_ch[ch] == chandef->center_ch) {
			txgapk_info->txgapk_mcc_ch[ch] = 0x0;
		}
	}
}

bool halrf_check_mcc_ch_8730e(struct rf_info *rf, struct rtw_chan_def *chandef)
{

	struct halrf_iqk_info *iqk_info = &rf->iqk;
	u8 path, ch;

	bool check = false;
	RF_DBG(rf, DBG_RF_RFK, "[IQK]===>%s, center_ch(%d)\n", __func__, chandef->center_ch);
	/*[IQK check_mcc_ch]*/
	for (ch = 0; ch < 2; ch++) {
		for (path = 0; path < KPATH; path++) {
			if (iqk_info->iqk_mcc_ch[ch][path] == chandef->center_ch) {
				check = true;
				return check;
			}
		}
	}
	return check;
}

void halrf_fw_ntfy_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	u8 i = 0x0;
	u32 data_to_fw[5] = {0};
	u16 len = (u16)(sizeof(data_to_fw) / sizeof(u32)) * 4;

	return;
	data_to_fw[0] = (u32) iqk_info->iqk_mcc_ch[0][0];
	data_to_fw[1] = (u32) iqk_info->iqk_mcc_ch[0][1];
	data_to_fw[2] = (u32) iqk_info->iqk_mcc_ch[1][0];
	data_to_fw[3] = (u32) iqk_info->iqk_mcc_ch[1][1];
	data_to_fw[4] = rf->hal_com->band[phy_idx].cur_chandef.center_ch;

	RF_DBG(rf, DBG_RF_RFK, "[IQK] len = 0x%x\n", len);
	for (i = 0; i < 5; i++) {
		RF_DBG(rf, DBG_RF_RFK, "[IQK] data_to_fw[%x] = 0x%x\n", i, data_to_fw[i]);
	}

	halrf_fill_h2c_cmd(rf, len, FWCMD_H2C_GET_MCCCH, 0xa, H2CB_TYPE_DATA, (u32 *) data_to_fw);

	return;
}

void halrf_before_one_shot_enable_8730e(struct rf_info *rf)
{

	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

	/* set 0x80d4[21:16]=0x03 (before oneshot NCTL) to get report later */
	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x03);

	RF_DBG(rf, DBG_RF_RFK, "======> before set one-shot bit, 0x%x= 0x%x\n", 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD));
}


bool halrf_one_shot_nctl_done_check_8730e(struct rf_info *rf, enum rf_path path)
{

	/* for check status */
	u32 r_bff8 = 0;
	u32 r_80fc = 0;
	bool is_ready = false;
	u16 count = 1;

	rf->nctl_ck_times[0] = 0;
	rf->nctl_ck_times[1] = 0;

	/* for 0xbff8 check NCTL DONE */
	while (count < 2000) {
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);

		if (r_bff8 == 0x55) {
			is_ready = true;
			break;
		}
		halrf_delay_us(rf, 10);
		count++;
	}

	halrf_delay_us(rf, 1);
	/* txgapk_info->txgapk_chk_cnt[path][id][0] = count; */
	rf->nctl_ck_times[0] = count;

	RF_DBG(rf, DBG_RF_RFK, "======> check 0xBFF8[7:0] = 0x%x, IsReady = %d, ReadTimes = %d,delay 1 us\n", r_bff8, is_ready, count);


	/* for 0x80fc check NCTL DONE */
	count = 1;
	is_ready = false;
	while (count < 2000) {
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);

		if (r_80fc == 0x8000) {
			is_ready = true;
			break;
		}
		halrf_delay_us(rf, 1);
		count++;
	}

	halrf_delay_us(rf, 1);
	/* txgapk_info->txgapk_chk_cnt[path][id][1] = count; */
	rf->nctl_ck_times[1] = count;

	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

	RF_DBG(rf, DBG_RF_RFK, "======> check 0x80fc[15:0] = 0x%x, IsReady = %d, ReadTimes = %d, 0x%x= 0x%x \n", r_80fc, is_ready, count, 0x8010, halrf_rreg(rf, 0x8010,
			MASKDWORD));

	return is_ready;
}

void halrf_aack_8730e(struct rf_info *rf)
{
	u32 c = 0;
	u32 busy;
	/*wait LCK*/
	while (c < 10000) {
		busy = halrf_rrf(rf, RF_PATH_A, 0xd4, BIT(4));
		if (busy == 0) {
			break;
		}
		halrf_delay_us(rf, 10);
		c++;
	}

	RF_DBG(rf, DBG_RF_LCK, "[LCK]AACK start!!!!!!!\n");
	c = 0;
	halrf_wrf(rf, RF_PATH_A, 0xb3, BIT(3), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0xb3, BIT(2), 0x1);
	/*wait AACK*/
	while (c < 10000) {
		busy = halrf_rrf(rf, RF_PATH_A, 0xb3, BIT(1));
		if (busy == 0) {
			break;
		}
		halrf_delay_us(rf, 10);
		c++;
	}
	RF_DBG(rf, DBG_RF_LCK, "[LCK]AACK end!!!!!!!\n");
}

void halrf_x2k_8730e(struct rf_info *rf)
{
	u32 c = 0;
	u32 busy;
	u32 rf_18;

	RF_DBG(rf, DBG_RF_LCK, "[LCK]X2K start!!!!!!!\n");
	rf_18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);

	halrf_wrf(rf, RF_PATH_A, 0x18, MASKRF, 0x10D08);
	//force reset prior data
	halrf_wrf(rf, RF_PATH_A, 0xbf, BIT(18), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0xbf, BIT(2), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0xbd, MASKRF, 0x00000);
	halrf_wrf(rf, RF_PATH_A, 0xbd, MASKRF, 0x80000);
	halrf_wrf(rf, RF_PATH_A, 0xbf, MASKRF, 0x00000);

	halrf_wrf(rf, RF_PATH_A, 0xbd, MASKRF, 0x00000);
	halrf_wrf(rf, RF_PATH_A, 0xbd, MASKRF, 0x80000);
	/*wait X2K*/
	while (c < 1000) {
		busy = halrf_rrf(rf, RF_PATH_A, 0xbc, BIT(15));
		if (busy == 0) {
			break;
		}
		halrf_delay_us(rf, 10);
		c++;
	}
	halrf_wrf(rf, RF_PATH_A, 0x18, MASKRF, rf_18);
	RF_DBG(rf, DBG_RF_LCK, "[LCK]Count(%d), X2K end!!!!!!!\n", c);
}

void halrf_synk_8730e(struct rf_info *rf)
{
	//halrf_aack_8730e(rf);
	halrf_x2k_8730e(rf);
}

void halrf_lck_8730e(struct rf_info *rf)
{
	u32 temp18, busy, c = 0;

	RF_DBG(rf, DBG_RF_LCK, "[LCK]DO LCK\n");
	temp18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
	halrf_wrf(rf, RF_PATH_A, 0x18, MASKRF, temp18);
	halrf_delay_us(rf, 10);
	while (c < 10000) {
		busy = halrf_rrf(rf, RF_PATH_A, 0xb3, BIT(1));
		if (busy == 0) {
			break;
		}
		halrf_delay_us(rf, 10);
		c++;
	}

	rf->lck_ther_s0 = rf->cur_ther_s0;
}

void halrf_lck_tracking_8730e(struct rf_info *rf)
{
	u32 c = 0, temp;
	u32 i, ther = 0, ther_avg = 0;
	u32 rf_b3_val = 0, rf_b3_cur = 0;
	u8 thermal_a = 0xff;
	s8 ther_diff = 0;
	bool do_lck = false;

	halrf_efuse_get_info(rf, EFUSE_INFO_RF_THERMAL_A, &thermal_a, 1);
	RF_DBG(rf, DBG_RF_LCK, "[LCK][lite]efuse_thermal = %x\n", thermal_a);
	RF_DBG(rf, DBG_RF_LCK, "[LCK][lite]lck_ther_s0 = %x\n", rf->lck_ther_s0);

	if (rf->lck_ther_s0 == 0 || thermal_a == 0xff) {
		halrf_lck_trigger(rf);
		return;
	}

		temp = HALRF_ABS(rf->lck_ther_s0, rf->cur_ther_s0);

	ther_diff = rf->cur_ther_s0 - thermal_a;

	if (ther_diff >= 96) {
		rf_b3_val = 0x6;
	} else if (ther_diff >= 80) {
		rf_b3_val = 0x5;
	} else if (ther_diff >= 64) {
		rf_b3_val = 0x4;
	} else if (ther_diff >= 48) {
		rf_b3_val = 0x3;
	} else if (ther_diff >= 32) {
		rf_b3_val = 0x2;
	} else if (ther_diff >= 16) {
		rf_b3_val = 0x1;
	}

	rf_b3_cur = halrf_rrf(rf, RF_PATH_A, 0xb3, 0x1F0);

	if ((rf_b3_cur != rf_b3_val) && (temp > 3)) {
		//RF_DBG(rf, DBG_RF_LCK, "[LCK][lite]efuse_thermal = %x\n",thermal_a);
		halrf_wrf(rf, RF_PATH_A, 0xb3, 0x1F0, rf_b3_val);
		RF_DBG(rf, DBG_RF_LCK, "[LCK][Smart]b3 = %x\n", halrf_rrf(rf, RF_PATH_A, 0xb3, MASKRF));
		do_lck = true;
	}

	if (temp >= LCK_TH_8730E) {
		do_lck = true;
	}

	if (do_lck) {
		halrf_lck_trigger(rf);
	}

	/*
		RF_DBG(rf, DBG_RF_LCK, "[LCK]MAX_delta = %d, LCK_TH=%x\n",
			temp, LCK_TH_8730E);

		if(temp >= LCK_TH_8730E) {
			halrf_aack_8730e(rf);
		halrf_lck_trigger(rf);
	}
	*/
}

static u32 check_rfc_reg[] = {0x9f, 0x0, 0x0, 0x0, 0x0,
			      0x0, 0x0, 0x10000, 0x1, 0x10001,
			      0x5, 0x10005, 0x8, 0x18, 0x10018,
			      0x2, 0x10002, 0x11, 0x10011, 0x53,
			      0x10055,	0x58, 0x63, 0x6e, 0x6f,
			      0x7e, 0x7f, 0x80, 0x81, 0x8d,
			      0x8f, 0x90, 0x92, 0x93, 0xa0,
			      0xb2, 0xc5
			     };
static u32 check_dack_reg[] = {0x12a0, 0x32a0, 0x12b8, 0x32b8, 0x030c,
			       0x032c, 0xc000, 0xc004, 0xc020, 0xc024,
			       0xc100, 0xc104, 0xc120, 0xc124, 0xc0d4,
			       0xc1d4, 0xc0f0, 0xc0f4, 0xc1f0, 0xc1f4,
			       0xc05c, 0xc080, 0xc048, 0xc06c, 0xc060,
			       0xc084, 0xc05c, 0xc080, 0xc048, 0xc06c,
			       0xc060, 0xc084
			      };
static u32 check_iqk_reg[] = {0x8000, 0x8004, 0x8008, 0x8080, 0x808c,
			      0x8120, 0x8124, 0x8138, 0x813c, 0x81dc,
			      0x8220, 0x8224, 0x8238, 0x823c, 0x82dc,
			      0x9fe0, 0x9fe4, 0x9fe8, 0x9fec, 0x9f30,
			      0x9f40, 0x9f50, 0x9f60, 0x9f70, 0x9f80,
			      0x9f90, 0x9fa0
			     };
static u32 check_dpk_reg[] = {0x80b0, 0x81bc, 0x82bc, 0x81b4, 0x82b4,
			      0x81c4, 0x82c4, 0x81c8, 0x82c8, 0x58d4,
			      0x78d4
			     };
static u32 check_tssi_reg[] = {0x0304, 0x5818, 0x581c, 0x5820, 0x1c60,
			       0x1c44, 0x5838, 0x5630, 0x5634, 0x58f8,
			       0x12c0, 0x120c, 0x1c04, 0x1c0c, 0x1c18,
			       0x7630, 0x7634, 0x7818, 0x781c, 0x7820,
			       0x3c60, 0x3c44, 0x7838, 0x78f8, 0x32c0,
			       0x320c, 0x3c04, 0x3c0c, 0x3c18
			      };

void halrf_quick_checkrf_8730e(struct rf_info *rf)
{
	u32 path, temp, i;
	u32	len = sizeof(check_rfc_reg) / sizeof(u32);
	u32	*add = (u32 *)check_rfc_reg;

	/*check RFC*/
	RF_DBG(rf, DBG_RF_CHK, "======RFC======\n");
	for (path = 0; path < 2; path++) {
		for (i = 0; i < len; i++) {
			temp = halrf_rrf(rf, path, add[i], MASKRF);
			RF_DBG(rf, DBG_RF_CHK, "RF%d 0x%x = 0x%x\n",
			       path,
			       add[i],
			       temp);
		}
	}
	/*check DACK*/
	RF_DBG(rf, DBG_RF_CHK, "======DACK======\n");
	len = sizeof(check_dack_reg) / sizeof(u32);
	add = check_dack_reg;
	for (i = 0; i < len; i++) {
		temp = halrf_rreg(rf, add[i], MASKDWORD);
		RF_DBG(rf, DBG_RF_CHK, "0x%x = 0x%x\n",
		       add[i],
		       temp);
	}
	/*check IQK*/
	RF_DBG(rf, DBG_RF_CHK, "======IQK======\n");
	len = sizeof(check_iqk_reg) / sizeof(u32);
	add = check_iqk_reg;

	for (i = 0; i < len; i++) {
		temp = halrf_rreg(rf, add[i], MASKDWORD);
		RF_DBG(rf, DBG_RF_CHK, "0x%x = 0x%x\n",
		       add[i],
		       temp);
	}
	/*check DPK*/
	RF_DBG(rf, DBG_RF_CHK, "======DPK======\n");
	len = sizeof(check_dpk_reg) / sizeof(u32);
	add = check_dpk_reg;
	for (i = 0; i < len; i++) {
		temp = halrf_rreg(rf, add[i], MASKDWORD);
		RF_DBG(rf, DBG_RF_CHK, "0x%x = 0x%x\n",
		       add[i],
		       temp);
	}
	/*check TSSI*/
	RF_DBG(rf, DBG_RF_CHK, "======TSSI======\n");
	len = sizeof(check_tssi_reg) / sizeof(u32);
	add = check_tssi_reg;
	for (i = 0; i < len; i++) {
		temp = halrf_rreg(rf, add[i], MASKDWORD);
		RF_DBG(rf, DBG_RF_CHK, "0x%x = 0x%x\n",
		       add[i],
		       temp);
	}
}

static u32 backup_mac_reg_8730e[] = {0x0};
static u32 backup_bb_reg_8730e[] = {0x12a0, 0x12b8, 0x2008, 0x030c, 0x032c, 0x8028};
static u32 backup_rf_reg_8730e[] = {0xee, 0xdf, 0xdb, 0x1f, 0x53, 0x63, 0x7c,
				    0x83, 0x87, 0x89, 0x8a, 0x8c, 0x94
				   };

#if 1
static struct halrf_iqk_ops iqk_ops = {
	.iqk_kpath = halrf_kpath_8730e,
	.iqk_mcc_page_sel = iqk_mcc_page_sel_8730e,
	.iqk_get_ch_info = iqk_get_ch_info_8730e,
	.iqk_preset = iqk_preset_8730e,
	.iqk_macbb_setting = iqk_macbb_setting_8730e,
	.iqk_start_iqk = iqk_start_iqk_8730e,
	.iqk_restore = iqk_restore_8730e,
	.iqk_afebb_restore = iqk_afebb_restore_8730e,
};

struct rfk_iqk_info rf_iqk_hwspec_8730e = {
	.rf_iqk_ops = &iqk_ops,
	.rf_max_path_num = 1,
	.rf_iqk_version = iqk_version_8730e,
	.rf_iqk_ch_num = 1,
	.rf_iqk_path_num = 1,
#if 0
	.backup_mac_reg = backup_mac_reg_8730e,
	.backup_mac_reg_num = ARRAY_SIZE(backup_mac_reg_8730e),
#endif
	.backup_mac_reg = backup_mac_reg_8730e,
	.backup_mac_reg_num = 0,
	.backup_bb_reg = backup_bb_reg_8730e,
	.backup_bb_reg_num = ARRAY_SIZE(backup_bb_reg_8730e),
	.backup_rf_reg = backup_rf_reg_8730e,
	.backup_rf_reg_num = ARRAY_SIZE(backup_rf_reg_8730e),
};

#endif
#endif
