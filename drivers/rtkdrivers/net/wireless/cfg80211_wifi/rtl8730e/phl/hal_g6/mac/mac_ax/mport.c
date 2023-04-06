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

#include "mport.h"
#include "../mac_ax.h"

#define MAX_TSF_SOURCE_PORT 7
#define MAX_TSF_TARGET_PORT 4
#define MAX_TSF_SYNC_OFFSET 0x3FFFF

static u32 cfg_regl[MAC_AX_PORT_NUM] = {
	REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1, REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1,
	REG_NAN_SETTING
};

static u32 cfg_reg2[MAC_AX_PORT_NUM] = {
	REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1, REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1,
	REG_DISATIM_BCNERLY_TBTTHOLD_BCNCS
};


static u32 bcnspc_regl[MAC_AX_PORT_NUM] = {
	REG_PORT0_BCN_SPACE, REG_BCN_SPACE_PORT1_2, REG_BCN_SPACE_PORT1_2
};

static u32 phb_regl[MAC_AX_PORT_NUM] = {
	REG_NOA0_TBTT_PROHIBIT, REG_NOA0_TBTT_PROHIBIT, REG_NAN_SETTING
};

/*
static u32 bcnpsr_regl[MAC_AX_BAND_NUM][MAC_AX_PORT_NUM] = {
	{R_AX_BCN_PSR_RPT_P0, R_AX_BCN_PSR_RPT_P1, R_AX_BCN_PSR_RPT_P2,
	 R_AX_BCN_PSR_RPT_P3, R_AX_BCN_PSR_RPT_P4},
	{R_AX_BCN_PSR_RPT_P0_C1, R_AX_BCN_PSR_RPT_P1_C1, R_AX_BCN_PSR_RPT_P2_C1,
	 R_AX_BCN_PSR_RPT_P3_C1, R_AX_BCN_PSR_RPT_P4_C1}
};

static u32 cnt_tmr_regl[MAC_AX_BAND_NUM][MAC_AX_PORT_NUM] = {
	{R_AX_BCN_CNT_TMR_P0, R_AX_BCN_CNT_TMR_P1, R_AX_BCN_CNT_TMR_P2,
	 R_AX_BCN_CNT_TMR_P3, R_AX_BCN_CNT_TMR_P4},
	{R_AX_BCN_CNT_TMR_P0_C1, R_AX_BCN_CNT_TMR_P1_C1, R_AX_BCN_CNT_TMR_P2_C1,
	 R_AX_BCN_CNT_TMR_P3_C1, R_AX_BCN_CNT_TMR_P4_C1}
};
*/
static u32 hiq_win_mbid_regl[MAC_AX_P0_MBID_MAX] = {
	REG_ATIMWND0_P0_BCN_ERLY_DMA_CTRL + 2, REG_BCN_ERLY_CTW_CTRL,
	REG_ATIMWND2_5, REG_ATIMWND2_5 + 1, REG_ATIMWND2_5 + 2,
	REG_ATIMWND2_5 + 3, REG_ATIMWND6_7_URGENT_NOLIMTHGQ,
	REG_ATIMWND6_7_URGENT_NOLIMTHGQ + 1
};

static u32 dtim_prd_port_regl[MAC_AX_PORT_NUM - 1] = {
	REG_DTIM_COUNT_0_3, REG_DTIM_COUNT_0_3 + 1
};

static u32 dtim_prd_mbid_regl[MAC_AX_P0_MBID_MAX] = {
	REG_DTIM_COUNT_0_3, REG_DTIM_COUNT_0_3 + 1, REG_DTIM_COUNT_0_3 + 2,
	REG_DTIM_COUNT_0_3 + 3, REG_DTIM_COUNT_4_7, REG_DTIM_COUNT_4_7 + 1,
	REG_DTIM_COUNT_4_7 + 2, REG_DTIM_COUNT_4_7 + 3
};


static u32 tsfl_regl[MAC_AX_PORT_NUM] = {
	REG_P0_TSFTR_L, REG_P1_TSFTR_L, REG_P2_TSFTR_L
};

static u32 bss_color_regl[MAC_AX_PORT_NUM - 1] = {
	REG_TRAN_BSSID0_H,
	REG_TRAN_BSSID1_H
};

static u32 b_en_l[MAC_AX_PORT_NUM] = {
	BIT_P0_EN_BCN_FUNCTION, BIT_P1_EN_BCN_FUNCTION, BIT_P2_EN_BCN_FUNCTION
};

static u32 b_txbcnrpt_l[MAC_AX_PORT_NUM] = {
	BIT_P0_EN_BCN_RPT, BIT_P1_EN_BCN_RPT, BIT_P2_EN_BCN_RPT
};

static u32 b_rxbcnrpt_l[MAC_AX_PORT_NUM] = {
	BIT_P0_RXBCN_RPT_ENABLE, BIT_P1_RXBCN_RPT_ENABLE, BIT_P2_RXBCN_RPT_ENABLE
};

static u32 b_rxupd_l[MAC_AX_PORT_NUM] = {
	BIT_P0_DIS_RX_BSSID_FIT, BIT_P1_DIS_RX_BSSID_FIT,
	BIT_P2_DIS_RX_BSSID_FIT
};

static u32 b_rxtsfupd_l[MAC_AX_PORT_NUM] = {
	BIT_P0_DIS_TSF_UDT, BIT_P1_DIS_TSF_UDT, BIT_P2_DIS_TSF_UDT
};
/*
static u32 b_bcntxen_l[MAC_AX_PORT_NUM] = {
	B_AX_BCNTX_EN_P0, B_AX_BCNTX_EN_P1, B_AX_BCNTX_EN_P2,
	B_AX_BCNTX_EN_P3, B_AX_BCNTX_EN_P4
};

static u32 b_shten_l[MAC_AX_PORT_NUM] = {
	B_AX_TBTT_UPD_SHIFT_SEL_P0, B_AX_TBTT_UPD_SHIFT_SEL_P1,
	B_AX_TBTT_UPD_SHIFT_SEL_P2, B_AX_TBTT_UPD_SHIFT_SEL_P3,
	B_AX_TBTT_UPD_SHIFT_SEL_P4
};

static u32 b_brken_l[MAC_AX_PORT_NUM] = {
	B_AX_BRK_SETUP_P0, B_AX_BRK_SETUP_P1, B_AX_BRK_SETUP_P2,
	B_AX_BRK_SETUP_P3, B_AX_BRK_SETUP_P4
};
*/
static u32 b_rsttsf_l[MAC_AX_PORT_NUM] = {
	BIT_TSFTR_RST, BIT_TSFTR1_RST, BIT_TSFTR2_RST
};

/*
static u32 b_rptvld_l[MAC_AX_PORT_NUM] = {
	B_AX_RPT_VALID_P0, B_AX_RPT_VALID_P1, B_AX_RPT_VALID_P2,
	B_AX_RPT_VALID_P3, B_AX_RPT_VALID_P4
};

static u32 b_timerr_l[MAC_AX_PORT_NUM] = {
	B_AX_TIM_ILEGAL_P0, B_AX_TIM_ILEGAL_P1, B_AX_TIM_ILEGAL_P2,
	B_AX_TIM_ILEGAL_P3, B_AX_TIM_ILEGAL_P4
};
*/
static u32 b_mbid_en_l[MAC_AX_P0_MBID_MAX] = {
	BIT_MBID_BCNQ0_EN, BIT_MBID_BCNQ1_EN, BIT_MBID_BCNQ2_EN, BIT_MBID_BCNQ3_EN,
	BIT_MBID_BCNQ4_EN, BIT_MBID_BCNQ5_EN, BIT_MBID_BCNQ6_EN, BIT_MBID_BCNQ7_EN
};

