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
#include "la_mode.h"

u32 mac_lamode_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_la_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, val;

	val32 = MAC_REG_R32(REG_WMAC_DBG_FTM_CTL);
	switch (cfg->la_mode) {
	case HALMAC_LA_MODE_DUMP_BB:
		val32 &= ~(BIT_EN_RXD_DEBUG | BIT_EN_TXD_DEBUG);
		break;
	case HALMAC_LA_MODE_DUMP_TXD:
		val32 = (val32 & (~BIT_EN_RXD_DEBUG)) | BIT_EN_TXD_DEBUG;
		break;
	case HALMAC_LA_MODE_DUMP_RXD:
		val32 = (val32 & (~BIT_EN_TXD_DEBUG)) | BIT_EN_RXD_DEBUG;
		break;
	default:
		return MACNOITEM;
	};

	if (cfg->la_restart_en) {
		val32 |= BIT_EN_RE_START;
	} else {
		val32 &= ~BIT_EN_RE_START;
	}
	MAC_REG_W32(REG_WMAC_DBG_FTM_CTL, val32);

	/*trig start*/
	val32 = MAC_REG_R32(REG_IQ_DUMP);
	val32 = ((val32 & (~BIT_EN_LA_MAC)) | BIT_R_EN_IQDUMP | BIT_R_IQDATA_DUMP) &
		(~BIT_MACDBG_TRIG_IQDUMP);
	val32 = BIT_SET_R_LEVEL_PULSE_SEL(val32, cfg->trig_level_sel);
	MAC_REG_W32(REG_IQ_DUMP, val32);

	/*trig stop*/
	val32 = MAC_REG_R32(REG_WMAC_DBG_FTM_CTL);
	switch (cfg->trig_stop) {
	case MACTRX_TRIG_STOP:
		val32 |= BIT_TRX_TRIG_SEL_WMAC;
		val = MAC_REG_R32(REG_CHINFO_TRIG_TYPE1);
		val = BIT_SET_R_RXTRIG_STOP_SEL(val, cfg->mac_rx_trig_stop_val);
		val = BIT_SET_R_TXTRIG_STOP_SEL(val, cfg->mac_tx_trig_stop_val);
		MAC_REG_W32(REG_CHINFO_TRIG_TYPE1, val);
		break;
	case MAC_DEBUG_PORT:
		val32 |= BIT_TRX_TRIG_SEL_DEBUG;
		MAC_REG_W32(REG_MASK_LA_MAC, cfg->debug_port_trig_stop_mask);
		MAC_REG_W32(REG_MATCH_REF_MAC, cfg->debug_port_trig_stop_ref);
		MAC_REG_W8(REG_WL_PIN_FUNC_CTRL0 + 3, cfg->mac_debug_sel);
		break;
	case MAC_TXDRXD:
		val32 |= BIT_TRX_TRIG_SEL_PATT;
		MAC_REG_W32(REG_MASK_LA_TRX_DBG_L, cfg->txd_trig_stop_mask_l);
		MAC_REG_W32(REG_MASK_LA_TRX_DBG_H, cfg->txd_trig_stop_mask_h);
		MAC_REG_W32(REG_REF_LA_TRX_DBG_L, cfg->txd_trig_stop_ref_l);
		MAC_REG_W32(REG_REF_LA_TRX_DBG_H, cfg->txd_trig_stop_ref_h);
		break;
	default:
		return MACNOITEM;
	};

	MAC_REG_W32(REG_WMAC_DBG_FTM_CTL, val32);

	/*trig timeout*/
	val32 = MAC_REG_R32(REG_WMAC_DBG_FTM_CTL);
	if (cfg->la_tri_len_bk) {
		val32 = BIT_SET_IQD_LENGTH_REF(val32, cfg->la_tgr_len);
	}
	val32 = BIT_SET_TRIG_TIME_UNIT_SEL(val32, cfg->la_tgr_tu_sel);
	MAC_REG_W32(REG_WMAC_DBG_FTM_CTL, val32);

	val32 = MAC_REG_R32(REG_IQ_DUMP);
	val32 = BIT_SET_R_TRIG_TIME_SEL(val32, cfg->la_tgr_time_val);
	MAC_REG_W32(REG_IQ_DUMP, val32);

	return MACSUCCESS;
}

struct mac_ax_la_status mac_get_lamode_st(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	struct mac_ax_la_status info;

	info.la_buf_wptr = 0;
	info.la_buf_rndup_ind = 0;

	val32 = MAC_REG_R32(REG_IQ_DUMP);

	info.la_buf_wptr = (val32 >> BIT_SHIFT_DUMP_OK_ADDR) & 0x7FFF;
	info.la_buf_rndup_ind = (val32 & BIT(31)) ? 1 : 0;

	return info;
}

