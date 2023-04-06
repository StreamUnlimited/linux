/******************************************************************************
 *
 * Copyright(c) 2021 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#include "../halrf_precomp.h"

#ifdef RF_8730E_SUPPORT

void _tssi_backup_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)
{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		reg_backup[i] = halrf_rreg(rf, reg[i], MASKDWORD);

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Backup BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
	}
}

void _tssi_reload_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)

{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Reload BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
	}
}

/*
void _halrf_tssi_hw_tx_8730e(struct rf_info *rf,
			u8 path, u16 cnt, u16 period, s16 dbm, u32 rate, u8 bw,
			     bool enable)
{
	struct rf_pmac_tx_info tx_info = {0};
	tx_info.ppdu = rate;
	tx_info.mcs = 7;
	tx_info.bw = bw;
	tx_info.nss = 1;
	tx_info.gi = 1;
	tx_info.txagc_cw = 0;
	tx_info.dbm = dbm;
	tx_info.cnt = cnt;
	tx_info.time = 20;
	tx_info.period = period;
	tx_info.length = 0;

	halrf_set_pmac_tx(rf, 0, path, &tx_info, enable, false);
}
*/
void _halrf_tssi_stf_tx_8730e(struct rf_info *rf, s16 dbm, bool enable)
{

	if (enable) {
		/*setting*/
		halrf_wreg(rf, 0x2008, 0x01ffffff, 0x00fffff);
		halrf_wreg(rf, 0x5688, 0x03ff0000, 0x3a1);
		halrf_wreg(rf, 0x5680, 0x000ffff0, 0x000);
		halrf_wreg(rf, 0x5680, 0x07f00000, 0x7f);
		halrf_wreg(rf, 0x5680, 0xf0000000, 0x2);
		halrf_wreg(rf, 0x5680, BIT(27), 0x0);
		halrf_wreg(rf, 0x5684, 0x0000007f, 0x1);
		halrf_wreg(rf, 0x5684, 0x007f0000, 0x0);
		halrf_wreg(rf, 0x5688, BIT(30), 0x1);
		halrf_wreg(rf, 0x120c, 0x0000f000, 0x1);
		/*run STF tx*/
		halrf_wreg(rf, 0x56cc, BIT(28), 0x1);//dbg mode for pwr setting
		halrf_wreg(rf, 0x56cc, 0x0ff80000, dbm);//force xdbm=pwr*4
		halrf_wreg(rf, 0x5864, BIT(27), 0x0);//disable iqk
		//wire r_single_tone_trig = r_page56_80[2];
		halrf_wreg(rf, 0x5680, BIT(2), 0x1);
		halrf_wreg(rf, 0x5680, BIT(2), 0x0);
	} else {
		/*clr STF tx*/
		halrf_wreg(rf, 0x5680, BIT(3), 0x0);
		halrf_wreg(rf, 0x5684, BIT(7), 0x1);
		halrf_wreg(rf, 0x5684, BIT(7), 0x0);
		halrf_wreg(rf, 0x5864, BIT(27), 0x1);//enable iqk
		halrf_wreg(rf, 0x56cc, BIT(28), 0x0);
		/*clr AFE*/
		halrf_wreg(rf, 0x033c, 0xFF000000, 0x1f);//pmactx=0x55
		halrf_wreg(rf, 0x023c, 0xFFFF0000, 0x2001);//pmactx=0
		//halrf_wreg(rf, 0x5670, BIT(0), 0x1);
		halrf_wreg(rf, 0x02fc, BIT(16), 0x1);
		halrf_wreg(rf, 0x02fc, BIT(20), 0x0);
		halrf_wreg(rf, 0x5670, BIT(0), 0x0);
		halrf_wreg(rf, 0x02fc, BIT(16), 0x0);
	}
}

void _halrf_tssi_set_sys_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x12a8, 0x0000000f, 0x5);//clk160
	halrf_wreg(rf, 0x12bc, 0x000ffff0, 0xb5b5);
	halrf_wreg(rf, 0x0300, 0xff000000, 0x0d);
	halrf_wreg(rf, 0x0304, 0x0000ffff, 0x0d0d);
	halrf_wreg(rf, 0x0308, 0xff000000, 0x08);
	halrf_wreg(rf, 0x0314, 0xffff0000, 0x0002);
	halrf_wreg(rf, 0x0318, 0x0000ffff, 0x0002);
	halrf_wreg(rf, 0x0318, 0xffff0000, 0x0002);
	halrf_wreg(rf, 0x0324, 0xffff0000, 0x0001);
	halrf_wreg(rf, 0x0020, 0x00006000, 0x3);
	halrf_wreg(rf, 0x0024, 0x00006000, 0x3);
	halrf_wreg(rf, 0x0704, 0xffff0000, 0x601e);
	halrf_wreg(rf, 0x0700, 0xf0000000, 0x4);
	halrf_wreg(rf, 0x0650, 0x3c000000, 0x0);

	halrf_wreg(rf, 0x120c, 0x000000ff, 0x33);
	halrf_wreg(rf, 0x12c0, 0x0ff00000, 0x33);
	halrf_wreg(rf, 0x58f8, 0x40000000, 0x0);

}

void _halrf_tssi_ini_txpwr_ctrl_bb_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	halrf_wreg(rf, 0x566c, 0x00001000, 0x0);
	halrf_wreg(rf, 0x5800, 0xffffffff, 0x003f807f);
#if 0
	halrf_wreg(rf, 0x5804, 0xffffffff, 0x02b27000);
	halrf_wreg(rf, 0x5808, 0xffffffff, 0x02b27000);

#endif
	halrf_wreg(rf, 0x580c, 0x0000007f, 0x40);
	halrf_wreg(rf, 0x580c, 0x0fffff00, 0x00040);
	halrf_wreg(rf, 0x5810, 0xffffffff, 0x59010000);
	halrf_wreg(rf, 0x5814, 0x01ffffff, 0x026d000);
	halrf_wreg(rf, 0x5814, 0xf8000000, 0x00);
	halrf_wreg(rf, 0x5814, 0x00000400, 0x1);//debug cck
	halrf_wreg(rf, 0x5818, 0xffffffff, 0x002c1800);
	halrf_wreg(rf, 0x581c, 0x3fffffff, 0x3dc80280);
	halrf_wreg(rf, 0x5820, 0xffffffff, 0x00000080);
	halrf_wreg(rf, 0x58e8, 0x0000003f, 0x03);//4pkt
	halrf_wreg(rf, 0x580c, 0x10000000, 0x1);
	halrf_wreg(rf, 0x580c, 0x40000000, 0x1);
	halrf_wreg(rf, 0x5834, 0x3fffffff, 0x000115f2);
	halrf_wreg(rf, 0x5838, 0x7fffffff, 0x0000121);
	halrf_wreg(rf, 0x5854, 0x3fffffff, 0x000115f2);
	halrf_wreg(rf, 0x5858, 0x7fffffff, 0x0000121);
	halrf_wreg(rf, 0x5860, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5864, 0x07ffffff, 0x00801ff);
	halrf_wreg(rf, 0x5898, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x589c, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x58a4, 0x000000ff, 0x16);
	halrf_wreg(rf, 0x58b0, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x58b4, 0x7fffffff, 0x0a002000);
	halrf_wreg(rf, 0x58b8, 0x7fffffff, 0x00007628);
	halrf_wreg(rf, 0x58bc, 0x07ffffff, 0x7f7807f);
	halrf_wreg(rf, 0x58c0, 0xfffe0000, 0x003f);
	halrf_wreg(rf, 0x58c4, 0xffffffff, 0x0003ffff);
	halrf_wreg(rf, 0x58c8, 0x00ffffff, 0x000000);
	halrf_wreg(rf, 0x58c8, 0xf0000000, 0x0);
	halrf_wreg(rf, 0x58cc, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x58d0, 0x07ffffff, 0x2008101);
	halrf_wreg(rf, 0x58d4, 0x000000ff, 0x00);
	halrf_wreg(rf, 0x58d4, 0x0003fe00, 0x0ff);
	halrf_wreg(rf, 0x58d4, 0x07fc0000, 0x100);
	if (rf->hal_com->cv == CAV) {
		halrf_wreg(rf, 0x58d8, 0xffffffff, 0x8008b733);        //r_tssi_bypass_by_C_max = r_page58_D8[8:0] acut
	} else {
		halrf_wreg(rf, 0x58d8, 0xffffffff, 0x8008b76c);
	}
	halrf_wreg(rf, 0x58dc, 0x0001ffff, 0x0807f);
	//halrf_wreg(rf, 0x58dc, 0xfff00000, 0x800);//tssi rst
	halrf_wreg(rf, 0x58f0, 0x0003ffff, 0x001ff);
	halrf_wreg(rf, 0x58f4, 0x000fffff, 0x000);
	halrf_wreg(rf, 0x58f8, 0x000fffff, 0x000);

}

