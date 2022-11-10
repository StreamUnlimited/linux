/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#include "halmac_hwimg_8730e.h"
#include "halmac_hwimg_raw_data_8730e.h"

#if MAC_AX_8730E_SUPPORT

void init_pwr_reg_8730e(struct mac_ax_adapter *adapter,  bool is_form_folder,
			u32 folder_len, u32 *folder_array)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32	i = 0;
	u32	array_len = 0;
	u32	*array = NULL;
	u32	v1 = 0, v2 = 0;

	PLTFM_MSG_ALWAYS("enter init mac pwr reg...\n");
	if (is_form_folder) {
		array_len = folder_len;
		array = folder_array;
	} else {
		array_len =
		sizeof(array_mp_8730e_pwr_reg_pxp) / sizeof(u32);
		array = (u32 *)array_mp_8730e_pwr_reg_pxp;
	}

	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];

		MAC_REG_W32(v1, v2);
		i = i + 2;
	}
	PLTFM_MSG_ALWAYS("finish init mac pwr reg...\n");
}

#endif /* MAC_AX_8730E_SUPPORT */