static u32 bcn_early_regl[MAC_AX_PORT_NUM] = {
	REG_ATIMWND0_P0_BCN_ERLY_DMA_CTRL, REG_NAN_SETTING,
	REG_TXPAUSE_CFG
};

static u32 b_hiq_nolmt_mbid_l[MAC_AX_P0_MBID_MAX] = {
	BIT_HIQ_NO_LMT_EN_ROOT, BIT_HIQ_NO_LMT_EN_VAP1,
	BIT_HIQ_NO_LMT_EN_VAP2, BIT_HIQ_NO_LMT_EN_VAP3,
	BIT_HIQ_NO_LMT_EN_VAP4, BIT_HIQ_NO_LMT_EN_VAP5,
	BIT_HIQ_NO_LMT_EN_VAP6, BIT_HIQ_NO_LMT_EN_VAP7
};
/*
static u32 b_hiq_nolmt_port_l[MAC_AX_PORT_NUM] = {
	B_AX_HIQ_NO_LMT_EN_P0_ROOT, B_AX_HIQ_NO_LMT_EN_P1,
	B_AX_HIQ_NO_LMT_EN_P2, B_AX_HIQ_NO_LMT_EN_P3, B_AX_HIQ_NO_LMT_EN_P4
};
*/
static u32 mbid_idx_l[MAC_AX_P0_MBID_MAX] = {
	MAC_AX_P0_ROOT, MAC_AX_P0_MBID1, MAC_AX_P0_MBID2, MAC_AX_P0_MBID3,
	MAC_AX_P0_MBID4, MAC_AX_P0_MBID5, MAC_AX_P0_MBID6, MAC_AX_P0_MBID7,
};

static u32 t32_togl_rpt_size =
	sizeof(struct mac_ax_t32_togl_rpt) * MAC_AX_PORT_NUM;
static u32 port_info_size =
	sizeof(struct mac_ax_port_info) * MAC_AX_PORT_NUM;
#if 1
static u32 _get_max_mbid(struct mac_ax_adapter *adapter, u8 *mbid_max)
{
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8730E)) {
		*mbid_max = MAC_AX_P0_MBID7;
	} else {
		return MACCHIPID;
	}

	return MACSUCCESS;
}

u32 get_bp_idx(u8 port)
{
	//return (band * MAC_AX_BAND_NUM + port);
	return port;
}

u32 _get_port_cfg(struct mac_ax_adapter *adapter,
		  enum mac_ax_port_cfg_type type,
		  struct mac_ax_port_cfg_para *para)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 mbssid_idx = para->mbssid_idx;
	u8 port = para->port;
	u8 band = para->band;
	u32 val32;
	u8 mbid_max;
	u32 ret;

	if (port >= MAC_AX_PORT_NUM) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	if (mbssid_idx) {
		ret = _get_max_mbid(adapter, &mbid_max);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] get_max_mbid %d\n", ret);
			return MACPORTCFGPORT;
		}

		if (mbssid_idx > (u32)mbid_max) {
			PLTFM_MSG_ERR("[ERR] invalid mbssid %d\n", mbssid_idx);
			return MACFUNCINPUT;
		}
	}

	switch (type) {
	case MAC_AX_PCFG_NET_TYPE:
		val32 = MAC_REG_R32(REG_CR);
		switch (port) {
		case MAC_AX_PORT_0:
			para->val = BIT_GET_NETYPE0(val32);
			break;
		case MAC_AX_PORT_1:
			para->val = BIT_GET_NETYPE1(val32);
			break;
		}
		break;

	case MAC_AX_PCFG_FUNC_SW:
		val32 = MAC_REG_R32(cfg_regl[port]);
		if (val32 & b_en_l[port]) {
			para->val = 1;
		} else {
			para->val = 0;
		}
		break;

	case MAC_AX_PCFG_BCN_INTV:
		val32 = MAC_REG_R32(bcnspc_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
			para->val = mbssid_idx ?
				    BIT_GET_BCN_SUB_SPACE_PORT0(val32) :
				    BIT_GET_BCN_SPACE_PORT0(val32);
			break;
		case MAC_AX_PORT_1:
			para->val = BIT_GET_BCN_SPACE3(val32);
			break;
		case MAC_AX_PORT_2:
			para->val = BIT_GET_BCN_SPACE4(val32);
			break;
		}
		break;

	case MAC_AX_PCFG_BCN_HOLD_TIME:
		val32 = MAC_REG_R32(phb_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
		case MAC_AX_PORT_1:
			para->val = BIT_GET_TBTT_HOLD_TIME(val32);
			break;
		case MAC_AX_PORT_2:
			para->val = BIT_GET_TBTT_PROHIBIT_HOLD_P2(val32);
			break;
		}
		break;

	case MAC_AX_PCFG_BCN_ERLY:
		val32 = MAC_REG_R32(bcn_early_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
			para->val = BIT_GET_DRVERLYITV_P0(val32);
			break;
		case MAC_AX_PORT_1:
			para->val = BIT_GET_DRVERLYITV_P1(val32);
			break;
		case MAC_AX_PORT_2:
			para->val = BIT_GET_DRVERLYITV_P2(val32);
			break;
		}
		break;

	case MAC_AX_PCFG_BCN_SETUP_TIME:
		val32 = MAC_REG_R32(REG_NOA0_TBTT_PROHIBIT);
		para->val = BIT_GET_TBTT_PROHIBIT_SETUP(val32);
		break;

	default:
		PLTFM_MSG_ERR("[ERR] invalid get cfg type %d\n", type);
		return MACPORTCFGTYPE;
	}

	return MACSUCCESS;
}

static u32 _port_cfg(struct mac_ax_adapter *adapter,
		     enum mac_ax_port_cfg_type type,
		     struct mac_ax_port_cfg_para *para)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 set_val = para->val;
	u8 port = para->port;
	u32 val32;
	u32 w_val32 = MAC_AX_R32_DEAD;

	if (port >= MAC_AX_PORT_NUM) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	switch (type) {
	case MAC_AX_PCFG_NET_TYPE:
		val32 = MAC_REG_R32(REG_CR);
		switch (port) {
		case MAC_AX_PORT_0:
			w_val32 = BIT_SET_NETYPE0(val32, set_val);
			break;
		case MAC_AX_PORT_1:
			w_val32 = BIT_SET_NETYPE1(val32, set_val);
			break;
		}
		if (w_val32 != val32) {
			MAC_REG_W32(REG_CR, w_val32);
		}
		break;

	default:
		PLTFM_MSG_ERR("[ERR] invalid cfg type %d\n", type);
		return MACPORTCFGTYPE;
	}

	return MACSUCCESS;
}

static u32 _bcn_setup_chk(struct mac_ax_adapter *adapter, u8 port,
			  u32 *set_val)
{
	struct mac_ax_port_cfg_para cfg_para;
	struct mac_ax_port_info *pinfo_self, *pinfo_tmp;
	u32 ret;
	u32 up_lmt = 0xF;
	u8 port_idx;

	pinfo_self = &adapter->port_info[get_bp_idx(port)];

	if (port == MAC_AX_PORT_0 && pinfo_self->stat == PORT_ST_DIS) {
		return MACSUCCESS;
	}

	if (*set_val > BIT_MASK_TBTT_PROHIBIT_SETUP) {
		PLTFM_MSG_ERR("[ERR] illegal bcn setup time %d\n", *set_val);
		return MACFUNCINPUT;
	}

	for (port_idx = MAC_AX_PORT_0; port_idx < MAC_AX_PORT_NUM; port_idx++) {
		pinfo_tmp = &adapter->port_info[get_bp_idx(port_idx)];
		if (pinfo_tmp->stat == PORT_ST_DIS) {
			continue;
		}
		if (port != MAC_AX_PORT_0 && port_idx == MAC_AX_PORT_0) {
			continue;
		}

		cfg_para.port = port_idx;
		cfg_para.mbssid_idx = 0;
		cfg_para.val = 0;
		ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_ERLY, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]get port cfg bcn erly fail %d\n",
				      ret);
			return ret;
		}

		if (*set_val >= up_lmt) {
			PLTFM_MSG_WARN("[WARN]setup time %d over hw limit\n", *set_val);
			*set_val = up_lmt - 1;
			return MACSETVALERR;
		}
	}

	return MACSUCCESS;
}
#endif

