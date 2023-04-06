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

/*8730E DPK ver:0x1 20220902*/

void _dpk_bkup_kip_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][DPK_KIP_REG_NUM_8730E])
{
	u8 i;

	for (i = 0; i < DPK_KIP_REG_NUM_8730E; i++) {
		reg_bkup[0][i] = halrf_rreg(rf, reg[i], MASKDWORD);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup 0x%x = %x\n", reg[i], reg_bkup[0][i]);*/
	}
}

void _dpk_bkup_bb_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[DPK_BB_REG_NUM_8730E])
{
	u8 i;

	for (i = 0; i < DPK_BB_REG_NUM_8730E; i++) {
		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup 0x%x = %x\n", reg[i], reg_bkup[i]);*/
	}
}

void _dpk_bkup_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][DPK_RF_REG_NUM_8730E],
	u8 path)
{
	u8 i;

	for (i = 0; i < DPK_RF_REG_NUM_8730E; i++) {
		rf_bkup[path][i] = halrf_rrf(rf, path, rf_reg[i], MASKRF);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);*/
	}
}

void _dpk_reload_kip_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][DPK_KIP_REG_NUM_8730E])
{
	u8 i;

	for (i = 0; i < DPK_KIP_REG_NUM_8730E; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_bkup[0][i]);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload 0x%x = %x\n", reg[i],
				   reg_bkup[0][i]);*/
	}
}

void _dpk_reload_bb_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[DPK_BB_REG_NUM_8730E])
{
	u8 i;

	for (i = 0; i < DPK_BB_REG_NUM_8730E; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_bkup[i]);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload 0x%x = %x\n", reg[i],
				   reg_bkup[i]);*/
	}
}

void _dpk_reload_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][DPK_RF_REG_NUM_8730E],
	u8 path)
{
	u8 i;

	for (i = 0; i < DPK_RF_REG_NUM_8730E; i++) {
		halrf_wrf(rf, path, rf_reg[i], MASKRF, rf_bkup[path][i]);
		/*if (DPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);*/
	}
}

bool _dpk_check_cal_8730e(struct rf_info *rf, u8 ktype)
{
	bool notready = true, fail = true;
	u16 delay_count = 0;

	//RF_DBG(rf, DBG_RF_DPK, "[DPK]===>%s\n", __func__);
	while (notready) {
		if (halrf_rreg(rf, 0xbff8, MASKBYTE0) == 0x55) {
			halrf_delay_ms(rf, 1);
			if (halrf_rreg(rf, 0x8010, MASKBYTE0) == 0x55) {
				notready = false;
			}
		} else {
			halrf_delay_us(rf, 10);
			delay_count++;
		}
		if (delay_count > 8200) {
			fail = true;
			RF_DBG(rf, DBG_RF_DPK, "[DPK]DPK oneshot timeout!!!\n");
			halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x32);
			RF_DBG(rf, DBG_RF_DPK, "[DPK][NCTL] 0x80fc= 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
			break;
		}
	}

	if (!notready) {
		fail = (bool)halrf_rreg(rf, 0x8008, BIT(26));
	}

	RF_DBG(rf, DBG_RF_DPK, "[DPK] cnt= %d\n", delay_count);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] Ktype= %x (%s)\n", ktype, fail ? "Fail" : "Done");

	//halrf_delay_ms(rf, 1);
	//halrf_wreg(rf, 0x8010, MASKBYTE0, 0x0);

	return fail;
}

u8 _dpk_one_shot_8730e(
	struct rf_info *rf,
	enum dpk_id id)
{
	bool fail = 0;
	u16 dpk_cmd = 0x0;

	//halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_START);
	if (halrf_rreg(rf, 0xbff8, MASKBYTE0) != 0x0) {
		halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);
		halrf_delay_us(rf, 100);
	}

	dpk_cmd = (u16)((id << 8) | 0x18);

	if (id == DPK_NCTL_RXAGC_8730E) {
		// ==== open DRFC to ARFC SW/SI ==== //
		halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
		halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en
	}

	halrf_wreg(rf, 0x8000, MASKDWORD, dpk_cmd);
	halrf_delay_ms(rf, 1);
	halrf_wreg(rf, 0x8000, MASKDWORD, dpk_cmd + 1);
	halrf_delay_ms(rf, 1);
	halrf_delay_ms(rf, 1);

	fail = _dpk_check_cal_8730e(rf, id);

	if (id == DPK_NCTL_RXAGC_8730E) {
		// ==== close DRFC to ARFC SW/SI ==== //
		halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
		halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	}

	//halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_STOP);

	return fail;
}

