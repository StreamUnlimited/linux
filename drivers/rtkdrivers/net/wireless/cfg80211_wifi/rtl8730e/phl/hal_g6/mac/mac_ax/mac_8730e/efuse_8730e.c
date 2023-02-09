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
#include "efuse_8730e.h"

#if MAC_AX_8730E_SUPPORT

u32 efuse_read_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 *data, int len)
{
	u32 ret = MACSUCCESS;
	otp_ipc_host_req_t otp_req = {0};
	u32 max_size = adapter->hw_info->efuse_size;
	int retry = 100;

	if (addr + len >= max_size) {
		*data = 0xFF;
		return MACEFUSEADDR;
	}

	otp_req.otp_id = LINUX_IPC_OTP_PHY_READ8;
	otp_req.addr = addr;
	otp_req.len = len;

	ret = rtk_otp_process(&otp_req, data);
	while ((retry > 0) && (ret == -EBUSY)) {
		ret = rtk_otp_process(&otp_req, data);
		retry--;
	}

	ret = (ret == 1) ? MACSUCCESS : MACEFUSEREAD;

	return ret;
}

u32 efuse_write_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 data, int len)
{
	u32 ret = MACSUCCESS;
	otp_ipc_host_req_t otp_req = {0};
	u32 max_size = adapter->hw_info->efuse_size;

	if (addr + len>= max_size) {
		return MACEFUSEADDR;
	}

	if (data == 0xFF) {
		return MACSUCCESS;
	}

	otp_req.otp_id = LINUX_IPC_OTP_PHY_WRITE8;
	otp_req.addr = addr;
	otp_req.len = len;
	otp_req.write_lock = 1;
	if (rtk_otp_process(&otp_req, &data) != 1) {
		return MACEFUSEWRITE;
	}

	return ret;
}

u32 efuse_read8_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 *data)
{
	u32 ret = MACSUCCESS;
	otp_ipc_host_req_t otp_req = {0};
	u32 max_size = adapter->hw_info->efuse_size;

	if (addr >= max_size) {
		*data = 0xFF;
		return MACEFUSEADDR;
	}

	otp_req.otp_id = LINUX_IPC_OTP_PHY_READ8;
	otp_req.addr = addr;
	otp_req.len = 1;
	if (rtk_otp_process(&otp_req, data) != 1) {
		return MACEFUSEREAD;
	}

	return ret;
}

u32 efuse_write8_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 data)
{
	u32 ret = MACSUCCESS;
	otp_ipc_host_req_t otp_req = {0};
	u32 max_size = adapter->hw_info->efuse_size;

	if (addr >= max_size) {
		return MACEFUSEADDR;
	}

	if (data == 0xFF) {
		return MACSUCCESS;
	}

	otp_req.otp_id = LINUX_IPC_OTP_PHY_WRITE8;
	otp_req.addr = addr;
	otp_req.len = 1;
	otp_req.write_lock = 1;
	if (rtk_otp_process(&otp_req, &data) != 1) {
		return MACEFUSEWRITE;
	}

	return ret;
}

/* Not used now, using dump_log_efuse_8730e. */
u32 read_log_efuse_8730e(struct mac_ax_adapter *adapter, u32 addr,
			 u32 size, u8 *val)
{
	u32 ret = MACSUCCESS;
	otp_ipc_host_req_t otp_req = {0};
	u32 max_size = adapter->hw_info->log_efuse_size;
	u8 *tmp_map = NULL;
	int retry = 100;

	tmp_map = (u8 *)PLTFM_MALLOC(1024);
	if (!tmp_map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		return MACNOBUF;
	}

	if (size >= 1024) {
		return MACEFUSESIZE;
	}

	if (addr >= max_size) {
		return MACEFUSEADDR;
	}

	otp_req.otp_id = LINUX_IPC_OTP_LOGI_READ_MAP;
	otp_req.len = 1024;

	ret = rtk_otp_process(&otp_req, tmp_map);
	while (retry && (ret == -EBUSY)) {
		ret = rtk_otp_process(&otp_req, tmp_map);
		retry--;
	}

	ret = (ret == 1) ? MACSUCCESS : MACEFUSEREAD;

	PLTFM_MEMCPY(val, tmp_map + addr, size);
	return ret;
}

u32 write_log_efuse_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 val)
{
	otp_ipc_host_req_t otp_req = {0};

	otp_req.otp_id = LINUX_IPC_OTP_LOGI_WRITE_MAP;
	otp_req.addr = addr;
	otp_req.len = 1;
	otp_req.write_lock = 1;
	if (rtk_otp_process(&otp_req, &val) != 1) {
		return MACEFUSEWRITE;
	}

	return MACSUCCESS;
}

u32 dump_log_efuse_8730e(struct mac_ax_adapter *adapter, u8 *efuse_map)
{
	struct rtw_hal_com_t *hal = (struct rtw_hal_com_t *)(adapter->drv_adapter);
	struct hal_info_t *hal_info = (struct hal_info_t *)(hal->hal_priv);
	struct efuse_t *efuse_info = (struct efuse_t *)(hal_info->efuse);
	u8 *tmp_map = NULL;
	otp_ipc_host_req_t otp_req = {0};
	u32 efuse_size = efuse_info->log_efuse_size;
	u32 ret = MACSUCCESS;
	int retry = 100;

	tmp_map = (u8 *)PLTFM_MALLOC(1024);
	if (!tmp_map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		return MACNOBUF;
	}

	otp_req.otp_id = LINUX_IPC_OTP_LOGI_READ_MAP;
	otp_req.len = 1024;
	ret = rtk_otp_process(&otp_req, tmp_map);
	while ((retry > 0) && (ret == -EBUSY)) {
		ret = rtk_otp_process(&otp_req, tmp_map);
		retry--;
	}

	ret = (ret == 1) ? MACSUCCESS : MACEFUSEREAD;

	if (efuse_size > 1024) {
		PLTFM_MEMCPY(efuse_map, tmp_map, 1024);
	} else {
		PLTFM_MEMCPY(efuse_map, tmp_map, efuse_size);
	}

	return ret;
}

#endif /* MAC_AX_8730E_SUPPORT */