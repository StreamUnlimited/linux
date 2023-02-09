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

#include "fwdl.h"
#include "fwofld.h"
#include "mac/mac_def.h"
#include "hal_headers_le.h"
#include "drv_types.h"
#include "hal_struct.h"
#include "rtw_security.h"

static u32 get_io_ofld_cap(struct mac_ax_adapter *adapter, u32 *val)
{
	*val |= FW_CAP_IO_OFLD;

	return MACSUCCESS;
}

u32 mac_get_fw_cap(struct mac_ax_adapter *adapter, u32 *val)
{
	*val = 0;
	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACFWNONRDY;
	}

	get_io_ofld_cap(adapter, val);

	return MACSUCCESS;
}

u32 mac_reset_fwofld_state(struct mac_ax_adapter *adapter, u8 op)
{
	switch (op) {
	case FW_OFLD_OP_DUMP_EFUSE:
		adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_PACKET_OFLD:
		adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_READ_OFLD:
		adapter->sm.read_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.read_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_WRITE_OFLD:
		adapter->sm.write_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.write_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_CONF_OFLD:
		adapter->sm.conf_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.conf_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;
	case FW_OFLD_OP_CH_SWITCH:
		adapter->sm.ch_switch = MAC_AX_OFLD_H2C_IDLE;
		break;

	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

static u32 cnv_write_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.write_request;

	if (state > MAC_AX_OFLD_REQ_CLEANED) {
		return MACPROCERR;
	}

	if (dest == MAC_AX_OFLD_REQ_IDLE) {
		if (state != MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_CLEANED) {
		if (state == MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_CREATED) {
		if (state == MAC_AX_OFLD_REQ_IDLE ||
		    state == MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_H2C_SENT) {
		if (state != MAC_AX_OFLD_REQ_CREATED) {
			return MACPROCERR;
		}
	}

	adapter->sm.write_request = dest;

	return MACSUCCESS;
}

u32 mac_write_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_conf_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static void
__mac_pkt_ofld_fill_txd(struct mac_ax_adapter *adapter,
			u8 *buf, u16 len, u8 type)
{
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)adapter->phl_adapter;
	struct dvobj_priv *pdvobj = (struct dvobj_priv *)(phl_com->drv_priv);
	struct _ADAPTER *drv_adapter = dvobj_get_primary_adapter(pdvobj);
	struct security_priv *psecuritypriv = &drv_adapter->securitypriv;
	struct rtw_t_meta_data mdata = {0};
	u32 txd_len = 0;

	txd_len = adapter->ops->txdesc_len(adapter, &mdata);
	mdata.offset = txd_len;
	mdata.pktlen = len;
	mdata.qsel = RTW_TX_QSEL_MGT;
	mdata.type = RTW_PHL_PKT_TYPE_MGNT;

	if (type == PKT_TYPE_PS_POLL) {
		mdata.nav_use_hdr = 1;
	} else {
		mdata.hw_seq_mode = 1;
		mdata.hw_ssn_sel = 0;
		mdata.spe_rpt = 1;
	}

	if (type == PKT_TYPE_BT_QOS_NULL) {
		mdata.bt_int = 1;
	}

	mdata.userate_sel = 1;
	mdata.f_rate = RTW_DATA_RATE_CCK1;

	if (type == PKT_TYPE_ARP_RSP) {
		switch (psecuritypriv->dot11PrivacyAlgrthm) {
		case _NO_PRIVACY_:
			mdata.sec_hw_enc = 0;
			mdata.sec_type = RTW_ENC_NONE;
			break;
		case _WEP40_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_WEP40;
			break;
		case _TKIP_:
		case _TKIP_WTMIC_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_TKIP;
			break;
		case _AES_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_CCMP;
			break;
		case _WEP104_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_WEP104;
			break;
		case _SMS4_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_GCMSMS4;
			break;
		case _GCMP_:
			mdata.sec_hw_enc = 1;
			mdata.sec_type = RTW_ENC_GCMP;
			break;
		default:
			mdata.sec_hw_enc = 0;
			mdata.sec_type = RTW_ENC_NONE;
			break;
		}
	}

	adapter->ops->build_txdesc(adapter, &mdata, buf, txd_len);
}

u32 mac_read_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;
	struct mac_ax_pkt_ofld_pkt *ofld_pkt = &adapter->pkt_ofld_pkt;
	u32 addr = 0;
	u8 buf[128] = {0};
	u32 txd_len = 0;
	u8 *pkt_buff = NULL;

	if (id == 0) {
		PHL_TRACE(COMP_PHL_PKTOFLD, _PHL_ERR_,
			  "id must be nonzore!\n");
		return MACNOTSUP;
	}

	txd_len = adapter->ops->txdesc_len(adapter, NULL);
	addr = MAC_TX_PKTBUF_OFFSET + (txff_info->rsvd_bcnq_addr + id) * 128;
	PLTFM_MEM_W(addr, buf, 128);
	ofld_pkt->pkt_len = (u16)buf[0] | ((u16)buf[1] << 8);
	if (ofld_pkt->pkt_len) {
		if (ofld_pkt->pkt) {
			PLTFM_FREE(ofld_pkt->pkt, ofld_pkt->pkt_len);
		}

		pkt_buff = (u8 *)PLTFM_MALLOC(ofld_pkt->pkt_len);
		if (!pkt_buff) {
			return MACBUFALLOC;
		}

		PLTFM_MEMCPY(pkt_buff, buf + txd_len, ofld_pkt->pkt_len);
		ofld_pkt->pkt_id = id;
		ofld_pkt->pkt = pkt_buff;
	}

	ofld_info->last_op = PKT_OFLD_OP_READ;

	return MACSUCCESS;
}

u32 mac_del_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;
	u32 addr = 0;
	u8 buf[128] = {0};

	if (id == 0) {
		PHL_TRACE(COMP_PHL_PKTOFLD, _PHL_ERR_,
			  "id must be nonzore!\n");
		return MACNOTSUP;
	}

	addr = MAC_TX_PKTBUF_OFFSET + (txff_info->rsvd_bcnq_addr + id) * 128;
	PLTFM_MEMSET(buf, 0, 128);
	PLTFM_MEM_W(addr, buf, 128);

	ofld_info->last_op = PKT_OFLD_OP_DEL;

	return MACSUCCESS;
}

u32 mac_add_pkt_ofld(struct mac_ax_adapter *adapter, u8 *pkt, u16 len, u8 *id,
		     u8 type)
{
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;
	u32 addr = 0;
	u32 page_num = 0;
	u32 txd_len = 0;
	u8 *ptxd = NULL;

	switch (type) {
	case PKT_TYPE_NULL_DATA:
		page_num = ofld_info->nulldata_page;
		break;
	case PKT_TYPE_BT_QOS_NULL:
		page_num = ofld_info->btqosnull_page;
		break;
	case PKT_TYPE_QOS_NULL:
		page_num = ofld_info->qosnull_page;;
		break;
	default:
		page_num = 0;
		break;
	}

	if (page_num == 0) {
		PHL_TRACE(COMP_PHL_PKTOFLD, _PHL_ERR_,
			  "unsupported type %d.\n",
			  type);
		return MACNOTSUP;
	}

	txd_len = adapter->ops->txdesc_len(adapter, NULL);
	ptxd = PLTFM_MALLOC(txd_len);
	if (ptxd == NULL) {
		return MACBUFALLOC;
	}
	__mac_pkt_ofld_fill_txd(adapter, ptxd, len, type);

	addr = MAC_TX_PKTBUF_OFFSET + (txff_info->rsvd_bcnq_addr + page_num) * 128;

	PHL_TRACE(COMP_PHL_PKTOFLD, _PHL_INFO_,
		  "write pkt to address 0x%06x of tx pkt buffer, page %d\n",
		 addr, txff_info->rsvd_bcnq_addr + page_num);
	PLTFM_MEM_W(addr, ptxd, txd_len);
	PLTFM_MEM_W(addr + txd_len, pkt, len);
	*id = page_num;

	if (ptxd) {
		PLTFM_FREE(ptxd, txd_len);
	}

	ofld_info->last_op = PKT_OFLD_OP_ADD;

	return MACSUCCESS;
}

u32 mac_pkt_ofld_packet(struct mac_ax_adapter *adapter,
			u8 **pkt_buf, u16 *pkt_len, u8 *pkt_id)
{
	struct mac_ax_pkt_ofld_pkt *pkt_info = &adapter->pkt_ofld_pkt;
	*pkt_buf = NULL;

	*pkt_buf = (u8 *)PLTFM_MALLOC(pkt_info->pkt_len);
	if (!*pkt_buf) {
		return MACBUFALLOC;
	}

	PLTFM_MEMCPY(*pkt_buf, pkt_info->pkt, pkt_info->pkt_len);

	*pkt_len = pkt_info->pkt_len;
	*pkt_id = pkt_info->pkt_id;

	adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;

	return MACSUCCESS;
}

u32 mac_dump_efuse_ofld(struct mac_ax_adapter *adapter, u32 efuse_size,
			bool is_hidden)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 cnv_read_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.read_request;

	if (state > MAC_AX_OFLD_REQ_CLEANED) {
		return MACPROCERR;
	}

	if (dest == MAC_AX_OFLD_REQ_IDLE) {
		if (state != MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_CLEANED) {
		if (state == MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_CREATED) {
		if (state == MAC_AX_OFLD_REQ_IDLE ||
		    state == MAC_AX_OFLD_REQ_H2C_SENT) {
			return MACPROCERR;
		}
	} else if (dest == MAC_AX_OFLD_REQ_H2C_SENT) {
		if (state != MAC_AX_OFLD_REQ_CREATED) {
			return MACPROCERR;
		}
	}

	adapter->sm.read_request = dest;

	return MACSUCCESS;
}

u32 mac_general_pkt_ids(struct mac_ax_adapter *adapter,
			struct mac_ax_general_pkt_ids *ids)
{
	u8 *buf;
	u32 ret;
	struct fwcmd_rsvdpage fwcmd_tbl;
	struct fwcmd_general_pkt *write_ptr;
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;

	fwcmd_tbl.cmd_id = FWCMD_H2C_RSVDPAGE_CMDID;
	fwcmd_tbl.data0 = ids->probersp;
	fwcmd_tbl.data1 = ids->pspoll;
	fwcmd_tbl.data2 = ids->nulldata;
	fwcmd_tbl.data3 = ids->qosnull;
	fwcmd_tbl.data4 = ids->btqosnull;
	fwcmd_tbl.data5 = ids->cts2self;
	fwcmd_tbl.data6 = 0; // unknown here.
	ret = rtw_hal_mac_send_h2c_ameba(hal_com, fwcmd_tbl.cmd_id, sizeof(struct fwcmd_rsvdpage) - 1, &fwcmd_tbl.data0);

	return ret;
}

u32 mac_add_cmd_ofld(struct mac_ax_adapter *adapter, struct rtw_mac_cmd *cmd)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 write_mac_reg_ofld(struct mac_ax_adapter *adapter,
		       u16 offset, u32 mask, u32 val, u8 lc)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_WRITE_OFLD,
		       0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0
	};

	cmd.offset = offset;
	cmd.mask = mask;
	cmd.value = val;
	cmd.lc = lc;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 poll_mac_reg_ofld(struct mac_ax_adapter *adapter,
		      u16 offset, u32 mask, u32 val, u8 lc)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_COMPARE_OFLD,
		       0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0
	};

	cmd.offset = offset;
	cmd.mask = mask;
	cmd.value = val;
	cmd.lc = lc;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 delay_ofld(struct mac_ax_adapter *adapter,
	       u32 val, u8 lc)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_DELAY_OFLD,
		       0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0
	};

	cmd.value = val;
	cmd.lc = lc;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 mac_ccxrpt_parsing(struct mac_ax_adapter *adapter, u8 *buf, struct mac_ax_ccxrpt *info)
{
	u32 val_d0;
	u32 val_d3;
	u32 dword0 = *((u32 *)buf);
	u32 dword3 = *((u32 *)(buf + 12));

	val_d0 = le32_to_cpu(dword0);
	val_d3 = le32_to_cpu(dword3);
	info->tx_state = GET_FIELD(val_d0, TXCCXRPT_TX_STATE);
	info->sw_define = GET_FIELD(val_d0, TXCCXRPT_SW_DEFINE);
	info->macid = GET_FIELD(val_d0, TXCCXRPT_MACID);
	info->pkt_ok_num = GET_FIELD(val_d3, TXCCXRPT_PKT_OK_NUM);
	info->data_txcnt = GET_FIELD(val_d3, TXCCXRPT_DATA_TX_CNT);

	return MACSUCCESS;
}

