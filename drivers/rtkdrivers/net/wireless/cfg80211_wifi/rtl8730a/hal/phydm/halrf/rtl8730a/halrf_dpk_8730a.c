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

/*---------------------------Define Local Constant---------------------------*/

/*8730A DPK ver:0x01 20211117*/

void _dpk_backup_bb_registers_8730a(struct dm_struct *dm,
				    u32 *reg,
				    u32 *reg_backup)
{
	u32 i;

	for (i = 0; i < DPK_BB_REG_NUM_8730A; i++) {
		reg_backup[i] = odm_get_bb_reg(dm, reg[i], MASKDWORD);
#if (DPK_REG_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Backup BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
#endif
	}
}

void _dpk_reload_bb_registers_8730a(struct dm_struct *dm,
				    u32 *reg,
				    u32 *reg_backup)
{
	u32 i;

	for (i = 0; i < DPK_BB_REG_NUM_8730A; i++) {
		odm_set_bb_reg(dm, reg[i], MASKDWORD, reg_backup[i]);
#if (DPK_REG_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Reload BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
#endif
	}
}

void _backup_rf_registers_8730a(struct dm_struct *dm,
				u32 *rf_reg,
				u32 rf_reg_backup[][DPK_RF_PATH_NUM_8730A])
{
	u32 i;

	for (i = 0; i < DPK_RF_REG_NUM_8730A; i++) {
		rf_reg_backup[i][RF_PATH_A] = odm_get_rf_reg(dm, RF_PATH_A, rf_reg[i], RFREG_MASK);
#if (DPK_REG_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Backup RF_A 0x%x = 0x%x\n",
		       rf_reg[i], rf_reg_backup[i][RF_PATH_A]);
#endif
	}
}

void _reload_rf_registers_8730a(struct dm_struct *dm,
				u32 *rf_reg,
				u32 rf_reg_backup[][DPK_RF_PATH_NUM_8730A])
{
	u32 i;

	for (i = 0; i < DPK_RF_REG_NUM_8730A; i++) {
		odm_set_rf_reg(dm, RF_PATH_A, rf_reg[i], RFREG_MASK,
			       rf_reg_backup[i][RF_PATH_A]);
#if (DPK_REG_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Reload RF_A 0x%x = 0x%x\n",
		       rf_reg[i], rf_reg_backup[i][RF_PATH_A]);
#endif
	}

	odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x80000);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x33, RFREG_MASK, 0x00005);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x3e, RFREG_MASK, 0x03c39);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, RFREG_MASK, 0x7ff34);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x00000);
}

void _dpk_information_8730a(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u32  reg_rf18;

	if ((odm_get_bb_reg(dm, R_0x4318, MASKDWORD) >> 28) & 0x1) {
		dpk_info->is_tssi_mode = true;
	} else {
		dpk_info->is_tssi_mode = false;
	}

	reg_rf18 = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, RFREG_MASK);

	dpk_info->dpk_band = (u8)((reg_rf18 & BIT(16)) >> 16); /*0/1:G/A*/
	dpk_info->dpk_ch = (u8)reg_rf18 & 0xff;
	dpk_info->dpk_bw = (u8)((reg_rf18 & BIT(10)) >> 10); /*1/0:20/40*/

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] Drv cut vision = 0x01, update time 20211009\n");

	//RF_DBG(dm, DBG_RF_DPK, "[DPK] RFE TYPE = 0x%x\n", dm->rfe_type);

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] TSSI mode %s , Band/ CH/ BW = %s / %d / %s\n",
	       dpk_info->is_tssi_mode == 1 ? "ON" : "OFF",
	       dpk_info->dpk_band == 0 ? "2G" : "5G",
	       dpk_info->dpk_ch,
	       dpk_info->dpk_bw == 1 ? "20M" : "40M");
}

u8 _dpk_thermal_read_8730a(void *dm_void,	u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_rf_reg(dm, RF_PATH_A, RF_0x42, BIT(19), 0x1);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x42, BIT(19), 0x0);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x42, BIT(19), 0x1);
	ODM_delay_us(15);

	return (u8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x42, 0x0007e);
}

void _dpk_get_tssi_mode_txagc(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8  txagc_rf, txagc_offset, path;
	u16 digital_bbgain, count = 0;
	u8 tx_bw;
	u32 tx_cnt = 0x0, poll_cnt = 0x0;

	//TBD
	//tx_bw = ~dpk_info->dpk_bw & 0x1;

}

void _dpk_tx_pause_8730a(struct dm_struct *dm)
{
	u8 reg_rf0_a;
	u16 count = 0;

	odm_write_1byte(dm, R_0x522, 0xff);
	odm_set_bb_reg(dm, R_0x1e70, 0x0000000f, 0x2); /*hw tx stop*/

	reg_rf0_a = (u8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000);

	while (reg_rf0_a == 2 && count < 2500) {
		reg_rf0_a = (u8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000);
		ODM_delay_us(2);
		count++;
	}

	RF_DBG(dm, DBG_RF_DPK, "[DPK] Tx pause!!\n");
}

