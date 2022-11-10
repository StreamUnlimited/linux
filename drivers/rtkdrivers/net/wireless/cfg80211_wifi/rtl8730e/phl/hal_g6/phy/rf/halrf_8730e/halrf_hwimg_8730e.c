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
#include "halrf_hwimg_raw_data_8730e.h"
#include "halrf_hwimg_nctl_raw_data_8730e.h"

bool halrf_check_cond_8730e(struct rf_info *rf, u32 para_opt)
{
	struct rtw_hal_com_t *hal = rf->hal_com;
	u32 cv_ver = (hal->cv == CAV) ? 15 : hal->cv;
	/*[20200204][Dino]Request from SD7 Sinda, and need Sinda to tell us what is the correct pkg_type/rfe_type parameters in PHL layer for halbb reference*/
	u8 pkg_type = 0; /*(hal->efuse->pkg_type == 0) ? 15 : hal->efuse->pkg_type;*/
	u8 rfe_type = 0; /*hal->efuse->rfe_type;*/
	u32 drv_cfg = cv_ver << 16 | pkg_type << 8 | rfe_type;
	u32 para_opt_tmp = 0;

	/*============== value Defined Check ===============*/
	/*Cart ver BIT[23:16]*/
	para_opt_tmp = para_opt & 0xff0000;
	if (para_opt_tmp && (para_opt_tmp != (drv_cfg & 0xff0000))) {
		return false;
	}

	/*PKG type, BIT[15:8]*/
	para_opt_tmp = para_opt & 0xff00;
	if (para_opt_tmp && (para_opt_tmp != (drv_cfg & 0xff00))) {
		return false;
	}

	/*RFE, BIT[7:0]*/
	para_opt_tmp = para_opt & 0xff;
	if (para_opt_tmp && (para_opt_tmp != (drv_cfg & 0xff))) {
		return false;
	}

	return true;
}

u32
halrf_get_8730e_nctl_reg_ver(void)
{
	return 0x5;
}

u32
halrf_get_8730e_radio_reg_ver(void)
{
	return RF_RELEASE_VERSION_8730E;
}

void halrf_config_8730e_store_radio_a_reg(struct rf_info *rf,
		u32 addr, u32 data)
{
	struct halrf_radio_info *radio = &rf->radio_info;
	u32 page = radio->write_times_a / RADIO_TO_FW_DATA_SIZE;
	u32 idx = radio->write_times_a % RADIO_TO_FW_DATA_SIZE;
	u32 reg_tmp = 0;

	RF_DBG(rf, DBG_RF_INIT, "======> %s\n", __func__);

	if (addr == 0xfe || addr == 0xfd || addr == 0xfc || addr == 0xfb ||
	    addr == 0xfa || addr == 0xf9) {
		RF_DBG(rf, DBG_RF_INIT, "Radio parameter is delay return!!!\n");
		return;
	}

	if (data > 0xfffff) {
		RF_DBG(rf, DBG_RF_INIT, "Radio parameter format error !!!\n");
	}

	/*DRFC only*/
	if (addr < 0x100) {
		return;
	}

	addr &= 0xff;
	reg_tmp = cpu_to_le32((addr << 20) | data);
	radio->radio_a_parameter[page][idx] = reg_tmp;

	RF_DBG(rf, DBG_RF_INIT, "radioA->radio_parameter[%d][%03d]=0x%08x\n",
	       page, idx, radio->radio_a_parameter[page][idx]);

	radio->write_times_a++;
}

void halrf_config_8730e_write_radio_a_reg_to_fw(struct rf_info *rf)
{
	struct halrf_radio_info *radio = &rf->radio_info;
	u8 page = (u8)(radio->write_times_a / RADIO_TO_FW_DATA_SIZE);
	u16 len = (radio->write_times_a % RADIO_TO_FW_DATA_SIZE) * 4;
	u8 i;

	RF_DBG(rf, DBG_RF_INIT, "======> %s   write_times_a=%d  page=%d  len=%d\n",
	       __func__, radio->write_times_a, page, len / 4);

	for (i = 0; i < page; i++) {
		halrf_fill_h2c_cmd(rf, RADIO_TO_FW_DATA_SIZE * 4, i, 8,
				   H2CB_TYPE_LONG_DATA, radio->radio_a_parameter[i]);
		RF_DBG(rf, DBG_RF_INIT, "page=%d   len=%d\n", i, len / 4);
	}
	halrf_fill_h2c_cmd(rf, len, i, 8,
			   H2CB_TYPE_LONG_DATA, radio->radio_a_parameter[i]);
	RF_DBG(rf, DBG_RF_INIT, "page=%d   len=%d\n", i, len / 4);
}

