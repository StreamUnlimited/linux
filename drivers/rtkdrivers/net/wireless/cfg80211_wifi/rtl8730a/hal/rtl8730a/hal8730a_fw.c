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
#include "hal8730a_fw.h"
#include <hal_data.h>		/* HAL_DATA_TYPE */
#include "../hal_halmac.h"	/* HRTW_HALMAC_H2C_MAX_SIZE, CMD_ID_RSVD_PAGE and etc. */
#include "rtl8730a.h"

#ifdef CONFIG_RTL8730A

enum {
	IPC_USER_POINT = 0,
	IPC_USER_DATA = 1
};

int rtl8730a_FirmwareEnable(PADAPTER adapter, u8 enable)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	u32 count = 0;
	u32 fw_ready = 0;
	int  ret = 0;
	ipc_msg_struct_t ipc_msg_temp = {0};

	ipc_msg_temp.msg_type = IPC_USER_POINT;
	ipc_msg_temp.msg_len = 1;

	if (enable) {
		RTW_DBG("Firmware Enable\n");
		ipc_msg_temp.msg = 1;
		ameba_ipc_channel_send(dvobj->fw_ipc, &ipc_msg_temp);
	} else {
		RTW_DBG("Firmware Disable\n");
		ipc_msg_temp.msg = 0;
		ameba_ipc_channel_send(dvobj->fw_ipc, &ipc_msg_temp);
	}

	/*polling for FW ready*/
	do {
		fw_ready = rtw_read32(adapter, REG_WL_MCUFW_CTRL);
		if ((enable && (fw_ready & BIT_ACK))\
		    || (!enable && !(fw_ready & BIT_ACK))) {
			break;
		}
		rtw_udelay_os(100);
	} while (++count < 1000);

	if (count >= 1000) {
		RTW_DBG("FW Enable failed\n");
		ret = -1;
	}

	return ret;
}

#endif