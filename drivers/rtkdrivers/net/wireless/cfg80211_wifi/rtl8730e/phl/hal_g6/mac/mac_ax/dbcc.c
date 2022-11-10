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

#include "dbcc.h"
#include "mac_priv.h"
#include "cpuio.h"

#if 0
u32 dbcc_wmm_add_macid(struct mac_ax_adapter *adapter,
		       struct mac_ax_role_info *info)
{
	struct mac_ax_dbcc_info *dbcc_info = adapter->dbcc_info;
	enum mac_ax_ss_wmm_tbl dst_link;
	u8 *cur_dbcc_wmm_type;
	u32 ret;
	u8 chk_emp, tar_dbcc_wmm, wmmidx, curr_bp;

	if (!dbcc_info) {
		PLTFM_MSG_ERR("no dbcc info when add macid\n");
		return MACNPTR;
	}

	curr_bp = (info->port & DBCC_PORT_MASK) |
		  (info->band == MAC_AX_BAND_1 ? DBCC_BAND_BIT : 0);
	tar_dbcc_wmm = MAC_AX_DBCC_WMM_INVALID;

	if (info->net_type == MAC_AX_NET_TYPE_AP) {
		for (wmmidx = MAC_AX_DBCC_WMM0; wmmidx < MAC_AX_DBCC_WMM_MAX; wmmidx++) {
			if (dbcc_info->dbcc_wmm_bp[wmmidx] == curr_bp &&
			    dbcc_info->dbcc_wmm_type[wmmidx] == info->net_type) {
				break;
			}
		}

		if (wmmidx < MAC_AX_DBCC_WMM_MAX) {
			tar_dbcc_wmm = wmmidx;
		}
	}

	cur_dbcc_wmm_type = &dbcc_info->dbcc_wmm_type[tar_dbcc_wmm];
	if ((info->net_type != MAC_AX_NET_TYPE_AP &&
	     *cur_dbcc_wmm_type != MAC_AX_NET_TYPE_INVLAID) ||
	    (info->net_type == MAC_AX_NET_TYPE_AP &&
	     *cur_dbcc_wmm_type != MAC_AX_NET_TYPE_INVLAID &&
	     *cur_dbcc_wmm_type != MAC_AX_NET_TYPE_AP)) {
		PLTFM_MSG_ERR("dbcc wmm%d is occupied by type%d\n",
			      tar_dbcc_wmm, *cur_dbcc_wmm_type);
		return MACPROCERR;
	}

	if (info->macid >= DBCC_WMM_LIST_SIZE) {
		PLTFM_MSG_ERR("macid%d not support in dbcc wmm add macid\n",
			      info->macid);
		return MACFUNCINPUT;
	}

	if (info->band == MAC_AX_BAND_0)
		dst_link = info->wmm ?
			   MAC_AX_SS_WMM_TBL_C0_WMM1 : MAC_AX_SS_WMM_TBL_C0_WMM0;
	else
		dst_link = info->wmm ?
			   MAC_AX_SS_WMM_TBL_C1_WMM1 : MAC_AX_SS_WMM_TBL_C1_WMM0;
	chk_emp = (!dbcc_info->dbcc_role_cnt[tar_dbcc_wmm] ||
		   *cur_dbcc_wmm_type != MAC_AX_NET_TYPE_AP) ? 1 : 0;
	ret = mac_ss_wmm_map_upd(adapter, (enum mac_ax_ss_wmm)tar_dbcc_wmm,
				 dst_link, chk_emp);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("mac_ss_wmm_map_upd %d/%d/%d/%d\n",
			      tar_dbcc_wmm, dst_link, chk_emp, ret);
		return ret;
	}

	*(dbcc_info->dbcc_wmm_list + info->macid) = tar_dbcc_wmm;
	dbcc_info->dbcc_role_cnt[tar_dbcc_wmm]++;
	*cur_dbcc_wmm_type = info->net_type;
	dbcc_info->dbcc_wmm_bp[tar_dbcc_wmm] = curr_bp;

	return MACSUCCESS;
}

u32 dbcc_wmm_rm_macid(struct mac_ax_adapter *adapter,
		      struct mac_ax_role_info *info)
{
	struct mac_ax_dbcc_info *dbcc_info = adapter->dbcc_info;
	u8 dbcc_wmm;

	if (!dbcc_info) {
		PLTFM_MSG_ERR("no dbcc info when rm macid\n");
		return MACNPTR;
	}

	if (info->macid >= DBCC_WMM_LIST_SIZE) {
		PLTFM_MSG_ERR("macid%d not support in dbcc wmm rm macid\n",
			      info->macid);
		return MACFUNCINPUT;
	}

	dbcc_wmm = *(dbcc_info->dbcc_wmm_list + info->macid);
	if (dbcc_wmm == MAC_AX_DBCC_WMM_INVALID) {
		PLTFM_MSG_ERR("macid%d dbcc wmm invalid in dbcc wmm list\n",
			      info->macid);
		return MACPROCERR;
	}

