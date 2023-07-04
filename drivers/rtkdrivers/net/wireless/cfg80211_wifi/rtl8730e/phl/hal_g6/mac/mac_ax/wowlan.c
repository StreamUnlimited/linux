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

#include "wowlan.h"
#include "mac_priv.h"
#include "mac_8730e/init_8730e.h"

static u32 wow_bk_status[4] = {0};
static u32 tgt_ind_orig;
static u32 frm_tgt_ind_orig;
static u32 wol_pattern_orig;
static u32 wol_uc_orig;
static u32 wol_magic_orig;
static u8 nlo_enable_record;

/* km */
u8 rtl8730e_set_keep_alive_cmd(struct mac_ax_adapter *adapter, struct keep_alive *parm)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;
	u8 u1H2CKeepAliveParm[H2C_KEEP_ALIVE_CTRL_LEN] = {0};
	/* km4 default parameters. */
	u8 adopt = 1, check_period = 5;
	u8 ret = _FAIL;
	u8 enable = parm->keepalive_en;

	check_period = parm->period;
	SET_H2CCMD_KEEPALIVE_PARM_ENABLE(u1H2CKeepAliveParm, enable);
	SET_H2CCMD_KEEPALIVE_PARM_ADOPT(u1H2CKeepAliveParm, adopt);
	SET_H2CCMD_KEEPALIVE_PARM_PKT_TYPE(u1H2CKeepAliveParm, parm->packet_type);
	SET_H2CCMD_KEEPALIVE_PARM_CHECK_PERIOD(u1H2CKeepAliveParm, check_period);

	ret = rtw_hal_mac_send_h2c_ameba(hal_com,
					 H2C_KEEP_ALIVE,
					 H2C_KEEP_ALIVE_CTRL_LEN,
					 u1H2CKeepAliveParm);

	return ret;
}

/* km */
u8 rtl8730e_set_disconnect_decision_cmd(struct mac_ax_adapter *adapter,
	struct disconnect_detect *parm)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;
	u8 u1H2CDisconDecisionParm[H2C_DISCON_DECISION_LEN] = {0};
	/* default parameters in km4. */
	u8 adopt = 1, check_period = 20, trypkt_num = 0;
	u8 ret = _FAIL;
	u8 enable = parm->disconnect_en;

	check_period = parm->check_period;
	trypkt_num = parm->try_pkt_count;

	SET_H2CCMD_DISCONDECISION_PARM_ENABLE(u1H2CDisconDecisionParm, enable);
	SET_H2CCMD_DISCONDECISION_PARM_ADOPT(u1H2CDisconDecisionParm, adopt);
	SET_H2CCMD_DISCONDECISION_PARM_CHECK_PERIOD(u1H2CDisconDecisionParm, check_period);
	SET_H2CCMD_DISCONDECISION_PARM_TRY_PKT_NUM(u1H2CDisconDecisionParm, trypkt_num);
	SET_H2CCMD_DISCONDECISION_PARM_TRY_OK_BCN_FAIL_COUNT_EN(u1H2CDisconDecisionParm, parm->tryok_bcnfail_count_en);

	ret = rtw_hal_mac_send_h2c_ameba(hal_com,
					 H2C_DISCON_DECISION,
					 H2C_DISCON_DECISION_LEN,
					 u1H2CDisconDecisionParm);
	return ret;
}

