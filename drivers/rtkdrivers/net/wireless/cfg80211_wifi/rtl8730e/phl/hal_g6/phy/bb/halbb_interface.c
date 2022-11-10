/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
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
#include "halbb_precomp.h"

void halbb_cfg_timers(struct bb_info *bb, enum bb_timer_cfg_t cfg,
		      struct halbb_timer_info *timer)
{
	BB_DBG(bb, DBG_INIT, "[%s] %s timer, event=%d\n", __func__,
	       ((cfg == BB_SET_TIMER) ? "SET" : ((cfg == BB_CANCEL_TIMER) ? "CANCEL" : ("RLS"))),
	       timer->event_idx);

	if (cfg == BB_INIT_TIMER) {
		BB_WARNING("[%s]\n", __func__);
		return;
	}

	if (cfg == BB_SET_TIMER) {
		if (timer->timer_state != BB_TIMER_IDLE) {
			BB_WARNING("[%s] state=%d\n", __func__, timer->timer_state);
			return;
		}

		timer->timer_state = BB_TIMER_RUN;
		halbb_set_timer(bb, &timer->timer_list, timer->cb_time);
	} else if (cfg == BB_CANCEL_TIMER) {
		halbb_cancel_timer(bb, &timer->timer_list);
		timer->timer_state = BB_TIMER_IDLE;
	} else if (cfg == BB_RELEASE_TIMER) {
		halbb_release_timer(bb, &timer->timer_list);
		timer->timer_state = BB_TIMER_RELEASE;
	}
}

u32 halbb_get_sys_time(struct bb_info *bb)
{
	return 0;
}

u32 halbb_phy0_to_phy1_ofst(struct bb_info *bb, u32 addr)
{
	u32 ofst = 0;

#ifdef HALBB_DBCC_SUPPORT
	switch (bb->ic_type) {
#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		ofst = halbb_phy0_to_phy1_ofst_8852a_2(bb, addr);
		break;
#endif
#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		ofst = halbb_phy0_to_phy1_ofst_8852c(bb, addr);
		break;
#endif

	default:
		break;
	}
#endif
	return ofst;
}

#ifdef BB_FW_OFLD_SUPPORT
bool halbb_check_fw_ofld(struct bb_info *bb)
{
	bool ret = bb->phl_com->dev_cap.fw_cap.offload_cap & BIT0;
	BB_DBG(bb, DBG_FW_INFO, "FW ofld ret = %d\n", (u8)ret);
	return ret;
}

bool halbb_fw_set_reg(struct bb_info *bb, u32 addr, u32 mask, u32 val, u8 lc)
{
	/* halbb_set_reg */
	struct rtw_mac_cmd cmd;
	u32 ret;

	cmd.src = RTW_MAC_BB_CMD_OFLD;
	cmd.type = RTW_MAC_WRITE_OFLD;
	cmd.lc = lc;
	cmd.offset = (u16)addr;
	cmd.value = val;
	cmd.mask = mask;
	ret = rtw_hal_mac_add_cmd_ofld(bb->hal_com, &cmd);
	BB_DBG(bb, DBG_FW_INFO, "FW ofld addr:%x, val:%x, msk:%x\n", addr, val, mask);
	if (ret) {
		BB_WARNING("IO offload fail: %d\n", ret);
		return false;
	} else {
		return true;
	}

}

bool halbb_fw_set_reg_cmn(struct bb_info *bb, u32 addr,
			  u32 mask, u32 val, enum phl_phy_idx phy_idx, u8 lc)
{
	bool ret = true;
	u32 val_mod = val;

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && phy_idx == HW_PHY_1) {
		addr += halbb_phy0_to_phy1_ofst(bb, addr);
	}
#endif

	ret = halbb_fw_set_reg(bb, addr, mask, val_mod, lc);
	return ret;
}

#endif

void halbb_set_cr(struct bb_info *bb, u32 addr, u32 val)
{
	if (bb->bb_dbg_i.cr_recorder_en) {
		BB_TRACE("[W] 0x%04x = 0x%08x\n", addr, val);
	}

	halbb_set_32(bb, addr, val);
}

u32 halbb_get_cr(struct bb_info *bb, u32 addr)
{
	u32 val = halbb_get_32(bb, addr);

	if (bb->bb_dbg_i.cr_recorder_en) {
		BB_TRACE("[R] 0x%04x = 0x%08x\n", addr, val);
	}

	return val;
}