void _dpk_information_8730e(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 kidx = dpk->cur_idx[path];

	dpk->bp[path][kidx].band = rf->hal_com->band[0].cur_chandef.band;
	dpk->bp[path][kidx].ch = rf->hal_com->band[0].cur_chandef.center_ch;
	dpk->bp[path][kidx].bw = rf->hal_com->band[0].cur_chandef.bw;
#ifdef RFDBG_TRACE_EN
	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d]: TSSI %s/ DBCC %s/ %s/ CH%d/ %s\n",
	       path, dpk->cur_idx[path], rf->is_tssi_mode[path] ? "on" : "off",
	       rf->hal_com->dbcc_en ? "on" : "off",
	       dpk->bp[path][kidx].band == 0 ? "2G" : (dpk->bp[path][kidx].band == 1 ? "5G" : "6G"),
	       dpk->bp[path][kidx].ch,
	       dpk->bp[path][kidx].bw == 0 ? "20M" : (dpk->bp[path][kidx].bw == 1 ? "40M" : "80M"));
#endif
}

void _dpk_bb_afe_setting_8730e(struct rf_info *rf)
{
	//01_ABLite_AFE_ON_BB_settings_20220519
	halrf_wreg(rf, 0x5864, BIT(28) | BIT(27), 0x3);
	halrf_wreg(rf, 0x2008, 0x01FFFFFF, 0x00ffffff);
	halrf_wreg(rf, 0x20fc, BIT(16), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28), 0x0);
	halrf_wreg(rf, 0x5670, MASKDWORD, 0xf801fffd);
	halrf_wreg(rf, 0x12a0, BIT(15), 0x1);
	halrf_wreg(rf, 0x5670, BIT(31), 0x1);
	halrf_wreg(rf, 0x12a0, 0x00007000, 0x7);
	halrf_wreg(rf, 0x12a0, BIT(19), 0x1);
	halrf_wreg(rf, 0x12a0, 0x00070000, 0x6);
	halrf_wreg(rf, 0x5670, BIT(30) | BIT(29), 0x1);
	halrf_wreg(rf, 0x030c, 0xFF000000, 0x0f);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0001);
	halrf_wreg(rf, 0x12b8, BIT(28), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28), 0x1);
}

void _dpk_bb_afe_restore_8730e(struct rf_info *rf)
{
	//10_ABLite_restore_AFE_BB_settings_20220824
	halrf_wreg(rf, 0x8108, MASKDWORD, 0x00408083);
	//halrf_wreg(rf, 0x80b8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x80cc, 0x0000003F, 0x0);
	halrf_wreg(rf, 0x8120, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x8138, MASKDWORD, 0x00000000);
	//halrf_wreg(rf, 0x8030, MASKDWORD, 0x0100FF00);
	halrf_wreg(rf, 0x20fc, BIT(16), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28), 0x0);
	halrf_wreg(rf, 0x5670, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x12a0, 0x000FF000, 0x0);
	//halrf_wreg(rf, 0x12b8, BIT(28), 0x0); don't set0 for tssi
	halrf_wreg(rf, 0x20fc, BIT(16), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24), 0x0);
}

void _dpk_tssi_pause_8730e(
	struct rf_info *rf,
	bool is_pause)
{
	halrf_wreg(rf, 0x5818, BIT(30), is_pause);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] TSSI %s\n",
	       is_pause ? "pause" : "resume");

}

void _dpk_kip_pwr_clk_on_8730e(struct rf_info *rf)
{
	/*cip power on*/
	halrf_wreg(rf, 0x8020, MASKDWORD, 0x030403E0);
	halrf_wreg(rf, 0x8000, BIT(3), 0x1);
	halrf_wreg(rf, 0x8008, BIT(7), 0x1);
}

