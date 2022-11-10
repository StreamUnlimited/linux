/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "hw.h"
#include "coex.h"
#include "twt.h"
#include "fwofld.h"
#include "mac_priv.h"
#include "trxcfg.h"
#include "common.h"

static struct mac_ax_host_rpr_cfg rpr_cfg_poh = {
	121, /* agg */
	255, /* tmr */
	0, /* agg_def */
	0, /* tmr_def */
	0, /* rsvd */
	MAC_AX_FUNC_EN, /* txok_en */
	MAC_AX_FUNC_EN, /* rty_lmt_en */
	MAC_AX_FUNC_EN, /* lft_drop_en */
	MAC_AX_FUNC_EN /* macid_drop_en */
};

static struct mac_ax_host_rpr_cfg rpr_cfg_stf = {
	121, /* agg */
	255, /* tmr */
	0, /* agg_def */
	0, /* tmr_def */
	0, /* rsvd */
	MAC_AX_FUNC_DIS, /* txok_en */
	MAC_AX_FUNC_DIS, /* rty_lmt_en */
	MAC_AX_FUNC_DIS, /* lft_drop_en */
	MAC_AX_FUNC_DIS /* macid_drop_en */
};

static struct mac_ax_drv_wdt_ctrl wdt_ctrl_def = {
	MAC_AX_PCIE_ENABLE,
	MAC_AX_PCIE_ENABLE
};
#if 1
struct mac_ax_hw_info *mac_get_hw_info(struct mac_ax_adapter *adapter)
{
	return adapter->hw_info->done ? adapter->hw_info : NULL;
}

u32 get_block_tx_sel_msk(enum mac_ax_block_tx_sel src, u32 *msk)
{
	/*0X1594*/
	switch (src) {
	case MAC_AX_CTN_CCA:
		*msk = BIT_CTN_CHK_CCA_P20;
		break;
	case MAC_AX_CTN_SEC20_CCA:
		*msk = BIT_CTN_CHK_CCA_S20;
		break;
	case MAC_AX_CTN_SEC40_CCA:
		*msk = BIT_CTN_CHK_CCA_S40;
		break;
	case MAC_AX_CTN_EDCCA:
		*msk = BIT_CTN_CHK_EDCCA;
		break;
	case MAC_AX_CTN_BTCCA:
		*msk = BIT_CTN_CHK_BTCCA;
		break;
	case MAC_AX_CTN_BASIC_NAV:
		*msk = BIT_CTN_CHK_BASIC_NAV;
		break;
	case MAC_AX_CTN_INTRA_NAV:
		*msk = BIT_CTN_CHK_INTRA_NAV;
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 cfg_block_tx(struct mac_ax_adapter *adapter,
		 enum mac_ax_block_tx_sel src, u8 band, u8 en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val, msk, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("%s: MAC is NOT enabled\n", __func__);
		return ret;
	}

	ret = get_block_tx_sel_msk(src, &msk);
	if (ret) {
		PLTFM_MSG_ERR("%s: %d is NOT supported\n", __func__, src);
		return ret;
	}

	val = MAC_REG_R32(REG_CCA_NAV_CHK);

	if (en) {
		val = val | msk;
	} else {
		val = val & ~msk;
	}
	MAC_REG_W32(REG_CCA_NAV_CHK, val);

	return MACSUCCESS;
}

u32 get_block_tx(struct mac_ax_adapter *adapter,
		 enum mac_ax_block_tx_sel src, u8 band, u8 *en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val, msk, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("%s: MAC is NOT enabled", __func__);
		return ret;
	}

	val = MAC_REG_R32(REG_CCA_NAV_CHK);

	ret = get_block_tx_sel_msk(src, &msk);
	if (ret) {
		PLTFM_MSG_ERR("%s: %d is NOT supported\n", __func__, src);
		return ret;
	}

	*en = !!(val & msk);

	return MACSUCCESS;
}