void halrf_config_8730e_radio_to_fw(struct rf_info *rf)
{
	halrf_config_8730e_write_radio_a_reg_to_fw(rf);
	//halrf_config_8730e_write_radio_b_reg_to_fw(rf);
}

void halrf_config_8730e_nctl_reg(struct rf_info *rf)
{
	u32	i = 0;
	u32	array_len = 0x0;
	u32 	*array = NULL;
	u32	v1 = 0, v2 = 0;
	u32 	cnt = 0x0;


	RF_DBG(rf, DBG_RF_INIT, "[RFK]===> %s\n", __func__);

	halrf_wreg(rf, 0x0c60, 0x00000003, 0x3);
	halrf_wreg(rf, 0x0c6c, 0x00000001, 0x1);
	halrf_wreg(rf, 0x58ac, 0x08000000, 0x1);
	halrf_wreg(rf, 0x0c80, 0x80000000, 0x1);
	array_len = sizeof(array_mp_8730e_nctl_reg) / sizeof(u32);
	array = (u32 *) &array_mp_8730e_nctl_reg;

	//halrf_write_fwofld_start(rf);
	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];
		halrf_cfg_rf_nctl_8730e(rf, v1, MASKDWORD, v2);
		i += 2;
	}
	//halrf_write_fwofld_end(rf);
}

bool halrf_sel_headline_8730e(struct rf_info *rf, u32 *array, u32 array_len,
			      u8 *headline_size, u8 *headline_idx)
{
	bool case_match = false;
	u8 cv_drv_org = rf->hal_com->cv;
	u8 rfe_drv_org = rf->phl_com->dev_cap.rfe_type;
	u32 cv_para = 0, rfe_para = 0;
	u32 compare_target = 0;
	u32 cv_max = 0;
	u32 i = 0;
	u32 cv_drv = (u32)cv_drv_org;
	u32 rfe_drv = (u32)rfe_drv_org;

	*headline_idx = 0;
	*headline_size = 0;

#if 0
	if (rf->rf_dbg_i.cr_dbg_mode_en) {
		rfe_drv = rf->rf_dbg_i.rfe_type_curr_dbg;
		cv_drv = rf->rf_dbg_i.cv_curr_dbg;
	}

	RF_DBG(rf, DBG_RF_INIT, "{RFE, Cart}={%d, %d}, dbg_en=%d\n",
	       rfe_drv, cv_drv, rf->rf_dbg_i.cr_dbg_mode_en);
#endif

	RF_DBG(rf, DBG_RF_INIT, "{RFE, Cart}={%d, %d}\n",
	       rfe_drv, cv_drv);

	while ((i + 1) < array_len) {
		if ((array[i] >> 28) != 0xf) {
			*headline_size = (u8)i;
			break;
		}
		RF_DBG(rf, DBG_RF_INIT, "array[%02d]=0x%08x, array[%02d]=0x%08x\n",
		       i, array[i], i + 1, array[i + 1]);
		i += 2;
	}

	RF_DBG(rf, DBG_RF_INIT, "headline_size=%d\n", i);

	if (i == 0) {
		return true;
	}

	/*case_idx:1 {RFE:Match, cv:Match}*/
	compare_target = ((rfe_drv & 0xff) << 16) | (cv_drv & 0xff);
	RF_DBG(rf, DBG_RF_INIT, "[1] CHK {RFE:Match, cv:Match}\n");
	for (i = 0; i < *headline_size; i += 2) {
		if ((array[i] & 0x0fffffff) == compare_target) {
			*headline_idx = (u8)(i >> 1);
			return true;
		}
	}
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:2 {RFE:Match, cv:Dont care}*/
	compare_target = ((rfe_drv & 0xff) << 16) | (DONT_CARE_8730E & 0xff);
	RF_DBG(rf, DBG_RF_INIT, "[2] CHK {RFE:Match, cv:Dont_Care}\n");
	for (i = 0; i < *headline_size; i += 2) {
		if ((array[i] & 0x0fffffff) == compare_target) {
			*headline_idx = (u8)(i >> 1);
			return true;
		}
	}
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:3 {RFE:Match, cv:Max_in_table}*/
	RF_DBG(rf, DBG_RF_INIT, "[3] CHK {RFE:Match, cv:Max_in_Table}\n");
	for (i = 0; i < *headline_size; i += 2) {
		rfe_para = (array[i] & 0x00ff0000) >> 16;
		cv_para = array[i] & 0x0ff;
		if (rfe_para == rfe_drv) {
			if (cv_para >= cv_max) {
				cv_max = cv_para;
				*headline_idx = (u8)(i >> 1);
				RF_DBG(rf, DBG_RF_INIT, "cv_max:%d\n", cv_max);
				case_match = true;
			}
		}
	}
	if (case_match) {
		return true;
	}
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:4 {RFE:Dont Care, cv:Max_in_table}*/
	RF_DBG(rf, DBG_RF_INIT, "[4] CHK {RFE:Dont_Care, cv:Max_in_Table}\n");
	for (i = 0; i < *headline_size; i += 2) {
		rfe_para = (array[i] & 0x00ff0000) >> 16;
		cv_para = array[i] & 0x0ff;
		if (rfe_para == DONT_CARE_8730E) {
			if (cv_para >= cv_max) {
				cv_max = cv_para;
				*headline_idx = (u8)(i >> 1);
				RF_DBG(rf, DBG_RF_INIT, "cv_max:%d\n", cv_max);
				case_match = true;
			}
		}
	}
	if (case_match) {
		return true;
	}
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:5 {RFE:Not_Match, cv:Not_Match}*/
	RF_DBG(rf, DBG_RF_INIT, "[5] CHK {RFE:Not_Match, cv:Not_Match}\n");
	RF_DBG(rf, DBG_RF_INIT, "\t all fail\n");
	return false;
}