void _dpk_afe_setting_8730a(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	if (dpk_info->is_tssi_mode) {
		_dpk_tx_pause_8730a(dm);
		odm_set_bb_reg(dm, R_0x1b20, 0x0F000000, 0x3); // bypass DPD
		_dpk_get_tssi_mode_txagc(dm);
		odm_set_bb_reg(dm, R_0x4384, BIT(30), 0x1);//PAUSE TSSI
	}

	_dpk_tx_pause_8730a(dm);

	/*01_8730A_AFE_ON_BB_settings.txt*/
	odm_set_bb_reg(dm, R_0x1c38, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, R_0x1810, MASKDWORD, 0x10001600);
	odm_set_bb_reg(dm, R_0x1814, MASKDWORD, 0x5bba5020);
	odm_set_bb_reg(dm, R_0x1818, MASKDWORD, 0x0001d429);
	odm_set_bb_reg(dm, R_0x181c, MASKDWORD, 0x00000140);
	odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x0); //force ADDA
	odm_set_bb_reg(dm, R_0x1860, 0xFF000000, 0x0f);
	odm_set_bb_reg(dm, R_0x1860, 0x00FFF000, 0x001);
	//--CLK rate, AD 80M--//
	odm_set_bb_reg(dm, R_0x9f0, 0x0000FFFF, 0x5555);
	odm_set_bb_reg(dm, R_0x1d40, BIT(3), 0x1);
	odm_set_bb_reg(dm, R_0x1d40, 0x00000007, 0x4);
	//--DAC clk rate,DA 160M--//
	odm_set_bb_reg(dm, R_0x9b4, 0x00000700, 0x5);
	odm_set_bb_reg(dm, R_0x9b4, 0x00003800, 0x5);
	odm_set_bb_reg(dm, R_0x9b4, 0x0001C000, 0x5);
	odm_set_bb_reg(dm, R_0x9b4, 0x000E0000, 0x5);
	odm_set_bb_reg(dm, R_0x1c20, BIT(5), 0x1);
	//--BB settings--//
	odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x0); //r_path_en_en
	odm_set_bb_reg(dm, R_0x1e28, 0x0000000F, 0x1);
	odm_set_bb_reg(dm, R_0x824, 0x000F0000, 0x1);
	//IQK clk on
	odm_set_bb_reg(dm, R_0x1cd0, 0xF0000000, 0x7);
	//Block CCA
	odm_set_bb_reg(dm, R_0x2a24, BIT(13), 0x1); //Prevent CCKCCA at sine PSD
	odm_set_bb_reg(dm, R_0x1c68, BIT(24), 0x1); //Prevent OFDM CCA
	//trx gating clk force on
	odm_set_bb_reg(dm, R_0x1864, BIT(31), 0x1);
	odm_set_bb_reg(dm, R_0x180c, BIT(27), 0x1);
	odm_set_bb_reg(dm, R_0x180c, BIT(30), 0x1);
	odm_set_bb_reg(dm, R_0x180c, BIT(31), 0x1);
	//go through iqk
	odm_set_bb_reg(dm, R_0x1e24, BIT(17), 0x1);
	odm_set_bb_reg(dm, R_0x180c, BIT(31), 0x1);
	//wire r_iqk_IO_RFC_en
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0);
	//Release ADDA fifo force off
	odm_set_bb_reg(dm, R_0x1c38, MASKDWORD, 0xffffffff);
	RF_DBG(dm, DBG_RF_DPK, "[DPK]AFE setting for DPK mode!\n");
}

void _dpk_afe_restore_8730a(struct dm_struct *dm)
{
	/*10_8730A_restore_AFE_BB_settings.txt*/
	/*--restore DPK&IQK settings--*/
	//Anapar force mode
	odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x1);
	odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x1);
	odm_set_bb_reg(dm, R_0x2a24, BIT(13), 0x0);
	odm_set_bb_reg(dm, R_0x1c68, BIT(24), 0x0);
	odm_set_bb_reg(dm, R_0x1864, BIT(31), 0x0);
	odm_set_bb_reg(dm, R_0x180c, BIT(27), 0x0);
	odm_set_bb_reg(dm, R_0x180c, BIT(30), 0x0);

	RF_DBG(dm, DBG_RF_DPK, "[DPK]AFE restore for Normal mode!\n");
}

u8 _dpk_rf_setting_8730a(struct dm_struct *dm,	u8 path)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 txagc = 0;
	u32 rf_93, rf_8e;

	odm_set_rf_reg(dm, path, RF_0x5, BIT(0), 0x0);
	odm_set_rf_reg(dm, path, RF_0x00, 0xfffff, 0x503E0);

	if (dpk_info->dpk_band == 0x0) { /*2G*/
		odm_set_rf_reg(dm, path, RF_0xef, RFREG_MASK, 0x80000);
		odm_set_rf_reg(dm, path, RF_0x33, RFREG_MASK, 0x00005);
		odm_set_rf_reg(dm, path, RF_0x3e, RFREG_MASK, 0x03c29);
		odm_set_rf_reg(dm, path, RF_0x3f, RFREG_MASK, 0x7ff34);
		odm_set_rf_reg(dm, path, RF_0xef, RFREG_MASK, 0x00000);
		odm_set_rf_reg(dm, path, RF_0x1, RFREG_MASK, 0x0001b); /*TXAGC*/
		odm_set_rf_reg(dm, path, RF_0x83, 0x000f0, 0x1); /*R1 Gain*/
		odm_set_rf_reg(dm, path, RF_0x83, 0x00007, 0x4); /*ATT Gain*/
		odm_set_rf_reg(dm, path, RF_0xdf, BIT(12), 0x1);
		odm_set_rf_reg(dm, path, RF_0x9e, BIT(8), 0x0); /*TIA gain*/
		odm_set_rf_reg(dm, path, RF_0x8f, 0x0e000, 0x3); /*PGA gain*/
	} else { /*5G*/
		odm_set_rf_reg(dm, path, RF_0x1, RFREG_MASK, 0x0001b); /*TXAGC*/
		odm_set_rf_reg(dm, path, RF_0x8c, 0x0e000, 0x7); /*att*/
		odm_set_rf_reg(dm, path, RF_0x8c, 0x01800, 0x3);
		odm_set_rf_reg(dm, path, RF_0x8c, 0x00600, 0x1);
		/*TIA gain -6db*/
		odm_set_rf_reg(dm, path, RF_0xdf, BIT(12), 0x1);
		odm_set_rf_reg(dm, path, RF_0x9e, BIT(8), 0x1);
		/*PGA gain*/
		odm_set_rf_reg(dm, path, RF_0x8f, 0x0e000, 0x3);
	}

	txagc = (u8)odm_get_rf_reg(dm, path, RF_0x1, 0x0001f);

