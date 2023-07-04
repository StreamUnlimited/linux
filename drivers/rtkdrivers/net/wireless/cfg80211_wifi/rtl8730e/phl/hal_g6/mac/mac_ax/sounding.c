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

#include "sounding.h"

u32 mac_get_csi_buffer_index(struct mac_ax_adapter *adapter,
			     u8 csi_buffer_id)
{
	u32 val32, ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}
	val32 = MAC_REG_R32(REG_WMAC_CSIDMA_CFG);
	val32 = BIT_GET_CSI_START_PAGE(val32);

	return val32;
}

u32 mac_set_csi_buffer_index(struct mac_ax_adapter *adapter,
			     u16 csi_buffer_id)
{
	u32 val32, ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_WMAC_CSIDMA_CFG);
	val32 = BIT_SET_CSI_START_PAGE(val32, csi_buffer_id);
	MAC_REG_W32(REG_WMAC_CSIDMA_CFG, val32);

	return MACSUCCESS;
}

u32 mac_init_snd_mee(struct mac_ax_adapter *adapter, u8 band)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	/*AP mode set tx gid to 63*/
	/*STA mode set tx gid to 0(default)*/
	MAC_REG_W32(REG_CSI_RRSR_BITMAP, CSI_RRSC_BMAP);

	val32 = MAC_REG_R32(REG_BEAMFORMING_CTRL);
	val32 |= BIT_WMAC_HE_NDPA_EN;
	MAC_REG_W32(REG_BEAMFORMING_CTRL, val32);

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 |= BIT_R_WMAC_VHT_NDPA_EN | BIT_R_WMAC_HT_NDPA_EN;
	val32 = BIT_SET_NDP_RX_STANDBY_TIMER(val32, NDP_RX_STANDBY_TIMER);
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 |= BIT_R_WMAC_BFPARAM_SEL | BIT_R_WMAC_USE_NSTS;
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_CSI_CTRL);
	val32 |= BIT_WMAC_CSI_RATE_FORCE_EN | BIT_CSI_GID_SEL;
	MAC_REG_W32(REG_CSI_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_csi_force_rate(struct mac_ax_adapter *adapter, u8 ht_rate,
		       u8 vht_rate, u8 he_rate)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 |= BIT_R_WMAC_BFPARAM_SEL;
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_CSI_CTRL);
	val32 |= BIT_WMAC_CSI_RATE_FORCE_EN;
	MAC_REG_W32(REG_CSI_CTRL, val32);

	val32 = MAC_REG_R32(REG_BEAMFORMING_CTRL);
	val32 = BIT_SET_WMAC_HT_CSI_RATE(val32, ht_rate);
	val32 = BIT_SET_WMAC_VHT_CSI_RATE(val32, vht_rate);
	val32 = BIT_SET_WMAC_HE_CSI_RATE(val32, he_rate);
	MAC_REG_W32(REG_BEAMFORMING_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_csi_rrsc(struct mac_ax_adapter *adapter, u32 rrsc)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 |= BIT_R_WMAC_BFPARAM_SEL;
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_CSI_CTRL);
	val32 &= ~BIT_WMAC_CSI_RATE_FORCE_EN;
	MAC_REG_W32(REG_CSI_CTRL, val32);

	MAC_REG_W32(REG_CSI_RRSR_BITMAP, rrsc);

	return MACSUCCESS;
}


u32 mac_set_csi_para_reg(struct mac_ax_adapter *adapter,
			 struct mac_reg_csi_para *csi_para)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret, i;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 |= BIT_R_WMAC_BFPARAM_SEL;
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_SOUNDING_CFG0);
	val32 = (csi_para->nc) | (csi_para->nr << 3) | (csi_para->ng << 6) |
		(csi_para->cb << 8) | (csi_para->cs << 10);
	MAC_REG_W32(REG_SOUNDING_CFG0, val32);

	/*set beamformer's mac address*/
	for (i = 0; i < 6; i++) {
		MAC_REG_W8(REG_ASSOCIATED_BFMER0_INFO + i, csi_para->mac_addr[i]);
	}

	return MACSUCCESS;

}
#if 0
u32 mac_set_csi_para_cctl(struct mac_ax_adapter *adapter,
			  struct mac_cctl_csi_para *csi_para)
{
	struct rtw_hal_mac_ax_cctl_info info;
	struct rtw_hal_mac_ax_cctl_info mask;
	struct mac_ax_ops *ax_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret, val32;