/* km */
u32 rtl8730e_set_remote_wake_ctrl_cmd(struct mac_ax_adapter *adapter, struct wow_global *parm)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)adapter->phl_adapter;
	struct dvobj_priv *pdvobj = (struct dvobj_priv *)(phl_com->drv_priv);
	struct _ADAPTER *drv_adapter = dvobj_get_primary_adapter(pdvobj);
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	struct security_priv *psecuritypriv = &drv_adapter->securitypriv;
	u8 u1H2CRemoteWakeCtrlParm[H2C_REMOTE_WAKE_CTRL_LEN] = {0};
	u8 h2c_aoac_rsvdpage[H2C_AOAC_RSVDPAGE_LOC_LEN] = {0};
	u32 addr = 0;
	u32 page_num = RSVD_PAGE_REMOTE_CTRL_INFO;
	u32 ret = RTW_HAL_STATUS_FAILURE, count = 0;
	u8 enable = parm->wow_en;

	/* copy remote control information to reserved page. */
	addr = TX_PKTBUF_OFFSET + (txff_info->rsvd_bcnq_addr + page_num) * 128;
	PLTFM_MEM_W(addr, (u8 *)&parm->rmcl_info_c,
		    sizeof(struct mac_ax_remotectrl_info_parm_));

	SET_H2CCMD_AOAC_RSVDPAGE_LOC_REMOTE_WAKE_CTRL_INFO(h2c_aoac_rsvdpage, RSVD_PAGE_REMOTE_CTRL_INFO);
	SET_H2CCMD_AOAC_RSVDPAGE_LOC_ARP_RSP(h2c_aoac_rsvdpage, RSVD_PAGE_ARP_RSP);
	ret = rtw_hal_mac_send_h2c_ameba(hal_com,
					 H2C_AOAC_RSVD_PAGE,
					 H2C_AOAC_RSVDPAGE_LOC_LEN,
					 h2c_aoac_rsvdpage);
	if (ret != RTW_HAL_STATUS_SUCCESS) {
		PLTFM_MSG_ERR("set AOAC_RSVDPAGE_LOC failed!\n");
		return ret;
	}

	SET_H2CCMD_REMOTE_WAKECTRL_ENABLE(u1H2CRemoteWakeCtrlParm, enable);
	SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_OFFLOAD_EN(u1H2CRemoteWakeCtrlParm, 1);
#ifdef CONFIG_GTK_OL
	if (psecuritypriv->binstallKCK_KEK == _TRUE &&
		psecuritypriv->dot11PrivacyAlgrthm == _AES_) {
		SET_H2CCMD_REMOTE_WAKE_CTRL_GTK_OFFLOAD_EN(u1H2CRemoteWakeCtrlParm, 1);
	} else {
		printk("no kck or security is not AES\n");
		SET_H2CCMD_REMOTE_WAKE_CTRL_GTK_OFFLOAD_EN(u1H2CRemoteWakeCtrlParm, 0);
	}
#endif

	SET_H2CCMD_REMOTE_WAKE_CTRL_FW_UNICAST_EN(u1H2CRemoteWakeCtrlParm, 1);

	/*
	 * filter NetBios name service pkt to avoid being waked-up
	 * by this kind of unicast pkt this exceptional modification
	 * is used for match competitor's behavior
	 */
	SET_H2CCMD_REMOTE_WAKE_CTRL_NBNS_FILTER_EN(u1H2CRemoteWakeCtrlParm, 1);


	if ((psecuritypriv->dot11PrivacyAlgrthm == _AES_) ||
		(psecuritypriv->dot11PrivacyAlgrthm == _NO_PRIVACY_) ||
		(psecuritypriv->dot11PrivacyAlgrthm == _WEP40_) ||
		(psecuritypriv->dot11PrivacyAlgrthm == _WEP104_)) {
		SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_ACTION(u1H2CRemoteWakeCtrlParm, 0);
	} else {
		SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_ACTION(u1H2CRemoteWakeCtrlParm, 1);
	}

	SET_H2CCMD_REMOTE_WAKE_CTRL_FW_PARSING_UNTIL_WAKEUP(u1H2CRemoteWakeCtrlParm, 1);


	ret = rtw_hal_mac_send_h2c_ameba(hal_com,
					 H2C_REMOTE_WAKE_CTRL,
					 H2C_REMOTE_WAKE_CTRL_LEN,
					 u1H2CRemoteWakeCtrlParm);
	return ret;
}

