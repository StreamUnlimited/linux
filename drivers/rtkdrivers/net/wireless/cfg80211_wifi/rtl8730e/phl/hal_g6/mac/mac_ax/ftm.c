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
#include "ftm.h"

#define FWCMD_H2C_FTM_INFO_PKTID_SH 0
#define FWCMD_H2C_FTM_INFO_PKTID_MSK 0xff
#define FWCMD_H2C_FTM_INFO_RSP_CH_SH 8
#define FWCMD_H2C_FTM_INFO_RSP_CH_MSK 0xff
#define FWCMD_H2C_FTM_INFO_TSF_TIMER_OFFSET_SH 16
#define FWCMD_H2C_FTM_INFO_TSF_TIMER_OFFSET_MSK 0xff
#define FWCMD_H2C_FTM_INFO_ASAP_SH 24
#define FWCMD_H2C_FTM_INFO_ASAP_MSK 0xff

u32 mac_update_ftm_info(struct mac_ax_adapter *adapter,
			struct fwcmd_ftm_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 fill_ftm_para(struct mac_ax_adapter *adapter,
		  struct mac_ax_ftm_para *ftm_info,
		  struct fwcmd_ftm_info *ftm_fw_info)
{
	ftm_fw_info->dword0 =
		cpu_to_le32(SET_WORD(ftm_info->pktid, FWCMD_H2C_FTM_INFO_PKTID) |
			    SET_WORD(ftm_info->rsp_ch, FWCMD_H2C_FTM_INFO_RSP_CH) |
			    SET_WORD(ftm_info->tsf_timer_offset, FWCMD_H2C_FTM_INFO_TSF_TIMER_OFFSET) |
			    SET_WORD(ftm_info->asap, FWCMD_H2C_FTM_INFO_ASAP));

	return MACSUCCESS;
}

u32 mac_ista_ftm_proc(struct mac_ax_adapter *adapter,
		      struct mac_ax_ftm_para *ftmr)
{
	u32 ftm_fw_info = 0, ret = 0;

	fill_ftm_para(adapter, ftmr,
		      (struct fwcmd_ftm_info *)(&ftm_fw_info));

	ret = (u8)mac_update_ftm_info(adapter,
				      (struct fwcmd_ftm_info *)(&ftm_fw_info));
	if (ret != MACSUCCESS) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_ista_ftm_enable(struct mac_ax_adapter *adapter,
			u8 macid, bool enable)
{
	u32 val32 = 0, ret = 0;
	struct rtw_hal_mac_ax_cctl_info info, msk = {0};
	struct mac_role_tbl *role;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	role = mac_role_srch(adapter, macid);
	if (!role) {
		PLTFM_MSG_ERR("%s: The MACID%d does not exist\n",
			      __func__, macid);
		return MACNOITEM;
	}

	if (enable) {
		ret = check_mac_en(adapter, 0, MAC_AX_MAC_SEL);
		if (ret != MACSUCCESS) {
			return ret;
		}

		// init BB FTM CLK
		val32 = MAC_REG_R32(0x10014);
		val32 |= (BIT5 | BIT6);
		MAC_REG_W32(0x10014, val32);

		val32 = MAC_REG_R32(0x109C4);
		val32 |= (BIT31 | BIT28 | BIT26);
		MAC_REG_W32(0x109C4, val32);

		// init TRX FTM
		val32 = MAC_REG_R32(R_AX_WMAC_FTM_CTL);
		val32 |= (B_AX_FTM_EN | B_AX_RXFTM_EN);
		MAC_REG_W32(R_AX_WMAC_FTM_CTL, val32);

		// Trun on FTM report
		msk.acq_rpt_en = 1;
		info.acq_rpt_en = 1;
		msk.mgq_rpt_en = 1;
		info.mgq_rpt_en = 1;
		msk.ulq_rpt_en = 1;
		info.ulq_rpt_en = 1;
		ret = mac_upd_cctl_info(adapter, &info, &msk, macid, 1);
	} else {
		// Turn off BB FTM CLK
		val32 = MAC_REG_R32(0x10014);
		val32 &= ~(BIT5 | BIT6);
		MAC_REG_W32(0x10014, val32);

		val32 = MAC_REG_R32(0x109C4);
		val32 &= ~(BIT31 | BIT28 | BIT26);
		MAC_REG_W32(0x109C4, val32);

		// Turn off TRX FTM
		val32 = MAC_REG_R32(R_AX_WMAC_FTM_CTL);
		val32 &= ~(B_AX_FTM_EN | B_AX_RXFTM_EN);
		MAC_REG_W32(R_AX_WMAC_FTM_CTL, val32);

		// Trun off FTM report
		msk.acq_rpt_en = 1;
		info.acq_rpt_en = 0;
		msk.mgq_rpt_en = 1;
		info.mgq_rpt_en = 0;
		msk.ulq_rpt_en = 1;
		info.ulq_rpt_en = 0;
		ret = mac_upd_cctl_info(adapter, &info, &msk, macid, 1);
	}

	return MACSUCCESS;
}