void halbb_set_reg(struct bb_info *bb, u32 addr, u32 mask, u32 val)
{
	u32 ori_val = 0;
	u32 shift;

	if (mask != MASKDWORD) {
		ori_val = halbb_get_32(bb, addr);
		shift = halbb_cal_bit_shift(mask);
		val = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}

	halbb_set_cr(bb, addr, val);
}

void halbb_set_reg_curr_phy(struct bb_info *bb, u32 addr, u32 mask, u32 val)
{
	enum phl_phy_idx phy_idx = bb->bb_phy_idx;
	u32 ori_val, shift;
	u32 val_mod = val;

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && phy_idx == HW_PHY_1) {
		addr += halbb_phy0_to_phy1_ofst(bb, addr);
	}
#endif

	if (mask != MASKDWORD) {
		shift = halbb_cal_bit_shift(mask);

		ori_val = halbb_get_32(bb, addr);
		val_mod = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}

	halbb_set_cr(bb, addr, val_mod);
}

void halbb_set_reg_cmn(struct bb_info *bb, u32 addr, u32 mask, u32 val, enum phl_phy_idx phy_idx)
{
	u32 ori_val, shift;
	u32 val_mod = val;

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && phy_idx == HW_PHY_1) {
		addr += halbb_phy0_to_phy1_ofst(bb, addr);
	}
#endif

	if (mask != MASKDWORD) {
		shift = halbb_cal_bit_shift(mask);

		ori_val = halbb_get_32(bb, addr);
		val_mod = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}

	halbb_set_cr(bb, addr, val_mod);
}

void halbb_set_reg_phy0_1(struct bb_info *bb, u32 addr, u32 mask, u32 val)
{
	u32 ori_val = 0, shift = 0;
	u32 val_mod = val;

	if (mask != MASKDWORD) {
		shift = halbb_cal_bit_shift(mask);

		ori_val = halbb_get_32(bb, addr);
		val_mod = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}

	halbb_set_cr(bb, addr, val_mod);

#ifdef HALBB_DBCC_SUPPORT
	if (!bb->hal_com->dbcc_en) {
		return;
	}

	addr += halbb_phy0_to_phy1_ofst(bb, addr);
	if (mask != MASKDWORD) {
		ori_val = halbb_get_32(bb, addr);
		val_mod = ((ori_val) & (~mask)) | (((val << shift)) & mask);
	}
	halbb_set_cr(bb, addr, val_mod);
#endif
}

u32 halbb_get_reg(struct bb_info *bb, u32 addr, u32 mask)
{
	u32 val_0 = 0;

	val_0 = (halbb_get_cr(bb, addr) & mask) >> halbb_cal_bit_shift(mask);

	return val_0;
}

u32 halbb_get_reg_curr_phy(struct bb_info *bb, u32 addr, u32 mask)
{
	enum phl_phy_idx phy_idx = bb->bb_phy_idx;
	u32 val_0 = 0;

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && phy_idx == HW_PHY_1) {
		addr += halbb_phy0_to_phy1_ofst(bb, addr);
	}
#endif

	val_0 = (halbb_get_cr(bb, addr) & mask) >> halbb_cal_bit_shift(mask);

	return val_0;
}


u32 halbb_get_reg_cmn(struct bb_info *bb, u32 addr, u32 mask, enum phl_phy_idx phy_idx)
{
	u32 val_0 = 0;

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && phy_idx == HW_PHY_1) {
		addr += halbb_phy0_to_phy1_ofst(bb, addr);
	}
#endif

	val_0 = (halbb_get_cr(bb, addr) & mask) >> halbb_cal_bit_shift(mask);

	return val_0;
}

