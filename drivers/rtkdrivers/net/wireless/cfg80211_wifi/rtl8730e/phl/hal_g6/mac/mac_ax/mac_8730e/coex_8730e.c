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
#include "coex_8730e.h"
#include "../../mac_reg.h"
#include "../hw.h"

#if MAC_AX_8730E_SUPPORT

#define MAC_AX_RTK_RATE 5

#define MAC_AX_BT_MODE_0_3 0
#define MAC_AX_BT_MODE_2 2

#define MAC_AX_CSR_DELAY 0
#define MAC_AX_CSR_PRI_TO 5
#define MAC_AX_CSR_TRX_TO 4

#define MAC_AX_CSR_RATE 80

#define MAC_AX_SB_DRV_MSK 0xFFFFFF
#define MAC_AX_SB_DRV_SH 0
#define MAC_AX_SB_FW_MSK 0x7F
#define MAC_AX_SB_FW_SH 24

#define MAC_AX_BTGS1_NOTIFY BIT(0)

u32 mac_coex_init_8730e(struct mac_ax_adapter *adapter,
			struct mac_ax_coex *coex)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val;
	u32 val32;

	/* Enable PTA, 0x74[16]=1 */
	val32 = MAC_REG_R32(REG_WL_BTCOEX_CTRL);
	MAC_REG_W32(REG_WL_BTCOEX_CTRL, val32 | BIT_ENBT_V1);

	/* PTA pre_wl_tx enable, WLAN TX can request during gnt_bt=1. 0x4C6[4]=1 */
	/* wl_tx signal to PTA does not consider EDCCA. 0x4C6[5]=0 */
	val32 = MAC_REG_R32(REG_PTA_STBC_CTRL);
	val32 = (val32 | BIT_PTA_WL_TX_EN) & ~BIT_PTA_WL_TX_EN;
	MAC_REG_W32(REG_PTA_STBC_CTRL, val32);

	/*GNT_BT=1 while select both, 0x763[4]=1 */
	val = MAC_REG_R32(REG_WMAC_SWAES_CFG);
	MAC_REG_W32(REG_WMAC_SWAES_CFG, val | BIT_GNT_BOTH_POL);

	val = MAC_REG_R32(REG_WLAN_ACT_MASK_CTRL_1);
	MAC_REG_W32(REG_WLAN_ACT_MASK_CTRL_1, val | BIT_EN_WL_ACT_MASK |
		    BIT_STATIS_BT_EN | BIT_R_GRANTALL_WLMASK);

	/* Disable BT_CCA. 0x523[7]=0 */
	val = MAC_REG_R32(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL);
	val = (val & ~BIT_DIS_BT_CCA);
	MAC_REG_W32(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL, val);

	switch (coex->pta_mode) {
	case MAC_AX_COEX_RTK_MODE:
		/* Select BT mode, 0x74[18:17]=2b'00 */
		val = MAC_REG_R32(REG_WL_BTCOEX_CTRL);
		val = BIT_SET_BTMODE_V1(val, MAC_AX_BT_MODE_0_3);
		MAC_REG_W32(REG_WL_BTCOEX_CTRL, val);
		/* Enable RTK mode, 0x778[0]=1 */
		val = MAC_REG_R32(REG_BT_PROPRI);
		MAC_REG_W32(REG_BT_PROPRI, val | BIT_RTK_BT_ENABLE);
		/* BT report packet sample rate, 0x790[5:0]=0x5 */
		val = MAC_REG_R32(REG_BT_TDMA_TIME_REGISTER);
		val = BIT_SET_BT_RPT_SAMPLE_RATE(val, MAC_AX_RTK_RATE);
		MAC_REG_W32(REG_BT_TDMA_TIME_REGISTER, val);
		break;
	case MAC_AX_COEX_CSR_MODE:
		val = MAC_REG_R32(REG_WL_BTCOEX_CTRL);
		val = BIT_SET_BTMODE_V1(val, MAC_AX_BT_MODE_2);
		MAC_REG_W32(REG_WL_BTCOEX_CTRL, val);

		val = MAC_REG_R32(REG_WLAN_ACT_MASK_CTRL_1);
		val = BIT_SET_BT_PRI_DETECT_TO(val, MAC_AX_CSR_PRI_TO);
		val = BIT_SET_BT_TRX_INIT_DETECT(val, MAC_AX_CSR_TRX_TO);
		val = BIT_SET_BT_STAT_DELAY(val, MAC_AX_CSR_DELAY);
		val = val | BIT_ENHBT;
		MAC_REG_W32(REG_WLAN_ACT_MASK_CTRL_1, val);

		MAC_REG_W8(REG_WMAC_SWAES_CFG + 2, MAC_AX_CSR_RATE);
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 mac_get_gnt_8730e(struct mac_ax_adapter *adapter,
		      struct mac_ax_coex_gnt *gnt_cfg)
{
	u32 val;
	struct mac_ax_gnt *gnt;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val = MAC_REG_R32(REG_BT_COEX_ENH);

	gnt = &gnt_cfg->band;
	gnt->gnt_bt_bb_sw_en = (val & BIT_R_GNT_BT_BB_SW_EN) ? 1 : 0;
	gnt->gnt_bt_bb_sw = (val & BIT_R_GNT_BT_BB_SW) ? 1 : 0;
	gnt->gnt_bt_rfc_sw_en = (val & BIT_R_GNT_BT_RFC_SW_EN) ? 1 : 0;
	gnt->gnt_bt_rfc_sw = (val & BIT_R_GNT_BT_RFC_SW) ? 1 : 0;

	return MACSUCCESS;
}

u32 mac_cfg_gnt_8730e(struct mac_ax_adapter *adapter,
		      struct mac_ax_coex_gnt *gnt_cfg)
{
	u32 val;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val = MAC_REG_R32(REG_BT_COEX_ENH);
	val &= 0xFFFFE1FF;
	switch (gnt_cfg->state) {
	case BTC_GNT_HW_PTA: /* 0x765[12:9]=4b'0000 */
		val = val & ~(BIT_R_GNT_BT_BB_SW_EN | BIT_R_GNT_BT_RFC_SW_EN |
			      BIT_R_GNT_BT_BB_SW | BIT_R_GNT_BT_RFC_SW);
		break;
	case BTC_GNT_SW_LOW:/* 0x765[12:9]=4b'0101 */
		val = (val | BIT_R_GNT_BT_BB_SW_EN | BIT_R_GNT_BT_RFC_SW_EN) &
		      ~(BIT_R_GNT_BT_BB_SW | BIT_R_GNT_BT_RFC_SW);
		break;
	case BTC_GNT_SW_HIGH:/* 0x765[12:9]=4b'1111 */
		val |= BIT_R_GNT_BT_BB_SW_EN | BIT_R_GNT_BT_RFC_SW_EN |
		       BIT_R_GNT_BT_BB_SW | BIT_R_GNT_BT_RFC_SW;
		break;
	default:
		return MACNOITEM;
	}

	MAC_REG_W32(REG_BT_COEX_ENH, val);
	/*
	val = (gnt_cfg->band.gnt_bt ? (BIT_R_GNT_BT_RFC_SW |
		 BIT_R_GNT_BT_BB_SW) : 0) |
		(gnt_cfg->band.gnt_bt_sw_en ?
		 (BIT_R_GNT_BT_RFC_SW_EN |
		  BIT_R_GNT_BT_BB_SW_EN) : 0) |
		(gnt_cfg->band.gnt_wl ? 0 : (BIT_R_GNT_BT_RFC_SW |
					  BIT_R_GNT_BT_BB_SW)) |
		(gnt_cfg->band.gnt_wl_sw_en ? 0 :
		 (BIT_R_GNT_BT_RFC_SW_EN | BIT_R_GNT_BT_BB_SW_EN));
	*/



	return MACSUCCESS;
}

#endif /* #if MAC_AX_8852A_SUPPORT */
