// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2018  Realtek Corporation.
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

void do_iqk_8730a(void *dm_void,
		  u8 delta_thermal_index,
		  u8 thermal_value,
		  u8 threshold)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	dm->rf_calibrate_info.thermal_value_iqk = thermal_value;
	halrf_iqk_trigger(dm, false);
}

void _iqk_information_8730a(struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	u32  reg_rf18;

	if (odm_get_bb_reg(dm, R_0x1e7c, BIT(30))) {
		iqk_info->is_tssi_mode = true;
	} else {
		iqk_info->is_tssi_mode = false;
	}

	reg_rf18 = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, RFREG_MASK);
	iqk_info->iqk_band = (u8)((reg_rf18 & BIT(16)) >> 16); /*0/1:G/A*/
	iqk_info->iqk_ch = (u8)reg_rf18 & 0xff;
	iqk_info->iqk_bw = (u8)((reg_rf18 & 0x400) >> 10); /*3/2/1:20/40/80*/
	/*
		RF_DBG(dm, DBG_RF_DPK, "[IQK] TSSI/ Band/ CH/ BW = %d / %s / %d / %s\n",
		       iqk_info->is_tssi_mode, iqk_info->iqk_band == 0 ? "2G" : "5G",
		       iqk_info->iqk_ch,
		       iqk_info->iqk_bw == 3 ? "20M" : (iqk_info->iqk_bw == 2 ? "40M" : "80M"));
	*/
	RF_DBG(dm, DBG_RF_IQK, "[IQK] TSSI/ Band/ CH/ BW = %d / %s / %d / %s\n",
	       iqk_info->is_tssi_mode, iqk_info->iqk_band == 0 ? "2G" : "5G",
	       iqk_info->iqk_ch, iqk_info->iqk_bw == 0 ? "40MHz" : "20MHz");
}

void _iqk_fill_iqk_xy_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk = &dm->IQK_info;

	RF_DBG(dm, DBG_RF_IQK, "[IQK] ======>%s\n", __func__);

	//odm_set_bb_reg(dm, 0x1b38, 0x3, 0x1);
	odm_set_bb_reg(dm, 0x1b38, 0x7FF00000, iqk->txxy[0][0]);
	odm_set_bb_reg(dm, 0x1b38, 0x0007FF00, iqk->txxy[0][1]);

	//lna=0x3
	//odm_set_bb_reg(dm, 0x1b34, 0x00070000, 0x3);
	odm_set_bb_reg(dm, 0x2908, 0x0000FFFF, 0x8083);//lna=0x3
	odm_set_bb_reg(dm, 0x1b3c, 0x7FF00000, iqk->rxxy[0][0]);
	odm_set_bb_reg(dm, 0x1b3c, 0x0007FF00, iqk->rxxy[1][0]);

	//lna=0x6
	//odm_set_bb_reg(dm, 0x1b34, 0x00070000, 0x6);
	odm_set_bb_reg(dm, 0x2908, 0x0000FFFF, 0x8086);//lna=0x3
	odm_set_bb_reg(dm, 0x1b3c, 0x7FF00000, iqk->rxxy[0][1]);
	odm_set_bb_reg(dm, 0x1b3c, 0x0007FF00, iqk->rxxy[1][1]);

	RF_DBG(dm, DBG_RF_IQK, "[IQK] 0x1b38 = 0x%x, 0x1b3c = 0x%x\n",
	       odm_get_bb_reg(dm, 0x1b38, MASKDWORD),
	       odm_get_bb_reg(dm, 0x1b3C, MASKDWORD));
}

void _iqk_backup_mac_bb_8730a(struct dm_struct *dm,
			      u32 *MAC_backup,
			      u32 *BB_backup,
			      u32 *backup_mac_reg,
			      u32 *backup_bb_reg)
{
	u32 i;

	for (i = 0; i < MAC_REG_NUM_8730A; i++) {
		MAC_backup[i] = odm_read_4byte(dm, backup_mac_reg[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]Backup mac addr = %x, value =% x\n", backup_mac_reg[i], MAC_backup[i]);
	}
	for (i = 0; i < BB_REG_NUM_8730A; i++) {
		BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]Backup bbaddr = %x, value =% x\n", backup_bb_reg[i], BB_backup[i]);
	}
	RF_DBG(dm, DBG_RF_IQK, "[IQK]BackupMacBB Success!!!!\n");
}

void _iqk_backup_rf_8730a(struct dm_struct *dm,
			  u32 RF_backup[][RF_PATH_MAX_8730A],
			  u32 *backup_rf_reg)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 i;

	for (i = 0; i < RF_REG_NUM_8730A; i++) {
		RF_backup[i][RF_PATH_A] = odm_get_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], RFREGOFFSETMASK);
	}
	RF_DBG(dm, DBG_RF_IQK, "[IQK]BackupRF Success!!!!\n");
}