void halrf_flag_2_default_8730e(bool *is_matched, bool *find_target)
{
	*is_matched = true;
	*find_target = false;
}


void
halrf_config_8730e_radio_a_reg(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct rtw_hal_com_t *hal = rf->hal_com;
	struct halrf_radio_info *radio = &rf->radio_info;
	bool is_matched, find_target;
	u32 cfg_target = 0, cfg_para = 0;
	u32 i = 0;
	u32 array_len = 0;
	u32 *array = NULL;
	u32 v1 = 0, v2 = 0;
	u8 h_size = 0;
	u8 h_idx = 0;

	halrf_write_fwofld_start(rf);

	radio->write_times_a = 0;
	hal_mem_set(hal, radio->radio_a_parameter, 0, sizeof(radio->radio_a_parameter));

	array_len = sizeof(array_mp_8730e_radioa) / sizeof(u32);
	array = (u32 *)array_mp_8730e_radioa;


	if (!halrf_sel_headline_8730e(rf, array, array_len, &h_size, &h_idx)) {
		RF_WARNING("[%s]Invalid RF CR Pkg\n", __func__);
		return;
	}

	RF_DBG(rf, DBG_RF_INIT, "h_size = %d, h_idx = %d\n", h_size, h_idx);

	if (h_size != 0) {
		cfg_target = array[h_idx << 1] & 0x0fffffff;
	}

	i += h_size;

	RF_DBG(rf, DBG_RF_INIT, "cfg_target = 0x%x\n", cfg_target);
	RF_DBG(rf, DBG_RF_INIT, "array[i] = 0x%x, array[i+1] = 0x%x\n", array[i], array[i + 1]);

	halrf_flag_2_default_8730e(&is_matched, &find_target);
	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];
		i += 2;

		switch (v1 >> 28) {
		case IF_8730E:
		case ELSE_IF_8730E:
			cfg_para = v1 & 0x0fffffff;
			RF_DBG(rf, DBG_RF_INIT, "*if (rfe=%d, cart=%d)\n",
			       (cfg_para & 0xff0000) >> 16, cfg_para & 0xff);
			break;
		case ELSE_8730E:
			RF_DBG(rf, DBG_RF_INIT, "*else\n");
			is_matched = false;
			if (!find_target) {
				RF_WARNING("Init RFCR Fail in Reg 0x%x\n", array[i]);
				return;
			}
			break;
		case END_8730E:
			RF_DBG(rf, DBG_RF_INIT, "*endif\n");
			halrf_flag_2_default_8730e(&is_matched, &find_target);
			break;
		case CHK_8730E:
			/*Check this para meets driver's requirement or not*/
			if (find_target) {
				RF_DBG(rf, DBG_RF_INIT, "\t skip\n");
				is_matched = false;
				break;
			}

			if (cfg_para == cfg_target) {
				is_matched = true;
				find_target = true;
			} else {
				is_matched = false;
				find_target = false;
			}
			RF_DBG(rf, DBG_RF_INIT, "\t match=%d\n", is_matched);
			break;
		default:
			if (is_matched) {
				halrf_cfg_rf_radio_a_8730e(rf, v1, v2);
				halrf_config_8730e_store_radio_a_reg(rf, v1, v2);
			}
			break;
		}
	}

	halrf_write_fwofld_end(rf);
	RF_DBG(rf, DBG_RF_INIT, "RFCR Init Success\n");
	halrf_config_8730e_write_radio_a_reg_to_fw(rf);
}

