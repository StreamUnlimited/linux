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

#include "common.h"

u8 shift_mask(u32 mask)
{
	u8 i;

	for (i = 0; i < 32; i++) {
		if ((mask >> i) & BIT0) {
			break;
		}
	}

	return i;
}

u32 mac_watchdog(struct mac_ax_adapter *adapter,
		 struct mac_ax_wdt_param *wdt_param)
{
	struct mac_ax_tsf_sync_info *sync_info = &adapter->tsf_sync_info;
	u32 ret = MACSUCCESS;

	if (sync_info->en_auto_sync) {
		return adapter->ops->tsf_sync(adapter);
	}

	return ret;
}