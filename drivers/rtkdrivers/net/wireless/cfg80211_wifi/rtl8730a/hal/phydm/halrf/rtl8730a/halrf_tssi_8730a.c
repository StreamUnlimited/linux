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

#if 0
u8 _halrf_driver_rate_to_tssi_rate_8730a(void *dm_void, u8 rate)
{
	u8 tssi_rate = 0;

	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (rate == ODM_MGN_1M) {
		tssi_rate = 0;
	} else if (rate == ODM_MGN_2M) {
		tssi_rate = 1;
	} else if (rate == ODM_MGN_5_5M) {
		tssi_rate = 2;
	} else if (rate == ODM_MGN_11M) {
		tssi_rate = 3;
	} else if (rate == ODM_MGN_6M) {
		tssi_rate = 4;
	} else if (rate == ODM_MGN_9M) {
		tssi_rate = 5;
	} else if (rate == ODM_MGN_12M) {
		tssi_rate = 6;
	} else if (rate == ODM_MGN_18M) {
		tssi_rate = 7;
	} else if (rate == ODM_MGN_24M) {
		tssi_rate = 8;
	} else if (rate == ODM_MGN_36M) {
		tssi_rate = 9;
	} else if (rate == ODM_MGN_48M) {
		tssi_rate = 10;
	} else if (rate == ODM_MGN_54M) {
		tssi_rate = 11;
	} else if (rate >= ODM_MGN_MCS0 && rate <= ODM_MGN_MCS7) { // ODM_MGN_VHT4SS_MCS9
		tssi_rate = rate - ODM_MGN_MCS0 + 12;
	} else
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF]======>%s not exit tx rate\n", __func__);

	return tssi_rate;
}

u8 _halrf_tssi_rate_to_driver_rate_8730a(void *dm_void, u8 rate)
{
	u8 driver_rate = 0;
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (rate == 0) {
		driver_rate = ODM_MGN_1M;
	} else if (rate == 1) {
		driver_rate = ODM_MGN_2M;
	} else if (rate == 2) {
		driver_rate = ODM_MGN_5_5M;
	} else if (rate == 3) {
		driver_rate = ODM_MGN_11M;
	} else if (rate == 4) {
		driver_rate = ODM_MGN_6M;
	} else if (rate == 5) {
		driver_rate = ODM_MGN_9M;
	} else if (rate == 6) {
		driver_rate = ODM_MGN_12M;
	} else if (rate == 7) {
		driver_rate = ODM_MGN_18M;
	} else if (rate == 8) {
		driver_rate = ODM_MGN_24M;
	} else if (rate == 9) {
		driver_rate = ODM_MGN_36M;
	} else if (rate == 10) {
		driver_rate = ODM_MGN_48M;
	} else if (rate == 11) {
		driver_rate = ODM_MGN_54M;
	} else if (rate >= 12 && rate <= 19) { //83
		driver_rate = rate + ODM_MGN_MCS0 - 12;
	} else
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF]======>%s not exit tx rate\n", __func__);
	return driver_rate;
}
#endif
u32 _halrf_get_efuse_tssi_offset_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 offset = 0;

	return offset;
}

void halrf_tssi_get_efuse_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

u32 halrf_get_online_tssi_de_8730a(void *dm_void, u8 path, s32 pout)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 tssi_de = 0;

	return tssi_de;
}

void halrf_tssi_set_efuse_de_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

u32 halrf_tssi_set_de_8730a(void *dm_void, u32 tssi_de)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	s8 de = 0;
	s32 offset = 0, db_temp;
	u8 i, rate, channel = *dm->channel, bandwidth = *dm->band_width;
	u8 idxbyrate[20];
	u32 tssi_dbm;
	u32 reg0x3axx, idxoffset;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI] ======>%s\n", __func__);

	i = (u8)odm_get_bb_reg(dm, 0x1884, BIT(20));
	de = (s8)(tssi_de & 0xff);
	de += phydm_get_tssi_trim_de(dm, i);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI]de= 0x%x\n", de);

	if (i == RF_PATH_A) {
		odm_set_bb_reg(dm, R_0x4334, 0x0FF00000, de);  //HT40
		odm_set_bb_reg(dm, R_0x43b0, 0x000000FF, de);  //OFDM
		odm_set_bb_reg(dm, R_0x43b0, 0xFF000000, de);  //HT20
		odm_set_bb_reg(dm, R_0x43b0, 0x0000FF00, de);  // RF40M OFDM 6M,txsc=1/2
		odm_set_bb_reg(dm, R_0x43b0, 0x00FF0000, de);  // RF40M OFDM 6M,txsc=0
		odm_set_bb_reg(dm, R_0x433c, 0x0FF00000, de);  //CCK
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI]0x4334= 0x%x\n",
		       odm_get_bb_reg(dm, R_0x4334, MASKDWORD));
	} else {
		odm_set_bb_reg(dm, R_0x4344, 0x0FF00000, de);  //HT40
		odm_set_bb_reg(dm, R_0x43b4, 0x000000FF, de);  //HT40
		odm_set_bb_reg(dm, R_0x43b4, 0x0000FF00, de);  //OFDM
		odm_set_bb_reg(dm, R_0x43b4, 0x00FF0000, de);  //RF40M OFDM 6M
		odm_set_bb_reg(dm, R_0x43b4, 0xFF000000, de);  //RF40M OFDM 6M
		odm_set_bb_reg(dm, R_0x43b8, 0x000000FF, de);  //HT20
		odm_set_bb_reg(dm, R_0x434c, 0x0FF00000, de);  //CCK
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI]0x4344= 0x%x\n",
		       odm_get_bb_reg(dm, R_0x4344, MASKDWORD));
	}
	offset = (s32)((de + 0x80) & 0xff);
	tssi_dbm = (offset * 100 + 5) / 8;
	return tssi_dbm;
}