void _iqk_restore_rf_8730a(struct dm_struct *dm,
			   u32 *rf_reg,
			   u32 temp[][RF_PATH_MAX_8730A])
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 i;

	for (i = 0; i < RF_REG_NUM_8730A; i++) {
		odm_set_rf_reg(dm, RF_PATH_A, rf_reg[i],
			       0xfffff, temp[i][RF_PATH_A]);
	}
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RestoreRF Success!!!!\n");
}

void _iqk_afe_setting_8730a(struct dm_struct *dm,	boolean do_iqk)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (do_iqk) {
		odm_set_bb_reg(dm, R_0x1b1c, MASKDWORD, 0xA210FC00);
		/*01_8730A_AFE_ON_BB_settings.txt*/
		odm_set_bb_reg(dm, R_0x1c38, MASKDWORD, 0x0);//ADDA fifo force off until clk is ready
		//--Anapar force mode--//
		odm_set_bb_reg(dm, R_0x1810, MASKDWORD, 0x10001600);
		odm_set_bb_reg(dm, R_0x1814, MASKDWORD, 0x5bba5020);
		odm_set_bb_reg(dm, R_0x1818, MASKDWORD, 0x0001d429);
		odm_set_bb_reg(dm, R_0x181c, MASKDWORD, 0x00000140);
		odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x0); //force ADDA

		odm_set_bb_reg(dm, R_0x1860, 0xFF000000, 0x0f); //ADDA all on
		odm_set_bb_reg(dm, R_0x1860, 0x00FFF000, 0x001); //ADDA all on
		//--CLK rate, AD 80M--//
		odm_set_bb_reg(dm, 0x09f0, 0x0000FFFF, 0x5555);
		odm_set_bb_reg(dm, 0x1d40, BIT(3), 0x1);
		odm_set_bb_reg(dm, 0x1d40, 0x00000007, 0x4);
		//--DAC clk rate,DA 160M--//
		odm_set_bb_reg(dm, 0x09b4, 0x00000700, 0x5);//[10:8]
		odm_set_bb_reg(dm, 0x09b4, 0x00003800, 0x5);//[13:11]
		odm_set_bb_reg(dm, 0x09b4, 0x0001C000, 0x5);//[16:14]
		odm_set_bb_reg(dm, 0x09b4, 0x000E0000, 0x5);//[19:17]
		odm_set_bb_reg(dm, R_0x1c20, BIT(5), 0x1);
		//--BB settings--//
		//odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000080);//IQK&DPK KIP power on
		odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x0); //r_path_en_en
		odm_set_bb_reg(dm, R_0x1e28, 0x0000000F, 0x1); //wire [3:0] path_en_seg0_sel
		odm_set_bb_reg(dm, R_0x824, 0x000F0000, 0x1); //wire [3:0] path_en_seg0_sel
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
		//odm_set_bb_reg(dm, R_0x1b1c, MASKDWORD, 0xA2103C02);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]AFE setting for IQK mode!!!!\n");
	} else {
		/*10_8730A_restore_AFE_BB_settings.txt*/
		//Anapar force mode
		odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x1);
		odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x1);
		odm_set_bb_reg(dm, R_0x2a24, BIT(13), 0x0);
		odm_set_bb_reg(dm, R_0x1c68, BIT(24), 0x0);
		odm_set_bb_reg(dm, R_0x1864, BIT(31), 0x0);
		odm_set_bb_reg(dm, R_0x180c, BIT(27), 0x0);
		odm_set_bb_reg(dm, R_0x180c, BIT(30), 0x0);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]AFE restore for Normal mode!!\n");
	}
//#endif
}