void
halrf_config_8730e_store_power_by_rate(struct rf_info *rf,
				       enum phl_phy_idx phy)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	//struct rtw_para_info_t *phy_reg_info = NULL;

	u32 i, j;
	u32 array_len = 0;
	u32 *array = NULL;

	array_len = sizeof(array_mp_8730e_txpwr_byrate) / sizeof(u32);
	array = (u32 *)array_mp_8730e_txpwr_byrate;


	for (i = 0; i < array_len; i += 4) {
		u32	band = array[i];
		u32	tx_num = array[i + 1];
		u32	rate_id = array[i + 2];
		u32	data = array[i + 3];

		halrf_power_by_rate_store_to_array(rf, band, tx_num, rate_id, data);
	}

	for (i = 0; i < PW_LMT_MAX_BAND; i++)
		for (j = 0; j < HALRF_DATA_RATE_MAX; j++)
			RF_DBG(rf, DBG_RF_INIT, "pwr_by_rate[%d][%03d]=%d\n",
			       i, j, pwr->tx_pwr_by_rate[i][j]);

	/*compiler error*/
	pwr->tx_pwr_by_rate[0][0]++;
	pwr->tx_pwr_by_rate[0][0]--;
}

void
halrf_config_8730e_store_power_limit(struct rf_info *rf,
				     enum phl_phy_idx phy)
{
	const struct halrf_tx_pw_lmt *array = NULL;
	struct halrf_tx_pw_lmt *parray = NULL;
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u32 i;
	u32 array_len = 0;
	u8 band, bandwidth, tx_num, rate, beamforming, regulation, chnl, val;

	array_len = sizeof(array_mp_8730e_txpwr_lmt) / sizeof(struct halrf_tx_pw_lmt);
	array = array_mp_8730e_txpwr_lmt;

	for (i = 0; i < array_len; i++) {
		band = array[i].band;
		bandwidth = array[i].bw;
		tx_num = array[i].ntx;
		rate = array[i].rs;
		beamforming = array[i].bf;
		regulation = array[i].reg;
		chnl = array[i].ch;
		val = array[i].val;

		pwr->regulation[band][regulation] = true;

		if (rate == PW_LMT_RS_CCK) {
			pwr->tx_shap_idx[band][TX_SHAPE_CCK][regulation] = array[i].tx_shap_idx;
			RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx[%d][CCK][%d]=%d\n",
			       __func__, band, regulation,
			       pwr->tx_shap_idx[band][TX_SHAPE_CCK][regulation]);
		} else {
			pwr->tx_shap_idx[band][TX_SHAPE_OFDM][regulation] = array[i].tx_shap_idx;
			RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx[%d][OFDM][%d]=%d\n",
			       __func__, band, regulation,
			       pwr->tx_shap_idx[band][TX_SHAPE_OFDM][regulation]);
		}

		halrf_power_limit_store_to_array(rf, regulation, band, bandwidth,
						 rate, tx_num, beamforming, chnl, val);
	}

	halrf_power_limit_set_worldwide(rf);
	halrf_power_limit_set_ext_pwr_limit_table(rf, 0);
}