void halrf_tssi_set_de_for_tx_verify_8730a(void *dm_void, u32 tssi_de, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _halrf_tssi_anapar_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _halrf_tssi_rf_setting_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void halrf_tssi_dck_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}
void _halrf_tssi_set_powerlevel_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

u32 halrf_tssi_set_powerbyrate_pout_8730a(void *dm_void, s16 power_offset, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 mask;
	s32 pout;
	u16 rate_reg;
	u8 tx_rate = phydm_get_tx_rate(dm);
	s8 rateidx_offset = 0, value0 = 0;
	u8 value;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] ======>%s\n", __func__);

	rateidx_offset = (s8)(power_offset / 25);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] tx_rate = %d\n", tx_rate);

	switch (tx_rate) {
	case MGN_1M:
		rate_reg = 0x3A00;
		mask = MASKBYTE0;
		break;
	case MGN_2M:
		rate_reg = 0x3A00;
		mask = MASKBYTE1;
		break;
	case MGN_5_5M:
		rate_reg = 0x3A00;
		mask = MASKBYTE2;
		break;
	case MGN_11M:
		rate_reg = 0x3A00;
		mask = MASKBYTE3;
		break;
	case MGN_6M:
		rate_reg = 0x3A04;
		mask = MASKBYTE0;
		break;
	case MGN_9M:
		rate_reg = 0x3A04;
		mask = MASKBYTE1;
		break;
	case MGN_12M:
		rate_reg = 0x3A04;
		mask = MASKBYTE2;
		break;
	case MGN_18M:
		rate_reg = 0x3A04;
		mask = MASKBYTE3;
		break;
	case MGN_24M:
		rate_reg = 0x3A08;
		mask = MASKBYTE0;
		break;
	case MGN_36M:
		rate_reg = 0x3A08;
		mask = MASKBYTE1;
		break;
	case MGN_48M:
		rate_reg = 0x3A08;
		mask = MASKBYTE2;
		break;
	case MGN_54M:
		rate_reg = 0x3A08;
		mask = MASKBYTE3;
		break;
	case MGN_MCS0:
		rate_reg = 0x3A0c;
		mask = MASKBYTE0;
		break;
	case MGN_MCS1:
		rate_reg = 0x3A0c;
		mask = MASKBYTE1;
		break;
	case MGN_MCS2:
		rate_reg = 0x3A0c;
		mask = MASKBYTE2;
		break;
	case MGN_MCS3:
		rate_reg = 0x3A10;
		mask = MASKBYTE3;
		break;
	case MGN_MCS4:
		rate_reg = 0x3A10;
		mask = MASKBYTE0;
		break;
	case MGN_MCS5:
		rate_reg = 0x3A10;
		mask = MASKBYTE1;
		break;
	case MGN_MCS6:
		rate_reg = 0x3A10;
		mask = MASKBYTE2;
		break;
	case MGN_MCS7:
		rate_reg = 0x3A10;
		mask = MASKBYTE3;
		break;
	default:
		rate_reg = 0x3A10;
		mask = MASKBYTE3;
		break;
	}

	value0 = (s8)(odm_get_bb_reg(dm, rate_reg, mask) & 0xff) + rateidx_offset;
	value = (u8)(value0 & 0xFF);
	odm_set_bb_reg(dm, rate_reg, mask, value);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] 0x%x [%x]= 0x%x \n", rate_reg, mask, value);
	pout = (s32)value0 * 100 / 4 + 1600;
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] Pout = %d (/100)\n", pout);
	return pout;
}

void _halrf_tssi_set_txpwr_bb_com_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _halrf_tssi_set_tmeter_tbl_zero_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
}

void _halrf_tssi_set_tmeter_tbl_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _tssi_tx_pause_8730a(struct dm_struct *dm)
{
	u8 reg_rf0_a, reg_rf0_b;
	u16 count = 0;

	odm_write_1byte(dm, R_0x522, 0xff);
	odm_set_bb_reg(dm, R_0x1e70, 0x0000000f, 0x2); /*hw tx stop*/

	reg_rf0_a = (u8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000);
	reg_rf0_b = (u8)odm_get_rf_reg(dm, RF_PATH_B, RF_0x00, 0xF0000);

	while (((reg_rf0_a == 2) || (reg_rf0_b == 2)) && count < 2500) {
		reg_rf0_a = (u8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000);
		reg_rf0_b = (u8)odm_get_rf_reg(dm, RF_PATH_B, RF_0x00, 0xF0000);
		ODM_delay_us(2);
		count++;
	}

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI] Tx pause!!\n");

}

void _halrf_tssi_set_rf_gap_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _halrf_tssi_set_slope_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

}

void _halrf_tssi_set_slope_cal_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void _halrf_tssi_set_tssi_track_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	//odm_set_bb_reg(dm, R_0x4320, BIT(24), 0x0);
	//odm_set_bb_reg(dm, R_0x439c, 0x0FFFFFF0, 0x080080);//0.125db/cw
}

void _halrf_run_tssi_slope_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	//wire r_tssi_en
	odm_set_bb_reg(dm, R_0x4318, BIT(28), 0x0);
	odm_set_bb_reg(dm, R_0x4318, BIT(28), 0x1);
}

void _halrf_rpt_tssi_adc_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
}

void halrf_enable_tssi_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] ======>%s\n", __func__);
}

void halrf_disable_tssi_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] ======>%s\n", __func__);
}

void _halrf_tssi_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][TSSI] ======>%s\n", __func__);
}
void halrf_do_tssi_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	//TBD
}

#endif