static inline u8 scanofld_ch_list_len(struct scan_chinfo_list *list)
{
	return list->size;
}

static inline void scanofld_ch_list_init(struct scan_chinfo_list *list)
{
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

static inline u32 scanofld_ch_list_insert_head(struct mac_ax_adapter *adapter,
		struct scan_chinfo_list *list,
		struct mac_ax_scanofld_chinfo *chinfo)
{
	struct scanofld_chinfo_node *node;

	node = (struct scanofld_chinfo_node *)PLTFM_MALLOC(sizeof(struct scanofld_chinfo_node));
	if (!node) {
		return MACNOBUF;
	}
	node->next = list->head;
	if (list->size == 0) {
		list->tail = node;
	}
	list->size++;
	list->head = node;
	node->chinfo = chinfo;
	return MACSUCCESS;
}

static inline u32 scanofld_ch_list_insert_tail(struct mac_ax_adapter *adapter,
		struct scan_chinfo_list *list,
		struct mac_ax_scanofld_chinfo *chinfo)
{
	struct scanofld_chinfo_node *node;

	node = (struct scanofld_chinfo_node *)PLTFM_MALLOC(sizeof(struct scanofld_chinfo_node));

	if (!node) {
		return MACNOBUF;
	}
	if (list->size == 0) {
		list->head = node;
	} else {
		list->tail->next = node;
	}

	list->tail = node;
	node->chinfo = chinfo;
	node->next = NULL;
	list->size++;
	return MACSUCCESS;
}

static inline void scanofld_ch_node_print(struct mac_ax_adapter *adapter,
		struct scanofld_chinfo_node *curr_node, u8 i)
{
	struct mac_ax_scanofld_chinfo *chinfo = curr_node->chinfo;

	PLTFM_MSG_TRACE("[scnofld][CH %d] - DWORD 0:%x\n", i, *((u32 *)(chinfo)));
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- period = %d\n", i, chinfo->period);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- dwell_time = %d\n", i, chinfo->dwell_time);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- central_ch = %d\n", i, chinfo->central_ch);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- pri_ch = %d\n", i, chinfo->pri_ch);
	PLTFM_MSG_TRACE("[scnofld][CH %d] - DWORD 1:%x\n", i, *((u32 *)(chinfo) + 1));
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- bw = %d\n", i, chinfo->bw);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- noti_dwell = %d\n", i, chinfo->c2h_notify_dwell);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- noti_preTX = %d\n", i, chinfo->c2h_notify_preTX);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- noti_postTX = %d\n", i, chinfo->c2h_notify_postTX);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- noti_leaveCh = %d\n", i, chinfo->c2h_notify_leaveCH);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- noti_enterCh = %d\n", i, chinfo->c2h_notify_enterCH);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- numAddtionPkt = %d\n", i, chinfo->num_addition_pkt);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- tx_pkt = %d\n", i, chinfo->tx_pkt);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- pause_tx_data = %d\n", i, chinfo->pause_tx_data);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- ch_band = %d\n", i, chinfo->ch_band);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- probe_req_pkt = %d\n", i, chinfo->probe_req_pkt_id);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- dfs_ch = %d\n", i, chinfo->dfs_ch);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- tx_null = %d\n", i, chinfo->tx_null);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- rand_seq_num = %d\n", i, chinfo->rand_seq_num);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- cfg_tx_pwr = %d\n", i, chinfo->cfg_tx_pwr);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- rsvd0 = %d\n", i, chinfo->rsvd0);
	PLTFM_MSG_TRACE("[scnofld][CH %d] - DWORD 2:%x\n", i, *((u32 *)(chinfo) + 2));
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 0 = %d\n", i, chinfo->additional_pkt_id[0]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 1 = %d\n", i, chinfo->additional_pkt_id[1]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 2 = %d\n", i, chinfo->additional_pkt_id[2]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 3 = %d\n", i, chinfo->additional_pkt_id[3]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] - DWORD 3:%x\n", i, *((u32 *)(chinfo) + 3));
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 4 = %d\n", i, chinfo->additional_pkt_id[4]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 5 = %d\n", i, chinfo->additional_pkt_id[5]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 6 = %d\n", i, chinfo->additional_pkt_id[6]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- id 7 = %d\n", i, chinfo->additional_pkt_id[7]);
	PLTFM_MSG_TRACE("[scnofld][CH %d] - DWORD 4:%x\n", i, *((u32 *)(chinfo) + 4));
	PLTFM_MSG_TRACE("[scnofld][CH %d] -- tx_pwr_idx = %d\n", i, chinfo->tx_pwr_idx);
}

