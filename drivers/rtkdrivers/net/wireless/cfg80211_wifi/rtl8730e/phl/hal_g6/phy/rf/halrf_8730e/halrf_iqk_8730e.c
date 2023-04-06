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

#ifdef RF_8730E_SUPPORT

void _iqk_rxk_setting_8730e(struct rf_info *rf)
{
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//05_BW2040_DAC_off_ADC0160_20220519
	halrf_wreg(rf, 0x20fc, BIT(16), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28), 0x0);
	halrf_wreg(rf, 0x12a0, BIT(19), 0x1);
	halrf_wreg(rf, 0x12a0, 0x00070000, 0x6);
	halrf_wreg(rf, 0x5670, 0x60000000, 0x1);
	halrf_wreg(rf, 0x12b8, BIT(28), 0x1);
	halrf_wreg(rf, 0x030c, 0xFF000000, 0x7);
	halrf_wreg(rf, 0x032c, 0xFFFF0000, 0x1);
	halrf_wreg(rf, 0x12b8, BIT(28), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28), 0x1);
}

bool _iqk_check_cal_8730e(struct rf_info *rf, u8 ktype)
{
	bool notready = true, fail = true;
	u16 delay_count = 0;

	/*RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);*/
	while (notready) {
		if (halrf_rreg(rf, 0xbff8, MASKBYTE0) == 0x55) {
			halrf_delay_us(rf, 10);
			if (halrf_rreg(rf, 0x8010, MASKBYTE0) == 0x55) {
				notready = false;
			}
		} else {
			halrf_delay_us(rf, 10);
			delay_count++;
		}
		if (delay_count > 8200) {
			fail = true;
			RF_DBG(rf, DBG_RF_IQK, "[IQK]IQK timeout!!!\n");
			break;
		}
	}

	if (!notready) {
		fail = (bool)halrf_rreg(rf, 0x8008, BIT(26));
	}

	halrf_wreg(rf, 0x8010, MASKBYTE0, 0x0);
	RF_DBG(rf, DBG_RF_IQK, "[IQK]cnt= %d\n", delay_count);
	RF_DBG(rf, DBG_RF_IQK, "[IQK] Ktype= %x (%s)\n", ktype, fail ? "Fail" : "Done");
	return fail;
}

void _iqk_rxsram_8730e(struct rf_info *rf)
{
#ifdef RFDBG_TRACE_EN
	u16 i;

	halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x8034, BIT(0), 0x1);
	halrf_wreg(rf, 0x8034, BIT(0), 0x0);
	halrf_delay_ms(rf, 2);

	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00030001);
	halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00000003);
	//read SRAM Data
	for (i = 0; i < 0x140; i++) {
		halrf_wreg(rf, 0x80d8, 0x000007FC, i);
		RF_DBG(rf, DBG_RF_RFK, "[IQK] SRAM%03x = 0x%x\n", i, halrf_rreg(rf, 0x80fc, MASKDWORD));
	}

	halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00000000);
#endif
}

void _iqk_nctl_xym_check_8730e(struct rf_info *rf, u8 ktype)
{
#ifdef RFDBG_TRACE_EN
	u32 reg_8038, reg_803c;
	u8 i;

	reg_8038 = halrf_rreg(rf, 0x8038, MASKDWORD);
	reg_803c = halrf_rreg(rf, 0x803c, MASKDWORD);

	switch (ktype) {
	case ID_TXK:
		halrf_wreg(rf, 0x801c, 0x00000003, 0x1);
		halrf_delay_us(rf, 100);
		halrf_wreg(rf, 0x8014, 0x000000FF, 0xe0);
		RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK_XYM0 = 0x%x\n", halrf_rreg(rf, 0x803c, MASKDWORD));
		halrf_delay_us(rf, 100);
		halrf_wreg(rf, 0x8014, 0x000000FF, 0xe1);
		RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK_XYM1 = 0x%x\n", halrf_rreg(rf, 0x803c, MASKDWORD));
		halrf_delay_us(rf, 100);
		halrf_wreg(rf, 0x8014, 0x000000FF, 0xe2);
		RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK_XYM2 = 0x%x\n", halrf_rreg(rf, 0x803c, MASKDWORD));
		halrf_wreg(rf, 0x803c, MASKDWORD, reg_803c);
		break;
	case ID_RXK:
		halrf_wreg(rf, 0x801c, 0x000000ff, 0x1);
		halrf_delay_us(rf, 100);
		for (i = 0; i < 0x6; i++) {
			halrf_wreg(rf, 0x8014, MASKDWORD, 0xc0 + i);
			halrf_wreg(rf, 0x8014, MASKDWORD, 0x00);
			RF_DBG(rf, DBG_RF_IQK, "[IQK] RXK_XYM%d = 0x%x\n", i, halrf_rreg(rf, 0x8038, MASKDWORD));
			halrf_delay_us(rf, 100);
		}
		halrf_wreg(rf, 0x8038, MASKDWORD, reg_8038);
		break;
	default:
		break;
	}

	halrf_wreg(rf, 0x801c, MASKDWORD, 0xA210FC00);
	halrf_wreg(rf, 0x8014, MASKDWORD, 0x00010100);
#endif
}