u32 halbb_get_reg_phy0_1(struct bb_info *bb, u32 addr, u32 mask, u32 *val_1)
{
	u32 val_0 = 0;
	u32 shift = halbb_cal_bit_shift(mask);

	val_0 = (halbb_get_cr(bb, addr) & mask) >> shift;

#ifdef HALBB_DBCC_SUPPORT
	if (!bb->hal_com->dbcc_en) {
		return val_0;
	}

	addr += halbb_phy0_to_phy1_ofst(bb, addr);
	*val_1 = (halbb_get_cr(bb, addr) & mask) >> shift;
#endif

	return val_0;
}
#if (defined(BB_8720E_SUPPORT) || defined(BB_8730E_SUPPORT))
u8 halbb_trans_h2c_id(struct bb_info *bb, u8 h2c_class_id, u8 h2c_cmd_id)
{
	u8 iot_fw_h2cid = 0;

	if (h2c_class_id == HALBB_H2C_RA) {
		switch (h2c_cmd_id) {
		case RA_H2C_MACIDCFG:
			iot_fw_h2cid = DM_RA_H2C_MACIDCFG;
			break;
		case RA_H2C_RSSISETTING:
			iot_fw_h2cid = DM_RA_H2C_RSSISETTING;
			break;
		case RA_H2C_GET_TXSTS:
			iot_fw_h2cid = DM_AP_REQ_TXRPT;
			break;
		case RA_H2C_RA_ADJUST:
			iot_fw_h2cid = DM_RA_H2C_RA_ADJUST;
			break;
		case RA_H2C_ADJUST_RA_MASK:
			iot_fw_h2cid = DM_RA_H2C_ADJUST_RA_MASK;
			break;
		case RA_H2C_RA_D_O_TIMER:
			iot_fw_h2cid = DM_RA_H2C_RA_D_O_TIMER;
			break;
		default:
			iot_fw_h2cid = h2c_cmd_id;
			break;
		}
	} else if (h2c_class_id == HALBB_H2C_DM) {
		switch (h2c_cmd_id) {
		case DM_H2C_FWTRACE:
			iot_fw_h2cid = DM_IOT_H2C_FWTRACE;
			break;
		case DM_H2C_FW_EDCCA:
			iot_fw_h2cid = DM_IOT_H2C_FW_EDCCA;
			break;
		default:
			iot_fw_h2cid = h2c_cmd_id;
			break;
		}
	} else {
		BB_WARNING("Error H2C CLASS=%d, ID=%d\n", h2c_class_id, h2c_cmd_id);
	}
	return iot_fw_h2cid;
}

bool halbb_conv_h2c_content(u8 h2c_cmd_id, u32 *pconv1, u32 *pconv2, u32 *pval)
{
	if (h2c_cmd_id == DM_RA_H2C_MACIDCFG) {
		struct bb_h2c_ra_cfg_info *ra_cfg;
		struct fw_h2c_ra_cfg_info_1 *ra_cfg_1;
		struct fw_h2c_ra_cfg_info_2 *ra_cfg_2;

		ra_cfg = (struct bb_h2c_ra_cfg_info *)pval;
		ra_cfg_1 = (struct fw_h2c_ra_cfg_info_1 *)pconv1;
		ra_cfg_2 = (struct fw_h2c_ra_cfg_info_2 *)pconv2;

		ra_cfg_1->is_dis_ra = ra_cfg->is_dis_ra;
		ra_cfg_1->fix_giltf = ra_cfg->fix_giltf;
		ra_cfg_1->upd_bw_nss_mask = ra_cfg->upd_bw_nss_mask;
		ra_cfg_1->upd_mask = ra_cfg->upd_mask;
		ra_cfg_1->bw_cap = ra_cfg->bw_cap;
		ra_cfg_1->macid = ra_cfg->macid;
		ra_cfg_1->dcm_cap = ra_cfg->dcm_cap;
		ra_cfg_1->er_cap = ra_cfg->er_cap;
		ra_cfg_1->init_rate_lv = ra_cfg->init_rate_lv;
		ra_cfg_1->upd_all = ra_cfg->upd_all;
		ra_cfg_1->en_sgi = ra_cfg->en_sgi;
		ra_cfg_1->ldpc_cap = ra_cfg->ldpc_cap;
		ra_cfg_1->stbc_cap = ra_cfg->stbc_cap;
		ra_cfg_1->fixed_csi_rate_l = ra_cfg->fixed_csi_rate_l;
		ra_cfg_1->csi_rate_control = ra_cfg->csi_rate_control;
		ra_cfg_1->is_6g = ra_cfg->is_6g;
		ra_cfg_1->ra_csi_rate_en = ra_cfg->ra_csi_rate_en;
		ra_cfg_1->fixed_csi_rate_en = ra_cfg->fixed_csi_rate_en;
		ra_cfg_1->cr_tbl_sel = ra_cfg->cr_tbl_sel;
		ra_cfg_1->fix_giltf_en = ra_cfg->fix_giltf_en;
		ra_cfg_1->fix_giltf = ra_cfg_1->fix_giltf;
		ra_cfg_1->partial_bw_su_er = ra_cfg->partial_bw_su_er;
		ra_cfg_1->band_num = ra_cfg->band_num;

		ra_cfg_2->mode_ctrl = ra_cfg->mode_ctrl;
		ra_cfg_2->upd_bw_nss_mask = ra_cfg->upd_bw_nss_mask;
		ra_cfg_2->upd_mask = ra_cfg->upd_mask;
		ra_cfg_2->upd_all = ra_cfg->upd_all;
		ra_cfg_2->macid = ra_cfg->macid;
		ra_cfg_2->ramask[0] = ra_cfg->ramask[0];
		ra_cfg_2->ramask[1] = ra_cfg->ramask[1];
		ra_cfg_2->ramask[2] = ra_cfg->ramask[2];
		ra_cfg_2->ramask[3] = ra_cfg->ramask[3];

	} else if (h2c_cmd_id == DM_RA_H2C_RSSISETTING) {
		struct bb_h2c_rssi_setting *rssi_cfg;
		struct fw_h2c_rssi_setting *rssi_i;

		rssi_cfg = (struct bb_h2c_rssi_setting *)pval;
		rssi_i = (struct fw_h2c_rssi_setting *)pconv1;

		rssi_i->macid = rssi_cfg->macid;
		rssi_i->rssi_a = rssi_cfg->rssi_a;
		rssi_i->bcn_rssi_a = rssi_cfg->bcn_rssi_a;
		rssi_i->is_fixed_rate = rssi_cfg->is_fixed_rate;
		rssi_i->fixed_rate = rssi_cfg->fixed_rate;
		rssi_i->fixed_rate_md = rssi_cfg->fixed_rate_md;
		rssi_i->fixed_giltf = rssi_cfg->fixed_giltf;
		rssi_i->fixed_bw = rssi_cfg->fixed_bw;

	} else {
		BB_WARNING("[%s] Error H2C_CMD_ID!!\n", __func__);
	}

	return true;
}
#endif

