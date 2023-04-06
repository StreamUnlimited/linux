/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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

#include "halbb_precomp.h"

u8 halbb_ch_2_band(struct bb_info *bb, u8 fc_ch)
{
	u8 band = 0;

	// 2G Band: (0)
	// 5G Band: (1):Low, (2): Mid, (3):High
	if (fc_ch >= 1 && fc_ch <= 14) {
		band = 0;
	} else if (fc_ch >= 36 && fc_ch <= 64) {
		band = 1;
	} else if (fc_ch >= 100 && fc_ch <= 144) {
		band = 2;
	} else if (fc_ch >= 149 && fc_ch <= 177) {
		band = 3;
	}

	return band;
}

u16 halbb_get_csi_buf_idx(struct bb_info *bb, u8 buf_idx, u8 txsc_idx)

{
	u8 table_size = 0;
	u8 i;
	u8 txsc_2_buf_idx_160[][2] = {{0, 0}, //BW all
		{8, 1}, //20M
		{6, 3},
		{4, 2},
		{2, 4},
		{1, 5},
		{3, 7},
		{5, 6},
		{7, 8},
		{12, 9}, //40M
		{10, 10},
		{9, 11},
		{11, 12},
		{14, 13}, //80M
		{13, 14}
	};
	u8 txsc_2_buf_idx_080[][2] = {{0, 0}, //BW all
		{4, 1}, //20M
		{2, 3},
		{1, 2},
		{3, 4},
		{10, 9}, //40M
		{9, 10}
	};
	u8 csi_sub_idx = 0xff;
	u16 rpt_val = 0;

	if (bb->ic_type & BB_IC_MAX_BW_160) {
		table_size = sizeof(txsc_2_buf_idx_160) / (sizeof(u8) * 2);
		for (i = 0; i < table_size; i++) {
			if (txsc_2_buf_idx_160[i][0] == txsc_idx) {
				csi_sub_idx = txsc_2_buf_idx_160[i][1];
				break;
			}
		}
	} else {
		table_size = sizeof(txsc_2_buf_idx_080) / (sizeof(u8) * 2);
		for (i = 0; i < table_size; i++) {
			if (txsc_2_buf_idx_080[i][0] == txsc_idx) {
				csi_sub_idx = txsc_2_buf_idx_080[i][1];
				break;
			}
		}
	}

	BB_DBG(bb, DBG_DBG_API, "%02d -> %02d\n", txsc_idx, csi_sub_idx);

	if (csi_sub_idx == 0xff) {
		rpt_val = 0xff;
	} else {
		rpt_val = (buf_idx << 6) | (csi_sub_idx << 2);
	}

	return  rpt_val;
}

u16 halbb_cfg_cmac_tx_ant(struct bb_info *bb, enum rf_path tx_path)
{
	u16 val = 0;

	return val;
}


void halbb_gpio_ctrl_dump(struct bb_info *bb)
{
}

void halbb_gpio_rfm(struct bb_info *bb, enum bb_path path,
		    enum bb_rfe_src_sel src, bool dis_tx_gnt_wl,
		    bool active_tx_opt, bool act_bt_en, u8 rfm_output_val)
{
}

void halbb_gpio_trsw_table(struct bb_info *bb, enum bb_path path,
			   bool path_en, bool trsw_tx, bool trsw_rx,
			   bool trsw, bool trsw_b)
{
}

void halbb_gpio_setting_all(struct bb_info *bb, u8 rfe_idx)
{
}

void halbb_gpio_setting(struct bb_info *bb, u8 gpio_idx, enum bb_path path,
			bool inv, enum bb_rfe_src_sel src)
{
}

void halbb_gpio_setting_init(struct bb_info *bb)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_gpio_setting_init_8730e(bb);
		break;
#endif

	default:
		break;
	}
}

void halbb_pre_agc_en(struct bb_info *bb, bool enable)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_pre_agc_en_8730e(bb, enable);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_pre_agc_en_8720e(bb, enable);
		break;
#endif

	default:
		break;
	}
}

void halbb_set_gain_error(struct bb_info *bb, u8 central_ch, enum band_type band, enum rf_path path)
{
}

