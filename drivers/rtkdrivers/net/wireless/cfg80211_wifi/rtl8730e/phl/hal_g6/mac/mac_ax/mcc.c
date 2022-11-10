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

#include "mcc.h"

u32 mac_reset_mcc_group(struct mac_ax_adapter *adapter, u8 group)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_reset_mcc_request(struct mac_ax_adapter *adapter, u8 group)
{
	if (adapter->sm.mcc_request[group] != MAC_AX_MCC_REQ_FAIL) {
		PLTFM_MSG_ERR("[ERR]%s: state != req fail\n", __func__);
		return MACPROCERR;
	}

	if (group > MCC_GROUP_ID_MAX) {
		PLTFM_MSG_ERR("[ERR]%s: invalid group: %d\n", __func__, group);
		return MACNOITEM;
	}

	adapter->sm.mcc_request[group] = MAC_AX_MCC_REQ_IDLE;

	return MACSUCCESS;
}

u32 mac_add_mcc(struct mac_ax_adapter *adapter, struct mac_ax_mcc_role *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_start_mcc(struct mac_ax_adapter *adapter,
		  struct mac_ax_mcc_start *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_stop_mcc(struct mac_ax_adapter *adapter, u8 group, u8 macid,
		 u8 prev_groups)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_del_mcc_group(struct mac_ax_adapter *adapter, u8 group,
		      u8 prev_groups)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_mcc_request_tsf(struct mac_ax_adapter *adapter, u8 group,
			u8 macid_x, u8 macid_y)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_mcc_macid_bitmap(struct mac_ax_adapter *adapter, u8 group,
			 u8 macid, u8 *bitmap, u8 len)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_mcc_sync_enable(struct mac_ax_adapter *adapter, u8 group,
			u8 source, u8 target, u8 offset)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_mcc_set_duration(struct mac_ax_adapter *adapter,
			 struct mac_ax_mcc_duration_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_get_mcc_status_rpt(struct mac_ax_adapter *adapter,
			   u8 group, u8 *status, u32 *tsf_high, u32 *tsf_low)
{
	struct mac_ax_mcc_group_info *mcc_info = &adapter->mcc_group_info;

	if (adapter->sm.mcc_group[group] == MAC_AX_MCC_EMPTY) {
		PLTFM_MSG_ERR("[ERR]%s: state = empty\n", __func__);
		return MACPROCERR;
	}

	if (group > MCC_GROUP_ID_MAX) {
		PLTFM_MSG_ERR("[ERR]%s: invalid group: %d\n", __func__, group);
		return MACNOITEM;
	}

	*status = mcc_info->groups[group].rpt_status;
	PLTFM_MSG_TRACE("[TRACE]%s: mcc status: %d\n", __func__, *status);

	*tsf_high = mcc_info->groups[group].rpt_tsf_high;
	*tsf_low = mcc_info->groups[group].rpt_tsf_low;

	PLTFM_MSG_TRACE("[TRACE]%s: report tsf_high: 0x%x\n",
			__func__, *tsf_high);
	PLTFM_MSG_TRACE("[TRACE]%s: report tsf_low: 0x%x\n",
			__func__, *tsf_low);

	return MACSUCCESS;
}

u32 mac_get_mcc_tsf_rpt(struct mac_ax_adapter *adapter, u8 group,
			u32 *tsf_x_high, u32 *tsf_x_low,
			u32 *tsf_y_high, u32 *tsf_y_low)
{
	struct mac_ax_mcc_group_info *mcc_info = &adapter->mcc_group_info;

	if (group > MCC_GROUP_ID_MAX) {
		PLTFM_MSG_ERR("[ERR]%s: invalid group: %d\n", __func__, group);
		return MACNOITEM;
	}

	*tsf_x_high = mcc_info->groups[group].tsf_x_high;
	*tsf_x_low = mcc_info->groups[group].tsf_x_low;
	PLTFM_MSG_TRACE("[TRACE]%s: tsf_x_high: 0x%x\n", __func__, *tsf_x_high);
	PLTFM_MSG_TRACE("[TRACE]%s: tsf_x_low: 0x%x\n", __func__, *tsf_x_low);

	*tsf_y_high = mcc_info->groups[group].tsf_y_high;
	*tsf_y_low += mcc_info->groups[group].tsf_y_low;
	PLTFM_MSG_TRACE("[TRACE]%s: tsf_y_high: 0x%x\n", __func__, *tsf_y_high);
	PLTFM_MSG_TRACE("[TRACE]%s: tsf_y_low: 0x%x\n", __func__, *tsf_y_low);

	adapter->sm.mcc_request[group] = MAC_AX_MCC_REQ_IDLE;

	return MACSUCCESS;
}

u32 mac_get_mcc_group(struct mac_ax_adapter *adapter, u8 *pget_group)
{
	struct mac_ax_state_mach *sm = &adapter->sm;
	u8 group_idx;

	for (group_idx = 0; group_idx <= MCC_GROUP_ID_MAX; group_idx++) {
		if (sm->mcc_group[group_idx] == MAC_AX_MCC_EMPTY) {
			*pget_group = group_idx;
			PLTFM_MSG_TRACE("[TRACE]%s: get mcc empty group %u\n",
					__func__, *pget_group);
			return MACSUCCESS;
		}
	}
	return MACMCCGPFL;
}

u32 mac_check_add_mcc_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr state: %d\n", __func__,
			group, adapter->sm.mcc_group[group]);

	if (adapter->sm.mcc_group[group] == MAC_AX_MCC_ADD_DONE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_start_mcc_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr state: %d\n", __func__,
			group, adapter->sm.mcc_group[group]);

	if (adapter->sm.mcc_group[group] == MAC_AX_MCC_START_DONE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_stop_mcc_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr state: %d\n", __func__,
			group, adapter->sm.mcc_group[group]);

	if (adapter->sm.mcc_group[group] == MAC_AX_MCC_STOP_DONE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_del_mcc_group_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr state: %d\n", __func__,
			group, adapter->sm.mcc_group[group]);

	if (adapter->sm.mcc_group[group] == MAC_AX_MCC_EMPTY) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_mcc_request_tsf_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr req state: %d\n", __func__,
			group, adapter->sm.mcc_request[group]);

	if (adapter->sm.mcc_request[group] == MAC_AX_MCC_REQ_DONE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_mcc_macid_bitmap_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr req state: %d\n", __func__,
			group, adapter->sm.mcc_request[group]);

	if (adapter->sm.mcc_request[group] == MAC_AX_MCC_REQ_IDLE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_mcc_sync_enable_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr req state: %d\n", __func__,
			group, adapter->sm.mcc_request[group]);

	if (adapter->sm.mcc_request[group] == MAC_AX_MCC_REQ_IDLE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

u32 mac_check_mcc_set_duration_done(struct mac_ax_adapter *adapter, u8 group)
{
	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr req state: %d\n", __func__,
			group, adapter->sm.mcc_request[group]);

	if (adapter->sm.mcc_request[group] == MAC_AX_MCC_REQ_IDLE) {
		return MACSUCCESS;
	} else {
		return MACPROCBUSY;
	}
}