void _iqk_preset_8730a(struct dm_struct *dm,	boolean do_iqk)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (do_iqk) {
		/*02_IQK_Preset.txt*/
		// RF does not control by bb
		odm_set_rf_reg(dm, RF_PATH_A, 0x05, BIT(0), 0x0);
		//RXIQKPLL mode table
		odm_set_rf_reg(dm, RF_PATH_A, 0xef, 0xFFFFF, 0x80000);
		odm_set_rf_reg(dm, RF_PATH_A, 0x33, 0xFFFFF, 0x00007);
		odm_set_rf_reg(dm, RF_PATH_A, 0x3E, 0xFFFFF, 0x00071);
		odm_set_rf_reg(dm, RF_PATH_A, 0x3f, 0xFFFFF, 0xFFB06);
		odm_set_rf_reg(dm, RF_PATH_A, 0xef, 0xFFFFF, 0x0);
		//cip_power_on
		odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000080);
		odm_set_bb_reg(dm, 0x2938, MASKDWORD, 0x000f0000);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]iqk_preset\n");

	} else {
		/*10_IQK_Reg_Restore.txt*/
		odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 0x0);
		// RF Restore
		odm_set_rf_reg(dm, RF_PATH_A, 0x05, BIT(0), 0x1);
		// [7]cip_power_on
		odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000000);
		// [2]lna_sel
		odm_set_bb_reg(dm, 0x2908, 0x0000FFFF, 0x00008000);
		// b8[20]= tst_iqk2set
		odm_set_bb_reg(dm, 0x1bb8, MASKDWORD, 0x00000000);
		odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x0);//[5:0]=ItQt
		odm_set_bb_reg(dm, 0x2920, MASKDWORD, 0x00000000);
		odm_set_bb_reg(dm, 0x2938, MASKDWORD, 0x00000000);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]iqk_reg_restore\n");
	}

}

void _iqk_restore_mac_bb_8730a(struct dm_struct *dm,
			       u32 *MAC_backup,
			       u32 *BB_backup,
			       u32 *backup_mac_reg,
			       u32 *backup_bb_reg)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 i;

	for (i = 0; i < MAC_REG_NUM_8730A; i++) {
		odm_write_4byte(dm, backup_mac_reg[i], MAC_backup[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]restore mac = %x, value = %x\n",backup_mac_reg[i],MAC_backup[i]);
	}
	for (i = 0; i < BB_REG_NUM_8730A; i++) {
		odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]restore bb = %x, value = %x\n",backup_bb_reg[i],BB_backup[i]);
	}
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RestoreMacBB Success!!!!\n");
}

void _iqk_rxk_rf_setting_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 rf_reg1F;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]rxk START : Set RF Setting!\n");
	//odm_set_bb_reg(dm, R_0x1860, BIT(30), 0x0); // DAC off
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x8f, 0x0E000, 0x0);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000, 0x7);
	if (iqk_info->iqk_band == 0) {
		//G mode: IQKPLL_EN_IQK_G: 20[8]=1
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x20, BIT(8), 0x1);
	} else {
		//A mode: IQKPLL_EN_IQK_A: 20[7]=1
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x20, BIT(7), 0x1);
	}

	rf_reg1F = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, 0xFFFFF);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x1f, 0xFFFFF, rf_reg1F);
	// 1E[5:0]=IQKPLL_FOS=6'h13 (4.25MHz)
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x1e, 0x0003F, 0x13);
	//1E[19]=POW_IQKPLL
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x1e, BIT(19), 0x0);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x1e, BIT(19), 0x1);
	// IQKPLL's settling time needs 60us.
	ODM_delay_ms(1);
}

void _iqk_txk_rf_setting_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 tx_pi_data;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]txk START : Set RF Setting!\n");
	// ----- START : Set RF Setting -----
	odm_set_rf_reg(dm, RF_PATH_A, RF_0xdf, 0x001FF, 0x1fc);
	if (iqk_info->iqk_band == 0) {//G mode
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x53, 0x00070, 0x1);
		// for LOK Setting
		odm_set_rf_reg(dm, RF_PATH_A, 0x59, BIT(18), 0x0); //EN_TXGAIN_FOR_LOK=0
		odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 0x1);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0xde, BIT(19), 0x0);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x7c, 0x0000F, 0x0);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x403E);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x56, 0x0FFFF, 0xe0ed);
	} else {
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x63, 0x001C0, 0x7);
		// for LOK Setting
		odm_set_rf_reg(dm, RF_PATH_A, 0x59, BIT(18), 0x0); //EN_TXGAIN_FOR_LOK=0
		odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 0x1);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0xde, BIT(19), 0x0);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x7c, 0x0000F, 0x4);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x403E);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x56, 0x0FFFF, 0xe0ea);
	}
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x8f, 0x0E000, 0x6);
	// ----- END : Set RF Setting -----
	/*
	// default IDAC
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x08, 0xFFFFF, 0x80200);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x09, 0xFFFFF, 0x80200);

	// TX_PI_DATA[19:0] is same as RF0x00
	tx_pi_data = odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFFF);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]TX_PI_DATA = 0x%x\n", tx_pi_data);
	odm_set_bb_reg(dm, R_0x1b20, 0x000FFFFF, tx_pi_data);
	odm_set_bb_reg(dm, R_0x1b20, 0x0F000000, 0x0); // disable DPD
	odm_set_bb_reg(dm, R_0x1bbc, 0x30000000, 0x0); // disable DPD
	odm_set_bb_reg(dm, R_0x1b1c, 0x0001C000, 0x0); // [16:14]=TX_P_Avg
	RF_DBG(dm, DBG_RF_IQK, "[IQK]1b20= 0x%x\n",
	       odm_get_bb_reg(dm, R_0x1b20, MASKDWORD));
	//odm_set_bb_reg(dm, R_0x1bb8, BIT(20), 0x0); // r_tst_iqk2set = 0x0
	*/
}