static inline void scanofld_ch_list_print(struct mac_ax_adapter *adapter,
		struct scan_chinfo_list *list, u8 verbose)
{
	struct scanofld_chinfo_node *curr_node = list->head;
	u8 i = 0;

	PLTFM_MSG_TRACE("[scnofld]------------------------------------------\n");
	PLTFM_MSG_TRACE("[scnofld][CH List] len = %d\n", list->size);

	while (verbose && curr_node) {
		scanofld_ch_node_print(adapter, curr_node, i);
		PLTFM_MSG_TRACE("\n");
		curr_node = curr_node->next;
		i++;
	}
	PLTFM_MSG_TRACE("[scnofld]------------------------------------------\n\n");
}

void mac_scanofld_ch_list_clear(struct mac_ax_adapter *adapter,
				struct scan_chinfo_list *list)
{
	struct scanofld_chinfo_node *curr_node = list->head;
	struct scanofld_chinfo_node *tmp;

	while (curr_node) {
		tmp = curr_node;
		curr_node = curr_node->next;
		PLTFM_FREE(tmp->chinfo, sizeof(struct mac_ax_scanofld_chinfo));
		PLTFM_FREE(tmp, sizeof(struct scanofld_chinfo_node));
		list->size--;
	}
	list->head = NULL;
	list->tail = NULL;
	scanofld_ch_list_print(adapter, list, 0);
}