#if 1
static u32 _bcn_erly_chk(struct mac_ax_adapter *adapter, u8 port,
			 u32 *set_val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_port_cfg_para cfg_para;
	struct mac_ax_port_info *pinfo;
	u32 ret, bcn_hold, bcn_spc, val32;

	pinfo = &adapter->port_info[get_bp_idx(port)];

	if (pinfo->stat == PORT_ST_DIS) {
		return MACSUCCESS;
	}

	if (*set_val > BIT_MASK_DRVERLYITV_P0 || !*set_val) {
		PLTFM_MSG_ERR("[ERR] illegal bcn erly %d\n", *set_val);
		return MACFUNCINPUT;
	}

	cfg_para.port = port;
	cfg_para.mbssid_idx = 0;
	cfg_para.val = 0;
	ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_SETUP_TIME, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]get port cfg bcn setup fail %d\n", ret);
		return ret;
	}
	if (*set_val * TU_TO_BCN_SET <= cfg_para.val) {
		PLTFM_MSG_WARN("[WARN]bcn erly %d below hw limit\n", *set_val);
		*set_val = cfg_para.val / TU_TO_BCN_SET + 1;
		return MACSETVALERR;
	}

	cfg_para.port = port;
	if (port == MAC_AX_PORT_0 &&
	    ((MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1) >> 16) & 0x7)) {
		cfg_para.mbssid_idx = 1;
	} else {
		cfg_para.mbssid_idx = 0;
	}
	cfg_para.val = 0;
	ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_INTV, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]get port cfg bcn intv fail %d\n", ret);
		return ret;
	}
	bcn_spc = cfg_para.val;

	cfg_para.port = port;
	cfg_para.mbssid_idx = 0;
	cfg_para.val = 0;
	ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_HOLD_TIME, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] get port cfg bcn hold fail %d\n", ret);
		return ret;
	}
	bcn_hold = cfg_para.val;

	val32 = bcn_spc * TU_TO_BCN_SET - bcn_hold;
	if (*set_val >= val32) {
		PLTFM_MSG_WARN("[WARN]bcn erly %d over hw limit\n", *set_val);
		*set_val = val32 - 1;
		return MACSETVALERR;
	}

	return MACSUCCESS;
}

static u32 _set_max_mbid_num(struct mac_ax_adapter *adapter,
			     struct mac_ax_port_cfg_para *para)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_port_cfg_para cfg_para;
	u8 port = para->port;
	u32 mbid_num = para->mbssid_idx;
	u32 ret;
	u32 bcn_erly;
	u32 hold_time;
	u32 subspc;
	u32 val32, w_val32;


	cfg_para.port = port;
	cfg_para.mbssid_idx = 0;

	ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_ERLY, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d get cfg bcn_erly fail %d\n",
			      port, ret);
		return ret;
	}
	bcn_erly = cfg_para.val;

	ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_HOLD_TIME, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d get cfg hold_time fail %d\n",
			      port, ret);
		return ret;
	}
	hold_time = cfg_para.val;

	subspc = para->val / (mbid_num + 1);
	if (bcn_erly + hold_time > subspc) {
		PLTFM_MSG_ERR("[ERR] BcnSubspc not enough for erly and hold time\n");
		return MACSUBSPCERR;
	}

	//set interval
	cfg_para.val = para->val;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_INTV, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg bcn intv fail %d\n",
			      port, ret);
		return ret;
	}

	//set mbssid num
	val32 = MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1);
	w_val32 = BIT_SET_MBID_BCN_NUM(val32, mbid_num);
	if (val32 != w_val32) {
		MAC_REG_W32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1, w_val32);
	}

	//set subspace
	val32 = MAC_REG_R32(bcnspc_regl[MAC_AX_PORT_0]);
	if (mbid_num) {
		w_val32 = BIT_SET_BCN_SUB_SPACE_PORT0(val32, subspc);
	} else {
		w_val32 = BIT_SET_BCN_SUB_SPACE_PORT0(val32, 0);
	}
	if (w_val32 != val32) {
		MAC_REG_W32(bcnspc_regl[MAC_AX_PORT_0], w_val32);
	}

	return MACSUCCESS;
}

static u32 port0_mbid_set(struct mac_ax_adapter *adapter, u8 mbid_num,
			  u8 mbid_max)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, i;

	val32 = MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1);
	val32 = BIT_SET_MBID_BCN_NUM(val32, mbid_num);
	MAC_REG_W32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1, val32);

	val32 = MAC_REG_R32(REG_NOA0_MBSSID_RD_CTRL);

	for (i = 0; i < mbid_max; i++) {
		if (i > (MAC_AX_P0_MBID_MAX - 1)) {
			PLTFM_MSG_ERR("mbid en idx %d over array %d\n", i,
				      MAC_AX_P0_MBID_MAX - 1);
			return MACCMP;
		}
		val32 &= ~b_mbid_en_l[i];
	}

	MAC_REG_W8(REG_NOA0_MBSSID_RD_CTRL, val32);

	return MACSUCCESS;
}
#endif
static u32 port0_subspc_set(struct mac_ax_adapter *adapter,
			    u8 mbid_num, u32 *bcn_erly, u32 *hold_time)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 subspc_u32;
	u32 val32, w_val32;
	u32 ret = MACSUCCESS;

	val32 = MAC_REG_R32(bcnspc_regl[MAC_AX_PORT_0]);
	subspc_u32 = mbid_num ?
		     (BIT_GET_BCN_SPACE_PORT0(val32) / (mbid_num + 1)) : 0;
	if (subspc_u32 > BIT_MASK_BCN_SUB_SPACE_PORT0) {
		PLTFM_MSG_ERR("[ERR] sub space %d overflow\n", subspc_u32);
		return MACSUBSPCERR;
	}

	w_val32 = BIT_SET_BCN_SUB_SPACE_PORT0(val32, subspc_u32);
	if (w_val32 != val32) {
		MAC_REG_W32(bcnspc_regl[MAC_AX_PORT_0], w_val32);
	}

	/*subspc_u32 *= TU_TO_BCN_SET;
	if (mbid_num && (BCN_HOLD_DEF + BCN_ERLY_DEF) > subspc_u32) {
		*bcn_erly = subspc_u32 * BCN_ERLY_RATIO / BCN_ITVL_RATIO;
		*hold_time = subspc_u32 * BCN_HOLD_RATIO / BCN_ITVL_RATIO;
	}*/

	return ret;
}

u32 dly_port_tu(struct mac_ax_adapter *adapter, u8 port, u32 dly_tu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 cnt;
	u32 ori_tsf;
	u32 dly_us = dly_tu * TU_UNIT;

	cnt = dly_tu * PORT_DLY_TU_CNT_LMT;
	ori_tsf = MAC_REG_R32(tsfl_regl[port]);
	PLTFM_DELAY_US(TU_UNIT);
	do {
		val32 = MAC_REG_R32(tsfl_regl[port]);
		if ((val32 >= ori_tsf &&
		     (val32 - ori_tsf) >= dly_us) ||
		    (val32 < ori_tsf &&
		     (ori_tsf - val32 + 1) >= dly_us)) {
			break;
		}

		if (val32 == ori_tsf) {
			PLTFM_MSG_ERR("P%d tsf not running 0x%X\n",
				      port, val32);
			return MACHWERR;
		}

		PLTFM_DELAY_US(TU_UNIT);
		cnt--;
	} while (cnt);

	if (!cnt) {
		return MACPOLLTO;
	}

	return MACSUCCESS;
}