void _iqk_lok_by_path_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 KFAIL = 1, i = 0;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]LOK!\n");
	// ============START : FLoK_coarse ========
	//odm_set_rf_reg(dm, RF_PATH_A, RF_0xf5, BIT(17), 0x1);  // clock gating
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]RF_0xf5[17] = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0xf5, BIT(17)));
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x1); //wire r_iqk_IO_RFC_en
	odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x09); // ItQt
	odm_set_bb_reg(dm, R_0x1b00, 0x00001FFF, 0x018); //set cal_path, process id
	odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00); //0x8010[7:0]-->0x2d9c[7:0]
	RF_DBG(dm, DBG_RF_IQK, "[IQK]1b00[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x1b00, MASKDWORD));
	if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x0) {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]START : NB TXLOK NCTL(one shot) corse!\n");
		odm_set_bb_reg(dm, R_0x1b00, BIT(0), 0x1);//one shot
		while (i < 10) {
			i++;
			ODM_delay_ms(1);
			if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x55) {
				RF_DBG(dm, DBG_RF_IQK, "[IQK]NCTL FLOK corse done, delaytime = %d ms!\n", i);
				break;
			} else {
				RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
				continue;
			}
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
	}
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0); //wire r_iqk_IO_RFC_en
	//============END : FLoK_coarse ========
	//============START : FLoK_Fine ========
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x403E);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RF_0x00 = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFFF));
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x1); //wire r_iqk_IO_RFC_en
	odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x09); // ItQt
	odm_set_bb_reg(dm, R_0x1b00, 0x00001FFF, 0x118); //set cal_path, process id
	odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00); //0x8010[7:0]-->0x2d9c[7:0]
	RF_DBG(dm, DBG_RF_IQK, "[IQK]1b00[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x1b00, MASKDWORD));
	if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x0) {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]START : NB TXLOK NCTL(one shot) fine!\n");
		odm_set_bb_reg(dm, 0x1b00, BIT(0), 0x1);//one shot
		while (i < 10) {
			i++;
			ODM_delay_ms(1);
			if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x55) {
				RF_DBG(dm, DBG_RF_IQK, "[IQK]NCTL FLOK fine done, delaytime = %d ms!\n", i);
				break;
			} else {
				RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
				continue;
			}
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
	}
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0); //wire r_iqk_IO_RFC_en
	//============END : FLoK_Fine ========
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RF 08 = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0x08, 0xFFFFF));
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RF 09 = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0x09, 0xFFFFF));
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]RF_0xf5[17] = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0xf5, BIT(17)));
}

