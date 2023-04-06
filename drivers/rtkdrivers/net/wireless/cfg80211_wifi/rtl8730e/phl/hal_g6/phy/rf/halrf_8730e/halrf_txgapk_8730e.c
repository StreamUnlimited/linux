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

void _txgapk_backup_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)
{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		reg_backup[i] = halrf_rreg(rf, reg[i], MASKDWORD);

		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Backup BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);*/
	}
}

void _txgapk_reload_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)

{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Reload BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);*/
	}
}

void _halrf_txgapk_bkup_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 *rf_bkup,
	u32 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		rf_bkup[i] = halrf_rrf(rf, RF_PATH_A, rf_reg[i], MASKRF);

		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Backup RF S%d 0x%x = %x\n",
			path, rf_reg[i], rf_bkup[i]);*/
	}
}

void _halrf_txgapk_reload_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 *rf_bkup,
	u32 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wrf(rf, RF_PATH_A, rf_reg[i], MASKRF, rf_bkup[i]);

		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Reload RF S%d 0x%x = %x\n",
			path, rf_reg[i], rf_bkup[i]);*/
	}
}


void _txgapk_afe_setting_8730e(struct rf_info *rf,
			       bool do_txgapk)
{
	if (do_txgapk) {
		/*01 AFE 0N BB setting*/
		//--r_gothrough_tx_iqkdpk--//
		halrf_wreg(rf, 0x5864, BIT(27), 0x1);
		halrf_wreg(rf, 0x5864, BIT(28), 0x1);
		//Tx-chain clk force on//
		halrf_wreg(rf, 0x2008, 0x01ffffff, 0x00fffff);
		//ADDA fifo rst //
		halrf_wreg(rf, 0x20fc, BIT(16), 0x1);
		halrf_wreg(rf, 0x20fc, BIT(20), 0x0);
		halrf_wreg(rf, 0x20fc, BIT(24), 0x1);
		halrf_wreg(rf, 0x20fc, BIT(28), 0x0);
		//BB for IQK DBG mode //
		halrf_wreg(rf, 0x5670, MASKDWORD, 0xf801fffd);
		//--CLK rate, DA 960M--//
		halrf_wreg(rf, 0x12a0, BIT(15), 0x1);
		halrf_wreg(rf, 0x5670, BIT(31), 0x1);
		halrf_wreg(rf, 0x12a0, 0x00007000, 0x7);
		//--DAC clk rate,AD 160M--//
		halrf_wreg(rf, 0x12a0, BIT(19), 0x1);
		halrf_wreg(rf, 0x12a0, 0x00070000, 0x6);
		halrf_wreg(rf, 0x5670, 0x60000000, 0x1);
		//--DAC ON AD ON--//
		halrf_wreg(rf, 0x030c, MASKBYTE3, 0x0f);
		halrf_wreg(rf, 0x032c, MASKHWORD, 0x0001);
		halrf_wreg(rf, 0x12b8, BIT(28), 0x1);
		// ADDA fifo rst//
		halrf_wreg(rf, 0x20fc, BIT(20), 0x1);
		halrf_wreg(rf, 0x20fc, BIT(28), 0x1);


		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] BB setting for AFE ON\n");*/
	} else {
		halrf_wrf(rf, 0, 0xEE, BIT(19), 0x0);
		halrf_wrf(rf, 0, 0x5, BIT(1), 0x0);
		halrf_wrf(rf, 0, 0xEE, BIT(18), 0x0);
		halrf_wrf(rf, 0, 0xDB, BIT(1), 0x0);
		halrf_wrf(rf, 0, 0x5, BIT(0), 0x1);
		/*dpk&iqk restore*/
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8108, MASKDWORD, 0x0A008083);
		halrf_wreg(rf, 0x80b8, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x80cc, 0x0000003f, 0x00000000);
		halrf_wreg(rf, 0x8120, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8138, MASKDWORD, 0x00000000);
		/*AFE restore*/
		halrf_wreg(rf, 0x20fc, BIT(16), 0x1);
		halrf_wreg(rf, 0x20fc, BIT(20), 0x0);
		halrf_wreg(rf, 0x20fc, BIT(24), 0x1);
		halrf_wreg(rf, 0x20fc, BIT(28), 0x0);
		halrf_wreg(rf, 0x5670, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x12a0, 0x000ff000, 0x0);
		//halrf_wreg(rf, 0x12b8, BIT(28), 0);
		halrf_wreg(rf, 0x20fc, BIT(16), 0x0);
		halrf_wreg(rf, 0x20fc, BIT(24), 0x0);
		/*RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Restore BB setting for AFE OFF\n");*/
	}
}

