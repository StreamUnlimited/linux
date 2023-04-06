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
	return 0x9;
}

u32
halrf_get_8730e_radio_reg_ver(void)
{
	return RF_RELEASE_VERSION_8730E;
}

void halrf_config_8730e_nctl_reg(struct rf_info *rf)
{
	u16	i = 0;
	u16	array_len = 0x0;
	u16 	*array = NULL;
	u16	v1 = 0, v2 = 0;
	u32	v3 = 0;

	RF_DBG(rf, DBG_RF_INIT, "[RFK]===> %s\n", __func__);
	halrf_wreg(rf, 0x0c60, 0x00000003, 0x3);
	halrf_wreg(rf, 0x0c6c, 0x00000001, 0x1);
	halrf_wreg(rf, 0x58ac, 0x08000000, 0x1);
	halrf_wreg(rf, 0x0c80, 0x80000000, 0x1);
	array_len = sizeof(array_mp_8730e_nctl_reg) / sizeof(u16);
	array = (u16 *) &array_mp_8730e_nctl_reg;

	while ((i + 2) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];
		v3 = array[i + 2] + (u32)(v2 << 16);
		halrf_cfg_rf_nctl_8730e(rf, v1, MASKDWORD, v3);
		i += 3;
	}
}

#if 0
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
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "{RFE, Cart}={%d, %d}\n",
	       rfe_drv, cv_drv);
#endif
	while ((i + 1) < array_len) {
		if ((array[i] >> 28) != 0xf) {
			*headline_size = (u8)i;
			break;
		}
#ifdef	CONFIG_PHYDM_CMD
		RF_DBG(rf, DBG_RF_INIT, "array[%02d]=0x%08x, array[%02d]=0x%08x\n",
		       i, array[i], i + 1, array[i + 1]);
#endif
		i += 2;
	}
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "headline_size=%d\n", i);
#endif
	if (i == 0) {
		return true;
	}

	/*case_idx:1 {RFE:Match, cv:Match}*/
	compare_target = ((rfe_drv & 0xff) << 16) | (cv_drv & 0xff);
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "[1] CHK {RFE:Match, cv:Match}\n");
#endif
	for (i = 0; i < *headline_size; i += 2) {
		if ((array[i] & 0x0fffffff) == compare_target) {
			*headline_idx = (u8)(i >> 1);
			return true;
		}
	}
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");
#endif
	/*case_idx:2 {RFE:Match, cv:Dont care}*/
	compare_target = ((rfe_drv & 0xff) << 16) | (DONT_CARE_8730E & 0xff);
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "[2] CHK {RFE:Match, cv:Dont_Care}\n");
#endif
	for (i = 0; i < *headline_size; i += 2) {
		if ((array[i] & 0x0fffffff) == compare_target) {
			*headline_idx = (u8)(i >> 1);
			return true;
		}
	}
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");
	/*case_idx:3 {RFE:Match, cv:Max_in_table}*/
	RF_DBG(rf, DBG_RF_INIT, "[3] CHK {RFE:Match, cv:Max_in_Table}\n");
#endif
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
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:4 {RFE:Dont Care, cv:Max_in_table}*/
	RF_DBG(rf, DBG_RF_INIT, "[4] CHK {RFE:Dont_Care, cv:Max_in_Table}\n");
#endif
	for (i = 0; i < *headline_size; i += 2) {
		rfe_para = (array[i] & 0x00ff0000) >> 16;
		cv_para = array[i] & 0x0ff;
		if (rfe_para == DONT_CARE_8730E) {
			if (cv_para >= cv_max) {
				cv_max = cv_para;
				*headline_idx = (u8)(i >> 1);
#ifdef	CONFIG_PHYDM_CMD
				RF_DBG(rf, DBG_RF_INIT, "cv_max:%d\n", cv_max);
#endif
				case_match = true;
			}
		}
	}
	if (case_match) {
		return true;
	}
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "\t fail\n");

	/*case_idx:5 {RFE:Not_Match, cv:Not_Match}*/
	RF_DBG(rf, DBG_RF_INIT, "[5] CHK {RFE:Not_Match, cv:Not_Match}\n");
	RF_DBG(rf, DBG_RF_INIT, "\t all fail\n");
#endif
	return false;
}

void halrf_flag_2_default_8730e(bool *is_matched, bool *find_target)
{
	*is_matched = true;
	*find_target = false;
}

void
halrf_config_8730e_radio_a_reg(struct rf_info *rf)
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

	array_len = sizeof(array_mp_8730e_radioa) / sizeof(u32);
	array = (u32 *)array_mp_8730e_radioa;


	if (!halrf_sel_headline_8730e(rf, array, array_len, &h_size, &h_idx)) {
#ifdef	CONFIG_PHYDM_CMD
		RF_WARNING("[%s]Invalid RF CR Pkg\n", __func__);
#endif
		return;
	}
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "h_size = %d, h_idx = %d\n", h_size, h_idx);
#endif
	if (h_size != 0) {
		cfg_target = array[h_idx << 1] & 0x0fffffff;
	}

	i += h_size;
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "cfg_target = 0x%x\n", cfg_target);
	RF_DBG(rf, DBG_RF_INIT, "array[i] = 0x%x, array[i+1] = 0x%x\n", array[i], array[i + 1]);