void mac_scanofld_reset_state(struct mac_ax_adapter *adapter)
{
	struct mac_ax_scanofld_info *scanofld_info;

	scanofld_info = &adapter->scanofld_info;

	PLTFM_MUTEX_LOCK(&scanofld_info->drv_chlist_state_lock);
	scanofld_info->drv_chlist_busy = 0;
	PLTFM_MUTEX_UNLOCK(&scanofld_info->drv_chlist_state_lock);

	PLTFM_MUTEX_LOCK(&scanofld_info->fw_chlist_state_lock);
	scanofld_info->fw_chlist_busy = 0;
	PLTFM_MUTEX_UNLOCK(&scanofld_info->fw_chlist_state_lock);

	scanofld_info->fw_scan_busy = 0;
}

u32 mac_add_scanofld_ch(struct mac_ax_adapter *adapter, struct mac_ax_scanofld_chinfo *chinfo,
			u8 send_h2c, u8 clear_after_send)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_scanofld(struct mac_ax_adapter *adapter, struct mac_ax_scanofld_param *scanParam)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_scanofld_fw_busy(struct mac_ax_adapter *adapter)
{
	if (adapter->scanofld_info.fw_scan_busy) {
		return MACPROCBUSY;
	} else {
		return MACSUCCESS;
	}
}