void _halrf_tssi_ini_txpwr_ctrl_bb_he_tb_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x58a0, MASKDWORD, 0x000000fe);
	halrf_wreg(rf, 0x58e4, 0x0000007f, 0x1f);
}

void _halrf_tssi_set_dck_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x580c, 0x0fff0000, 0x000);
	halrf_wreg(rf, 0x5814, 0x00001000, 0x1);
	halrf_wreg(rf, 0x5814, 0x00002000, 0x1);
	halrf_wreg(rf, 0x5814, 0x00004000, 0x1);
	halrf_wreg(rf, 0x5814, 0x00038000, 0x5);
	halrf_wreg(rf, 0x5814, 0x003c0000, 0x6);
	halrf_wreg(rf, 0x5814, 0x18000000, 0x0);
}

void _halrf_tssi_set_bbgain_split_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5818, 0x08000000, 0x1);
	halrf_wreg(rf, 0x58d4, 0xf0000000, 0x7);
	halrf_wreg(rf, 0x58f0, 0x000c0000, 0x1);
	halrf_wreg(rf, 0x58f0, 0xfff00000, 0x400);
}

void _halrf_tssi_set_tmeter_tbl_8730e(struct rf_info *rf)
{
	struct halrf_pwr_track_info *pwr_trk = &rf->pwr_track;
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	struct rtw_hal_com_t *hal = rf->hal_com;

	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	u8 i, thermal = 0xff, j;
	u32 thermal_offset_tmp = 0;
	s8 thermal_offset[128] = {0};
	s8 thermal_up_a[DELTA_SWINTSSI_SIZE] = {0}, thermal_down_a[DELTA_SWINTSSI_SIZE] = {0};

	if (tssi_info->tssi_type[HW_PHY_0] == TSSI_CAL) {
		halrf_wreg(rf, 0x5810, 0x00010000, 0x0);
		halrf_wreg(rf, 0x5810, 0x01000000, 0x1);

		halrf_wreg(rf, 0x58ec, 0x00007F00, 32);
		halrf_wreg(rf, 0x5654, 0x7F000000, 32);

		for (i = 0; i < 128; i = i + 4) {
			thermal_offset_tmp = (thermal_offset[i] & 0xff) |
					     (thermal_offset[i + 1] & 0xff) << 8 |
					     (thermal_offset[i + 2] & 0xff) << 16 |
					     (thermal_offset[i + 3] & 0xff) << 24;

			halrf_wreg(rf, (0x1400 + i), MASKDWORD, thermal_offset_tmp);
			RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
			       "[TSSI] write addr:0x%x value=0x%08x\n",
			       (0x1400 + i), thermal_offset_tmp);

		}

		halrf_wreg(rf, 0x5864, BIT(26), 0x1);
		halrf_wreg(rf, 0x5864, BIT(26), 0x0);
		RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
		       "[TSSI pretx] 0x1c08=0x%x \n", halrf_rreg(rf, 0x1c08, MASKDWORD));
		halrf_wreg(rf, 0x5810, BIT(23), 0x1);
		halrf_wreg(rf, 0x5810, BIT(23), 0x0);
		RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
		       "[TSSI pretx] 0x1c08=0x%x \n", halrf_rreg(rf, 0x1c08, MASKDWORD));

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======>%s TSSI Cal Mode return !!!\n", __func__);

		return;
	}
	if (channel >= 1 && channel <= 14) {
		hal_mem_cpy(hal, thermal_up_a, pwr_trk->delta_swing_table_idx_2ga_p, sizeof(thermal_up_a));
		hal_mem_cpy(hal, thermal_down_a, pwr_trk->delta_swing_table_idx_2ga_n, sizeof(thermal_down_a));

	} else if (channel >= 36 && channel <= 64) {
		hal_mem_cpy(hal, thermal_up_a, pwr_trk->delta_swing_table_idx_5ga_p[0], sizeof(thermal_up_a));
		hal_mem_cpy(hal, thermal_down_a, pwr_trk->delta_swing_table_idx_5ga_n[0], sizeof(thermal_down_a));

	} else if (channel >= 100 && channel <= 144) {
		hal_mem_cpy(hal, thermal_up_a, pwr_trk->delta_swing_table_idx_5ga_p[1], sizeof(thermal_up_a));
		hal_mem_cpy(hal, thermal_down_a, pwr_trk->delta_swing_table_idx_5ga_n[1], sizeof(thermal_down_a));

	} else if (channel >= 149 && channel <= 177) {
		hal_mem_cpy(hal, thermal_up_a, pwr_trk->delta_swing_table_idx_5ga_p[2], sizeof(thermal_up_a));
		hal_mem_cpy(hal, thermal_down_a, pwr_trk->delta_swing_table_idx_5ga_n[2], sizeof(thermal_down_a));
	}
	/*path s0*/

	hal_mem_set(hal, thermal_offset, 0, sizeof(thermal_offset));
	thermal = rf->phl_com->efuse_data.thermal_a;

	/*thermal = 32;*/

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI] channel=%d thermal_pahtA=0x%x \n",
	       channel, thermal);
	halrf_wreg(rf, 0x5810, 0x00010000, 0x0);
	halrf_wreg(rf, 0x5810, 0x01000000, 0x1);

	if (thermal == 0xff) {//no PG
		halrf_wreg(rf, 0x58ec, 0x00007F00, 32);//T0
		halrf_wreg(rf, 0x5654, 0x7F000000, 32);//T1


		for (i = 0; i < 128; i = i + 4) {
			thermal_offset_tmp = (thermal_offset[i] & 0xff) |
					     (thermal_offset[i + 1] & 0xff) << 8 |
					     (thermal_offset[i + 2] & 0xff) << 16 |
					     (thermal_offset[i + 3] & 0xff) << 24;

			halrf_wreg(rf, (0x1400 + i), MASKDWORD, thermal_offset_tmp);
			RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
			       "[TSSI] no PG write addr:0x%x value=0x%08x\n",
			       (0x1400 + i), thermal_offset_tmp);
		}
	}
	tssi_info->thermal[RF_PATH_A] = thermal >> 1;

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI_TRK] thermal base=%d \n", tssi_info->thermal[RF_PATH_A]);

	if (thermal != 0xff) {
		halrf_wreg(rf, 0x58ec, 0x00007F00, (thermal >> 1));
		halrf_wreg(rf, 0x5654, 0x7F000000, (thermal >> 1));

		i = 0;
		for (j = 0; j < 64; j++) {//T0>T1
			if (i < DELTA_SWINTSSI_SIZE) {
				thermal_offset[j] = -1 * thermal_down_a[i++];
			} else {
				thermal_offset[j] = -1 * thermal_down_a[DELTA_SWINTSSI_SIZE - 1];
			}
		}

		i = 1;
		for (j = 127; j >= 64; j--) {//T0<T1
			if (i < DELTA_SWINTSSI_SIZE) {
				thermal_offset[j] = thermal_up_a[i++];
			} else {
				thermal_offset[j] = thermal_up_a[DELTA_SWINTSSI_SIZE - 1];
			}
		}
		for (i = 0; i < 128; i = i + 4) {
			RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
			       "[TSSI] thermal_offset[%.2d]=%.2x %.2x %.2x %.2x\n",
			       i, thermal_offset[i + 3] & 0xff, thermal_offset[i + 2] & 0xff,
			       thermal_offset[i + 1] & 0xff, thermal_offset[i] & 0xff);
		}
		for (i = 0; i < 128; i = i + 4) {
			thermal_offset_tmp = (thermal_offset[i] & 0xff) |
					     (thermal_offset[i + 1] & 0xff) << 8 |
					     (thermal_offset[i + 2] & 0xff) << 16 |
					     (thermal_offset[i + 3] & 0xff) << 24;

			halrf_wreg(rf, (0x1400 + i), MASKDWORD, thermal_offset_tmp);
			RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
			       "[TSSI] PG write addr:0x%x value=0x%08x\n",
			       (0x1400 + i), thermal_offset_tmp);
		}
	}
	//halrf_wreg(rf, 0x5810, 0x00010000, 0x1);//debug disable Tmeter tbl
	halrf_wreg(rf, 0x5864, BIT(26), 0x1);
	halrf_wreg(rf, 0x5864, BIT(26), 0x0);
	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI pretx] 0x1c08=0x%x \n", halrf_rreg(rf, 0x1c08, MASKDWORD));
	halrf_wreg(rf, 0x5810, BIT(23), 0x1);
	halrf_wreg(rf, 0x5810, BIT(23), 0x0);
	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI pretx] 0x1c08=0x%x \n", halrf_rreg(rf, 0x1c08, MASKDWORD));

}
#if 0
void _halrf_tssi_set_tmeter_tbl_zere_8730e(struct rf_info *rf,
		enum phl_phy_idx phy, enum rf_path path)
{
	u8 i;
	u32 thermal_offset_tmp = 0;
	s8 thermal_offset[128] = {0};

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======>%s   path=%d\n", __func__, path);

