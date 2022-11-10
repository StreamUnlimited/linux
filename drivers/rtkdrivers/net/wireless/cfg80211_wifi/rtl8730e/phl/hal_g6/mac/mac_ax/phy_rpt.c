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
#include "phy_rpt.h"
#include "mac_priv.h"

#define MAC_AX_RX_CNT_SIZE 96
#define MAC_AX_DISP_QID_HOST 0x2
#define MAC_AX_DISP_QID_WLCPU 0xB
#define MAC_AX_DFS_HDR_SIZE 8
#define MAC_AX_DFS_RPT_SIZE 8
#define MAC_AX_DFS_RPT_SIZE_SH 3
#define MAC_AX_CH_INFO_BUF 0
#define B_AX_CH_INFO_BUF_128 0
#define B_AX_GET_CH_INFO_TO_DIS 0
#define B_AX_GET_CH_INFO_TO_8 2
#define B_AX_GET_CH_INFO_TO_28 7
#define B_AX_CH_INFO_INTVL_DIS 0
#define B_AX_CH_INFO_INTVL_1 1
#define B_AX_CH_INFO_INTVL_2 2
#define B_AX_CH_INFO_INTVL_4 4
#define B_AX_CH_INFO_INTVL_7 7
#define B_AX_CH_INFO_REQ_2 1
#define B_AX_DFS_BUF_64 1

#define MAC_AX_MAC_INFO_USE_SIZE 4
struct mac_ax_mac_info_t {
	u32 dword0;
	u32 dword1;
};

struct mac_ax_dfs_hdr_t {
	u32 dword0;
	u32 dword1;
};
#if 1
static u32 get_ppdu_status_cfg(struct mac_ax_adapter *adapter,
			       struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ppdu_stat *ppdu = &cfg->u.ppdu;
	u32 val;
	u32 ret = 0;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("MAC is not ready\n");
		return ret;
	}

	val = MAC_REG_R32(REG_RCR);
	cfg->sts_append_en = !!(val & BIT_APP_PHYSTS);

	val = MAC_REG_R32(REG_RXPKT_CTL);
	cfg->sts_append_mpdu = !!(val & BIT_APP_PHYSTS_PER_SUBMPDU);

	return MACSUCCESS;
}

static u32 cfg_ppdu_status(struct mac_ax_adapter *adapter,
			   struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ppdu_stat *ppdu = &cfg->u.ppdu;
	u32 val;
	u32 ret = 0;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("MAC is not ready\n");
		goto END;
	}

	val = MAC_REG_R32(REG_RCR);
	if (cfg->sts_append_en) {
		val |= BIT_APP_PHYSTS;
	} else {
		val &= ~BIT_APP_PHYSTS;
	}
	MAC_REG_W32(REG_RCR, val);

	val = MAC_REG_R32(REG_RXPKT_CTL);
	if (cfg->sts_append_mpdu) {
		val |= BIT_APP_PHYSTS_PER_SUBMPDU;
	} else {
		val &= ~BIT_APP_PHYSTS_PER_SUBMPDU;
	}
	MAC_REG_W32(REG_RXPKT_CTL, val);

END:
	return ret;
}

static u32 get_ch_info_cfg(struct mac_ax_adapter *adapter,
			   struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32 val;
	u32 ret = 0;
	struct mac_ax_ch_info *chif = &cfg->u.chif;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("MAC is not ready\n");
		return ret;
	}

	PLTFM_MEMSET(cfg, 0, sizeof(*cfg));

	val = MAC_REG_R32(REG_CHINFO_CFG);

	cfg->en = !!(val & BIT_R_CHINFO_EN);

	return MACSUCCESS;
}

static u32 cfg_ch_info(struct mac_ax_adapter *adapter,
		       struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32 ret = MACSUCCESS;
	struct mac_role_tbl *role;
	struct mac_ax_ch_info *chif = &cfg->u.chif;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		PLTFM_MSG_ERR("MAC is not ready\n");
	}

	if (cfg->chif_en) {
		MAC_REG_W32(REG_CHINFO_CFG, MAC_REG_R32(REG_CHINFO_CFG) | BIT_R_CHINFO_EN);
	}

	return ret;
}