u32 dly_port_us(struct mac_ax_adapter *adapter, u8 port, u32 dly_us)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 cnt;
	u32 ori_tsf;

	cnt = dly_us * PORT_DLY_US_CNT_LMT;
	ori_tsf = MAC_REG_R32(tsfl_regl[port]);
	PLTFM_DELAY_US(10);
	do {
		val32 = MAC_REG_R32(tsfl_regl[port]);
		if ((val32 >= ori_tsf &&
		     (val32 - ori_tsf) >= dly_us) ||
		    (val32 < ori_tsf &&
		     (ori_tsf - val32 + 1) >= dly_us)) {
			break;
		}

		if (val32 == ori_tsf) {
			PLTFM_MSG_ERR("P%d tsf not running 0x%X\n",
				      port, val32);
			return MACHWERR;
		}

		PLTFM_DELAY_US(10);
		cnt--;
	} while (cnt);

	if (!cnt) {
		return MACPOLLTO;
	}

	return MACSUCCESS;
}

u32 rst_port_info(struct mac_ax_adapter *adapter)
{
	u8 p_idx;

	for (p_idx = MAC_AX_PORT_0; p_idx < MAC_AX_PORT_NUM; p_idx++)
		PLTFM_MEMSET(&adapter->port_info[get_bp_idx(p_idx)], 0,
			     sizeof(struct mac_ax_port_info));

	return MACSUCCESS;
}

#if 1
static u32 chk_bcnq_empty(struct mac_ax_adapter *adapter, u8 port,
			  u32 bcn_spc, u8 *empty)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 cnt;

	*empty = 1;
	cnt = CHK_BCNQ_CNT * bcn_spc;

	do {
		val32 = MAC_REG_R32(REG_QUE_EMPTY_BCNQ_INFO);
		val32 = BIT_GET_PKT_NUM_BCNQ(val32);
		if (val32 == 0) {
			*empty = 1;
			return MACSUCCESS;
		}
		cnt--;
	} while (cnt);

	if (!cnt) {
		PLTFM_MSG_ERR("[ERR] chk bcnq empty1 timeout\n");
		*empty = 0;
		return MACPOLLTO;
	}
	return MACSUCCESS;
}

static u32 fast_bcn_drop(struct mac_ax_adapter *adapter, u8 port,
			 struct mac_ax_port_info *pinfo)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_port_cfg_para cfg_para;
	u32 ret = MACSUCCESS;
	u32 mbid_drp_sel = 0;
	u32 val32;
	u32 bcn_spc;
	u8 mbid_num;
	u8 is_empty;
	u32 i;

	cfg_para.port = port;
	cfg_para.mbssid_idx = 0;
	cfg_para.val = 1;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_SETUP_TIME, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] port cfg func setup time fail %d\n", ret);
		return ret;
	}

	cfg_para.val = 0;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_HOLD_TIME, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] port cfg func hold time fail %d\n", ret);
		return ret;
	}

	cfg_para.val = 2;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_ERLY, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] port cfg func bcn early fail %d\n", ret);
		return ret;
	}

	if (port == MAC_AX_PORT_0 && pinfo->mbssid_en_stat == MAC_AX_MBSSID_ENABLED) {
		ret = _get_max_mbid(adapter, &mbid_num);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] get_max_mbid %d\n", ret);
			return MACPORTCFGPORT;
		}
		bcn_spc = (u32)(mbid_num + 1) * BCN_FAST_DRP_TBTT;

		val32 = MAC_REG_R32(bcnspc_regl[MAC_AX_PORT_0]);
		val32 = BIT_SET_BCN_SUB_SPACE_PORT0(val32, BCN_TBTT / (bcn_spc));
		MAC_REG_W32(bcnspc_regl[MAC_AX_PORT_0], val32);
		val32 = BIT_SET_BCN_SPACE_PORT0(val32, BCN_TBTT);
		MAC_REG_W32(bcnspc_regl[MAC_AX_PORT_0], val32);

		val32 = MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1);
		val32 = BIT_SET_MBID_BCN_NUM(val32, mbid_num);
		MAC_REG_W32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1, val32);

		val32 = MAC_REG_R32(REG_NOA0_MBSSID_RD_CTRL);
		for (i = 0; i <= mbid_num; i++) {
			val32 |= b_mbid_en_l[i];
		}
		MAC_REG_W32(REG_NOA0_MBSSID_RD_CTRL, val32);

	} else {
		bcn_spc = BCN_FAST_DRP_TBTT;
		cfg_para.val = BCN_TBTT;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_INTV, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] port cfg func bcn intv fail %d\n",
				      ret);
			return ret;
		}
	}

	ret = chk_bcnq_empty(adapter, port, bcn_spc, &is_empty);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] chk bcnq empty fail %d\n", ret);
		return ret;
	}

	ret = dly_port_tu(adapter, port, bcn_spc + 1);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d dly %d tu fail %d\n",
			      port, bcn_spc + 1, ret);
		return ret;
	}

	return MACSUCCESS;
}
#endif
u32 _patch_port_dis_flow(struct mac_ax_adapter *adapter, u8 port,
			 struct mac_ax_port_info *pinfo)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_port_cfg_para cfg_para;
	u32 bcn_set_bk = MAC_AX_R32_DEAD;
	u32 bcn_spc;
	u32 val32;
	u32 ret = MACSUCCESS;
	u8 patch_flag, phb_bkp_flag;
	u8 mbid_max;

	patch_flag = chk_patch_port_dis_flow(adapter) ? 1 : 0;
	phb_bkp_flag = patch_flag && port != MAC_AX_PORT_0 ? 1 : 0;

	if (phb_bkp_flag) {
		bcn_set_bk = MAC_REG_R32(phb_regl[port]);
	}

	if (pinfo->stat == PORT_ST_AP || pinfo->stat == PORT_ST_ADHOC) {
		ret = fast_bcn_drop(adapter, port, pinfo);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] fast bcn drop fail %d\n",
				      ret);
			goto end;
		}
	}

	if (patch_flag) {
		cfg_para.port = port;
		cfg_para.mbssid_idx = 0;
		cfg_para.val = 0;
		ret = _get_port_cfg(adapter, MAC_AX_PCFG_BCN_INTV, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] get port cfg bcn intv fail %d\n",
				      ret);
			goto end;
		}
		bcn_spc = cfg_para.val;

		switch (port) {
		case MAC_AX_PORT_0:
			val32 = MAC_REG_R32(phb_regl[port]);
			val32 = BIT_SET_TBTT_PROHIBIT_SETUP(val32, 0);
			val32 = BIT_SET_TBTT_HOLD_TIME(val32, 1);
			MAC_REG_W32(phb_regl[port], val32);

			val32 = MAC_REG_R32(bcn_early_regl[port]);
			val32 = BIT_SET_DRVERLYITV_P0(val32, 0);
			MAC_REG_W32(bcn_early_regl[port], val32);
			break;
		case MAC_AX_PORT_1:
			val32 = MAC_REG_R32(phb_regl[port]);
			val32 = BIT_SET_TBTT_PROHIBIT_SETUP(val32, 0);
			val32 = BIT_SET_TBTT_HOLD_TIME(val32, 1);
			MAC_REG_W32(phb_regl[port], val32);

			val32 = MAC_REG_R32(bcn_early_regl[port]);
			val32 = BIT_SET_DRVERLYITV_P1(val32, 0);
			MAC_REG_W32(bcn_early_regl[port], val32);
			break;
		case MAC_AX_PORT_2:
			val32 = MAC_REG_R32(phb_regl[0]);
			val32 = BIT_SET_TBTT_PROHIBIT_SETUP(val32, 0);
			MAC_REG_W32(phb_regl[0], val32);

			val32 = MAC_REG_R32(phb_regl[port]);
			val32 = BIT_SET_TBTT_PROHIBIT_HOLD_P2(val32, 1);
			MAC_REG_W32(phb_regl[port], val32);

			val32 = MAC_REG_R32(bcn_early_regl[port]);
			val32 = BIT_SET_DRVERLYITV_P2(val32, 0);
			MAC_REG_W32(bcn_early_regl[port], val32);
			break;
		}

		ret = dly_port_tu(adapter, port, bcn_spc + 1);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d dly %d tu fail %d\n",
				      port, bcn_spc, ret);
			goto end;
		}
	}

	if (port == MAC_AX_PORT_0 && pinfo->mbssid_en_stat == MAC_AX_MBSSID_ENABLED) {
		ret = _get_max_mbid(adapter, &mbid_max);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] get_max_mbid %d\n", ret);
			return MACPORTCFGPORT;
		}

		ret = port0_mbid_set(adapter, 0, mbid_max);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d mbid set fail %d\n",
				      port, ret);
			return ret;
		}
	}

	val32 = MAC_REG_R32(cfg_regl[port]) & ~(b_en_l[port]);
	MAC_REG_W32(cfg_regl[port], val32);

	val32 = MAC_REG_R32(cfg_reg2[port]);
	val32 |= b_rsttsf_l[port];
	MAC_REG_W32(cfg_reg2[port], val32);

	pinfo->stat = PORT_ST_DIS;
