/******************************************************************************
 *
 * Copyright(c) 2015 - 2018 Realtek Corporation.
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
 *****************************************************************************/
#define _RTL8730A_HALINIT_C_

#include <drv_types.h>		/* PADAPTER, basic_types.h and etc. */
#include <hal_data.h>		/* GET_HAL_SPEC(), HAL_DATA_TYPE */
#include "../hal_halmac.h"	/* HALMAC API */
#include "rtl8730a.h"


void rtl8730a_init_hal_spec(PADAPTER adapter)
{
	struct hal_spec_t *hal_spec;


	hal_spec = GET_HAL_SPEC(adapter);
	rtw_halmac_fill_hal_spec(adapter_to_dvobj(adapter), hal_spec);

	hal_spec->ic_name = "rtl8730a";
	hal_spec->macid_num = 128;
	/* hal_spec->sec_cam_ent_num follow halmac setting */
	hal_spec->sec_cap = SEC_CAP_CHK_BMC | SEC_CAP_CHK_EXTRA_SEC;
	hal_spec->macid_cap = MACID_DROP;

	hal_spec->rfpath_num_2g = 1;
	hal_spec->rfpath_num_5g = 1;
	hal_spec->rf_reg_path_num = hal_spec->rf_reg_path_avail_num = 1;
	hal_spec->rf_reg_trx_path_bmp = 0x11;
	hal_spec->max_tx_cnt = 1;

	hal_spec->tx_nss_num = 1;
	hal_spec->rx_nss_num = 1;
	hal_spec->band_cap = BAND_CAP_2G | BAND_CAP_5G;
	hal_spec->bw_cap = BW_CAP_20M;
	hal_spec->port_num = 5;
	hal_spec->proto_cap = PROTO_CAP_11B | PROTO_CAP_11G | PROTO_CAP_11N;

	hal_spec->txgi_max = 127;
	hal_spec->txgi_pdbm = 4;

	hal_spec->wl_func = 0
			    | WL_FUNC_P2P
			    | WL_FUNC_MIRACAST
			    | WL_FUNC_TDLS
			    ;

	hal_spec->tx_aclt_unit_factor = 8;

	hal_spec->rx_tsf_filter = 1;

	hal_spec->pg_txpwr_saddr = EEPROM_TX_PWR_INX_8730A;
	hal_spec->pg_txgi_diff_factor = 2;

	hal_spec->hci_type = 0;

	rtw_macid_ctl_init_sleep_reg(adapter_to_macidctl(adapter)
				     , REG_MACID_SLEEP_8730A
				     , REG_MACID_SLEEP2_8730A
				     , REG_MACID_SLEEP2_8730A
				     , REG_MACID_SLEEP3_8730A);

	rtw_macid_ctl_init_drop_reg(adapter_to_macidctl(adapter)
				    , REG_MACID_DROP0_8730A
				    , REG_MACID_DROP1_8730A
				    , REG_MACID_DROP2_8730A
				    , REG_MACID_DROP3_8730A);
}

