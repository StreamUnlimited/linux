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

#include "trxcfg.h"
#include "mac_priv.h"
#if 1
u32 check_mac_en(struct mac_ax_adapter *adapter,
		 enum mac_ax_hwmod_sel sel)
{
	if (adapter->sm.pwr != MAC_AX_PWR_ON) {
		return MACPROCERR;
	}

	if (sel == MAC_AX_MAC_SEL) {
		if (adapter->sm.mac_func != MAC_AX_FUNC_ON) {
			return MACIOERRDMAC;
		}
	} else {
		PLTFM_MSG_ERR("[ERR] mac sel: %d\n", sel);
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 mac_resp_chk_cca(struct mac_ax_adapter *adapter,
		     struct mac_ax_resp_chk_cca *cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	if (cfg->resp_ack_chk_cca_en) {
		val32 = MAC_REG_R32(REG_WMAC_TRXPTCL_CTL);
		val32 |= BIT_DIS_RESP_ACKINCCA;
		MAC_REG_W32(REG_WMAC_TRXPTCL_CTL, val32);
	} else {
		val32 = MAC_REG_R32(REG_WMAC_TRXPTCL_CTL);
		val32 &= ~BIT_DIS_RESP_ACKINCCA;
		MAC_REG_W32(REG_WMAC_TRXPTCL_CTL, val32);
	}

	return MACSUCCESS;
}

u32 mac_sifs_chk_cca_en(struct mac_ax_adapter *adapter, u8 en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}
	if (en) {
		val32 = MAC_REG_R32(REG_CCA_NAV_CHK);
		val32 |= (BIT_CTN_CHK_CCA_S40 | BIT_CTN_CHK_CCA_S20 |
			  BIT_CTN_CHK_EDCCA);
		MAC_REG_W32(REG_CCA_NAV_CHK, val32);
	} else {
		val32 = MAC_REG_R32(REG_CCA_NAV_CHK);
		val32 &= ~(BIT_CTN_CHK_CCA_S40 | BIT_CTN_CHK_CCA_S20 |
			   BIT_CTN_CHK_EDCCA);
		MAC_REG_W32(REG_CCA_NAV_CHK, val32);
	}

	return MACSUCCESS;
}
#endif
u32 dmac_init(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info,
	      enum mac_ax_band band)
{
	u32 ret = 0;
	struct mac_ax_priv_ops *p_ops;



	p_ops = adapter_to_priv_ops(adapter);
	ret = p_ops->sec_info_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sec info tbl init %d\n", ret);
		return ret;
	}

	return ret;
}
#if 1
u32 mac_cfg_sec(struct mac_ax_adapter *adapter,
		struct halmac_security_setting *set)
{
	u32 ret = MACSUCCESS;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 sec_cfg, val8;

	MAC_REG_W16(REG_CR, MAC_REG_R16(REG_CR) | BIT_MAC_SEC_EN);

	val8 = MAC_REG_R8(REG_SECCFG + 1);
	if (set->compare_keyid == 1) {
		MAC_REG_W8(REG_SECCFG + 1, val8 | BIT(0));
	} else {
		MAC_REG_W8(REG_SECCFG + 1, val8 & ~BIT(0));
	}
	sec_cfg = MAC_REG_R8(REG_SECCFG);
	/* BC/MC uses default key */
	/* cam entry 0~3, kei id = 0 -> entry0, kei id = 1 -> entry1... */
	sec_cfg |= (BIT_TXBCUSEDK | BIT_RXBCUSEDK);
	if (set->tx_encryption == 1) {
		sec_cfg |= BIT_TXENC;
	} else {
		sec_cfg &= ~BIT_TXENC;
	}

	if (set->rx_decryption == 1) {
		sec_cfg |= BIT_RXDEC;
	} else {
		sec_cfg &= ~BIT_RXDEC;
	}

	MAC_REG_W8(REG_SECCFG, sec_cfg);

	return MACSUCCESS;
}

u32 mac_trx_init(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info)
{
	u32 ret = MACSUCCESS;
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_rrsr_cfg rrsr_cfg = {0};

	ret = p_ops->init_trx_cfg(adapter, info->mode);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]init trx %x\n", ret);
		return ret;
	}
	ret = p_ops->init_protocol_cfg(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]init ptcl %x\n", ret);
		return ret;
	}

	ret = p_ops->init_edca_cfg(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]init edca %x\n", ret);
		return ret;
	}

	ret = p_ops->init_wmac_cfg(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]init wmac %x\n", ret);
		return ret;
	}

	rrsr_cfg.rrsr_rate_en = 1;
	rrsr_cfg.cck_cfg = 0xF;
	rrsr_cfg.ref_rate_sel = 1;
	rrsr_cfg.ref_rate = 0x387;
	rrsr_cfg.ofdm_cfg = 0xFF;
	rrsr_cfg.ht_cfg = 0xFF;
	rrsr_cfg.vht_cfg = 0xFF;
	rrsr_cfg.he_cfg = 0xFF;
	ret = p_ops->set_rrsr_cfg(adapter, &rrsr_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]init wmac %x\n", ret);
		return ret;
	}

	adapter->sm.mac_func = MAC_AX_FUNC_ON;

	ret = p_ops->sec_info_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sec info tbl init %d\n", ret);
		return ret;
	}

	ret = rst_port_info(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]MAC rst port info %d\n", ret);
		return ret;
	}

	ret = rx_fltr_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]MAC RX filter init %d\n", ret);
		return ret;
	}

	ret = spatial_reuse_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]MAC Spatial Reuse init %d\n", ret);
		return ret;
	}

	/*
		ret = mac_err_imr_ctrl(adapter, MAC_AX_FUNC_EN);
		if (ret) {
			PLTFM_MSG_ERR("[ERR] enable err IMR %d\n", ret);
			return ret;
		}

		ret = set_host_rpr(adapter, info->rpr_cfg);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]set host rpr %d\n", ret);
			return ret;
		}
		*/
	ret = set_l2_status(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]%s %d\n", __func__, ret);
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_two_nav_cfg(struct mac_ax_adapter *adapter,
		    struct mac_ax_2nav_info *info)

{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;
	u8 val8 = 0;

	val8 = MAC_REG_R32(REG_RX_NAV_CTRL);
	val8 |= ((info->plcp_upd_nav_en ? BIT_WMAC_PLCP_UP_NAV_EN : 0)) |
		((info->tgr_fram_upd_nav_en ? BIT_WMAC_TF_UP_NAV_EN : 0));

	MAC_REG_W8(REG_RX_NAV_CTRL, val8);

	return MACSUCCESS;
}
#endif


