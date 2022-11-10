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
#include "coex.h"
#include "../mac_reg.h"
#include "hw.h"
#include "power_saving.h"

#define MAC_AX_RTK_RATE 5

#define MAC_AX_BT_MODE_0_3 0
#define MAC_AX_BT_MODE_2 2

#define MAC_AX_CSR_DELAY 0
#define MAC_AX_CSR_PRI_TO 5
#define MAC_AX_CSR_TRX_TO 4

#define MAC_AX_CSR_RATE 80

#define MAC_AX_SB_DRV_MSK 0xFFFFFF
#define MAC_AX_SB_DRV_SH 0
#define MAC_AX_SB_FW_MSK 0x7F
#define MAC_AX_SB_FW_SH 24

#define MAC_AX_BTGS1_NOTIFY BIT(0)

u32 mac_cfg_plt(struct mac_ax_adapter *adapter, struct mac_ax_plt *plt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;
	u32 val;

	ret = check_mac_en(adapter, MAC_AX_MAC_SEL);
	if (ret) {
		return ret;
	}

	val = MAC_REG_R32(REG_OTHER_CFG);
	if (plt->rx & BIT_EN_BT_POLLUTE_RX) {
		MAC_REG_W32(REG_OTHER_CFG, val | BIT_EN_BT_POLLUTE_RX);
	} else {
		MAC_REG_W32(REG_OTHER_CFG, val & ~BIT_EN_BT_POLLUTE_RX);
	}

	val = MAC_REG_R32(REG_NDPA_CTRL_LBK_ACQ_STOP);
	if (plt->tx & BIT_R_EN_GNT_BT_AWAKE) {
		MAC_REG_W32(REG_NDPA_CTRL_LBK_ACQ_STOP, val | BIT_R_EN_GNT_BT_AWAKE);
	} else {
		MAC_REG_W32(REG_NDPA_CTRL_LBK_ACQ_STOP, val & ~BIT_R_EN_GNT_BT_AWAKE);
	}

	return MACSUCCESS;
}

u32 mac_read_coex_reg(struct mac_ax_adapter *adapter,
		      const u32 offset, u32 *val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (offset > 0xFF) {
		PLTFM_MSG_ERR("[ERR]offset exceed coex reg\n");
		return MACBADDR;
	}

	//*val = MAC_REG_R32(R_AX_BTC_CFG + offset);

	return MACSUCCESS;
}

u32 mac_write_coex_reg(struct mac_ax_adapter *adapter,
		       const u32 offset, const u32 val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (offset > 0xFF) {
		PLTFM_MSG_ERR("[ERR]offset exceed coex reg\n");
		return MACBADDR;
	}

	//MAC_REG_W32(R_AX_BTC_CFG + offset, val);

	return MACSUCCESS;
}
