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

void _halrf_psd_backup_bb_registers_8730e(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)
{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		reg_backup[i] = halrf_rreg(rf, reg[i], MASKDWORD);

		//RF_DBG(rf, DBG_RF_PSD, "[PSD] Backup BB 0x%08x = 0x%08x\n",
//			reg[i], reg_backup[i]);

	}
}

void _halrf_psd_reload_bb_registers_8730e(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)

{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

		//RF_DBG(rf, DBG_RF_PSD, "[PSD] Reload BB 0x%08x = 0x%08x\n",
//			reg[i], reg_backup[i]);

	}
}

void _halrf_psd_bkup_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg)
{
	struct halrf_psd_data *psd_info = &rf->psd;
	u8 i;

	for (i = 0; i < PSD_RF_REG_NUM_8730E; i++) {
		psd_info->rf_bkup[0][i] = halrf_rrf(rf, RF_PATH_A, rf_reg[i], MASKRF);

//		RF_DBG(rf, DBG_RF_PSD, "[IQK_PSD] Backup RF 0x%x = %x\n",
//			rf_reg[i], psd_info->rf_bkup[0][i]);
	}
}

void _halrf_psd_reload_rf_8730e(
	struct rf_info *rf,
	u32 *rf_reg)
{
	struct halrf_psd_data *psd_info = &rf->psd;
	u8 i;

	for (i = 0; i < PSD_RF_REG_NUM_8730E; i++) {
		halrf_wrf(rf, RF_PATH_A, rf_reg[i], MASKRF, psd_info->rf_bkup[0][i]);

//		RF_DBG(rf, DBG_RF_PSD, "[IQK_PSD] Reload RF 0x%x = %x\n",
//			rf_reg[i], psd_info->rf_bkup[0][i]);
	}
}

void halrf_psd_init_8730e(struct rf_info *rf, enum phl_phy_idx phy,
			  u8 path, u8 iq_path, u32 avg, u32 fft)
{
	struct halrf_psd_data *psd_info = &rf->psd;

	u32 bb_reg[PSD_BACKUP_NUM_8730E] = {
		0x15864, 0x12008, 0x10c60, 0x10c6c, 0x158ac,
		0x10c80, 0x120fc, 0x15670, 0x112a0, 0x1030c,
		0x1032c, 0x112b8, 0x18080, 0x18000, 0x18018,
		0x18014, 0x1801c, 0x1800c, 0x180cc, 0x18008
	};


	u32 rf_reg[PSD_RF_REG_NUM_8730E] = {0x0, 0x5, 0x8f, 0x10000, 0x10005};

	RF_DBG(rf, DBG_RF_PSD, "======> %s\n", __func__);

	//printf("======> %s\n", __func__);

	if (psd_info->psd_progress == 0) {

		_halrf_psd_backup_bb_registers_8730e(rf, phy, bb_reg,
						     psd_info->psd_reg_backup, PSD_BACKUP_NUM_8730E);

		_halrf_psd_bkup_rf_8730e(rf, rf_reg);

	}
	psd_info->psd_progress = 1;

	psd_info->path = path;
	psd_info->iq_path = iq_path;
	psd_info->avg = avg;
	psd_info->fft = fft;

	RF_DBG(rf, DBG_RF_PSD, "[PSD] fft = %d, avg = %d, iq = %d\n",
	       fft, avg, iq_path);

	halrf_wrf(rf, path, 0x5, 0x00001, 0x0);
	halrf_wrf(rf, path, 0x0, 0xF0000, 0x3);

	/*01_ABLite_PSD_AFE_ON_BB_settings_20220519*/
	halrf_wreg(rf, 0x15864, 0x18000000, 0x3);
	halrf_wreg(rf, 0x12008, 0x01FFFFFF, 0x00fffff);
	halrf_wreg(rf, 0x10c60, 0x3, 0x3);
	halrf_wreg(rf, 0x10c6c, 0x1, 0x1);
	halrf_wreg(rf, 0x158ac, 0x08000000, 0x1);
	halrf_wreg(rf, 0x10c80, 0x80000000, 0x1);
	halrf_wreg(rf, 0x120fc, 0x00010000, 0x1);  //bit 16
	halrf_wreg(rf, 0x120fc, 0x00100000, 0x0);  //bit 20
	halrf_wreg(rf, 0x120fc, 0x01000000, 0x1);  //bit 24
	halrf_wreg(rf, 0x120fc, 0x10000000, 0x0);  //bit 28
	halrf_wreg(rf, 0x15670, 0xffffffff, 0xF801fffd);
	halrf_wreg(rf, 0x112a0, 0x00008000, 0x1);
	halrf_wreg(rf, 0x15670, 0x80000000, 0x1);
	halrf_wreg(rf, 0x112a0, 0x00007000, 0x7);
	halrf_wreg(rf, 0x112a0, 0x00080000, 0x1);  //bit 19
	halrf_wreg(rf, 0x112a0, 0x00070000, 0x6);
	halrf_wreg(rf, 0x15670, 0x60000000, 0x1);
	halrf_wreg(rf, 0x1030c, 0xFF000000, 0x07);
	halrf_wreg(rf, 0x1032c, 0xFFFF0000, 0x0001);
	halrf_wreg(rf, 0x112b8, 0x10000000, 0x1);  //bit 28
	halrf_wreg(rf, 0x120fc, 0x00100000, 0x1);  //bit 20
	halrf_wreg(rf, 0x120fc, 0x10000000, 0x1);  //bit 28
	/*02*/
	halrf_wreg(rf, 0x18080, 0xffffffff, 0x00000006);
	halrf_wreg(rf, 0x18008, 0xffffffff, 0x00000280);
	halrf_wreg(rf, 0x18018, 0xffffffff, 0x40010101);

#if 1
	if (fft == 160) {
		halrf_wreg(rf, 0x1801c, 0x00003000, 0x0);
	} else if (fft == 320) {
		halrf_wreg(rf, 0x1801c, 0x00003000, 0x1);
	} else if (fft == 640) {
		halrf_wreg(rf, 0x1801c, 0x00003000, 0x2);
	} else { /*1280*/
		halrf_wreg(rf, 0x1801c, 0x00003000, 0x3);
	}

	if (avg == 1) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x0);
	} else if (avg == 2) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x1);
	} else if (avg == 4) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x2);
	} else if (avg == 8) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x3);
	} else if (avg == 16) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x4);
	} else if (avg == 64) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x6);
	} else if (avg == 128) {
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x7);
	} else {	/*32*/
		halrf_wreg(rf, 0x1801c, 0x000e0000, 0x5);
	}

	halrf_wreg(rf, 0x18018, 0x00000006, iq_path);