u32 mac_scanofld_chlist_busy(struct mac_ax_adapter *adapter)
{
	if (adapter->scanofld_info.drv_chlist_busy || adapter->scanofld_info.fw_chlist_busy) {
		return MACPROCBUSY;
	} else {
		return MACSUCCESS;
	}
}

u32 get_ccxrpt_event(struct mac_ax_adapter *adapter,
		     struct rtw_c2h_info *c2h,
		     enum phl_msg_evt_id *id, u8 *c2h_info)
{
	struct mac_ax_ccxrpt *info;
	u32 val_d0, val_d3;
	u32 dword0 = *((u32 *)c2h->content);
	u32 dword3 = *((u32 *)(c2h->content + 12));

	info = (struct mac_ax_ccxrpt *)c2h_info;
	val_d0 = le32_to_cpu(dword0);
	val_d3 = le32_to_cpu(dword3);
	info->tx_state = GET_FIELD(val_d0, TXCCXRPT_TX_STATE);
	info->sw_define = GET_FIELD(val_d0, TXCCXRPT_SW_DEFINE);
	info->macid = GET_FIELD(val_d0, TXCCXRPT_MACID);
	info->pkt_ok_num = GET_FIELD(val_d3, TXCCXRPT_PKT_OK_NUM);
	info->data_txcnt = GET_FIELD(val_d3, TXCCXRPT_DATA_TX_CNT);