static u32 send_h2c_gtk_ofld(struct mac_ax_adapter *adapter,
			     struct gtk_ofld *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_arp_ofld(struct mac_ax_adapter *adapter,
			     struct arp_ofld *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_ndp_ofld(struct mac_ax_adapter *adapter,
			     struct ndp_ofld *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_realwow(struct mac_ax_adapter *adapter,
			    struct realwow *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_nlo(struct mac_ax_adapter *adapter,
			struct nlo *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_wakeup_ctrl(struct mac_ax_adapter *adapter,
				struct wakeup_ctrl *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_negative_pattern(struct mac_ax_adapter *adapter,
				     struct negative_pattern *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_cfg_dev2hst_gpio(struct mac_ax_adapter *adapter,
			 struct mac_ax_dev2hst_gpio_info *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_uphy_ctrl(struct mac_ax_adapter *adapter,
			      struct uphy_ctrl *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_wowcam_upd(struct mac_ax_adapter *adapter,
			       struct wowcam_upd *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_cfg_wow_wake(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_wow_wake_info *info,
		     struct mac_ax_remotectrl_info_parm_ *content)
{
	u32 ret = 0, i = 0;
	struct wow_global parm1;
	struct wakeup_ctrl parm2;
	struct mac_role_tbl *role;
	//struct mac_ax_sec_iv_info sec_iv_info = {0};
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	parm2.pattern_match_en = info->pattern_match_en;
	parm2.magic_en = info->magic_en;
	parm2.hw_unicast_en = info->hw_unicast_en;
	parm2.fw_unicast_en = info->fw_unicast_en;
	parm2.deauth_wakeup = info->deauth_wakeup;
	parm2.rekey_wakeup = info->rekey_wakeup;
	parm2.eap_wakeup = info->eap_wakeup;
	parm2.all_data_wakeup = info->all_data_wakeup;
	parm2.mac_id = macid;

	parm1.wow_en = info->wow_en;
	parm1.drop_all_pkt = info->drop_all_pkt;
	parm1.rx_parse_after_wake = info->rx_parse_after_wake;
	parm1.mac_id = macid;
	parm1.pairwise_sec_algo = info->pairwise_sec_algo;
	parm1.group_sec_algo = info->group_sec_algo;
	if (content) {
		PLTFM_MEMCPY(parm1.rmcl_info_c,
			     content,
			     sizeof(struct mac_ax_remotectrl_info_parm_));
	}

	if (info->wow_en) {
		role = mac_role_srch(adapter, macid);
		if (role) {
			tgt_ind_orig = role->info.tgt_ind;
			frm_tgt_ind_orig = role->info.frm_tgt_ind;
			wol_pattern_orig = role->info.wol_pattern;
			wol_uc_orig = role->info.wol_uc;
			wol_magic_orig = role->info.wol_magic;
			wow_bk_status[(macid >> 5)] |= BIT(macid & 0x1F);
			role->info.wol_pattern = (u8)parm2.pattern_match_en;
			role->info.wol_uc = info->hw_unicast_en;
			role->info.wol_magic = info->magic_en;

			ret = mac_change_role(adapter, &role->info);
			if (ret) {
				PLTFM_MSG_ERR("role change failed\n");
				return ret;
			}
		} else {
			PLTFM_MSG_ERR("role search failed\n");
			return MACNOITEM;
		}
	} else {
		if (wow_bk_status[(macid >> 5)] & BIT(macid & 0x1F)) {
			//restore address cam
			role = mac_role_srch(adapter, macid);
			if (role) {
				role->info.tgt_ind = (u8)tgt_ind_orig;
				role->info.frm_tgt_ind = (u8)frm_tgt_ind_orig;
				role->info.wol_pattern = (u8)wol_pattern_orig;
				role->info.wol_uc = (u8)wol_uc_orig;
				role->info.wol_magic = (u8)wol_magic_orig;
				ret = mac_change_role(adapter, &role->info);
				if (ret) {
					PLTFM_MSG_ERR("role change failed\n");
					return ret;
				}
			}
			wow_bk_status[(macid >> 5)] &= ~BIT(macid & 0x1F);
		} else {
			PLTFM_MSG_ERR("role search failed\n");
			return MACNOITEM;
		}
	}

	ret = rtl8730e_set_remote_wake_ctrl_cmd(adapter, &parm1);
	if (ret) {
		PLTFM_MSG_ERR("set wow global failed\n");
	}

	return ret;
}

u32 mac_cfg_disconnect_det(struct mac_ax_adapter *adapter,
			   u8 macid,
			   struct mac_ax_disconnect_det_info *info)
{
	u32 ret = 0;
	struct disconnect_detect parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	parm.disconnect_detect_en = info->disconnect_detect_en;
	parm.tryok_bcnfail_count_en =
		info->tryok_bcnfail_count_en;
	parm.disconnect_en = info->disconnect_en;
	parm.mac_id = macid;
	parm.check_period = info->check_period;
	parm.try_pkt_count = info->try_pkt_count;
	parm.tryok_bcnfail_count_limit =
		info->tryok_bcnfail_count_limit;

	ret = rtl8730e_set_disconnect_decision_cmd(adapter, &parm);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

/* km */
u32 mac_cfg_keep_alive(struct mac_ax_adapter *adapter,
		       u8 macid,
		       struct mac_ax_keep_alive_info *info)
{
	u32 ret = 0;
	struct keep_alive parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	parm.keepalive_en = info->keepalive_en;
	parm.packet_type = info->packet_type;
	parm.period = info->period;
	parm.mac_id = macid;

	ret = rtl8730e_set_keep_alive_cmd(adapter, &parm);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_cfg_gtk_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_gtk_ofld_info *info,
		     struct mac_ax_gtk_info_parm_ *content)
{
	u32 ret = 0;
	struct gtk_ofld parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	parm.gtk_en = info->gtk_en;
	parm.tkip_en = info->tkip_en;
	parm.ieee80211w_en = info->ieee80211w_en;
	parm.pairwise_wakeup = info->pairwise_wakeup;
	parm.norekey_wakeup = info->norekey_wakeup;
	parm.mac_id = macid;
	parm.gtk_rsp_id = info->gtk_rsp_id;
	parm.pmf_sa_query_id = info->pmf_sa_query_id;
	parm.bip_sec_algo = info->bip_sec_algo;
	parm.algo_akm_suit = info->algo_akm_suit;

	if (content)
		PLTFM_MEMCPY(&parm.gtk_info_content, content,
			     sizeof(struct mac_ax_gtk_info_parm_));

	ret = send_h2c_gtk_ofld(adapter, &parm);
	if (ret) {
		return ret;
	}
	return MACSUCCESS;
}

u32 mac_cfg_arp_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_arp_ofld_info *info,
		     void *parp_info_content)
{
	u32 ret = 0;
	struct arp_ofld parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct arp_ofld));
	parm.arp_en = info->arp_en;
	parm.arp_action = info->arp_action;
	parm.mac_id = macid;
	parm.arp_rsp_id = info->arp_rsp_id;

	return MACSUCCESS;
}

u32 mac_cfg_ndp_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_ndp_ofld_info *info,
		     struct mac_ax_ndp_info_parm_ *content)
{
	u32 ret = 0;
	struct ndp_ofld parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct ndp_ofld));
	parm.ndp_en = info->ndp_en;
	parm.na_id = info->na_id;
	parm.mac_id = macid;

	if (content)
		PLTFM_MEMCPY(&parm.ndp_info_content, content,
			     sizeof(struct mac_ax_ndp_info_parm_) * 2);

	ret = send_h2c_ndp_ofld(adapter, &parm);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_cfg_realwow(struct mac_ax_adapter *adapter,
		    u8 macid,
		    struct mac_ax_realwow_info *info,
		    struct mac_ax_realwowv2_info_parm_ *content)
{
	u32 ret = 0;
	struct realwow parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct realwow));
	parm.realwow_en = info->realwow_en;
	parm.auto_wakeup = info->auto_wakeup;
	parm.mac_id = macid;
	parm.keepalive_id = info->keepalive_id;
	parm.wakeup_pattern_id = info->wakeup_pattern_id;
	parm.ack_pattern_id = info->ack_pattern_id;
	if (content)
		PLTFM_MEMCPY(&parm.realwow_info_content, content,
			     sizeof(struct mac_ax_realwowv2_info_parm_));

	ret = send_h2c_realwow(adapter, &parm);
	if (ret) {
		return ret;
	}
	return MACSUCCESS;
}

u32 mac_cfg_nlo(struct mac_ax_adapter *adapter,
		u8 macid,
		struct mac_ax_nlo_info *info,
		struct mac_ax_nlo_networklist_parm_ *content)
{
	u32 ret = 0;
	struct nlo parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct nlo));
	parm.nlo_en = info->nlo_en;
	parm.nlo_32k_en = info->nlo_32k_en;
	parm.ignore_cipher_type = info->ignore_cipher_type;
	parm.mac_id = macid;

	if (content)
		PLTFM_MEMCPY(&parm.nlo_networklistinfo_content,
			     content,
			     sizeof(struct mac_ax_nlo_networklist_parm_));

	ret = send_h2c_nlo(adapter, &parm);
	if (ret) {
		return ret;
	}
	if (info->nlo_en) {
		nlo_enable_record = 1;
	}
	return MACSUCCESS;
}

u32 mac_cfg_uphy_ctrl(struct mac_ax_adapter *adapter,
		      struct mac_ax_uphy_ctrl_info *info)
{
	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	return MACSUCCESS;
}

u32 mac_cfg_wowcam_upd(struct mac_ax_adapter *adapter,
		       struct mac_ax_wowcam_upd_info *info)
{
	u32 ret = 0;
	struct wowcam_upd parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct wowcam_upd));
	parm.r_w = info->r_w;
	parm.idx = info->idx;
	parm.wkfm1 = info->wkfm1;
	parm.wkfm2 = info->wkfm2;
	parm.wkfm3 = info->wkfm3;
	parm.wkfm4 = info->wkfm4;
	parm.crc = info->crc;
	parm.negative_pattern_match = info->negative_pattern_match;
	parm.skip_mac_hdr = info->skip_mac_hdr;
	parm.uc = info->uc;
	parm.mc = info->mc;
	parm.bc = info->bc;
	parm.valid = info->valid;

	ret = send_h2c_wowcam_upd(adapter, &parm);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 get_wake_reason(struct mac_ax_adapter *adapter, u8 *wowlan_wake_reason)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_c2hreg_offset *c2hreg_offset;

	c2hreg_offset = p_ops->get_c2hreg_offset(adapter);
	if (!c2hreg_offset) {
		PLTFM_MSG_ERR("%s: get c2hreg offset fail\n", __func__);
		return MACNPTR;
	}

	*wowlan_wake_reason = MAC_REG_R8(c2hreg_offset->data3 + 3);

	return MACSUCCESS;
}