bool halbb_fill_h2c_cmd(struct bb_info *bb, u16 cmdlen, u8 cmdid,
			u8 classid, u32 *pval)
{
	u32 rt_val = 0;
	bool c_val = false;
	struct rtw_g6_h2c_hdr hdr = {0};
	struct rtw_hal_com_t *hal_com = NULL;

#if (defined(BB_8720E_SUPPORT) || defined(BB_8730E_SUPPORT))
	u16 length1 = 0;
	u16 length2 = 0;
	u8 elementid = 0;
	struct fw_h2c_ra_cfg_info_1 *ra_cfg_1;
	struct fw_h2c_ra_cfg_info_2 *ra_cfg_2;
	struct fw_h2c_rssi_setting *rssi_i;

	hal_com = bb->hal_com;
	elementid = halbb_trans_h2c_id(bb, classid, cmdid);
	if (elementid == DM_RA_H2C_MACIDCFG) {
		length1 = sizeof(struct fw_h2c_ra_cfg_info_1);
		length2 = sizeof(struct fw_h2c_ra_cfg_info_2);

		ra_cfg_1 = hal_mem_alloc(bb->hal_com, length1);

		if (!ra_cfg_1) {
			BB_WARNING("[%s] Error RA_CFG_1 allocat failed!!\n", __func__);
			return false;
		}
		ra_cfg_2 = hal_mem_alloc(bb->hal_com, length2);
		if (!ra_cfg_2) {
			BB_WARNING("[%s] Error RA_CFG_2 allocat failed!!\n", __func__);
			return false;
		}

		c_val = halbb_conv_h2c_content(elementid, (u32 *)ra_cfg_1, (u32 *)ra_cfg_2, pval);
		rtw_hal_mac_send_h2c_ameba(hal_com, elementid, length1, (u8 *)ra_cfg_1);

		BB_DBG(bb, DBG_RA, "RA trans mask: %x %x %x %x\n", ra_cfg_2->mode_ctrl, ra_cfg_2->upd_bw_nss_mask,
		       ra_cfg_2->upd_mask, ra_cfg_2->ramask[0]);

		rtw_hal_mac_send_h2c_ameba(hal_com, DM_RA_H2C_MACIDCFG2, length2, (u8 *)ra_cfg_2);

		if (ra_cfg_1) {
			hal_mem_free(bb->hal_com, ra_cfg_1, length1);
		}
		if (ra_cfg_2) {
			hal_mem_free(bb->hal_com, ra_cfg_2, length2);
		}

	} else if (elementid == DM_RA_H2C_RSSISETTING) {
		length1 = sizeof(struct fw_h2c_rssi_setting);
		rssi_i = hal_mem_alloc(bb->hal_com, length1);
		if (!rssi_i) {
			BB_WARNING("[%s] Error RSSI allocat failed!!\n", __func__);
			return false;
		}

		c_val = halbb_conv_h2c_content(elementid, (u32 *)rssi_i, (u32 *)rssi_i, pval);
		length1 = sizeof(struct fw_h2c_rssi_setting);
		rt_val =  rtw_hal_mac_send_h2c_ameba(hal_com, elementid, length1, (u8 *)rssi_i);

		if (rssi_i) {
			hal_mem_free(bb->hal_com, rssi_i, length1);
		}
	} else {
		length1 = cmdlen;
		rt_val =  rtw_hal_mac_send_h2c_ameba(hal_com, elementid, length1, (u8 *)pval);
	}
#else
	hdr.h2c_class = classid;
	hdr.h2c_func = cmdid;
	hdr.content_len = cmdlen;
	hdr.type = H2CB_TYPE_LONG_DATA;
	hdr.rec_ack = 0;
	hdr.done_ack = 0;
	hal_com = bb->hal_com;
	BB_DBG(bb, DBG_FW_INFO, "H2C: %x %x %x\n", classid, cmdid, cmdlen);
	rt_val =  rtw_hal_mac_send_h2c(hal_com, &hdr, pval);
#endif
	if (rt_val != 0) {
		BB_WARNING("Error H2C CLASS=%d, ID=%d, Rt_v = %d\n", classid, cmdid, rt_val);
		return false;
	} else {
		return true;
	}
}