#if 1
	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] txagc=0x%x, RF_0x0=0x%x, 0x1=0x%x, 0x5=0x%x, 0x8c=0x%x, 0x8f=0x%x\n",
	       txagc,
	       odm_get_rf_reg(dm, (enum rf_path)path, RF_0x0, RFREG_MASK),
	       odm_get_rf_reg(dm, (enum rf_path)path, RF_0x1, RFREG_MASK),
	       odm_get_rf_reg(dm, (enum rf_path)path, RF_0x5, RFREG_MASK),
	       odm_get_rf_reg(dm, (enum rf_path)path, RF_0x8c, RFREG_MASK),
	       odm_get_rf_reg(dm, (enum rf_path)path, RF_0x8f, RFREG_MASK));
#endif

	return txagc;
}

void _halrf_rx_dck_8730a(struct dm_struct *dm,	u8 path)
{
	u8 dck_ok = 0;
	u32 cnt = 0, reg = 0;

	/*RF RXDCK*/
	reg = odm_get_rf_reg(dm, path, RF_0x92, RFREG_MASK);
	odm_set_rf_reg(dm, path, RF_0x92, RFREG_MASK, reg);

	reg = odm_get_rf_reg(dm, path, RF_0x93, RFREG_MASK);
	odm_set_rf_reg(dm, path, RF_0x93, RFREG_MASK, reg);

	odm_set_rf_reg(dm, path, 0xdf, BIT(10), 0x1);
	odm_set_rf_reg(dm, path, 0x8e, BIT(8), 0x1);
	odm_set_rf_reg(dm, path, 0x8e, BIT(6), 0x1);

	//odm_set_rf_reg(dm, RF_PATH_A, 0x9b, BIT(18), 0x0); /*set DCK_OK to 0*/
	odm_set_rf_reg(dm, path, 0x81, BIT(16), 0x0); /*POW_DCK*/
	odm_set_rf_reg(dm, path, 0x81, BIT(16), 0x1); /*POW_DCK*/

	dck_ok = (u8)odm_get_rf_reg(dm, path, 0x9b, BIT(18));
	while (dck_ok != 0x1 && cnt < 1000) {
		ODM_delay_us(10);
		dck_ok = (u8)odm_get_rf_reg(dm, path, 0x9b, BIT(18));
		cnt++;
	}

	if (cnt == 1000) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] RX_DCK timeout!!!!\n");
	} else {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] RX_DCK Done (cnt = %d)\n", cnt);
	}

	odm_set_rf_reg(dm, path, 0x81, BIT(16), 0x0); /*POW_DCK*/
}

void _dpk_kip_setting_8730a(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	/*03_8730A_DPK_Preset.txt*/
	odm_set_bb_reg(dm, R_0x1b00, BIT(3), 0x1);
	odm_set_bb_reg(dm, R_0x1b08, BIT(7), 0x1);
	/*RX IQC*/
	odm_set_bb_reg(dm, R_0x1b3c, MASKDWORD, 0x20000000);
	/*force iq skew FDF placed in Tx*/
	odm_set_bb_reg(dm, R_0x2920, 0xc0000000, 0x3);

	//TXAGC_RF_K
	odm_set_bb_reg(dm, R_0x1bc4, MASKDWORD, 0x001a001a);
	/*one shot to write 1bc4 (txagc_RF_k) into table*/
	odm_set_bb_reg(dm, R_0x1bb8, BIT(0), 0x1);
	odm_set_bb_reg(dm, R_0x1bb8, BIT(0), 0x0);

	/*txagc_bnd*/
	odm_set_bb_reg(dm, R_0x1bbc, MASKDWORD, 0x00087e1f);
	odm_set_bb_reg(dm, R_0x29bc, MASKDWORD, 0x00000000);

	/*copy 0x8f[19:0] to 0x1bec[19:0]*/
	odm_set_bb_reg(dm, R_0x1bec, 0x000FFFFF,
		       odm_get_rf_reg(dm, RF_PATH_A, RF_0x8f, RFREG_MASK));

	/*TPG sel*/
	if (dpk_info->dpk_bw == 1) {
		odm_set_bb_reg(dm, R_0x1bf8, MASKDWORD, 0xd2000065);        /*20M*/
	} else {
		odm_set_bb_reg(dm, R_0x1bf8, MASKDWORD, 0xd0000068);        /*40M*/
	}

	RF_DBG(dm, DBG_RF_DPK, "[DPK] KIP setting for DPK\n");
}

void _dpk_kip_restore_8730a(struct dm_struct *dm)
{
	odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000000); /*KIP power off*/
	odm_set_bb_reg(dm, R_0x2908, MASKLWORD, 0x8000);
	odm_set_bb_reg(dm, R_0x1bb8, MASKDWORD, 0x00000000);
	odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x0); /*[5:0 ItQt]*/
	odm_set_bb_reg(dm, R_0x2920, MASKDWORD, 0x00000000); /*release iq skew FDF to BB*/
	odm_set_bb_reg(dm, R_0x2938, MASKDWORD, 0x00000000);
	odm_set_bb_reg(dm, R_0x1bf8, MASKDWORD, 0x00000000); /*DPD intput to BB*/

	odm_set_bb_reg(dm, R_0x1bb8, BIT(6), 0x1); /*txagc_offset to KIP*/

	RF_DBG(dm, DBG_RF_DPK, "[DPK] KIP restore\n");
}

u32 _dpk_dbg_report_read_8730a(struct dm_struct *dm, u8 index)
{
	u32 reg_1bfc;

	odm_write_1byte(dm, R_0x1bd6, index);
	reg_1bfc = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);

	RF_DBG(dm, DBG_RF_DPK, "[DPK][DBG] idx 0x%x = 0x%x\n", index, reg_1bfc);

	return reg_1bfc;
}