void _iqk_txk_by_path_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 KFAIL = 1;
	u8 i = 0x0;
	u32 IQK_CMD = 0;

	if (*dm->band_width == CHANNEL_WIDTH_20) {
		IQK_CMD = 0x318;
		RF_DBG(dm, DBG_RF_IQK, "[IQK]WB_20M FTXK !\n");
	} else {
		IQK_CMD = 0x418;
		RF_DBG(dm, DBG_RF_IQK, "[IQK]WB_40M FTXK !\n");
	}

	//IQK_CMD = 0x218;
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]NB FTXK !\n");
	// ====== START : NB TXIQK =====
	// ====START : NCTL=====
	// NCTL done for driver
	//odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x1); //wire r_iqk_IO_RFC_en
	//odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00);//0x2D9C [7:0]=0x0
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x403E);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RF_0x00 = 0x%x\n", odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFFF));
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003F, 0x09);
	halrf_delay_10us(1);
	//if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x0) {
	RF_DBG(dm, DBG_RF_IQK, "[IQK]START : NB TXIQK NCTL(one shot)!\n");
	odm_set_bb_reg(dm, R_0x1b00, 0x00001FFF, IQK_CMD);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]1b00 = 0x%x!\n", odm_get_bb_reg(dm, R_0x1b00, MASKDWORD));
	odm_set_bb_reg(dm, 0x1b00, BIT(0), 0x1);//one shot
	ODM_delay_ms(10);

	while (i < 10) {
		i++;
		ODM_delay_ms(1);
		if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x55) {
			RF_DBG(dm, DBG_RF_IQK, "[IQK]NCTL FRxK done, delaytime = %d ms!\n", i);
			halrf_delay_10us(1);
			odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00);//0x2D9C [7:0]=0x0
			break;
		} else {
			RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
			continue;
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]2d9c[7:0] = 0x%x!\n", odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF));
	}
	RF_DBG(dm, DBG_RF_IQK, "[IQK]END : NB TXIQK NCTL!\n");

	RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK 1b08[26]= 0x%x!\n", odm_get_bb_reg(dm, 0x1b08, BIT(26)));
	KFAIL = (u8)odm_get_bb_reg(dm, 0x1b08, BIT(26));
	RF_DBG(dm, DBG_RF_IQK, "[IQK]TXIQK %s!\n", (KFAIL == 0 ? "success" : "fail"));
	//ODM_delay_us(10);
	odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00);//0x2D9C [7:0]=0x0

	RF_DBG(dm, DBG_RF_IQK, "[IQK]BBreg_1b38= 0x%x\n", odm_get_bb_reg(dm, 0x1b38, MASKDWORD));

	if (!KFAIL) {
		iqk_info->txxy[0][0] = odm_get_bb_reg(dm, R_0x1b38, 0x7FF00000);
		iqk_info->txxy[0][1] = odm_get_bb_reg(dm, R_0x1b38, 0x0007FF00);
		iqk_info->iqk_fail_report[0][0][0] = true;
	} else {
		iqk_info->iqk_fail_report[0][0][0] = false; //[TX][path][tx]
		odm_set_bb_reg(dm, 0x2924, BIT(8), 0x0); //disable tx_FDF
	}

	/*} else {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]Jump : NB TXIQK NCTL not ready!\n");
	}*/
#if IQK_8730A_SHOW
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xF0000, 0x2);
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x56, 0x01C00, 0x1);
	odm_set_bb_reg(dm, R_0x1bb8, BIT(20), 0x1);
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003F, 0x09);
	odm_set_bb_reg(dm, 0x1bd4, MASKDWORD, 0x00350001);
#endif
}