u8 halbb_stop_ic_trx(struct bb_info *bb, u8 set_type)
{
	struct rtw_hal_com_t	*hal_i = bb->hal_com;
	u8 i = 0;
	bool trx_idle_success = false;
	u32 dp = 0;

	if (set_type == HALBB_SET) {
		/*[Stop TRX]---------------------------------------------------------*/
		if (bb->ic_type & BB_IC_AX_SERIES) {
			/*set debug port to 0x0*/
			if (!halbb_bb_dbg_port_racing(bb, DBGPORT_PRI_3)) {
				return HALBB_SET_FAIL;
			}
			halbb_set_bb_dbg_port_ip(bb, DBGPORT_IP_TD);
			halbb_set_bb_dbg_port(bb, 0x205);

			for (i = 0; i < 100; i++) {
				dp = halbb_get_bb_dbg_port_val(bb);
				/* CCA_all && mux_state*/
				if ((dp & 0x80000f00) == 0) {
					BB_DBG(bb, DBG_DBG_API, "Stop trx wait for (%d) times\n", i);
					trx_idle_success = true;
					break;
				}
				halbb_delay_ms(bb, 1);
			}
			halbb_release_bb_dbg_port(bb);
		}

		if (trx_idle_success) {
			/*pause all TX queue*/
			rtw_hal_tx_pause(hal_i, 0, true, PAUSE_RSON_PSD);
			rtw_hal_tx_pause(hal_i, 1, true, PAUSE_RSON_PSD);
			halbb_ctrl_rx_cca(bb, false, HW_PHY_0);
			halbb_ctrl_rx_cca(bb, false, HW_PHY_1);
		} else {
			return HALBB_SET_FAIL;
		}

		return HALBB_SET_SUCCESS;

	} else { /*@if (set_type == HALBB_REVERT)*/
		/*Release all TX queue*/
		rtw_hal_tx_pause(hal_i, 0, false, PAUSE_RSON_PSD);
		rtw_hal_tx_pause(hal_i, 1, false, PAUSE_RSON_PSD);
		halbb_ctrl_rx_cca(bb, true, HW_PHY_0);
		halbb_ctrl_rx_cca(bb, true, HW_PHY_1);

		return HALBB_SET_SUCCESS;
	}
}

u8 halbb_get_txsc(struct bb_info *bb, u8 pri_ch, u8 central_ch,
		  enum channel_width cbw, enum channel_width dbw)
{
	u8 txsc_idx = 0;
	u8 tmp = 0;
	u8 ofst = 0;

	if ((cbw == dbw) || (cbw == CHANNEL_WIDTH_20)) {
		txsc_idx = 0;
		BB_DBG(bb, DBG_PHY_CONFIG, "[TXSC] TxSC_idx = %d\n", txsc_idx);
		return txsc_idx;
	}

	switch (cbw) {
	case CHANNEL_WIDTH_40:
		txsc_idx = pri_ch > central_ch ? 1 : 2;
		break;
	case CHANNEL_WIDTH_80:
		if (dbw == CHANNEL_WIDTH_20) {
			if (pri_ch > central_ch) {
				txsc_idx = (pri_ch - central_ch) >> 1;
			} else {
				txsc_idx = ((central_ch - pri_ch) >> 1) + 1;
			}
		} else {
			txsc_idx = pri_ch > central_ch ? 9 : 10;
		}
		break;
	case CHANNEL_WIDTH_160:
		if (pri_ch > central_ch) {
			tmp = (pri_ch - central_ch) >> 1;
		} else {
			tmp = ((central_ch - pri_ch) >> 1) + 1;
		}

		if (dbw == CHANNEL_WIDTH_20) {
			txsc_idx = tmp;
		} else if (dbw == CHANNEL_WIDTH_40) {
			if ((tmp == 1) || (tmp == 3)) {
				txsc_idx = 9;
			} else if ((tmp == 5) || (tmp == 7)) {
				txsc_idx = 11;
			} else if ((tmp == 2) || (tmp == 4)) {
				txsc_idx = 10;
			} else if ((tmp == 6) || (tmp == 8)) {
				txsc_idx = 12;
			} else {
				return 0xff;
			}
		} else {
			txsc_idx = pri_ch > central_ch ? 13 : 14;
		}
		break;
	case CHANNEL_WIDTH_80_80:
		if (dbw == CHANNEL_WIDTH_20) {
			if (pri_ch > central_ch) {
				txsc_idx = (10 - (pri_ch - central_ch)) >> 1;
			} else {
				txsc_idx = ((central_ch - pri_ch) >> 1) + 5;
			}
		} else if (dbw == CHANNEL_WIDTH_40) {
			txsc_idx = pri_ch > central_ch ? 10 : 12;
		} else {
			txsc_idx = 14;
		}
	default:
		break;
	}
	BB_DBG(bb, DBG_PHY_CONFIG, "[TXSC] TxSC_idx = %d\n", txsc_idx);
	return txsc_idx;
}