void _txgapk_rf_setting_8730e(struct rf_info *rf)
{
	halrf_wrf(rf, RF_PATH_A, 0x5, BIT(0), 0x0);
	halrf_wreg(rf, 0x8000, BIT(3), 0x1); //
	halrf_wreg(rf, 0x8008, BIT(7), 0x1); //
	// set RF mode into DPK mode//
	halrf_wrf(rf, RF_PATH_A, 0x0, 0x000FFFF0, 0x503e);
	// ==== set Att : TXG 83[7:4] ==== //
	halrf_wrf(rf, RF_PATH_A, 0x83, 0x000000f0, 0x8);
	// ==== set PGA2 : DPK gain 79[12]=1, 79[15:13] ==== //
	halrf_wrf(rf, RF_PATH_A, 0x79, BIT(12), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x79, 0x0000e000, 0x7);
	/*RF_DBG(rf, DBG_RF_TXGAPK, "0x5= 0x%x, 0x0 = 0x%x\n",
		halrf_rrf(rf, RF_PATH_A, 0x5, MASKRF), halrf_rrf(rf, RF_PATH_A, 0x0, MASKRF));*/

}
#if 0
bool _txgapk_nctl_done_8730e(struct rf_info *rf)
{
	u8 reg_2d9c = 0;
	u16 count = 0;

	/*Driver waits NCTL sync done*/
	reg_2d9c = halrf_rreg(rf, 0x2d9c, MASKBYTE0);

	while (reg_2d9c != 0x55 && count < 1000) {
		halrf_delay_us(rf, 10);
		reg_2d9c = halrf_rreg(rf, 0x2d9c, MASKBYTE0);
		count++;
	}
	RF_DBG(rf, DBG_RF_TXGAPK,
	       "[TXGAPK] count =0x%x\n", count);

	if (reg_2d9c == 0x55) {
		return true;
	} else {
		return false;
	}
}
#endif
bool _txgapk_nctl_done_8730e(struct rf_info *rf)
{
	bool notready = true, fail = true;
	u32 delay_count = 0;
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
			RF_DBG(rf, DBG_RF_TXGAPK, "[gapk]gapk timeout!!!\n");
			break;
		}
	}

	if (!notready) {
		fail = (bool)halrf_rreg(rf, 0x8008, BIT(26));
	}
	/*RF_DBG(rf, DBG_RF_TXGAPK, "[gapk] 0xbff8= 0x%x\n", halrf_rreg(rf, 0xbff8, MASKDWORD));*/

	halrf_wreg(rf, 0x8010, MASKBYTE0, 0x0);
	RF_DBG(rf, DBG_RF_TXGAPK, "[gapk]cnt= %d\n", delay_count);
	RF_DBG(rf, DBG_RF_TXGAPK, "[gapk]fail = 0x%x \n", fail);

	return fail;
}