u32 set_enable_bb_rf(struct mac_ax_adapter *adapter, u8 enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 value32;
	u8 wl_rfc_s0 = 0;
	u8 wl_rfc_s1 = 0;

	if (enable == 1) {
		value32 = MAC_REG_R32(REG_WL_FUNC_EN);
		value32 = value32 | BIT_FEN_BBRSTB_V2 | BIT_FEN_BB_GLB_RSTN_V2;
		MAC_REG_W32(REG_WL_FUNC_EN, value32);
		adapter->sm.bb_func = MAC_AX_FUNC_ON;
	} else {
		adapter->sm.bb_func = MAC_AX_FUNC_OFF;
		value32 = MAC_REG_R32(REG_WL_FUNC_EN);
		value32 = value32 & (~(BIT_FEN_BBRSTB_V2 | BIT_FEN_BB_GLB_RSTN_V2));
		MAC_REG_W32(REG_WL_FUNC_EN, value32);
	}
	return MACSUCCESS;
}

static u32 set_append_fcs(struct mac_ax_adapter *adapter, u8 enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 value32;

	value32 = MAC_REG_R32(REG_RCR);
	value32 = enable == 1 ? value32 | BIT_APP_FCS :
		  value32 & ~BIT_APP_FCS;
	MAC_REG_W32(REG_RCR, value32);

	return MACSUCCESS;
}

static u32 set_accept_icverr(struct mac_ax_adapter *adapter, u8 enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 value32;

	value32 = MAC_REG_R32(REG_RCR);
	value32 = enable == 1 ? (value32 | BIT_AICV) :
		  (value32 & ~BIT_AICV);
	MAC_REG_W32(REG_RCR, value32);

	return MACSUCCESS;
}

u32 get_data_rty_limit(struct mac_ax_adapter *adapter,
		       struct mac_ax_rty_lmt *rty)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	struct mac_role_tbl *role;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("%s: MAC is NOT enabled", __func__);
		return ret;
	}

	rty->tx_cnt = BIT_GET_LRL(MAC_REG_R32(REG_RETRY_LIMIT_SIFS));

	return ret;
}

u32 set_xtal_aac(struct mac_ax_adapter *adapter, u8 aac_mode)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	u8 val8;

	val8 = SYS_REG_R8(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_2);
	val8 = BIT_SET_AAC_MODE(val8, aac_mode);
	SYS_REG_W8(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_2, val8);

	return MACSUCCESS;
}

u32 set_nav_padding(struct mac_ax_adapter *adapter,
		    struct mac_ax_nav_padding *nav)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;
	u8 val8;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		return ret;
	}

	if (nav->nav_pad_en) {
		MAC_REG_W16(REG_NAV_CTRL + 2, nav->nav_padding);
		val8 = MAC_REG_R8(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL);
		if (nav->over_txop_en) {
			val8 |= BIT(6);
		} else {
			val8 &= ~BIT(6);
		}
		MAC_REG_W8(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL, val8);
	} else {
		MAC_REG_W16(REG_NAV_CTRL + 2, 0);
	}

	return MACSUCCESS;
}


u32 mac_get_tx_cnt(struct mac_ax_adapter *adapter,
		   struct mac_ax_tx_cnt *cnt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u8 sel;

	if (check_mac_en(adapter, MAC_AX_MAC_SEL)) {
		return MACHWNOTEN;
	}

	for (sel = 0; sel < MAC_AX_TX_ALLTYPE; sel++) {
		val32 = MAC_REG_R32(REG_TRX_PKTCNT_CTRL);
		val32 = BIT_SET_R_WMAC_TXPKTCNT_SEL(val32, (sel + 4));
		MAC_REG_W32(REG_TRX_PKTCNT_CTRL, val32);

		cnt->txcnt[sel] = MAC_REG_R16(REG_TRX_PKTCNT_RPT);
	}
	return MACSUCCESS;
}