void _dpk_kip_preset_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x803c, MASKDWORD, 0x20000000);
	halrf_wreg(rf, 0x80b8, MASKDWORD, 0x00000000);

	// force iq skew FDF placed in Tx
	halrf_wreg(rf, 0x8120, 0xc0000000, 0x3);

	// ==== txagc_boundary setting ==== //
	halrf_wreg(rf, 0x80bc, 0x00000FFF, 0x7ff); //1F,3F
	halrf_wreg(rf, 0x81bc, 0x0000003F, 0x00);

	//copy RFC 0x79 to 180ec[19:0]
	halrf_wreg(rf, 0x80ec, MASKRF, halrf_rrf(rf, RF_PATH_A, 0x79, MASKRF));

	//TPG_20M selected
	halrf_wreg(rf, 0x80f8, MASKDWORD, 0xd20000C2);

	//[23:16]timingSync_thr
	//halrf_wreg(rf, 0x8088, MASKDWORD, 0x00FF8A00);

}

void _dpk_kip_restore_8730e(struct rf_info *rf)
{
	//07_dpd_restore_20220628
	halrf_wreg(rf, 0x8120, 0xc0000000, 0x0);
	//halrf_wreg(rf, 0x8000, BIT(3), 0x0); //do not set for iqk oneshot
	halrf_wreg(rf, 0x8008, BIT(7), 0x0);
	halrf_wreg(rf, 0x80f8, MASKDWORD, 0x00000000);
	//RF_DBG(rf, DBG_RF_DPK, "[DPK]  restore KIP\n");
}

void _dpk_kip_set_txagc_8730e(
	struct rf_info *rf,
	enum rf_path path,
	u8 txagc)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	//SW_SI 0x4[24]=0x1
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x1); //wire r_iqk_IO_RFC_en

	halrf_drf_direct_cntrl_8730e(rf, false); /*switch control to direct write*/
	halrf_wrf(rf, path, 0x10001, MASKRF, txagc);
	halrf_wreg(rf, 0x800c, BIT(11) | BIT(10), 0x2);
	halrf_wreg(rf, 0x8030, BIT(26) | BIT(25), 0x1);
	halrf_wreg(rf, 0x8034, 0xFF000000, 0x57);
	halrf_wreg(rf, 0x8014, BIT(28), 0x1);
	halrf_wreg(rf, 0x8014, BIT(28), 0x0);

	// set TxAGC_RF_k for DPD
	halrf_wreg(rf, 0x81c4, 0x0001F800, txagc);
	halrf_wreg(rf, 0x81c8, 0x0001F800, txagc);
	dpk->bp[path][0].txagc_dpk = txagc;

	//SW_SI 0x4[24]=0x0
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	halrf_wreg(rf, 0x5670, BIT(1), 0x0); //wire r_iqk_IO_RFC_en

	RF_DBG(rf, DBG_RF_DPK, "[DPK] set TXAGC = 0x%x\n", halrf_rrf(rf, path, 0x10001, MASKRF));
}

void _dpk_get_thermal_8730e(struct rf_info *rf, u8 kidx, enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	dpk->bp[path][kidx].ther_dpk = halrf_get_thermal_8730e(rf, path);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] thermal@DPK (by RFC)= 0x%x\n", dpk->bp[path][kidx].ther_dpk);

}


void _dpk_lut_sram_write_8730e(struct rf_info *rf)
{
	u32 small_pwr;
	u16 reg;

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);

	// read LUT1
	//small_pwr = halrf_rreg(rf, 0x99a0, MASKDWORD); //point 9
	small_pwr = halrf_rreg(rf, 0x998c, MASKDWORD); //point 4

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);

	// write LUT1
	for (reg = 0x9980; reg < 0x998c ; reg += 4) {
		halrf_wreg(rf, reg, MASKDWORD, small_pwr);
	}

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
}

void _dpk_lut_sram_clear_8730e(struct rf_info *rf)
{
	u16 reg;

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);
#if 0
	// write LUT0
	for (reg = 0x9900; reg < 0x9980 ; reg += 4) {
		halrf_wreg(rf, reg, MASKDWORD, 0x02000000);
	}

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);
#endif
	// write LUT1
	for (reg = 0x9980; reg < 0x9a00 ; reg += 4) {
		halrf_wreg(rf, reg, MASKDWORD, 0x02000000);
	}
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
}