void _iqk_rxk_by_path_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 KFAIL = 1;
	u8 i = 0;
	u32 rx_pi_data, reg_1b3c;
	u32 rf_reg1F;
	u16 reg1b00 = 0x518;

	//++++++ lna small ++++++++++++++++++++
	if (iqk_info->iqk_band == 0) { //G
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0x03FF0, 0x180);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x83, 0x00300, 0x0);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x83, 0x1FC00, 0x14);

		odm_set_rf_reg(dm, RF_PATH_A, 0x87, 0xFFFFF, 0x54886);
	} else { //A
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0x03FF0, 0x186);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x8c, 0x00180, 0x1);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x8c, 0x0007F, 0x27);

		odm_set_rf_reg(dm, RF_PATH_A, 0x94, 0xFFFFF, 0x00070);
		odm_set_rf_reg(dm, RF_PATH_A, 0x89, 0xFFFFF, 0x000C4);
		odm_set_rf_reg(dm, RF_PATH_A, 0x8A, 0xFFFFF, 0x5FC9F);
	}

	// [19:0]=RX_PI_DATA,[19:16]=RF_Mode,[13:11]=LNA, [10]=TIA, [9:5]=RxBB
	// RX_PI_DATA[19:0] is same as RF0x00
	rx_pi_data = odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFFF);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]RX_PI_DATA = 0x%x\n", rx_pi_data);
	odm_set_bb_reg(dm, R_0x1b24, 0x000FFFFF, rx_pi_data);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]1b24= 0x%x\n",
	       odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
	// ====START : NCTL=====
	odm_set_bb_reg(dm, 0x1b10, 0x000000FF, 0x00);
	odm_set_bb_reg(dm, 0x2908, 0x0000FFFF, 0x8083);//lna=0x3
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x1);//wire r_iqk_IO_RFC_en
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003F, 0x3f);
	odm_set_bb_reg(dm, 0x1b2c, 0x0FFF0000, 0x044);// Rx_tone_idx(4.25MHz)

	if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x0) {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]START : NB TXIQK NCTL(one shot)!\n");
		//set cal_path, process id
		odm_set_bb_reg(dm, R_0x1b00, 0x00001FFF, reg1b00);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]reg 1b00 = 0x%x!\n",
		       odm_get_bb_reg(dm, R_0x1b00, MASKDWORD));
		odm_set_bb_reg(dm, R_0x1b00, BIT(0), 0x1);//one shot
		//ODM_delay_us(1000);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]one shot delay 1ms!\n");
		while (i < 10) {
			i++;
			ODM_delay_ms(1);
			//ODM_delay_us(1000);
			if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x55) {
				//ODM_delay_us(10);
				//odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00);//0x2D9C [7:0]=0x0
				RF_DBG(dm, DBG_RF_IQK, "[IQK]NCTL FRxK done, delaytime = %d ms!\n", i);
				break;
			}
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]END : NB RXIQK NCTL!\n");
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK 1b08[26]= 0x%x!\n", odm_get_bb_reg(dm, 0x1b08, BIT(26)));
		KFAIL = (u8)odm_get_bb_reg(dm, 0x1b08, BIT(26));
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK small LNA %s!\n", (KFAIL == 0 ? "success" : "fail"));
		reg_1b3c = odm_get_bb_reg(dm, 0x1b3c, MASKDWORD);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]small LNA RXIQK, BBreg_1b3c= 0x%x\n", reg_1b3c);
		//1b3c auto write!
		if (!KFAIL) {
			iqk_info->rxxy[0][0] = (reg_1b3c & 0x7FF00000) >> 20;  //rx_x
			iqk_info->rxxy[1][0] = (reg_1b3c & 0x0007FF00) >> 8 ;  //rx_y
			iqk_info->iqk_fail_report[1][0][0] = true;
		} else {
			iqk_info->iqk_fail_report[1][0][0] = false; //[RX][path][rxs]
			odm_set_bb_reg(dm, 0x2924, BIT(8), 0x0); //disable tx_FDF
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]rxxy[0][0][0]= 0x%x\n", iqk_info->rxxy[0][0]);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]rxxy[0][1][0]= 0x%x\n", iqk_info->rxxy[1][0]);
	} else {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]Jump : NB RXIQK NCTL not ready!\n");
	}
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0);//wire r_iqk_IO_RFC_en
	//++++++ lna large ++++++++++++++++++++
	if (iqk_info->iqk_band == 0) { //G
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x7348);

		odm_set_rf_reg(dm, RF_PATH_A, RF_0x83, 0x00300, 0x2);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x83, 0x1FC00, 0x14);

		rx_pi_data = odm_get_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFFF);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RX_PI_DATA = 0x%x\n", rx_pi_data);
		odm_set_bb_reg(dm, R_0x1b24, 0x000FFFFF, rx_pi_data);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]1b24= 0x%x\n",
		       odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
	} else { //A
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x00, 0xFFFF0, 0x7348);

		odm_set_rf_reg(dm, RF_PATH_A, RF_0x8c, 0x00180, 0x1);
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x8c, 0x0007F, 0x17);
	}
	// ====START : NCTL=====
	odm_set_bb_reg(dm, 0x1b10, 0x000000FF, 0x00);
	odm_set_bb_reg(dm, 0x2908, 0x0000FFFF, 0x8086);//lna=0x6
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x1);//wire r_iqk_IO_RFC_en
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003F, 0x3f);
	odm_set_bb_reg(dm, 0x1b2c, 0x0FFF0000, 0x044);// Rx_tone_idx(4.25MHz)

	if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x0) {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]START : NB RXIQK NCTL(one shot)!\n");
		odm_set_bb_reg(dm, 0x1b00, 0x00001FFF, reg1b00);//set cal_path, process id
		odm_set_bb_reg(dm, 0x1b00, BIT(0), 0x1);//one shot
		//ODM_delay_us(1000);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]one shot delay 1ms!\n");
		while (i < 10) {
			i++;
			ODM_delay_ms(1);
			if (odm_get_bb_reg(dm, R_0x2d9c, 0x000000FF) == 0x55) {
				//ODM_delay_us(10);
				//odm_set_bb_reg(dm, R_0x1b10, 0x000000FF, 0x00);//0x2D9C [7:0]=0x0
				RF_DBG(dm, DBG_RF_IQK, "[IQK]NCTL FRxK done, delaytime = %d ms!\n", i);
				break;
			}
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]END : NB RXIQK NCTL!\n");
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK 1b08[26]= 0x%x!\n", odm_get_bb_reg(dm, 0x1b08, BIT(26)));
		KFAIL = (u8)odm_get_bb_reg(dm, 0x1b08, BIT(26));
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK large LNA %s!\n", (KFAIL == 0 ? "success" : "fail"));
		reg_1b3c = odm_get_bb_reg(dm, 0x1b3c, MASKDWORD);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]large LNA RXIQK,BBreg_1b3c= 0x%x\n", reg_1b3c);
		//1b3c auto write!
		if (!KFAIL) {
			iqk_info->rxxy[0][1] = (reg_1b3c & 0x7FF00000) >> 20;  //rx_x
			iqk_info->rxxy[1][1] = (reg_1b3c & 0x0007FF00) >> 8 ;  //rx_y
			iqk_info->iqk_fail_report[1][0][1] = true; //[RX][path][rxl]
		} else {
			iqk_info->iqk_fail_report[1][0][1] = false; //[RX][path][rxl]
			odm_set_bb_reg(dm, 0x2924, BIT(8), 0x0); //disable tx_FDF
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]rxxy[0][0][1]= 0x%x\n", iqk_info->rxxy[0][1]);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]rxxy[0][1][1]= 0x%x\n", iqk_info->rxxy[1][1]);
	} else {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]Jump : NB RXIQK NCTL not ready!\n");
	}
	odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0);//wire r_iqk_IO_RFC_en
	// =====Disable RXIQKPLL =====
	if (iqk_info->iqk_band == 0) { //G
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x20, BIT(8), 0x0);
	} else { //A
		odm_set_rf_reg(dm, RF_PATH_A, RF_0x20, BIT(7), 0x0);
	}
	//1E[19]=POW_IQKPLL
	odm_set_rf_reg(dm, RF_PATH_A, RF_0x1e, BIT(19), 0x0);
	//odm_set_bb_reg(dm, R_0x1860, BIT(30), 0x1); // DAC on
}