bool _iqk_one_shot_8730e(struct rf_info *rf, u8 ktype)
{
	bool fail = false;
	u16 iqk_cmd = 0x0;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_IQK, RFK_ONESHOT_START);
	if (halrf_rreg(rf, 0xbff8, MASKBYTE0) != 0x0) {
		halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);
		halrf_delay_us(rf, 100);
	}

	switch (ktype) {
	case ID_FLoK_coarse:
		iqk_cmd = 0x019;
		break;
	case ID_FLoK_fine:
		iqk_cmd = 0x119;
		break;
	case ID_FLOK_vbuffer:
		iqk_cmd = 0x219;
		break;
	case ID_TX_TIA_coarse:
		iqk_cmd = 0x319;
		break;
	case ID_TX_TIA_fine:
		iqk_cmd = 0x419;
		break;
	case ID_NBRXK:
		iqk_cmd = 0x519;
		break;
	case ID_RXK:
		iqk_cmd = 0x619;
		break;
	case ID_NBTXK:
		iqk_cmd = 0x819;
		break;
	case ID_TXK:
		iqk_cmd = 0x919;
		break;

	default:
		return false;
		break;
	}

	halrf_wreg(rf, 0x8000, 0x1FFF, iqk_cmd - 1);
	halrf_wreg(rf, 0x8000, 0x1FFF, iqk_cmd);

	fail = _iqk_check_cal_8730e(rf, ktype);

	//halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_IQK, RFK_ONESHOT_STOP);

	return fail;
}

bool _iqk_rxk_8730e(struct rf_info *rf, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	bool kfail = false;
	u8 cv_drv = rf->hal_com->cv;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//RF_DBG(rf, DBG_RF_IQK, "[IQK] RXK START \n");
	halrf_wrf(rf, path, 0x0, MASKRFMODE, 0x7);
	if (iqk_info->iqk_band[path] == BAND_ON_24G) { /*2G*/
		halrf_wrf(rf, path, 0x20, BIT(8), 0x1);
	} else { /*5G*/
		halrf_wrf(rf, path, 0x20, BIT(7), 0x1);
	}
	halrf_wrf(rf, path, 0x1f, MASKRF, halrf_rrf(rf, path, 0x18, MASKRF));
	//RF_DBG(rf, DBG_RF_IQK, "[IQK] RF 0x1f= 0x%x\n", halrf_rrf(rf, path, 0x1f, MASKRF));
	halrf_delay_ms(rf, 1);
	halrf_wrf(rf, path, 0x1e, 0x0003F, 0x13); //IQKPLL_FOS=4.25MHz
	halrf_wrf(rf, path, 0x1e, BIT(19), 0x0);
	halrf_wrf(rf, path, 0x1e, BIT(19), 0x1);
	halrf_delay_ms(rf, 10); // IQKPLL's settling time

	//++++++ lna small +++++++++++
	halrf_wrf(rf, path, 0x79, BIT(12), 0x0);
	halrf_wrf(rf, path, 0x0, 0x03FF0, 0x106);
	if (iqk_info->iqk_band[path] == BAND_ON_24G) { /*2G*/
		halrf_wrf(rf, path, 0x83, 0x00300, 0x2);
		halrf_wrf(rf, path, 0x83, 0x1FC00, 0x00);
		halrf_wrf(rf, path, 0x87, MASKRF, 0x54886);
	} else {
		halrf_wrf(rf, path, 0x8c, 0x00180, 0x2);
		halrf_wrf(rf, path, 0x8c, 0x0007F, 0x17);
		//IQK Pout -6dBm for RXA
		halrf_wrf(rf, path, 0x94, MASKRF, 0x00070);
		halrf_wrf(rf, path, 0x89, MASKRF, 0x000C4);
		halrf_wrf(rf, path, 0x8a, MASKRF, 0x5FC9F);
	}

	halrf_wreg(rf, 0x8024, MASKRF, halrf_rrf(rf, path, 0x0, MASKRF));
	//RF_DBG(rf, DBG_RF_IQK, "[IQK] 0x8024= 0x%x\n", halrf_rreg(rf, 0x8024, MASKDWORD));
	halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);
	halrf_wreg(rf, 0x8108, MASKDWORD, 0x02408083);
	halrf_wreg(rf, 0x8008, BIT(9), 0x1);
	//halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x3f);
	halrf_wreg(rf, 0x802c, 0x0FFF0000, 0x022); // Rx_tone_idx=0x022
	/*oneshot*/
	kfail |= _iqk_one_shot_8730e(rf, ID_NBRXK);
	iqk_info->bp_rxkresult[0] = halrf_rreg(rf, 0x803c, MASKDWORD); //bp lnasmall
	//halrf_wreg(rf, 0x5670, BIT(1), 0);
	RF_DBG(rf, DBG_RF_IQK, "[IQK]LNA2 kfail= %d, 0x819C = 0x%x\n", kfail, halrf_rreg(rf, 0x819c, MASKDWORD));