u32 mac_get_wow_wake_rsn(struct mac_ax_adapter *adapter, u8 *wake_rsn,
			 u8 *reset)
{
	u32 ret = MACSUCCESS;

	ret = get_wake_reason(adapter, wake_rsn);
	if (ret != MACSUCCESS) {
		return ret;
	}

	switch (*wake_rsn) {
	case MAC_AX_WOW_DMAC_ERROR_OCCURRED:
	case MAC_AX_WOW_EXCEPTION_OCCURRED:
	case MAC_AX_WOW_L0_TO_L1_ERROR_OCCURRED:
	case MAC_AX_WOW_ASSERT_OCCURRED:
	case MAC_AX_WOW_L2_ERROR_OCCURRED:
	case MAC_AX_WOW_WDT_TIMEOUT_WAKE:
		*reset = 1;
		break;
	default:
		*reset = 0;
		break;
	}

	return MACSUCCESS;
}

u32 mac_cfg_wow_sleep(struct mac_ax_adapter *adapter,
		      u8 sleep)
{
	u32 ret;
	u32 val32;
	u8 dbg_page;
	struct mac_ax_phy_rpt_cfg cfg;
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

#if 0
	PLTFM_MEMSET(&cfg, 0, sizeof(struct mac_ax_phy_rpt_cfg));
#if MAC_AX_FW_REG_OFLD
	if (adapter->sm.fwdl == MAC_AX_FWDL_INIT_RDY) {
		if (sleep) {
			ret = _patch_redu_rx_qta(adapter);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]patch reduce rx qta %d\n", ret);
				return ret;
			}

			cfg.type = MAC_AX_PPDU_STATUS;
			cfg.en = 0;
			ret = mac_ops->cfg_phy_rpt(adapter, &cfg);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]cfg_phy_rpt failed %d\n", ret);
				return ret;
			}

			ret = MAC_REG_W_OFLD(R_AX_RX_FUNCTION_STOP, B_AX_HDR_RX_STOP, 1, 0);
			if (ret) {
				return ret;
			}
			if (nlo_enable_record == 0) {
				ret = MAC_REG_W_OFLD(R_AX_RX_FLTR_OPT, B_AX_SNIFFER_MODE, 0, 0);
			} else {
				ret = MAC_REG_W_OFLD(R_AX_RX_FLTR_OPT, B_AX_SNIFFER_MODE, 1, 0);
				PLTFM_MSG_TRACE("Enable sniffer mode since nlo enable");
				nlo_enable_record = 1;
			}
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_ACTION_FWD0, 0x00000000, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_ACTION_FWD1, 0x00000000, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_TF_FWD, 0x00000000, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_HW_RPT_FWD, 0x00000000, 1);
			if (ret) {
				return ret;
			}
		} else {
			ret = _patch_restr_rx_qta(adapter);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]patch resume rx qta %d\n", ret);
				return ret;
			}

			ret = MAC_REG_W_OFLD(R_AX_RX_FUNCTION_STOP, B_AX_HDR_RX_STOP, 0, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W_OFLD(R_AX_RX_FLTR_OPT, B_AX_SNIFFER_MODE, 1, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_ACTION_FWD0, TRXCFG_MPDU_PROC_ACT_FRWD, 0);
			if (ret) {
				return ret;
			}
			ret = MAC_REG_W32_OFLD(R_AX_TF_FWD, TRXCFG_MPDU_PROC_TF_FRWD, 1);
			if (ret) {
				return ret;
			}

			cfg.type = MAC_AX_PPDU_STATUS;
			cfg.en = 1;
			ret = mac_ops->cfg_phy_rpt(adapter, &cfg);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]cfg_phy_rpt failed %d\n", ret);
				return ret;
			}
		}
		return MACSUCCESS;
	}