#endif

	halrf_wreg(rf, 0x18000, 0xffffffff, 0x00000008);

	halrf_wreg(rf, 0x18014, 0xffffffff, 0x40010100);

	halrf_wreg(rf, 0x1800c, 0x00000C00, 0x3);
	halrf_wreg(rf, 0x180cc, 0x0000003F, 0x3F);
}

void halrf_psd_restore_8730e(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct halrf_psd_data *psd_info = &rf->psd;
	u32 bb_reg[PSD_BACKUP_NUM_8730E] = {
		0x15864, 0x12008, 0x10c60, 0x10c6c, 0x158ac,
		0x10c80, 0x120fc, 0x15670, 0x112a0, 0x1030c,
		0x1032c, 0x112b8, 0x18080, 0x18000, 0x18018,
		0x18014, 0x1801c, 0x1800c, 0x180cc, 0x18008
	};
	u32 rf_reg[PSD_RF_REG_NUM_8730E] = {0x0, 0x5, 0x8f, 0x10000, 0x10005};

	RF_DBG(rf, DBG_RF_PSD, "======> %s\n", __func__);

	/*10/11.txt*/
	halrf_wreg(rf, 0x18080, 0xffffffff, 0x00000002);
	halrf_wreg(rf, 0x18008, 0xffffffff, 0x00000000);
	halrf_wreg(rf, 0x112b8, 0x10000000, 0x0);

	halrf_wreg(rf, 0x120fc, 0x00010000, 0x1);  //bit 16
	halrf_wreg(rf, 0x120fc, 0x00100000, 0x0);  //bit 20
	halrf_wreg(rf, 0x120fc, 0x01000000, 0x1);  //bit 24
	halrf_wreg(rf, 0x120fc, 0x10000000, 0x0);  //bit 28

	halrf_wreg(rf, 0x15670, 0xffffffff, 0x00000000);
	halrf_wreg(rf, 0x112a0, 0x000FF000, 0x00);
	halrf_wreg(rf, 0x120fc, 0x00010000, 0x0);  //bit 16
	halrf_wreg(rf, 0x120fc, 0x01000000, 0x0);  //bit 24

	halrf_wrf(rf, RF_PATH_A, 0xEE, 0x80000, 0x0);
	halrf_wrf(rf, RF_PATH_A, 0x5, 0x00001, 0x1);

	if (psd_info->psd_progress == 1) {
		_halrf_psd_reload_bb_registers_8730e(rf, phy, bb_reg,
						     psd_info->psd_reg_backup, PSD_BACKUP_NUM_8730E);

		_halrf_psd_reload_rf_8730e(rf, rf_reg);
	}

	psd_info->psd_progress = 0;
}