void _dpk_lut_sram_read_8730e(
	struct rf_info *rf)
{
#ifdef RFDBG_TRACE_EN
	u16 reg;

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);
#if 0
	// read LUT0
	for (reg = 0x9900; reg < 0x9980 ; reg += 4) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK][LUT] 0x%04x = 0x%07x\n", reg,
		       halrf_rreg(rf, reg, MASKDWORD));
	}
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00020000);
#endif
	// read LUT1
	for (reg = 0x9980; reg < 0x9a00 ; reg += 4) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK][LUT] 0x%04x = 0x%07x\n", reg,
		       halrf_rreg(rf, reg, MASKDWORD));
	}
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
#endif
}

void _dpk_rf_setting_8730e(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	// DPK Feedback path setting
	if (dpk->bp[path][0].band == BAND_ON_24G) { /*2G*/
		halrf_wrf(rf, path, 0x00, MASKRF, 0x503E0);
		/*att*/
		halrf_wrf(rf, path, 0x83, 0x000F0, 0x2); //-25db
		/*PGA2 */
		halrf_wrf(rf, path, 0x79, BIT(12), 0x1);
		halrf_wrf(rf, path, 0x79, 0x0E000, 0x3);
	} else { /*5G*/
		halrf_wrf(rf, path, 0x00, MASKRF, 0x503E0);
		/*att*/
		halrf_wrf(rf, path, 0x8c, 0x0E000, 0x1); //-28db
		/*PGA2 */
		halrf_wrf(rf, path, 0x79, BIT(12), 0x1);
		halrf_wrf(rf, path, 0x79, 0x0E000, 0x3); //16db
	}
	RF_DBG(rf, DBG_RF_DPK, "[DPK] ARF 0x0/0x79/0x83 = 0x%x/ 0x%x/ 0x%x\n",
	       halrf_rrf(rf, path, 0x00, MASKRF),
	       halrf_rrf(rf, path, 0x79, MASKRF),
	       halrf_rrf(rf, path, 0x83, MASKRF));
}

u32 _dpk_dbg_report_8730e(struct rf_info *rf, u8 index, bool sel)
{
	u32 rpt;

	halrf_wreg(rf, 0x80d4, MASKDWORD, index << 16 | sel);
	rpt = halrf_rreg(rf, 0x80fc, MASKDWORD);
	RF_DBG(rf, DBG_RF_DPK, "[DPK][DBG] idx 0x%x = 0x%x\n", index, rpt);

	return rpt;
}

u8 _dpk_gainloss_8730e(
	struct rf_info *rf)
{
	u32 rpt;
	u8 result;
	bool gl_fail;

	if (_dpk_one_shot_8730e(rf, DPK_NCTL_TXAGC_8730E)) {
		return 0xff;
	}

	rpt = _dpk_dbg_report_8730e(rf, 0xa, 0);
	// read Gain_Loss new fail [16]
	gl_fail = (rpt & BIT(16)) >> 16;
	result = (rpt & 0x1F00) >> 8; //[12:8]
	if (gl_fail) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] GL< 2db fail, tmp GL = 0x%x\n", result);
	} else {
		result = 0xa - result;
		RF_DBG(rf, DBG_RF_DPK, "[DPK] tmp GL = %d\n", result);
	}
	return result;
}

u8 _dpk_set_offset_8730e(
	struct rf_info *rf,
	enum rf_path path,
	s8 gain_offset)
{
	u8 txagc;

	txagc = (u8)halrf_rrf(rf, path, 0x10001, MASKRF);

	if (txagc - gain_offset < 0x2e) {
		txagc = 0x2e;
	} else if (txagc - gain_offset > 0x3f) {
		txagc = 0x3f;
	} else {
		txagc = txagc - gain_offset;
	}

	_dpk_kip_set_txagc_8730e(rf, path, txagc);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] Adjust TxAGC (offset %d) = 0x%x\n", gain_offset, txagc);
	return txagc;
}