#ifdef RFDBG_TRACE_EN
	if (rf->dbg_component & DBG_RF_RFK) {
		_iqk_nctl_xym_check_8730e(rf, ID_RXK);
		_iqk_rxsram_8730e(rf);
	}
#endif
	//++++++ lna large ++++++++++++++++++++
	if (iqk_info->iqk_band[path] == BAND_ON_24G) { /*2G*/
		//++++++ lna large ++++++++++++++++++++
		halrf_wrf(rf, path, 0x79, BIT(12), 0x0);
		if (cv_drv == CAV) { /*A-CUT*/
			halrf_wrf(rf, path, 0x0, 0x03FF0, 0x300);
		} else {
			halrf_wrf(rf, path, 0x0, 0x03FF0, 0x316);
		}
		halrf_wrf(rf, path, 0x83, 0x00300, 0x3);
		halrf_wrf(rf, path, 0x83, 0x1FC00, 0x7f);
		halrf_wrf(rf, path, 0x87, MASKRF, 0x54886);
		halrf_wreg(rf, 0x8024, MASKRF, halrf_rrf(rf, path, 0x0, MASKRF));
		//RF_DBG(rf, DBG_RF_IQK, "[IQK] 0x8024= 0x%x\n", halrf_rreg(rf, 0x8024, MASKDWORD));
		halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);
		halrf_wreg(rf, 0x8108, MASKDWORD, 0x03408086);
		halrf_wreg(rf, 0x8008, BIT(9), 0x1);
		//halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
		halrf_wreg(rf, 0x80cc, 0x0000003F, 0x3f);
		halrf_wreg(rf, 0x802c, 0x0FFF0000, 0x022); // Rx_tone_idx=0x022
		/*oneshot*/
		kfail |= _iqk_one_shot_8730e(rf, ID_NBRXK);
		//halrf_wreg(rf, 0x5670, BIT(1), 0);
	} else if (iqk_info->iqk_band[path] == BAND_ON_5G) {
		//++++++ lna large ++++++++++++++++++++
		halrf_wrf(rf, path, 0x79, BIT(12), 0x0);
		if (cv_drv == CAV) { /*A-CUT*/
			halrf_wrf(rf, path, 0x0, 0x03FF0, 0x300);
		} else {
			halrf_wrf(rf, path, 0x0, 0x03FF0, 0x310);
		}
		halrf_wrf(rf, path, 0x8c, 0x00180, 0x0);
		halrf_wrf(rf, path, 0x8c, 0x0007f, 0x7f);
		//IQK Pout -6dBm for RXA
		halrf_wrf(rf, path, 0x94, MASKRF, 0x00070);
		halrf_wrf(rf, path, 0x89, MASKRF, 0x000C4);
		halrf_wrf(rf, path, 0x8a, MASKRF, 0x5FC9F);
		halrf_wreg(rf, 0x8024, MASKRF, halrf_rrf(rf, path, 0x0, MASKRF));
		//RF_DBG(rf, DBG_RF_IQK, "[IQK] 0x8024= 0x%x\n", halrf_rreg(rf, 0x8024, MASKDWORD));
		halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);
		halrf_wreg(rf, 0x8108, MASKDWORD, 0x03608086);
		halrf_wreg(rf, 0x8008, BIT(9), 0x1);
		//halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
		halrf_wreg(rf, 0x80cc, 0x0000003F, 0x3f);
		halrf_wreg(rf, 0x802c, 0x0FFF0000, 0x022); // Rx_tone_idx=0x022
		/*oneshot*/
		kfail |= _iqk_one_shot_8730e(rf, ID_NBRXK);

	} else {
		halrf_wreg(rf, 0x81a0, MASKDWORD, halrf_rreg(rf, 0x803c, MASKDWORD));
	}

	iqk_info->bp_rxkresult[1] = halrf_rreg(rf, 0x803c, MASKDWORD); //bp lnalarge
	RF_DBG(rf, DBG_RF_IQK, "[IQK]LNA6 kfail= %d, 0x81a0 = 0x%x\n", kfail,
	       halrf_rreg(rf, 0x81a0, MASKDWORD));
