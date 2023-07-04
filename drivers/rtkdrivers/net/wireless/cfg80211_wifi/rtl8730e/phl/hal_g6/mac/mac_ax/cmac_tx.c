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
#include "cmac_tx.h"
#include "mac_priv.h"

static u32 stop_macid_ctn(struct mac_ax_adapter *adapter,
			  struct mac_role_tbl *role,
			  struct mac_ax_sch_tx_en_cfg *bak);

static void sch_2_u32(struct mac_ax_adapter *adapter,
		      struct mac_ax_sch_tx_en *tx_en, u32 *val32);

static u32 tx_duty_h2c(struct mac_ax_adapter *adapter,
		       u16 pause_intvl, u16 tx_intvl);

#if 1
static void u8_2_sch(struct mac_ax_adapter *adapter,
		      struct mac_ax_sch_tx_en *tx_en, u8 val8)
{
	tx_en->be = val8 & RTW_TXEN_BE ? 1 : 0;
	tx_en->bk = val8 & RTW_TXEN_BK ? 1 : 0;
	tx_en->vi = val8 & RTW_TXEN_VI ? 1 : 0;
	tx_en->vo = val8 & RTW_TXEN_VO ? 1 : 0;
	tx_en->mgnt = val8 & RTW_TXEN_MG ? 1 : 0;
	tx_en->hiq = val8 & RTW_TXEN_HI ? 1 : 0;
	tx_en->bcnq = val8 & RTW_TXEN_BCN ? 1 : 0;
	tx_en->cpumgnt = val8 & RTW_TXEN_CPUMG ? 1 : 0;
}

static u32 stop_macid_ctn(struct mac_ax_adapter *adapter,
			  struct mac_role_tbl *role,
			  struct mac_ax_sch_tx_en_cfg *bak)
{
	struct mac_ax_sch_tx_en_cfg cfg;
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_hw_sch_tx_en(adapter, bak);
	if (ret != MACSUCCESS)
		return ret;

	cfg.band = role->info.band;
	u8_2_sch(adapter, &cfg.tx_en_mask, 0);

	u8_2_sch(adapter, &cfg.tx_en, 0);
	u8_2_sch(adapter, &cfg.tx_en_mask, 0xFF);
	cfg.tx_en_mask.mgnt = 0;
	cfg.tx_en_mask.hiq = 0;
	cfg.tx_en_mask.bcnq = 0;

	ret = set_hw_sch_tx_en(adapter, &cfg);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}


u32 set_hw_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg)
{
	u16 max_agg_num;
	u32 ret;
	u32 val32;

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	max_agg_num = cfg->max_agg_num;

	val32 = MAC_REG_R32(REG_PROT_MODE_CTRL);
	if (max_agg_num > 0 && max_agg_num <= BIT_MASK_MAX_AGG_NUM) {
		val32 = BIT_SET_MAX_AGG_NUM(val32, max_agg_num);
	} else {
		return MACSETVALERR;
	}

	if (cfg->rts_max_agg_num > 0 && cfg->rts_max_agg_num <= BIT_MASK_RTS_MAX_AGG_NUM) {
		val32 = BIT_SET_RTS_MAX_AGG_NUM(val32, cfg->rts_max_agg_num);
	} else {
		return MACSETVALERR;
	}

	MAC_REG_W32(REG_PROT_MODE_CTRL, val32);

	return MACSUCCESS;
}

u32 set_hw_fast_edca_param(struct mac_ax_adapter *adapter,
			   struct mac_ax_usr_edca_param *param)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u16 value16;
	u32 offset;

	switch (param->ac) {
	case MAC_AX_CMAC_AC_SEL_VO:
		offset = REG_FAST_EDCA_VOVI_SETTING;
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		offset = REG_FAST_EDCA_VOVI_SETTING + 2;
		break;
	case MAC_AX_CMAC_AC_SEL_BE:
		offset = REG_FAST_EDCA_BEBK_SETTING;
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		offset = REG_FAST_EDCA_BEBK_SETTING + 2;
		break;
	default:
		return MACNOITEM;
	}
	value16 = MAC_REG_R16(offset);
	value16 &= 0xFF;
	value16 = value16 | (param->queue_to << 8);
	MAC_REG_W16(offset, value16);

	return MACSUCCESS;
}

