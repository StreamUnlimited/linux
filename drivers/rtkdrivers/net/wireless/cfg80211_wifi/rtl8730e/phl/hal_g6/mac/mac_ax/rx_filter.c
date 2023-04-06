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
#include "rx_filter.h"

/* control frames: drop, mgnt frames: to host, data frame: to host */
static enum mac_ax_fwd_target trxcfg_rx_fltr_mgnt_frame = MAC_AX_FWD_ACPT;
static enum mac_ax_fwd_target trxcfg_rx_fltr_ctrl_frame = MAC_AX_FWD_ACPT;
static enum mac_ax_fwd_target trxcfg_rx_fltr_data_frame = MAC_AX_FWD_ACPT;

static struct mac_ax_rx_fltr_ctrl_t rx_fltr_init_opt = {
	0, /* acpt_aap */
	1, /* acpt_apm */
	1, /* acpt_am */
	1, /* acpt_ab */
	0, /* acpt_add3 */
	0, /* acpt_apwrmgt */
	1, /* chk_cbssid_data */
	1, /* chk_cbssid_mgnt */
	0, /* acpt_acrc32 */
	0, /* acpt_aicv */
	0, /* chk_ta_mgnt */
	0, /* acpt_ack_cbssid */
};

u32 rx_fltr_init(struct mac_ax_adapter *adapter)
{
	u32 ret;
	struct mac_ax_ops *mac_ax_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_rx_fltr_ctrl_t opt_msk = {
		0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
		0x1, 0x1
	};

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ret = mac_ax_ops->set_rx_fltr_typ_opt(adapter,
					      MAC_AX_PKT_MGNT,
					      trxcfg_rx_fltr_mgnt_frame);
	if (ret != MACSUCCESS) {
		return ret;
	}
	ret = mac_ax_ops->set_rx_fltr_typstyp_opt(adapter,
					      MAC_AX_PKT_CTRL, 10,
					      trxcfg_rx_fltr_ctrl_frame);
	if (ret != MACSUCCESS) {
		return ret;
	}
	ret = mac_ax_ops->set_rx_fltr_typ_opt(adapter,
					      MAC_AX_PKT_DATA,
					      trxcfg_rx_fltr_data_frame);
	if (ret != MACSUCCESS) {
		return ret;
	}
	ret = mac_ax_ops->set_rx_fltr_opt(adapter,
					  &rx_fltr_init_opt,
					  &opt_msk);
	if (ret != MACSUCCESS) {
		return ret;
	}

	return MACSUCCESS;
}

static void rx_fltr_opt_2_uint(struct mac_ax_adapter *adapter,
			       struct mac_ax_rx_fltr_ctrl_t *fltr_opt,
			       u32 *mac_fltr_value)
{
	u32 val32 = 0x0;

	val32 =
		((fltr_opt->acpt_aap) ? BIT_AAP : 0) |
		((fltr_opt->acpt_apm) ? BIT_APM : 0) |
		((fltr_opt->acpt_am) ? BIT_AM : 0) |
		((fltr_opt->acpt_ab) ? BIT_AB : 0) |
		((fltr_opt->acpt_add3) ? BIT_ADD3 : 0) |
		((fltr_opt->acpt_apwrmgt) ? BIT_APWRMGT : 0) |
		((fltr_opt->chk_cbssid_data) ? BIT_CBSSID_DATA : 0) |
		((fltr_opt->chk_cbssid_mgnt) ? BIT_CBSSID_MGNT : 0) |
		//((fltr_opt->acpt_pwr_mgnt_pkt) ? B_AX_A_PWR_MGNT : 0) |
		((fltr_opt->acpt_acrc32) ? BIT_ACRC32 : 0) |
		((fltr_opt->acpt_aicv) ? BIT_AICV : 0) |
		((fltr_opt->chk_ta_mgnt) ? BIT_CHKTA_MGNT : 0) |
		((fltr_opt->acpt_ack_cbssid) ? BIT_ACK_CBSSID : 0);

	*mac_fltr_value = val32;

}

u32 mac_get_rx_fltr_opt(struct mac_ax_adapter *adapter,
			struct mac_ax_rx_fltr_ctrl_t *fltr_opt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	if (!fltr_opt) {
		PLTFM_MSG_ERR("[ERR]%s opt is null\n", __func__);
		return MACNPTR;
	}

	val32 = MAC_REG_R32(REG_RCR);
	fltr_opt->acpt_aap = ((val32 & BIT_AAP) != 0);
	fltr_opt->acpt_apm = ((val32 & BIT_APM) != 0);
	fltr_opt->acpt_am = ((val32 & BIT_AM) != 0);
	fltr_opt->acpt_ab = ((val32 & BIT_AB) != 0);
	fltr_opt->acpt_add3 = ((val32 & BIT_ADD3) != 0);
	fltr_opt->acpt_apwrmgt = ((val32 & BIT_APWRMGT) != 0);
	fltr_opt->chk_cbssid_data = ((val32 & BIT_CBSSID_DATA) != 0);
	fltr_opt->chk_cbssid_mgnt = ((val32 & BIT_CBSSID_MGNT) != 0);
	//fltr_opt->acpt_pwr_mgnt_pkt = ((val32 & B_AX_A_PWR_MGNT) != 0);
	fltr_opt->acpt_acrc32 = ((val32 & BIT_ACRC32) != 0);
	fltr_opt->acpt_aicv = ((val32 & BIT_AICV) != 0);
	fltr_opt->chk_ta_mgnt = ((val32 & BIT_CHKTA_MGNT) != 0);
	fltr_opt->acpt_ack_cbssid = ((val32 & BIT_ACK_CBSSID) != 0);

	return MACSUCCESS;
}

