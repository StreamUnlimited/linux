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

#include "hci_fc.h"
#include "mac_priv.h"

static u32 chcfg_size = sizeof(struct mac_ax_hfc_ch_cfg) * MAC_AX_DMA_CH_NUM;

static u32 chinfo_size = sizeof(struct mac_ax_hfc_ch_info) * MAC_AX_DMA_CH_NUM;

u32 hfc_info_init(struct mac_ax_adapter *adapter)
{
	adapter->hfc_param =
		(struct mac_ax_hfc_param *)
		PLTFM_MALLOC(sizeof(struct mac_ax_hfc_param));

	adapter->hfc_param->ch_cfg =
		(struct mac_ax_hfc_ch_cfg *)PLTFM_MALLOC(chcfg_size);

	adapter->hfc_param->ch_info =
		(struct mac_ax_hfc_ch_info *)PLTFM_MALLOC(chinfo_size);

	adapter->hfc_param->pub_cfg =
		(struct mac_ax_hfc_pub_cfg *)
		PLTFM_MALLOC(sizeof(struct mac_ax_hfc_pub_cfg));

	adapter->hfc_param->pub_info =
		(struct mac_ax_hfc_pub_info *)
		PLTFM_MALLOC(sizeof(struct mac_ax_hfc_pub_info));

	adapter->hfc_param->prec_cfg =
		(struct mac_ax_hfc_prec_cfg *)
		PLTFM_MALLOC(sizeof(struct mac_ax_hfc_prec_cfg));

	adapter->hfc_param->en = 0;
	adapter->hfc_param->h2c_en = 0;
	adapter->hfc_param->mode = 0;

	return MACSUCCESS;
}

u32 hfc_info_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_FREE(adapter->hfc_param->prec_cfg,
		   sizeof(struct mac_ax_hfc_prec_cfg));

	PLTFM_FREE(adapter->hfc_param->pub_info,
		   sizeof(struct mac_ax_hfc_pub_info));

	PLTFM_FREE(adapter->hfc_param->pub_cfg,
		   sizeof(struct mac_ax_hfc_pub_cfg));

	PLTFM_FREE(adapter->hfc_param->ch_info, chinfo_size);
	PLTFM_FREE(adapter->hfc_param->ch_cfg, chcfg_size);
	PLTFM_FREE(adapter->hfc_param, sizeof(struct mac_ax_hfc_param));

	return MACSUCCESS;
}