	if (info->tx_state) {
		*id = MSG_EVT_CCX_REPORT_TX_FAIL;
	} else {
		*id = MSG_EVT_CCX_REPORT_TX_OK;
	}

	return MACSUCCESS;
}

u32 pktofld_self_test(struct mac_ax_adapter *adapter)
{
	u32 poll_cnt, ret;
	u16 pkt_len, i;
	u8 pkt_id, state;
	u8 *pkt;

	pkt_len = 16;
	pkt = (u8 *)PLTFM_MALLOC(pkt_len);
	for (i = 0; i < pkt_len; i++) {
		pkt[i] = 15 - i;
	}

	ret = mac_add_pkt_ofld(adapter, pkt, pkt_len, &pkt_id, 0);
	if (ret != 0) {
		PLTFM_FREE(pkt, pkt_len);
		pkt = NULL;
		PLTFM_MSG_ERR("Packet ofld self test fail at ADD\n");
		return ret;
	}
	PLTFM_FREE(pkt, pkt_len);
	pkt = NULL;

	for (poll_cnt = 1000; poll_cnt > 0; poll_cnt--) {
		state = adapter->sm.pkt_ofld;
		if (state == MAC_AX_OFLD_H2C_IDLE) {
			break;
		}
		PLTFM_DELAY_MS(1);
	}
	if (poll_cnt == 0) {
		PLTFM_MSG_ERR("Packet ofld self test timeout at ADD\n");
		return MACPOLLTO;
	}

	ret = mac_read_pkt_ofld(adapter, pkt_id);
	if (ret != 0) {
		PLTFM_MSG_ERR("Packet ofld self test fail at READ\n");
		return ret;
	}

	ret = mac_pkt_ofld_packet(adapter, &pkt, &pkt_len, &pkt_id);
	if (ret != 0) {
		PLTFM_MSG_ERR("Packet ofld self test fail at READOUT\n");
		return ret;
	}
	for (i = 0; i < pkt_len; i++) {
		if (pkt[i] != (15 - i)) {
			PLTFM_FREE(pkt, pkt_len);
			pkt = NULL;
			PLTFM_MSG_ERR("Packet ofld self test fail at READOUT (Byte %d)\n", i);
			return MACCMP;
		}
	}
	PLTFM_FREE(pkt, pkt_len);
	pkt = NULL;

	ret = mac_del_pkt_ofld(adapter, pkt_id);
	if (ret != 0) {
		PLTFM_MSG_ERR("Packet ofld self test fail at DEL\n");
		return ret;
	}
	for (poll_cnt = 1000; poll_cnt > 0; poll_cnt--) {
		state = adapter->sm.pkt_ofld;
		if (state == MAC_AX_OFLD_H2C_IDLE) {
			break;
		}
		PLTFM_DELAY_MS(1);
	}
	if (poll_cnt == 0) {
		PLTFM_MSG_ERR("Packet ofld self test timout at DEL\n");
		return MACPOLLTO;
	}
	PLTFM_MSG_TRACE("Packet ofld self test pass\n");

	return MACSUCCESS;
}

u32 mac_cfg_bcn_filter(struct mac_ax_adapter *adapter, struct mac_ax_bcn_fltr cfg)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_bcn_filter_rssi(struct mac_ax_adapter *adapter, u8 macid, u8 size, u8 *rssi)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_bcn_filter_tp(struct mac_ax_adapter *adapter, u8 macid, u16 tx, u16 rx)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_host_efuse_rec(struct mac_ax_adapter *adapter, u32 host_id, u32 efuse_val)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

