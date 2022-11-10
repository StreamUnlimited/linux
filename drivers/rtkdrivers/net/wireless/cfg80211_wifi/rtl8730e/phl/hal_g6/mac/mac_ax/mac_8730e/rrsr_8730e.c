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
#include "rrsr_8730e.h"
#if MAC_AX_8730E_SUPPORT

u32 mac_get_rrsr_cfg_8730e(struct mac_ax_adapter *adapter,
			   struct mac_ax_rrsr_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	val32 = MAC_REG_R32(REG_RESP_RATE_RRSR_1);
	cfg->rrsr_rate_en = BIT_GET_RESP_RATE_EN(val32);
	cfg->cck_cfg = BIT_GET_RESP_RRSR_CCK(val32);

	val32 = MAC_REG_R32(REG_RESP_CONTROL);
	cfg->ref_rate = BIT_GET_RESP_TXRATE1(val32);
	cfg->ref_rate_sel = (val32 & BIT(9)) ? 1 : 0;

	val32 = MAC_REG_R32(REG_RESP_RATE_RRSR);
	cfg->ofdm_cfg = BIT_GET_RESP_RRSR_OFDM(val32);
	cfg->ht_cfg = BIT_GET_RESP_RRSR_HT(val32);
	cfg->vht_cfg = BIT_GET_RESP_RRSR_VHT(val32);
	cfg->he_cfg = BIT_GET_RESP_RRSR_HE(val32);

	val32 = MAC_REG_R32(REG_RRSR);
	cfg->rsc = BIT_GET_RRSR_RSC(val32);

	val32 = MAC_REG_R32(REG_RESP_CONTROL);
	cfg->dcm_en = (val32 & BIT_RESP_DCM_EN) ? 1 : 0;

	val32 = MAC_REG_R32(REG_RESP_CONTROL_1);
	cfg->doppler_sel = (val32 & BIT_RESP_DOPPLER_SEL) ? 1 : 0;
	cfg->doppler_ctrl = BIT_GET_RESP_DOPPLER_CTRL(val32);

	return MACSUCCESS;
}

u32 mac_set_rrsr_cfg_8730e(struct mac_ax_adapter *adapter,
			   struct mac_ax_rrsr_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 val32_dcm;


	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	val32 = MAC_REG_R32(REG_RESP_RATE_RRSR_1);
	val32 = BIT_SET_RESP_RATE_EN(val32, cfg->rrsr_rate_en);
	val32 = BIT_SET_RESP_RRSR_CCK(val32, cfg->cck_cfg);
	MAC_REG_W32(REG_RESP_RATE_RRSR_1, val32);

	val32 = MAC_REG_R32(REG_RESP_CONTROL);
	val32 = BIT_SET_RESP_TXRATE1(val32, cfg->ref_rate);
	if (cfg->ref_rate_sel) {
		val32 |= BIT(9);
	} else {
		val32 &= ~BIT(9);
	}
	MAC_REG_W32(REG_RESP_CONTROL, val32);

	val32 = MAC_REG_R32(REG_RESP_RATE_RRSR);
	val32 = BIT_SET_RESP_RRSR_OFDM(val32, cfg->ofdm_cfg);
	val32 = BIT_SET_RESP_RRSR_HT(val32, cfg->ht_cfg);
	val32 = BIT_SET_RESP_RRSR_VHT(val32, cfg->vht_cfg);
	val32 = BIT_SET_RESP_RRSR_HE(val32, cfg->he_cfg);
	MAC_REG_W32(REG_RESP_RATE_RRSR, val32);

	/*doppler & dcm*/
	val32 = MAC_REG_R32(REG_RESP_CONTROL_1);
	val32_dcm = MAC_REG_R32(REG_RESP_CONTROL);

	val32 = BIT_SET_RESP_DOPPLER_CTRL(val32, cfg->doppler_ctrl);
	if (cfg->doppler_sel) {
		val32 |= BIT_RESP_DOPPLER_SEL;
	} else {
		val32 &= ~BIT_RESP_DOPPLER_SEL;
	}


	if (cfg->dcm_sel) {
		val32 |= BIT_RESP_DCM_SEL;
		MAC_REG_W32(REG_RESP_CONTROL_1, val32);
		if (cfg->dcm_en) {
			MAC_REG_W32(REG_RESP_CONTROL, val32_dcm | BIT_RESP_DCM_EN);
		} else {
			MAC_REG_W32(REG_RESP_CONTROL, val32_dcm & ~BIT_RESP_DCM_EN);
		}
	} else {
		val32 &= ~BIT_RESP_DCM_SEL;
		MAC_REG_W32(REG_RESP_CONTROL_1, val32);
	}

	val32 = MAC_REG_R32(REG_RRSR);
	val32 = BIT_SET_RRSR_RSC(val32, cfg->rsc);
	MAC_REG_W32(REG_RRSR, val32);

	return MACSUCCESS;
}