	ret = check_mac_en(adapter, (u8)csi_para->band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}
	/*
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A)) {
		if (is_cv(adapter, CBV)) {
			if (csi_para->ng == 3)
				return MACHWNOSUP;
		}
	} else if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		if (is_cv(adapter, CAV)) {
			if (csi_para->ng == 3)
				return MACHWNOSUP;
		}
	}
	*/
	val32 = MAC_REG_R32(csi_para->band ? R_AX_TRXPTCL_RESP_CSI_CTRL_0_C1 :
			    R_AX_TRXPTCL_RESP_CSI_CTRL_0);
	val32 &= (~B_AX_BFMEE_BFPARAM_SEL);
	MAC_REG_W32(csi_para->band ? R_AX_TRXPTCL_RESP_CSI_CTRL_0_C1 :
		    R_AX_TRXPTCL_RESP_CSI_CTRL_0, val32);

	info.nc = csi_para->nc;
	info.nr = csi_para->nr;
	info.ng = csi_para->ng;
	info.cb = csi_para->cb;
	info.cs = csi_para->cs;
	//info.csi_txbf_en = csi_para->bf_en; BB HW BUG not support
	info.csi_txbf_en = 0x0;
	info.csi_stbc_en = csi_para->stbc_en;
	info.csi_ldpc_en = csi_para->ldpc_en;
	info.csi_para_en = 1;
	info.csi_fix_rate = csi_para->rate;
	info.csi_gi_ltf = csi_para->gi_ltf;
	info.uldl = csi_para->gid_sel;
	info.csi_bw = csi_para->bw;

	PLTFM_MEMSET(&mask, 0, sizeof(mask));
	mask.nc = 0x7;
	mask.nr = 0x7;
	mask.ng = 0x3;
	mask.cb = 0x3;
	mask.cs = 0x3;
	mask.csi_txbf_en = 0x1;
	mask.csi_stbc_en = 0x1;
	mask.csi_ldpc_en = 0x1;
	mask.csi_para_en = 0x1;
	mask.csi_fix_rate = 0x1FF;
	mask.csi_gi_ltf = 0x7;
	mask.uldl = 0x1;
	mask.csi_bw = 0x3;
	ret = ax_ops->upd_cctl_info(adapter, &info, &mask, csi_para->macid, 1);
	if (ret) {
		return MACCCTLWRFAIL;
	}

	return MACSUCCESS;
}
#endif

u32 mac_deinit_mee(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	val32 = MAC_REG_R32(REG_BEAMFORMING_CTRL);
	val32 &= ~BIT_WMAC_HE_NDPA_EN;
	MAC_REG_W32(REG_BEAMFORMING_CTRL, val32);

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	val32 &= ~(BIT_R_WMAC_HT_NDPA_EN | BIT_R_WMAC_VHT_NDPA_EN);
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	return MACSUCCESS;
}

u32 mac_snd_sup(struct mac_ax_adapter *adapter, struct mac_bf_sup *bf_sup)
{
	/*
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A)) {
		bf_sup->bf_entry_num = 16;
		bf_sup->su_buffer_num = 16;
		bf_sup->mu_buffer_num = 6;
	} else if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		bf_sup->bf_entry_num = 16;
		bf_sup->su_buffer_num = 16;
		bf_sup->mu_buffer_num = 6;
	} else if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852C)) {
		bf_sup->bf_entry_num = 16;
		bf_sup->su_buffer_num = 16;
		bf_sup->mu_buffer_num = 6;
	} else {
		return MACNOTSUP;
	}*/
	return MACSUCCESS;
}

u32 mac_gid_pos(struct mac_ax_adapter *adapter, struct mac_gid_pos *mu_gid)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, ret;

	MAC_REG_W32(REG_GID_POS_EN_L, mu_gid->gid_tab[0]);
	MAC_REG_W32(REG_GID_POS_EN_H, mu_gid->gid_tab[1]);

	MAC_REG_W32(REG_GID_POS0, mu_gid->user_pos[0]);
	MAC_REG_W32(REG_GID_POS1, mu_gid->user_pos[1]);
	MAC_REG_W32(REG_GID_POS2, mu_gid->user_pos[2]);
	MAC_REG_W32(REG_GID_POS3, mu_gid->user_pos[3]);

	return MACSUCCESS;
}