#ifdef RFDBG_TRACE_EN
	if (rf->dbg_component & DBG_RF_RFK) {
		_iqk_nctl_xym_check_8730e(rf, ID_RXK);
		_iqk_rxsram_8730e(rf);
	}
#endif
	halrf_wreg(rf, 0x8008, BIT(9), 0x0);
	halrf_wrf(rf, path, 0x20, BIT(8) | BIT(7), 0x0);
	halrf_wrf(rf, path, 0x1e, BIT(19), 0x0);

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ RXK END ============//\n");

	return kfail;
}

void _iqk_wa_for_5ghb_8730e(struct rf_info *rf, bool pa_on)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	//----- 20230131 WA for HB PA ON @TXIQK mode ----//
	if (pa_on == 1) {
		// Modify TXIQK mode table
		halrf_wrf(rf, RF_PATH_A, 0xef, MASKRF, 0x80000);
		halrf_wrf(rf, RF_PATH_A, 0x33, MASKRF, 0x00004);
		halrf_wrf(rf, RF_PATH_A, 0x3E, MASKRF, 0x03d01); //PA ON
		halrf_wrf(rf, RF_PATH_A, 0x3F, MASKRF, 0x7e0d4);
		halrf_wrf(rf, RF_PATH_A, 0xef, MASKRF, 0x00000);
		// debug 0xde[13]=1 , backup 0x61
		iqk_info->temp_rf61 = halrf_rrf(rf, RF_PATH_A, 0x61, MASKRF);
		halrf_wrf(rf, RF_PATH_A, 0xDE, BIT(13), 0x1);
		halrf_wrf(rf, RF_PATH_A, 0x61, 0x00F00, 0);
	} else {
		// Modify TXIQK mode table
		halrf_wrf(rf, RF_PATH_A, 0xef, MASKRF, 0x80000);
		halrf_wrf(rf, RF_PATH_A, 0x33, MASKRF, 0x00004);
		halrf_wrf(rf, RF_PATH_A, 0x3E, MASKRF, 0x01d01); //PA_OFF
		halrf_wrf(rf, RF_PATH_A, 0x3F, MASKRF, 0x7e0d4);
		halrf_wrf(rf, RF_PATH_A, 0xef, MASKRF, 0x00000);
		// restore 0x61
		halrf_wrf(rf, RF_PATH_A, 0x61, MASKRF, iqk_info->temp_rf61);
		halrf_wrf(rf, RF_PATH_A, 0xDE, BIT(13), 0x0);
	}
}

bool _iqk_txk_8730e(struct rf_info *rf, u8 path)
{
	//struct halrf_iqk_info *iqk_info = &rf->iqk;
	bool kfail = false;

	_iqk_wa_for_5ghb_8730e(rf, 1); //WA for HB PA ON

	halrf_wrf(rf, path, 0x57, 0x1F000, 0xd); //TXBB

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]TXK START\n");

	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x12);
	/*oneshot*/
	kfail = _iqk_one_shot_8730e(rf, ID_TXK);
	halrf_wreg(rf, 0x8018, 0x6, 0x0); //reset iQ

	//iqk_info->bp_txkresult[0] = halrf_rreg(rf, 0x8038, MASKDWORD);
#ifdef RFDBG_TRACE_EN
	if (rf->dbg_component & DBG_RF_RFK) {
		_iqk_nctl_xym_check_8730e(rf, ID_TXK);
	}
#endif
	RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK fail= %d, 0x8038 = 0x%x\n", kfail,
	       halrf_rreg(rf, 0x8038, MASKDWORD));

	_iqk_wa_for_5ghb_8730e(rf, 0);

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ TXK END ============//\n");

	return kfail;
}

bool _lok_finetune_check_8730e(struct rf_info *rf, u8 path)
{
	bool is_fail = false;
	u32 temp = 0x0;
	u8 core_i = 0x0, core_q = 0x0;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	temp = halrf_rrf(rf, path, 0x58, MASKRF);
	core_i = (temp & 0xf8000) >> 15;
	core_q = (temp & 0x07c00) >> 10;

	if (core_i  < 0x2 || core_i  > 0x1d || core_q < 0x2 || core_q > 0x1d) {
		is_fail = true;
	} else {
		is_fail = false;
	}

	RF_DBG(rf, DBG_RF_IQK, "[IQK] _lok_finetune_check %s\n", is_fail ? "fail" : "OK");
	return is_fail;
}