u32 set_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param)
{
	u32 val32 = 0;
	u32 reg_edca;
	u32 ret = MACNOITEM;
	u8 ecw;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ret = get_edca_addr(adapter, param, &reg_edca);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ecw = (param->ecw_max << 4) | param->ecw_min;

	switch (param->ac) {
	case MAC_AX_CMAC_AC_SEL_VO:
		val32 = (param->aifs_us << BIT_SHIFT_AIFS_VO) |
				(ecw << BIT_SHIFT_CW_VO) |
				(param->txop_32us << BIT_SHIFT_TXOPLIMIT_VO);
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		val32 = (param->aifs_us << BIT_SHIFT_AIFS_VI) |
			(ecw << BIT_SHIFT_CW_VI) |
			(param->txop_32us << BIT_SHIFT_TXOPLIMIT_VI);
		break;
	case MAC_AX_CMAC_AC_SEL_BE:
		val32 = (param->aifs_us << BIT_SHIFT_AIFS_BE) |
				(ecw << BIT_SHIFT_CW_BE) |
				(param->txop_32us << BIT_SHIFT_TXOPLIMIT_BE);
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		val32 = (param->aifs_us << BIT_SHIFT_AIFS_BK) |
				(ecw << BIT_SHIFT_CW_BK) |
				(param->txop_32us << BIT_SHIFT_TXOPLIMIT_BK);
		break;
	default:
		return MACNOITEM;
	}

	MAC_REG_W32(reg_edca, val32);


	return MACSUCCESS;
}

u32 get_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param)
{
	u32 val32;
	u32 reg_edca;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ret = get_edca_addr(adapter, param, &reg_edca);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(reg_edca);
	param->aifs_us = BIT_GET_AIFS_VO(val32);
	param->ecw_max = (BIT_GET_CW_VO(val32) >> 4) & 0xF;
	param->ecw_min = BIT_GET_CW_VO(val32) & 0xF;
	param->txop_32us = BIT_GET_TXOPLIMIT_VO(val32);

	return MACSUCCESS;
}

u32 set_hw_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param)
{
	u32 val32;
	u32 reg_edca;
	u32 ret;
	u8 ecw;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ret = get_muedca_param_addr(adapter, param, &reg_edca);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(reg_edca);
	ecw = (param->ecw_max << 4) | param->ecw_min;
	switch (param->ac) {
	case MAC_AX_CMAC_AC_SEL_VO:
		val32 = BIT_SET_MUEDCA_VO_PARAM_AIFS(val32, param->aifs_us);
		val32 = BIT_SET_MUEDCA_VO_PARAM_CW(val32, ecw);
		val32 = BIT_SET_MUEDCA_VO_PARAM_TIMER(val32, param->muedca_timer_32us);
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		val32 = BIT_SET_MUEDCA_VI_PARAM_AIFS(val32, param->aifs_us);
		val32 = BIT_SET_MUEDCA_VI_PARAM_CW(val32, ecw);
		val32 = BIT_SET_MUEDCA_VI_PARAM_TIMER(val32, param->muedca_timer_32us);
		break;
	case MAC_AX_CMAC_AC_SEL_BE:
		val32 = BIT_SET_MUEDCA_BE_PARAM_AIFS(val32, param->aifs_us);
		val32 = BIT_SET_MUEDCA_BE_PARAM_CW(val32, ecw);
		val32 = BIT_SET_MUEDCA_BE_PARAM_TIMER(val32, param->muedca_timer_32us);
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		val32 = BIT_SET_MUEDCA_BK_PARAM_AIFS(val32, param->aifs_us);
		val32 = BIT_SET_MUEDCA_BK_PARAM_CW(val32, ecw);
		val32 = BIT_SET_MUEDCA_BK_PARAM_TIMER(val32, param->muedca_timer_32us);
		break;
	default:
		return MACNOITEM;
	}

	MAC_REG_W32(reg_edca, val32);

	return MACSUCCESS;
}

u32 set_hw_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg)
{
	u32 ret;
	u16 val16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val16 = MAC_REG_R16(REG_AX_TB_MUEDCA_CTRL);

	if (cfg->countdown_en) {
		val16 |= BIT_MUEDCA_EN;
	} else {
		val16 &= ~BIT_MUEDCA_EN;
	}

	if (cfg->tb_update_en) {
		val16 |= BIT_SET_MUEDCATIMER_TF;
	} else {
		val16 &= ~BIT_SET_MUEDCATIMER_TF;
	}

	MAC_REG_W16(REG_AX_TB_MUEDCA_CTRL, val16);

	return MACSUCCESS;
}