void halbb_bb_reset(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_bb_reset_8730e(bb, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_bb_reset_8720e(bb, phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_reset_bb(struct bb_info *bb)
{
	halbb_bb_reset(bb, HW_PHY_0);
#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en) {
		halbb_bb_reset(bb, HW_PHY_1);
	}
#endif
}

void halbb_tssi_bb_reset(struct bb_info *bb)
{
}

u32 halbb_read_rf_reg(struct bb_info *bb, enum rf_path path, u32 addr, u32 mask)
{
	u32 val = 0;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		val = halbb_read_rf_reg_8730e(bb, path, addr, mask);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		val = halbb_read_rf_reg_8720e(bb, path, addr, mask);
		break;
#endif

	default:
		val = 0;
		break;
	}

	return val;
}

bool halbb_write_rf_reg(struct bb_info *bb, enum rf_path path, u32 addr, u32 mask,
			u32 data)
{
	bool rpt = true;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_write_rf_reg_8730e(bb, path, addr, mask, data);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_write_rf_reg_8720e(bb, path, addr, mask, data);
		break;
#endif
	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_rf_set_bb_reg(struct bb_info *bb, u32 addr, u32 mask, u32 data)
{
	bool rpt = false;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_rf_write_bb_reg_8730e(bb, addr, mask, data);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_rf_write_bb_reg_8720e(bb, addr, mask, data);
		break;
#endif

	default:
		break;
	}

	if (bb->bb_dbg_i.cr_recorder_rf_en) {
		BB_TRACE("[RF][W] 0x%04x[0x%08x] = 0x%08x\n", addr, mask, data);
	}

	if (!rpt) {
		BB_WARNING("[%s][IQK]error IO 0x%x\n", __func__, addr);
	}

	return rpt;
}

u32 halbb_rf_get_bb_reg(struct bb_info *bb, u32 addr, u32 mask)
{
	u32 val = 0;

	val = halbb_get_reg(bb, addr, mask);

	if (bb->bb_dbg_i.cr_recorder_rf_en) {
		BB_TRACE("[RF][R] 0x%04x[0x%08x] = 0x%08x\n", addr, mask, val);
	}

	return val;
}

void halbb_dfs_en(struct bb_info *bb, bool en)
{
}

void halbb_adc_en(struct bb_info *bb, bool en)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_adc_en_8730e(bb, en);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_adc_en_8720e(bb, en);
		break;
#endif

	default:
		break;
	}
}

void halbb_tssi_cont_en(struct bb_info *bb, bool en, enum rf_path path)
{
}