void _iqk_iqk_by_step_8730a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 counter = 0x0;

	RF_DBG(dm, DBG_RF_IQK, "iqk_info->iqk_step = %d\n", iqk_info->iqk_step);

	switch (iqk_info->iqk_step) {
	case 0: /*S0 LOK*/
		_iqk_txk_rf_setting_8730a(dm);
		_iqk_lok_by_path_8730a(dm);
		iqk_info->iqk_step++;
		break;
	case 1:	/*S0 TXIQK*/
		_iqk_txk_by_path_8730a(dm);
		iqk_info->iqk_step++;
		break;
#if IQK_8730A_TONE
	case 2: /*S0 RXIQK*/
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXIQK!\n");
		_iqk_rxk_rf_setting_8730a(dm);
		_iqk_rxk_by_path_8730a(dm);
		iqk_info->iqk_step++;
		break;
#endif
	default:
		iqk_info->iqk_step = IQK_STEP_8730A;
		break;
	}

	return;
}

void _iqk_start_iqk_8730a(struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 i = 0;
	u32 mode = 3;

	/*backup RF mode*/
	mode = odm_get_rf_reg(dm, RF_PATH_A, 0x0, 0xF0000);

	RF_DBG(dm, DBG_RF_IQK, "[IQK]backup rf0[19:16] = 0x%x\n", mode);

	_iqk_preset_8730a(dm, true);

	btc_set_gnt_wl_bt_8730a(dm, true);

	while (i < 10) {
		_iqk_iqk_by_step_8730a(dm);
		if (iqk_info->iqk_step == IQK_STEP_8730A) {
			break;
		}
		i++;
	}

	btc_set_gnt_wl_bt_8730a(dm, false);
#if IQK_8730A_TONE
	_iqk_preset_8730a(dm, false);

	/* restore RF mode*/
	odm_set_rf_reg(dm, RF_PATH_A, 0x0, 0xF0000, mode);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]restore pathA rf0[19:16] = 0x%x\n",
	       odm_get_rf_reg(dm, RF_PATH_A, 0x0, 0xF0000));
#endif
}

void _iq_calibrate_8730a_init(struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 i, j, k, m;
	static boolean firstrun = true;

	if (firstrun) {
		//odm_read_and_config_mp_8730a_cal_init(dm);
		//firstrun = false;
		RF_DBG(dm, DBG_RF_IQK, "[IQK]=====>PHY_IQCalibrate_8730A_Init\n");

		for (i = 0; i < RF_PATH_MAX_8730A; i++) {
			for (j = 0; j < 2; j++) {
				iqk_info->lok_fail[i] = true;
				iqk_info->iqk_fail[j][i] = true;

				//iqk_info->iqc_matrix[j][i] = 0x20000000;
			}
			iqk_info->txxy[i][0] = 0x200;
			iqk_info->txxy[i][1] = 0x000;
			iqk_info->rxxy[0][0] = 0x200;
			iqk_info->rxxy[1][0] = 0x000;
			iqk_info->rxxy[0][1] = 0x200;
			iqk_info->rxxy[1][1] = 0x000;
		}

		for (i = 0; i < 2; i++) { //i:band,j:path
			iqk_info->iqk_channel[i] = 0x0;

			for (j = 0; j < RF_PATH_MAX_8730A; j++) {
				iqk_info->lok_idac[i][j] = 0x0;
				iqk_info->rxiqk_agc[i][j] = 0x0;
				iqk_info->bypass_iqk[i][j] = 0x0;

				for (k = 0; k < 2; k++) {
					iqk_info->iqk_fail_report[i][j][k] = true;//i:TX/RX,j:path,k:Rxs/RXl
				}

				for (k = 0; k < 2; k++) {//i:band,j:path,k=tx/rx
					iqk_info->retry_count[i][j][k] = 0x0;
				}
			}
		}
	}

}