	if (!dbcc_info->dbcc_role_cnt[dbcc_wmm]) {
		PLTFM_MSG_ERR("dbcc wmm%d rm macid%d underflow\n",
			      dbcc_wmm, info->macid);
		if (dbcc_info->dbcc_wmm_type[dbcc_wmm] !=
		    MAC_AX_NET_TYPE_INVLAID) {
			PLTFM_MSG_ERR("dbcc wmm not invalid when no role\n");
			return MACPROCERR;
		}
		return MACCMP;
	}

	*(dbcc_info->dbcc_wmm_list + info->macid) = MAC_AX_DBCC_WMM_INVALID;
	dbcc_info->dbcc_role_cnt[dbcc_wmm]--;
	if (!dbcc_info->dbcc_role_cnt[dbcc_wmm]) {
		dbcc_info->dbcc_wmm_type[dbcc_wmm] = MAC_AX_NET_TYPE_INVLAID;
		dbcc_info->dbcc_wmm_bp[dbcc_wmm] = DBCC_BP_INVALID;
	}

	return MACSUCCESS;
}


u32 dbcc_trx_ctrl_bkp(struct mac_ax_adapter *adapter, enum mac_ax_band band)
{
	struct mac_ax_ops *mops = adapter_to_mac_ops(adapter);
	struct mac_ax_dbcc_info *dbcc_info = adapter->dbcc_info;
	struct mac_ax_phy_rpt_cfg *ppdu_rpt_cfg;
	struct mac_ax_phy_rpt_cfg *chinfo_cfg;
	u32 ret;

	if (!dbcc_info) {
		PLTFM_MSG_ERR("no dbcc info when bkp trx\n");
		return MACNPTR;
	}

	if (dbcc_info->bkp_flag[band]) {
		PLTFM_MSG_ERR("band%d trx ctrl already backup\n", band);
		return MACPROCERR;
	}

	ppdu_rpt_cfg = &dbcc_info->ppdu_rpt_bkp[band];
	ppdu_rpt_cfg->type = MAC_AX_PPDU_STATUS;
	ppdu_rpt_cfg->u.ppdu.band = band;
	ret = mops->get_phy_rpt_cfg(adapter, ppdu_rpt_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("get ppdu stus rpt cfg %d\n", ret);
		return ret;
	}

	chinfo_cfg = &dbcc_info->chinfo_bkp[band];
	chinfo_cfg->type = MAC_AX_CH_INFO;
	ret = mops->get_phy_rpt_cfg(adapter, chinfo_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("get ch info cfg %d\n", ret);
		return ret;
	}

	dbcc_info->bkp_flag[band] = 1;
	return ret;
}

u32 mac_dbcc_trx_ctrl(struct mac_ax_adapter *adapter,
		      enum mac_ax_band band, u8 pause)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ops *mops = adapter_to_mac_ops(adapter);
	struct mac_ax_dbcc_info *dbcc_info = adapter->dbcc_info;
	struct mac_ax_sch_tx_en_cfg sch_cfg;
	struct mac_ax_phy_rpt_cfg phyrpt_cfg;
	struct mac_ax_phy_rpt_cfg *ppdu_rpt_bkp;
	struct mac_ax_phy_rpt_cfg *chinfo_cfg_kbp;
	u32 ret, reg;
	u16 val16;

	if (!dbcc_info) {
		PLTFM_MSG_ERR("no dbcc info when trx ctrl\n");
		return MACNPTR;
	}

	if (pause) {
		ret = dbcc_trx_ctrl_bkp(adapter, band);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("B%d dbcc trx bkp %d\n", band, ret);
			return ret;
		}
	} else if (!dbcc_info->bkp_flag[band]) {
		PLTFM_MSG_ERR("B%d dbcc trx is not backup\n", band);
		return MACPROCERR;
	}

	if (!pause && band == MAC_AX_BAND_1 &&
	    check_mac_en(adapter, MAC_AX_BAND_1, MAC_AX_CMAC_SEL) != MACSUCCESS) {
		dbcc_info->bkp_flag[band] = 0;
		return MACSUCCESS;
	}

	ppdu_rpt_bkp = &dbcc_info->ppdu_rpt_bkp[band];
	chinfo_cfg_kbp = &dbcc_info->chinfo_bkp[band];

	sch_cfg.band = (u8)band;
	if (pause) {
		PLTFM_MEMSET(&sch_cfg.tx_en, 0, SCH_TX_EN_SIZE);
	} else {
		PLTFM_MEMSET(&sch_cfg.tx_en, 0xFF, SCH_TX_EN_SIZE);
	}
	PLTFM_MEMSET(&sch_cfg.tx_en_mask, 0xFF, SCH_TX_EN_SIZE);
	ret = mops->set_hw_value(adapter, MAC_AX_HW_SET_SCH_TXEN_CFG,
				 (void *)&sch_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("B%d pause%d sch txen cfg %d\n", band, pause, ret);
		return ret;
	}

	reg = band == MAC_AX_BAND_0 ? R_AX_RCR : R_AX_RCR_C1;
	val16 = MAC_REG_R16(reg);
	if (pause) {
		MAC_REG_W16(reg, val16 | B_AX_STOP_RX_IN);
	} else {
		MAC_REG_W16(reg, val16 & ~B_AX_STOP_RX_IN);
	}

	phyrpt_cfg.type = MAC_AX_PPDU_STATUS;
	phyrpt_cfg.en = pause ? 0 : ppdu_rpt_bkp->en;
	phyrpt_cfg.dest = ppdu_rpt_bkp->dest;
	phyrpt_cfg.u.ppdu = ppdu_rpt_bkp->u.ppdu;
	ret = mops->cfg_phy_rpt(adapter, &phyrpt_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("B%d pause%d ppdu status cfg %d\n", band, pause, ret);
		return ret;
	}

	phyrpt_cfg.type = MAC_AX_CH_INFO;
	phyrpt_cfg.en = pause ? 0 : chinfo_cfg_kbp->en;
	phyrpt_cfg.dest = chinfo_cfg_kbp->dest;
	phyrpt_cfg.u.chif = chinfo_cfg_kbp->u.chif;
	ret = mops->cfg_phy_rpt(adapter, &phyrpt_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("B%d pause%d ch info cfg %d\n", band, pause, ret);
		return ret;
	}

	if (!pause) {
		dbcc_info->bkp_flag[band] = 0;
	}

	return ret;
}

