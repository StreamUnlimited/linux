/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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
#include "h2c_agg.h"

static u32 mac_h2c_agg_tx_single_normal_h2c(struct mac_ax_adapter *adapter, u8 *h2cb)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 mac_h2c_agg_tx_single_agg_h2c(struct mac_ax_adapter *adapter, u8 *agg_h2cb)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

void mac_h2c_agg_enable(struct mac_ax_adapter *adapter, u8 enable)
{
	PLTFM_MUTEX_LOCK(&adapter->h2c_agg_info.h2c_agg_lock);
	adapter->h2c_agg_info.h2c_agg_en = enable;
	PLTFM_MUTEX_UNLOCK(&adapter->h2c_agg_info.h2c_agg_lock);
}

u32 mac_h2c_agg_tx(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

void mac_h2c_agg_flush(struct mac_ax_adapter *adapter)
{
	struct mac_ax_h2c_agg_node *cur_agg_node = NULL;
	struct mac_ax_h2c_agg_node *tmp_agg_node = NULL;

	PLTFM_MUTEX_LOCK(&adapter->h2c_agg_info.h2c_agg_lock);
	cur_agg_node = adapter->h2c_agg_info.h2c_agg_queue_head;

	while (cur_agg_node) {
#if MAC_AX_PHL_H2C
		PLTFM_RECYCLE_H2C((struct rtw_h2c_pkt *)cur_agg_node->h2c_pkt);
#else
		h2cb_free(adapter, (struct h2c_buf *)cur_agg_node->h2c_pkt);
#endif
		tmp_agg_node = cur_agg_node;
		cur_agg_node = cur_agg_node->next;
		PLTFM_FREE(tmp_agg_node, sizeof(struct mac_ax_h2c_agg_node));
	}

	adapter->h2c_agg_info.h2c_agg_queue_head = NULL;
	adapter->h2c_agg_info.h2c_agg_queue_last = NULL;
	adapter->h2c_agg_info.h2c_agg_pkt_num = 0;
	PLTFM_MUTEX_UNLOCK(&adapter->h2c_agg_info.h2c_agg_lock);
}