end:
	if (phb_bkp_flag) {
		if (bcn_set_bk != MAC_AX_R32_DEAD) {
			MAC_REG_W32(phb_regl[port], bcn_set_bk);
		} else {
			PLTFM_MSG_ERR("[ERR]prev ret %d\n", ret);
			PLTFM_MSG_ERR("[ERR]P%d phb reg dead\n", port);
			ret = MACCMP;
		}
	}

	return ret;
}
/*
u32 _patch_tbtt_shift_setval(struct mac_ax_adapter *adapter, u32 bcnspc,
			     u32 *shift_val)
{
	if (!chk_patch_tbtt_shift_setval(adapter))
		return MACSUCCESS;

	if (!*shift_val)
		return MACSUCCESS;

	if (*shift_val >= bcnspc)
		return MACCMP;

	*shift_val = (bcnspc - *shift_val) | TBTT_SHIFT_OFST_MSB;

	return MACSUCCESS;
}
*/
u32 mac_port_cfg(struct mac_ax_adapter *adapter,
		 enum mac_ax_port_cfg_type type,
		 struct mac_ax_port_cfg_para *para)
{
	struct mac_ax_port_cfg_para cfg_para;
	struct mac_ax_port_info *pinfo;
	u8 band = para->band;
	u8 port = para->port;

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 mbssid_idx = para->mbssid_idx;
	u32 set_val = para->val;
	u16 w_val16 = MAC_AX_R16_DEAD;
	u32 val32 = 0;
	u32 w_val32 = MAC_AX_R32_DEAD;
	u8 mbid_max, val8 = 0;
	u8 i = 0, j = 0;
	u32 ret = MACSUCCESS;
	u8 mbssid_num = 0;

