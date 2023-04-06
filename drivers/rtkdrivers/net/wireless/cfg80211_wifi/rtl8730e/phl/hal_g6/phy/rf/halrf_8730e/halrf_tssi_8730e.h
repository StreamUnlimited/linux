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
#ifndef _HALRF_TSSI_8730E_H_
#define _HALRF_TSSI_8730E_H_
#ifdef RF_8730E_SUPPORT

#define TSSI_VER_8730E 0x0A

#define TSSI_PATH_MAX_8730E 1

/*@--------------------------Define Parameters-------------------------------*/


/*@-----------------------End Define Parameters-----------------------*/
void halrf_tssi_get_efuse_8730e(struct rf_info *rf);
void halrf_set_tssi_de_for_tx_verify_8730e(struct rf_info *rf,
		u32 tssi_de);
void halrf_set_tssi_de_offset_8730e(struct rf_info *rf,
				    u32 tssi_de_offset);
void halrf_set_tssi_de_offset_zero_8730e(struct rf_info *rf);
void halrf_do_tssi_8730e(struct rf_info *rf);
void halrf_tssi_tracking_8730e(struct rf_info *rf);
void halrf_tssi_ant_open_8730e(struct rf_info *rf);
void halrf_do_tssi_init_8730e(struct rf_info *rf);
void halrf_do_tssi_scan_8730e(struct rf_info *rf);
void halrf_tssi_enable_8730e(struct rf_info *rf);
void halrf_tssi_disable_8730e(struct rf_info *rf);
s32 halrf_get_online_tssi_de_8730e(struct rf_info *rf,
				   s32 dbm, s32 puot);
void halrf_tssi_set_efuse_to_de_8730e(struct rf_info *rf);
#if 0
void halrf_get_tssi_info_8730e(struct rf_info *rf,
			       char input[][16], u32 *_used, char *output, u32 *_out_len);
void halrf_tssi_default_txagc_8730e(struct rf_info *rf, bool enable);
void halrf_tssi_scan_ch_8730e(struct rf_info *rf, enum rf_path path);
void halrf_tssi_backup_txagc_8730e(struct rf_info *rf, bool enable);
u32 halrf_tssi_get_final_8730e(struct rf_info *rf, enum rf_path path);
#endif
#endif
#endif	/*_HALRF_SET_PWR_TABLE_8730E_H_*/
