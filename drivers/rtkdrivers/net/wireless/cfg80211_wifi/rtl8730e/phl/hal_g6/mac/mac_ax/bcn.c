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

#if MAC_AX_FEATURE_HV
#include "../hv_ax/dbgpkg_hv.h"
#endif

#ifdef RTW_PHL_BCN_IOT
u32 mac_send_bcn(struct mac_ax_adapter *adapter, struct mac_ax_bcn_info *info)
{
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)(adapter->phl_adapter);
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;
	struct mac_ax_bcn_priv *bcn_info = NULL;
	void *d = phlcom_to_drvpriv(phl_com);

	bcn_info = (struct mac_ax_bcn_priv *)phl_com->bcn_info;
	if (!bcn_info) {
		PLTFM_MSG_ERR("bcn_info is NULL.\n");
		return MACNPTR;
	}

	_os_spinlock(d, &bcn_info->lock, _bh, NULL);
	bcn_info->port = info->port;
	bcn_info->mbssid = info->mbssid;
	bcn_info->band = info->band;
	bcn_info->grp_ie_ofst = info->grp_ie_ofst;
	bcn_info->macid = info->macid;
	bcn_info->ssn_sel = info->ssn_sel;
	bcn_info->ssn_mode = info->ssn_mode;
	bcn_info->rate_sel = info->rate_sel;
	bcn_info->txpwr = info->txpwr;
	bcn_info->txinfo_ctrl_en = info->txinfo_ctrl_en;
	bcn_info->ntx_path_en = info->ntx_path_en;
	bcn_info->path_map_a = info->path_map_a;
	bcn_info->path_map_b = info->path_map_b;
	bcn_info->path_map_c = info->path_map_c;
	bcn_info->path_map_d = info->path_map_d;
	bcn_info->antsel_a = info->antsel_a;
	bcn_info->antsel_b = info->antsel_b;
	bcn_info->antsel_c = info->antsel_c;
	bcn_info->antsel_d = info->antsel_d;
	bcn_info->sw_tsf = info->sw_tsf;
	bcn_info->csa_ofst = info->csa_ofst;
	_os_mem_cpy(phlcom_to_drvpriv(phl_com), bcn_info->bcn_vir_addr,
		    info->pld_buf, info->pld_len);
	bcn_info->bcn_len = info->pld_len;
	bcn_info->tx_bcn = true;
	_os_spinunlock(d, &bcn_info->lock, _bh, NULL);

	return MACSUCCESS;
}
#endif