bool halbb_test_h2c_c2h_flow(struct bb_info *bb)
{
	u32 rt_val = 0;
	u32 *bb_h2c;
	u8 h2ctest[4] = {1, 0, 0, 0};

	bb_h2c = (u32 *) &h2ctest;
	rt_val =  halbb_fill_h2c_cmd(bb, 1, DM_H2C_FW_H2C_TEST, HALBB_H2C_DM, bb_h2c);
	if (rt_val != 0) {
		BB_WARNING("Error H2C TEST\n");
		return false;
	} else {
		return true;
	}
}

u32 halbb_c2h_rua_parsing(struct bb_info *bb, u8 cmdid, u8 len, u8 *c2h)
{
	u32 val = 0;
	return val;
}

#if !defined(BB_8730E_SUPPORT) && !defined(BB_8720E_SUPPORT) /* HALBB WARNING */
u32 halbb_c2h_mu_gptbl_rpt(struct bb_info *bb, u16 len, u8 *c2h)
{
	/* Set MU grouping table and return value */
	u32 val = 0;
	u8 i = 0;
	u8 j = 0;
	u8 k = 0;
	u8 mask = 0x03;
	struct hal_mu_score_tbl *mu_sc_tbl = &bb->hal_com->bb_mu_score_tbl;

	/*
	 Need to do MU protect to prevent error c2h sending
	 this function will be return to prevent error c2h
	*/
	return val;
	for (i = 0; i < HAL_MAX_MU_STA_NUM; i++)
		for (j = 0; j < HAL_MAX_MU_SCORE_SIZE; j++) {
			if (mask == 0x03) {
				mu_sc_tbl->mu_score[i].score[j] = (c2h[k] & mask);
				mask = mask << 2;
			} else if (mask == 0x0c) {
				mu_sc_tbl->mu_score[i].score[j] = (c2h[k] & mask) >> 2;
				mask = mask << 2;
			} else if (mask == 0x30) {
				mu_sc_tbl->mu_score[i].score[j] = (c2h[k] & mask) >> 4;
				mask = mask << 2;
			} else { /*(mask == 0xc0) */
				mu_sc_tbl->mu_score[i].score[j] = (c2h[k] & mask) >> 6;
				mask = 0x03;
				k++;
			}
		}
	return val;
}
#endif

#ifdef HALBB_DYN_L2H_SUPPORT
u32 halbb_c2h_lowrt_rty(struct bb_info *bb, u16 len, u8 *c2h)
{
	u32 c2h_rty_cnt = 0;
	struct bb_dyn_l2h_info *dyn_l2h_i = &bb->bb_dyn_l2h_i;

	c2h_rty_cnt = (*c2h) + ((*(c2h + 1)) << 8);
	dyn_l2h_i->low_rate_rty_cnt += c2h_rty_cnt;
	return 0;
}

