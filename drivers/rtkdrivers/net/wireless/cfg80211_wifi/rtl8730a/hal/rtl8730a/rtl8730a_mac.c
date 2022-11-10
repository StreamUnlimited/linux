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
#define _RTL8730A_MAC_C_

#include <drv_types.h>		/* PADAPTER, basic_types.h and etc. */
#include <hal_data.h>		/* HAL_DATA_TYPE */
#include "../hal_halmac.h"	/* Register Definition and etc. */
#include "rtl8730a.h"		/* FW array */


inline u8 rtl8730a_rcr_config(PADAPTER p, u32 rcr)
{
	u32 v32;
	int err;


	v32 = GET_HAL_DATA(p)->ReceiveConfig;
	v32 ^= rcr;
	v32 &= BIT_APP_PHYSTS_8730A;
	if (v32) {
		v32 = rcr & BIT_APP_PHYSTS_8730A;
		RTW_INFO("%s: runtime %s rx phy status!\n",
			 __FUNCTION__, v32 ? "ENABLE" : "DISABLE");
		if (v32) {
			err = rtw_halmac_config_rx_info(adapter_to_dvobj(p), HALMAC_DRV_INFO_PHY_STATUS);
			if (err) {
				RTW_INFO("%s: Enable rx phy status FAIL!!", __FUNCTION__);
				rcr &= ~BIT_APP_PHYSTS_8730A;
			}
		} else {
			err = rtw_halmac_config_rx_info(adapter_to_dvobj(p), HALMAC_DRV_INFO_NONE);
			if (err) {
				RTW_INFO("%s: Disable rx phy status FAIL!!", __FUNCTION__);
				rcr |= BIT_APP_PHYSTS_8730A;
			}
		}
	}

	err = rtw_write32(p, REG_RCR_8730A, rcr);
	if (_FAIL == err) {
		return _FALSE;
	}

	GET_HAL_DATA(p)->ReceiveConfig = rcr;
	return _TRUE;
}

inline u8 rtl8730a_rx_ba_ssn_appended(PADAPTER p)
{
	return rtw_hal_rcr_check(p, BIT_APP_BASSN_8730A);
}

inline u8 rtl8730a_rx_fcs_append_switch(PADAPTER p, u8 enable)
{
	u32 rcr_bit;
	u8 ret = _TRUE;

	rcr_bit = BIT_APP_FCS_8730A;
	if (_TRUE == enable) {
		ret = rtw_hal_rcr_add(p, rcr_bit);
	} else {
		ret = rtw_hal_rcr_clear(p, rcr_bit);
	}

	return ret;
}

inline u8 rtl8730a_rx_fcs_appended(PADAPTER p)
{
	return rtw_hal_rcr_check(p, BIT_APP_FCS_8730A);
}

inline u8 rtl8730a_rx_tsf_addr_filter_config(PADAPTER p, u8 config)
{
	u8 v8;
	int err;

	v8 = GET_HAL_DATA(p)->rx_tsf_addr_filter_config;

	if (v8 != config) {

		err = rtw_write8(p, REG_NAN_RX_FILTER_8730A, config);
		if (_FAIL == err) {
			return _FALSE;
		}
	}

	GET_HAL_DATA(p)->rx_tsf_addr_filter_config = config;
	return _TRUE;
}

u8 rtl8730a_get_rx_drv_info_size(struct _ADAPTER *a)
{
	struct dvobj_priv *d;
	u8 size = 80;	/* HALMAC_RX_DESC_DUMMY_SIZE_MAX_88XX */
	int err = 0;


	d = adapter_to_dvobj(a);

	err = rtw_halmac_get_rx_drv_info_sz(d, &size);
	if (err) {
		RTW_WARN(FUNC_ADPT_FMT ": Fail to get DRV INFO size!!(err=%d)\n",
			 FUNC_ADPT_ARG(a), err);
		size = 80;
	}

	return size;
}

u32 rtl8730a_get_tx_desc_size(struct _ADAPTER *a)
{
	struct dvobj_priv *d;
	u32 size = 40;	/* HALMAC_TX_DESC_SIZE_8730A */
	int err = 0;


	d = adapter_to_dvobj(a);

	err = rtw_halmac_get_tx_desc_size(d, &size);
	if (err) {
		RTW_WARN(FUNC_ADPT_FMT ": Fail to get TX Descriptor size!!(err=%d)\n",
			 FUNC_ADPT_ARG(a), err);
		size = 48;
	}

	return size;
}

u32 rtl8730a_get_rx_desc_size(struct _ADAPTER *a)
{
	struct dvobj_priv *d;
	u32 size = 24;	/* HALMAC_RX_DESC_SIZE_8730A */
	int err = 0;


	d = adapter_to_dvobj(a);

	err = rtw_halmac_get_rx_desc_size(d, &size);
	if (err) {
		RTW_WARN(FUNC_ADPT_FMT ": Fail to get RX Descriptor size!!(err=%d)\n",
			 FUNC_ADPT_ARG(a), err);
		size = 24;
	}

	return size;
}

/*
 * _rx_report - Get/Reset RX counter report
 * @a:		struct _ADAPTER*
 * @type:	rx report type
 * @reset:	reset counter or not
 *		0: read counter and don't reset counter
 *		1: reset counter only
 *
 * Get/Reset RX (error) report counter from hardware.
 *
 * Rteurn counter when reset==0, otherwise always return 0.
 */
u16 _rx_report(struct _ADAPTER *a, enum rx_rpt_type type, u8 reset)
{
	u32 sel = 0;
	u16 counter = 0;

	/* Rx packet counter report selection */
	sel = BIT_RXERR_RPT_SEL_V1_3_0_8730A(type);
	if (type & BIT(4)) {
		sel |= BIT_RXERR_RPT_SEL_V1_4_8730A;
	}

	if (reset) {
		sel |= BIT_RXERR_RPT_RST_8730A;
	}

	rtw_write8(a, REG_RXERR_RPT_8730A + 3, (sel >> 24) & 0xFF);

	if (!reset) {
		counter = rtw_read16(a, REG_RXERR_RPT_8730A);
	}

	return counter;
}

/**
 * rtl8730a_rx_report_get - Get RX counter report
 * @a:		struct _ADAPTER*
 * @type:	rx report type
 *
 * Get RX (error) report counter from hardware.
 *
 * Rteurn counter for specific rx report.
 */
u16 rtl8730a_rx_report_get(struct _ADAPTER *a, enum rx_rpt_type type)
{
	return _rx_report(a, type, 0);
}

/**
 * rtl8730a_rx_report_reset - Reset RX counter report
 * @a:		struct _ADAPTER*
 *
 * Reset RX (error) report counter of hardware.
 */
void rtl8730a_rx_report_reset(struct _ADAPTER *a, enum rx_rpt_type type)
{
	_rx_report(a, type, 1);
}