u32 mac_set_adapter_info(struct mac_ax_adapter *adapter,
			 struct mac_ax_adapter_info *set)
{
#ifdef RTW_WKARD_GET_PROCESSOR_ID
	adapter->hw_info->adpt_info.cust_proc_id.proc_id.proc_id_h =
		set->cust_proc_id.proc_id.proc_id_h;
	adapter->hw_info->adpt_info.cust_proc_id.proc_id.proc_id_l =
		set->cust_proc_id.proc_id.proc_id_l;
#endif
	return MACSUCCESS;
}

u32 mac_set_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val)
{
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32 ret = MACSUCCESS;

	if (!val) {
		PLTFM_MSG_ERR("[ERR]: the parameter is NULL in %s\n", __func__);
		return MACNPTR;
	}

	switch (hw_id) {
	case MAC_AX_HW_SETTING:
		break;
	case MAC_AX_HW_SET_SCH_TXEN_CFG:
		ret = set_hw_sch_tx_en(adapter,
				       (struct mac_ax_sch_tx_en_cfg *)val);
		break;
	case MAC_AX_HW_SET_AMPDU_CFG:
		ret = set_hw_ampdu_cfg(adapter, (struct mac_ax_ampdu_cfg *)val);
		break;
	case MAC_AX_HW_SET_FAST_EDCA_PARAM:
		ret = set_hw_fast_edca_param(adapter,
					     (struct mac_ax_usr_edca_param *)val);
		break;
	case MAC_AX_HW_SET_EDCA_PARAM:
		ret = set_hw_edca_param(adapter,
					(struct mac_ax_edca_param *)val);
		break;
	case MAC_AX_HW_SET_MUEDCA_PARAM:
		ret = set_hw_muedca_param(adapter,
					  (struct mac_ax_muedca_param *)val);
		break;
	case MAC_AX_HW_SET_MUEDCA_CTRL:
		ret = set_hw_muedca_ctrl(adapter,
					 (struct mac_ax_muedca_cfg *)val);
		break;
	case MAC_AX_HW_SET_BW_CFG:
		ret = cfg_mac_bw(adapter, (struct mac_ax_cfg_bw *)val);
		break;
	case MAC_AX_HW_SET_LIFETIME_CFG:
		ret = set_hw_lifetime_cfg(adapter,
					  (struct mac_ax_lifetime_cfg *)val);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case MAC_AX_HW_EN_BB_RF:
		ret = set_enable_bb_rf(adapter, *(u8 *)val);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case MAC_AX_HW_SET_APP_FCS:
		set_append_fcs(adapter, *(u8 *)val);
		break;
	case MAC_AX_HW_SET_RX_ICVERR:
		set_accept_icverr(adapter, *(u8 *)val);
		break;
	case MAC_AX_HW_SET_COEX_GNT:
		ret = p_ops->cfg_gnt(adapter, (struct mac_ax_coex_gnt *)val);
		break;
	case MAC_AX_HW_SET_POLLUTED:
		mac_cfg_plt(adapter, (struct mac_ax_plt *)val);
		break;
	case MAC_AX_HW_SET_SLOT_TIME:
		mac_set_slot_time(adapter, *(enum mac_ax_slot_time *)val);
		break;
	case MAC_AX_HW_SET_XTAL_AAC_MODE:
		set_xtal_aac(adapter, *(u8 *)val);
		break;
	case MAC_AX_HW_SET_NAV_PADDING:
		ret = set_nav_padding(adapter, (struct mac_ax_nav_padding *)val);
		break;
	case MAC_AX_HW_SET_HW_RTS_TH:
		ret = mac_set_hw_rts_th(adapter,
					(struct mac_ax_hw_rts_th *)val);
		break;
	case MAC_AX_HW_SET_RRSR_CFG:
		ret = p_ops->set_rrsr_cfg(adapter,
					  (struct mac_ax_rrsr_cfg *)val);
		break;
	case MAC_AX_HW_SET_CSI_RRSR_CFG:
		ret = p_ops->set_csi_rrsr_cfg(adapter,
					      (struct mac_ax_csi_rrsr_cfg *)val);
		break;
	case MAC_AX_HW_SET_ADAPTER:
		ret = mac_set_adapter_info(adapter,
					   (struct mac_ax_adapter_info *)val);
		break;
	case MAC_AX_HW_SET_RESP_ACK_CHK_CCA:
		ret = mac_resp_chk_cca(adapter, (struct mac_ax_resp_chk_cca *)val);
		break;

	default:
		return MACNOITEM;
	}

	return ret;
}