u8 _dpk_pas_read_8730e(
	struct rf_info *rf,
	u8 is_check)
{
#ifdef RFDBG_TRACE_EN
	u8 i, j;
	u32 val1_i = 0, val1_q = 0, val2_i = 0, val2_q = 0;

	if (is_check) {
		halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06);
		halrf_wreg(rf, 0x8090, 0x00000007, 0x00);
		val1_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val1_i >= 0x800) {
			val1_i = 0x1000 - val1_i;
		}
		val1_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val1_q >= 0x800) {
			val1_q = 0x1000 - val1_q;
		}

		halrf_wreg(rf, 0x8090, 0x00000007, 0x07);
		val2_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val2_i >= 0x800) {
			val2_i = 0x1000 - val2_i;
		}
		val2_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val2_q >= 0x800) {
			val2_q = 0x1000 - val2_q;
		}

		RF_DBG(rf, DBG_RF_DPK, "[DPK] PAS_delta = 0x%x\n",
		       (val1_i * val1_i + val1_q * val1_q) /
		       (val2_i * val2_i + val2_q * val2_q));
	} else {
		for (i = 0; i < 8; i++) {
			halrf_wreg(rf, 0x8090, 0x00000007, i);
			for (j = 0; j < 4; j++) {
				halrf_wreg(rf, 0x80d4, MASKDWORD, (0x0006 + j) << 16);
				RF_DBG(rf, DBG_RF_DPK, "[DPK] PAS_Read[%02d]= 0x%08x\n",
				       4 * i + j, halrf_rreg(rf, 0x80fc, MASKDWORD));
			}
		}
	}

	if ((val1_i * val1_i + val1_q * val1_q) >= ((val2_i * val2_i + val2_q * val2_q) * 8 / 5)) {
		return 1;
	} else
#endif
		return 0;
}

u8 _dpk_agc_8730e(
	struct rf_info *rf,
	enum rf_path path,
	u8 init_txagc)
{
	u8 tmp_txagc, i = 0, tmp_gl_idx = 0;
	u8 goout = 0, agc_cnt = 0;
	u8 tmp_pga2;

	tmp_txagc = init_txagc;

	do {
		switch (i) {
		case 0: /*reset PGA2*/
		case 1:
		case 2:
			/*reset PGA2*/
			halrf_wrf(rf, path, 0x79, 0x0E000, 0x3);
			/*Gain adjustment*/
			// ====== Ramp PAScan pattern select ====== //
			halrf_wreg(rf, 0x80c8, BIT(19) | BIT(18), 0x1);
			if (_dpk_one_shot_8730e(rf, DPK_NCTL_RXAGC_8730E)) {
				agc_cnt += 2;
				break;
			}
			tmp_pga2 = (_dpk_dbg_report_8730e(rf, 0x5, 0) >> 8) & 0x7;
			halrf_wrf(rf, path, 0x79, 0x0E000, tmp_pga2);
			/*GAIN_LOSS and idx*/
			tmp_gl_idx = _dpk_gainloss_8730e(rf);
			//_dpk_pas_read_8730e(rf, false);

			if (tmp_gl_idx >= 4 && tmp_gl_idx <= 0xa) {
				i = 3;        /*GL > criterion*/
			} else if (tmp_gl_idx == 0x1f) {
				i = 4;        /*GL < criterion*/
			} else if (tmp_gl_idx == 0x0 || tmp_gl_idx == 0x1) {
				goout = 1;        /*GL = 2db*/
			} else if (tmp_gl_idx == 0xff) {
				i = 6;        /*GL fail retry*/
			} else {
				i = 5;
			}
			break;

		case 3: /*GL > criterion*/
			if (tmp_txagc == 0x2e) {
				goout = 1;
				//RF_DBG(rf, DBG_RF_DPK, "[DPK] Txagc@lower bound!!\n");
			} else {
				tmp_txagc = _dpk_set_offset_8730e(rf, path, 0x3); /*tx gain -3*/
			}
			i = 0;
			agc_cnt++;
			break;

		case 4:	/*GL < criterion*/
			if (tmp_txagc == 0x3f) {
				goout = 1;
				//RF_DBG(rf, DBG_RF_DPK, "[DPK] Txagc@upper bound!!\n");
			} else {
				tmp_txagc = _dpk_set_offset_8730e(rf, path, 0xff); /*tx gain +1*/
			}
			i = 0;
			agc_cnt++;
			break;

		case 5:	/*set tx gain for DPK*/
			tmp_txagc = _dpk_set_offset_8730e(rf, path, tmp_gl_idx);
			i = 0;
			agc_cnt++;
			break;

		case 6: /*GL fail retry*/
			tmp_txagc = 0xff;
			i = 0;
			agc_cnt++;
			break;
		default:
			goout = 1;
			break;
		}
	} while (!goout && (agc_cnt < 6));
	RF_DBG(rf, DBG_RF_DPK, "[DPK] Txagc / agc_cnt for DPK = 0x%x / 0x%x\n",
	       tmp_txagc, agc_cnt);
	return tmp_txagc;
}