u32 _dpk_report_result_8730a(struct dm_struct *dm, u8 item)
{
	u32 result;
	u16 dc_i, dc_q;
	u8 corr_val, corr_idx;

	switch (item) {
	case PAS_READ:
		odm_set_bb_reg(dm, R_0x1bcc, BIT(26), 0x1);
		odm_set_bb_reg(dm, R_0x1b90, MASKDWORD, 0x0005e018);
		result = _dpk_dbg_report_read_8730a(dm, 0x06);
		odm_set_bb_reg(dm, R_0x1bcc, BIT(26), 0x0);
		break;

	case LOSS_CHK:
		result = (_dpk_dbg_report_read_8730a(dm, 0x01) & 0x3FFF0000) >> 16;
		if (result >> 13 != 0) {
			result = (s16)(0xc000 | result);
		}
		result = (6 * result * 100) >> 9;
		break;

	case GAIN_CHK:
		result = _dpk_dbg_report_read_8730a(dm, 0x02) & 0x3FFF;
		if (result >> 13 != 0) {
			result = (s16)(0xc000 | result);
		}
		result = (6 * result * 100) >> 9;
		break;

	case SYNC_CHK:
		odm_write_1byte(dm, R_0x1bd6, 0x0c);
		corr_idx = (u8)odm_get_bb_reg(dm, 0x1bfc, 0x000000ff);
		corr_val = (u8)odm_get_bb_reg(dm, 0x1bfc, 0x0000ff00);

		RF_DBG(dm, DBG_RF_DPK, "[DPK] Corr_idx / Corr_val = %d / %d\n",
		       corr_idx, corr_val);

		odm_write_1byte(dm, 0x1bd6, 0x0d);
		dc_i = (u16)odm_get_bb_reg(dm, 0x1bfc, 0x03ff0000);
		dc_q = (u16)odm_get_bb_reg(dm, 0x1bfc, 0x000003ff);

		if (dc_i >> 9 == 1) {
			dc_i = 0x400 - dc_i;
		}
		if (dc_q >> 9 == 1) {
			dc_q = 0x400 - dc_q;
		}

		RF_DBG(dm, DBG_RF_DPK, "[DPK] DC I/Q = %d / %d\n", dc_i, dc_q);

		if ((dc_i > 200) || (dc_q > 200)) {
			result = 1;
		} else {
			result = 0;
		}
		break;

	default:
		result = 0;
		break;
	}
	return result;
}

u8 _dpk_one_shot_8730a(struct dm_struct *dm, u8 path, u8 action)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 fail_report = 0, reg_2d9c = 0;
	u16 shot_code = 0, count = 0;

	//btc_set_gnt_wl_bt_8730a(dm, true);

	/*reset 0x2d9c*/
	odm_write_1byte(dm, R_0x1b10, 0x00);

	shot_code = ((0x10 + action) << 8) | 0x18;
	RF_DBG(dm, DBG_RF_DPK, "[DPK] one-shot code = 0x%x!!!!\n", shot_code);

	/*one shot*/
	odm_write_2byte(dm, R_0x1b00, shot_code);
	odm_write_2byte(dm, R_0x1b00, shot_code + 1);

	/*Driver waits NCTL sync done*/
	reg_2d9c = odm_read_1byte(dm, R_0x2d9c);

	while (reg_2d9c != 0x55 && count < 1000) {
		ODM_delay_us(10);
		reg_2d9c = odm_read_1byte(dm, R_0x2d9c);
		count++;
	}

	dpk_info->one_shot_cnt++;

	//btc_set_gnt_wl_bt_8730a(dm, false);

	if (count == 1000) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] one-shot over 10ms!!!!\n");
		fail_report = 1;
	} else if (odm_get_bb_reg(dm, R_0x1b08, BIT(26))) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] reg1b08 = 0x%x\n", odm_read_4byte(dm, R_0x1b08));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] NCTL Sync fail!!!!\n");
		_dpk_dbg_report_read_8730a(dm, 0xa); /*dpk fail report*/
		_dpk_dbg_report_read_8730a(dm, 0xc); /*sync report*/
		_dpk_dbg_report_read_8730a(dm, 0xd); /*rxdc*/
		_dpk_dbg_report_read_8730a(dm, 0x10); /*mean phase*/
		_dpk_dbg_report_read_8730a(dm, 0x1); /*LUT_gain*/
		_dpk_dbg_report_read_8730a(dm, 0x2); /*pga_gain*/
		_dpk_dbg_report_read_8730a(dm, 0x12);
		fail_report = 1;
	}

	return fail_report; /*LMS fail report*/
}

void _dpk_pas_read_8730a(struct dm_struct *dm,	u8 path)
{
	u8 k, j;
	u32 reg_1bfc;

	odm_set_bb_reg(dm, R_0x1bcc, BIT(26), 0x0);

	for (k = 0; k < 8; k++) {
		odm_set_bb_reg(dm, R_0x1b90, MASKDWORD, 0x0005E018 + k);
		for (j = 0; j < 4; j++) {
			reg_1bfc = _dpk_dbg_report_read_8730a(dm, 0x06 + j);
#if(DPK_PAS_CHK_DBG_8730A)
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d PAS read = 0x%x\n",
			       path, reg_1bfc);
#endif
		}
	}
}

boolean _dpk_lms_iq_check_8730a(struct dm_struct *dm,
				u8 addr,
				u32 reg_1bfc)
{
	u32 i_val = 0, q_val = 0;

	if (DPK_SRAM_IQ_DBG_8730A && addr < 16)
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] 0x1bfc[%2d] = 0x%x\n", addr, reg_1bfc);

	i_val = (reg_1bfc & 0x003FF800) >> 11;
	q_val = reg_1bfc & 0x000007FF;

	if (((q_val & 0x400) >> 10) == 1) {
		q_val = 0x800 - q_val;
	}

	if (addr == 0 && ((i_val * i_val + q_val * q_val) < 0x197a9)) {
		/* LMS (I^2 + Q^2) < -4dB happen*/
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] LUT < -4dB happen, I=0x%x, Q=0x%x\n",
		       i_val, q_val);
		return 1;
	} else {
		return 0;
	}
}