#if 0
bool _iqk_xy_check_8730e(struct rf_info *rf, u8 ktype)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	bool is_fail = false;
	u32 temp[2];
	s16 tx_i[2], tx_q[2];
	s16 rx_i[2][2], rx_q[2][2];
	u16 diff[4];
	u8 i, j;

	/*compare 2's IQC XYM*/
	switch (ktype) {
	case ID_TXK:
		for (i = 0; i < 2; i++) {
			temp[i] = iqk_info->bp_txkresult[i];
			tx_i[i] = (u16)((temp[i] & 0x7FF00000) >> 20);
			tx_q[i] = (u16)((temp[i] & 0x0007FF00) >> 8);
			if ((tx_i[i] & BIT(10)) >> 10 == 1) {
				tx_i[i] |= 0xf800;
			}
			if ((tx_q[i] & BIT(10)) >> 10 == 1) {
				tx_q[i] |= 0xf800;
			}
			//RF_DBG(rf, DBG_RF_IQK, "[IQK]tx_i = 0x%x, tx_q = 0x%x\n", tx_i[i], tx_q[i]);
		}
		diff[0] = HALRF_ABS(tx_i[1], tx_i[0]);
		diff[1] = HALRF_ABS(tx_q[1], tx_q[0]);
		RF_DBG(rf, DBG_RF_IQK, "[IQK]TXK diff[0] = 0x%x, diff[1] = 0x%x\n", diff[0], diff[1]);
		if (diff[0] + diff[1] > 10) {
			is_fail = true;
		}
		break;
	case ID_RXK:
		for (j = 0; j < 1; j++) {
			for (i = 0; i < 2; i++) {
				temp[i] = iqk_info->bp_rxkresult[i];
				rx_i[j][i] = (u16)((temp[i] & 0x7FF00000) >> 20);
				rx_q[j][i] = (u16)((temp[i] & 0x0007FF00) >> 8);
				if ((rx_i[j][i] & BIT(10)) >> 10 == 1) {
					rx_i[j][i] |= 0xf800;
				}
				if ((rx_q[j][i] & BIT(10)) >> 10 == 1) {
					rx_q[j][i] |= 0xf800;
				}
				//RF_DBG(rf, DBG_RF_IQK, "[IQK]rx_i = 0x%x, rx_q = 0x%x\n", rx_i[j][i], rx_q[j][i]);
			}
			diff[2 * j] = HALRF_ABS(rx_i[j][1], rx_i[j][0]);
			diff[2 * j + 1] = HALRF_ABS(rx_q[j][1], rx_q[j][0]);
			RF_DBG(rf, DBG_RF_IQK, "[IQK]RXK diff[%d] = 0x%x, diff[%d] = 0x%x\n",
			       2 * j, diff[2 * j], 2 * j + 1, diff[2 * j + 1]);
		}
		if (diff[0] + diff[1] > 10) {
			is_fail = true;
		}
		break;
	default:
		break;
	}
	RF_DBG(rf, DBG_RF_IQK, "[IQK]_iqk_xy_check_fail = %d\n", is_fail);

	return is_fail;
}
#endif

void _iqk_tia_dck_8730e(struct rf_info *rf, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	bool fail = false;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ TIA DCK START ============//\n");

	halrf_wrf(rf, path, 0x57, 0x1F000, 0x6); //TXBB

	//============ START: _lok_coarse_wo_swap_after_vbufferlok ============ //
	//SW_SI 0x4[24]=0x1
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x12); //ItQt

	fail = _iqk_one_shot_8730e(rf, ID_TX_TIA_coarse);

	//SW_SI 0x4[24]=0x0
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	// ====== End : Coasrse TIA_DCK =====

	// ====== START : _lok_vbuffer =====
	halrf_wrf(rf, path, 0x57, 0x1F000, 0x12); //TXBB
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x24); //ItQt

	fail |= _iqk_one_shot_8730e(rf, ID_FLOK_vbuffer);

	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0);

	iqk_info->lok_fin_fail[0][path] = fail;

	RF_DBG(rf, DBG_RF_IQK, "[IQK] TIA_DCK %s, 0x7a = 0x%x\n",
	       fail ? "fail" : "OK", halrf_rrf(rf, path, 0x7a, MASKRF));
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ TIA_DCK END ============//\n");
}