bool _dpk_nctl_learning_8730e(
	struct rf_info *rf,
	u8 txagc)
{
	bool result = 0;
	s8 txagc_ofst;

	/*05-NCTL_DPD_Learning*/
	//0x1808c[5]=1, 0x1808c[6]=0/1==>LUT0/LUT1
	//halrf_wreg(rf, 0x808c, BIT(5), 0x1);
	//halrf_wreg(rf, 0x808c, BIT(6), 0x1);

	//txagc[5:0] = txagc_dbg ? txagc_man : txagc_bb
	halrf_wreg(rf, 0x813c, BIT(31), 0x1);
	halrf_wreg(rf, 0x813c, 0x3F000000, txagc);
	RF_DBG(rf, DBG_RF_DPK, "[DPK][DBG] 0x813c = 0x%x\n", halrf_rreg(rf, 0x813c, MASKDWORD));
	/*LMS only one-shot*/
	result = _dpk_one_shot_8730e(rf, DPK_NCTL_LMS_8730E);
	_dpk_lut_sram_write_8730e(rf); //for small pwr WA
	_dpk_pas_read_8730e(rf, false);
	_dpk_lut_sram_read_8730e(rf);
	result |= (_dpk_dbg_report_8730e(rf, 0x0a, 0) & BIT(1)) >> 1;
	halrf_wreg(rf, 0x81d4, MASKDWORD, 0x00130000);
	RF_DBG(rf, DBG_RF_DPK, "[DPK][DBG] auto_gs = 0x%x\n", halrf_rreg(rf, 0x81fc, MASKDWORD));

	//06_DPD_on_20220628
	if (rf->is_tssi_mode[0]) {
		// ============ TSSI mode on/off ============ //
		halrf_wreg(rf, 0x80b8, 0x00000070, 0x1); //TSSI on
		// set TxAGC_BB_k for TSSI
		halrf_wreg(rf, 0x81c4, 0x3FF00000, 0x3A0);
		halrf_wreg(rf, 0x81c8, 0x3FF00000, 0x3A0);
		halrf_wreg(rf, 0x81d4, 0x003F0000, 0x10);
		txagc_ofst = (s8)halrf_rreg(rf, 0x81fc, 0x0001FE00) & 0xff;
	} else {
		halrf_wreg(rf, 0x80b8, 0x00000070, 0x0); //TSSI off
		txagc_ofst = (s8)halrf_rreg(rf, 0x5818, MASKBYTE0) & 0xff;
	}
	// pwsf shift
	//0xd8[29:21]: pwsf_offset s(9,3)
	halrf_wreg(rf, 0x80bc, 0xFFF00000, 0x200);
	halrf_wreg(rf, 0x80d8, 0x3FE00000, 0x4F + txagc_ofst);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] txagc_ofst= 0x%x, pwsf_offset = 0x%x\n", txagc_ofst, halrf_rreg(rf, 0x80d8, MASKDWORD));

	return result;
}

bool _dpk_main_8730e(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 txagc = 0x39, kidx = dpk->cur_idx[path];
	bool is_fail = false;
	RF_DBG(rf, DBG_RF_DPK, "[DPK] ========= S%d[%d] DPK Start =========\n", path, kidx);
	_dpk_kip_pwr_clk_on_8730e(rf);

	_dpk_lut_sram_clear_8730e(rf);

	halrf_rf_direct_cntrl_8730e(rf, false); /*switch control to direct write*/

	_dpk_rf_setting_8730e(rf, path);

	_dpk_kip_set_txagc_8730e(rf, path, txagc);

	_dpk_kip_preset_8730e(rf);

	txagc = _dpk_agc_8730e(rf, path, txagc);

	if (txagc == 0xff) {
		is_fail = true;
		goto _error;
	}

	_dpk_pas_read_8730e(rf, false);
	_dpk_get_thermal_8730e(rf, kidx, path);

	is_fail = _dpk_nctl_learning_8730e(rf, txagc);

	halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);