	halrf_wreg(rf, 0x5810, 0x00010000, 0x0);
	halrf_wreg(rf, 0x5810, 0x01000000, 0x1);

	halrf_wreg(rf, 0x5810, 0x0000fc00, 32);
	halrf_wreg(rf, 0x5864, 0x03f00000, 32);

	for (i = 0; i < 128; i = i + 4) {
		thermal_offset_tmp = (thermal_offset[i] & 0xff) |
				     (thermal_offset[i + 1] & 0xff) << 8 |
				     (thermal_offset[i + 2] & 0xff) << 16 |
				     (thermal_offset[i + 3] & 0xff) << 24;

		halrf_wreg(rf, (0x5c00 + i), MASKDWORD, thermal_offset_tmp);

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
		       "[TSSI] write addr:0x%x value=0x%08x\n",
		       (0x5c00 + i), thermal_offset_tmp);
	}

	halrf_wreg(rf, 0x5864, BIT(26), 0x1);
	halrf_wreg(rf, 0x5864, BIT(26), 0x0);

}

#endif
void _halrf_tssi_slope_cal_org_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5608, 0x07ffffff, 0x0201008);
	halrf_wreg(rf, 0x560c, 0x07ffffff, 0x0201008);
	halrf_wreg(rf, 0x5610, 0x07ffffff, 0x0201008);
	halrf_wreg(rf, 0x5614, 0x07ffffff, 0x0201008);
	halrf_wreg(rf, 0x5618, 0x07ffffff, 0x0201008);
	halrf_wreg(rf, 0x561c, 0x000001ff, 0x008);
	halrf_wreg(rf, 0x561c, 0xffff0000, 0x0808);
	halrf_wreg(rf, 0x5620, MASKDWORD, 0x08080808);
	halrf_wreg(rf, 0x5624, MASKDWORD, 0x08080808);
	halrf_wreg(rf, 0x5628, MASKDWORD, 0x08080808);
	halrf_wreg(rf, 0x562c, 0x0000ffff, 0x0808);
	halrf_wreg(rf, 0x581c, 0x00100000, 0x0);
}

void _halrf_tssi_set_alignment_default_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;

	halrf_wreg(rf, 0x5604, 0x80000000, 0x1);
	halrf_wreg(rf, 0x5604, 0x003fffff, 0x2d2721);
	halrf_wreg(rf, 0x5634, 0x3ffffc00, 0x000000);
	halrf_wreg(rf, 0x5638, 0x000fffff, 0x000000);
	halrf_wreg(rf, 0x5640, 0x3ffffc00, 0x000000);
	halrf_wreg(rf, 0x5644, 0x000fffff, 0x000000);
	halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000FAC);
	//halrf_wreg(rf, 0x5600, 0x3fffffff, 0x000000);
	if (rf->hal_com->cv == CAV) {
		halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000FAC);
		if (channel >= 1 && channel <= 14) {
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f2d2a21);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3e8);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x029);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x030);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x085);
			halrf_wreg(rf, 0x563c, 0x000003ff, 0x3e8);//r_tssi_J_cck_G0
			halrf_wreg(rf, 0x563c, 0x000FFC00, 0x029);//r_tssi_J_cck_G1
			halrf_wreg(rf, 0x563c, 0x3FF00000, 0x030);//r_tssi_J_cck_G2
			halrf_wreg(rf, 0x5640, 0x000003ff, 0x085);
		} else if (channel >= 36 && channel <= 64) {
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3f0);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x021);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x04a);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x099);
			halrf_wreg(rf, 0x5818, MASKBYTE0, 0xea);//txagc offset acut
		} else if (channel >= 100 && channel <= 144) {
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3f0);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x021);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x04a);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x095);
			halrf_wreg(rf, 0x5818, MASKBYTE0, 0xf2);//txagc offset acut
		} else if (channel >= 149 && channel <= 177) {
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3f0);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x021);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x04c);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x09b);
			halrf_wreg(rf, 0x5818, MASKBYTE0, 0xf2);//txagc offset acut
		}
	} else {
		halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000FAC);
		if (channel >= 1 && channel <= 14) {
			halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000692);
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x2a212121);
			halrf_wreg(rf, 0x5604, MASKDWORD, 0x3f3f332d);
			halrf_wreg(rf, 0x5630, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5634, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0x97);
			halrf_wreg(rf, 0x563c, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5640, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5644, 0x000003ff, 0x97);
		} else if (channel >= 36 && channel <= 64) {
			/*
			halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000692);
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x27212121);
			halrf_wreg(rf, 0x5604, MASKDWORD, 0x3f3f3930);
			halrf_wreg(rf, 0x5630, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5634, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0x97);
			halrf_wreg(rf, 0x563c, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5640, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5644, 0x000003ff, 0x97);
			*/
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3f9);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x020);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x03c);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x091);

			//halrf_wreg(0x5818, MASKBYTE0, 0xea);//txagc offset acut
		} else if (channel >= 100 && channel <= 144) {
			/*
			halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000692);
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x27212121);
			halrf_wreg(rf, 0x5604, MASKDWORD, 0x3f3f3930);
			halrf_wreg(rf, 0x5630, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5634, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0x97);
			halrf_wreg(rf, 0x563c, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5640, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5644, 0x000003ff, 0x97);
			*/
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x3f3);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x020);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x03c);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x091);

			//halrf_wreg(rf, 0x5818, MASKBYTE0, 0xf2);//txagc offset acut
		} else if (channel >= 149 && channel <= 177) {
			/*
			halrf_wreg(rf, 0x56c0, MASKDWORD, 0x00000692);
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x27212121);
			halrf_wreg(rf, 0x5604, MASKDWORD, 0x3f3f3930);
			halrf_wreg(rf, 0x5630, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5634, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0x97);
			halrf_wreg(rf, 0x563c, MASKDWORD, 0x3E1F87E1);
			halrf_wreg(rf, 0x5640, MASKDWORD, 0x88516C0E);
			halrf_wreg(rf, 0x5644, 0x000003ff, 0x97);
			*/
			halrf_wreg(rf, 0x5600, MASKDWORD, 0x3f302721);//r_tssi_curve_p0
			halrf_wreg(rf, 0x5630, 0x000003ff, 0x000);//r_tssi_J_ofdm_G0
			halrf_wreg(rf, 0x5630, 0x000FFC00, 0x025);//r_tssi_J_ofdm_G1
			halrf_wreg(rf, 0x5630, 0x3FF00000, 0x041);//r_tssi_J_ofdm_G2
			halrf_wreg(rf, 0x5634, 0x000003ff, 0x091);

			//halrf_wreg(rf, 0x5818, MASKBYTE0, 0xf2);//txagc offset acut
		}
		halrf_wreg(rf, 0x5604, BIT(31), 0x1);//wire r_tssi_curve_en = r_page56_04[31];
	}

}

