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
 * 8852C V004
 ******************************************************************************/

#include "../pwr.h"
#include "../pwr_seq_func.h"
#include "pwr_seq_func_8730e.h"
#if MAC_AX_8730E_SUPPORT

#define MAC_AX_HCI_SEL_SDIO_UART 0
#define MAC_AX_HCI_SEL_MULTI_USB 1
#define MAC_AX_HCI_SEL_PCIE_UART 2
#define MAC_AX_HCI_SEL_PCIE_USB 3
#define MAC_AX_HCI_SEL_MULTI_SDIO 4

u32 mac_pwr_on_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	struct rtw_hal_com_t *hal_com = (struct rtw_hal_com_t *)adapter->drv_adapter;

	/*0x42008208[24]=1 enable WL RFAFE control circuit*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0, val32 & (~FEN_WLAFE_CTRL));
	PLTFM_DELAY_US(10);
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0, val32 | FEN_WLAFE_CTRL);
	PLTFM_DELAY_US(10);

	/*0x42008940[6:5]=2'b11, [9:8]=2'b11 enable WL RF and AFE(0.9V & 3.3V)*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0, val32 | HW_EN_LAFE_2REGU | \
		    HW_EN_WLAFE | HW_EN_WLRF_2REGU | HW_EN_WLRF);

	/*0x42008208[7]=1 release WLON reset*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0, val32 & (~FEN_WLON));
	PLTFM_DELAY_US(10);
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0, val32 | FEN_WLON);
	PLTFM_DELAY_US(10);

	/*0x42008218[5]=1 enable clk of KM4 accessing WL*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_CKE_GRP1);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_CKE_GRP1, val32 | CKE_WMAC);
	PLTFM_DELAY_US(10);

	/*0x40000004[0]=1 enable WL PMC*/
	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	MAC_REG_W32(REG_WL_FUNC_EN, val32 | BIT_FEN_MAC_PMC);

	/*0x40000020[0]=1 power on WLOFF*/
	val32 = MAC_REG_R32(REG_WL_PMC_CTRL);
	MAC_REG_W32(REG_WL_PMC_CTRL, val32 | BIT_WL_PMC_ONMAC);

	/*polling 0x40000020[0]=0*/
	ret = pwr_poll_u32(adapter, REG_WL_PMC_CTRL, BIT_WL_PMC_ONMAC, 0);
	if (ret) {
		return ret;
	}

	/* reset WLRFC  0x8960,set[7:0]=0,then set [7:0]=0xff,enable RFC clk */
	val32 = MAC_REG_R32(REG_SYS_PWC_ISO_CTRL);
	val32 &= ~BIT_WL_SAVE_EN;
	MAC_REG_W32(REG_SYS_PWC_ISO_CTRL, val32);

	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	val32 &= ~BIT_FEN_WLOFF;
	MAC_REG_W32(REG_WL_FUNC_EN, val32);

	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	val32 |= BIT_FEN_WLOFF;
	MAC_REG_W32(REG_WL_FUNC_EN, val32);

	/*0x40000004[17:16]=2'b11 release BB reset and enable CCK/OFDM*/
	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	MAC_REG_W32(REG_WL_FUNC_EN, val32 | BIT_FEN_BB_GLB_RSTN_V2 |
		    BIT_FEN_BBRSTB_V2);

	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_RFC);
	val32 &= ~WLAFE_MASK_ANAPOW_RFC;
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_RFC, val32);

	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_RFC);
	val32 |= WLAFE_MASK_ANAPOW_RFC;
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_RFC, val32);

	val32 = MAC_REG_R32(REG_CLKDIV_CKSEL);
	val32 |= BIT_WL_CKRFC_EN;
	MAC_REG_W32(REG_CLKDIV_CKSEL, val32);

#ifdef CONFIG_BTCOEX
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_WLRFC_CTRL);
	val32 &= ~WLAFE_BIT_WLRFC_PWC_SEL;
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_WLRFC_CTRL, val32);
#endif

	adapter->sm.pwr = MAC_AX_PWR_ON;
	adapter->sm.plat = MAC_AX_PLAT_ON;
	adapter->sm.io_st = MAC_AX_IO_ST_NORM;
	adapter->sm.mac_func = MAC_AX_FUNC_ON;

	return MACSUCCESS;
}