void _txgapk_one_shot_8730e(struct rf_info *rf, u8 sel)
{
	u32 p_id;
	bool k_fail;

	if (halrf_rreg(rf, 0xbff8, MASKBYTE0) != 0) {
		halrf_wreg(rf, 0x8010, 0x000000ff, 0x0);
		halrf_delay_us(rf, 100);
		/*RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK] 0xbff8 = 0x%x\n", halrf_rreg(rf, 0xbff8,MASKBYTE0));*/
	}

	p_id = 0x00000c19 | (sel << 8);
	halrf_wreg(rf, 0x8000, MASKDWORD, p_id);
	/*RF_DBG(rf, DBG_RF_TXGAPK,
		 "[TXGAPK] 0x8000 = 0x%x\n", halrf_rreg(rf, 0x8000,MASKDWORD));*/

	k_fail = _txgapk_nctl_done_8730e(rf);

	halrf_wreg(rf, 0x8010, 0x000000FF, 0x0);

	RF_DBG(rf, DBG_RF_TXGAPK,
	       "[TXGAPK] p_id = 0x%x, k_fail = %d\n", p_id, k_fail);
}
void _txgapk_dbg_krpt_8730e(struct rf_info *rf)
{
	u8 i, diff[11];
	/*Read Dout Results: D=s7.4*/
	/*46: report = gapk_rpt*/
	halrf_wreg(rf, 0x80d4, 0x003f0000, 0x2e);
	halrf_wreg(rf, 0x80f4, 0x00000f00, 0x3);

	for (i = 0; i < 4; i++) {
		diff[i] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ D[%d]=0x%x\n", i, diff[i]);
	}

	halrf_wreg(rf, 0x80f4, 0x00000f00, 0x4);
	for (i = 0; i < 4; i++) {
		diff[i + 4] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ D[%d]=0x%x\n",
		       i + 4, diff[i + 4]);
	}

	halrf_wreg(rf, 0x80f4, 0x00000F00, 0x5);
	for (i = 0; i < 3; i++) {
		diff[i + 8] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ D[%d]=0x%x\n",
		       i + 8, diff[i + 8]);
	}

	/*Read Tout Results: Ta=s7.4*/
	halrf_wreg(rf, 0x80f4, 0x00000f00, 0x9);
	for (i = 0; i < 4; i++) {
		diff[i] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ Ta[%d]=0x%x\n", i, diff[i]);
	}

	halrf_wreg(rf, 0x80f4, 0x00000f00, 0xa);
	for (i = 0; i < 4; i++) {
		diff[i + 4] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ Ta[%d]=0x%x\n",
		       i + 4, diff[i + 4]);
	}

	halrf_wreg(rf, 0x80f4, 0x00000f00, 0xb);
	for (i = 0; i < 3; i++) {
		diff[i + 8] = (u8)halrf_rreg(rf, 0x80fc, 0x7f << (i * 7));
		RF_DBG(rf, DBG_RF_TXGAPK,
		       "[TXGAPK][DBG] READ Ta[%d]=0x%x\n",
		       i + 8, diff[i + 8]);
	}

}
void _txgapk_enable_gapk_8730e(struct rf_info *rf, u8 band, u8 table_sel)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;

	if (band == 0x0) { /*2G*/
		halrf_wreg(rf, 0x815c, MASKDWORD, 0x433d3731);
		halrf_wreg(rf, 0x8160, MASKDWORD, 0x615b5549);
		halrf_wreg(rf, 0x8164, MASKDWORD, 0x21616161);
		halrf_wreg(rf, 0x8168, MASKDWORD, 0x302d2a24);
		halrf_wreg(rf, 0x816c, MASKDWORD, 0x30303030);
		halrf_wreg(rf, 0x8170, MASKDWORD, 0x00003030);
	} else {
		if (rf->hal_com->cv == CAV) {
		halrf_wreg(rf, 0x815c, MASKDWORD, 0x433d312b);
		halrf_wreg(rf, 0x8160, MASKDWORD, 0x67554F49);
		halrf_wreg(rf, 0x8164, MASKDWORD, 0x216d6d6d);
		halrf_wreg(rf, 0x8168, MASKDWORD, 0x302A2724);
		halrf_wreg(rf, 0x816c, MASKDWORD, 0x33333333);
		halrf_wreg(rf, 0x8170, MASKDWORD, 0x00003333);
		if (channel >= 149 && channel <= 177) {
			halrf_wreg(rf, 0x815c, MASKDWORD, 0x433d312b);
			halrf_wreg(rf, 0x8160, MASKDWORD, 0x6D674F49);
			halrf_wreg(rf, 0x8164, MASKDWORD, 0x216d6d6D);
			}
		} else {
			halrf_wreg(rf, 0x815c, MASKDWORD, 0x433d2b25);
			halrf_wreg(rf, 0x8160, MASKDWORD, 0x5b554F49);
			halrf_wreg(rf, 0x8164, MASKDWORD, 0x216d6d67);
			halrf_wreg(rf, 0x8168, MASKDWORD, 0x302A2724);
			halrf_wreg(rf, 0x816c, MASKDWORD, 0x33333333);
			halrf_wreg(rf, 0x8170, MASKDWORD, 0x00003333);
			if (channel >= 149 && channel <= 177) {
				halrf_wreg(rf, 0x815c, MASKDWORD, 0x433d2b25);
				halrf_wreg(rf, 0x8160, MASKDWORD, 0x675b5549);
				halrf_wreg(rf, 0x8164, MASKDWORD, 0x216d6d6d);
			}
		}
	}

	switch (table_sel) {
	case GAPK_TRK_K_8730E:
		if (band == 0x0) { /*2G*/
			halrf_wreg(rf, 0x8170, 0x03ff0000, 0x06D); //gapk_on_table0_setting
		} else {
			if (rf->hal_com->cv == CAV) {
				halrf_wreg(rf, 0x8170, 0x03ff0000, 0xBD); //gapk_on_table0_setting
				if (channel >= 149 && channel <= 177) {
					halrf_wreg(rf, 0x8170, 0x03ff0000, 0x5D);
				}
			} else {
				halrf_wreg(rf, 0x8170, 0x03ff0000, 0x16D); //gapk_on_table0_setting
				if (channel >= 149 && channel <= 177) {
					halrf_wreg(rf, 0x8170, 0x03ff0000, 0xAD);
				}
			}
		}

		break;

	case GAPK_PWR_K_8730E:
		if (band == 0x0) { /*2G*/
			halrf_wreg(rf, 0x817c, 0x000003ff, 0x00D); //gapk_on_table0_setting

		} else {
			if (rf->hal_com->cv == CAV) {
				halrf_wreg(rf, 0x817c, 0x000003ff, 0x015);    //gapk_on_table0_setting
			} else {
				halrf_wreg(rf, 0x817c, 0x000003ff, 0x015);    //gapk_on_table0_setting
			}
		}
		break;
	default:
		break;
	}

}

