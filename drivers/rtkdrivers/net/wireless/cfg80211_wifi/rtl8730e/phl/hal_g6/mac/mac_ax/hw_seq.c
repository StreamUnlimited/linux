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
#include "hw_seq.h"


u32 mac_set_hwseq_reg(struct mac_ax_adapter *adapter,
		      u8 idx,
		      u16 val)
{
	u32 reg_val;
	u32 ret = MACNOTSUP;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	switch (idx) {
	case R_AX_HW_SEQ_0:
		reg_val = MAC_REG_R32(REG_HW_SEQ0_1);
		reg_val = BIT_SET_HW_SSN_SEQ0(reg_val, val);
		MAC_REG_W32(REG_HW_SEQ0_1, reg_val);
		ret = MACSUCCESS;
		break;
	case R_AX_HW_SEQ_1:
		reg_val = MAC_REG_R32(REG_HW_SEQ0_1);
		reg_val = BIT_SET_HW_SSN_SEQ1(reg_val, val);
		MAC_REG_W32(REG_HW_SEQ0_1, reg_val);
		ret = MACSUCCESS;
		break;
	case R_AX_HW_SEQ_2:
		reg_val = MAC_REG_R32(REG_HW_SEQ2_3);
		reg_val = BIT_SET_HW_SSN_SEQ2(reg_val, val);
		MAC_REG_W32(REG_HW_SEQ2_3, reg_val);
		ret = MACSUCCESS;
		break;
	case R_AX_HW_SEQ_3:
		reg_val = MAC_REG_R32(REG_HW_SEQ2_3);
		reg_val = BIT_SET_HW_SSN_SEQ3(reg_val, val);
		MAC_REG_W32(REG_HW_SEQ2_3, reg_val);
		ret = MACSUCCESS;
		break;
	default:
		ret = MACNOITEM;
	}
	return ret;
}

u32 mac_get_hwseq_cfg(struct mac_ax_adapter *adapter,
		      u8 macid, u8 ref_sel,
		      struct mac_ax_dctl_seq_cfg *seq_info)
{
	struct mac_ax_dctl_info info = {0};
	struct mac_ax_dctl_info mask = {0};
	struct mac_ax_ops *a_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = 0;

	seq_info->seq0_val = BIT_GET_HW_SSN_SEQ0(MAC_REG_R32(REG_HW_SEQ0_1));
	seq_info->seq1_val = BIT_GET_HW_SSN_SEQ1(MAC_REG_R32(REG_HW_SEQ0_1));
	seq_info->seq2_val = BIT_GET_HW_SSN_SEQ2(MAC_REG_R32(REG_HW_SEQ2_3));
	seq_info->seq3_val = BIT_GET_HW_SSN_SEQ3(MAC_REG_R32(REG_HW_SEQ2_3));

	return MACSUCCESS;
}