static u32 get_append_fcs(struct mac_ax_adapter *adapter, u8 *enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	*enable = MAC_REG_R32(REG_RCR) & BIT_APP_FCS ? 1 : 0;

	return MACSUCCESS;
}

static u32 get_accept_icverr(struct mac_ax_adapter *adapter, u8 *enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	*enable = MAC_REG_R32(REG_RCR) & BIT_AICV ? 1 : 0;

	return MACSUCCESS;
}

u32 get_pwr_state(struct mac_ax_adapter *adapter, enum mac_ax_mac_pwr_st *st)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val;

	/*0x400000A4 [14] lps; [13] on; [12] off*/
	val = MAC_REG_R32(REG_SYSON_FSM_MON_V1);

	if (val & BIT(12)) {
		*st = MAC_AX_MAC_OFF;
		adapter->mac_pwr_info.pwr_in_lps = 0;
		adapter->sm.fw_rst = MAC_AX_FW_RESET_IDLE;
		adapter->sm.pwr = MAC_AX_PWR_OFF;
		adapter->sm.mac_rdy = MAC_AX_MAC_NOT_RDY;
		PLTFM_MSG_WARN("WL MAC is in off state.\n");
	} else if (val & BIT(13)) {
		*st = MAC_AX_MAC_ON;
	} else if (val & BIT(14)) {
		*st = MAC_AX_MAC_LPS;
	} else {
		PLTFM_MSG_ERR("Unexpected MAC state = 0x%X\n", val);
		return MACPWRSTAT;
	}

	return MACSUCCESS;
}

void get_dflt_nav(struct mac_ax_adapter *adapter, u16 *nav)
{
	/* data NAV is consist of SIFS and ACK/BA time */
	/* currently, we use SIFS + 64-bitmap BA as default NAV */
	/* we use OFDM-6M to estimate BA time */
	/* BA time = PLCP header(20us) + 32 bytes/data_rate */
	*nav = 63;
}

u32 mac_get_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val)
{
	u32 ret = MACSUCCESS;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	if (!val) {
		PLTFM_MSG_ERR("[ERR]: the parameter is NULL in %s\n", __func__);
		return MACNPTR;
	}

	switch (hw_id) {
	case MAC_AX_HW_MAPPING:
		break;
	case MAC_AX_HW_GET_EFUSE_SIZE:
		*(u32 *)val = adapter->hw_info->efuse_size;
		break;
	case MAC_AX_HW_GET_LOGICAL_EFUSE_SIZE:
		*(u32 *)val = adapter->hw_info->log_efuse_size;
		break;
	case MAC_AX_HW_GET_SCH_TXEN_STATUS:
		ret = get_hw_sch_tx_en(adapter,
				       (struct mac_ax_sch_tx_en_cfg *)val);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case MAC_AX_HW_GET_EDCA_PARAM:
		ret = get_hw_edca_param(adapter,
					(struct mac_ax_edca_param *)val);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case MAC_AX_HW_GET_LIFETIME_CFG:
		ret = get_hw_lifetime_cfg(adapter,
					  (struct mac_ax_lifetime_cfg *)val);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case MAC_AX_HW_GET_APP_FCS:
		get_append_fcs(adapter, (u8 *)val);
		break;
	case MAC_AX_HW_GET_RX_ICVERR:
		get_accept_icverr(adapter, (u8 *)val);
		break;
	case MAC_AX_HW_GET_PWR_STATE:
		get_pwr_state(adapter, (enum mac_ax_mac_pwr_st *)val);
		break;
	case MAC_AX_HW_GET_WAKE_REASON:
		ret = get_wake_reason(adapter, (u8 *)val);
		if (ret != 0) {
			return ret;
		}
		break;
	case MAC_AX_HW_GET_COEX_GNT:
		ret = p_ops->get_gnt(adapter, (struct mac_ax_coex_gnt *)val);
		break;
	case MAC_AX_HW_GET_TX_CNT:
		ret = mac_get_tx_cnt(adapter, (struct mac_ax_tx_cnt *)val);
		if (ret != 0) {
			return ret;
		}
		break;
	case MAC_AX_HW_GET_TSF:
		ret = mac_get_tsf(adapter, (struct mac_ax_port_tsf *)val);
		break;
	case MAC_AX_HW_GET_DATA_RTY_LMT:
		get_data_rty_limit(adapter, (struct mac_ax_rty_lmt *)val);
		break;
	case MAC_AX_HW_GET_DFLT_NAV:
		get_dflt_nav(adapter, (u16 *)val);
		break;
	case MAC_AX_HW_GET_FW_CAP:
		ret = mac_get_fw_cap(adapter, (u32 *)val);
		break;
	case MAC_AX_HW_GET_RRSR_CFG:
		ret = p_ops->get_rrsr_cfg(adapter,
					  (struct mac_ax_rrsr_cfg *)val);
		break;
	case MAC_AX_HW_GET_CSI_RRSR_CFG:
		ret = p_ops->get_csi_rrsr_cfg(adapter,
					      (struct mac_ax_csi_rrsr_cfg *)val);
		break;
	default:
		return MACNOITEM;
	}

	return ret;
}