#endif
	halrf_flag_2_default_8730e(&is_matched, &find_target);
	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];
		i += 2;

		switch (v1 >> 28) {
		case IF_8730E:
		case ELSE_IF_8730E:
			cfg_para = v1 & 0x0fffffff;
#ifdef	CONFIG_PHYDM_CMD
			RF_DBG(rf, DBG_RF_INIT, "*if (rfe=%d, cart=%d)\n",
			       (cfg_para & 0xff0000) >> 16, cfg_para & 0xff);
#endif
			break;
		case ELSE_8730E:
#ifdef	CONFIG_PHYDM_CMD
			RF_DBG(rf, DBG_RF_INIT, "*else\n");
#endif
			is_matched = false;
			if (!find_target) {
#ifdef	CONFIG_PHYDM_CMD
				RF_WARNING("Init RFCR Fail in Reg 0x%x\n", array[i]);
#endif
				return;
			}
			break;
		case END_8730E:
#ifdef	CONFIG_PHYDM_CMD
			RF_DBG(rf, DBG_RF_INIT, "*endif\n");
#endif
			halrf_flag_2_default_8730e(&is_matched, &find_target);
			break;
		case CHK_8730E:
			/*Check this para meets driver's requirement or not*/
			if (find_target) {
#ifdef	CONFIG_PHYDM_CMD
				RF_DBG(rf, DBG_RF_INIT, "\t skip\n");
#endif
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
#ifdef	CONFIG_PHYDM_CMD
			RF_DBG(rf, DBG_RF_INIT, "\t match=%d\n", is_matched);
#endif
			break;
		default:
			if (is_matched) {
				halrf_cfg_rf_radio_a_8730e(rf, v1, v2);
			}
			break;
		}
	}

	halrf_write_fwofld_end(rf);
#ifdef	CONFIG_PHYDM_CMD
	RF_DBG(rf, DBG_RF_INIT, "RFCR Init Success\n");
#endif
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
#endif

void
halrf_config_8730e_new_radio_a_reg(struct rf_info *rf)
{
	u16 i = 0, array_len = 0;
	u32 *array = NULL;
	u32 v1 = 0, v2 = 0;
	u8 cv_drv = rf->hal_com->cv;

	if (cv_drv == CAV) { /*A-CUT*/
		array_len = sizeof(array_mp_8730e_radioa_cuta) / sizeof(u32);
		array = (u32 *)array_mp_8730e_radioa_cuta;
	} else {
		array_len = sizeof(array_mp_8730e_radioa) / sizeof(u32);
		array = (u32 *)array_mp_8730e_radioa;
	}
	/*headline TBD*/
	while (i  < array_len) {
		v1 = (array[i] & 0xfff00000) >> 20;
		v2 = array[i] & 0x000fffff;
		i ++;
		if (v1 > 0xff) {
			v1 = 0x10000 + (v1 & 0xff);
		}
		/*laod radio a table*/
		halrf_wrf(rf, RF_PATH_A, v1, MASKRF, v2);
		RF_DBG(rf, DBG_RF_INIT, "[RF_a] %08X %08X\n", v1, v2);
	}

	RF_DBG(rf, DBG_RF_INIT, "RFCR Init Success\n");
}

void
halrf_config_8730e_store_power_by_rate(struct rf_info *rf)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	//struct rtw_para_info_t *phy_reg_info = NULL;

	u32 i, j;
	u32 array_len = 0;
	u32 *array = NULL;

	array_len = sizeof(array_mp_txpwr_byrate) / sizeof(u32);
	array = (u32 *)array_mp_txpwr_byrate;


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
halrf_config_8730e_init_power_limit(struct rf_info *rf)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u8 i, band;
	u8 array_len = sizeof(regu_en);

	/*limit table not from folder*/
	pwr->extra_regd_idx = 0xff;

	for (band = 0; band < PW_LMT_MAX_BAND; band++) {
		for (i = 0; i < array_len; i++) {
			if (regu_en[i] == 1) {
				pwr->regulation[band][i] = true;
			}
			RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->regulation[%d][%d]=%d\n",
			       __func__, band, i, pwr->regulation[band][i]);
		}
	}
}