u32 mac_pwr_off_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/*0x40000004[17:16]=2'b0 reset BB and disable clk */
	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	MAC_REG_W32(REG_WL_FUNC_EN, val32 & ~(BIT_FEN_BB_GLB_RSTN_V2 |
		    BIT_FEN_BBRSTB_V2));

	/* for A cut bug wloff error 0.4V */
	val32 = MAC_REG_R32(REG_SYS_PWC_ISO_CTRL);
	val32 |= BIT_WL_SAVE_EN;
	MAC_REG_W32(REG_SYS_PWC_ISO_CTRL, val32);

	/* 42008968[2]=1,RFC retention mode */
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_WLRFC_CTRL);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_WLRFC_CTRL, val32 | WLAFE_BIT_WLRFC_SAVE_EN);

	/*0x40000020[1]=1 power off WLOFF*/
	val32 = MAC_REG_R32(REG_WL_PMC_CTRL);
	MAC_REG_W32(REG_WL_PMC_CTRL, val32 | BIT_WL_PMC_OFFMAC);

	/*polling 0x40000020[1]=0*/
	ret = pwr_poll_u32(adapter, REG_WL_PMC_CTRL, BIT_WL_PMC_OFFMAC, 0);
	if (ret) {
		return ret;
	}

	/*0x40000004[0]=0 disable WL PMC*/
	val32 = MAC_REG_R32(REG_WL_FUNC_EN);
	MAC_REG_W32(REG_WL_FUNC_EN, val32 & ~BIT_FEN_MAC_PMC);

	/*0x42008218[5]=0 disable clk of KM4 accessing WL*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_CKE_GRP1);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_CKE_GRP1, val32 & ~CKE_WMAC);
	PLTFM_DELAY_US(10);

	/*0x42008940[6:5]=2'b00 [9:8]=2'b00 disable WL RF & AFE*/
	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0);
	SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0,\
		    val32 & ~(HW_EN_LAFE_2REGU | HW_EN_WLAFE));

	val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_BT_CTRL0);
	val32 &= LSYS_BIT_BT_USE_WL_RFAFE;
	//BT_S0->0x42008250[0]=0, BT_S1-> 0x42008250[0]=1
	if (val32 == 0) {  //BT_S0 not use WL RF
		val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0);
		SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0,\
			    val32 & ~(HW_EN_WLRF_2REGU | HW_EN_WLRF));
	}

	/*0x42008208[7]=0 reset WLON*/
	//val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	//SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0, val32 & ~FEN_WLON);
	//PLTFM_DELAY_US(10);

	/*0x42008208[24]=0 disable WL RFAFE control circuit*/
	//val32 = SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0);
	//SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FEN_GRP0,
	//	    val32 & ~FEN_WLAFE_CTRL);
	//PLTFM_DELAY_US(10);

	return MACSUCCESS;
}