void _halrf_tssi_run_slope_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;

	halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5820, 0x80000000, 0x1);

	if (rf->hal_com->cv > CAV) {
		if (channel >= 1 && channel <= 14) {
			halrf_wreg(rf, 0x5624, 0x000000ff, 0x28);
			halrf_wreg(rf, 0x5610, 0x000001ff, 0x21);
			halrf_wreg(rf, 0x562c, 0x000000ff, 0x28);
			halrf_wreg(rf, 0x5618, 0x07fc0000, 0x1c);
			halrf_wreg(rf, 0x581c, 0x00100000, 0x1);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0xB8);
			halrf_wreg(rf, 0x5644, 0x000003ff, 0xdc);
		} else {
			halrf_wreg(rf, 0x5624, 0x000000ff, 0x28);
			halrf_wreg(rf, 0x5610, 0x000001ff, 0x21);
			halrf_wreg(rf, 0x581c, 0x00100000, 0x1);
			halrf_wreg(rf, 0x5638, 0x000003ff, 0xB8);
		}
	}
}

void _halrf_tssi_set_slope_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5818, 0x10000000, 0x0);
	halrf_wreg(rf, 0x5814, 0x00000800, 0x1);
	halrf_wreg(rf, 0x581c, 0x20000000, 0x1);
	halrf_wreg(rf, 0x58e8, 0x0000003f, 0x0f);
	halrf_wreg(rf, 0x581c, 0x000003ff, 0x280);
	halrf_wreg(rf, 0x581c, 0x000ffc00, 0x200);
	halrf_wreg(rf, 0x58b8, 0x007f0000, 0x00);
	halrf_wreg(rf, 0x58b8, 0x7f000000, 0x00);
	halrf_wreg(rf, 0x58b4, 0x7f000000, 0x0a);
	halrf_wreg(rf, 0x58b8, 0x0000007f, 0x28);
	halrf_wreg(rf, 0x58b8, 0x00007f00, 0x76);
	halrf_wreg(rf, 0x5810, 0x20000000, 0x0);
	halrf_wreg(rf, 0x5814, 0x20000000, 0x1);
	halrf_wreg(rf, 0x580c, 0x10000000, 0x1);
	halrf_wreg(rf, 0x580c, 0x40000000, 0x1);
	halrf_wreg(rf, 0x5834, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x5834, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5838, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5838, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x5854, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x5854, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5858, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5858, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x5824, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x5824, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5828, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5828, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x582c, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x582c, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5830, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5830, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x583c, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x583c, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5840, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5840, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x5844, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x5844, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5848, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5848, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x584c, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x584c, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5850, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5850, 0x003ff000, 0x000);
	halrf_wreg(rf, 0x585c, 0x0003ffff, 0x115f2);
	halrf_wreg(rf, 0x585c, 0x3ffc0000, 0x000);
	halrf_wreg(rf, 0x5860, 0x00000fff, 0x121);
	halrf_wreg(rf, 0x5860, 0x003ff000, 0x000);

}

void _halrf_tssi_set_tssi_track_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5818, 0x10000000, 0x0);
	halrf_wreg(rf, 0x5814, 0x00000800, 0x0);
	halrf_wreg(rf, 0x581c, 0x20000000, 0x1);
	halrf_wreg(rf, 0x5864, 0x000003ff, 0x1ff);
	halrf_wreg(rf, 0x5864, 0x000ffc00, 0x200);
	halrf_wreg(rf, 0x5820, 0x00000fff, 0x080);
	halrf_wreg(rf, 0x5814, 0x01000000, 0x0);

}

void _halrf_tssi_set_txagc_offset_mv_avg_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x58e4, 0x00003800, 0x0);//moving avg=0
	halrf_wreg(rf, 0x58e4, 0x00004000, 0x0);
	halrf_wreg(rf, 0x58e4, 0x00008000, 0x1);
	halrf_wreg(rf, 0x58e4, 0x000f0000, 0x0);
	halrf_wreg(rf, 0x58e8, 0x0000003f, 0x03);

}

u32 _halrf_tssi_get_cck_efuse_group_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	u32 offset_index = 0;

	if (channel >= 1 && channel <= 2) {
		offset_index = 0;
	} else if (channel >= 3 && channel <= 5) {
		offset_index = 1;
	} else if (channel >= 6 && channel <= 8) {
		offset_index = 2;
	} else if (channel >= 9 && channel <= 11) {
		offset_index = 3;
	} else if (channel >= 12 && channel <= 13) {
		offset_index = 4;
	} else if (channel == 14) {
		offset_index = 5;
	}

	return offset_index;
}

u32 _halrf_tssi_get_ofdm_efuse_group_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	u32 offset_index = 0;

	if (channel >= 1 && channel <= 2) {
		offset_index = 0;
	} else if (channel >= 3 && channel <= 5) {
		offset_index = 1;
	} else if (channel >= 6 && channel <= 8) {
		offset_index = 2;
	} else if (channel >= 9 && channel <= 11) {
		offset_index = 3;
	} else if (channel >= 12 && channel <= 14) {
		offset_index = 4;
	} else if (channel >= 36 && channel <= 40) {
		offset_index = 5;
	} else if (channel >= 44 && channel <= 48) {
		offset_index = 6;
	} else if (channel >= 52 && channel <= 56) {
		offset_index = 7;
	} else if (channel >= 60 && channel <= 64) {
		offset_index = 8;
	} else if (channel >= 100 && channel <= 104) {
		offset_index = 9;
	} else if (channel >= 108 && channel <= 112) {
		offset_index = 10;
	} else if (channel >= 116 && channel <= 120) {
		offset_index = 11;
	} else if (channel >= 124 && channel <= 128) {
		offset_index = 12;
	} else if (channel >= 132 && channel <= 136) {
		offset_index = 13;
	} else if (channel >= 140 && channel <= 144) {
		offset_index = 14;
	} else if (channel >= 149 && channel <= 153) {
		offset_index = 15;
	} else if (channel >= 157 && channel <= 161) {
		offset_index = 16;
	} else if (channel >= 165 && channel <= 169) {
		offset_index = 17;
	} else if (channel >= 173 && channel <= 177) {
		offset_index = 18;
	} else if (channel > 40 && channel < 44) {
		offset_index = 0x0506;
	} else if (channel > 48  && channel < 52) {
		offset_index = 0x0607;
	} else if (channel > 56 && channel < 60) {
		offset_index = 0x0708;
	} else if (channel > 104 && channel < 108) {
		offset_index = 0x090a;
	} else if (channel > 112 && channel < 116) {
		offset_index = 0x0a0b;
	} else if (channel > 120 && channel < 124) {
		offset_index = 0x0b0c;
	} else if (channel > 128 && channel < 132) {
		offset_index = 0x0c0d;
	} else if (channel > 136 && channel < 140) {
		offset_index = 0x0d0e;
	} else if (channel > 153 && channel < 157) {
		offset_index = 0x0f10;
	} else if (channel > 161 && channel < 165) {
		offset_index = 0x1011;
	} else if (channel > 169 && channel < 173) {
		offset_index = 0x1112;
	}

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI efuse] ======> %s offset_index=0x%x(%d) channel=%d\n",
	       __func__, offset_index, offset_index, channel);

	return offset_index;
}