void halbb_fw_ctrl_rtyrpt(struct bb_info *bb, u8 rpt_rtycnt, u8 en_fw_rpt)
{
	struct bb_fw_dbg_cmn_info *fwmn_i = &bb->bb_fwdbg_i;
	u32 *bb_h2c = (u32 *) fwmn_i;
	u8 cmdlen = sizeof(struct bb_fw_dbg_cmn_info);
	bool ret_val = false;

	fwmn_i->fw_cmn_info |= (en_fw_rpt & 0x01);
	fwmn_i->fw_rty_rpt_ctrl = rpt_rtycnt;
	BB_DBG(bb, DBG_FW_INFO, "FW CTRL RTYRPT: %d %d\n", fwmn_i->fw_cmn_info, fwmn_i->fw_rty_rpt_ctrl);
	BB_DBG(bb, DBG_FW_INFO, "FW CMN CTRL: %x %x\n", bb_h2c[0], bb_h2c[1]);
	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, DM_H2C_FWTRACE, HALBB_H2C_DM, bb_h2c);
}

#endif

/* Remove after 8852A B cut */
/*
u32 halbb_c2h_fw_trig_tx_rpt(struct bb_info *bb, u16 len, u8 *c2h)
{
	struct bb_c2h_fw_tx_rpt *fw_tx_i = &bb->bb_fwtx_c2h_i;
	bool tx_stat = c2h[0] & BIT(0);
	u32 val = (u32)false;

	if (tx_stat) {
		BB_DBG(bb, DBG_FW_INFO, "[FW][C2H] Tx done\n");
	} else {
		BB_DBG(bb, DBG_FW_INFO, "[FW][C2H] Tx fail\n");
	}

	// PD hit enable
	halbb_set_reg(bb, 0xa3c, BIT(9), 0);
	halbb_set_reg(bb, 0xabc, BIT(9), 0);

	fw_tx_i->tx_done = tx_stat;

	val = (u32)true;
	return val;
}*/

u32 halbb_c2h_fw_h2c_test(struct bb_info *bb, u16 len, u8 *c2h)
{
	u16 i;
	u32 val = (u32)false;

	for (i = 0; i < len; i++) {
		BB_DBG(bb, DBG_FW_INFO, "FW H2C and C2H test: %d\n", c2h[i]);
	}
	return val;
}

u32 halbb_c2h_ra_parsing(struct bb_info *bb, u8 cmdid, u16 len, u8 *c2h)
{
	u32 val = 0;
	u16 i;

	BB_DBG(bb, DBG_FW_INFO, "FW C2H RA parsing: cmdid:%d len:%d\n", cmdid, len);
	BB_DBG(bb, DBG_FW_INFO, "FW C2H RA parsing: content ==>");

	for (i = 0; i < len; i++) {
		BB_DBG(bb, DBG_FW_INFO, "%x", *(c2h + i));
	}
	BB_DBG(bb, DBG_FW_INFO, "<== \n ");
	switch (cmdid) {
	case HALBB_C2HRA_STS_RPT:
		val = halbb_get_fw_ra_rpt(bb, len, c2h);
		break;
#if !defined(BB_8730E_SUPPORT) && !defined(BB_8720E_SUPPORT) /* HALBB WARNING */
	case HALBB_C2HRA_MU_GPTBL_RPT:
		val = halbb_c2h_mu_gptbl_rpt(bb, len, c2h);
		break;
#endif
	case HALBB_C2HRA_TXSTS:
		val = halbb_get_txsts_rpt(bb, len, c2h);
		break;
	default:
		break;
	}
	return val;
}

u32 halbb_c2h_dm_parsing(struct bb_info *bb, u8 cmdid, u16 len, u8 *c2h)
{
	u32 val = 0;
	u16 i;

	BB_DBG(bb, DBG_FW_INFO, "FW C2H DM parsing: cmdid:%d len:%d\n", cmdid, len);
	BB_DBG(bb, DBG_FW_INFO, "FW C2H DM parsing: content ==>");
	for (i = 0; i < len; i++) {
		BB_DBG(bb, DBG_FW_INFO, "%x", *(c2h + i));
	}
	BB_DBG(bb, DBG_FW_INFO, "<== \n ");
	switch (cmdid) {
	case DM_C2H_FW_TEST:
		val = halbb_c2h_fw_h2c_test(bb, len, c2h);
		break;
	case DM_C2H_FW_TRIG_TX_RPT: /* Remove after 8852A B cut */
		break;
#ifdef HALBB_DYN_L2H_SUPPORT
	case DM_C2H_LOWRT_RTY:
		val = halbb_c2h_lowrt_rty(bb, len, c2h);
		break;
#endif
#ifdef HALBB_DIG_MCC_SUPPORT
	case DM_C2H_MCC_DIG:
		val = halbb_c2h_mccdm_check(bb, len, c2h);
		break;
#endif
	default:
		break;
	}
	return val;
}