#endif
	if (sleep) {
		ret = _patch_redu_rx_qta(adapter);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]patch reduce rx qta %d\n", ret);
			return ret;
		}
		val32 = MAC_REG_R32(R_AX_RX_FUNCTION_STOP);
		val32 |= B_AX_HDR_RX_STOP;
		MAC_REG_W32(R_AX_RX_FUNCTION_STOP, val32);
		if (nlo_enable_record == 0) {
			val32 = MAC_REG_R32(R_AX_RX_FLTR_OPT);
			val32 &= ~B_AX_SNIFFER_MODE;
			MAC_REG_W32(R_AX_RX_FLTR_OPT, val32);
		} else {
			val32 = MAC_REG_R32(R_AX_RX_FLTR_OPT);
			val32 |= B_AX_SNIFFER_MODE;
			MAC_REG_W32(R_AX_RX_FLTR_OPT, val32);
			PLTFM_MSG_TRACE("Enable sniffer mode since nlo enable");
			nlo_enable_record = 0;
		}

		cfg.type = MAC_AX_PPDU_STATUS;
		cfg.en = 0;
		ret = mac_ops->cfg_phy_rpt(adapter, &cfg);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]cfg_phy_rpt failed %d\n", ret);
			return ret;
		}

		MAC_REG_W32(R_AX_ACTION_FWD0, 0x00000000);
		MAC_REG_W32(R_AX_ACTION_FWD1, 0x00000000);
		MAC_REG_W32(R_AX_TF_FWD, 0x00000000);
		MAC_REG_W32(R_AX_HW_RPT_FWD, 0x00000000);
	} else {
		ret = _patch_restr_rx_qta(adapter);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]patch resume rx qta %d\n", ret);
			return ret;
		}
		val32 = MAC_REG_R32(R_AX_RX_FUNCTION_STOP);
		val32 &= ~B_AX_HDR_RX_STOP;
		MAC_REG_W32(R_AX_RX_FUNCTION_STOP, val32);
		val32 = MAC_REG_R32(R_AX_RX_FLTR_OPT);
		MAC_REG_W32(R_AX_RX_FLTR_OPT, val32);

		cfg.type = MAC_AX_PPDU_STATUS;
		cfg.en = 1;
		ret = mac_ops->cfg_phy_rpt(adapter, &cfg);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]cfg_phy_rpt failed %d\n", ret);
			return ret;
		}

		MAC_REG_W32(R_AX_ACTION_FWD0, TRXCFG_MPDU_PROC_ACT_FRWD);
		MAC_REG_W32(R_AX_TF_FWD, TRXCFG_MPDU_PROC_TF_FRWD);

		/*PLTFM_MSG_ERR("[wow] Start to dump PLE debug pages\n");
		for (dbg_page = 0; dbg_page < 4; dbg_page++)
			mac_dump_ple_dbg_page(adapter, dbg_page);*/
	}
