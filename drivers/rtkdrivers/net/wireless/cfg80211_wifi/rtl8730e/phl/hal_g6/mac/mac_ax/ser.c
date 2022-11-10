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

#include "ser.h"
#include "mac_priv.h"

#define DBG_SENARIO_SH 28

#define L0_TO_L1_EVENT_SH 28
#define L0_TO_L1_EVENT_MSK 0xF

enum mac_ax_l0_to_l1_event {
	MAC_AX_L0_TO_L1_CHIF_IDLE = 0,
	MAC_AX_L0_TO_L1_CMAC_DMA_IDLE = 1,
	MAC_AX_L0_TO_L1_RLS_PKID = 2,
	MAC_AX_L0_TO_L1_PTCL_IDLE = 3,
	MAC_AX_L0_TO_L1_RX_QTA_LOST = 4,
	MAC_AX_L0_TO_L1_DLE_STAT_HANG = 5,
	MAC_AX_L0_TO_L1_PCIE_STUCK = 6,
	MAC_AX_L0_TO_L1_EVENT_MAX = 15,
};

#if 0
u32 mac_chk_err_status(struct mac_ax_adapter *adapter, u8 *ser_status)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 cnt = MAC_SET_ERR_DLY_CNT;
	u32 ret = MACSUCCESS;
	enum mac_ax_err_info err;

	PLTFM_MUTEX_LOCK(&adapter->hw_info->err_get_lock);
	adapter->sm.l2_st = MAC_AX_L2_DIS;

	while (--cnt) {
		if (MAC_REG_R32(R_AX_HALT_C2H_CTRL)) {
			break;
		}
		PLTFM_DELAY_US(MAC_SET_ERR_DLY_US);
	}
	if (!cnt) {
		PLTFM_MSG_ERR("Polling FW err status fail\n");
		ret = MACPOLLTO;
		goto end;
	}

	err = (enum mac_ax_err_info)MAC_REG_R32(R_AX_HALT_C2H);

	switch (err) {
	case MAC_AX_ERR_L1_ERR_DMAC:
	case MAC_AX_ERR_L0_PROMOTE_TO_L1:
		*ser_status = MAC_AX_L1_TRUE;
		break;
	default:
		*ser_status = MAC_AX_L1_FALSE;
		break;
	}

end:
	adapter->sm.l2_st = MAC_AX_L2_EN;
	PLTFM_MUTEX_UNLOCK(&adapter->hw_info->err_get_lock);
	return ret;
}
#endif