void halbb_bb_reset_en(struct bb_info *bb, bool en, enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_bb_reset_en_8730e(bb, en, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_bb_reset_en_8720e(bb, en, phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_ctrl_rf_mode(struct bb_info *bb, enum phl_rf_mode mode)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_ctrl_rf_mode_8730e(bb, mode);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_ctrl_rf_mode_8720e(bb, mode);
		break;
#endif

	default:
		break;
	}
}

bool halbb_ctrl_rx_path(struct bb_info *bb, enum rf_path rx_path)
{
	bool rpt = true;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_ctrl_rx_path_8730e(bb, rx_path);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_ctrl_rx_path_8720e(bb, rx_path);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_ctrl_tx_path(struct bb_info *bb, enum rf_path tx_path)
{
	bool rpt = true;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_ctrl_tx_path_8730e(bb, tx_path);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_ctrl_tx_path_8720e(bb, tx_path);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

void halbb_ctrl_trx_path(struct bb_info *bb_0, enum rf_path tx_path, u8 tx_nss,
			 enum rf_path rx_path, u8 rx_nss)
{
	struct bb_info *bb = bb_0;
#ifdef HALBB_DBCC_SUPPORT
	enum phl_phy_idx phy_idx = HW_PHY_0;

	/*Only take 2SS-IC into consideration*/
	if (bb->hal_com->dbcc_en) {
		if (tx_path >= RF_PATH_AB || rx_path >= RF_PATH_AB ||
		    tx_nss != 1 || rx_nss != 1 ||
		    tx_path != rx_path) {
			BB_WARNING("[%s]tx_path=%d, rx_path=%d\n", __func__, tx_path, rx_path);
			return;
		}
		phy_idx = (rx_path == RF_PATH_A) ? HW_PHY_0 : HW_PHY_1;
	}

	HALBB_GET_PHY_PTR(bb_0, bb, phy_idx);
#endif

	BB_DBG(bb, DBG_DBCC, "[%s]tx_path=%d, tx_nss=%d, rx_path=%d, rx_nss=%d\n", __func__, tx_path, tx_nss, rx_path, rx_nss);

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_ctrl_trx_path_8730e(bb, tx_path, tx_nss, rx_path, rx_nss);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_ctrl_trx_path_8720e(bb, tx_path, tx_nss, rx_path, rx_nss);
		break;
#endif

	default:
		break;
	}

}

u16 halbb_fc_mapping(struct bb_info *bb, enum band_type band, u8 central_ch)
{
	u16 central_freq = 0;

#ifdef BB_SUPPORT_BAND6G
	if (band == BAND_ON_6G) {
		central_freq = 5955 + (central_ch - 1) * 5;
	} else
#endif
		if (band == BAND_ON_5G) {
			central_freq = 5180 + (central_ch - 36) * 5;
		} else { // band == BAND_ON_2G
			if (central_ch == 14) {
				central_freq = 2484;
			} else {
				central_freq = 2412 + (central_ch - 1) * 5;
			}
		}

	return central_freq;
}

bool halbb_ctrl_bw(struct bb_info *bb, u8 pri_ch_idx, enum band_type band, enum channel_width bw,
		   enum phl_phy_idx phy_idx)
{
	/*For MP Use Only*/
	bool rpt = true;
	struct bb_api_info *bb_api = &bb->bb_api_i;

	bb_api->pri_ch_idx = pri_ch_idx;
	bb_api->bw = bw;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_ctrl_bw_8730e(bb, pri_ch_idx, bw, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_ctrl_bw_8720e(bb, pri_ch_idx, bw, phy_idx);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_ctrl_bw_ch(struct bb_info *bb, u8 pri_ch, u8 central_ch_seg0,
		      u8 central_ch_seg1, enum band_type band,
		      enum channel_width bw, enum phl_phy_idx phy_idx)
{
	bool rpt = true;
	struct bb_api_info *bb_api = &bb->bb_api_i;

	BB_DBG(bb, DBG_DBCC, "[%s] CH_pri/fc=%03d/%03d, bw=%dM, phy_idx=%d\n",
	       __func__, pri_ch, central_ch_seg0, 20 << bw, phy_idx);

	bb_api->central_ch = central_ch_seg0;
	bb_api->band = band;
	bb_api->bw = bw;
	bb_api->pri_ch_idx = pri_ch;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_ctrl_bw_ch_8730e(bb, pri_ch, central_ch_seg0, bw,
					     band, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_ctrl_bw_ch_8720e(bb, pri_ch, central_ch_seg0, bw,
					     band, phy_idx);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

void halbb_ctrl_rx_cca(struct bb_info *bb, bool cca_en, enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_ctrl_rx_cca_8730e(bb, cca_en, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_ctrl_rx_cca_8720e(bb, cca_en, phy_idx);
		break;
#endif

	default:
		break;
	}
}

bool halbb_query_cck_en(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	bool cck_en = false;

	return cck_en;
}

void halbb_ctrl_cck_en(struct bb_info *bb, bool cck_enable,
		       enum phl_phy_idx phy_idx)
{
}

void halbb_ctrl_ofdm_en(struct bb_info *bb, bool ofdm_enable,
			enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_ctrl_ofdm_en_8730e(bb, ofdm_enable, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_ctrl_ofdm_en_8720e(bb, ofdm_enable, phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_ctrl_btg(struct bb_info *bb, bool btg)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_ctrl_btg_8730e(bb, btg);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_ctrl_btg_8720e(bb, btg);
		break;
#endif

	default:
		break;
	}
}

void halbb_ctrl_btc_preagc(struct bb_info *bb, bool bt_en)
{

}

void halbb_pop_en(struct bb_info *bb, bool en, enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_pop_en_8730e(bb, en, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_pop_en_8720e(bb, en, phy_idx);
		break;
#endif

	default:
		break;
	}
}

bool halbb_querry_pop_en(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_querry_pop_en_8730e(bb, phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_querry_pop_en_8720e(bb, phy_idx);
		break;
#endif
	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_set_pd_lower_bound(struct bb_info *bb, u8 bound,
			      enum channel_width bw, enum phl_phy_idx phy_idx)
{
	bool rpt = true;
	struct bb_api_info *bb_api = &bb->bb_api_i;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_set_pd_lower_bound_8730e(bb, bound, bw, phy_idx);
		break;
#endif
#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_set_pd_lower_bound_8720e(bb, bound, bw, phy_idx);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_set_pd_lower_bound_cck(struct bb_info *bb, u8 bound,
				  enum channel_width bw, enum phl_phy_idx phy_idx)
{
	bool rpt = true;
	struct bb_api_info *bb_api = &bb->bb_api_i;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_set_pd_lower_bound_cck_8730e(bb, bound, bw, phy_idx);
		break;
#endif
#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_set_pd_lower_bound_cck_8720e(bb, bound, bw, phy_idx);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

u8 halbb_querry_pd_lower_bound(struct bb_info *bb, bool get_en_info,
			       enum phl_phy_idx phy_idx)
{
	u8 rpt = 0;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rpt = halbb_querry_pd_lower_bound_8730e(bb, get_en_info, phy_idx);
		break;
#endif
#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rpt = halbb_querry_pd_lower_bound_8720e(bb, get_en_info, phy_idx);
		break;
#endif
	default:
		rpt = 0;
		break;
	}

	return rpt;
}

u8 halbb_get_losel(struct bb_info *bb)
{
	u8 rpt = 0xff;

	return rpt;
}

void halbb_set_igi(struct bb_info *bb, u8 lna_idx, bool tia_idx, u8 rxbb_idx,
		   enum rf_path path)
{
	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_set_igi_8730e(bb, lna_idx, tia_idx, rxbb_idx, path);
		break;
#endif
#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_set_igi_8720e(bb, lna_idx, tia_idx, rxbb_idx, path);
		break;
#endif

	default:
		break;
	}
}

void halbb_set_tx_pow_pattern_shap(struct bb_info *bb, u8 ch,
				   bool is_ofdm, enum phl_phy_idx phy_idx)
{
	struct rtw_tpu_info *tpu = &bb->hal_com->band[phy_idx].rtw_tpu_i;
	u8 shape_idx = tpu->tx_ptrn_shap_idx;
	if (is_ofdm) {
		halbb_tx_triangular_shap_cfg_8730e(bb, shape_idx, phy_idx);
	} else {
		halbb_tx_dfir_shap_cck_8730e(bb, ch, shape_idx, phy_idx);
	}
	return;
}

void halbb_set_tx_pow_ref(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	struct rtw_tpu_info *tpu = &bb->hal_com->band[phy_idx].rtw_tpu_i;

	switch (bb->ic_type) {

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		rtw_hal_mac_write_pwr_ref_reg(bb->hal_com, (enum phl_band_idx)phy_idx);

		halbb_set_tx_pow_ref_8730e(bb, tpu->ref_pow_ofdm,
					   tpu->ref_pow_cck,
					   tpu->ofst_int,
					   tpu->base_cw_0db,
					   tpu->tssi_16dBm_cw,
					   &tpu->ref_pow_ofdm_cw,
					   &tpu->ref_pow_cck_cw,
					   phy_idx);
		break;
#endif

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		rtw_hal_mac_write_pwr_ref_reg(bb->hal_com, (enum phl_band_idx)phy_idx);

		halbb_set_tx_pow_ref_8720e(bb, tpu->ref_pow_ofdm,
					   tpu->ref_pow_cck,
					   tpu->ofst_int,
					   tpu->base_cw_0db,
					   tpu->tssi_16dBm_cw,
					   &tpu->ref_pow_ofdm_cw,
					   &tpu->ref_pow_cck_cw,
					   phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_normal_efuse_verify(struct bb_info *bb, s8 rx_gain_offset,
			       enum rf_path rx_path, enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_normal_efuse_verify_8720e(bb, rx_gain_offset, rx_path, phy_idx);
		break;
#endif

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_normal_efuse_verify_8730e(bb, rx_gain_offset, rx_path, phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_normal_efuse_verify_cck(struct bb_info *bb, s8 rx_gain_offset,
				   enum rf_path rx_path,
				   enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

#ifdef BB_8720E_SUPPORT
	case BB_RTL8720E:
		halbb_normal_efuse_verify_cck_8720e(bb, rx_gain_offset, rx_path, phy_idx);
		break;
#endif

#ifdef BB_8730E_SUPPORT
	case BB_RTL8730E:
		halbb_normal_efuse_verify_cck_8730e(bb, rx_gain_offset, rx_path, phy_idx);
		break;
#endif

	default:
		break;
	}
}

void halbb_ic_hw_setting_non_io(struct bb_info *bb)
{
#ifdef HALBB_DYN_1R_CCA_SUPPORT
	halbb_dyn_1r_cca_rst(bb);
#endif
}

void halbb_ic_hw_setting_low_io(struct bb_info *bb)
{
#ifdef HALBB_DYN_1R_CCA_SUPPORT
	halbb_dyn_1r_cca_rst(bb);
#endif
}

void halbb_ic_hw_setting(struct bb_info *bb)
{

#ifdef HALBB_DYN_1R_CCA_SUPPORT
	halbb_dyn_1r_cca(bb);
#endif

	switch (bb->ic_type) {


	default:
		break;
	}
}

void halbb_ic_hw_setting_dbg(struct bb_info *bb, char input[][16],
			     u32 *_used, char *output, u32 *_out_len)
{
#ifdef BB_DYN_CFO_TRK_LOP
	u32 val[5] = {0};
	struct bb_cfo_trk_info *cfo_trk = &bb->bb_cfo_trk_i;
	struct bb_dyn_cfo_trk_lop_info *dctl = &cfo_trk->bb_dyn_cfo_trk_lop_i;
#endif

	if (_os_strcmp(input[1], "-h") == 0) {
#ifdef BB_DYN_CFO_TRK_LOP
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "cfo_trk en {0/1}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "cfo_trk force {0:SNR, 1:link}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "cfo_trk para {0:SNR, 1:link} {data_val} {pilot_val}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "cfo_trk snr_th {th_l} {th_h}\n");
#endif
		return;
	}

#ifdef BB_DYN_CFO_TRK_LOP
	if (_os_strcmp(input[1], "cfo_trk") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);

		if (_os_strcmp(input[2], "en") == 0) {
			halbb_dyn_cfo_trk_loop_en(bb, (bool)val[0]);
		} else if (_os_strcmp(input[2], "force") == 0) {
			halbb_dyn_cfo_trk_loop_en(bb, false);

			if (val[0] == 0) {
				halbb_cfo_trk_loop_cr_cfg(bb, DCTL_SNR);
			} else if (val[0] == 1) {
				halbb_cfo_trk_loop_cr_cfg(bb, DCTL_LINK);
			} else {
				return;
			}

			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "Force state=%d\n", val[0]);
		} else if (_os_strcmp(input[2], "para") == 0) {
			HALBB_SCAN(input[4], DCMD_HEX, &val[1]);
			HALBB_SCAN(input[5], DCMD_HEX, &val[2]);

			if (val[0] >= 2) {
				return;
			}

			dctl->bb_cfo_trk_lop_cr_i[val[0]].dctl_data = (u8)val[1];
			dctl->bb_cfo_trk_lop_cr_i[val[0]].dctl_pilot = (u8)val[2];

			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "state:%d, dctl_data=0x%x, dctl_pilot=0x%x\n", val[0],
				    dctl->bb_cfo_trk_lop_cr_i[val[0]].dctl_data,
				    dctl->bb_cfo_trk_lop_cr_i[val[0]].dctl_pilot);

		} else if (_os_strcmp(input[2], "snr_th") == 0) {
			HALBB_SCAN(input[4], DCMD_DECIMAL, &val[1]);

			dctl->dctl_snr_th_l = (u16)val[0] << RSSI_MA_H;
			dctl->dctl_snr_th_h = (u16)val[1] << RSSI_MA_H;

			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "dctl_snr_th_{l/h}={%d/%d}\n",
				    val[0], val[1]);
		}
	} else
#endif
	{
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "Set Err\n");
	}
}

void halbb_ic_api_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		      char *output, u32 *_out_len)
{
	u32 val[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
#ifdef BB_8852B_SUPPORT
	u32 j = 0;
	bool judge_f;
#endif

	if (_os_strcmp(input[1], "-h") == 0) {
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "trx_path {tx_path} {tx_nss} {rx_path} {rx_nss}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "igi ({lna_idx (0~6)} {tia_idx (0~1)} {rxbb_idx (0~31)} {path (0~1)})\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "Rx_setting({patch_idx (0:default, 1:patch-1, 2:patch-2)})\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rfe dump\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rfe all {rfe_idx}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rfe cmn {gpio_idx} {path} {inv} {src}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rfe rfm {path} {src} {dis_tx} {ac_tx} {ac_bt} {val}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rfe trsw {path} {path_en} {trsw_tx} {trsw_rx} {trsw} {trsw_b}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "rf sw_si rw {run_idx} {rx_path} {reg_addr}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "gain_show {a_mode / g_mode} path\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "op1db_show {a_mode / g_mode} path\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "agc [default | fix_gain | tia_shrink | step_en | -h] ...\n");
	} else if (_os_strcmp(input[1], "trx_path") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);
		HALBB_SCAN(input[4], DCMD_DECIMAL, &val[2]);
		HALBB_SCAN(input[5], DCMD_DECIMAL, &val[3]);
		halbb_ctrl_trx_path(bb, (enum rf_path)val[0], (u8)val[1],
				    (enum rf_path)val[2], (u8)val[3]);

	} else if (_os_strcmp(input[1], "igi") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);
		HALBB_SCAN(input[4], DCMD_DECIMAL, &val[2]);
		HALBB_SCAN(input[5], DCMD_DECIMAL, &val[3]);

		if (val[0] > 6) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "Invalid LNA index!\n");
		} else if (val[1] > 1) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "Invalid TIA index!\n");
		} else if (val[2] > 31) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "Invalid RxBB index!\n");
		} else {
			halbb_set_igi(bb, (u8)val[0], (bool)val[1], (u8)val[2],
				      (enum rf_path)val[3]);
		}
	} else if (_os_strcmp(input[1], "rx_setting") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		if (val[0] > 2) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "Invalid Patch index!\n");
			return;
		}
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			    "Rx setting Patch-%d Success!\n", val[0]);
	} else if (_os_strcmp(input[1], "dbg") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[4], DCMD_DECIMAL, &val[1]);
		HALBB_SCAN(input[5], DCMD_DECIMAL, &val[2]);
