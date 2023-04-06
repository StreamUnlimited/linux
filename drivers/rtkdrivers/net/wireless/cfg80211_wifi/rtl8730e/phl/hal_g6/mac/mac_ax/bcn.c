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

#include "bcn.h"
#include "mcc.h"
#include "mac_priv.h"
#include "twt.h"
#include "mac/pltfm_cfg.h"
#include "hal_api.h"

#if MAC_AX_FEATURE_HV
#include "../hv_ax/dbgpkg_hv.h"
#endif

#ifdef RTW_PHL_BCN_IOT
u32 mac_send_bcn(struct mac_ax_adapter *adapter, struct mac_ax_bcn_info *info)
{
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)(adapter->phl_adapter);
	struct rtw_hal_com_t *hal_com = (struct rtw_hal_com_t *)(adapter->drv_adapter);
	void *d = phlcom_to_drvpriv(phl_com);
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	struct rtw_t_meta_data mdata = {0};
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct mac_ax_bcn_priv *bcn_info = NULL;
	u32 desc_len = 0, addr = 0, page_num = 0;
	u8 *pdesc = NULL;
	u32 mac_result = MACSUCCESS;

	bcn_info = (struct mac_ax_bcn_priv *)phl_com->bcn_info;
	if (!bcn_info) {
		PLTFM_MSG_ERR("bcn_info is NULL.\n");
		return MACNPTR;
	}

	_os_spinlock(d, &bcn_info->lock, _bh, NULL);
	mdata.type = RTW_PHL_PKT_TYPE_MGNT;
	mdata.offset = TX_WIFI_INFO_SIZE;
	mdata.macid = info->macid;
	mdata.mbssid = info->mbssid;
	mdata.band = info->band;
	mdata.qsel = AX_TXDESC_QSEL_BCN;
	mdata.bc = 1;
	mdata.pktlen = info->pld_len;
	mdata.port_id = info->port;
	mdata.userate_sel = 1;
	mdata.f_rate = info->rate_sel;
	mdata.hw_seq_mode = info->ssn_mode;
	mdata.sw_seq = info->ssn_sel;
	mdata.wdinfo_en = 1;
	desc_len = adapter->ops->txdesc_len(adapter, NULL);
	if (desc_len + info->pld_len >= 4 * 128) {
		PLTFM_MSG_ERR("bcn size is exceed!\n");
		mac_result = MACBUFSZ;
		goto func_exit;
	}

	pdesc = PLTFM_MALLOC(desc_len);
	if (pdesc == NULL) {
		PLTFM_MSG_ERR("allocate tx desc failed!\n");
		mac_result = MACBUFALLOC;
		goto func_exit;
	}

	hal_status = adapter->ops->build_txdesc(adapter, &mdata,
						 pdesc, desc_len);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PLTFM_MSG_ERR("fill tx desc failed!\n");
		mac_result = MACNOBUF;
		goto free_desc;
	}

	if (info->port == MAC_AX_PORT_0) {
		page_num = txff_info->rsvd_bcnq_addr;
	} else {
		page_num = txff_info->rsvd_bcnq1_addr;
	}

	addr = TX_PKTBUF_OFFSET + page_num * 128;
	PHL_TRACE(COMP_PHL_MAC, _PHL_INFO_,
		  "write bcn pkt to address 0x%06x of tx pkt buffer, page %d\n",
		  addr, page_num);
	PLTFM_MEM_W(addr, pdesc, desc_len);
	PLTFM_MEM_W(addr + desc_len, info->pld_buf, info->pld_len);

	if (info->port == MAC_AX_PORT_0) {
		rtw_hal_config_interrupt(hal_com->hal_priv, RTW_PHL_EN_P0_AP_MODE_INT);
	} else {
		rtw_hal_config_interrupt(hal_com->hal_priv, RTW_PHL_EN_P1_AP_MODE_INT);
	}

	mac_result = MACSUCCESS;
free_desc:
	PLTFM_FREE(pdesc, desc_len);
func_exit:
	_os_spinunlock(d, &bcn_info->lock, _bh, NULL);

	return mac_result;
}
#endif