u32 rtw_halbb_c2h_parsing(struct bb_info *bb, u8 classid, u8 cmdid, u16 len, u8 *c2h)
{
	u32 val = 0;
	switch (classid) {
	case HALBB_C2H_RUA:
		break;
	case HALBB_C2H_RA:
		val = halbb_c2h_ra_parsing(bb, cmdid, len, c2h);
		break;
	case HALBB_C2H_DM:
		val = halbb_c2h_dm_parsing(bb, cmdid, len, c2h);
		break;
	default:
		break;
	}
	return val;
}

enum rf_path halbb_config_path(struct bb_info *bb, enum phl_band_idx band_idx,
			       u8 rf_num)
{
	enum rf_path path_set = RF_PATH_A;

	if (bb->num_rf_path == 1) {
		path_set = RF_PATH_A;
	} else if (bb->num_rf_path == 2) {
		path_set = RF_PATH_AB;
	} else if (bb->num_rf_path == 3) {
		path_set = RF_PATH_ABC;
	} else {
		path_set = RF_PATH_ABCD;
	}
	if (rf_num == 1) {
		path_set = RF_PATH_B;
	}
#ifdef HALBB_DBCC_SUPPORT
	/* While DBCC enable, PHY0 should set path_en = 0,
	PHY1 should set path_en = 1, and PHY0 Txpath_map_a should set 0, others
	will be config by HW
	*/

	if (bb->hal_com->dbcc_en) {
		if (band_idx == HW_BAND_0) {
			path_set = RF_PATH_A;
		} else {
			path_set = RF_PATH_B;
		}
	}
#endif
	return path_set;
}

enum rtw_hal_status
halbb_config_cmac_tbl(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i,
		      struct rtw_hal_mac_ax_cctl_info *cctrl,
		      struct rtw_hal_mac_ax_cctl_info *cctl_info_mask) {
	enum rtw_hal_status ret = RTW_HAL_STATUS_FAILURE;
#if !defined(BB_8730E_SUPPORT) && !defined(BB_8720E_SUPPORT) /* HALBB WARNING don't support pathb/c/d */
	enum rf_path path_set = RF_PATH_A;
	u16 cfg;
	u16 macid = 0;
	u8 rf_num = 0;
	bool dbcc_en = false;
	enum phl_band_idx band_idx = HW_BAND_0;

	if (NULL == phl_sta_i)
	{
		return ret;
	}

	rf_num = bb->hal_com->rfpath_tx_num;
#ifdef HALBB_DBCC_SUPPORT
	band_idx = phl_sta_i->rlink->hw_band;
#endif
	/* BB related mask */
	cctl_info_mask->txpwr_mode = 0x7;
	cctl_info_mask->ntx_path_en = 0xf;
	cctl_info_mask->path_map_a = 0x3;
	cctl_info_mask->path_map_b = 0x3;
	cctl_info_mask->path_map_c = 0x3;
	cctl_info_mask->path_map_d = 0x3;
	cctl_info_mask->antsel_a = 0x1;
	cctl_info_mask->antsel_b = 0x1;
	cctl_info_mask->antsel_c = 0x1;
	cctl_info_mask->antsel_d = 0x1;
	cctl_info_mask->doppler_ctrl = 0x3;
	cctl_info_mask->txpwr_tolerence = 0xf;

	/* BB related cmac tbl */
	path_set = halbb_config_path(bb, band_idx, rf_num);
	cctrl->txpwr_mode = 0; /* txpwr ctrl */
	cfg = halbb_cfg_cmac_tx_ant(bb, path_set);
	cctrl->ntx_path_en = cfg & 0x0f;
	cctrl->path_map_a = ((cfg >> 4) & 0x03);
	cctrl->path_map_b = ((cfg >> 6) & 0x03);
	cctrl->path_map_c = ((cfg >> 8) & 0x03);
	cctrl->path_map_d = ((cfg >> 10) & 0x03);
	cctrl->antsel_a = 0x0;
	cctrl->antsel_b = 0x0;
	cctrl->antsel_c = 0x0;
	cctrl->antsel_d = 0x0;
	cctrl->doppler_ctrl = 0;
	cctrl->txpwr_tolerence = 0;
	ret = RTW_HAL_STATUS_SUCCESS;
	BB_DBG(bb, DBG_FW_INFO, "HALBB CMAC tbl config: path = %x\n", cfg);
#endif
	return ret;
}