u32 mac_set_rx_fltr_opt(struct mac_ax_adapter *adapter,
			struct mac_ax_rx_fltr_ctrl_t *fltr_opt,
			struct mac_ax_rx_fltr_ctrl_t *fltr_opt_msk)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_rx_fltr_ctrl_t opt = {0};
	u32 mac_opt;
	u32 mac_opt_msk;
	u32 mac_opt_value;
	u32 ret = MACSUCCESS;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return ret;
	}

	if (!fltr_opt || !fltr_opt_msk) {
		PLTFM_MSG_ERR("[ERR]%s opt is null\n", __func__);
		return MACNPTR;
	}

	mac_get_rx_fltr_opt(adapter, &opt);

	rx_fltr_opt_2_uint(adapter,
			   fltr_opt,
			   &mac_opt);

	rx_fltr_opt_2_uint(adapter,
			   fltr_opt_msk,
			   &mac_opt_msk);

	rx_fltr_opt_2_uint(adapter,
			   &opt,
			   &mac_opt_value);

	mac_opt_value = (mac_opt & mac_opt_msk) |
			(~(~mac_opt & mac_opt_msk) & mac_opt_value);


	mac_opt_value &= 0x3FF;
	mac_opt_value |= (0xFFFFC000 & MAC_REG_R32(REG_RCR));

	MAC_REG_W32(REG_RCR, mac_opt_value);

	return ret;
}

u32 mac_set_typ_fltr_opt(struct mac_ax_adapter *adapter,
			 enum mac_ax_pkt_t type,
			 enum mac_ax_fwd_target fwd_target)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u16 val16;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	switch (fwd_target) {
	case MAC_AX_FWD_DONT_ACPT:
		val16 = RX_FLTR_FRAME_DROP;
		break;
	case MAC_AX_FWD_ACPT:
		val16 = RX_FLTR_FRAME_RECV;
		break;
	default:
		PLTFM_MSG_ERR("[ERR]%s fwd target err\n", __func__);
		return MACNOITEM;
	}

	switch (type) {
	case MAC_AX_PKT_MGNT:
		MAC_REG_W16(REG_RXFLTMAP0, val16);
		break;
	case MAC_AX_PKT_CTRL:
		MAC_REG_W16(REG_RXFLTMAP0 + 2, val16);
		break;
	case MAC_AX_PKT_DATA:
		MAC_REG_W16(REG_RXFLTMAP, val16);
		break;
	default:
		PLTFM_MSG_ERR("[ERR]%s type err\n", __func__);
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 mac_set_typsbtyp_fltr_opt(struct mac_ax_adapter *adapter,
			      enum mac_ax_pkt_t type,
			      u8 subtype,
			      enum mac_ax_fwd_target fwd_target)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u16 val16;
	u32 val32;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	if (fwd_target != MAC_AX_FWD_DONT_ACPT &&
	    fwd_target != MAC_AX_FWD_ACPT) {
		PLTFM_MSG_ERR("[ERR]%s fwd target err\n", __func__);
		return MACNOITEM;
	}

	switch (type) {
	case MAC_AX_PKT_MGNT:
		val16 = MAC_REG_R16(REG_RXFLTMAP0);
		val16 |= fwd_target ? BIT(subtype) : 0;
		MAC_REG_W16(REG_RXFLTMAP0, val16);
		break;
	case MAC_AX_PKT_CTRL:
		val16 = MAC_REG_R16(REG_RXFLTMAP0 + 2);
		val16 |= fwd_target ? BIT(subtype) : 0;
		MAC_REG_W16(REG_RXFLTMAP0 + 2, val16);
		break;
	case MAC_AX_PKT_DATA:
		val16 = MAC_REG_R32(REG_RXFLTMAP);
		val16 |= fwd_target ? BIT(subtype) : 0;
		MAC_REG_W16(REG_RXFLTMAP, val16);
		break;
	default:
		PLTFM_MSG_ERR("[ERR]%s type err\n", __func__);
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 mac_set_typsbtyp_fltr_detail(struct mac_ax_adapter *adapter,
				 enum mac_ax_pkt_t type,
				 struct mac_ax_rx_fltr_elem *elem)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 cmp_bit = 1;
	u16 reg, val16;
	u8 idx;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	switch (type) {
	case MAC_AX_PKT_CTRL:
		reg = REG_RXFLTMAP0 + 2;
		break;
	case MAC_AX_PKT_MGNT:
		reg = REG_RXFLTMAP0;
		break;
	case MAC_AX_PKT_DATA:
		reg = REG_RXFLTMAP;
		break;
	default:
		PLTFM_MSG_ERR("[ERR]%s type err\n", __func__);
		return MACNOITEM;
	}

	val16 = MAC_REG_R16(reg);

	for (idx = 0; idx < RX_FLTR_SUBTYPE_NUM; idx++) {
		if ((elem->subtype_mask & cmp_bit) != 0) {
			val16 |=  elem->target_arr[idx] ? BIT(idx) : 0;
		}
		cmp_bit = cmp_bit << ENTRY_IDX;
	}
	MAC_REG_W16(reg, val16);

	return MACSUCCESS;
}