s8
halrf_config_8730e_power_limit_by_ch(struct rf_info *rf,
				     u8 limit_rate, u8 regulation, u8 chnl)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u8 band = rf->hal_com->band[0].cur_chandef.band;
	u8 shap_rate = 0;
	s8 val = 127;

	if (limit_rate == PW_LMT_RS_CCK) {
		shap_rate = TX_SHAPE_CCK;
	} else {
		shap_rate = TX_SHAPE_OFDM;
	}

	switch (regulation) {
	case PW_LMT_REGU_FCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_fcc[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_fcc[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_fcc[band][limit_rate];
		break;
	case PW_LMT_REGU_ETSI:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_etsi[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_etsi[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_etsi[band][limit_rate];
		break;
	case PW_LMT_REGU_MKK:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_mkk[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_mkk[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_mkk[band][limit_rate];
		break;
	case PW_LMT_REGU_IC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_ic[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_ic[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_ic[band][limit_rate];
		break;
	case PW_LMT_REGU_KCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_kcc[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_kcc[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_kcc[band][limit_rate];
		break;
	case PW_LMT_REGU_ACMA:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_acma[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_acma[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_acma[band][limit_rate];
		break;
	case PW_LMT_REGU_NCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_ncc[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_ncc[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_ncc[band][limit_rate];
		break;
	case PW_LMT_REGU_MEXICO:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_mexico[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_mexico[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_mexico[band][limit_rate];
		break;
	case PW_LMT_REGU_CHILE:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_chile[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_chile[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_chile[band][limit_rate];
		break;
	case PW_LMT_REGU_UKRAINE:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_ukraine[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_ukraine[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_ukraine[band][limit_rate];
		break;
	case PW_LMT_REGU_CN:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_cn[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_cn[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_cn[band][limit_rate];
		break;
	case PW_LMT_REGU_QATAR:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_qatar[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_qatar[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_qatar[band][limit_rate];
		break;
	case PW_LMT_REGU_UK:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_uk[limit_rate][chnl];
		} else {
			val = tx_pwr_limit_5g_uk[limit_rate][chnl];
		}

		pwr->tx_shap_idx[band][shap_rate][regulation] = tx_shap_uk[band][limit_rate];
		break;
	case PW_LMT_REGU_WW13:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_2g_ww[limit_rate][chnl];
			//RF_DBG(rf, DBG_RF_INIT,"limit_2g_ww sizeof = %d\n", sizeof(tx_pwr_limit_2g_ww));
		} else {
			val = tx_pwr_limit_5g_ww[limit_rate][chnl];
			//RF_DBG(rf, DBG_RF_INIT,"limit_5g_ww sizeof = %d\n", sizeof(tx_pwr_limit_5g_ww));
		}
		break;
	default:
		break;
	}

	RF_DBG(rf, DBG_RF_POWER, "======>%s pwr->tx_shap_idx[%d][%d][%d]=%d\n",
	       __func__, band, shap_rate, regulation,
	       pwr->tx_shap_idx[band][TX_SHAPE_CCK][regulation]);
	RF_DBG(rf, DBG_RF_POWER, "======>%s reg_idx=%d limit[%d][%d] = %d\n",
	       __func__, regulation, limit_rate, chnl, val);

	return val;
}

s8
halrf_config_8730e_power_limit_ru_by_ch(struct rf_info *rf,
					u8 regulation, u8 bandwith, u8 chnl)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u8 band = rf->hal_com->band[0].cur_chandef.band;
	s8 val = 127;

	/*pwr->tx_shap_idx_ru TBD*/

	switch (regulation) {
	case PW_LMT_REGU_FCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_fcc[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_fcc[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_ETSI:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_etsi[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_etsi[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_MKK:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_mkk[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_mkk[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_IC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_ic[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_ic[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_KCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_kcc[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_kcc[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_ACMA:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_acma[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_acma[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_NCC:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_ncc[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_ncc[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_MEXICO:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_mexico[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_mexico[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_CHILE:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_chile[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_chile[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_UKRAINE:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_ukraine[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_ukraine[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_CN:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_cn[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_cn[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_QATAR:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_qatar[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_qatar[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_UK:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_uk[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_uk[bandwith][chnl];
		}
		break;
	case PW_LMT_REGU_WW13:
		if (band == BAND_ON_24G) {
			val = tx_pwr_limit_ru_2g_ww[bandwith][chnl];
		} else {
			val = tx_pwr_limit_ru_5g_ww[bandwith][chnl];
		}
		break;
	default:
		break;
	}

#if 0
	if (limit_rate == PW_LMT_RS_CCK) {
		pwr->tx_shap_idx_ru[band][TX_SHAPE_CCK][regulation] = array[i].tx_shap_idx;
#ifdef	CONFIG_PHYDM_CMD
		RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx_ru[%d][CCK][%d]=%d\n",
		       __func__, band, regulation,
		       pwr->tx_shap_idx_ru[band][TX_SHAPE_CCK][regulation]);
#endif
	} else {
		pwr->tx_shap_idx_ru[band][TX_SHAPE_OFDM][regulation] = array[i].tx_shap_idx;
#ifdef	CONFIG_PHYDM_CMD
		RF_DBG(rf, DBG_RF_INIT, "======>%s pwr->tx_shap_idx_ru[%d][OFDM][%d]=%d\n",
		       __func__, band, regulation,
		       pwr->tx_shap_idx_ru[band][TX_SHAPE_OFDM][regulation]);
#endif
	}
#endif
	RF_DBG(rf, DBG_RF_POWER, "======>%s reg_idx=%d limit_ru[%d][%d] = %d\n",
	       __func__, regulation, bandwith, chnl, val);
	return val;
}

void
halrf_config_8730e_store_pwr_track(struct rf_info *rf)
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

/*
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
*/