static u32 get_dfs_cfg(struct mac_ax_adapter *adapter,
		       struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_dfs *dfs = &cfg->u.dfs;
	u32 ret = 0, val;

	val = MAC_REG_R32(REG_DFS_CFG);
	cfg->dfs_en = !(val & BIT_R_DFS_STOP);

	val = MAC_REG_R32(REG_DFS_AGG_TIME);
	dfs->num_th = BIT_GET_R_DFS_AGG_NUM(val);
	dfs->timeout = BIT_GET_R_DFS_AGG_TIME(val);

	return MACSUCCESS;
}

static u32 cfg_dfs(struct mac_ax_adapter *adapter,
		   struct mac_ax_phy_rpt_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_dfs *dfs = &cfg->u.dfs;
	u32 ret = 0, val;
	u32 dfs_to = 0;

	if (cfg->dfs_en) {
		MAC_REG_W32(REG_DFS_CFG, MAC_REG_R32(REG_DFS_CFG) & ~BIT_R_DFS_STOP);
	} else {
		MAC_REG_W32(REG_DFS_CFG, MAC_REG_R32(REG_DFS_CFG) | BIT_R_DFS_STOP);
	}

	val = MAC_REG_R32(REG_DFS_AGG_TIME);
	val = BIT_SET_R_DFS_AGG_NUM(val, dfs->num_th);
	val = BIT_SET_R_DFS_AGG_TIME(val, dfs->timeout);
	MAC_REG_W32(REG_DFS_AGG_TIME, val);

	return MACSUCCESS;
}

u32 mac_cfg_phy_rpt(struct mac_ax_adapter *adapter,
		    struct mac_ax_phy_rpt_cfg *rpt)
{
	u32(*handle)(struct mac_ax_adapter * adapter,
		     struct mac_ax_phy_rpt_cfg * rpt);

	switch (rpt->type) {
	case MAC_AX_PPDU_STATUS:
		handle = cfg_ppdu_status;
		break;
	case MAC_AX_CH_INFO:
		handle = cfg_ch_info;
		break;
	case MAC_AX_DFS:
		handle = cfg_dfs;
		break;
	default:
		PLTFM_MSG_ERR("Wrong PHY report type\n");
		return MACFUNCINPUT;
	}

	return handle(adapter, rpt);
}

u32 mac_get_phy_rpt_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_phy_rpt_cfg *rpt)
{
	u32(*handle)(struct mac_ax_adapter * adapter,
		     struct mac_ax_phy_rpt_cfg * rpt);

	switch (rpt->type) {
	case MAC_AX_PPDU_STATUS:
		handle = get_ppdu_status_cfg;
		break;
	case MAC_AX_CH_INFO:
		handle = get_ch_info_cfg;
		break;
	case MAC_AX_DFS:
		handle = get_dfs_cfg;
		break;
	default:
		PLTFM_MSG_ERR("Wrong PHY report type\n");
		return MACFUNCINPUT;
	}

	return handle(adapter, rpt);
}
#endif
static u32 parse_mac_info(struct mac_ax_adapter *adapter,
			  u8 *buf, u32 len,
			  struct mac_ax_ppdu_rpt *rpt)
{
	struct mac_ax_ppdu_usr *usr;
	struct mac_ax_mac_info_t *macinfo;
	u8 i;
	u32 ret = MACSUCCESS;
	u32 accu_size = sizeof(struct mac_ax_mac_info_t);
	u32 val;
	u8 *ptr;

	macinfo = (struct mac_ax_mac_info_t *)buf;

	/* dword0 */
	val = le32_to_cpu(macinfo->dword0);
	rpt->usr_num = (u8)GET_FIELD(val, AX_MAC_INFO_USR_NUM);
	if (rpt->usr_num > MAC_AX_PPDU_MAX_USR) {
		PLTFM_MSG_ERR("The user num in mac info is invalid\n");
		ret = MACPARSEERR;
		goto END;
	}
	rpt->fw_def = (u8)GET_FIELD(val, AX_MAC_INFO_FW_DEFINE);
	rpt->lsig_len = (u16)GET_FIELD(val, AX_MAC_INFO_LSIG_LEN);
	rpt->is_to_self = !!(val & AX_MAC_INFO_IS_TO_SELF);
	rpt->rx_cnt_size = val & AX_MAC_INFO_RX_CNT_VLD ?
			   MAC_AX_RX_CNT_SIZE : 0;