s8 _halrf_tssi_get_ofdm_efuse_tssi_de_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u32 group_idx;
	s8 first_de = 0, second_de = 0, final_de;

	group_idx = _halrf_tssi_get_ofdm_efuse_group_8730e(rf);

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======> %s   group_idx=0x%x(%d)\n",
		__func__, group_idx, group_idx);*/

	if (group_idx >= 0x0506) {
		first_de = tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_MCS][group_idx >> 8];

		second_de = tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_MCS][group_idx & 0xff];

		final_de = (first_de + second_de) / 2;

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI efuse] first_de=%d second_de=%d final_de=%d\n",
		       first_de, second_de, final_de);
	} else {
		final_de = tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_MCS][group_idx];

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI efuse] final_de=%d\n",
		       final_de);
	}

	return final_de;
}


u32 _halrf_tssi_get_tssi_trim_efuse_group_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	u32 group_index = 0;

	if (channel >= 1 && channel <= 8) {
		group_index = 0;
	} else if (channel >= 9 && channel <= 14) {
		group_index = 1;
	} else if (channel >= 36 && channel <= 48) {
		group_index = 2;
	} else if (channel >= 52 && channel <= 64) {
		group_index = 3;
	} else if (channel >= 100 && channel <= 112) {
		group_index = 4;
	} else if (channel >= 116 && channel <= 128) {
		group_index = 5;
	} else if (channel >= 132 && channel <= 144) {
		group_index = 6;
	} else if (channel >= 149 && channel <= 177) {
		group_index = 7;
	}
#if 0
	else if (channel > 48 && channel < 52) {
		group_index = 0x0203;
	} else if (channel > 112 && channel < 116) {
		group_index = 0x0405;
	}
#endif


	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI Trim] ======> %s offset_index=0x%x(%d) channel=%d\n",
	       __func__, group_index, group_index, channel);
	return group_index;
}

s8 _halrf_tssi_get_ofdm_tssi_trim_de_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u32 group_idx;
	s8 first_de = 0, second_de = 0, final_de;

	group_idx = _halrf_tssi_get_tssi_trim_efuse_group_8730e(rf);

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI Trim] ======> %s   group_idx=0x%x(%d)\n",
		__func__, group_idx, group_idx);*/

	if (group_idx >= 0x0203) {
		first_de = tssi_info->tssi_trim[RF_PATH_A][group_idx >> 8];

		second_de = tssi_info->tssi_trim[RF_PATH_A][group_idx & 0xff];

		final_de = (first_de + second_de) / 2;

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI Trim] Trim_first_de=%d Trim_second_de=%d Trim_final_de=%d\n",
		       first_de, second_de, final_de);
	} else {
		final_de = tssi_info->tssi_trim[RF_PATH_A][group_idx];


		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI Trim] Trim_final_de=%d\n",
		       final_de);
	}

	return final_de;
}

void _halrf_tssi_alimentk_8730e(struct rf_info *rf)
{
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	s16 power_2g[4] = {48, 20, 4, 4};
	s16 power_5g[4] = {48, 20, 4, 4};
	s16 power[4] = {0};
	s32 tssi_alim_offset[4] = {0};
	s32 aliment_diff, tssi_cw_default;
	u32 tssi_cw_rpt[4] = {0}, tmp, retry;
	u32 tssi_cw_default_addr[4] =
	{0x5634, 0x5630, 0x5630, 0x5630};

	u32 tssi_cw_default_mask[4] =
	{0x000003ff, 0x3ff00000, 0x000ffc00, 0x000003ff};

	u32 bb_reg[6] = {0x5820, 0x4978, 0x58e4,
			 0x49c0, 0x0d18, 0x0d80
			};
	u32 bb_reg_backup[6] = {0};
	u32 backup_num = 6;

	u8 i, j, k;


	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======> %s   channel=%d   path=%d\n",
		__func__, channel, path);*/

	halrf_hal_bb_backup_info(rf, 0);

	if (channel >= 1 && channel <= 14) {
		for (i = 0; i < 4; i++) {
			power[i] = power_2g[i];
		}
	} else {
		for (i = 0; i < 4; i++) {
			power[i] = power_5g[i];
		}
	}

	_tssi_backup_bb_registers_8730e(rf, bb_reg, bb_reg_backup,
					backup_num);

	halrf_wreg(rf, 0x5820, 0x0000f000, 0x8);

	halrf_wreg(rf, 0x58e4, 0x00003800, 0x2);

	halrf_wreg(rf, 0x58e8, 0x0000003f, 0xf);

	//halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_TSSI, RFK_START);
	for (j = 0; j < 2; j++) {
		halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
		halrf_wreg(rf, 0x5820, 0x80000000, 0x1);

		//tx_couter = halrf_rreg(rf, 0x1a40, 0xffff);
		//_halrf_tssi_hw_tx_8730e(rf, RF_PATH_A, 100, 5000, power[j], rate, bw, true);
		_halrf_tssi_stf_tx_8730e(rf, power[j], true);

		//halrf_delay_ms(rf, 10);

		//tx_couter_tmp = halrf_rreg(rf, 0x1a40, 0xffff) - tx_couter;
		/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI PA K] First HWTXcounter=%d\n",
			tx_couter_tmp);*/
		for (k = 0; halrf_rreg(rf, 0x1c18, BIT(16)) == 0; k++) {
			if (rf->phl_com->drv_mode == RTW_DRV_MODE_MP) {
				halrf_delay_ms(rf, 1);
				retry = 30;
			} else {
				halrf_delay_us(rf, 30);
				retry = 100;
			}

			//tx_couter_tmp = halrf_rreg(rf, 0x1a40, 0xffff) - tx_couter;
			/*
			RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI PA K] Flow k = %d HWTXcounter=%d path=%d\n",
			       k, tx_couter_tmp, path);
			*/
			if (k > retry) {
				RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI PA K] TSSI finish bit k > %d mp:100ms normal:30us \n",
				       k);
				_tssi_reload_bb_registers_8730e(rf, bb_reg, bb_reg_backup,
								backup_num);
				//_halrf_tssi_hw_tx_8730e(rf, RF_PATH_A, 100, 5000, power[j], rate, bw, false);
				_halrf_tssi_stf_tx_8730e(rf, power[j], false);

				halrf_tx_mode_switch(rf, 0, 0);

				return;
			}
		}

		tssi_cw_rpt[j] = halrf_rreg(rf, 0x1c18, 0x000001ff);

		//_halrf_tssi_hw_tx_8730e(rf, RF_PATH_A, 100, 5000, power[j], rate, bw, false);

		//tx_couter = halrf_rreg(rf, 0x1a40, 0xffff) - tx_couter;
		_halrf_tssi_stf_tx_8730e(rf, power[j], false);

		/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
		       "[TSSI PA K] Fianl HWTXcounter = %d path=%d\n",
			tx_couter, RF_PATH_A);*/

	}
	for (j = 0; j < 2; j++) {
		RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
		       "[TSSI PA K] power[%d]=%d  tssi_cw_rpt[%d]=%d\n",
		       j, power[j], j, tssi_cw_rpt[j]);
	}

	/*tssi_alim_offset[1]*/
	tmp = halrf_rreg(rf, tssi_cw_default_addr[1],
			 tssi_cw_default_mask[1]);

	if (tmp & BIT(8)) {
		tssi_cw_default = tmp | 0xfffffe00;
	} else {
		tssi_cw_default = tmp;
	}

	tssi_alim_offset[1] = tssi_cw_rpt[0] - ((power[0] - power[1]) * 2) -
			      tssi_cw_rpt[1] + tssi_cw_default;

	aliment_diff = tssi_alim_offset[1] - tssi_cw_default;

	/*tssi_alim_offset[2]*/
	tmp = halrf_rreg(rf, tssi_cw_default_addr[2],
			 tssi_cw_default_mask[2]);

	if (tmp & BIT(8)) {
		tssi_cw_default = tmp | 0xfffffe00;
	} else {
		tssi_cw_default = tmp;
	}

	tssi_alim_offset[2] = tssi_cw_default + aliment_diff;

	/*tssi_alim_offset[3]*/
	tmp = halrf_rreg(rf, tssi_cw_default_addr[3], tssi_cw_default_mask[3]);

	if (tmp & BIT(8)) {
		tssi_cw_default = tmp | 0xfffffe00;
	} else {
		tssi_cw_default = tmp;
	}

	tssi_alim_offset[3] = tssi_cw_default + aliment_diff;

	tmp = ((tssi_alim_offset[1] & 0x3ff) << 20) |
	      ((tssi_alim_offset[2] & 0x3ff) << 10) |
	      (tssi_alim_offset[3] & 0x3ff);
	halrf_wreg(rf, 0x5630, 0x3fffffff, tmp);

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI PA K] tssi_alim_offset = 0x%x   0x%x   0x%x   0x%x\n",
	       halrf_rreg(rf, 0x5634, 0x000003ff),
	       halrf_rreg(rf, 0x5630, 0x3ff00000),
	       halrf_rreg(rf, 0x5630, 0x000ffc00),
	       halrf_rreg(rf, 0x5630, 0x000003ff));
	_tssi_reload_bb_registers_8730e(rf, bb_reg, bb_reg_backup, backup_num);
	halrf_hal_bb_restore_info(rf, 0);
	halrf_tx_mode_switch(rf, 0, 0);
}