	if (port >= MAC_AX_PORT_NUM) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	ret = _get_max_mbid(adapter, &mbid_max);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] get_max_mbid %d\n", ret);
		return MACPORTCFGPORT;
	}

	if (mbssid_idx) {
		if (mbssid_idx > (u32)mbid_max) {
			PLTFM_MSG_ERR("[ERR] invalid MBSSID number %d\n",
				      mbssid_idx);
			return MACFUNCINPUT;
		}
	}

	pinfo = &adapter->port_info[get_bp_idx(port)];

	switch (type) {
	case MAC_AX_PCFG_FUNC_SW:
		if (set_val) {
			PLTFM_MSG_ERR("[ERR] use port init to enable port\n");
			return MACFUNCINPUT;
		}

		if (pinfo->stat == PORT_ST_DIS) {
			PLTFM_MSG_WARN("[WARN]P%d stat already disable\n", port);
			break;
		}

		//ret = _patch_port_dis_flow(adapter, port, pinfo);


		pinfo->mbssid_en_stat = MAC_AX_MBSSID_INIT;
		break;
	case MAC_AX_PCFG_TX_RPT:
		val32 = MAC_REG_R32(cfg_regl[port]);
		w_val32 = set_val ? val32 | b_txbcnrpt_l[port] :
			  val32 & ~b_txbcnrpt_l[port];
		if (w_val32 != val32) {
			MAC_REG_W32(cfg_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_RX_SW:
		val32 = MAC_REG_R32(cfg_regl[port]);
		w_val32 = set_val ? val32 | b_rxupd_l[port] :
			  val32 & ~b_rxupd_l[port];
		if (w_val32 != val32) {
			MAC_REG_W32(cfg_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_RX_RPT:
		val32 = MAC_REG_R32(cfg_regl[port]);
		w_val32 = set_val ? val32 | b_rxbcnrpt_l[port] :
			  val32 & ~b_rxbcnrpt_l[port];
		if (w_val32 != val32) {
			MAC_REG_W32(cfg_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_RX_SYNC:
		val32 = MAC_REG_R32(cfg_regl[port]);
		w_val32 = set_val ? val32 & ~b_rxtsfupd_l[port] :
			  val32 | b_rxtsfupd_l[port];
		if (w_val32 != val32) {
			MAC_REG_W32(cfg_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_BCN_PRCT:
		val32 = MAC_REG_R32(REG_QUEUE_INCOL_EN);
		w_val32 = set_val ? val32 | BIT_EN_TBTT_AREA_BLK_4AC :
			  val32 & ~(BIT_EN_TBTT_AREA_BLK_4AC);
		if (w_val32 != val32) {
			MAC_REG_W32(REG_QUEUE_INCOL_EN, w_val32);
		}
		break;

	case MAC_AX_PCFG_TBTT_AGG:
		if (set_val > BIT_MASK_P0_TBTT_CNT_THRS) {
			PLTFM_MSG_ERR("[ERR] illegal tbtt agg %d\n", set_val);
			return MACFUNCINPUT;
		}

		val32 = MAC_REG_R32(REG_POF_BCNELY_AGG_TBTT);
		val32 = BIT_SET_P0_TBTT_CNT_THRS(val32, set_val);
		MAC_REG_W32(REG_POF_BCNELY_AGG_TBTT, val32);

		break;
	case MAC_AX_PCFG_RST_TSF:
		val32 = MAC_REG_R32(cfg_reg2[port]);
		w_val32 = set_val ? val32 | b_rsttsf_l[port] :
			  val32 & ~b_rsttsf_l[port];
		if (w_val32 != val32) {
			MAC_REG_W32(cfg_reg2[port], val32);
		}
		break;

	case MAC_AX_PCFG_HIQ_WIN:
		if (mbssid_idx < 2) {
			MAC_REG_W16(hiq_win_mbid_regl[mbssid_idx], (u8)set_val);
		} else {
			MAC_REG_W8(hiq_win_mbid_regl[mbssid_idx], (u8)set_val);
		}
		break;

	case MAC_AX_PCFG_HIQ_DTIM:
		if (set_val > BIT_MASK_DTIM_COUNT_ROOT) {
			PLTFM_MSG_ERR("[ERR] illegal dtim prd %d\n", set_val);
			return MACFUNCINPUT;
		}

		if (port == MAC_AX_PORT_0)
			MAC_REG_W8(dtim_prd_mbid_regl[mbssid_idx],
				   (u8)set_val);
		else
			MAC_REG_W8(dtim_prd_port_regl[port],
				   (u8)set_val);
		break;

	case MAC_AX_PCFG_HIQ_NOLIMIT:
		val32 = MAC_REG_R32(REG_ATIMWND6_7_URGENT_NOLIMTHGQ);
		w_val32 = set_val ? val32 | b_hiq_nolmt_mbid_l[mbssid_idx] :
			  val32 & ~b_hiq_nolmt_mbid_l[mbssid_idx];
		if (w_val32 != val32) {
			MAC_REG_W32(REG_ATIMWND6_7_URGENT_NOLIMTHGQ, w_val32);
		}
		break;

	case MAC_AX_PCFG_BCN_SETUP_TIME:
		ret = _bcn_setup_chk(adapter, port, &para->val);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_WARN("[WARN]bcn setup chk %d\n", ret);
			return ret;
		}

		val32 = MAC_REG_R32(REG_NOA0_TBTT_PROHIBIT);
		w_val32 = BIT_SET_TBTT_PROHIBIT_SETUP(val32, set_val);
		if (w_val32 != val32) {
			MAC_REG_W32(REG_NOA0_TBTT_PROHIBIT, w_val32);
		}

		break;

	case MAC_AX_PCFG_BCN_HOLD_TIME:
		val32 = MAC_REG_R32(phb_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
		case MAC_AX_PORT_1:
			w_val32 = BIT_SET_TBTT_HOLD_TIME(val32, set_val);
			break;
		case MAC_AX_PORT_2:
			w_val32 = BIT_SET_TBTT_PROHIBIT_HOLD_P2(val32, set_val);
			break;
		default:
			break;
		}

		if (w_val32 != val32) {
			MAC_REG_W32(phb_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_MBSSID_EN:
		if (port != MAC_AX_PORT_0 || !mbssid_idx) {
			PLTFM_MSG_ERR("[ERR] mbssid en invalid for P%d MB%d\n",
				      port, mbssid_idx);
			return MACFUNCINPUT;
		}

		val32 = MAC_REG_R32(REG_NOA0_MBSSID_RD_CTRL);
		mbssid_num =
			(MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1) >> 16) & 0x7;
		if (mbssid_idx > mbssid_num) {
			PLTFM_MSG_ERR("[ERR] mbssid %d over %d\n", mbssid_idx, mbssid_num);
			return MACFUNCINPUT;
		}

		w_val32 = set_val ? val32 | b_mbid_en_l[mbssid_idx] :
			  val32 & ~b_mbid_en_l[mbssid_idx];

		if (w_val32 != val32) {
			MAC_REG_W32(REG_NOA0_MBSSID_RD_CTRL, w_val32);
		}

		pinfo->mbssid_en_stat = MAC_AX_MBSSID_ENABLED;
		break;

	case MAC_AX_PCFG_BCN_ERLY:
		ret = _bcn_erly_chk(adapter, port, &para->val);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_WARN("[WARN]bcn erly chk %d\n", ret);
			return ret;
		}

		val32 = MAC_REG_R32(bcn_early_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
			w_val32 = BIT_SET_DRVERLYITV_P0(val32, set_val);
			break;
		case MAC_AX_PORT_1:
			w_val32 = BIT_SET_DRVERLYITV_P1(val32, set_val);
			break;
		case MAC_AX_PORT_2:
			w_val32 = BIT_SET_DRVERLYITV_P2(val32, set_val);
			break;
		}
		if (w_val32 != val32) {
			MAC_REG_W32(bcn_early_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_BSS_CLR:
		if (set_val > BIT_MASK_R_BSS_COLOR0) {
			PLTFM_MSG_ERR("[ERR] illegal bss color %d\n", set_val);
			return MACFUNCINPUT;
		}

		val32 = MAC_REG_R32(bss_color_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
			w_val32 = BIT_SET_R_BSS_COLOR0(val32, set_val);
			break;
		case MAC_AX_PORT_1:
			w_val32 = BIT_SET_R_BSS_COLOR1(val32, set_val);
			break;
		}

		if (w_val32 != val32) {
			MAC_REG_W32(bss_color_regl[port], w_val32);
		}
		break;

	case MAC_AX_PCFG_MBSSID_NUM:
		if (port != MAC_AX_PORT_0 || pinfo->stat != PORT_ST_AP) {
			PLTFM_MSG_ERR("[ERR]port is not 0 or is not AP\n");
			return MACPORTCFGPORT;
		}
		_set_max_mbid_num(adapter, para);
		break;

	case MAC_AX_PCFG_BCN_INTV:
		if (set_val > BIT_MASK_BCN_SPACE_PORT0 || !set_val) {
			PLTFM_MSG_ERR("[ERR] illegal bcn itvl %d\n", set_val);
			return MACFUNCINPUT;
		}

		val32 = MAC_REG_R32(bcnspc_regl[port]);
		switch (port) {
		case MAC_AX_PORT_0:
			w_val32 = BIT_SET_BCN_SPACE_PORT0(val32, set_val);
			break;
		case MAC_AX_PORT_1:
			w_val32 = BIT_SET_BCN_SPACE3(val32, set_val);
			break;
		case MAC_AX_PORT_2:
			w_val32 = BIT_SET_BCN_SPACE4(val32, set_val);
			break;
		}
		if (w_val32 != val32) {
			MAC_REG_W32(bcnspc_regl[port], w_val32);
		}
		break;
	case MAC_AX_PCFG_SW_BCN:
#ifdef RTW_PHL_BCN_IOT
		if (set_val == 1) {
			/* set REG_CR bit 8 */
			val8 = MAC_REG_R8(REG_CR + 1);
			val8 |= BIT(0); // ENSWBCN
			MAC_REG_W8(REG_CR + 1, val8);

			/* Software beacon should set this bit */
			val32 = MAC_REG_R32(REG_AXI_CTRL);
			val32 |= BIT_MBSSID_ENSWBCN_BACKDOOR;
			MAC_REG_W32(REG_AXI_CTRL, val32);

			val32 = MAC_REG_R32(REG_DWBCN1_CTRL);
			switch (port) {
			case MAC_AX_PORT_0:
				val32 &= ~BIT_SW_BCN_SEL_EN;
				break;
			case MAC_AX_PORT_1:
				val32 |= BIT_SW_BCN_SEL_EN;
				val32 = BIT_SET_SW_BCN_SEL(val32, 1);
				break;
			case MAC_AX_PORT_2:
				val32 |= BIT_SW_BCN_SEL_EN;
				val32 = BIT_SET_SW_BCN_SEL(val32, 2);
				break;
			}
			MAC_REG_W32(REG_DWBCN1_CTRL, val32);
		} else {
			/* set REG_CR bit 8 */
			val8 = MAC_REG_R8(REG_CR + 1);
			val8 &= ~BIT(0); // ENSWBCN
			MAC_REG_W8(REG_CR + 1, val8);

			/* Software beacon should set this bit */
			val32 = MAC_REG_R32(REG_AXI_CTRL);
			val32 &= ~BIT_MBSSID_ENSWBCN_BACKDOOR;
			MAC_REG_W32(REG_AXI_CTRL, val32);

			val32 = MAC_REG_R32(REG_DWBCN1_CTRL);
			val32 &= ~(BIT_SW_BCN_SEL_EN | BITS_SW_BCN_SEL);
			MAC_REG_W32(REG_DWBCN1_CTRL, val32);
		}
#endif
		break;

	default:
		PLTFM_MSG_ERR("[ERR] invalid cfg type %d\n", type);
		return MACPORTCFGTYPE;
	}

	return ret;
}

u32 mac_port_init(struct mac_ax_adapter *adapter,
		  struct mac_ax_port_init_para *para)
{
	struct mac_ax_port_info *pinfo;
	u8 port = para->port_idx;
	u8 net_type = para->net_type;

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_port_cfg_para cfg_para;
	struct mac_ax_pkt_drop_info info;
	u8 mbid_num;
	u8 mbid_max;
	u8 i = 0;
	u32 ret = MACSUCCESS;
	u32 val32;
	u32 bcn_erly = BCN_ERLY_DEF;
	u32 hold_time = BCN_HOLD_DEF;

	if (port >= MAC_AX_PORT_NUM) {
		PLTFM_MSG_ERR("[ERR]invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	mbid_num = net_type == MAC_AX_NET_TYPE_AP && port == MAC_AX_PORT_0 ?
		   para->mbid_num : 0;

	ret = _get_max_mbid(adapter, &mbid_max);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]get_max_mbid %d\n", ret);
		return MACPORTCFGPORT;
	}

	if (mbid_num > mbid_max) {
		PLTFM_MSG_ERR("[ERR]invalid MBSSID number %d\n", mbid_num);
		return MACFUNCINPUT;
	}
	//if (mbid_num && !(mbid_num % 2))
	//	mbid_num++;

	pinfo = &adapter->port_info[get_bp_idx(port)];

	if (mbid_num) {
		pinfo->mbssid_en_stat = MAC_AX_MBSSID_ENABLED;
	}

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]chk mac en %d\n", ret);
		return ret;
	}

	cfg_para.port = port;
	cfg_para.mbssid_idx = 0;
	cfg_para.net_type = net_type;
	/*
	if (pinfo->stat != PORT_ST_DIS) {
		cfg_para.val = 0;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_FUNC_SW, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d cfg func sw 0 fail %d\n",
				      port, ret);
			return ret;
		}
	}*/

	cfg_para.val = (net_type == MAC_AX_NET_TYPE_AP) ? 1 : 0;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_TX_RPT, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg tx rpt fail %d\n",
			      port, ret);
		return ret;
	}

	cfg_para.val = (net_type == MAC_AX_NET_TYPE_AP) ? 0 : 1;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_RX_RPT, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg rx rpt fail %d\n",
			      port, ret);
		return ret;
	}

	cfg_para.val = net_type;
	ret = _port_cfg(adapter, MAC_AX_PCFG_NET_TYPE, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg net type fail %d\n",
			      port, ret);
		return ret;
	}
	/*
	cfg_para.val = net_type == MAC_AX_NET_TYPE_NO_LINK ? 0 : 1;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_PRCT, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg bcn prct fail %d\n",
			      port, ret);
		return ret;
	}*/

	cfg_para.val = 0;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_RX_SW, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg rx sw fail %d\n",
			      port, ret);
		return ret;
	}

	/* disable update TSF */
	cfg_para.val = 0;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_RX_SYNC, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg rx sync fail %d\n",
			      port, ret);
		return ret;
	}

	cfg_para.val = (u32)para->bcn_interval;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_INTV, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg bcn intv fail %d\n",
			      port, ret);
		return ret;
	}

	cfg_para.val = (u32)para->bss_color;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BSS_CLR, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg bss_color fail %d\n",
			      port, ret);
		return ret;
	}
	/*
	cfg_para.val = TBTT_AGG_DEF;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_TBTT_AGG, &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg tbtt agg fail %d\n",
			      port, ret);
		return ret;
	}*/

	for (i = 0; i <= mbid_num; i++) {
		cfg_para.mbssid_idx = mbid_idx_l[i];
		cfg_para.val = para->hiq_win;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_HIQ_WIN, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d mbid%d cfg hiq win %d\n",
				      port, cfg_para.mbssid_idx, ret);
			return ret;
		}
		cfg_para.val = (u32)para->dtim_period;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_HIQ_DTIM, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d mbid%d cfg hiq dtim %d\n",
				      port, cfg_para.mbssid_idx, ret);
			return ret;
		}

		/* set high queue to no limit mode */
		cfg_para.val = 1;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_HIQ_NOLIMIT, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d mbid%d cfg hiq no limit %d\n",
				      port, cfg_para.mbssid_idx, ret);
			return ret;
		}
	}

	if (port == MAC_AX_PORT_0) {
		ret = port0_subspc_set(adapter, mbid_num,
				       &bcn_erly, &hold_time);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d subspc set fail %d\n",
				      port, ret);
			return ret;
		}
	}

	cfg_para.val = hold_time;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_HOLD_TIME, &cfg_para);
	if (ret == MACSETVALERR)
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_HOLD_TIME,
				   &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg hold time fail %d\n",
			      port, ret);
		return ret;
	}

	val32 = MAC_REG_R32(cfg_regl[port]) | b_en_l[port];
	MAC_REG_W32(cfg_regl[port], val32);

	switch (net_type) {
	case MAC_AX_NET_TYPE_NO_LINK:
		pinfo->stat = PORT_ST_NOLINK;
		cfg_para.val = 0;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_SW_BCN, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d disable sw beacon fail %d\n",
			      port, ret);
			return ret;
		}
		break;
	case MAC_AX_NET_TYPE_ADHOC:
		pinfo->stat = PORT_ST_ADHOC;
		break;
	case MAC_AX_NET_TYPE_INFRA:
		pinfo->stat = PORT_ST_INFRA;
		cfg_para.val = 0;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_SW_BCN, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d disable sw beacon fail %d\n",
			      port, ret);
			return ret;
		}
		break;
	case MAC_AX_NET_TYPE_AP:
		pinfo->stat = PORT_ST_AP;
		cfg_para.val = 1;
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_SW_BCN, &cfg_para);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d enable sw beacon fail %d\n",
			      port, ret);
			return ret;
		}
		break;
	}

	ret = dly_port_us(adapter, port, BCN_ERLY_SET_DLY);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d dly %d us fail %d\n",
			      port, BCN_ERLY_SET_DLY, ret);
		return ret;
	}

	cfg_para.val = bcn_erly;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_ERLY, &cfg_para);
	if (ret == MACSETVALERR) {
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_ERLY, &cfg_para);
	}
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg bcn early fail %d\n",
			      port, ret);
		return ret;
	}

	cfg_para.val = BCN_SETUP_DEF;
	ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_SETUP_TIME, &cfg_para);
	if (ret == MACSETVALERR)
		ret = mac_port_cfg(adapter, MAC_AX_PCFG_BCN_SETUP_TIME,
				   &cfg_para);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]P%d cfg setup time fail %d\n",
			      port, ret);
		return ret;
	}

	if (port == MAC_AX_PORT_0) {
		ret = port0_mbid_set(adapter, mbid_num, mbid_max);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]P%d mbid set fail %d\n",
				      port, ret);
			return ret;
		}
	}

	return ret;
}
#if 1
u32 mac_tsf_sync(struct mac_ax_adapter *adapter, u8 from_port, u8 to_port,
		 s32 sync_offset, enum mac_ax_tsf_sync_act action)
{
	u32 val32, ret = MACSUCCESS;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (sync_offset > BIT_MASK_TSFTR_SYNC_OFFSET) {
		PLTFM_MSG_ERR("[ERR] Invalid tsf_sync offset.\n");
		return MACFUNCINPUT;
	}

	val32 = MAC_REG_R32(REG_TSFT_SYN_OFFSET_SLOT);
	val32 = BIT_SET_TSFTR_SYNC_OFFSET(val32, sync_offset);
	MAC_REG_W32(REG_TSFT_SYN_OFFSET_SLOT, val32);

	switch (action) {
	case MAC_AX_P0_P1:
		MAC_REG_W32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1,
			    MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1) | BIT_TSFTR_SYNC_EN);
		break;
	case MAC_AX_P1_P0:
		MAC_REG_W32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1,
			    MAC_REG_R32(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1) | BIT_TSFTR1_SYNC_EN);
		break;
	case MAC_AX_P1_P2:
		MAC_REG_W32(REG_DISATIM_BCNERLY_TBTTHOLD_BCNCS,
			    MAC_REG_R32(REG_DISATIM_BCNERLY_TBTTHOLD_BCNCS) | BIT_TSFTR2_SYNC_EN);
		break;
	case MAC_AX_P2_P1:
		MAC_REG_W32(REG_DISATIM_BCNERLY_TBTTHOLD_BCNCS,
			    MAC_REG_R32(REG_DISATIM_BCNERLY_TBTTHOLD_BCNCS) | BIT_TSFTR3_SYNC_EN);
		break;
	default:
		PLTFM_MSG_ERR("[ERR] Invalid tsf_sync input.\n");
		return MACFUNCINPUT;
	}

	return ret;
}
#endif
//modify by fw
u32 mac_parse_bcn_stats_c2h(struct mac_ax_adapter *adapter,
			    u8 *content, struct mac_ax_bcn_cnt *val)

