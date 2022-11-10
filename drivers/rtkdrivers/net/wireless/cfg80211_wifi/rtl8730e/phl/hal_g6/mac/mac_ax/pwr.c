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
#include "pwr.h"
#include "coex.h"
#include "mac_priv.h"

static u32 pwr_cmd_poll(struct mac_ax_adapter *adapter, struct mac_pwr_cfg *seq)
{
	u8 val = 0, offset;
	u32 base;
	u32 cnt;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	cnt = PWR_POLL_CNT;
	base = seq->base;
	offset = seq->offset;

	while (cnt--) {
		val = SYS_REG_R8(base, offset);
		val &= seq->msk;
		if (val == (seq->val & seq->msk)) {
			return MACSUCCESS;
		}
		PLTFM_DELAY_US(PWR_POLL_DLY_US);
	}

	PLTFM_MSG_ERR("[ERR] Polling timeout\n");
	PLTFM_MSG_ERR("[ERR] addr: %X, %X\n", base + offset, seq->base + seq->offset);
	PLTFM_MSG_ERR("[ERR] val: %X, %X\n", val, seq->val);

	return MACPOLLTO;
}

static u32 sub_pwr_seq_start(struct mac_ax_adapter *adapter,
			     u8 cv_msk, u8 intf_msk, struct mac_pwr_cfg *seq)
{
	u8 val, offset;
	u32 base;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	while (seq->cmd != PWR_CMD_END) {
		if (!(seq->intf_msk & intf_msk) || !(seq->cut_msk & cv_msk)) {
			goto next_seq;
		}

		switch (seq->cmd) {
		case PWR_CMD_WRITE:
			base = seq->base;
			offset = seq->offset;
			val = SYS_REG_R8(base, offset);
			val &= ~(seq->msk);
			val |= (seq->val & seq->msk);

			SYS_REG_W8(base, offset, val);
			break;
		case PWR_CMD_POLL:
			ret = pwr_cmd_poll(adapter, seq);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]pwr cmd poll %d\n", ret);
				return ret;
			}
			break;
		case PWR_CMD_DELAY:
			if (seq->val == PWR_DELAY_US) {
				PLTFM_DELAY_US(seq->offset);
			} else {
				PLTFM_DELAY_US(seq->offset * 1000);
			}
			break;
		default:
			PLTFM_MSG_ERR("[ERR]unknown pwr seq cmd %d\n",
				      seq->cmd);
			return MACNOITEM;
		}
next_seq:
		seq++;
	}

	return MACSUCCESS;
}

u32 pwr_seq_start(struct mac_ax_adapter *adapter,
		  struct mac_pwr_cfg **seq)
{
	u8 cv;
	u8 intf;
	u32 ret;
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	struct mac_pwr_cfg *sub_seq = *seq;
#if MAC_AX_USB_SUPPORT
	u32 val = 0;
#endif

	switch (hw_info->cv) {
	case CAV:
		cv = PWR_CAV_MSK;
		break;
	case CBV:
		cv = PWR_CBV_MSK;
		break;
	case CCV:
		cv = PWR_CCV_MSK;
		break;
	case CDV:
		cv = PWR_CDV_MSK;
		break;
	case CEV:
		cv = PWR_CEV_MSK;
		break;
	default:
		PLTFM_MSG_ERR("[ERR]cut version\n");
		return MACNOITEM;
	}

	switch (hw_info->intf) {
	case MAC_AX_INTF_SDIO:
		intf = PWR_INTF_MSK_SDIO;
		break;
	case MAC_AX_INTF_USB:
#if MAC_AX_USB_SUPPORT
		val = get_usb_mode(adapter);
		if (val == MAC_AX_USB3) {
			intf = PWR_INTF_MSK_USB3;
		} else {
			intf = PWR_INTF_MSK_USB2;
		}
		break;
#endif
	case MAC_AX_INTF_PCIE:
		intf = PWR_INTF_MSK_PCIE;
		break;
	case MAC_AX_INTF_AXI:
		intf = PWR_INTF_MSK_AXI;
		break;
	default:
		PLTFM_MSG_ERR("[ERR]interface\n");
		return MACNOITEM;
	}