_error:

	if (is_fail == false) {
		dpk->bp[path][kidx].path_ok = 1;
	} else {
		dpk->bp[path][kidx].path_ok = 0;
	}

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d] DPK %s\n", path, kidx, is_fail ? "Check" : "Success");
	return is_fail;

}

void _dpk_cal_select_8730e(
	struct rf_info *rf,
	bool force,
	u8 path)
{

	u32 kip_bkup[DPK_RF_PATH_MAX_8730E][DPK_KIP_REG_NUM_8730E] = {{0}};
	u32 bb_bkup[DPK_BB_REG_NUM_8730E] = {0};
	u32 rf_bkup[DPK_RF_PATH_MAX_8730E][DPK_RF_REG_NUM_8730E] = {{0}};

	u32 kip_reg[] = {0x8030, 0x8034, 0x819c, 0x81a0, 0x813c};
	u32 bb_reg[] = {0x5670, 0x12a0, 0x12b8, 0x2008, 0x030c, 0x032c};
	u32 rf_reg[DPK_RF_REG_NUM_8730E] = {0x0, 0x83, 0x79, 0x5, 0x10005};

	bool is_fail = true;

	_dpk_bkup_bb_8730e(rf, bb_reg, bb_bkup);

	_dpk_bkup_kip_8730e(rf, kip_reg, kip_bkup);
	_dpk_bkup_rf_8730e(rf, rf_reg, rf_bkup, path);
	_dpk_information_8730e(rf, path);
	if (rf->is_tssi_mode[path]) {
		_dpk_tssi_pause_8730e(rf, true);
	}

	_dpk_bb_afe_setting_8730e(rf);

	is_fail = _dpk_main_8730e(rf, path);
	halrf_dpk_onoff_8730e(rf, path, is_fail);

	_dpk_bb_afe_restore_8730e(rf);
	_dpk_reload_bb_8730e(rf, bb_reg, bb_bkup);

	_dpk_kip_restore_8730e(rf);
	_dpk_reload_kip_8730e(rf, kip_reg, kip_bkup);
	_dpk_reload_rf_8730e(rf, rf_reg, rf_bkup, path);
	if (rf->is_tssi_mode[path]) {
		_dpk_tssi_pause_8730e(rf, false);
	}
}

bool _dpk_bypass_check_8730e(
	struct rf_info *rf)
{
	struct halrf_fem_info *fem = &rf->fem;
	bool result;

	if (fem->epa_2g) {
		/*RF_DBG(rf, DBG_RF_DPK, "[DPK] Skip DPK due to 2G_ext_PA exist!!\n");*/
		result = 1;
	} else {
		result = 0;
	}

	return result;
}

void _dpk_force_bypass_8730e(
	struct rf_info *rf)
{
	u8 path;

	for (path = 0; path < DPK_RF_PATH_MAX_8730E; path++) {
		halrf_dpk_onoff_8730e(rf, path, true);
	}
}

void halrf_dpk_8730e(
	struct rf_info *rf,
	bool force)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	RF_DBG(rf, DBG_RF_DPK, "[DPK] ****** DPK Start (Ver: 0x%x, Cv: %d, RF_para: %d) ******\n",
	       DPK_VER_8730E, rf->hal_com->cv, RF_RELEASE_VERSION_8730E);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] Driver mode = %d\n", rf->phl_com->drv_mode);
	if (_dpk_bypass_check_8730e(rf) || (!dpk->is_dpk_enable)) {
		_dpk_force_bypass_8730e(rf);
	} else {
		_dpk_cal_select_8730e(rf, force, RF_PATH_A);
	}

}

void halrf_dpk_onoff_8730e(
	struct rf_info *rf,
	enum rf_path path,
	bool off)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	//bool off_reverse;
	u8 val, kidx = dpk->cur_idx[path];

	val = dpk->is_dpk_enable & !off & dpk->bp[path][kidx].path_ok;

	if (val) {
		halrf_wreg(rf, 0x80bc, BIT(29) | BIT(28), 2);
	} else {
		halrf_wreg(rf, 0x80bc, BIT(29) | BIT(28), 0);
		halrf_wreg(rf, 0x8020, BIT(25), 1);	//bypass_dpd
		halrf_wreg(rf, 0x8020, BIT(24), 0);	//dpd_off
	}
	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d] DPK %s !!!\n", path, kidx,
	       val ? "enable" : "disable");
}