u32 mac_dbcc_move_wmm(struct mac_ax_adapter *adapter,
		      struct mac_ax_role_info *info)
{
	struct mac_ax_dbcc_info *dbcc_info = adapter->dbcc_info;
	struct deq_enq_info q_info;
	enum mac_ax_ss_wmm_tbl dst_link;
	u8 dbcc_wmm;
	u8 role_cnt;
	u8 wmm_type;
	u8 dbcc_en = info->band;
	u32 ret;

	if (!dbcc_info) {
		PLTFM_MSG_ERR("no dbcc info when move wmm\n");
		return MACNPTR;
	}

	dbcc_wmm = *(dbcc_info->dbcc_wmm_list + info->macid);
	role_cnt = dbcc_info->dbcc_role_cnt[dbcc_wmm];
	wmm_type = dbcc_info->dbcc_wmm_type[dbcc_wmm];

	if (!role_cnt) {
		PLTFM_MSG_ERR("dbcc en%d move wmm%d role cnt 0\n",
			      dbcc_en, dbcc_wmm);
		return MACCMP;
	}

	dst_link = dbcc_en ?
		   MAC_AX_SS_WMM_TBL_C1_WMM0 : MAC_AX_SS_WMM_TBL_C0_WMM0;
	if (dbcc_en == MAC_AX_BAND_0)
		dst_link = info->wmm ?
			   MAC_AX_SS_WMM_TBL_C0_WMM1 : MAC_AX_SS_WMM_TBL_C0_WMM0;
	else
		dst_link = info->wmm ?
			   MAC_AX_SS_WMM_TBL_C1_WMM1 : MAC_AX_SS_WMM_TBL_C1_WMM0;
	ret = mac_ss_wmm_sta_move(adapter,
				  (enum mac_ax_ss_wmm)dbcc_wmm,
				  dst_link);
	if (ret == MACARDYDONE) {
		if (wmm_type != MAC_AX_NET_TYPE_AP)
			PLTFM_MSG_WARN("dbcc en%d move wmm%d is already mapped\n",
				       dbcc_en, dbcc_wmm);
		return MACSUCCESS;
	} else if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("dbcc en%d move wmm%d sta %d\n",
			      dbcc_en, dbcc_wmm, ret);
		return ret;
	}

	PLTFM_MEMSET(&q_info, 0, sizeof(struct deq_enq_info));

	if (!dbcc_en) {
		q_info.src_pid = WDE_DLE_PID_C1;
		q_info.src_qid = WDE_DLE_QID_MG0_C1;
		q_info.dst_pid = WDE_DLE_PID_C0;
		q_info.dst_qid = WDE_DLE_QID_MG0_C0;

		ret = deq_enq_all(adapter, &q_info);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("dbcc dis move all mgq %d\n", ret);
			return ret;
		}

		q_info.src_pid = WDE_DLE_PID_C1;
		q_info.src_qid = WDE_DLE_QID_HI_C1;
		q_info.dst_pid = WDE_DLE_PID_C0;
		q_info.dst_qid = WDE_DLE_QID_HI_C0;

		ret = deq_enq_all(adapter, &q_info);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("dbcc dis move all hiq %d\n", ret);
			return ret;
		}
	}

	return MACSUCCESS;
}
#endif