u32 set_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg)
{
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 tx_en_u32 = 0, mask_u32 = 0, val32;
	struct mac_ax_sch_tx_en tx_en;
	struct mac_ax_sch_tx_en tx_en_mask;
	u8 chip_id = adapter->hw_info->chip_id;

	tx_en = cfg->tx_en;
	tx_en_mask = cfg->tx_en_mask;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	sch_2_u32(adapter, &tx_en, &tx_en_u32);
	sch_2_u32(adapter, &tx_en_mask, &mask_u32);

	val32 = MAC_REG_R32(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL);
	val32 &= 0xFF00FFFF;
	/* tx en is "enable tx, so add ~ for tx pause " */
	val32 |= (~(tx_en_u32 & mask_u32) & 0x00FF0000);
	MAC_REG_W32(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL, val32);

	return MACSUCCESS;
}

u32 get_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg)
{
	u32 ret, val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_sch_tx_en tx_en;
	u8 chip_id = adapter->hw_info->chip_id;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL);
	u32_2_sch(adapter, &tx_en, val32);
	cfg->tx_en = tx_en;

	return MACSUCCESS;
}

u32 set_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg)
{
	u32 ret;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_BNDY_LIFETIME_TAIL);
	val32 &= 0xFFF0FFFF;
	val32 |= (cfg->en.voq_en ? BIT_LIFETIME_VO_EN : 0) |
		 (cfg->en.viq_en ? BIT_LIFETIME_VI_EN : 0) |
		 (cfg->en.beq_en ? BIT_LIFETIME_BE_EN : 0) |
		 (cfg->en.bkq_en ? BIT_LIFETIME_BK_EN : 0);
	MAC_REG_W32(REG_BNDY_LIFETIME_TAIL, val32);

	val32 = (cfg->val.bebk_val << 16) | cfg->val.vovi_val;
	MAC_REG_W32(REG_PKT_LIFE_TIME, val32);

	return MACSUCCESS;
}

u32 get_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg)
{
	u32 ret;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_BNDY_LIFETIME_TAIL);
	cfg->en.voq_en = !!(val32 & BIT_LIFETIME_VO_EN);
	cfg->en.viq_en = !!(val32 & BIT_LIFETIME_VI_EN);
	cfg->en.beq_en = !!(val32 & BIT_LIFETIME_BE_EN);
	cfg->en.bkq_en = !!(val32 & BIT_LIFETIME_BK_EN);

	val32 = MAC_REG_R32(REG_PKT_LIFE_TIME);
	cfg->val.vovi_val = BIT_GET_PKT_LIFTIME_VOVI(val32);
	cfg->val.bebk_val = BIT_GET_PKT_LIFTIME_BEBK(val32);

	return MACSUCCESS;
}

u32 resume_sch_tx(struct mac_ax_adapter *adapter,
		  struct mac_ax_sch_tx_en_cfg *bak)
{
	u32 ret;

	u32_2_sch(adapter, &bak->tx_en_mask, 0xFFFFFFFF);
	ret = set_hw_sch_tx_en(adapter, bak);
	if (ret != MACSUCCESS) {
		return ret;
	}

	return MACSUCCESS;
}