	/* dowrd1 */
	val = le32_to_cpu(macinfo->dword1);
	rpt->service = (u16)GET_FIELD(val, AX_MAC_INFO_SERVICE);
	rpt->plcp_size = (u8)GET_FIELD(val, AX_MAC_INFO_PLCP_LEN) * 8;

	/* dword2 */
	usr = rpt->usr;
	ptr = (u8 *)(macinfo + 1);
	for (i = 0; i < rpt->usr_num; i++, usr++) {
		val = le32_to_cpu(*((u32 *)ptr));
		usr->vld = !!(val & AX_MAC_INFO_MAC_ID_VALID);
		usr->has_data = !!(val & AX_MAC_INFO_HAS_DATA);
		usr->has_ctrl = !!(val & AX_MAC_INFO_HAS_CTRL);
		usr->has_mgnt = !!(val & AX_MAC_INFO_HAS_MGNT);
		usr->has_bcn = !!(val & AX_MAC_INFO_HAS_BCN);
		usr->macid = (u8)GET_FIELD(val, AX_MAC_INFO_MACID);
		accu_size += MAC_AX_MAC_INFO_USE_SIZE;
		ptr += MAC_AX_MAC_INFO_USE_SIZE;
	}

	/* 8-byte alignment */
	accu_size += rpt->usr_num & BIT(0) ? MAC_AX_MAC_INFO_USE_SIZE : 0;
	ptr += rpt->usr_num & BIT(0) ? MAC_AX_MAC_INFO_USE_SIZE : 0;
	if (rpt->rx_cnt_size) {
		rpt->rx_cnt_ptr = ptr;
		accu_size += rpt->rx_cnt_size;
		ptr += rpt->rx_cnt_size;
	}

	if (rpt->plcp_size) {
		rpt->plcp_ptr = ptr;
		accu_size += rpt->plcp_size;
		ptr += rpt->plcp_size;
	}

	if (len > accu_size) {
		rpt->phy_st_ptr = ptr;
		rpt->phy_st_size = len - accu_size;
	}
END:
	return ret;
}

u32 mac_parse_ppdu(struct mac_ax_adapter *adapter,
		   u8 *buf, u32 ppdu_len, u8 mac_info,
		   struct mac_ax_ppdu_rpt *rpt)
{
	u32 ret = MACSUCCESS;

	PLTFM_MEMSET(rpt, 0, sizeof(struct mac_ax_ppdu_rpt));

	if (mac_info) {
		ret = parse_mac_info(adapter, buf, ppdu_len, rpt);
	} else {
		rpt->phy_st_ptr = buf;
		rpt->phy_st_size = ppdu_len;
	}

	return ret;
}

u32 mac_parse_dfs(struct mac_ax_adapter *adapter,
		  u8 *buf, u32 dfs_len, struct mac_ax_dfs_rpt *rpt)
{
	struct mac_ax_dfs_hdr_t *dfs;
	u32 len;
	u32 msk = BIT(MAC_AX_DFS_RPT_SIZE_SH) - 1;
	u32 ret = 0;
	u32 val;


	dfs = (struct mac_ax_dfs_hdr_t *)buf;

	// dword0
	val = le32_to_cpu(dfs->dword0);
	rpt->drop_num = (u16)GET_FIELD(val, AX_DFS_DROP_NUM);
	rpt->max_cont_drop = (u16)GET_FIELD(val, AX_DFS_MAX_CONT_DROP);

	// dword1
	val = le32_to_cpu(dfs->dword1);
	rpt->total_drop = (u16)GET_FIELD(val, AX_DFS_TOTAL_DROP);
	len = dfs_len - MAC_AX_DFS_HDR_SIZE;
	rpt->dfs_num = (u16)(len >> MAC_AX_DFS_RPT_SIZE_SH);
	if (len & msk) {
		PLTFM_MSG_ERR("The DFS report size is wrong\n");
		ret = MACPARSEERR;
	}
	rpt->dfs_ptr = buf + MAC_AX_DFS_HDR_SIZE;

	return ret;
}

u32 mac_rst_drv_info(struct mac_ax_adapter *adapter)
{
	adapter->hw_info->mac_drv_info = MAC_AX_DRV_INFO_NONE;

	return MACSUCCESS;
}