void halrf_tssi_tracking_8730e(struct rf_info *rf)
{

	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u8 i;
	u8 cur_ther, reg_ther;
	s32 delta_ther = 0;
	s8 gain_offset;
	u8 temp[3] = {0};

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======>%s \n",__func__);*/

	for (i = 0; i < 3; i++) {
		temp[i] = (u8)halrf_rreg(rf, 0x1c10, 0x7F000000); /*[30:24]*/
	}
	cur_ther = (temp[0] + temp[1] + temp[2]) / 3;

	if (cur_ther == 0) {
		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI_TRK] thermal now=0,return \n");
		return;
	}

	if (halrf_rreg(rf, 0x1c7c, BIT(18)) == 0) { //tssi on
		reg_ther = cur_ther;
		/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI_TRK] [01]reg_ther=%d \n",reg_ther);*/
		if (reg_ther) {
			tssi_info->base_ther[RF_PATH_A] = reg_ther;
		}
	}

	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI_TRK] thermal now=%d \n", cur_ther);
	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI_TRK] thermal base=%d \n", tssi_info->base_ther[RF_PATH_A]);
	delta_ther = cur_ther - tssi_info->base_ther[RF_PATH_A];

	gain_offset = (s8)delta_ther * 15 / 10;
	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI_TRK] rf->delta_ther=%d gain_offset=%d \n",
	       delta_ther, gain_offset);

	if (!rf->rfk_is_processing) {
		halrf_wreg(rf, 0x5820, 0x1FE00000, gain_offset);//[28:21]

	}
}

void _halrf_tssi_pre_tx_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	struct rtw_tpu_info *tpu = &rf->hal_com->band[0].rtw_tpu_i;
	u8 channel = rf->hal_com->band[0].cur_chandef.center_ch;
	s16 xdbm, xdbm_temp;
	s8 power;

	if (rf->phl_com->drv_mode == RTW_DRV_MODE_MP) {
		if (rf->hal_com->cv == CAV) {
			xdbm = 15 * 4;
		} else {
			xdbm = -10 * 4;
		}
		xdbm_temp = halrf_rreg(rf, 0x4594, 0x7fc00000);

	} else {
		if (rf->hal_com->cv > CAV) {
			xdbm = -10 * 4;
		} else {
			if (tpu->pwr_lmt_en == true) {
				power = (s8)halrf_get_power(rf, RF_PATH_A,
							    RTW_DATA_RATE_OFDM6, 0, 0, CHANNEL_WIDTH_20,
							    0, channel);
			} else {
				power = halrf_get_power_by_rate(rf, RF_PATH_A,
								RTW_DATA_RATE_OFDM6, 0, 0);
			}

			xdbm = power * 100 / 4;
			if (xdbm > 1700) {
				xdbm = 15 * 4;
			} else {
				return;
			}
		}

	}


	//halrf_btc_rfk_ntfy(rf, (BIT(phy) << 4), RF_BTC_TSSI, RFK_START);
	halrf_tmac_tx_pause(rf, 0, true);
	//tx_couter= halrf_rreg(rf, 0x1a40, 0xffff);
	//_halrf_tssi_hw_tx_8730e(rf, RF_PATH_A, 40, 10, xdbm, T_HT_MF, 0, true);
	_halrf_tssi_stf_tx_8730e(rf, xdbm, true);
	halrf_delay_ms(rf, 10);
	//_halrf_tssi_hw_tx_8730e(rf, RF_PATH_A, 40, 10, xdbm, T_HT_MF, 0, false);
	_halrf_tssi_stf_tx_8730e(rf, xdbm, false);

	tssi_info->base_ther[RF_PATH_A] = (u8)halrf_rreg(rf, 0x1cd0, 0x0000007F);
	//halrf_btc_rfk_ntfy(rf, (BIT(phy) << 4), RF_BTC_TSSI, RFK_STOP);
	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI pretx] 0x1c60=0x%x \n", halrf_rreg(rf, 0x1c60, MASKDWORD));
	if (rf->phl_com->drv_mode == RTW_DRV_MODE_MP) {
		halrf_wreg(rf, 0x4594, 0x7fc00000, xdbm_temp);
	}

	halrf_tx_mode_switch(rf, 0, 0);
	halrf_tmac_tx_pause(rf, 0, false);
	//halrf_btc_rfk_ntfy(rf, (BIT(phy) << 4), RF_BTC_TSSI, RFK_STOP);

}

void halrf_do_tssi_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	halrf_tssi_disable_8730e(rf);
	_halrf_tssi_set_sys_8730e(rf);
	_halrf_tssi_ini_txpwr_ctrl_bb_8730e(rf);
	_halrf_tssi_ini_txpwr_ctrl_bb_he_tb_8730e(rf);
	_halrf_tssi_set_dck_8730e(rf);
	_halrf_tssi_set_bbgain_split_8730e(rf);
	_halrf_tssi_set_tmeter_tbl_8730e(rf);
	_halrf_tssi_slope_cal_org_8730e(rf);
	_halrf_tssi_set_alignment_default_8730e(rf);
	_halrf_tssi_set_slope_8730e(rf);
	_halrf_tssi_run_slope_8730e(rf);
	if (rf->hal_com->cv == CAV) {
		halrf_tmac_tx_pause(rf, 0, true);
		_halrf_tssi_alimentk_8730e(rf);
		halrf_tmac_tx_pause(rf, 0, false);
	}

	if (rf->phl_com->drv_mode == RTW_DRV_MODE_MP) {
		if (tssi_info->tssi_type[0] == TSSI_ON || tssi_info->tssi_type[0] == TSSI_CAL) {
			halrf_tssi_enable_8730e(rf);
			halrf_tssi_set_efuse_to_de_8730e(rf);
			_halrf_tssi_pre_tx_8730e(rf);
		} else {
			halrf_tssi_disable_8730e(rf);
		}

	} else {
		if (rf->support_ability & HAL_RF_TX_PWR_TRACK) {
			halrf_tssi_enable_8730e(rf);
			halrf_tssi_set_efuse_to_de_8730e(rf);
			_halrf_tssi_pre_tx_8730e(rf);
		}
	}

	tssi_info->do_tssi = true;
}
void halrf_do_tssi_init_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;

	halrf_tssi_disable_8730e(rf);
	_halrf_tssi_set_sys_8730e(rf);
	_halrf_tssi_ini_txpwr_ctrl_bb_8730e(rf);
	//_halrf_tssi_ini_txpwr_ctrl_bb_he_tb_8730e(rf);
	_halrf_tssi_set_dck_8730e(rf);
	_halrf_tssi_set_bbgain_split_8730e(rf);
	//_halrf_tssi_set_tmeter_tbl_8730e(rf);
	_halrf_tssi_slope_cal_org_8730e(rf);
	_halrf_tssi_set_alignment_default_8730e(rf);
	_halrf_tssi_set_slope_8730e(rf);
	_halrf_tssi_run_slope_8730e(rf);
	if (rf->hal_com->cv == CAV) {
		halrf_tmac_tx_pause(rf, 0, true);
		_halrf_tssi_alimentk_8730e(rf);
		halrf_tmac_tx_pause(rf, 0, false);
	}
	if (rf->support_ability & HAL_RF_TX_PWR_TRACK) {
		halrf_tssi_enable_8730e(rf);
		halrf_tssi_set_efuse_to_de_8730e(rf);
		halrf_wreg(rf, 0x58e8, 0x0000003f, 0x0);//1pkt
	}

}