u32 cfg_mac_bw(struct mac_ax_adapter *adapter, struct mac_ax_cfg_bw *cfg)
{
	u8 value8 = 0;
	u16 value16 = 0;
	u8 chk_val8 = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct rtw_hal_com_t *hal_com =
		(struct rtw_hal_com_t *)adapter->drv_adapter;

	u8 txsc20 = 0, txsc40 = 0, txsc80 = 0;

	switch (cfg->cbw) {
	/* fall through */
	case CHANNEL_WIDTH_80:
		txsc40 = rtw_hal_bb_get_txsc(hal_com, cfg->pri_ch,
					     cfg->central_ch, cfg->cbw,
					     CHANNEL_WIDTH_40);
	/* fall through */
	case CHANNEL_WIDTH_40:
		txsc20 = rtw_hal_bb_get_txsc(hal_com, cfg->pri_ch,
					     cfg->central_ch, cfg->cbw,
					     CHANNEL_WIDTH_20);
		break;
	default:
		break;
	}

	value16 = MAC_REG_R16(REG_WMAC_TRXPTCL_CTL);
	value16 = value16 & (~(BIT(7) | BIT(8)));

	value8 = MAC_REG_R8(REG_CFEND_RATE_SC_CTRL + 3);

	switch (cfg->cbw) {
	case CHANNEL_WIDTH_80:
		value8 = value8 | BIT(1);
		value8 = txsc20 | (txsc40 << 4); //TXSC_80M;
		break;
	case CHANNEL_WIDTH_40:
		value16 = value16 | BIT(0);
		value8 = txsc20; //TXSC_40M;
		break;
	case CHANNEL_WIDTH_20:
		value16 = 0; //TXSC_20M;
		break;
	default:
		break;
	}

	MAC_REG_W8(REG_CFEND_RATE_SC_CTRL + 3, value8);

	MAC_REG_W16(REG_WMAC_TRXPTCL_CTL, value16);

	chk_val8 = MAC_REG_R8(REG_BCN_AMPDU_CTCL);
	chk_val8 = chk_val8 & (~(BIT(0)));

	/*Setting for CCK rate in 5G/6G Channel protection*/
	if (cfg->pri_ch >= CHANNEL_5G) { // remove after phl setting band_type
		chk_val8 |= BIT_CHECK_CCK_EN;
	}
	MAC_REG_W8(REG_BCN_AMPDU_CTCL, chk_val8);

	return MACSUCCESS;
}
#endif
/*
u32 mac_write_xtal_si(struct mac_ax_adapter *adapter,
		      u8 offset, u8 val, u8 bitmask)
{
	u32 cnt = 0;
	u32 write_val = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	cnt = XTAL_SI_POLLING_CNT;
	write_val = SET_CLR_WORD(write_val, offset, B_AX_WL_XTAL_SI_ADDR);
	write_val = SET_CLR_WORD(write_val, val, B_AX_WL_XTAL_SI_DATA);
	write_val = SET_CLR_WORD(write_val, bitmask, B_AX_WL_XTAL_SI_BITMASK);
	write_val = SET_CLR_WORD(write_val, XTAL_SI_NORMAL_WRITE,
				 B_AX_WL_XTAL_SI_MODE);
	write_val = (write_val | B_AX_WL_XTAL_SI_CMD_POLL);
	MAC_REG_W32(R_AX_WLAN_XTAL_SI_CTRL, write_val);

	while ((MAC_REG_R32(R_AX_WLAN_XTAL_SI_CTRL) & B_AX_WL_XTAL_SI_CMD_POLL)
						== B_AX_WL_XTAL_SI_CMD_POLL) {
		if (!cnt) {
			PLTFM_MSG_ERR("[ERR]xtal si not ready(W)\n");
			return MACPOLLTO;
		}
		cnt--;
		PLTFM_DELAY_US(XTAL_SI_POLLING_DLY_US);
	}

	return MACSUCCESS;
}

u32 mac_read_xtal_si(struct mac_ax_adapter *adapter,
		     u8 offset, u8 *val)
{
	u32 cnt = 0;
	u32 write_val = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	cnt = XTAL_SI_POLLING_CNT;
	write_val = SET_CLR_WORD(write_val, offset, B_AX_WL_XTAL_SI_ADDR);
	write_val = SET_CLR_WORD(write_val, 0x00, B_AX_WL_XTAL_SI_DATA);
	write_val = SET_CLR_WORD(write_val, 0x00, B_AX_WL_XTAL_SI_BITMASK);
	write_val = SET_CLR_WORD(write_val, XTAL_SI_NORMAL_READ,
				 B_AX_WL_XTAL_SI_MODE);
	write_val = (write_val | B_AX_WL_XTAL_SI_CMD_POLL);
	MAC_REG_W32(R_AX_WLAN_XTAL_SI_CTRL, write_val);

	while ((MAC_REG_R32(R_AX_WLAN_XTAL_SI_CTRL) & B_AX_WL_XTAL_SI_CMD_POLL)
						== B_AX_WL_XTAL_SI_CMD_POLL) {
		if (!cnt) {
			PLTFM_MSG_ERR("[ERR]xtal_si not ready(R)\n");
			return MACPOLLTO;
		}
		cnt--;
		PLTFM_DELAY_US(XTAL_SI_POLLING_DLY_US);
	}

	*val = MAC_REG_R8(R_AX_WLAN_XTAL_SI_CTRL + 1);

	return MACSUCCESS;
}
*/