u32 get_edca_addr(struct mac_ax_adapter *adapter,
		  struct mac_ax_edca_param *param, u32 *reg_edca)
{
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}


	switch (param->ac) {
	case MAC_AX_CMAC_AC_SEL_VO:
		*reg_edca = REG_EDCA_VO_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		*reg_edca = REG_EDCA_VI_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_BE:
		*reg_edca = REG_EDCA_BE_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		*reg_edca = REG_EDCA_BK_PARAM;
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 get_muedca_param_addr(struct mac_ax_adapter *adapter,
			  struct mac_ax_muedca_param *param,
			  u32 *reg_edca)
{
	u32 ret;
	enum mac_ax_cmac_ac_sel ac;

	ac = param->ac;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	switch (ac) {
	case MAC_AX_CMAC_AC_SEL_BE:
		*reg_edca = REG_AX_MUEDCA_BE_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		*reg_edca = REG_AX_MUEDCA_BK_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		*reg_edca = REG_AX_MUEDCA_VI_PARAM;
		break;
	case MAC_AX_CMAC_AC_SEL_VO:
		*reg_edca = REG_AX_MUEDCA_VO_PARAM;
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

static void sch_2_u32(struct mac_ax_adapter *adapter,
		      struct mac_ax_sch_tx_en *tx_en, u32 *val32)
{
	*val32 = ((tx_en->be ? BIT_MAC_STOPBE : 0) |
		  (tx_en->bk ? BIT_MAC_STOPBK : 0) |
		  (tx_en->vi ? BIT_MAC_STOPVI : 0) |
		  (tx_en->vo ? BIT_MAC_STOPVO : 0) |
		  (tx_en->mgnt ? BIT_MAC_STOPMGQ : 0) |
		  (tx_en->hiq ? BIT_MAC_STOPHIQ : 0) |
		  (tx_en->bcnq ? BIT_MAC_STOPBCNQ : 0) |
		  (tx_en->cpumgnt ? BIT_STOP_CPUMGQ : 0));
}

void u32_2_sch(struct mac_ax_adapter *adapter,
	       struct mac_ax_sch_tx_en *tx_en, u32 val32)
{
	tx_en->be = val32 & BIT_MAC_STOPBE ? 1 : 0;
	tx_en->bk = val32 & BIT_MAC_STOPBK ? 1 : 0;
	tx_en->vi = val32 & BIT_MAC_STOPVI ? 1 : 0;
	tx_en->vo = val32 & BIT_MAC_STOPVO ? 1 : 0;
	tx_en->mgnt = val32 & BIT_MAC_STOPMGQ ? 1 : 0;
	tx_en->hiq = val32 & BIT_MAC_STOPHIQ ? 1 : 0;
	tx_en->bcnq = val32 & BIT_MAC_STOPBCNQ ? 1 : 0;
	tx_en->cpumgnt = val32 & BIT_STOP_CPUMGQ ? 1 : 0;
}

u32 mac_set_hw_rts_th(struct mac_ax_adapter *adapter,
		      struct mac_ax_hw_rts_th *th)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;
	u16 val32;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_PROT_MODE_CTRL);
	val32 = BIT_SET_RTS_LEN_TH(val32, th->len_th);
	val32 = BIT_SET_RTS_TXTIME_TH(val32, th->time_th);
	MAC_REG_W32(REG_PROT_MODE_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_get_hw_rts_th(struct mac_ax_adapter *adapter,
		      struct mac_ax_hw_rts_th *th)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;
	u16 val;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val = MAC_REG_R16(REG_PROT_MODE_CTRL);

	th->len_th = BIT_GET_RTS_LEN_TH(val);
	th->time_th = BIT_GET_RTS_TXTIME_TH(val);

	return MACSUCCESS;
}
#endif
u32 mac_tx_duty(struct mac_ax_adapter *adapter,
		u16 pause_intvl, u16 tx_intvl)
{
	u32 ret;

	if (!(pause_intvl) || !(tx_intvl)) {
		return MACFUNCINPUT;
	}

	ret = tx_duty_h2c(adapter, pause_intvl, tx_intvl);
	if (ret != MACSUCCESS) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_tx_duty_stop(struct mac_ax_adapter *adapter)
{
	u32 ret;

	ret = tx_duty_h2c(adapter, 0, 0);
	if (ret != MACSUCCESS) {
		return ret;
	}

	return MACSUCCESS;
}

u32 tx_duty_h2c(struct mac_ax_adapter *adapter,
		u16 pause_intvl, u16 tx_intvl)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 stop_macid_tx(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		  enum tb_stop_sel stop_sel, struct macid_tx_bak *bak)
{
	u8 band;
	u32 ret;
	struct mac_ax_macid_pause_cfg pause;

	band = role->info.band;

	pause.macid = role->macid;
	pause.pause = 1;
	ret = set_macid_pause(adapter, &pause);
	if (ret != MACSUCCESS)
		return ret;

	bak->sch_bak.band = band;
	ret = stop_macid_ctn(adapter, role, &bak->sch_bak);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

u32 resume_macid_tx(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		    struct macid_tx_bak *bak)
{
	u32 ret;
	struct mac_ax_macid_pause_cfg pause_cfg;

	if (role->info.band == MAC_AX_BAND_0) {
		u8_2_sch(adapter, &bak->sch_bak.tx_en_mask, 0xFF);
		ret = set_hw_sch_tx_en(adapter, &bak->sch_bak);
		if (ret != MACSUCCESS)
			return ret;
	}

	pause_cfg.macid = role->macid;
	pause_cfg.pause = 0;
	ret = set_macid_pause(adapter, &pause_cfg);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}