void halrf_do_tssi_scan_8730e(struct rf_info *rf)
{
	//if POUT>Xdbm scan need trigger tssi traking to en txagc_offset
	halrf_wreg(rf, 0x5818, BIT(28), 0x0);
	halrf_wreg(rf, 0x5818, BIT(28), 0x1);
	//moving avg=0 for SCAN
	halrf_wreg(rf, 0x58e4, 0x00003800, 0x0);

}

void halrf_tssi_get_efuse_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;

	/*path s0*/
	_rtw_memcpy(tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_CCK], rf->phl_com->efuse_data.tssi_2g, 6);
	_rtw_memcpy(tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_CCK], rf->phl_com->efuse_data.tssi_2g + 6, 5);
	_rtw_memcpy(&tssi_info->tssi_efuse[RF_PATH_A][EFUSE_TSSI_MCS][5], rf->phl_com->efuse_data.tssi_5g, 14);
}

void halrf_set_tssi_de_for_tx_verify_8730e(struct rf_info *rf,
		u32 tssi_de)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u32 tssi_de_tmp;
	s32 s_tssi_de, tmp;
	s8 tssi_trim;


	if (tssi_de & BIT(7)) {
		s_tssi_de = tssi_de | 0xffffff00;
	} else {
		s_tssi_de = tssi_de;
	}

	tssi_trim = _halrf_tssi_get_ofdm_tssi_trim_de_8730e(rf);
	tmp = s_tssi_de + tssi_trim;


	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] tmp(%d) = s_tssi_de(%d) + tssi_trim(%d)\n",
	       tmp, s_tssi_de, tssi_trim);

	tssi_de_tmp = tmp & 0x3ff;

	halrf_wreg(rf, 0x5858, 0x003ff000, tssi_de_tmp);
	halrf_wreg(rf, 0x5860, 0x003ff000, tssi_de_tmp);
	halrf_wreg(rf, 0x5838, 0x003ff000, tssi_de_tmp);
	halrf_wreg(rf, 0x5828, 0x003ff000, tssi_de_tmp);
	halrf_wreg(rf, 0x5830, 0x003ff000, tssi_de_tmp);
	tssi_info->tssi_de[RF_PATH_A] = tmp;

	/*Save TSSI data for WINCLI*/
	tssi_info->curr_tssi_cck_de[RF_PATH_A] =
		(s8)(halrf_rreg(rf, 0x5858, 0x003ff000) & 0xff);
	tssi_info->curr_tssi_ofdm_de[RF_PATH_A] =
		(s8)(halrf_rreg(rf, 0x5838, 0x003ff000) & 0xff);
	tssi_info->curr_tssi_efuse_cck_de[RF_PATH_A] = (s8)s_tssi_de;
	tssi_info->curr_tssi_efuse_ofdm_de[RF_PATH_A] = (s8)s_tssi_de;
	tssi_info->curr_tssi_trim_de[RF_PATH_A] = tssi_trim;

}

void halrf_set_tssi_de_offset_8730e(struct rf_info *rf, u32 tssi_de_offset)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	s32 tssi_tmp, s_tssi_de_offset;

	if (tssi_de_offset & BIT(7)) {
		s_tssi_de_offset = tssi_de_offset | 0xffffff00;
	} else {
		s_tssi_de_offset = tssi_de_offset;
	}

	s_tssi_de_offset = s_tssi_de_offset * -1;


	tssi_tmp = tssi_info->tssi_de[RF_PATH_A] + s_tssi_de_offset;
	tssi_tmp = tssi_tmp & 0x3ff;
	halrf_wreg(rf, 0x5838, 0x003ff000, tssi_tmp);	/*20M*/
	halrf_wreg(rf, 0x5858, 0x003ff000, tssi_tmp);	/*CCK long*/
	halrf_wreg(rf, 0x5828, 0x003ff000, tssi_tmp);	/*5M*/
	halrf_wreg(rf, 0x5830, 0x003ff000, tssi_tmp);	/*10M*/
	halrf_wreg(rf, 0x5860, 0x003ff000, tssi_tmp);	/*CCK short*/
	/*
		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] tssi_tmp(%d) = tssi_info->tssi_de[RF_PATH_A](%d) + s_tssi_de_offset(%d) path=%d\n",
		       tssi_tmp, tssi_info->tssi_de[RF_PATH_A],
		s_tssi_de_offset, RF_PATH_A);

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Set TSSI DE 0x5838[21:12]=0x%x 0x5858[21:12]=0x%x path=%d\n",
		       halrf_rreg(rf, 0x5838, 0x003ff000),
		       halrf_rreg(rf, 0x5858, 0x003ff000),
		RF_PATH_A);
	*/
}

void halrf_set_tssi_de_offset_zero_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5838, 0x003ff000, 0x0);	/*20M*/
	halrf_wreg(rf, 0x5858, 0x003ff000, 0x0);	/*CCK long*/
	halrf_wreg(rf, 0x5828, 0x003ff000, 0x0);	/*5M*/
	halrf_wreg(rf, 0x5830, 0x003ff000, 0x0);	/*10M*/
	halrf_wreg(rf, 0x5860, 0x003ff000, 0x0);	/*CCK short*/
}

void halrf_tssi_enable_8730e(struct rf_info *rf)
{

	_halrf_tssi_set_tssi_track_8730e(rf);
	_halrf_tssi_set_txagc_offset_mv_avg_8730e(rf);
	halrf_wreg(rf, 0x58e4, 0x00004000, 0x0);
	halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5820, 0x80000000, 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x10055, 0x00080, 0x1);
	halrf_wreg(rf, 0x5818, 0x18000000, 0x3);

	rf->is_tssi_mode[RF_PATH_A] = true;

}


void halrf_tssi_disable_8730e(struct rf_info *rf)
{
	halrf_wreg(rf, 0x5820, 0x80000000, 0x0);
	halrf_wreg(rf, 0x5818, 0x10000000, 0x0);
	halrf_wreg(rf, 0x58e4, 0x00004000, 0x1);

	rf->is_tssi_mode[RF_PATH_A] = false;

}

s32 halrf_get_online_tssi_de_8730e(struct rf_info *rf, s32 dbm, s32 puot)
{
	s32 de = ((puot - dbm) * 8) / 100;
	s32 s_tssi_offset;
	u32 tssi_offset;
	s8 tssi_trim;

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======> %s   phy=%d\n", __func__, phy_idx);*/


	tssi_offset = halrf_rreg(rf, 0x5838, 0x003ff000);
	/*tssi_offset = halrf_rreg(rf, 0x5858, 0x003ff000);*/

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Set TSSI DE 0x5838[21:12]=0x%x\n",
		tssi_offset);*/


	if (tssi_offset & BIT(9)) {
		s_tssi_offset = tssi_offset | 0xfffffc00;
	} else {
		s_tssi_offset = tssi_offset;
	}

	tssi_trim = _halrf_tssi_get_ofdm_tssi_trim_de_8730e(rf);


	RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] de(%d) = de(%d) + s_tssi_offset(%d) - tssi_trim(%d)\n",
	       (de + s_tssi_offset - tssi_trim), de, s_tssi_offset, tssi_trim);

	de = de + s_tssi_offset - tssi_trim;

	de = de & 0xff;

	return de;
}