void _phy_iq_calibrate_8730a(struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 MAC_backup[MAC_REG_NUM_8730A], BB_backup[BB_REG_NUM_8730A], RF_backup[RF_REG_NUM_8730A][RF_PATH_MAX_8730A];
	u32 backup_mac_reg[MAC_REG_NUM_8730A] = {0x520};
	u32 backup_bb_reg[BB_REG_NUM_8730A] = {0x09f0, 0x09b4, 0x1c38, 0x1860, 0x1cd0, 0x824, 0x2a24, 0x1d40, 0x1c20, 0x1880, 0x180c, 0x1b1c}; //?not sure
	u32 backup_rf_reg[RF_REG_NUM_8730A] = {0x5, 0xde, 0xdf, 0xef, 0x1f, 0x94, 0x89, 0x8A, 0x87};//0x0, 0x8f
	u32 temp0;
	boolean is_mp = false;
	boolean Kpass = true;
	u8 i = 0, j = 0, k = 0;
	u8 ab = 0;

	if (*dm->mp_mode) {
		is_mp = true;
	}

	RF_DBG(dm, DBG_RF_IQK, "[IQK]==========IQK strat!!!!!==========\n");
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]Interface = %d, Cv = %x\n", dm->support_interface, dm->cut_version);
	RF_DBG(dm, DBG_RF_IQK, "[IQK] Test chip V01\n");

	iqk_info->iqk_times++;
	iqk_info->kcount = 0;
	_iqk_information_8730a(dm);
	_iqk_backup_mac_bb_8730a(dm, MAC_backup, BB_backup, backup_mac_reg, backup_bb_reg);
	_iqk_backup_rf_8730a(dm, RF_backup, backup_rf_reg);
#if IQK_8730A_TONE
	while (i < 3) {
		i++;
		dm->n_iqk_cnt++;
		iqk_info->kcount = i;
#endif
		iqk_info->iqk_step = 0;
		RF_DBG(dm, DBG_RF_IQK, "[IQK]Kcount = %d\n", iqk_info->kcount);

		_iqk_afe_setting_8730a(dm, true);
		_iqk_start_iqk_8730a(dm);
#if IQK_8730A_TONE
		_iqk_afe_setting_8730a(dm, false);

		halrf_delay_10us(1);

		Kpass = ((iqk_info->iqk_fail_report[0][0][0]) &
			 (iqk_info->iqk_fail_report[1][0][0]) &
			 (iqk_info->iqk_fail_report[1][0][1]));

		RF_DBG(dm, DBG_RF_IQK, "[IQK]Kpass = %s\n", (Kpass == true) ? "true" : "false");
		if (Kpass == true) {
			dm->n_iqk_ok_cnt++;
			RF_DBG(dm, DBG_RF_IQK, "[IQK]Kpass! break!\n");
			break;
		} else {
			RF_DBG(dm, DBG_RF_IQK, "[IQK]Kfail! fill old IQK value!\n");
			_iqk_fill_iqk_xy_8730a(dm);
			dm->n_iqk_fail_cnt++;
			//RF_DBG(dm, DBG_RF_IQK, "[IQK]Kfail! break!\n");
			//break;
		}


	}

	_iqk_restore_rf_8730a(dm, backup_rf_reg, RF_backup);
	_iqk_restore_mac_bb_8730a(dm, MAC_backup, BB_backup, backup_mac_reg, backup_bb_reg);
#endif
	odm_set_bb_reg(dm, R_0x180c, BIT(31), 0x1);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]==========IQK end!!!!!==========\n");
}

/*IQK_version:0x1, NCTL:0x1*/
/*1.max tx pause while IQK*/
/*2.CCK off while IQK*/
void phy_iq_calibrate_8730a(void *dm_void, boolean is_recovery)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	//return;

	dm->rf_calibrate_info.is_iqk_in_progress = true;
	/*FW IQK*/
	//odm_set_bb_reg(dm, R_0x180c, BIT(31), 0x0);

	_iq_calibrate_8730a_init(dm);

	_phy_iq_calibrate_8730a(dm);
	//odm_set_bb_reg(dm, R_0x180c, BIT(31), 0x1);

	dm->rf_calibrate_info.is_iqk_in_progress = false;
}

#endif