void _halrf_txgapk_track_table_nctl_8730e(struct rf_info *rf)
{
	//struct halrf_gapk_info *txgapk_info = &rf->gapk;

	//halrf_wreg(rf,0x8170, 0x03ff0000, 0x155);//gapk_on_table0_setting

	//	========START : Do Track GapK =====
	halrf_wreg(rf, 0x80e4, 0x00003f00, 0x10); //set PA_GAPK_INDEX[5:0] when PAD GapK
	halrf_wreg(rf, 0x80f0, 0x000001f0, 0x04); //gain_idx_bnd
	halrf_wreg(rf, 0x8030, 0x0000003f, 0x24);//2nd ItQt=100_100
	halrf_wreg(rf, 0x80f0, 0x0000f000, 0x0); //select table0
	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	//[0]=Psd_Gapk_en
	//[1]=gapk_type, 1'b0 : PAD GapK , 1'b1 : PA GapK
	halrf_wreg(rf, 0x80f0, 0x00000003, 0x1);
	// process id=5'h0c	; Track_gapk
	halrf_wreg(rf, 0x80cc, 0x0000003f, 0x12); // ItQt
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x24); //BB@ABP2 = +2.25MHz,[27:16]=rx_tone_idx

	_txgapk_one_shot_8730e(rf, GAPK_TRK_K_8730E);

	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);

	//  ========END : Do Track GapK =====

	// ====== START : Ta to RFC Table =====
	halrf_wrf(rf, RF_PATH_A, 0x1005f, BIT(9), 0x1);// TX_GAPK_EN_CAL_TABLE
	halrf_wrf(rf, RF_PATH_A, 0x10018, BIT(19), 0x0);// CAL_TABLE_SEL
	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	halrf_wreg(rf, 0x80f0, BIT(22), 0x1);
	halrf_wreg(rf, 0x80f0, 0x00000003, 0x1);
	//oneshot write TA to RFC
	_txgapk_one_shot_8730e(rf, GAPK_TRK_WTA_8730E);
	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	halrf_wreg(rf, 0x80f0, BIT(22), 0x0);
	/*RF_DBG(rf, DBG_RF_TXGAPK, "0x5670= 0x%x, 0x80f0 = 0x%x\n",
		halrf_rreg(rf, 0x5670, MASKDWORD), halrf_rreg(rf, 0x80f0, MASKDWORD));*/
	// ====== END : Ta to RFC Table =====
#if (GAPK_RPT_DBG_8730E)
	_txgapk_dbg_krpt_8730e(rf);
#endif

	/*RF_DBG(rf, DBG_RF_TXGAPK,
		   "[TXGAPK]  ========END : Do Track GapK =====\n");*/


}