void halrf_tssi_set_efuse_to_de_8730e(struct rf_info *rf)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u32 i, group_idx;
	s32 tmp;
	s8 ofmd_de, tssi_trim;

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======> %s\n", __func__);*/
	i = 0;
	if (tssi_info->tssi_type[HW_PHY_0] == TSSI_CAL) {
		/*halrf_set_tssi_de_offset_zero_8730e(rf, phy);*/
		//RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "======> %s TSSI_CAL mode set TSSI Trim !!!\n", __func__);
		tssi_trim = _halrf_tssi_get_ofdm_tssi_trim_de_8730e(rf);
		tmp = (s32)tssi_trim;
		halrf_wreg(rf, 0x5858, 0x003ff000, (u32)(tmp & 0x3ff));
		halrf_wreg(rf, 0x5860, 0x003ff000, (u32)(tmp & 0x3ff));

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] TSSI_CAL Set TSSI Trim CCK DE 0x%x[21:12]=0x%x\n",
		       0x5858, halrf_rreg(rf, 0x5858, 0x003ff000));
		halrf_wreg(rf, 0x5838, 0x003ff000, (u32)(tmp & 0x3ff));
		halrf_wreg(rf, 0x5828, 0x003ff000, (u32)(tmp & 0x3ff));
		halrf_wreg(rf, 0x5830, 0x003ff000, (u32)(tmp & 0x3ff));

		RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] TSSI_CAL Set TSSI Trim MCS DE 0x%x[21:12]=0x%x\n",
		       0x5838, halrf_rreg(rf, 0x5838, 0x003ff000));

		/*Save TSSI data for WINCLI*/
		tssi_info->curr_tssi_cck_de[i] =
			(s8)(halrf_rreg(rf, 0x5858, 0x003ff000) & 0xff);
		tssi_info->curr_tssi_ofdm_de[i] =
			(s8)(halrf_rreg(rf, 0x5838, 0x003ff000) & 0xff);
		tssi_info->curr_tssi_efuse_cck_de[i] = 0;
		tssi_info->curr_tssi_efuse_ofdm_de[i] = 0;
		tssi_info->curr_tssi_trim_de[i] = tssi_trim;
		return;
	}


	/*CCK*/
	group_idx = _halrf_tssi_get_cck_efuse_group_8730e(rf);
	tssi_trim = _halrf_tssi_get_ofdm_tssi_trim_de_8730e(rf);

	tmp = tssi_info->tssi_efuse[i][EFUSE_TSSI_CCK][group_idx] +
	      tssi_trim;


	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI] tmp(0x%x) = tssi_efuse_cck[%d](0x%x) + tssi_trim(0x%x)\n",
	       tmp, group_idx, tssi_info->tssi_efuse[i][EFUSE_TSSI_CCK][group_idx],
	       tssi_trim);

	halrf_wreg(rf, 0x5858, 0x003ff000,
		   (u32)(tmp & 0x3ff));
	halrf_wreg(rf, 0x5860, 0x003ff000,
		   (u32)(tmp & 0x3ff));

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Set TSSI CCK DE 0x%x[21:12]=0x%x\n",
		0x5858, halrf_rreg(rf, 0x5858, 0x003ff000));*/

	/*OFDM*/
	ofmd_de = _halrf_tssi_get_ofdm_efuse_tssi_de_8730e(rf);
	tssi_trim = _halrf_tssi_get_ofdm_tssi_trim_de_8730e(rf);
	tmp = ofmd_de + tssi_trim;


	RF_DBG(rf, DBG_RF_TX_PWR_TRACK,
	       "[TSSI] tmp(0x%x) = tssi_efuse_ofdm(0x%x) + tssi_trim(0x%x) \n",
	       tmp, ofmd_de, tssi_trim);
	halrf_wreg(rf, 0x5838, 0x003ff000,
		   (u32)(tmp & 0x3ff));
	halrf_wreg(rf, 0x5828, 0x003ff000,
		   (u32)(tmp & 0x3ff));
	halrf_wreg(rf, 0x5830, 0x003ff000,
		   (u32)(tmp & 0x3ff));

	/*RF_DBG(rf, DBG_RF_TX_PWR_TRACK, "[TSSI] Set TSSI MCS DE 0x%x[21:12]=0x%x\n",
		0x5838, halrf_rreg(rf, 0x5838, 0x003ff000));*/

	/*Save TSSI data for WINCLI*/
	tssi_info->curr_tssi_cck_de[i] =
		(s8)(halrf_rreg(rf, 0x5858, 0x003ff000) & 0xff);
	tssi_info->curr_tssi_ofdm_de[i] =
		(s8)(halrf_rreg(rf, 0x5838, 0x003ff000) & 0xff);
	tssi_info->curr_tssi_efuse_cck_de[i] = tssi_info->tssi_efuse[i][EFUSE_TSSI_CCK][group_idx];
	tssi_info->curr_tssi_efuse_ofdm_de[i] = ofmd_de;
	tssi_info->curr_tssi_trim_de[i] = tssi_trim;

}

void halrf_tssi_ant_open_8730e(struct rf_info *rf)
{
	return;
}

#if 0
void halrf_get_tssi_info_8730e(struct rf_info *rf,
			       char input[][16], u32 *_used, char *output, u32 *_out_len)
{
	struct halrf_tssi_info *tssi_info = &rf->tssi;
	u32 tmp, tmp1, pg_ther, cur_ther;
	s32 delta_tssi;
	s32 diff_ther;
	s32 int_tmp[TSSI_PATH_MAX_8730E], float_tmp[TSSI_PATH_MAX_8730E];
	s8 txagc_offset[TSSI_PATH_MAX_8730E] = {0};

	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = %x\n",
		    "RF Para Ver", halrf_get_radio_ver_from_reg(rf));

	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = 0x%x \n",
		    "TSSI DCK A ", halrf_rreg(rf, 0x1c04, 0x00FFF000));

	tmp = halrf_rreg(rf, 0x1c78, 0x1ff);
	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = %d.%d \n",
		    "T-MAC xdbm A / B",
		    tmp / 4, tmp * 100 / 4 % 100);

	halrf_wreg(rf, 0x58b4, 0x0000001f, 0x7);
	tmp = halrf_rreg(rf, 0x1c00, 0x0ffc0000);
	if (tmp & BIT(9)) {
		delta_tssi = tmp | 0xfffffc00;
	} else {
		delta_tssi = tmp;
	}
	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = %d / %d / %d\n",
		    "TSSI C / Final / Delta A", halrf_rreg(rf, 0x1c00, 0x000001ff),
		    halrf_rreg(rf, 0x1c00, 0x0003fe00), delta_tssi);

	pg_ther = halrf_rreg(rf, 0x5810, 0x0000fc00);
	cur_ther = halrf_rreg(rf, 0x1c10, 0xff000000);
	diff_ther = pg_ther - cur_ther;
	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = %d / %d / %d / %d\n",
		    "TherA PG / Cur / Diff / F", ((tssi_info->thermal[RF_PATH_A] == 0xff) ? 0xff : pg_ther),
		    cur_ther, diff_ther,
		    (s8)halrf_rreg(rf, 0x1c08, 0xff000000));

	txagc_offset[RF_PATH_A] = (s8)halrf_rreg(rf, 0x1c60, 0xff000000);
	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = 0x%x (%d) \n",
		    "TXAGC Offset A ",
		    (txagc_offset[RF_PATH_A] & 0xff), txagc_offset[RF_PATH_A]);

	int_tmp[RF_PATH_A] = txagc_offset[RF_PATH_A] * 125 / 1000;
	float_tmp[RF_PATH_A] = txagc_offset[RF_PATH_A] * 125 % 1000;
	float_tmp[RF_PATH_A] < 0 ? float_tmp[RF_PATH_A] = float_tmp[RF_PATH_A] * -1 : 0;
	RF_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used, " %-25s = %s%d.%ddB \n",
		    "TXAGC Offset dB A",
		    (int_tmp[RF_PATH_A] == 0 && txagc_offset[RF_PATH_A] < 0) ? "-" : "",
		    int_tmp[RF_PATH_A], float_tmp[RF_PATH_A]);
}
#endif
#endif	/*RF_8730E_SUPPORT*/