#ifdef BB_8852A_2_SUPPORT
		if (_os_strcmp(input[2], "pop_en") == 0) {
			halbb_pop_en(bb, (bool)val[0],
				     (enum phl_phy_idx)val[1]);
		} else if (_os_strcmp(input[2], "set_pd_low") == 0) {
			halbb_set_pd_lower_bound(bb, (u8)val[0],
						 (enum channel_width)val[1],
						 (enum phl_phy_idx)val[2]);
		} else if (_os_strcmp(input[2], "per") == 0) {
			halbb_get_per_8852a_2(bb, (enum phl_phy_idx)val[0]);
		}
#endif
	} else if (_os_strcmp(input[1], "gain_ofst") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);
		HALBB_SCAN(input[4], DCMD_DECIMAL, &val[2]);
		halbb_normal_efuse_verify(bb, (s8)val[0], (enum rf_path)val[1], (enum phl_phy_idx)val[2]);
#ifdef BB_8852B_SUPPORT
	} else if (_os_strcmp(input[1], "rf_sw_si_rw") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);
		HALBB_SCAN(input[4], DCMD_DECIMAL, &val[2]);
		HALBB_SCAN(input[5], DCMD_HEX, &val[3]);

		if (val[0] > 2) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "[SW_SI] Do not run test!\n");
			return;
		} else if (val[1] > 10000) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "[SW_SI] Out-of test range!\n");
			return;
		} else if (val[2] > 3) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "[SW_SI] Wrong path setting!\n");
			return;
		} else if (val[3] > 0xff) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "[SW_SI] Wrong addr setting!\n");
			return;
		} else {

			while (j < val[1]) {
				judge_f = halbb_rf_sw_si_test(bb, (enum rf_path)val[2], (u8)val[3], j);

				if (!judge_f) {
					BB_WARNING("[%s]  while ocunter = %d\n", __func__, j);
					break;
				} else {
					BB_WARNING("[%s]  while ocunter = %d\n", __func__, j);
				}

				j++;
			}

		}
