/******************************************************************************
*
* Copyright(c) 2012 - 2017 Realtek Corporation.
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
#include "hal8730e_fw.h"
#include <ameba_ipc/ameba_ipc.h>
#include "../../type.h"
#include "mac/mac_ax/state_mach.h"

#ifdef CONFIG_RTL8730E

enum {
	IPC_USER_POINT = 0,
	IPC_USER_DATA = 1
};

static u32 rtl8730e_fw_enable(struct mac_ax_adapter *adapter, u8 enable)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_fw_info *fw = &adapter->fw_info;
	struct rtw_hal_com_t *hal = (struct rtw_hal_com_t *)(adapter->drv_adapter);
	struct dvobj_priv *dvobj = (struct dvobj_priv *)(hal->drv_priv);
	PAXI_DATA axi_data = dvobj_to_axi(dvobj);
	u32 count = 0;
	u32 fw_ready = 0;
	u32  ret = MACSUCCESS;
	ipc_msg_struct_t ipc_msg_temp = {0};

	ipc_msg_temp.msg_type = IPC_USER_POINT;
	ipc_msg_temp.msg_len = 1;

	if (enable) {
		PLTFM_MSG_TRACE("Firmware Enable\n");
		ipc_msg_temp.msg = 1;
		ameba_ipc_channel_send(axi_data->fw_ipc, &ipc_msg_temp);
	} else {
		PLTFM_MSG_TRACE("Firmware Disable\n");
		ipc_msg_temp.msg = 0;
		ameba_ipc_channel_send(axi_data->fw_ipc, &ipc_msg_temp);
	}

	/*polling for FW ready*/
	do {
		fw_ready = MAC_REG_R32(REG_WL_MCUFW_CTRL);
		if ((enable && (fw_ready & BIT6))\
		    || (!enable && !(fw_ready & BIT6))) {
			break;
		}
		rtw_udelay_os(100);
	} while (++count < 1000);

	if (count >= 1000) {
		PLTFM_MSG_TRACE("FW Enable failed\n");
		ret = MACFWSTATEERR;
	} else {
		if (enable) {
			adapter->sm.fwdl = MAC_AX_FWDL_INIT_RDY;
		} else {
			adapter->sm.fwdl = MAC_AX_FWDL_IDLE;
			fw->last_hmebox_num = 0;
		}
	}

	return ret;
}

u32 enable_fw_8730e(struct mac_ax_adapter *adapter, u8 enable)
{
	return rtl8730e_fw_enable(adapter, enable);
}

#endif