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

#include "p2p.h"

static u32 p2p_info_size = sizeof(struct mac_ax_p2p_info) * P2P_MAX_NUM;

static u32 _get_valid_p2pid(struct mac_ax_adapter *adapter, u8 macid, u8 *p2pid)
{
	struct mac_ax_p2p_info *info = adapter->p2p_info;
	u8 idx;

	for (idx = 0; idx < P2P_MAX_NUM; idx++) {
		if (info[idx].run && info[idx].macid == macid) {
			PLTFM_MSG_ERR("[ERR]macid %d has running p2pid %d\n",
				      macid, idx);
			*p2pid = P2PID_INVALID;
			return MACSUCCESS;
		}
	}

	for (idx = 0; idx < P2P_MAX_NUM; idx++) {
		if (!info[idx].run && !info[idx].wait_dack) {
			*p2pid = idx;
			return MACSUCCESS;
		}
	}

	PLTFM_MSG_ERR("[ERR]no valid p2p\n");
	*p2pid = P2PID_INVALID;
	return MACHWNOSUP;
}

static u32 _get_macid_p2pid(struct mac_ax_adapter *adapter, u8 macid, u8 *p2pid)
{
	struct mac_ax_p2p_info *info = adapter->p2p_info;
	u8 idx;

	for (idx = 0; idx < P2P_MAX_NUM; idx++) {
		if (info[idx].run && info[idx].macid == macid) {
			*p2pid = idx;
			return MACSUCCESS;
		}
	}

	PLTFM_MSG_ERR("[ERR]no valid p2pid for macid %d\n", macid);
	*p2pid = P2PID_INVALID;
	return MACFUNCINPUT;
}

u32 get_wait_dack_p2pid(struct mac_ax_adapter *adapter, u8 *p2pid)
{
	struct mac_ax_p2p_info *info = adapter->p2p_info;
	u8 idx;
	u8 hit = 0;

	for (idx = 0; idx < P2P_MAX_NUM; idx++) {
		if (info[idx].wait_dack && !hit) {
			*p2pid = idx;
			hit = 1;
		} else if (info[idx].wait_dack) {
			PLTFM_MSG_ERR("[ERR]multiple wait dack p2p\n");
			return MACPROCERR;
		}
	}

	if (!hit) {
		*p2pid = P2PID_INVALID;
	}

	return MACSUCCESS;
}

u32 p2p_info_init(struct mac_ax_adapter *adapter)
{
	u32 i;

	adapter->p2p_info =
		(struct mac_ax_p2p_info *)PLTFM_MALLOC(p2p_info_size);
	for (i = 0; i < P2P_MAX_NUM; i++)
		PLTFM_MEMSET(&adapter->p2p_info[i], 0,
			     sizeof(struct mac_ax_p2p_info));

	return MACSUCCESS;
}

u32 p2p_info_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_FREE(adapter->p2p_info, p2p_info_size);

	return MACSUCCESS;
}

u32 rst_p2p_info(struct mac_ax_adapter *adapter)
{
	PLTFM_MEMSET(adapter->p2p_info, 0, p2p_info_size);

	return MACSUCCESS;
}

u32 mac_p2p_act_h2c(struct mac_ax_adapter *adapter,
		    struct mac_ax_p2p_act_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_p2p_macid_ctrl_h2c(struct mac_ax_adapter *adapter,
			   struct mac_ax_p2p_macid_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_get_p2p_stat(struct mac_ax_adapter *adapter)
{
	switch (adapter->sm.p2p_stat) {
	case MAC_AX_P2P_ACT_IDLE:
		return MACSUCCESS;
	case MAC_AX_P2P_ACT_BUSY:
		return MACPROCBUSY;
	case MAC_AX_P2P_ACT_FAIL:
	default:
		return MACP2PSTFAIL;
	}
}

