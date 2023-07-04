/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _RTL8730E_HALINIT_C_
#include "../hal_headers.h"
#include "../hal_api.h"
#include "rtl8730e_hal.h"


void init_hal_spec_8730e(struct rtw_phl_com_t *phl_com,
			 struct hal_info_t *hal)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct protocol_cap_t *hw_proto_cap = hal_com->proto_hw_cap;

	hal_spec->ic_name = "rtl8730e";
	hal_spec->macid_num = hal_mac_get_macid_num(hal);
	/* hal_spec->sec_cam_ent_num follow halmac setting */
	hal_spec->sec_cap = SEC_CAP_CHK_BMC;

	hal_spec->rfpath_num_2g = 1;
	hal_spec->rfpath_num_5g = 1;
	hal_spec->rf_reg_path_num = 1;
	hal_com->rfpath_rx_num = 1;
	hal_com->rfpath_tx_num = 1;
	hal_com->phy_hw_cap[0].rx_num = 1;
	hal_com->phy_hw_cap[0].tx_num = 1;
	hal_com->phy_hw_cap[1].rx_num = 1;
	hal_com->phy_hw_cap[1].tx_num = 1;
	hal_com->phy_hw_cap[0].hw_rts_time_th = 0;
	hal_com->phy_hw_cap[1].hw_rts_time_th = 0;
	hal_com->phy_hw_cap[0].hw_rts_len_th = 0;
	hal_com->phy_hw_cap[1].hw_rts_len_th = 0;
	hal_spec->max_tx_cnt = 1;
	hal_spec->band_cap = BAND_CAP_2G | BAND_CAP_5G;
	hal_spec->bw_cap = BW_CAP_20M;
	hal_spec->port_num = 2;
	hal_spec->wmm_num = 2;

	hal_spec->proto_cap = PROTO_CAP_11B | PROTO_CAP_11G | PROTO_CAP_11N /*|
			      PROTO_CAP_11AC | PROTO_CAP_11AX*/;

	hal_spec->wl_func = 0 /*| WL_FUNC_FTM */;

	hal_spec->max_csi_buf_su_nr = 4;
	hal_spec->max_csi_buf_mu_nr = 0;
	hal_spec->max_bf_ent_nr = 16;
	hal_spec->max_su_sta_nr = 16;
	hal_spec->max_mu_sta_nr = 0;
#ifdef RTW_WKARD_PHY_CAP
	/* HE */
	hw_proto_cap[0].he_su_bfme = 1;
	hw_proto_cap[0].he_su_bfmr = 0;
	hw_proto_cap[0].he_mu_bfme = 1;
	hw_proto_cap[0].he_mu_bfmr = 0;

	hw_proto_cap[1].he_su_bfme = 1;
	hw_proto_cap[1].he_su_bfmr = 0;
	hw_proto_cap[1].he_mu_bfme = 1;
	hw_proto_cap[1].he_mu_bfmr = 0;

	hw_proto_cap[0].trig_cqi_fb = 1;
	hw_proto_cap[0].non_trig_cqi_fb = 1;
	hw_proto_cap[1].trig_cqi_fb = 1;
	hw_proto_cap[1].non_trig_cqi_fb = 1;

	/* VHT */
	hw_proto_cap[0].vht_su_bfmr = 0;
	hw_proto_cap[0].vht_su_bfme = 1;
	hw_proto_cap[0].vht_mu_bfmr = 0;
	hw_proto_cap[0].vht_mu_bfme = 1;

	hw_proto_cap[1].vht_su_bfmr = 0;
	hw_proto_cap[1].vht_su_bfme = 1;
	hw_proto_cap[1].vht_mu_bfmr = 0;
	hw_proto_cap[1].vht_mu_bfme = 1;

	/* HT */
	hw_proto_cap[0].ht_su_bfmr = 0;
	hw_proto_cap[0].ht_su_bfme = 1;

	hw_proto_cap[1].ht_su_bfmr = 0;
	hw_proto_cap[1].ht_su_bfme = 1;

	/* STBC Tx*/
	hw_proto_cap[0].stbc_tx = 0; /* Revmoe later */
	hw_proto_cap[1].stbc_tx = 0; /* Revmoe later */
	hw_proto_cap[0].stbc_ht_tx = 0;
	hw_proto_cap[1].stbc_ht_tx = 0;
	hw_proto_cap[0].stbc_vht_tx = 0;
	hw_proto_cap[1].stbc_vht_tx = 0;
	hw_proto_cap[0].stbc_he_tx = 0;
	hw_proto_cap[1].stbc_he_tx = 0;
	hw_proto_cap[0].stbc_tx_greater_80mhz = 0;
	hw_proto_cap[1].stbc_tx_greater_80mhz = 0;

	/* STBC Rx*/
	hw_proto_cap[0].stbc_ht_rx = 1;
	hw_proto_cap[1].stbc_ht_rx = 1;
	hw_proto_cap[0].stbc_vht_rx = 1;
	hw_proto_cap[1].stbc_vht_rx = 1;
	hw_proto_cap[0].stbc_he_rx = 1;
	hw_proto_cap[1].stbc_he_rx = 1;
	hw_proto_cap[0].stbc_rx_greater_80mhz = 0;
	hw_proto_cap[1].stbc_rx_greater_80mhz = 0;