u32 set_l2_status(struct mac_ax_adapter *adapter)
{
	adapter->sm.l2_st = MAC_AX_L2_EN;

	return MACSUCCESS;
}
#if 1
u32 mac_write_pwr_ofst_mode(struct mac_ax_adapter *adapter,
			    struct rtw_tpu_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;
	s8 *tmp = &tpu->pwr_ofst_mode[0];

	val32 = MAC_REG_R32(REG_AX_PWR_RATE_OFST_CTRL) & ~0xFFFFF;
	val32 |= NIB_2_DW(0, 0, 0, tmp[4], tmp[3], tmp[2], tmp[1], tmp[0]);
	MAC_REG_W32(REG_AX_PWR_RATE_OFST_CTRL, val32);


	return MACSUCCESS;
}

u32 mac_write_pwr_ofst_bw(struct mac_ax_adapter *adapter,
			  struct rtw_tpu_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;
	s8 *tmp = &tpu->pwr_ofst_bw[0];

	val32 = MAC_REG_R32(REG_AX_PWR_LMT_CTRL) & ~0xFFFFF;
	val32 |= NIB_2_DW(0, 0, 0, tmp[4], tmp[3], tmp[2], tmp[1], tmp[0]);
	MAC_REG_W32(REG_AX_PWR_LMT_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_write_pwr_ref_reg(struct mac_ax_adapter *adapter,
			  struct rtw_tpu_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;

	val32 = MAC_REG_R32(REG_AX_PWR_RATE_CTRL) & ~0xFFFFC00;
	val32 |= (((tpu->ref_pow_ofdm & 0x1ff) << 19) |
		  ((tpu->ref_pow_cck & 0x1ff) << 10));
	MAC_REG_W32(REG_AX_PWR_RATE_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_write_pwr_limit_en(struct mac_ax_adapter *adapter,
			   struct rtw_tpu_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;

	val32 = MAC_REG_R32(REG_AX_PWR_LMT_CTRL) & ~0x300000;
	if (tpu->pwr_lmt_en) {
		val32 |=  0x300000;
	}
	MAC_REG_W32(REG_AX_PWR_LMT_CTRL, val32);

	val32 = MAC_REG_R32(REG_AX_PWR_RU_LMT_CTRL) & ~BIT18;
	if (tpu->pwr_lmt_en) {
		val32 |=  BIT18;
	}
	MAC_REG_W32(REG_AX_PWR_RU_LMT_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_read_pwr_reg(struct mac_ax_adapter *adapter,
		     const u32 offset, u32 *val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 access_offset = offset;

	if (offset < REG_AX_PWR_RATE_CTRL || offset > REG_AX_PWR_END) {
		PLTFM_MSG_ERR("[ERR]offset exceed pwr ctrl reg %x\n", offset);
		return MACBADDR;
	}

	*val = MAC_REG_R32(access_offset);

	return MACSUCCESS;
}

u32 mac_write_msk_pwr_reg(struct mac_ax_adapter *adapter,
			  const u32 offset, u32 mask, u32 val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u32 access_offset = offset;
	u32 ori_val = 0;
	u8 shift;

	if (offset < REG_AX_PWR_RATE_CTRL || offset > REG_AX_PWR_END) {
		PLTFM_MSG_ERR("[ERR]offset exceed pwr ctrl reg %x\n", offset);
		return MACBADDR;
	}


	if (mask != 0xffffffff) {
		shift = shift_mask(mask);
		ori_val = MAC_REG_R32(offset);
		val = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}
	MAC_REG_W32(offset, val);

	return MACSUCCESS;
}

u32 mac_write_pwr_reg(struct mac_ax_adapter *adapter,
		      const u32 offset, u32 val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u32 access_offset = offset;

	if (offset < REG_AX_PWR_RATE_CTRL || offset > REG_AX_PWR_END) {
		PLTFM_MSG_ERR("[ERR]offset exceed pwr ctrl reg %x\n", offset);
		return MACBADDR;
	}

	MAC_REG_W32(access_offset, val);

	return MACSUCCESS;
}

u32 mac_write_pwr_limit_rua_reg(struct mac_ax_adapter *adapter,
				struct rtw_tpu_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u16 cr = REG_AX_PWR_RU_LMT_TABLE0;
	s8 *tmp;
	u8 i, j;

	for (i = 0; i < HAL_MAX_PATH; i++) {
		for (j = 0; j < TPU_SIZE_RUA; j++) {
			tmp = &tpu->pwr_lmt_ru[i][j][0];
			MAC_REG_W32(cr, BT_2_DW(tmp[3], tmp[2], tmp[1], tmp[0]));
			cr += 4;
			MAC_REG_W32(cr, BT_2_DW(tmp[7], tmp[6], tmp[5], tmp[4]));
			cr += 4;
		}
	}

	return MACSUCCESS;
}

u32 mac_write_pwr_limit_reg(struct mac_ax_adapter *adapter,
			    struct rtw_tpu_pwr_imt_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ss_ofst = 0;
	u16 cr = 0;
	s8 *tmp, *tmp_1;
	u8 i;

	for (i = 0; i < HAL_MAX_PATH; i++) {
		tmp = &tpu->pwr_lmt_cck_20m[i][0];
		tmp_1 = &tpu->pwr_lmt_cck_40m[i][0];
		cr = REG_AX_PWR_LMT_TABLE1 + ss_ofst;
		MAC_REG_W32(cr, BT_2_DW(0, 0, 0, tmp[0]));

		tmp = &tpu->pwr_lmt_lgcy_20m[i][0];
		tmp_1 = &tpu->pwr_lmt_20m[i][0][0];
		cr = REG_AX_PWR_LMT_TABLE1 + ss_ofst;
		MAC_REG_W32(cr, BT_2_DW(0, tmp_1[0], 0, tmp[0]));

		ss_ofst += PWR_LMT_TBL_UNIT;
	}

	return MACSUCCESS;
}

u32 mac_write_pwr_by_rate_reg(struct mac_ax_adapter *adapter,
			      struct rtw_tpu_pwr_by_rate_info *tpu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ss_ofst = 0;
	u16 cr = 0;
	s8 *tmp;
	u8 i, j;

	for (i = 0; i <= 8; i += 4) {
		tmp = &tpu->pwr_by_rate_lgcy[i];
		cr = REG_AX_PWR_BY_RATE_TABLE0 + i;
		MAC_REG_W32(cr, BT_2_DW(tmp[3], tmp[2], tmp[1], tmp[0]));
	}

	for (i = 0; i < HAL_MAX_PATH; i++) {
		for (j = 0; j <= 12; j += 4) {
			tmp = &tpu->pwr_by_rate[i][j];
			cr = REG_AX_PWR_BY_RATE_TABLE3 + j + ss_ofst;
			MAC_REG_W32(cr, BT_2_DW(tmp[3], tmp[2], tmp[1],
						tmp[0]));
		}
		ss_ofst += 0x10; /*16*/
	}

	return MACSUCCESS;
}

u32 mac_read_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 *val)
{

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (sc_xo) {
		*val = (SYS_REG_R32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0) >>
			BIT_SHIFT_XTAL_SC_XO) & BIT_MASK_XTAL_SC_XO;
	} else {
		*val = (SYS_REG_R32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0) >>
			BIT_SHIFT_XTAL_SC_XI) & BIT_MASK_XTAL_SC_XI;
	}

	return MACSUCCESS;
}

u32 mac_write_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	if (sc_xo) {
		val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0);
		val32 = BIT_SET_XTAL_SC_XO(val32, val);
		SYS_REG_W32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0, val32);
	} else {
		val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0);
		val32 = BIT_SET_XTAL_SC_XI(val32, val);
		SYS_REG_W32(SYSTEM_CTRL_BASE_LP, ANAPAR_XTAL_ON_0, val32);
	}

	return MACSUCCESS;
}

u32 mac_write_bbrst_reg(struct mac_ax_adapter *adapter, u8 val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	if (val)
		MAC_REG_W32(REG_WL_FUNC_EN, val32 | BIT_FEN_BB_GLB_RSTN_V2 |
			    BIT_FEN_BBRSTB_V2);
	else
		MAC_REG_W32(REG_WL_FUNC_EN, val32 & (~(BIT_FEN_BB_GLB_RSTN_V2 |
						       BIT_FEN_BBRSTB_V2)));

	return MACSUCCESS;
}
#endif

/*
u32 mac_get_bt_dis(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	return !!(MAC_REG_R32(R_AX_WL_BT_PWR_CTRL) & B_AX_BT_DISN_EN);
}

u32 mac_set_bt_dis(struct mac_ax_adapter *adapter, u8 en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val;

	val = MAC_REG_R32(R_AX_WL_BT_PWR_CTRL);
	val = en ? val | B_AX_BT_DISN_EN : val & ~B_AX_BT_DISN_EN;
	MAC_REG_W32(R_AX_WL_BT_PWR_CTRL, val);

	return MACSUCCESS;
}
*/