u32 mac_get_csi_rrsr_cfg_8730e(struct mac_ax_adapter *adapter,
			       struct mac_ax_csi_rrsr_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	cfg->csi_rrsr_opt = (val32 & BIT_R_WMAC_BFPARAM_SEL) ? 1 : 0;

	val32 = MAC_REG_R32(REG_CSI_CTRL);
	cfg->rsc = BIT_GET_WMAC_CSI_RSC_FORCE(val32);

	val32 = MAC_REG_R32(REG_BEAMFORMING_CTRL);
	cfg->csi_rrsr_ht_cfg = BIT_GET_WMAC_HT_CSI_RATE(val32);
	cfg->csi_rrsr_vht_cfg = BIT_GET_WMAC_VHT_CSI_RATE(val32);
	cfg->csi_rrsr_he_cfg = BIT_GET_WMAC_HE_CSI_RATE(val32);

	return MACSUCCESS;
}

u32 mac_set_csi_rrsr_cfg_8730e(struct mac_ax_adapter *adapter,
			       struct mac_ax_csi_rrsr_cfg *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, val32_rate;

	val32 = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (val32 != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]%s MAC not enable\n", __func__);
		return val32;
	}

	val32 = MAC_REG_R32(REG_SND_PTCL_CTRL);
	if (cfg->csi_rrsr_opt) {
		val32 |= BIT_R_WMAC_BFPARAM_SEL;
	} else {
		val32 &= ~BIT_R_WMAC_BFPARAM_SEL;
	}
	MAC_REG_W32(REG_SND_PTCL_CTRL, val32);

	val32 = MAC_REG_R32(REG_CSI_CTRL);
	if (cfg->csi_force_en) {
		val32 |= BIT_WMAC_CSI_RATE_FORCE_EN;

		val32_rate = MAC_REG_R32(REG_BEAMFORMING_CTRL);
		val32_rate = BIT_SET_WMAC_HT_CSI_RATE(val32_rate, cfg->csi_rrsr_ht_cfg);
		val32_rate = BIT_SET_WMAC_VHT_CSI_RATE(val32_rate, cfg->csi_rrsr_vht_cfg);
		val32_rate = BIT_SET_WMAC_HE_CSI_RATE(val32_rate, cfg->csi_rrsr_he_cfg);
		MAC_REG_W32(REG_BEAMFORMING_CTRL, val32_rate);
	} else {
		val32 &= ~BIT_WMAC_CSI_RATE_FORCE_EN;
	}

	MAC_REG_W32(REG_CSI_CTRL, val32);


	val32 = (cfg->csi_rrsr_ofdm_mask) | (cfg->csi_rrsr_ht_mask << 8) |
		(cfg->csi_rrsr_vht_mask << 16) | (cfg->csi_rrsr_he_mask << 24);
	MAC_REG_W32(REG_CSI_RRSR_BITMAP, val32);


	val32 = MAC_REG_R32(REG_CSI_CTRL);
	val32 = BIT_SET_WMAC_CSI_RSC_FORCE(val32, cfg->rsc);
	MAC_REG_W32(REG_CSI_CTRL, val32);

	return MACSUCCESS;
}

#endif /* #if MAC_AX_8852A_SUPPORT */