#endif

	/*get mac capability*/
	phl_com->dev_cap.hw_sup_flags = HW_SUP_TCP_TX_CHKSUM |
					HW_SUP_MULTI_BSSID;

	phl_com->dev_cap.hw_sup_flags |= HW_SUP_TCP_RX_CHKSUM;

	phl_com->dev_cap.hw_sup_flags |= HW_SUP_OFDMA | HW_SUP_CHAN_INFO;


#ifdef RTW_WKARD_LAMODE
	hal_com->dev_hw_cap.la_mode = true;/*TODO : get info from halbb*/
#endif

#ifdef CONFIG_DBCC_SUPPORT
	if (phl_com->dev_cap.hw_sup_flags & HW_SUP_DBCC) {
		hal_com->dev_hw_cap.dbcc_sup = true;        /*get info from efuse*/
	}
#endif
	hal_com->dev_hw_cap.hw_hdr_conv = true;
	hal_com->dev_hw_cap.tx_mu_ru = true;

#ifdef CONFIG_MCC_SUPPORT
	hal_com->dev_hw_cap.mcc_sup = true;
#endif /* CONFIG_MCC_SUPPORT */

	hal_com->dev_hw_cap.sta_ulru = RTW_HW_CAP_ULRU_ENABLE;
#ifdef RTW_WKARD_BB_DISABLE_STA_2G40M_ULOFDMA
	hal_com->dev_hw_cap.sta_ulru_2g40mhz = RTW_HW_CAP_ULRU_DISABLE;
#endif

#ifdef CONFIG_PHL_TWT
	hal_com->dev_hw_cap.twt_sup = RTW_PHL_TWT_REQ_SUP | RTW_PHL_TWT_RSP_SUP;
#endif /* CONFIG_PHL_TWT */

	hal_com->dev_hw_cap.ps_cap.ips_cap = PS_CAP_PWR_OFF;
	hal_com->dev_hw_cap.ps_cap.ips_wow_cap =
		PS_CAP_PWRON | PS_CAP_RF_OFF | PS_CAP_CLK_GATED | PS_CAP_PWR_GATED;
	hal_com->dev_hw_cap.ps_cap.lps_cap =
		PS_CAP_PWRON | PS_CAP_RF_OFF | PS_CAP_CLK_GATED | PS_CAP_PWR_GATED;
	hal_com->dev_hw_cap.ps_cap.lps_wow_cap =
		PS_CAP_PWRON | PS_CAP_RF_OFF | PS_CAP_CLK_GATED | PS_CAP_PWR_GATED;

	hal_com->dev_hw_cap.hw_stype_cap = EFUSE_HW_STYPE_NONE_8730E;
	hal_com->dev_hw_cap.wl_func_cap = EFUSE_WL_FUNC_NONE;
	hal_com->dev_hw_cap.rpq_agg_num = 0;
}