void _dpk_rxsram_read_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u16 i;
	u32 reg_1bfc;

	odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x00030001);
	odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0x00000000);
	for (i = 0; i < 0x1FF; i += 0x4) {
		odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0x00000003 | (i << 2));
		reg_1bfc = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] 0x1bfc[%3x] = 0x%x\n", i, reg_1bfc);
	}

	odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0x00000000);
}

u8 _dpk_lut_sram_read_8730a(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 i;
	u32 reg_29fc = 0;

	/*even*/
	odm_set_bb_reg(dm, R_0x29d4, MASKDWORD, 0x00020000);

	for (i = 0; i < 0x10; i++) {
		odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0xc00000e1 | (i << 1));
		reg_29fc = odm_get_bb_reg(dm, R_0x29fc, MASKDWORD);

#if(DPK_SRAM_read_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d LUT read = 0x%x\n",
		       path, reg_29fc);
#endif
		if (i > 2 && _dpk_lms_iq_check_8730a(dm, i, reg_29fc)) {
			return 0;
		}
	}
	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);

	/*odd*/
	odm_set_bb_reg(dm, R_0x29d4, MASKDWORD, 0x00030000);

	for (i = 0; i < 0x10; i++) {
		odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0xc00000e1 | (i << 1));
		reg_29fc = odm_get_bb_reg(dm, R_0x29fc, MASKDWORD);

#if(DPK_SRAM_read_DBG_8730A)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d LUT read = 0x%x\n",
		       path, reg_29fc);
#endif

		if (i > 2 && _dpk_lms_iq_check_8730a(dm, i, reg_29fc)) {
			return 0;
		}
	}

	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);

	/*Auto GS check*/
	odm_set_bb_reg(dm, R_0x29d4, 0x001f0000, 0x13);
	if (odm_get_bb_reg(dm, R_0x29fc, MASKDWORD) == 0) {
		return 0;
	}

	return 1;
}

void _dpk_lut_sram_clear_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u8 i;

	//clear even pathA
	for (i = 0; i < 0x10; i++) {
		odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0xd0000061 | (i << 1));
	}
	//clear odd pathA
	for (i = 0; i < 0x10; i++) {
		odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x900000e1 | (i << 1));
	}
	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);
	/*GainLoss = 1db*/
	odm_set_bb_reg(dm, R_0x1bbc, BIT(27), 0x1);
	odm_set_bb_reg(dm, R_0x1be8, MASKDWORD, 0x40004000);
}

void _dpk_manual_lut_write_8730a(struct dm_struct *dm, u8 path)
{
	u8 i;
	u32 lut07;

	/*Fill lut 1-6th with lut7 value to fixed DPK Kfail issue*/
	//read even (LUT table 7th entrie)
	odm_set_bb_reg(dm, R_0x29d4, MASKDWORD, 0x00020000);
	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0xc00000e7);
	lut07 = odm_get_bb_reg(dm, R_0x29fc, MASKDWORD) & 0x003fffff;

	//write even
	/*enable lut even write*/
	odm_set_bb_reg(dm, R_0x1bdc, BIT(31) | BIT(30), 0x3);
	/*select even 0-2th entrie*/
	for (i = 0; i < 0x3; i++) {
		odm_write_1byte(dm, R_0x1bdc, 0x61 | (i << 1));
		odm_set_bb_reg(dm, R_0x1bdc, 0x3fffff00, lut07);
	}
	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);

	//write odd
	odm_set_bb_reg(dm, R_0x1bdc, BIT(31) | BIT(30), 0x2);
	/*select even 0-2th entrie*/
	for (i = 0; i < 0x3; i++) {
		odm_write_1byte(dm, R_0x1bdc, 0xe1 | (i << 1));
		odm_set_bb_reg(dm, R_0x1bdc, 0x3fffff00, lut07);
	}
	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);
}

void _dpk_gs_normalize_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u8 i;
	u32 reg_29fc = 0, i_val, q_val, pwr, gs_nom;
	boolean need_cal = true;

	/*even*/
	odm_set_bb_reg(dm, 0x29d4, MASKDWORD, 0x00020000);

	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0xc00000e7);
	reg_29fc = odm_get_bb_reg(dm, 0x29fc, 0x003FFFFF); /*[21:0]*/

	odm_set_bb_reg(dm, R_0x1bdc, MASKDWORD, 0x00000000);

	i_val = (reg_29fc & 0x003FF800) >> 11;
	q_val = reg_29fc & 0x000007FF;

	if (((q_val & 0x400) >> 10) == 1) {
		q_val = 0x800 - q_val;
	}

	pwr = i_val * i_val + q_val * q_val;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] gs pwr = %d\n", pwr);

	if (247479 >= pwr && pwr > 220566) {	/*-0.25 ~ -0.75*/
		gs_nom = 0x43ca;
	} else if (220566 >= pwr && pwr > 196580) {	/*-0.75 ~ -1.25*/
		gs_nom = 0x47cf;
	} else if (196580 >= pwr && pwr > 175202) {	/*-1.25 ~ -1.75*/
		gs_nom = 0x4c10;
	} else if (175202 >= pwr && pwr > 156149) {	/*-1.75 ~ -2.25*/
		gs_nom = 0x5092;
	} else if (156149 >= pwr && pwr > 139168) {	/*-2.25 ~ -2.75*/
		gs_nom = 0x5558;
	} else if (139168 >= pwr && pwr > 124033) {	/*-2.75 ~ -3.25*/
		gs_nom = 0x5a67;
	} else if (124033 >= pwr && pwr > 110545) {	/*-3.25 ~ -3.75*/
		gs_nom = 0x5fc2;
	} else if (110545 >= pwr && pwr > 98523) {	/*-3.75 ~ -4.25*/
		gs_nom = 0x656e;
	} else if (98523 >= pwr && pwr > 87809) {	/*-4.25 ~ -4.75*/
		gs_nom = 0x6b71;
	} else if (87809 >= pwr && pwr > 78260) {	/*-4.75 ~ -5.25*/
		gs_nom = 0x71cf;
	} else {
		need_cal = false;
	}

	odm_set_bb_reg(dm, 0x29d4, MASKDWORD, 0x00130000);
	reg_29fc = odm_get_bb_reg(dm, 0x29fc, MASKDWORD) >> 16;

	if (need_cal) {
		odm_set_bb_reg(dm, 0x1be8, MASKHWORD, gs_nom);
		odm_set_bb_reg(dm, 0x1bbc, BIT(27), 0x1); /*gs manual mode*/
		RF_DBG(dm, DBG_RF_DPK, "[DPK] gs cal to 0x%x, auto_gs 0x%x\n",
		       gs_nom, reg_29fc);
	}

}