u8 halbb_set_cmac_txpwr_mode(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	/* power by macid */
	return 0;
}

u8 halbb_set_cmac_ntx_en(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 1;
}

u8 halbb_set_cmac_path_map_a(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 0;
}

u8 halbb_set_cmac_path_map_b(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 1;
}

u8 halbb_set_cmac_path_map_c(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 2;
}

u8 halbb_set_cmac_path_map_d(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 3;
}

u8 halbb_set_cmac_antsel_a(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	/* antenna selection*/
	return 0;
}

u8 halbb_set_cmac_antsel_b(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 0;
}

u8 halbb_set_cmac_antsel_c(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 0;
}

u8 halbb_set_cmac_antsel_d(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 0;
}

u8 halbb_set_cmac_pwr_tol(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	return 0;
}

u8 halbb_set_cmac_databw_er(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	/* 0: RU242, 1:RU106*/
	return 0;
}

bool halbb_set_pwr_by_rate_tbl(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct halbb_pwr_by_rate_tbl pwr_t = {{0}};
	u8 i = 0;
	enum rtw_data_rate ru_pwr_rate[PWR_TBL_NUM] = {RTW_DATA_RATE_HE_NSS1_MCS0,
						       RTW_DATA_RATE_HE_NSS1_MCS1, RTW_DATA_RATE_HE_NSS1_MCS2,
						       RTW_DATA_RATE_HE_NSS1_MCS3, RTW_DATA_RATE_HE_NSS1_MCS4,
						       RTW_DATA_RATE_HE_NSS1_MCS5, RTW_DATA_RATE_HE_NSS1_MCS6,
						       RTW_DATA_RATE_HE_NSS1_MCS7, RTW_DATA_RATE_HE_NSS1_MCS8,
						       RTW_DATA_RATE_HE_NSS1_MCS9, RTW_DATA_RATE_HE_NSS1_MCS10,
						       RTW_DATA_RATE_HE_NSS1_MCS11, RTW_DATA_RATE_HE_NSS2_MCS0,
						       RTW_DATA_RATE_HE_NSS2_MCS1, RTW_DATA_RATE_HE_NSS2_MCS2,
						       RTW_DATA_RATE_HE_NSS2_MCS3, RTW_DATA_RATE_HE_NSS2_MCS4,
						       RTW_DATA_RATE_HE_NSS2_MCS5, RTW_DATA_RATE_HE_NSS2_MCS6,
						       RTW_DATA_RATE_HE_NSS2_MCS7, RTW_DATA_RATE_HE_NSS2_MCS8,
						       RTW_DATA_RATE_HE_NSS2_MCS9, RTW_DATA_RATE_HE_NSS2_MCS10,
						       RTW_DATA_RATE_HE_NSS2_MCS11, RTW_DATA_RATE_HE_NSS1_MCS0,
						       RTW_DATA_RATE_HE_NSS1_MCS1, RTW_DATA_RATE_HE_NSS1_MCS3,
						       RTW_DATA_RATE_HE_NSS1_MCS4, RTW_DATA_RATE_HE_NSS2_MCS0,
						       RTW_DATA_RATE_HE_NSS2_MCS1, RTW_DATA_RATE_HE_NSS2_MCS3,
						       RTW_DATA_RATE_HE_NSS2_MCS4
						      };
	u32 *pval = (u32 *)&pwr_t;
	u8 cmdlen = sizeof(pwr_t);
	u8 dcm = 0;
	enum rtw_data_rate rate;
	enum channel_width  bw = phl_sta_i->chandef.bw;
	u8 channel = phl_sta_i->chandef.center_ch;
	s16 pwr_db = 0;


	for (i = 0; i < PWR_TBL_NUM; i++) {
		rate = ru_pwr_rate[i];
		if (i >= 24) {
			dcm = 1;
		}
		/*rtw_hal_rf_read_pwr_table(bb->hal_com, 0, rate, bw, channel, 0, dcm, 0, &pwr_db);*/
		pwr_t.pwr_by_rate[i * 2] = (u8)(pwr_db & 0xff);
		pwr_t.pwr_by_rate[i * 2 + 1] = (u8)((pwr_db >> 8) & 0xff);
	}
	/* Get pwr by rate tbl from halrf */
	halbb_fill_h2c_cmd(bb, cmdlen, RUA_H2C_PWR_TBL, HALBB_H2C_RUA, pval);
	return false;
}