void init_default_value_8730e(struct hal_info_t *hal)
{

	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct rtw_chan_def *chandef = NULL;
	u8 bid = 0;

	for (bid = 0; bid < HW_BAND_MAX; bid++) {

		chandef = &(hal_com->band[bid].cur_chandef);
		chandef->bw = CHANNEL_WIDTH_MAX;
		chandef->band = BAND_ON_5G;
		chandef->chan = 0;
		chandef->offset = CHAN_OFFSET_NO_EXT;
	}
}

enum rtw_hal_status hal_get_efuse_8730e(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal,
					struct hal_init_info_t *init_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	FUNCIN();

	rtw_hal_efuse_process(hal, init_info->ic_name);

	FUNCOUT();

	return RTW_HAL_STATUS_SUCCESS;

hal_power_off_fail:
hal_fast_init_fail:
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "==> %s : hal get efuse fail\n", __func__);
	return hal_status;
}

enum rtw_hal_status hal_start_8730e(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal,
				    struct hal_init_info_t *init_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phy_cap_t *phy_cap = phl_com->phy_cap;
	struct rtw_chan_def ch = {0};
	u8 val = 0;

	/* Read phy parameter files */
	rtw_hal_dl_all_para_file(phl_com, init_info->ic_name, hal);

	hal_status = rtw_hal_mac_hal_init(phl_com, hal, init_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		goto hal_init_fail;
	}

	rtw_hal_set_rxfltr_by_mode(hal, HW_BAND_0, RX_FLTR_MODE_STA_NORMAL);
	/* MAC Suggested : 11264 Byte */
	rtw_hal_mac_set_rxfltr_mpdu_size(hal->hal_com, HW_BAND_0, 0x2c00);
	rtw_hal_mac_set_hw_rts_th(hal, HW_BAND_0,
				  phy_cap[HW_BAND_0].hw_rts_time_th,
				  phy_cap[HW_BAND_0].hw_rts_len_th);
	/*update phy cap of tx agg info */
	rtw_hal_mac_init_txagg_num(hal);

#ifdef CONFIG_BTCOEX
	/* power on config for btc */
	rtw_hal_btc_power_on_ntfy(hal);
#endif

	/* EFUSE config */
	rtw_hal_efuse_process(hal, init_info->ic_name);
	/*update final cap of txagg info*/
	rtw_hal_final_cap_decision(phl_com, hal);

	/*[Pre-config BB/RF] BBRST / RFC reset */
	rtw_hal_mac_enable_bb_rf(hal, 0);
	rtw_hal_mac_enable_bb_rf(hal, 1);

	/* load parameters or config mac, phy, btc, ... */
#ifdef USE_TRUE_PHY
	rtw_hal_init_bb_reg(hal);
	rtw_hal_init_rf_reg(phl_com, hal);
#endif

#ifdef CONFIG_BTCOEX
	/* After mac/bb/rf initialized, set btc config */
	rtw_hal_btc_init_coex_cfg_ntfy(hal);
#endif
	/* start watchdog/dm */
	rtw_hal_rf_dm_init(hal);
	rtw_hal_bb_dm_init(hal);

	ch.band = BAND_MAX;
	ch.bw = CHANNEL_WIDTH_20;
	ch.offset = CHAN_OFFSET_NO_EXT;
	ch.chan = 0;
	hal_status = rtw_hal_set_ch_bw(hal, HW_BAND_0, &ch, true);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		goto hal_init_fail;
	}

	rtw_hal_cfg_bb_rfe_gpio(hal);

	hal_status = rtw_hal_mac_get_append_fcs(hal, &val);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		goto hal_init_fail;
	} else {
		phl_com->append_fcs = val;
	}

	hal_status = rtw_hal_mac_get_acpt_icv_err(hal, &val);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		goto hal_init_fail;
	} else {
		phl_com->append_fcs = val;
	}

	/* enable hw encypt end dencypt for mgnt unitcast */
	rtw_hal_mac_config_hw_mgnt_sec(hal, true);

	PHL_INFO("==> Default ENABLE RX_PPDU_STS for Band0\n");
	/* Enable PPDU STS in default for BAND-0 for phy status */
	hal->hal_com->band[HW_BAND_0].ppdu_sts_appen_info = HAL_PPDU_PHYSTS;
	hal->hal_com->band[HW_BAND_0].ppdu_sts_filter = HAL_PPDU_HAS_CRC_OK;
	rtw_hal_mac_ppdu_stat_cfg(
		hal, HW_BAND_0, true,
		hal->hal_com->band[HW_BAND_0].ppdu_sts_appen_info,
		hal->hal_com->band[HW_BAND_0].ppdu_sts_filter);

	if (hal->hal_com->band[HW_BAND_0].ppdu_sts_filter | HAL_PPDU_HAS_CRC_OK) {
		rtw_hal_acpt_crc_err_pkt(hal, HW_BAND_0, true);
	}

	phl_com->ppdu_sts_info.en_ppdu_sts[HW_BAND_0] = true;
	/*TODO Enable PPDU STS in default for BAND-1 for phy status */

	//hal_status = rtw_hal_hdr_conv_cfg(hal, phl_com->dev_cap.hw_hdr_conv);
	//if (hal_status != RTW_HAL_STATUS_SUCCESS) {
	//	goto hal_init_fail;
	//}

	return RTW_HAL_STATUS_SUCCESS;