bool _iqk_lok_8730e(struct rf_info *rf, u8 path)
{
	bool fail = false;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ FLOK START ============//\n");

	halrf_wrf(rf, path, 0x57, 0x1F000, 0x0); //TXBB

	//============ START: _lok_coarse_fine_wi_swap ============ //
	//SW_SI 0x4[24]=0x1
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x802c, 0x00000FFF, 0x24); //Tx Tone idx
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x09); //ItQt

	_iqk_one_shot_8730e(rf, ID_FLoK_fine);

	//SW_SI 0x4[24]=0x0
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	RF_DBG(rf, DBG_RF_IQK, "[IQK] LOK coarse %s, 0x08 = 0x%x\n",
	       fail ? "fail" : "OK", halrf_rrf(rf, path, 0x08, MASKRF));
	// ====== End : Coasrse FLOK =====

	halrf_wrf(rf, path, 0x57, 0x1F000, 0x12); //TXBB

	//============ START: _lok_vbuffer ============ //
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x24); //ItQt

	_iqk_one_shot_8730e(rf, ID_FLOK_vbuffer);

	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0);
	RF_DBG(rf, DBG_RF_IQK, "[IQK] LOK fine %s, 0x09 = 0x%x\n",
	       fail ? "fail" : "OK", halrf_rrf(rf, path, 0x09, MASKRF));
	// ====== End : Fine FLOK =====
	fail = _lok_finetune_check_8730e(rf, path);

	if (fail) {
		halrf_wrf(rf, path, 0x08, MASKRF, 0x80200);
		halrf_wrf(rf, path, 0x09, MASKRF, 0x80200);
		RF_DBG(rf, DBG_RF_IQK, "[IQK] LOK Fail, write default value !!!\n");
	}

	_iqk_tia_dck_8730e(rf, path);

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]//============ FLOK END ============//\n");

	return fail;
}

void _iqk_txk_setting_8730e(struct rf_info *rf, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	if (iqk_info->iqk_band[path] == BAND_ON_24G) { /*2G*/
		//03_SwapLOK_TxK_Gmode_NCTL_2022704
		halrf_wrf(rf, path, 0xdf, 0x000FF, 0x98); //TX Gain Debug
		halrf_wrf(rf, path, 0x5, BIT(1), 0x1);
		halrf_wrf(rf, path, 0xee, BIT(19), 0x1); //WE_LUT_TX_LOK
		halrf_wrf(rf, path, 0x7c, 0x0000F, 0x0);
		halrf_wrf(rf, path, 0x30, 0x0F000, 0x0);
		halrf_wrf(rf, path, 0x0, 0xFFFF0, 0x403e);
		halrf_wrf(rf, path, 0x56, 0x0FFF0, 0xe0e);
		//0x57[16:12]=TxBB
		halrf_wrf(rf, path, 0x57, 0x1F000, 0xa);
		halrf_wreg(rf, 0x8028, BIT(31), 0x0); //diff_ob
		halrf_wreg(rf, 0x8018, BIT(2), 0x1); //Q-off
		//RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK Gmode setting done \n");
	} else { /*5G*/
		halrf_wrf(rf, path, 0xdf, 0x000FF, 0x98); //TX Gain Debug
		halrf_wrf(rf, path, 0x5, BIT(1), 0x1);
		halrf_wrf(rf, path, 0x63, 0x001C0, 0x0);
		halrf_wrf(rf, path, 0xee, BIT(19), 0x1); //WE_LUT_TX_LOK
		halrf_wrf(rf, path, 0x7c, 0x0000F, 0x4);
		halrf_wrf(rf, path, 0x30, 0x0F000, 0x4);
		halrf_wrf(rf, path, 0x0, 0xFFFF0, 0x403e);
		halrf_wrf(rf, path, 0x56, 0x0FFF0, 0xe4e);
		//0x57[16:12]=TxBB
		halrf_wrf(rf, path, 0x57, 0x1F000, 0xa);
		halrf_wreg(rf, 0x8028, BIT(31), 0x1); //diff_ob
		halrf_wreg(rf, 0x8018, BIT(1), 0x1); //i-off
		//RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK Amode setting done \n");
	}
}

void _iqk_info_iqk_8730e(struct rf_info *rf, u8 path)
{
#ifdef RFDBG_TRACE_EN
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	iqk_info->thermal[path] = halrf_get_thermal_8730e(rf, path);
	iqk_info->thermal_rek_en = false;

	RF_DBG(rf, DBG_RF_IQK, "[IQK] S%d_thermal = %d\n", path,
	       iqk_info->thermal[path]);
	RF_DBG(rf, DBG_RF_IQK, "[IQK] S%d_LOK_COR %s\n", path,
	       iqk_info->lok_cor_fail[0][path] ? "fail" : "OK");
	RF_DBG(rf, DBG_RF_IQK, "[IQK] S%d_LOK_FIN %s\n", path,
	       iqk_info->lok_fin_fail[0][path] ? "fail" : "OK");
	RF_DBG(rf, DBG_RF_IQK, "[IQK] S%d_TXIQK %s\n", path,
	       iqk_info->iqk_tx_fail[0][path] ? "fail" : "OK");
	RF_DBG(rf, DBG_RF_IQK, "[IQK] S%d_RXIQK %s\n", path,
	       iqk_info->iqk_rx_fail[0][path] ? "fail" : "OK");
#endif
}