#if MAC_AX_FEATURE_HV
u32 mac_enter_lps_sdio_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_AX_FWD1IMR_V1);
	MAC_REG_W32(R_AX_FWD1IMR_V1, val32 | BIT(0));

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM32PP_V1);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM3_PC_EN);

	/* XTAL_SI 0x90[0] = 1 then 0x90[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x10, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[3] = 0 then 0x90[2] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 0 */
	/* XTAL_SI 0x90[5] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[6] = 0 */
	/* XTAL_SI 0x90[6] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x90[31:0] = 0x04_80_A1_B0 */
	MAC_REG_W32(R_AX_WLLPS_CTRL, 0x0480A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_AX_WLLPS_CTRL);
	MAC_REG_W32(R_AX_WLLPS_CTRL, val32 | B_AX_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_AX_WLLPS_CTRL, B_AX_WL_LPS_EN, 0);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_enter_lps_usb_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_AX_FWD1IMR_V1);
	MAC_REG_W32(R_AX_FWD1IMR_V1, val32 | BIT(0));

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM32PP_V1);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM3_PC_EN);

	/* XTAL_SI 0x90[0] = 1 then 0x90[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x10, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[3] = 0 then 0x90[2] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 0 */
	/* XTAL_SI 0x90[5] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[6] = 0 */
	/* XTAL_SI 0x90[6] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x90[31:0] = 0x04_81_A1_B0 */
	MAC_REG_W32(R_AX_WLLPS_CTRL, 0x0481A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_AX_WLLPS_CTRL);
	MAC_REG_W32(R_AX_WLLPS_CTRL, val32 | B_AX_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_AX_WLLPS_CTRL, B_AX_WL_LPS_EN, 0);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_enter_lps_pcie_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_AX_FWD1IMR_V1);
	MAC_REG_W32(R_AX_FWD1IMR_V1, val32 | BIT(0));

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM32PP_V1);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND, val32 | B_AX_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_AX_AFE_CTRL1);
	MAC_REG_W32(R_AX_AFE_CTRL1, val32 & ~B_AX_DMEM3_PC_EN);

	/* XTAL_SI 0x90[0] = 1 then 0x90[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0x02, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x10, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[3] = 0 then 0x90[2] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 0 */
	/* XTAL_SI 0x90[5] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[6] = 0 */
	/* XTAL_SI 0x90[6] = 0 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 & ~B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x90[31:0] = 0x04_81_A1_B0 */
	MAC_REG_W32(R_AX_WLLPS_CTRL, 0x0481A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_AX_WLLPS_CTRL);
	MAC_REG_W32(R_AX_WLLPS_CTRL, val32 | B_AX_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_AX_WLLPS_CTRL, B_AX_WL_LPS_EN, 0);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_leave_lps_sdio_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x4083[7] = 1 == 0x4080[31] = 1 */
	val32 = MAC_REG_R32(R_AX_SDIO_HRPWM1_V1);
	MAC_REG_W32(R_AX_SDIO_HRPWM1_V1, val32 | BIT(31));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_AX_RPWM,
			   B_AX_RPWM_TOGGLE, B_AX_RPWM_TOGGLE);
	if (ret) {
		return ret;
	}

	/* delay 0x10 ms */
	while (--cnt) {
		PLTFM_DELAY_US(50);
	}

	/* 0x4083[7] = 0 == 0x4080[31] = 0 */
	val32 = MAC_REG_R32(R_AX_SDIO_HRPWM1_V1);
	MAC_REG_W32(R_AX_SDIO_HRPWM1_V1, val32 & ~(BIT(31)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_AX_FWD1ISR_V1);
	MAC_REG_W32(R_AX_FWD1ISR_V1, val32 | B_AX_FS_RPWM_INT);

	/* 0x18[6] = 1 */
	/* XTAL_SI 0x90[6] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x40, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 1 */
	/* XTAL_SI 0x90[5] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x20, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[2] = 1 then 0x90[3] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x04, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x08, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x80, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[1] = 0 then 0x90[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_leave_lps_usb_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x5203[7] = 1 == 0x5200[31] = 1 */
	val32 = MAC_REG_R32(R_AX_USB_D2F_F2D_INFO_V1);
	MAC_REG_W32(R_AX_USB_D2F_F2D_INFO_V1, val32 | BIT(31));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_AX_RPWM,
			   B_AX_RPWM_TOGGLE, B_AX_RPWM_TOGGLE);
	if (ret) {
		return ret;
	}

	/* delay 0x10 ms */
	while (--cnt) {
		PLTFM_DELAY_US(50);
	}

	/* 0x5203[7] = 0 == 0x5200[31] = 0 */
	val32 = MAC_REG_R32(R_AX_USB_D2F_F2D_INFO_V1);
	MAC_REG_W32(R_AX_USB_D2F_F2D_INFO_V1, val32 & ~(BIT(31)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_AX_FWD1ISR_V1);
	MAC_REG_W32(R_AX_FWD1ISR_V1, val32 | B_AX_FS_RPWM_INT);

	/* 0x18[6] = 1 */
	/* XTAL_SI 0x90[6] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x40, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 1 */
	/* XTAL_SI 0x90[5] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x20, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[2] = 1 then 0x90[3] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x04, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x08, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x80, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[1] = 0 then 0x90[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_leave_lps_pcie_8852c(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x30C1[7] = 1 */
	val32 = MAC_REG_R32(R_AX_PCIE_HRPWM_V1);
	MAC_REG_W32(R_AX_PCIE_HRPWM_V1, val32 | BIT(15));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_AX_RPWM,
			   B_AX_RPWM_TOGGLE, B_AX_RPWM_TOGGLE);
	if (ret) {
		return ret;
	}

	/* delay 0x10 ms */
	while (--cnt) {
		PLTFM_DELAY_US(50);
	}

	/* 0x30C1[7] = 0 */
	val32 = MAC_REG_R32(R_AX_PCIE_HRPWM_V1);
	MAC_REG_W32(R_AX_PCIE_HRPWM_V1, val32 & ~(BIT(15)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_AX_FWD1ISR_V1);
	MAC_REG_W32(R_AX_FWD1ISR_V1, val32 | B_AX_FS_RPWM_INT);

	/* 0x18[6] = 1 */
	/* XTAL_SI 0x90[6] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x40, 0x40);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* 0x18[5] = 1 */
	/* XTAL_SI 0x90[5] = 1 */
	val32 = MAC_REG_R32(R_AX_SYS_ADIE_PAD_PWR_CTRL);
	MAC_REG_W32(R_AX_SYS_ADIE_PAD_PWR_CTRL, val32 | B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x20, 0x20);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[2] = 1 then 0x90[3] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x04, 0x04);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x08, 0x08);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[4] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x10);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x80[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S0, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x81[0] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WL_RFC_S1, 0x01, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0xA1[1] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_SRAM_CTRL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[7] = 1 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0x80, 0x80);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	/* XTAL_SI 0x90[1] = 0 then 0x90[0] = 0 */
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x02);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_ANAPAR_WL, 0, 0x01);
	if (ret) {
		PLTFM_MSG_ERR("Write XTAL_SI fail!\n");
		return ret;
	}

	return MACSUCCESS;
}

#endif

#endif /* #if MAC_AX_8852C_SUPPORT */