void
halrf_config_8730e_store_power_limit_ru(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	const struct halrf_tx_pw_lmt_ru *array = NULL;
	struct halrf_tx_pw_lmt_ru *parray = NULL;
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u32 i;
	u32 array_len = 0;
	u8 band, bandwidth, tx_num, rate, regulation, chnl, val;

	array_len = sizeof(array_mp_8730e_txpwr_lmt_ru) / sizeof(struct halrf_tx_pw_lmt_ru);
	array = array_mp_8730e_txpwr_lmt_ru;

	for (i = 0; i < array_len; i++) {
		band = array[i].band;
		bandwidth = array[i].bw;
		tx_num = array[i].ntx;
		rate = array[i].rs;
		regulation = array[i].reg;
		chnl = array[i].ch;
		val = array[i].val;

		if (rate == PW_LMT_RS_CCK) {
			pwr->tx_shap_idx[band][TX_SHAPE_CCK][regulation] = array[i].tx_shap_idx;
			RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx[%d][CCK][%d]=%d\n",
			       __func__, band, regulation,
			       pwr->tx_shap_idx[band][TX_SHAPE_CCK][regulation]);
		} else {
			pwr->tx_shap_idx[band][TX_SHAPE_OFDM][regulation] = array[i].tx_shap_idx;
			RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx[%d][OFDM][%d]=%d\n",
			       __func__, band, regulation,
			       pwr->tx_shap_idx[band][TX_SHAPE_OFDM][regulation]);
		}

		halrf_power_limit_ru_store_to_array(rf, band, bandwidth, tx_num,
						    rate, regulation, chnl, val);
	}

	halrf_power_limit_ru_set_worldwide(rf);
	halrf_power_limit_set_ext_pwr_limit_ru_table(rf, 0);
}

void
halrf_config_8730e_store_pwr_track(struct rf_info *rf,
				   enum phl_phy_idx phy)
{
	//struct halrf_pwr_track_info *tmp_info = NULL;
	struct halrf_pwr_track_info *pwr_trk = &rf->pwr_track;
	struct rtw_hal_com_t *hal = rf->hal_com;

	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_2ga_p,
		    (void *)delta_swingidx_mp_2ga_p_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE);
	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_2ga_n,
		    (void *)delta_swingidx_mp_2ga_n_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE);
	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_2g_cck_a_p,
		    (void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE);
	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_2g_cck_a_n,
		    (void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE);

	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_5ga_p,
		    (void *)delta_swingidx_mp_5ga_p_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE * 3);
	hal_mem_cpy(hal, pwr_trk->delta_swing_table_idx_5ga_n,
		    (void *)delta_swingidx_mp_5ga_n_txpwrtrkssi_8730e,
		    DELTA_SWINGIDX_SIZE * 3);

}

void
_halrf_config_rfe_xtal_track_table_8730e(struct rf_info *rf)
{
#if 0
	struct halrf_xtal_info *xtal_trk = &rf->xtal_track;
	struct rtw_hal_com_t *hal = rf->hal_com;

	hal_mem_cpy(hal, xtal_trk->delta_swing_xtal_table_idx_p,
		    (void *)delta_swing_xtal_mp_p_txxtaltrack_8730e,
		    DELTA_SWINGIDX_SIZE);
	hal_mem_cpy(hal, xtal_trk->delta_swing_xtal_table_idx_n,
		    (void *)delta_swing_xtal_mp_n_txxtaltrack_8730e,
		    DELTA_SWINGIDX_SIZE);
#endif
}

void
halrf_config_8730e_store_xtal_track(struct rf_info *rf,
				    enum phl_phy_idx phy)
{
#if 0
	//struct halrf_xtal_info *tmp_info = NULL;
	struct halrf_xtal_info *xtal_trk = &rf->xtal_track;
	struct rtw_hal_com_t *hal = rf->hal_com;

	_halrf_config_rfe_xtal_track_table_8730e(rf);
#else
	_halrf_config_rfe_xtal_track_table_8730e(rf);
#endif
}