u8 _dpk_agc_tune_8730a(struct dm_struct *dm, u8 path, u8 txagc)
{
	u8 agc_backoff, offset = 0, new_agc = txagc;
	u8 tmp_pga = 0, result = 0;
	s16 tmp_gain, tmp_loss, gl;

#if (DPK_PAS_DBG_8730A)
	_dpk_pas_read_8730a(dm, path);
#endif

	tmp_pga = odm_get_rf_reg(dm, path, RF_0x8f, 0x0E000); /*[15:13]*/

	agc_backoff = (u8)_dpk_report_result_8730a(dm, PAS_READ);

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] TXAGC/PGA/GL_idx = 0x%x / 0x%x / 0x%x\n",
	       txagc, tmp_pga, agc_backoff);

	/*check auto_pga fail & retry*/
	tmp_gain = (s16)_dpk_report_result_8730a(dm, GAIN_CHK);
	tmp_loss = (s16)_dpk_report_result_8730a(dm, LOSS_CHK);
	gl = tmp_gain - tmp_loss;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] G=%d.%02d, GL=%d.%02ddB\n",
	       tmp_gain / 100, HALRF_ABS(0, tmp_gain) % 100,
	       gl / 100, gl % 100);

	if (tmp_pga == 0x7 && tmp_gain > -100) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK][AGC] Auto PGA abnormal!!\n");
		result = 4;
		return result;
	} else if (tmp_pga == 0x0 && tmp_gain < -800) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK][AGC] Auto PGA abnormal!!\n");
		result = 4;
		return result;
	} else if (txagc == 0x1f && gl > 180) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK][AGC] TXAGC done\n");
		result = 3;
		return result;
	}

	/*Adjust TRXAGC*/
	if (agc_backoff < 0x5) {
		result = 1;
	} else if (agc_backoff == 0xA) {
		result = 2;
	} else if (agc_backoff < 0xA && agc_backoff > 0x4) {
		result = 3;
		new_agc = txagc - (0xA - agc_backoff);

		offset = (0xa - agc_backoff) / 2;
		if (agc_backoff < 9 && tmp_pga != 0x7) {
			if (tmp_pga == 0x3 || (tmp_pga < 0x3 && (tmp_pga + offset > 0x3))) { /*due to PGA gap*/
				tmp_pga += offset + 1;
			} else {
				tmp_pga += offset;
			}

			RF_DBG(dm, DBG_RF_DPK, "[DPK] Adjust PGA +%d for DPK\n", offset);
		}

		if (tmp_pga > 0x7) {
			tmp_pga = 0x07;
		}

		odm_set_rf_reg(dm, path, RF_0x1, 0x0001f, new_agc);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x8f, 0x0e000, tmp_pga);

		RF_DBG(dm, DBG_RF_DPK, "[DPK] New TXAGC/PGA = 0x%x / 0x%x\n", new_agc, tmp_pga);
	} else {
		result = 4;
	}
	return result;
}

u8 _dpk_gainloss_adjust_8730a(struct dm_struct *dm, u8 path, u8 ori_agc)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 tmp_txagc = 0, i = 0;
	u8 goout = 0, agc_cnt = 0, agc_done = 0;

	tmp_txagc = ori_agc;

	do {
		switch (i) {
		case 0:
			odm_set_rf_reg(dm, path, RF_0x8f, 0x0E000, 0x3);
			/*wire r_iqk_IO_RFC_en*/
			odm_set_bb_reg(dm, R_0x1880, BIT(21), 1);
			_dpk_one_shot_8730a(dm, path, GAIN_LOSS);
			odm_set_bb_reg(dm, R_0x1880, BIT(21), 0);
			odm_set_rf_reg(dm, path, RF_0x0, RFREG_MASK, 0x503e0);

			if (agc_cnt == 0 && _dpk_report_result_8730a(dm, SYNC_CHK) == 1) {
				RF_DBG(dm, DBG_RF_DPK, "[DPK] SYNC check fail !!\n");
				goout = 1;
				break;
			}
			RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1=0x%x,0x56=0x%x\n",
			       odm_get_rf_reg(dm, path, RF_0x1, RFREG_MASK),
			       odm_get_rf_reg(dm, path, RF_0x56, RFREG_MASK));

			i = _dpk_agc_tune_8730a(dm, path, tmp_txagc);
			agc_cnt++;
			break;

		case 1: /*GL_BACK < 0x5*/
			if (tmp_txagc < 0xa) {
				goout = 1;
				RF_DBG(dm, DBG_RF_DPK,
				       "[DPK][AGC] TXAGC@ lower bound!!\n");
				break;
			}
			tmp_txagc = tmp_txagc - 2;
			odm_set_rf_reg(dm, path, RF_0x1, 0x0001f, tmp_txagc);
			RF_DBG(dm, DBG_RF_DPK, "[DPK][AGC] TXAGC(-2) = 0x%x\n",
			       tmp_txagc);
			i = 0;
			break;

		case 2:	/*GL_BACK = 0xA*/
			if (tmp_txagc == 0x1f) {
				goout = 1;
				RF_DBG(dm, DBG_RF_DPK,
				       "[DPK][AGC] TXAGC@ upper bound!!\n");
				break;
			}

			if (tmp_txagc > 0x1c) {
				tmp_txagc = tmp_txagc + 1;
			} else {
				tmp_txagc = tmp_txagc + 2;
			}
			odm_set_rf_reg(dm, path, RF_0x1, 0x0001f, tmp_txagc);
			RF_DBG(dm, DBG_RF_DPK,
			       "[DPK][AGC] TXAGC(+) = 0x%x\n", tmp_txagc);
			i = 0;
			break;

		case 3:/*adjust done*/
			agc_done = 1;
			goout = 1;
			break;

		case 4: /*adjust fail*/
			_dpk_dbg_report_read_8730a(dm, 0x0c);
			_dpk_dbg_report_read_8730a(dm, 0x0d);
			_dpk_dbg_report_read_8730a(dm, 0x2);
			//_dpk_rxsram_read_8730a(dm, path);
			goout = 1;
			RF_DBG(dm, DBG_RF_DPK, "[DPK][AGC] AUTO PGA fail!!!\n");
			break;

		default:
			goout = 1;
			break;
		}
	} while (!goout && (agc_cnt < 5));

	RF_DBG(dm, DBG_RF_DPK, "[DPK] ++++GLEND: cnt %d, agc_done = 0x%x++++\n",
	       agc_cnt, agc_done);
	return agc_done;
}