{
	u32 *pdata, data0, data1;
	u8 port_mbssid;

	pdata = (u32 *)content;
	data0 = le32_to_cpu(*pdata);
	data1 = le32_to_cpu(*(pdata + 1));

	port_mbssid = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_PORT_MBSSID_IDX);
	if (port_mbssid < MAC_AX_P0_MBID_LAST) {
		val->port = 0;
		val->mbssid = port_mbssid;
	} else {
		val->port = port_mbssid -  MAC_AX_P0_MBID_LAST + 1;
		val->mbssid = 0;
	}
	val->band = data0 & FWCMD_C2H_BCN_CNT_BAND_IDX ? 1 : 0;

	val->cca_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_CCA_FAIL_CNT);
	val->edcca_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_EDCCA_FAIL_CNT);
	val->nav_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_NAV_FAIL_CNT);
	val->txon_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_TXON_FAIL_CNT);
	val->mac_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_MAC_FAIL_CNT);
	val->others_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_OTHERS_FAIL_CNT);
	val->lock_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_LOCK_FAIL_CNT);
	val->cmp_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_CMP_FAIL_CNT);
	val->invalid_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_INVALID_FAIL_CNT);
	val->srchend_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_SRCHEND_FAIL_CNT);
	val->ok_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_OK_CNT);

	return MACSUCCESS;
}
//modify by fw
#if 0
u32 get_bcn_stats_event(struct mac_ax_adapter *adapter,
			struct rtw_c2h_info *c2h,
			enum phl_msg_evt_id *id, u8 *c2h_info)
{
	struct mac_ax_bcn_cnt *info;
	u32 *pdata, data0, data1;
	u8 port_mbssid;