hal_init_fail:
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "==> %s : hal init fail\n", __func__);
	return hal_status;
}

enum rtw_hal_status hal_stop_8730e(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

#ifdef CONFIG_BTCOEX
	/* power off config for btc */
	rtw_hal_btc_power_off_ntfy(hal);
#endif
	hal_status = rtw_hal_mac_hal_deinit(phl_com, hal);
	rtw_hal_bb_dm_deinit(phl_com, hal);

	return hal_status;
}

#ifdef CONFIG_WOWLAN
enum rtw_hal_status
hal_wow_init_8730e(struct rtw_phl_com_t *phl_com,
		   struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
		   struct hal_init_info_t *init_info) {
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

#ifndef CONFIG_AXI_HCI
	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_WOWLAN);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}
#endif

	hal_status = rtw_hal_mac_role_sync(hal_info, sta);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: role sync fail!\n", __func__);
		goto exit;
	}

	hal_status = rtw_hal_update_sta_entry(hal_info, sta, true);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: update sta entry fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

#ifdef RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
	rtw_hal_mac_config_hw_mgnt_sec(hal_info, true);
#endif

exit:
	return hal_status;
}

enum rtw_hal_status
hal_wow_deinit_8730e(struct rtw_phl_com_t *phl_com,
		     struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
		     struct hal_init_info_t *init_info) {
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	/* AOAC Report */

#ifndef CONFIG_AXI_HCI
	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_NIC);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}
#endif

	hal_status = rtw_hal_mac_role_sync(hal_info, sta);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: role sync fail!\n", __func__);
		goto exit;
	}

	hal_status = rtw_hal_update_sta_entry(hal_info, sta, true);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: update sta entry fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	/* To Do : Recover RA ? */

#ifdef RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
	rtw_hal_mac_config_hw_mgnt_sec(hal_info, false);
#endif

exit:
	return hal_status;
}
#endif /* CONFIG_WOWLAN */

#ifdef RTW_PHL_BCN //fill 8730e bcn ops

enum rtw_hal_status hal_config_beacon_8730e(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry)
{
	if (hal_mac_ax_config_beacon(hal, bcn_entry) == RTW_HAL_STATUS_FAILURE) {
		return RTW_HAL_STATUS_FAILURE;
	}

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status hal_update_beacon_8730e(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry)
{
	if (hal_mac_ax_send_beacon(hal, bcn_entry) == RTW_HAL_STATUS_FAILURE) {
		return RTW_HAL_STATUS_FAILURE;
	}

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status
hal_mp_init_8730e(struct rtw_phl_com_t *phl_com,
		  struct hal_info_t *hal_info,
		  struct hal_init_info_t *init_info) {
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

#ifndef CONFIG_AXI_HCI
	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_NIC);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}
#endif

exit:
	return hal_status;
}

enum rtw_hal_status
hal_mp_deinit_8730e(struct rtw_phl_com_t *phl_com,
		    struct hal_info_t *hal_info,
		    struct hal_init_info_t *init_info) {
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

#ifndef CONFIG_AXI_HCI
	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_NIC);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
	{
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}
#endif

exit:
	return hal_status;
}

#endif //RTW_PHL_BCN