u32 halrf_psd_get_point_data_8730e(struct rf_info *rf,
				   enum phl_phy_idx phy, s32 point)
{
	struct halrf_psd_data *psd_info = &rf->psd;
	u32 val_tmp, val, data;

	//halrf_wrf(rf, RF_PATH_A, 0x5, 0x00001, 0x0);
	halrf_wreg(rf, 0x1802c, 0x0fff0000, (point & 0xfff));
	halrf_wreg(rf, 0x18034, 0x00000001, 0x1);
	halrf_wreg(rf, 0x18034, 0x00000001, 0x0);
	if (point == 0) {
		halrf_delay_ms(rf, 5);
	} else {
		halrf_delay_ms(rf, 1);
	}
	//halrf_delay_us(rf, 100);

	halrf_wreg(rf, 0x180d4, 0xffffffff, 0x00210001);
	val_tmp = halrf_rreg(rf, 0x180fc, 0x00ff0000);
	halrf_wreg(rf, 0x180d4, 0xffffffff, 0x00220001);
	val = halrf_rreg(rf, 0x180fc, 0xffffffff);

	//data = (val_tmp << 24) | (val >> 8);  //10log255 = 24db, drown in the noise
	data = (val_tmp << 26) | (val >> 6);  //10log64 = 18db, drown in the noise
	/*full power on bit37*/

	//RF_DBG(rf, DBG_RF_PSD, "======> %s  point=0x%x  data=0x%08x, val=0x%08x\n",
	//	__func__, point, data ,val);
	//RF_DBG(rf, DBG_RF_PSD, "point=0x%x  data=0x%08x\n",
	//	point, data);

	return data;
}

void halrf_psd_query_8730e(struct rf_info *rf, enum phl_phy_idx phy,
			   u32 point, u32 start_point, u32 stop_point, u32 *outbuf)
{
	struct halrf_psd_data *psd_info = &rf->psd;
	struct rtw_hal_com_t *hal = rf->hal_com;
	u32 i = 0, j = 0;
	s32 point_temp;

	RF_DBG(rf, DBG_RF_PSD, "======> %s point=%d start_point=%d stop_point=%d\n",
	       __func__, point, start_point, stop_point);

	if (psd_info->psd_result_running == true) {
		RF_DBG(rf, DBG_RF_PSD, "======> %s PSD Running Return !!!\n", __func__);
		return;
	}

	psd_info->psd_result_running = true;

	hal_mem_set(hal, psd_info->psd_data, 0, sizeof(psd_info->psd_data));

	i = start_point;
	while (i < stop_point) {
		if (i >= point) {
			point_temp = i - point;
		} else {
			point_temp = i - point;
			point_temp = point_temp & 0xfff;
		}

		psd_info->psd_data[j] = halrf_psd_get_point_data_8730e(rf, phy, point_temp);

		i++;
		j++;
	}
	/*
	RF_DBG(rf, DBG_RF_PSD, "psd_info->psd_data\n");

	for (i = 0; i < 320; i = i + 10) {
		RF_DBG(rf, DBG_RF_PSD, "%d  %d	%d  %d	%d  %d	%d  %d	%d  %d\n",
			psd_info->psd_data[i], psd_info->psd_data[i + 1],
			psd_info->psd_data[i + 2], psd_info->psd_data[i + 3],
			psd_info->psd_data[i + 4], psd_info->psd_data[i + 5],
			psd_info->psd_data[i + 6], psd_info->psd_data[i + 7],
			psd_info->psd_data[i + 8], psd_info->psd_data[i + 9]);
	}

	RF_DBG(rf, DBG_RF_PSD, "======> %s PSD End !!!\n", __func__);
	*/
	hal_mem_cpy(hal, outbuf, psd_info->psd_data, 320 * 4);

	psd_info->psd_result_running = false;
}

#endif	/*RF_8730E_SUPPORT*/