void _halrf_txgapk_power_table_nctl_8730e(struct rf_info *rf)
{
	//struct halrf_gapk_info *txgapk_info = &rf->gapk;

	//halrf_wreg(rf,0x817c, 0x000003ff, 0x005);//gapk_on_table0_setting

	//	========START : Do Track GapK =====
	//halrf_wreg(rf,0x80e4, 0x00003f00, 0x24);//set PA_GAPK_INDEX[5:0] when PAD GapK
	halrf_wreg(rf, 0x80f0, 0x000001f0, 0x01); //gain_idx_bnd
	halrf_wreg(rf, 0x8030, 0x0000003f, 0x24);//2nd ItQt=100_100
	halrf_wreg(rf, 0x80f0, 0x0000f000, 0x0); //select table0
	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	//[0]=Psd_Gapk_en
	//[1]=gapk_type, 1'b0 : PAD GapK , 1'b1 : PA GapK
	halrf_wreg(rf, 0x80f0, 0x00000003, 0x3);
	// process id=5'h0c ; Track_gapk
	halrf_wreg(rf, 0x80cc, 0x0000003f, 0x12); // ItQt
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x024); //BB@ABP2 = +2.25MHz,[27:16]=rx_tone_idx

	_txgapk_one_shot_8730e(rf, GAPK_PWR_K_8730E);

	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);

	//	========END : Do Track GapK =====

	// ====== START : Ta to RFC Table =====

	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) | BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x1);
	halrf_wreg(rf, 0x80f0, BIT(22), 0x1);
	halrf_wreg(rf, 0x80f0, 0x00000003, 0x3);
	//oneshot write TA to RFC
	_txgapk_one_shot_8730e(rf, GAPK_PWR_WTA_8730E);
	//SW_SI
	halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
	//wire r_iqk_IO_RFC_en
	halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	halrf_wreg(rf, 0x80f0, BIT(22), 0x0);
	// ====== END : Ta to RFC Table =====
#if (GAPK_RPT_DBG_8730E)
	_txgapk_dbg_krpt_8730e(rf);
#endif

	/*RF_DBG(rf, DBG_RF_TXGAPK,
		"[TXGAPK]  ========END : Do Power GapK =====\n");*/

}



void _txgapk_config_offset_table_8730e(struct rf_info *rf)
{
	u32 reg_rf18;
	u8  band;

	reg_rf18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
	band = (u8)((reg_rf18 & BIT(16)) >> 16); /*0/1:G/A*/
	/*RF_DBG(rf, DBG_RF_TXGAPK, "band= 0x%x, reg_rf18 = 0x%x\n",
		band, reg_rf18);*/


	_txgapk_rf_setting_8730e(rf);

#if GAPK_TRACK_EN_8730E
	_txgapk_enable_gapk_8730e(rf, band, GAPK_TRK_K_8730E);
	_halrf_txgapk_track_table_nctl_8730e(rf);
#endif

#ifdef GAPK_POWER_EN_8730E
	_txgapk_enable_gapk_8730e(rf, band, GAPK_PWR_K_8730E);
	_halrf_txgapk_power_table_nctl_8730e(rf);
#endif


}


void halrf_do_txgapk_8730e(struct rf_info *rf)
{
	u32 bb_reg[2] = {0x8030, 0x8034};
	u32 bb_reg_backup[2] = {0};
	u32 backup_num = 2;
	u32 rf_bkup[4] = {0};
	u32 rf_reg[4] = {0x0, 0x83, 0x79, 0xdf};
	u32 rf_backup_num = 4;


	_txgapk_backup_bb_registers_8730e(rf, bb_reg, bb_reg_backup,
					  backup_num);
	_halrf_txgapk_bkup_rf_8730e(rf, rf_reg, rf_bkup, rf_backup_num);
	_txgapk_afe_setting_8730e(rf, true);
	_txgapk_config_offset_table_8730e(rf);
	_txgapk_afe_setting_8730e(rf, false);
	halrf_wreg(rf, 0x8120, 0xc0000000, 0x0);
	halrf_wrf(rf, RF_PATH_A, 0x5, BIT(0), 0x1);
	//halrf_wreg(rf, 0x8000, BIT(3), 0x0);
	halrf_wreg(rf, 0x8008, BIT(7), 0x0);
	_halrf_txgapk_reload_rf_8730e(rf, rf_reg, rf_bkup, rf_backup_num);
	_txgapk_reload_bb_registers_8730e(rf, bb_reg, bb_reg_backup,
					  backup_num);
	//halrf_wreg(rf, 0xE014, BIT(0), 0x1);
	//halrf_wreg(rf, 0x5670, BIT(1), 0x0);
	//halrf_wmac32(rf, 0x0004, halrf_rmac32(rf, 0x0004) & ~ BIT(24));
}

#endif