#endif
	}

#if 0
	else if (_os_strcmp(input[1], "sc_idx") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		for (i = 0; i <= 15; i++) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				    "rpt(0, %d) = 0x%x\n", i,
				    halbb_get_csi_buf_idx(bb, (u8)val[0], i));
		}
	}
#endif

	*_used = used;
	*_out_len = out_len;
}

#ifdef HALBB_DIG_MCC_SUPPORT
u8 halbb_upd_mcc_macid(struct bb_info *bb, struct bb_mcc_i *mi)
{
	struct halbb_mcc_dm *mcc_dm = &bb->mcc_dm;
	u8 i = 0, role_ch = 0, band_idx = MCC_BAND_NUM;

	BB_DBG(bb, DBG_DIG, "<====== %s ======>\n", __func__);

	role_ch = mi->chandef->center_ch;

	for (i = 0; i < MCC_BAND_NUM; i++) {
		if (mcc_dm->mcc_rf_ch[i].center_ch == role_ch) {
			band_idx = i;
			break;
		}
	}

	if (band_idx == MCC_BAND_NUM) {
		BB_WARNING("%s, band_idx = %d", __func__, band_idx);
		return HALBB_SET_FAIL;
	}

	if (mi->type == PHL_RTYPE_AP) {
		mcc_dm->softap_macid = mi->self_macid;
		BB_DBG(bb, DBG_DIG, "SoftAP macid = %d\n",
		       mcc_dm->softap_macid);
	}

	return HALBB_SET_SUCCESS;
}