u8 _dpk_nctl_learning_8730a(struct dm_struct *dm,	u8 path)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 result = 0;

	/*05-NCTL_DPD_Learning*/
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x0, RFREG_MASK, 0x503e0);

	/*LMS only one-shot*/
	result = _dpk_one_shot_8730a(dm, path, DO_DPK);

	dpk_info->thermal_dpk[path] = _dpk_thermal_read_8730a(dm, path);
	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK_track] S%d thermal at K= %d\n", path,
	       dpk_info->thermal_dpk[path]);

	RF_DBG(dm, DBG_RF_DPK, "[DPK][DO_DPK] 0x1b20=0x%x, 0x1bcc=0x%x\n",
	       odm_get_bb_reg(dm, R_0x1b20, MASKDWORD),
	       odm_get_bb_reg(dm, R_0x1bcc, MASKDWORD));

	RF_DBG(dm, DBG_RF_DPK, "[DPK][DO_DPK] DPK Fail = %x\n", result);

#if (DPK_LMS_DBG_8730A)
	_dpk_pas_read_8730a(dm, path);
#endif
	return result;
}

void _dpk_on_8730a(struct dm_struct *dm, u8 path)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u8 rf_txagc = 0x11;
	s16 pwsf_offset = 0;
	u8 tx_agc;

	tx_agc = odm_get_rf_reg(dm, path, RF_0x1, RFREG_MASK) & 0x1f;

	dpk_info->txagc[path] = tx_agc;

	/*PWSF+1 = 0.125db; +8 = +1db*/
	pwsf_offset = (((0x1a - tx_agc) << 3) + 0x4f) & 0x1ff;

	odm_set_bb_reg(dm, R_0x1bd8, 0x3fe00000, pwsf_offset);

	dpk_info->pwsf[path] = pwsf_offset;

	/*LUT_point 6th selected to do auto gainscaling for workaround1*/
	odm_set_bb_reg(dm, R_0x1bec, 0x00e00000, 0x6);

	/*dpk default setting*/
	odm_write_1byte(dm, R_0x1b23, 0x03);
	odm_set_bb_reg(dm, R_0x1bbc, 0xff000000, 0x20);

	if ((dpk_info->dpk_path_ok & BIT(path)) >> path) {
		odm_set_bb_reg(dm, R_0x1b20, BIT(25), 0x0);
		/*workaround1*/
		_dpk_manual_lut_write_8730a(dm, path);
		_dpk_gs_normalize_8730a(dm);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPD on!!!\n", path);
	}
}

u8 _dpk_check_fail_8730a(struct dm_struct *dm,
			 boolean is_fail,
			 u8 path)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 result = 0;

	if (!is_fail && _dpk_lut_sram_read_8730a(dm, path)) {
		dpk_info->dpk_path_ok = dpk_info->dpk_path_ok | (1 << path);
		result = 1; /*check PASS*/
	}

	return result;
}

void _dpk_calibrate_8730a(struct dm_struct *dm)
{
	u8 dpk_fail = 1, retry_cnt;
	u8 txagc = 0, agc_done = 0;
	u8 path = RF_PATH_A;

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] =========== S%d DPK Start ===========\n", path);


	for (retry_cnt = 0; retry_cnt < 1; retry_cnt++) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] retry = %d\n", retry_cnt);

		txagc = _dpk_rf_setting_8730a(dm, path);
		_halrf_rx_dck_8730a(dm, path);

		_dpk_kip_setting_8730a(dm);

		agc_done = _dpk_gainloss_adjust_8730a(dm, path, txagc);

		if (agc_done) {
			dpk_fail = _dpk_nctl_learning_8730a(dm, path);
		}

		if (_dpk_check_fail_8730a(dm, dpk_fail, path)) {
			_dpk_on_8730a(dm, RF_PATH_A);
			break;
		}
	}

	_dpk_kip_restore_8730a(dm);

	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] =========== S%d DPK Finish ==========\n", path);
}