#endif
	return MACSUCCESS;
}

u32 mac_get_wow_fw_status(struct mac_ax_adapter *adapter, u8 *status,
			  u8 func_en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (func_en) {
		func_en = 1;
	}

	*status = !!((MAC_REG_R8(REG_WOW_NAN_CTRL) & BIT_WFMSK));

	if (func_en == *status) {
		*status = 1;
	} else {
		*status = 0;
	}

	return MACSUCCESS;
}

u32 _mac_request_aoac_report_rx_rdy(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 _mac_request_aoac_report_rx_not_rdy(struct mac_ax_adapter *adapter)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 mac_request_aoac_report(struct mac_ax_adapter *adapter,
			    u8 rx_ready)
{
	u32 ret;
	struct mac_ax_wowlan_info *wow_info = &adapter->wowlan_info;

	if (adapter->sm.aoac_rpt != MAC_AX_AOAC_RPT_IDLE) {
		return MACPROCERR;
	}

	if (wow_info->aoac_report) {
		PLTFM_FREE(wow_info->aoac_report,
			   sizeof(struct mac_ax_aoac_report));
	}
	wow_info->aoac_report = (u8 *)PLTFM_MALLOC(sizeof(struct mac_ax_aoac_report));
	if (!wow_info->aoac_report) {
		PLTFM_MSG_ERR("%s: malloc fail\n", __func__);
		return MACBUFALLOC;
	}

	adapter->sm.aoac_rpt = MAC_AX_AOAC_RPT_H2C_SENDING;

	if (rx_ready) {
		ret = _mac_request_aoac_report_rx_rdy(adapter);
	} else {
		ret = _mac_request_aoac_report_rx_not_rdy(adapter);
	}

	return ret;
}

u32 mac_read_aoac_report(struct mac_ax_adapter *adapter,
			 struct mac_ax_aoac_report *rpt_buf, u8 rx_ready)
{
	struct mac_ax_wowlan_info *wow_info = &adapter->wowlan_info;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u8 cnt = 200;
	u32 val32;

#if 0
	while ((rx_ready) && (adapter->sm.aoac_rpt != MAC_AX_AOAC_RPT_H2C_DONE)) {
		PLTFM_DELAY_MS(1);
		if (--cnt == 0) {
			PLTFM_MSG_ERR("[ERR] read aoac report(%d) fail\n",
				      adapter->sm.aoac_rpt);
			adapter->sm.aoac_rpt = MAC_AX_AOAC_RPT_IDLE;
			val32 = MAC_REG_R32(R_AX_CH12_TXBD_IDX);
			PLTFM_MSG_ERR("CH12_TXBD=%x\n", val32);
			MAC_REG_W32(R_AX_PLE_DBG_FUN_INTF_CTL, 80010002);
			val32 = MAC_REG_R32(R_AX_PLE_DBG_FUN_INTF_DATA);
			PLTFM_MSG_ERR("PLE_C2H=%x\n", val32);
			MAC_REG_W32(R_AX_PLE_DBG_FUN_INTF_CTL, 80010003);
			val32 = MAC_REG_R32(R_AX_PLE_DBG_FUN_INTF_DATA);
			PLTFM_MSG_ERR("PLE_H2C=%x\n", val32);
			val32 = MAC_REG_R32(REG_RXQ_RXBD_IDX);
			PLTFM_MSG_ERR("RXQ_RXBD=%x\n\n", val32);
			return MACPOLLTO;
		}
	}

	if (wow_info->aoac_report) {
		PLTFM_MEMCPY(rpt_buf, wow_info->aoac_report,
			     sizeof(struct mac_ax_aoac_report));
		PLTFM_FREE(wow_info->aoac_report,
			   sizeof(struct mac_ax_aoac_report));
		wow_info->aoac_report = NULL;
	} else {
		PLTFM_MSG_ERR("[ERR] aoac report memory allocate fail\n");
		ret = MACBUFALLOC;
	}

	adapter->sm.aoac_rpt = MAC_AX_AOAC_RPT_IDLE;
#endif

	return ret;
}

u32 mac_check_aoac_report_done(struct mac_ax_adapter *adapter)
{
	PLTFM_MSG_TRACE("[TRACE]%s: curr state: %d\n", __func__,
			adapter->sm.aoac_rpt);

	if (adapter->sm.aoac_rpt == MAC_AX_AOAC_RPT_H2C_DONE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_wow_stop_trx(struct mac_ax_adapter *adapter)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 mac_cfg_wow_auto_test(struct mac_ax_adapter *adapter, u8 rxtest)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}
