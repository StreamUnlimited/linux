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

#include "twt.h"

u32 twt_info_init(struct mac_ax_adapter *adapter)
{
	adapter->twt_info =
		(struct mac_ax_twt_info *)PLTFM_MALLOC(TWT_INFO_SIZE);
	adapter->twt_info->err_rec = 0;
	adapter->twt_info->pdbg_info = (u8 *)PLTFM_MALLOC(TWT_DBG_INFO_SIZE);
	PLTFM_MEMSET(adapter->twt_info->pdbg_info, 0, TWT_DBG_INFO_SIZE);

	return MACSUCCESS;
}

u32 twt_info_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_FREE(adapter->twt_info->pdbg_info, TWT_DBG_INFO_SIZE);
	PLTFM_FREE(adapter->twt_info, TWT_INFO_SIZE);

	return MACSUCCESS;
}

u32 mac_twt_info_upd_h2c(struct mac_ax_adapter *adapter,
			 struct mac_ax_twt_para *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_twt_act_h2c(struct mac_ax_adapter *adapter,
		    struct mac_ax_twtact_para *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_twt_staanno_h2c(struct mac_ax_adapter *adapter,
			struct mac_ax_twtanno_para *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

void mac_twt_wait_anno(struct mac_ax_adapter *adapter,
		       u8 *c2h_content, u8 *upd_addr)
{
	u32 plat_c2h_content = *(u32 *)(c2h_content);
	struct mac_ax_twtanno_c2hpara *para =
		(struct mac_ax_twtanno_c2hpara *)upd_addr;

	para->wait_case = GET_FIELD(plat_c2h_content,
				    FWCMD_C2H_WAIT_ANNOUNCE_WAIT_CASE);
	para->macid0 = GET_FIELD(plat_c2h_content,
				 FWCMD_C2H_WAIT_ANNOUNCE_MACID0);
	para->macid1 = GET_FIELD(plat_c2h_content,
				 FWCMD_C2H_WAIT_ANNOUNCE_MACID1);
	para->macid2 = GET_FIELD(plat_c2h_content,
				 FWCMD_C2H_WAIT_ANNOUNCE_MACID2);
}
#if 1
u32 mac_get_tsf(struct mac_ax_adapter *adapter, struct mac_ax_port_tsf *tsf)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg_l = 0;
	u32 reg_h = 0;
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret != MACSUCCESS) {
		return ret;
	}

	switch (tsf->port) {
	case MAC_AX_PORT_0:
		reg_h = REG_P0_TSFTR_H;
		reg_l = REG_P0_TSFTR_L;
		break;
	case MAC_AX_PORT_1:
		reg_h = REG_P1_TSFTR_H;
		reg_l = REG_P1_TSFTR_L;
		break;
	case MAC_AX_PORT_2:
		reg_h = REG_P2_TSFTR_H;
		reg_l = REG_P2_TSFTR_L;
		break;
	default:
		return MACFUNCINPUT;
	}

	tsf->tsf_h = MAC_REG_R32(reg_h);
	tsf->tsf_l = MAC_REG_R32(reg_l);

	return MACSUCCESS;
}
#endif