void _iqk_by_path_8730e(struct rf_info *rf, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	bool lok_is_fail = false;
	u8 i = 0;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//LOK
	for (i = 0; i < 3; i++) {
		_iqk_txk_setting_8730e(rf, path);
		lok_is_fail = _iqk_lok_8730e(rf, path);
		if (!lok_is_fail) {
			break;
		}
	}
	iqk_info->lok_cor_fail[0][path] = lok_is_fail;

	//TXK
	iqk_info->iqk_tx_fail[0][path] = _iqk_txk_8730e(rf, path);

	//RX
	_iqk_rxk_setting_8730e(rf);
	iqk_info->iqk_rx_fail[0][path] = _iqk_rxk_8730e(rf, path);

	_iqk_info_iqk_8730e(rf, path);
}

void iqk_get_ch_info_8730e(struct rf_info *rf, enum phl_phy_idx phy, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);

	iqk_info->iqk_band[path] = rf->hal_com->band[phy].cur_chandef.band;
	iqk_info->iqk_bw[path] = rf->hal_com->band[phy].cur_chandef.bw;
	iqk_info->iqk_ch[path] = rf->hal_com->band[phy].cur_chandef.center_ch;
	RF_DBG(rf, DBG_RF_IQK, "[IQK]S%d (PHY%d): / DBCC %s/ %s/ CH%d/ %s\n",
	       path, phy,  rf->hal_com->dbcc_en ? "on" : "off",
	       iqk_info->iqk_band[path]  == 0 ? "2G" : (iqk_info->iqk_band[path]  == 1 ? "5G" : "6G"),
	       iqk_info->iqk_ch[path],
	       iqk_info->iqk_bw[path] == 0 ? "20M" : (iqk_info->iqk_bw[path] == 1 ? "40M" : "80M"));
	RF_DBG(rf, DBG_RF_IQK, "[IQK] times = 0x%x\n", iqk_info->iqk_times);
}

void iqk_start_iqk_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path)
{
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	_iqk_by_path_8730e(rf, path);
}

void iqk_restore_8730e(struct rf_info *rf, u8 path)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	halrf_wrf(rf, path, 0xee, BIT(19), 0x0);
	halrf_wrf(rf, path, 0x5, BIT(1), 0x0);
	halrf_wrf(rf, path, 0xee, BIT(18), 0x0);
	halrf_wrf(rf, path, 0xdb, BIT(1), 0x0);

	//RF_DBG(rf, DBG_RF_IQK, "===> %s\n", __func__);
	/*if K fail restore initial IQC value*/
	if (iqk_info->iqk_tx_fail[0][path] == 1) {
		halrf_wreg(rf, 0x8038, MASKDWORD, 0x20000000);
	}

	if (iqk_info->iqk_rx_fail[0][path] == 1) {
		halrf_wreg(rf, 0x803c, MASKDWORD, 0x20000000);
	}

	//RF_DBG(rf, DBG_RF_IQK, "[IQK] TXK 0x8038 = 0x%x\n", halrf_rreg(rf, 0x8038, MASKDWORD));
	//RF_DBG(rf, DBG_RF_IQK, "[IQK] RXK 0x803C = 0x%x\n", halrf_rreg(rf, 0x803c, MASKDWORD));
}

void iqk_afebb_restore_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path)
{
	//RF_DBG(rf, DBG_RF_IQK, "===> %s\n", __func__);
	//10_ABLite_restore_AFE_BB_settings_20220609
	halrf_wreg(rf, 0x8008, MASKDWORD, 0x0);
	halrf_wreg(rf, 0x8108, MASKDWORD, 0x00408083);
	halrf_wreg(rf, 0x80b8, MASKDWORD, 0x0);
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x0);
	halrf_wreg(rf, 0x8120, MASKDWORD, 0x0);
	halrf_wreg(rf, 0x8138, MASKDWORD, 0x0);
	halrf_wreg(rf, 0x20fc, 0x00010000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x00100000, 0x0);
	halrf_wreg(rf, 0x20fc, 0x01000000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x10000000, 0x0);
	halrf_wreg(rf, 0x5670, MASKDWORD, 0x0);
	halrf_wreg(rf, 0x12a0, 0x000FF000, 0x0);
	halrf_wreg(rf, 0x20fc, 0x00010000, 0x0);
	halrf_wreg(rf, 0x20fc, 0x01000000, 0x0);
}