u32 rtl8730a_power_on(PADAPTER adapter)
{
	struct dvobj_priv *d;
	PHAL_DATA_TYPE hal;
	u8 bMacPwrCtrlOn;
	u32 value32 = 0;
	int err = 0;
	u8 ret = _SUCCESS;


	d = adapter_to_dvobj(adapter);

	bMacPwrCtrlOn = _FALSE;
	rtw_hal_get_hwreg(adapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
	if (bMacPwrCtrlOn == _TRUE) {
		goto out;
	}

	err = rtw_halmac_poweron(d);
	if (err) {
		RTW_ERR("%s: Power ON Fail!!\n", __FUNCTION__);
		ret = _FAIL;
		goto out;
	}

	bMacPwrCtrlOn = _TRUE;
	rtw_hal_set_hwreg(adapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);

out:
	return ret;
}

void rtl8730a_power_off(PADAPTER adapter)
{
	struct dvobj_priv *d;
	u8 bMacPwrCtrlOn;
	int err = 0;


	d = adapter_to_dvobj(adapter);

	bMacPwrCtrlOn = _FALSE;
	rtw_hal_get_hwreg(adapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
	if (bMacPwrCtrlOn == _FALSE) {
		goto out;
	}

	bMacPwrCtrlOn = _FALSE;
	rtw_hal_set_hwreg(adapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);

	GET_HAL_DATA(adapter)->bFWReady = _FALSE;

	err = rtw_halmac_poweroff(d);
	if (err) {
		RTW_ERR("%s: Power OFF Fail!!\n", __FUNCTION__);
		goto out;
	}

out:
	return;
}

static void rtl8730a_init_bcn_head(PADAPTER adapter)
{
#ifdef CONFIG_PORT_BASED_TXBCN
	u16 head;
	u32 page_size;
	u32 pgae_nr;

	rtw_hal_get_def_var(adapter, HAL_DEF_TX_PAGE_SIZE, &page_size);
	pgae_nr = MAX_BEACON_LEN / page_size;

	/* read HEAD from port 0, and then extend to port 1, 2, 3, ... */
	if (rtw_halmac_get_rsvd_drv_pg_bndy(adapter_to_dvobj(adapter), &head)) {
		/* if failed to read head from halmac, read head from register */
		u32 val;

		val = rtw_read32(adapter, REG_BCN_CTRL_0_8730A);
		head = BIT_GET_BCN0_HEAD_8730A(val);
	}

	/* port 1 */
	head += pgae_nr;
	rtw_write16(adapter, REG_BCN_CTRL_0_8730A + 2, head);
	rtw_write32(adapter, REG_BCNQ_BDNY_V2_8730A, (1 << 28) | head);

	/* port 2 */
	head += pgae_nr;
	rtw_write16(adapter, REG_BCN_CTRL_1_8730A, head);
	rtw_write32(adapter, REG_BCNQ_BDNY_V2_8730A, (2 << 28) | head);

	/* port 3 */
	head += pgae_nr;
	rtw_write16(adapter, REG_BCN_CTRL_1_8730A + 2, head);
	rtw_write32(adapter, REG_BCNQ_BDNY_V2_8730A, (3 << 28) | head);
#endif
}

u8 rtl8730a_hal_init(PADAPTER adapter)
{
	struct dvobj_priv *d;
	PHAL_DATA_TYPE hal;
	int err, err_get_dma_ch_map;

	d = adapter_to_dvobj(adapter);
	hal = GET_HAL_DATA(adapter);

	hal->bFWReady = _FALSE;
	hal->fw_ractrl = _FALSE;

	err = rtw_halmac_init_hal(d);
	if (err) {
		RTW_ERR("%s Enable Firmware failed\n", __FUNCTION__);
		goto post;
	}

	RTW_INFO("%s enable Firmware success\n", __FUNCTION__);
	RTW_INFO("%s FW Version:%d SubVersion:%d FW size:%d\n", "NIC",
		 hal->firmware_version, hal->firmware_sub_version, hal->firmware_size);

	/* Sync driver status with hardware setting */
	rtw_hal_get_hwreg(adapter, HW_VAR_RCR, NULL);
	hal->bFWReady = _TRUE;
	hal->fw_ractrl = _TRUE;
	hal->RxTag = 0;

post:
	err_get_dma_ch_map = rtw_halmac_get_tx_dma_ch_map(d, hal->dma_ch_map, sizeof(hal->dma_ch_map));
	if (err_get_dma_ch_map) {
		RTW_ERR("%s Get TX DMA channel map failed\n", __func__);
	}

	rtl8730a_init_bcn_head(adapter);

	if (err) {
		return _FALSE;
	}

	return _TRUE;
}

u8 rtl8730a_mac_verify(PADAPTER adapter)
{
	struct dvobj_priv *d;
	int err;


	d = adapter_to_dvobj(adapter);

	err = rtw_halmac_self_verify(d);
	if (err) {
		RTW_INFO("%s fail\n", __FUNCTION__);
		return _FALSE;
	}

	RTW_INFO("%s successful\n", __FUNCTION__);
	return _TRUE;
}

void rtl8730a_init_misc(PADAPTER adapter)
{
	PHAL_DATA_TYPE hal;
	u8 v8 = 0;
	u32 v32 = 0;
#ifdef RTW_AMPDU_AGG_RETRY_AND_NEW
	u32 ctrl;
#endif /* RTW_AMPDU_AGG_RETRY_AND_NEW */


	hal = GET_HAL_DATA(adapter);

	/*
	 * Sync driver status and hardware setting
	 */

	/* initial security setting */
	invalidate_cam_all(adapter);

	/* check RCR/ICV bit */
	rtw_hal_rcr_clear(adapter, BIT_ACRC32_8730A | BIT_AICV_8730A);

	/* clear rx ctrl frame */
	rtw_write16(adapter, REG_RXFLTMAP1_8730A, 0);

	/*Enable MAC security engine*/
	rtw_write16(adapter, REG_CR_8730A, (rtw_read16(adapter, REG_CR_8730A) | BIT_MAC_SEC_EN_8730A));

#ifdef CONFIG_TCP_CSUM_OFFLOAD_RX
	rtw_hal_rcr_add(adapter, BIT_TCPOFLD_EN_8730A);
#endif /* CONFIG_TCP_CSUM_OFFLOAD_RX*/

#ifdef RTW_AMPDU_AGG_RETRY_AND_NEW
	v32 = rtw_read32(adapter, REG_FWHW_TXQ_HWSSN_CTRL_8730A);
	ctrl = v32;
	/* Enable AMPDU aggregation mode with retry MPDUs and new MPDUs */
	v32 &= ~BIT_EN_RTY_BK_8730A;
	/* Don't agg if retry packet rate fall back */
	v32 |= (BIT_EN_RTY_BK_COD_8730A << 24); /* 0x423[2] */
	if (v32 != ctrl) {
		rtw_write32(adapter, REG_FWHW_TXQ_HWSSN_CTRL_8730A, v32);
	}

	RTW_INFO("%s: AMPDU agg retry with new/break when rate fall back: "
		 "%s / %s\n", __FUNCTION__,
		 (v32 & BIT_EN_RTY_BK_8730A) ? "false" : "true",
		 (v32 & BIT_EN_RTY_BK_COD_8730A) ? "true" : "false");
#endif /* RTW_AMPDU_AGG_RETRY_AND_NEW */

#ifdef CONFIG_LPS_PWR_TRACKING
	rtl8730a_set_fw_thermal_rpt_cmd(adapter, _TRUE, hal->eeprom_thermal_meter + THERMAL_DIFF_TH);
#endif

}

u32 rtl8730a_init(PADAPTER adapter)
{
	u8 ok = _TRUE;
	PHAL_DATA_TYPE hal;

	hal = GET_HAL_DATA(adapter);

	ok = rtl8730a_hal_init(adapter);
	if (_FALSE == ok) {
		return _FAIL;
	}

	rtl8730a_phy_init_haldm(adapter);
#ifdef CONFIG_BEAMFORMING
	rtl8730a_phy_bf_init(adapter);
#endif

#ifdef CONFIG_FW_MULTI_PORT_SUPPORT
	/*HW / FW init*/
	rtw_hal_set_default_port_id_cmd(adapter, 0);
#endif

#ifdef CONFIG_BT_COEXIST
	/* Init BT hw config. */
	if (_TRUE == hal->EEPROMBluetoothCoexist) {
		rtw_btcoex_HAL_Initialize(adapter, _FALSE);
#ifdef CONFIG_FW_MULTI_PORT_SUPPORT
		rtw_hal_set_wifi_btc_port_id_cmd(adapter);
#endif
	} else
#endif /* CONFIG_BT_COEXIST */
		rtw_btcoex_wifionly_hw_config(adapter);

	rtl8730a_init_misc(adapter);

	return _SUCCESS;
}

u32 rtl8730a_deinit(PADAPTER adapter)
{
	struct dvobj_priv *d;
	PHAL_DATA_TYPE hal;
	int err;


	d = adapter_to_dvobj(adapter);
	hal = GET_HAL_DATA(adapter);

	hal->bFWReady = _FALSE;
	hal->fw_ractrl = _FALSE;

	err = rtw_halmac_deinit_hal(d);
	if (err) {
		return _FAIL;
	}

	return _SUCCESS;
}

void rtl8730a_init_default_value(PADAPTER adapter)
{
	PHAL_DATA_TYPE hal;
	u8 i;


	hal = GET_HAL_DATA(adapter);

	/* init default value */
	hal->fw_ractrl = _FALSE;

	if (!adapter_to_pwrctl(adapter)->bkeepfwalive) {
		hal->LastHMEBoxNum = 0;
	}

	/* init phydm default value */
	hal->bIQKInitialized = _FALSE;

	/* init Efuse variables */
	hal->EfuseUsedBytes = 0;
	hal->EfuseUsedPercentage = 0;

	hal->EfuseHal.fakeEfuseBank = 0;
	hal->EfuseHal.fakeEfuseUsedBytes = 0;
	_rtw_memset(hal->EfuseHal.fakeEfuseContent, 0xFF, EFUSE_MAX_HW_SIZE);
	_rtw_memset(hal->EfuseHal.fakeEfuseInitMap, 0xFF, EFUSE_MAX_MAP_LEN);
	_rtw_memset(hal->EfuseHal.fakeEfuseModifiedMap, 0xFF, EFUSE_MAX_MAP_LEN);
	hal->EfuseHal.BTEfuseUsedBytes = 0;
	hal->EfuseHal.BTEfuseUsedPercentage = 0;
	_rtw_memset(hal->EfuseHal.BTEfuseContent, 0xFF, EFUSE_MAX_BT_BANK * EFUSE_MAX_HW_SIZE);
	_rtw_memset(hal->EfuseHal.BTEfuseInitMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset(hal->EfuseHal.BTEfuseModifiedMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	hal->EfuseHal.fakeBTEfuseUsedBytes = 0;
	_rtw_memset(hal->EfuseHal.fakeBTEfuseContent, 0xFF, EFUSE_MAX_BT_BANK * EFUSE_MAX_HW_SIZE);
	_rtw_memset(hal->EfuseHal.fakeBTEfuseInitMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset(hal->EfuseHal.fakeBTEfuseModifiedMap, 0xFF, EFUSE_BT_MAX_MAP_LEN);

}