	info = (struct mac_ax_bcn_cnt *)c2h_info;
	pdata = (u32 *)c2h->content;
	data0 = le32_to_cpu(*pdata);
	data1 = le32_to_cpu(*(pdata + 1));

	port_mbssid = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_PORT_MBSSID_IDX);
	if (port_mbssid < MAC_AX_P0_MBID_LAST) {
		info->port = 0;
		info->mbssid = port_mbssid;
	} else {
		info->port = port_mbssid -  MAC_AX_P0_MBID_LAST + 1;
		info->mbssid = 0;
	}
	info->band = data0 & FWCMD_C2H_BCN_CNT_BAND_IDX ? 1 : 0;

	info->cca_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_CCA_FAIL_CNT);
	info->edcca_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_EDCCA_FAIL_CNT);
	info->nav_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_NAV_FAIL_CNT);
	info->txon_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_TXON_FAIL_CNT);
	info->mac_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_MAC_FAIL_CNT);
	info->others_cnt = GET_FIELD(data0, FWCMD_C2H_BCN_CNT_OTHERS_FAIL_CNT);
	info->lock_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_LOCK_FAIL_CNT);
	info->cmp_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_CMP_FAIL_CNT);
	info->invalid_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_INVALID_FAIL_CNT);
	info->srchend_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_SRCHEND_FAIL_CNT);
	info->ok_cnt = GET_FIELD(data1, FWCMD_C2H_BCN_CNT_OK_CNT);

	*id = MSG_EVT_BCN_CNT_RPT;

	return MACSUCCESS;
}
//modify by fw
u32 mac_tsf32_togl_h2c(struct mac_ax_adapter *adapter,
		       struct mac_ax_t32_togl_info *info)
{
#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
#else
	struct h2c_buf *h2cb;
#endif
	struct fwcmd_tsf32_togl *hdr;
	u32 ret = MACSUCCESS;

	if (info->port >= MAC_AX_PORT_NUM) {
		PLTFM_MSG_ERR("[ERR]invalid port %d\n", info->port);
		return MACFUNCINPUT;
	}

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb) {
		return MACNPTR;
	}

	hdr = (struct fwcmd_tsf32_togl *)
	      h2cb_put(h2cb, sizeof(struct fwcmd_tsf32_togl));
	if (!hdr) {
		ret = MACNOBUF;
		goto fail;
	}

	hdr->dword0 =
		cpu_to_le32(SET_WORD(info->port, FWCMD_H2C_TSF32_TOGL_PORT) |
			    SET_WORD(info->early, FWCMD_H2C_TSF32_TOGL_EARLY) |
			    (info->band ? FWCMD_H2C_TSF32_TOGL_BAND : 0) |
			    (info->en ? FWCMD_H2C_TSF32_TOGL_EN : 0));

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C,
			      FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD,
			      FWCMD_H2C_FUNC_TSF32_TOGL,
			      0,
			      0);
	if (ret) {
		goto fail;
	}

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret) {
		goto fail;
	}

#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
#endif
	if (ret) {
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}
#endif
#if 1
u32 mac_get_t32_togl_rpt(struct mac_ax_adapter *adapter,
			 struct mac_ax_t32_togl_rpt *ret_rpt)
{
	struct mac_ax_t32_togl_rpt *rpt;
	u8 p_idx;

	for (p_idx = MAC_AX_PORT_0; p_idx < MAC_AX_PORT_NUM; p_idx++) {
		rpt = &adapter->t32_togl_rpt[get_bp_idx(p_idx)];
		if (!rpt->valid) {
			continue;
		}
		PLTFM_MEMCPY(ret_rpt, rpt,
			     sizeof(struct mac_ax_t32_togl_rpt));
		rpt->valid = 0;
		return MACSUCCESS;
	}

	PLTFM_MSG_WARN("[WARN]no tsf32 togl rpt find\n");
	return MACNOITEM;
}

u32 mport_info_init(struct mac_ax_adapter *adapter)
{
	u8 p_idx;
	u32 idx;

	adapter->t32_togl_rpt =
		(struct mac_ax_t32_togl_rpt *)PLTFM_MALLOC(t32_togl_rpt_size);
	adapter->port_info =
		(struct mac_ax_port_info *)PLTFM_MALLOC(port_info_size);
	for (p_idx = MAC_AX_PORT_0; p_idx < MAC_AX_PORT_NUM; p_idx++) {
		idx = get_bp_idx(p_idx);
		PLTFM_MEMSET(&adapter->t32_togl_rpt[idx], 0,
			     sizeof(struct mac_ax_t32_togl_rpt));
		PLTFM_MEMSET(&adapter->port_info[idx], 0,
			     sizeof(struct mac_ax_port_info));
	}

	return MACSUCCESS;
}

u32 mport_info_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_FREE(adapter->t32_togl_rpt, t32_togl_rpt_size);
	PLTFM_FREE(adapter->port_info, port_info_size);

	return MACSUCCESS;
}
#endif