void halbb_mcc_stop(struct bb_info *bb)
{
	struct halbb_mcc_dm *mcc_dm = &bb->mcc_dm;
	u8 i = 0, j = 0;

	BB_DBG(bb, DBG_DIG, "<====== %s ======>\n", __func__);

	if (mcc_dm->mcc_status_en == false) {
		return;
	}

	mcc_dm->mcc_status_en = false;

	for (i = 0; i < MCC_BAND_NUM; i++) {
		mcc_dm->sta_cnt[i] = 0;
		mcc_dm->mcc_rf_ch[i].chan = INVALID_INIT_VAL;
		mcc_dm->mcc_rf_ch[i].center_ch = INVALID_INIT_VAL;
	}
}

u8 halbb_mcc_start(struct bb_info *bb, struct bb_mcc_i *mi_1,
		   struct bb_mcc_i *mi_2)
{
	u8 ret = HALBB_SET_FAIL;
	struct halbb_mcc_dm *mcc_dm = &bb->mcc_dm;

	BB_DBG(bb, DBG_DIG, "<====== %s ======>\n", __func__);

	halbb_mem_cpy(bb, &mcc_dm->mcc_rf_ch[MCC_BAND_1], mi_1->chandef,
		      sizeof(struct rtw_chan_def));

	halbb_mem_cpy(bb, &mcc_dm->mcc_rf_ch[MCC_BAND_2], mi_2->chandef,
		      sizeof(struct rtw_chan_def));

	ret = halbb_upd_mcc_macid(bb, mi_1);
	if (ret != HALBB_SET_SUCCESS) {
		goto exit;
	}

	ret = halbb_upd_mcc_macid(bb, mi_2);
	if (ret != HALBB_SET_SUCCESS) {
		goto exit;
	}

	mcc_dm->mcc_status_en = true;
	ret = HALBB_SET_SUCCESS;
exit:
	return ret;

}
#endif

void halbb_btc_bb_switchchannel_para(struct bb_info *bb, u8 central_ch, u8 bt_on)
{
	halbb_set_gain_error_8730e(bb, central_ch, MAIN_ANT, bt_on);
}