void halrf_dpk_track_8730e(
	struct rf_info *rf)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 path, i, kidx = 0;
	u8 trk_idx = 0, txagc_rf = 0;
	s8 txagc_bb = 0, txagc_bb_tp = 0, ini_diff = 0, txagc_ofst = 0;
	u8 cur_ther, ther_avg_cnt = 0;
	u32 ther_avg[2] = {0};
	s8 delta_ther[2] = {0};

	for (path = 0; path < DPK_RF_PATH_MAX_8730E; path++) {

		if (dpk->bp[path][kidx].ther_dpk == 0) {
			return;
		}

		kidx = dpk->cur_idx[path];
		cur_ther = rf->cur_ther_s0;
		RF_DBG(rf, DBG_RF_DPK_TRACK,
		       "[DPK_TRK] ================[S%d[%d] (CH %d)]================\n",
		       path, kidx, dpk->bp[path][kidx].ch);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] thermal now = %d\n", cur_ther);

		dpk->ther_avg[path][dpk->ther_avg_idx] = cur_ther;

		/*Average times */
		ther_avg_cnt = 0;
		for (i = 0; i < THERMAL_DPK_AVG_NUM; i++) {
			if (dpk->ther_avg[path][i]) {
				ther_avg[path] += dpk->ther_avg[path][i];
				ther_avg_cnt++;
#if 0
				RF_DBG(rf, DBG_RF_DPK_TRACK,
				       "[DPK_TRK] thermal avg[%d] = %d\n", i,
				       dpk->thermal_dpk_avg[path][i]);
#endif
			}
		}

		/*Calculate Average ThermalValue after average enough times*/
		if (ther_avg_cnt) {
			cur_ther = (u8)(ther_avg[path] / ther_avg_cnt);
#if 0
			RF_DBG(rf, DBG_RF_DPK_TRACK,
			       "[DPK_TRK] thermal avg total = %d, avg_cnt = %d\n",
			       ther_avg[path], ther_avg_cnt);
#endif
			RF_DBG(rf, DBG_RF_DPK_TRACK,
			       "[DPK_TRK] thermal avg = %d (DPK @ %d)\n",
			       cur_ther, dpk->bp[path][kidx].ther_dpk);
		}

		if (dpk->bp[path][kidx].ch != 0 && cur_ther != 0) {
			delta_ther[path] = dpk->bp[path][kidx].ther_dpk - cur_ther;
		}

		if (dpk->bp[path][kidx].band == 0) { /*2G*/
			delta_ther[path] = delta_ther[path] * 3 / 5;        //T +0.65:-0.125 (Pout)
		} else {
			delta_ther[path] = delta_ther[path] * 3 / 4;        //T/0.125=0.7
		}

		txagc_rf = (u8)halrf_rreg(rf, 0x1c60 + (path << 13), 0x0000003f); /*[5:0]*/

		txagc_ofst = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), MASKBYTE3); /*[31:24]*/
#ifdef RFDBG_TRACE_EN
		trk_idx = (u8)halrf_rrf(rf, path, 0x1005D, 0xFC000); /*[19:14] for integer*/
		txagc_bb = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), MASKBYTE2); /*[23:16]*/
		txagc_bb_tp = (u8)halrf_rreg(rf, 0x1c04 + (path << 13), 0x00000007); /*[2:0]*/
		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_RF / track_idx = 0x%x / %d\n",
		       txagc_rf, trk_idx);
		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_bb_tp / txagc_bb = 0x%x / 0x%x\n",
		       txagc_bb_tp, txagc_bb);
		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_offset / delta_ther = %d / %d\n", txagc_ofst, delta_ther[path]);
#endif

		if (txagc_rf != 0 && cur_ther != 0) {
			ini_diff = txagc_ofst + delta_ther[path];
		}

		if (rf->rfk_is_processing != true && halrf_rreg(rf, 0x80f0, BIT(31)) == 0x0 && txagc_rf != 0) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] New pwsf = 0x%x\n", 0x4F + ini_diff);

			halrf_wreg(rf, 0x80d8, 0x3FE00000, 0x4F + ini_diff);
		}
	}
	dpk->ther_avg_idx++;

	if (dpk->ther_avg_idx == THERMAL_DPK_AVG_NUM) {
		dpk->ther_avg_idx = 0;
	}
}

#endif
