/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation. All rights reserved.
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

#include "halmac_axi_8730a.h"
#include "halmac_pwr_seq_8730a.h"
#include "../halmac_init_87xx.h"
#include "../halmac_common_87xx.h"
#include "../halmac_axi_87xx.h"
#include "../halmac_87xx_cfg.h"
#include "sysreg_lsys.h"

#if (HALMAC_8730A_SUPPORT && HALMAC_AXI_SUPPORT)

/**
  * @brief  Enables or disables the APB peripheral clock and function
  * @param  APBPeriph: specifies the APB peripheral to gates its clock.
  *      this parameter can be one of @ref UART_IRDA_PULSE_SHIFT_define
  * @param  APBPeriph_Clock: specifies the APB peripheral clock config.
  *      this parameter can be one of @ref RCC_FUNC_Exported_Constants
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_PeriphClockCmd(struct halmac_adapter *adapter, u32 APBPeriph_in, u32 APBPeriph_Clock_in, u8 NewState)
{
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	u32 ClkRegIndx = (APBPeriph_Clock_in >> 30) & 0x03;
	u32 APBPeriph_Clock = APBPeriph_Clock_in & (~(BIT(31) | BIT(30)));

	u32 FuncRegIndx = (APBPeriph_in >> 30) & 0x03;
	u32 APBPeriph = APBPeriph_in & (~(BIT(31) | BIT(30)));

	u32 Reg;
	u32 TempVal = 0;

	//clock
	switch (ClkRegIndx) {
	case 0x0:
		Reg = REG_LSYS_CKE_GRP0;
		break;
	case 0x1:
		Reg = REG_LSYS_CKE_GRP1;
		break;
	case 0x2:
		Reg = REG_LSYS_CKE_GRP2;
		break;
	case 0x3:
		Reg = REG_AON_CLK;
		break;
	}

	if (APBPeriph_Clock_in != APBPeriph_CLOCK_NULL) {
		if (NewState == ENABLE) {
			TempVal = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, Reg);
			TempVal |= APBPeriph_Clock;
			HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, Reg, TempVal);
		} else {
			TempVal = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, Reg);
			TempVal &= ~APBPeriph_Clock;
			HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, Reg, TempVal);
		}
	}

	//function
	switch (FuncRegIndx) {
	case 0x0:
		Reg = REG_LSYS_FEN_GRP0;
		break;
	case 0x1:
		Reg = REG_LSYS_FEN_GRP1;
		break;
	case 0x2:
		Reg = REG_LSYS_FEN_GRP2;
		break;
	case 0x3:
		Reg = REG_AON_FEN;
		break;
	}

	if (APBPeriph_in != APBPeriph_NULL) {
		if (NewState == ENABLE) {
			TempVal = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, Reg);
			TempVal |= APBPeriph;
			HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, Reg, TempVal);
		} else {
			TempVal = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, Reg);
			TempVal &= ~APBPeriph;
			HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, Reg, TempVal);
		}
	}

	rtw_udelay_os(10);

	return;
}

/**
 * mac_pwr_switch_axi_8730a() - switch mac power
 * @adapter : the adapter of halmac
 * @pwr : power state
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
mac_pwr_switch_axi_8730a(struct halmac_adapter *adapter,
			 enum halmac_mac_power pwr) {
	u8 value8 = 0;
	u16 value16 = 0;
	u32 value32 = 0;
	u8 rpwm = 0;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	enum halmac_ret_status status;
	u32 pollingCount = 0;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	PLTFM_MSG_TRACE("[TRACE]pwr = %x\n", pwr);
	PLTFM_MSG_TRACE("[TRACE]8730A pwr seq ver = %s\n",
			HALMAC_8730A_PWR_SEQ_VER);

	adapter->rpwm = HALMAC_REG_R8(REG_AXI_HRPWM1);

	/* Check FW still exist or not */
	value16 = HALMAC_REG_R16(REG_WL_MCUFW_CTRL);
	if ((value16 & BIT_FW_INIT_RDY) == BIT_FW_INIT_RDY)
	{
		/* Leave 32K */
		rpwm = (u8)((adapter->rpwm ^ BIT(7)) & 0x80);
		HALMAC_REG_W8(REG_AXI_HRPWM1, rpwm);
	}

	value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_PWC);
	if (value32 & LSYS_BIT_PWC_LP_WIFION)
	{
		adapter->halmac_state.mac_pwr = HALMAC_MAC_POWER_ON;
	} else
	{
		adapter->halmac_state.mac_pwr = HALMAC_MAC_POWER_OFF;
	}

	/* Check if power switch is needed */
	if (pwr == HALMAC_MAC_POWER_ON &&
	    adapter->halmac_state.mac_pwr == HALMAC_MAC_POWER_ON)
	{
		PLTFM_MSG_WARN("[WARN]power state unchange!!\n");
		return HALMAC_RET_PWR_UNCHANGE;
	}

	if (pwr == HALMAC_MAC_POWER_OFF)
	{
		status = trxdma_check_idle_87xx(adapter);
		if (status != HALMAC_RET_SUCCESS) {
			return status;
		}

		/* Reset BB */
		value32 = HALMAC_REG_R32(REG_WL_FUNC_EN);
		value32 &= ~(BIT_FEN_BBRSTB_V2 | BIT_FEN_BB_GLB_RSTN_V2);
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);

		/*Power off WL*/
		value32 = HALMAC_REG_R32(REG_WL_PMC_CTRL);
		value32 |= BIT_WL_PMC_OFFMAC;
		HALMAC_REG_W32(REG_WL_PMC_CTRL, value32);

		do {
			value32 = HALMAC_REG_R32(REG_WL_PMC_CTRL);
			value32 &= BIT_WL_PMC_OFFMAC;
			if (value32 == 0) {
				break;
			} else {
				rtw_mdelay_os(1);
				if (pollingCount++ > 5000) {
					PLTFM_MSG_ERR("%s Fail to polling 0x20[0]\n", __func__);
					return HALMAC_RET_FAIL;
				}
			}
		} while (1);

		/*Disable WL PMC*/
		value32 = HALMAC_REG_R32(REG_WL_FUNC_EN);
		value32 &= ~BIT_FEN_MAC_PMC;
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);

		/*Disable clk of KM4 accessing WL*/
		RCC_PeriphClockCmd(adapter, APBPeriph_NULL, APBPeriph_WMAC_CLOCK, DISABLE);

		/*Reset WLON*/
		RCC_PeriphClockCmd(adapter, APBPeriph_WLON, APBPeriph_WLON_CLOCK, DISABLE);

		/**disable WL RFAFE(0.9V and 3.3V) (bypass if BT use WL RFAFE)*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0);
		value32 &= ~(WLAFE_BIT_HW_EN_RFAFE | WLAFE_BIT_HW_EN_RFAFE_2REGU);
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0, value32);

		/*disable WL RFAFE control circuit (bypass if BT use WL RFAFE)*/
		RCC_PeriphClockCmd(adapter, APBPeriph_WLAFE, APBPeriph_NULL, DISABLE);

		/*Enable ISO of WLON*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_ISO);
		value32 |= LSYS_BIT_ISO_LP_WIFION;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_ISO, value32);

		/* Disable power cut of WLON */
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_PWC);
		value32 &= ~LSYS_BIT_PWC_LP_WIFION;;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_PWC, value32);

		adapter->halmac_state.mac_pwr = HALMAC_MAC_POWER_OFF;
		adapter->halmac_state.dlfw_state = HALMAC_DLFW_NONE;
		init_adapter_dynamic_param_87xx(adapter);
	} else
	{
		/* 1. Enable WIFI Power */
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_PWC);
		value32 |= LSYS_BIT_PWC_LP_WIFION;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_PWC, value32);

		/*2. delay 100us*/
		rtw_udelay_os(100);

		/*3. Disable ISO of WLON*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, REG_LSYS_ISO);
		value32 &= ~LSYS_BIT_ISO_LP_WIFION;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, REG_LSYS_ISO, value32);

		/*4. Enable WL RFAFE control circuit*/
		RCC_PeriphClockCmd(adapter, APBPeriph_WLAFE, APBPeriph_CLOCK_NULL, ENABLE);

		/*Enable hw auto response, DD will modify default value to 1*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAN_PWRCTL);
		value32 |= PMC_BIT_WL_HW_RESP_FEN;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAN_PWRCTL, value32);

		/*5. Enable WL RFAFE (0.9V and 3.3V)*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0);
		value32 |= (WLAFE_BIT_HW_EN_RFAFE | WLAFE_BIT_HW_EN_RFAFE_2REGU);
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_POW_MAC_0, value32);

		/*6. CK320M_AFE_EN, HW should do this, mp chip will let HW set this*/
		value32 = HALMAC_SYS_REG_R32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_1);
		value32 |= WLAFE_BIT_CK320M_AFE_EN;
		HALMAC_SYS_REG_W32(SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_MAC_1, value32);

		/*7. Release WLON reset*/
		RCC_PeriphClockCmd(adapter, APBPeriph_WLON, APBPeriph_WLON_CLOCK, ENABLE);

		/*8. Enable clk of KM4 accessing WL*/
		RCC_PeriphClockCmd(adapter, APBPeriph_NULL, APBPeriph_WMAC_CLOCK, ENABLE);

		/*9. delay 2us*/
		rtw_udelay_os(2);

		/*10. Enable WL PMC*/
		value32 = HALMAC_REG_R32(REG_WL_FUNC_EN);
		value32 |= BIT_FEN_MAC_PMC;
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);

		/*Power on WL*/
		value32 = HALMAC_REG_R32(REG_WL_PMC_CTRL);
		value32 |= BIT_WL_PMC_ONMAC;
		HALMAC_REG_W32(REG_WL_PMC_CTRL, value32);

		do {
			value32 = HALMAC_REG_R32(REG_WL_PMC_CTRL);
			value32 &= BIT_WL_PMC_ONMAC;
			if (value32 == 0) {
				break;
			} else {
				rtw_mdelay_os(1);
				if (pollingCount++ > 5000) {
					PLTFM_MSG_ERR("%s Fail to polling 0x20[0]\n", __func__);
					return HALMAC_RET_FAIL;
				}
			}
		} while (1);

		/* *Release BB reset */
		value32 = HALMAC_REG_R32(REG_WL_FUNC_EN);
		value32 |= (BIT_FEN_BBRSTB_V2 | BIT_FEN_BB_GLB_RSTN_V2);
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);

		adapter->halmac_state.mac_pwr = HALMAC_MAC_POWER_ON;
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return HALMAC_RET_SUCCESS;
}

/**
 * phy_cfg_pcie_8730a() - phy config
 * @adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
phy_cfg_axi_8730a(struct halmac_adapter *adapter,
		  enum halmac_intf_phy_platform pltfm) {
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);


	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return HALMAC_RET_SUCCESS;
}

/**
 * intf_tun_axi_8730a() - pcie interface fine tuning
 * @adapter : the adapter of halmac
 * Author : Rick Liu
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
intf_tun_axi_8730a(struct halmac_adapter *adapter) {
	return HALMAC_RET_SUCCESS;
}

/**
 * cfgspc_set_axi_8730a() - pcie configuration space setting
 * @adapter : the adapter of halmac
 * Author : Rick Liu
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
cfgspc_set_axi_8730a(struct halmac_adapter *adapter,
		     struct halmac_pcie_cfgspc_param *param) {
	return HALMAC_RET_NOT_SUPPORT;
}

#endif /* HALMAC_8730A_SUPPORT*/