void iqk_preset_8730e(struct rf_info *rf, u8 path)
{
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//02_IQK_Preset_20220519

	halrf_wrf(rf, path, 0x5, 0x00001, 0x0);
	//halrf_wrf(rf, path, 0x10005, 0x00001, 0x0);
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00000008);
	halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080);
	halrf_wreg(rf, 0x8138, MASKDWORD, 0x000f0000);
}

void iqk_macbb_setting_8730e(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path)
{
	//RF_DBG(rf, DBG_RF_IQK, "[IQK]===> %s\n", __func__);
	halrf_wreg(rf, 0x5864, 0x18000000, 0x3);
	halrf_wreg(rf, 0x2008, 0x01FFFFFF, 0xfffff);
	halrf_wreg(rf, 0x20fc, 0x00010000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x00100000, 0x0);
	halrf_wreg(rf, 0x20fc, 0x01000000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x10000000, 0x0);
	halrf_wreg(rf, 0x5670, 0xFFFFFFFF, 0xf801fffd);
	halrf_wreg(rf, 0x12a0, 0x00008000, 0x1);
	halrf_wreg(rf, 0x5670, 0x80000000, 0x1);
	halrf_wreg(rf, 0x12a0, 0x00007000, 0x7);
	halrf_wreg(rf, 0x12a0, 0x00080000, 0x1);
	halrf_wreg(rf, 0x12a0, 0x00070000, 0x6);
	halrf_wreg(rf, 0x5670, 0x60000000, 0x1);
	halrf_wreg(rf, 0x30c, 0xFF000000, 0xf);
	halrf_wreg(rf, 0x32c, 0xFFFF0000, 0x1);
	halrf_wreg(rf, 0x12b8, 0x10000000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x00100000, 0x1);
	halrf_wreg(rf, 0x20fc, 0x10000000, 0x1);
}

#if 0
void halrf_iqk_track_8730e(
	struct rf_info *rf)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	u8 path = 0x0, i = 0x0;
	u8 cur_ther = 0x0, ther_avg_cnt = 0;
	u32 ther_avg[2] = {0};

	/*only check path 0 */
	for (path = 0; path < 1; path++) {
		cur_ther = halrf_get_thermal_8730e(rf, path);
		iqk_info->ther_avg[path][iqk_info->ther_avg_idx] = cur_ther;
		/*Average times */
		ther_avg_cnt = 0;
		for (i = 0; i < 0x4; i++) {
			if (iqk_info->ther_avg[path][i]) {
				ther_avg[path] += iqk_info->ther_avg[path][i];
				ther_avg_cnt++;
#if 0
				RF_DBG(rf, DBG_RF_IQK,
				       "[IQK] thermal avg[%d] = %d\n", i,
				       iqk_info->ther_avg[path][i]);
#endif
			}
		}
		/*Calculate Average ThermalValue after average enough times*/
		if (ther_avg_cnt) {
			cur_ther = (u8)(ther_avg[path] / ther_avg_cnt);
		}
		if (HALRF_ABS(cur_ther, iqk_info->thermal[path]) > IQK_THR_ReK) {
			iqk_info->thermal_rek_en = true;
		} else {
			iqk_info->thermal_rek_en = false;
		}
		RF_DBG(rf, DBG_RF_IQK, "[IQK]S%x, iqk_ther =%d, ther_now = %d\n", path, iqk_info->thermal[path], cur_ther);
	}
	iqk_info->ther_avg_idx++;
	if (iqk_info->ther_avg_idx == 0x4) {
		iqk_info->ther_avg_idx = 0;
	}
}
#endif

bool halrf_iqk_get_ther_rek_8730e(struct rf_info *rf)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	return iqk_info->thermal_rek_en;
}

u32 halrf_get_iqk_ver_8730e(void)
{
	return iqk_version_8730e;
}

void iqk_init_8730e(struct rf_info *rf)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;

	if (!iqk_info->is_iqk_init) {
		//RF_DBG(rf, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
		iqk_info->is_iqk_init = true;
		iqk_info->thermal_rek_en = false;
		iqk_info->iqk_times = 0x0;
		/*channel/path/TRX(TX:0, RX:1) */
		iqk_info->is_fw_iqk = false;
		/*idx/path*/
		iqk_info->lok_cor_fail[0][0] = false;
		iqk_info->lok_fin_fail[0][0] = false;
		iqk_info->iqk_tx_fail[0][0] = false;
		iqk_info->iqk_rx_fail[0][0] = false;
	}
}

#endif


