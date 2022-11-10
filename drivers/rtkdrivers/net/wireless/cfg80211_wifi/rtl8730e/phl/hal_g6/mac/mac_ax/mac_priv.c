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

#include "mac_priv.h"
#if MAC_AX_8730E_SUPPORT
#include "mac_8730e/mac_priv_8730e.h"
#endif

u32 get_mac_ax_priv_size(void)
{
	return sizeof(struct mac_ax_priv_ops *);
}

struct mac_ax_priv_ops *mac_ax_get_priv_ops(struct mac_ax_adapter *adapter,
		enum mac_ax_intf intf)
{
	struct mac_ax_priv_ops *priv_ops = NULL;

	switch (adapter->hw_info->chip_id) {
#if MAC_AX_8730E_SUPPORT
	case MAC_AX_CHIP_ID_8730E:
		priv_ops = get_mac_8730e_priv_ops(intf);
		break;
#endif

	default:
		return NULL;
	}

	return priv_ops;
}