void _dpk_result_summary_8730a(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 path = dpk_info->dpk_current_path;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] ======== DPK Result Summary =======\n");

	for (path = 0; path < DPK_RF_PATH_NUM_8730A; path++) {
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] S%d txagc = 0x%x, pwsf offset = 0x%x\n",
		       path, dpk_info->txagc[path],
		       dpk_info->pwsf[path]);

		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPK is %s\n", path,
		       ((dpk_info->dpk_path_ok & BIT(path)) >> path) ?
		       "Success" : "Fail");
	}

	RF_DBG(dm, DBG_RF_DPK, "[DPK] dpk_path_ok = 0x%x\n",
	       dpk_info->dpk_path_ok);
	RF_DBG(dm, DBG_RF_DPK, "[DPK] dpk_one_shot_cnt = 0x%x\n",
	       dpk_info->one_shot_cnt);

	RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1b20=0x%x, 0x1bd8=0x%x, 0x1bbc=0x%x\n",
	       odm_get_bb_reg(dm, R_0x1b20, MASKDWORD),
	       odm_get_bb_reg(dm, R_0x1bd8, MASKDWORD),
	       odm_get_bb_reg(dm, R_0x1bbc, MASKDWORD));
	RF_DBG(dm, DBG_RF_DPK, "[DPK] ======== DPK Result Summary =======\n");
}

void _dpk_result_reset_8730a(struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u8 i, path;

	dpk_info->dpk_path_ok = 0x0;
	dpk_info->one_shot_cnt = 0;

	for (path = 0; path < DPK_RF_PATH_NUM_8730A; path++) {
		dpk_info->txagc[path] = 0;
		dpk_info->pwsf[path] = 0;
		dpk_info->last_offset[path] = 0;
		dpk_info->thermal_dpk[path] = 0;
		dpk_info->thermal_init[path] = 0;
	}
}

void dpk_reload_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	return;
}

void _dpk_force_bypass_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_bb_reg(dm, R_0x1b00, 0x0000000f, 0x8);
	odm_set_bb_reg(dm, R_0x1b20, BIT(25), 0x1);

	RF_DBG(dm, DBG_RF_DPK, "[DPK] DPK Force bypass !!!\n");
}

void do_dpk_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	u32 bb_reg_backup[DPK_BB_REG_NUM_8730A];
	u32 rf_reg_backup[DPK_RF_REG_NUM_8730A][DPK_RF_PATH_NUM_8730A];

	u32 bb_reg[DPK_BB_REG_NUM_8730A] = {R_0x522, R_0x9f0, R_0x1d40,
					    R_0x1b38, R_0x1b3c, R_0x1e70, R_0x1c38, R_0x1880, R_0x4384
					   };
	u32 rf_reg[DPK_RF_REG_NUM_8730A] = {
		RF_0x0, RF_0x1, RF_0x5, RF_0x1f, RF_0x83, RF_0x8c, RF_0x8f,
		RF_0x9e, RF_0x92, RF_0x93, RF_0xdf
	};

	if (!dpk_info->is_dpk_pwr_on) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Skip DPK due to DPD PWR off !!\n");
		_dpk_lut_sram_clear_8730a(dm);
		_dpk_force_bypass_8730a(dm);
		return;
	}

	if (!dpk_info->is_dpk_enable) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Disable DPK !!\n");
		_dpk_force_bypass_8730a(dm);
		return;
	}


	RF_DBG(dm, DBG_RF_DPK,
	       "[DPK] ************* DPK Start *************\n");

	_dpk_information_8730a(dm);
	_dpk_result_reset_8730a(dm);
	_dpk_backup_bb_registers_8730a(dm, bb_reg, bb_reg_backup);
	_backup_rf_registers_8730a(dm, rf_reg, rf_reg_backup);
	_dpk_afe_setting_8730a(dm);
	_dpk_calibrate_8730a(dm);
	_dpk_result_summary_8730a(dm);
	_dpk_afe_restore_8730a(dm);

	_reload_rf_registers_8730a(dm, rf_reg, rf_reg_backup);
	_dpk_reload_bb_registers_8730a(dm, bb_reg, bb_reg_backup);
}

void dpk_enable_disable_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 path;

	for (path = 0; path < DPK_RF_PATH_NUM_8730A; path++) {
		if ((dpk_info->dpk_path_ok & BIT(path)) >> path) {
			if (dpk_info->is_dpk_enable) {
				odm_set_bb_reg(dm, R_0x1b20, BIT(25), 0x0);
				RF_DBG(dm, DBG_RF_DPK,
				       "[DPK] S%d DPK enable !!!\n", path);
			} else {
				odm_set_bb_reg(dm, R_0x1b20, BIT(25), 0x1);
				RF_DBG(dm, DBG_RF_DPK,
				       "[DPK] S%d DPK disable !!!\n", path);
			}
		}
	}
}

void dpk_track_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;
#if 0
	u8 trk_idx = 0, txagc_rf = 0;
	s8 txagc_bb = 0, txagc_bb_tp = 0, txagc_ofst = 0;

	if (!dpk_info->is_dpk_pwr_on || !dpk_info->is_dpk_enable) {
		return;
	} else {
		RF_DBG(dm, DBG_RF_DPK_TRACK,
		       "[DPK_track] ================[CH %d]================\n",
		       dpk_info->dpk_ch);
	}

	txagc_rf = odm_get_bb_reg(dm, R_0x42f0, 0x0000001f); /*[4:0]*/

	if (rf->is_tssi_mode[RF_PATH_A]) { /*TSSI mode*/
		trk_idx = (u8)odm_get_rf_reg(dm, RF_PATH_A, 0x5D, 0x7E000); /*[18:13] for integer*/

		RF_DBG(dm, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_RF / track_idx = 0x%x / %d\n",
		       txagc_rf, trk_idx);

		txagc_bb = (s8)odm_get_bb_reg(dm, R_0x42f0, 0x0000FF00); /*[15:8]*/
		txagc_bb_tp = (u8)odm_get_bb_reg(dm, 0x4278, 0x000000FF); /*[7:0]*/

		RF_DBG(dm, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_bb_tp / txagc_bb = 0x%x / 0x%x\n",
		       txagc_bb_tp, txagc_bb);

		txagc_ofst = (s8)odm_get_bb_reg(dm, 0x42f4, 0x000000FF); /*[7:0]*/


		RF_DBG(dm, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_offset = %d\n", txagc_ofst);
	}

#endif
}
#endif