	while (sub_seq) {
		ret = sub_pwr_seq_start(adapter, cv, intf, sub_seq);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]sub pwr seq %d\n", ret);
			return ret;
		}
		seq++;
		sub_seq = *seq;
	}

	return MACSUCCESS;
}

u32 mac_pwr_switch(struct mac_ax_adapter *adapter, u8 on)
{
	u32 ret = MACSUCCESS;
	u32 ret_end;
	struct mac_pwr_cfg **pwr_seq;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32(*pwr_func)(struct mac_ax_adapter * adapter);
	u32 val32;

	/*0xA4[14:12]  14:lps  13:on  12:off*/
	val32 = MAC_REG_R32(REG_SYSON_FSM_MON_V1);
	val32 = (val32 >> 12) & 0x7;
	if (val32 == MAC_OFF && on == MAC_AX_MAC_OFF) {
		PLTFM_MSG_WARN("MAC has already powered off\n");
		return MACSUCCESS;
	}

	if (!p_ops->intf_pwr_switch) {
		PLTFM_MSG_ERR("interface power switch func is NULL\n");
		ret = MACNPTR;
		return ret;
	}

	if (on) {
		pwr_seq = adapter->hw_info->pwr_on_seq;
		pwr_func = p_ops->pwr_on;
	} else {
		pwr_seq = adapter->hw_info->pwr_off_seq;
		pwr_func = p_ops->pwr_off;
		adapter->sm.pwr = MAC_AX_PWR_PRE_OFF;
		adapter->sm.mac_func = MAC_AX_FUNC_OFF;
		adapter->sm.bb_func = MAC_AX_FUNC_OFF;
		adapter->sm.plat = MAC_AX_PLAT_OFF;
	}

	ret = p_ops->intf_pwr_switch(adapter, PWR_PRE_SWITCH, on);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("interface pre pwr switch fails %d\n", ret);
		goto END;
	}

	if (on) {
		/*		ret = _patch_aon_int_leave_lps(adapter);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("AON interrupt leave LPS fail %X\n", ret);
					goto END;
				}
		*/
		val32 = MAC_REG_R32(REG_SYSON_FSM_MON_V1);
		val32 = (val32 >> 12) & 0x7;
		if (val32 == MAC_ON) {
			PLTFM_MSG_WARN("MAC has already powered on %d\n", val32);
			ret = MACALRDYON;
			goto END;
		} else if (val32 == MAC_LPS) {
			PLTFM_MSG_ERR("MAC leave LPS fail %d\n", val32);
			ret = MACPWRSTAT;
			goto END;
		}
	}

	if (!pwr_func) {
		ret = pwr_seq_start(adapter, pwr_seq);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr seq start %d\n", ret);
			adapter->sm.pwr = MAC_AX_PWR_ERR;
			goto END;
		}
	} else {
		ret = pwr_func(adapter);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr func %d\n", ret);
			adapter->sm.pwr = MAC_AX_PWR_ERR;
			goto END;
		}
	}

	ret = p_ops->intf_pwr_switch(adapter, PWR_POST_SWITCH, on);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("interface post pwr switch fails %d\n", ret);
		adapter->sm.pwr = MAC_AX_PWR_ERR;
		goto END;
	}

	mac_ax_init_state(adapter);

	if (on) {
		adapter->sm.pwr = MAC_AX_PWR_ON;
		adapter->sm.plat = MAC_AX_PLAT_ON;
		adapter->sm.io_st = MAC_AX_IO_ST_NORM;
		adapter->sm.fw_rst = MAC_AX_FW_RESET_IDLE;
		adapter->sm.l2_st = MAC_AX_L2_DIS;
		adapter->mac_pwr_info.pwr_in_lps = 0;
	} else {
		adapter->sm.pwr = MAC_AX_PWR_OFF;
		adapter->sm.l2_st = MAC_AX_L2_DIS;
	}
END:
	ret_end = p_ops->intf_pwr_switch(adapter, PWR_END_SWITCH, on);
	if (ret_end != MACSUCCESS) {
		PLTFM_MSG_ERR("interface end pwr switch fails %d\n", ret_end);
		adapter->sm.pwr = MAC_AX_PWR_ERR;
		return ret_end;
	}
	return ret;
}
