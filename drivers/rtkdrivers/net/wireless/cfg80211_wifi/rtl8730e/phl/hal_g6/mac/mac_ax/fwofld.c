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

u32 mac_check_fwofld_done(struct mac_ax_adapter *adapter, u8 op)
{
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;

	switch (op) {
	case FW_OFLD_OP_DUMP_EFUSE:
		if (adapter->sm.efuse_ofld == MAC_AX_OFLD_H2C_IDLE) {
			return MACSUCCESS;
		}
		break;

	case FW_OFLD_OP_PACKET_OFLD:
		if (ofld_info->last_op == PKT_OFLD_OP_READ) {
			if (adapter->sm.pkt_ofld == MAC_AX_OFLD_H2C_DONE) {
				return MACSUCCESS;
			}
		} else {
			if (adapter->sm.pkt_ofld == MAC_AX_OFLD_H2C_IDLE) {
				return MACSUCCESS;
			}
		}
		break;
	case FW_OFLD_OP_READ_OFLD:
		if (adapter->sm.read_h2c == MAC_AX_OFLD_H2C_DONE) {
			return MACSUCCESS;
		}
		break;
	case FW_OFLD_OP_WRITE_OFLD:
		if (adapter->sm.write_h2c == MAC_AX_OFLD_H2C_IDLE) {
			return MACSUCCESS;
		}
		break;
	case FW_OFLD_OP_CONF_OFLD:
		if (adapter->sm.conf_h2c == MAC_AX_OFLD_H2C_IDLE) {
			return MACSUCCESS;
		}
		break;
	case FW_OFLD_OP_CH_SWITCH:
		if (adapter->sm.ch_switch == MAC_AX_OFLD_H2C_IDLE ||
		    adapter->sm.ch_switch == MAC_AX_CH_SWITCH_GET_RPT) {
			return MACSUCCESS;
		}
		break;
	default:
		return MACNOITEM;
	}

	return MACPROCBUSY;
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

u32 mac_clear_write_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.write_request == MAC_AX_OFLD_REQ_H2C_SENT) {
		return MACPROCERR;
	}

	if (cnv_write_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED)
	    != MACSUCCESS) {
		return MACPROCERR;
	}

	PLTFM_FREE(adapter->write_ofld_info.buf,
		   adapter->write_ofld_info.buf_size);
	adapter->write_ofld_info.buf = NULL;
	adapter->write_ofld_info.buf_wptr = NULL;
	adapter->write_ofld_info.last_req = NULL;
	adapter->write_ofld_info.buf_size = 0;
	adapter->write_ofld_info.avl_buf_size = 0;
	adapter->write_ofld_info.used_size = 0;
	adapter->write_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_write_request(struct mac_ax_adapter *adapter,
			  struct mac_ax_write_req *req, u8 *value, u8 *mask)
{
	struct mac_ax_write_ofld_info *ofld_info = &adapter->write_ofld_info;
	struct fwcmd_write_ofld_req *write_ptr;
	u32 data_len = 0;
	u8 state;

	state = adapter->sm.write_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(WRITE_OFLD_MAX_LEN);
		if (!ofld_info->buf) {
			return MACNPTR;
		}
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = WRITE_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = WRITE_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	data_len = sizeof(struct mac_ax_write_req);
	data_len += req->value_len;
	if (req->mask_en == 1) {
		data_len += req->value_len;
	}

	if (ofld_info->avl_buf_size < data_len) {
		return MACNOBUF;
	}

	if (!value) {
		return MACNPTR;
	}

	if (req->mask_en == 1 && !mask) {
		return MACNPTR;
	}

	if (cnv_write_ofld_state(adapter,
				 MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS) {
		return MACPROCERR;
	}

	if (ofld_info->req_num != 0) {
		ofld_info->last_req->ls = 0;
	}

	ofld_info->last_req = (struct mac_ax_write_req *)ofld_info->buf_wptr;

	req->ls = 1;

	write_ptr = (struct fwcmd_write_ofld_req *)ofld_info->buf_wptr;
	write_ptr->dword0 =
		cpu_to_le32(SET_WORD(req->value_len,
				     FWCMD_H2C_WRITE_OFLD_REQ_VALUE_LEN) |
			    SET_WORD(req->ofld_id,
				     FWCMD_H2C_WRITE_OFLD_REQ_OFLD_ID) |
			    SET_WORD(req->entry_num,
				     FWCMD_H2C_WRITE_OFLD_REQ_ENTRY_NUM) |
			    req->polling | req->mask_en | req->ls
			   );

	write_ptr->dword1 =
		cpu_to_le32(SET_WORD(req->offset,
				     FWCMD_H2C_WRITE_OFLD_REQ_OFFSET)
			   );

	ofld_info->buf_wptr += sizeof(struct mac_ax_write_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_write_req);
	ofld_info->used_size += sizeof(struct mac_ax_write_req);

	PLTFM_MEMCPY(ofld_info->buf_wptr, value, req->value_len);

	ofld_info->buf_wptr += req->value_len;
	ofld_info->avl_buf_size -= req->value_len;
	ofld_info->used_size += req->value_len;

	if (req->mask_en == 1) {
		PLTFM_MEMCPY(ofld_info->buf_wptr, mask, req->value_len);
		ofld_info->buf_wptr += req->value_len;
		ofld_info->avl_buf_size -= req->value_len;
		ofld_info->used_size += req->value_len;
	}

	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_write_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 cnv_conf_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.conf_request;

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

	adapter->sm.conf_request = dest;

	return MACSUCCESS;
}

u32 mac_clear_conf_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.conf_request == MAC_AX_OFLD_REQ_H2C_SENT) {
		return MACPROCERR;
	}

	if (cnv_conf_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED) !=
	    MACSUCCESS) {
		return MACPROCERR;
	}

	PLTFM_FREE(adapter->conf_ofld_info.buf,
		   adapter->conf_ofld_info.buf_size);
	adapter->conf_ofld_info.buf = NULL;
	adapter->conf_ofld_info.buf_wptr = NULL;
	adapter->conf_ofld_info.buf_size = 0;
	adapter->conf_ofld_info.avl_buf_size = 0;
	adapter->conf_ofld_info.used_size = 0;
	adapter->conf_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_conf_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_conf_ofld_req *req)
{
	struct mac_ax_conf_ofld_info *ofld_info = &adapter->conf_ofld_info;
	struct fwcmd_conf_ofld_req_cmd *write_ptr;
	u8 state;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	state = adapter->sm.conf_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(CONF_OFLD_MAX_LEN);
		if (!ofld_info->buf) {
			return MACNPTR;
		}
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = CONF_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = CONF_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	if (ofld_info->avl_buf_size < sizeof(struct mac_ax_conf_ofld_req)) {
		return MACNOBUF;
	}

	if (cnv_conf_ofld_state(adapter, MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS) {
		return MACPROCERR;
	}

	write_ptr = (struct fwcmd_conf_ofld_req_cmd *)ofld_info->buf_wptr;
	write_ptr->dword0 =
		cpu_to_le32(SET_WORD(req->device,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_DEVICE)
			   );

	write_ptr->dword1 =
		cpu_to_le32(SET_WORD(req->req.hioe.hioe_op,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_HIOE_OP) |
			    SET_WORD(req->req.hioe.inst_type,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_INST_TYPE) |
			    SET_WORD(req->req.hioe.data_mode,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_DATA_MODE)
			   );

	write_ptr->dword2 = cpu_to_le32(req->req.hioe.param0.register_addr);

	write_ptr->dword3 =
		cpu_to_le32(SET_WORD(req->req.hioe.param1.byte_data_h,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_BYTE_DATA_H) |
			    SET_WORD(req->req.hioe.param2.byte_data_l,
				     FWCMD_H2C_CONF_OFLD_REQ_CMD_BYTE_DATA_L)
			   );

	ofld_info->buf_wptr += sizeof(struct mac_ax_conf_ofld_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_conf_ofld_req);
	ofld_info->used_size += sizeof(struct mac_ax_conf_ofld_req);

	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_conf_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static inline void mac_pkt_ofld_set_bitmap(u8 *bitmap, u16 index)
{
	bitmap[index >> 3] |= (1 << (index & 7));
}

static inline void mac_pkt_ofld_unset_bitmap(u8 *bitmap, u16 index)
{
	bitmap[index >> 3] &= ~(1 << (index & 7));
}

static inline u8 mac_pkt_ofld_get_bitmap(u8 *bitmap, u16 index)
{
	return bitmap[index / 8] & (1 << (index & 7)) ? 1 : 0;
}

u32 mac_read_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_del_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_add_pkt_ofld(struct mac_ax_adapter *adapter, u8 *pkt, u16 len, u8 *id)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_pkt_ofld_packet(struct mac_ax_adapter *adapter,
			u8 **pkt_buf, u16 *pkt_len, u8 *pkt_id)
{
	struct mac_ax_pkt_ofld_pkt *pkt_info = &adapter->pkt_ofld_pkt;
	*pkt_buf = NULL;

	if (adapter->sm.pkt_ofld != MAC_AX_OFLD_H2C_DONE) {
		return MACPROCERR;
	}

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

u32 mac_efuse_ofld_map(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       u32 efuse_size)
{
	u32 size = efuse_size;
	struct mac_ax_efuse_ofld_info *ofld_info = &adapter->efuse_ofld_info;

	if (adapter->sm.efuse_ofld != MAC_AX_OFLD_H2C_DONE) {
		return MACPROCERR;
	}

	PLTFM_MEMCPY(efuse_map, ofld_info->buf, size);

	adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_IDLE;

	return MACSUCCESS;
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

u32 mac_clear_read_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.read_request == MAC_AX_OFLD_REQ_H2C_SENT) {
		return MACPROCERR;
	}

	if (cnv_read_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED)
	    != MACSUCCESS) {
		return MACPROCERR;
	}

	PLTFM_FREE(adapter->read_ofld_info.buf,
		   adapter->read_ofld_info.buf_size);
	adapter->read_ofld_info.buf = NULL;
	adapter->read_ofld_info.buf_wptr = NULL;
	adapter->read_ofld_info.last_req = NULL;
	adapter->read_ofld_info.buf_size = 0;
	adapter->read_ofld_info.avl_buf_size = 0;
	adapter->read_ofld_info.used_size = 0;
	adapter->read_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_read_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_read_req *req)
{
	struct mac_ax_read_ofld_info *ofld_info = &adapter->read_ofld_info;
	struct fwcmd_read_ofld_req *write_ptr;
	u8 state;

	state = adapter->sm.read_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(READ_OFLD_MAX_LEN);
		if (!ofld_info->buf) {
			return MACNPTR;
		}
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = READ_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = READ_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	if (ofld_info->avl_buf_size < sizeof(struct mac_ax_read_req)) {
		return MACNOBUF;
	}

	if (cnv_read_ofld_state(adapter, MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS) {
		return MACPROCERR;
	}

	if (ofld_info->req_num != 0) {
		ofld_info->last_req->ls = 0;
	}

	ofld_info->last_req = (struct mac_ax_read_req *)ofld_info->buf_wptr;

	req->ls = 1;

	write_ptr = (struct fwcmd_read_ofld_req *)ofld_info->buf_wptr;
	write_ptr->dword0 =
		cpu_to_le32(SET_WORD(req->value_len,
				     FWCMD_H2C_READ_OFLD_REQ_VALUE_LEN) |
			    SET_WORD(req->ofld_id,
				     FWCMD_H2C_READ_OFLD_REQ_OFLD_ID) |
			    SET_WORD(req->entry_num,
				     FWCMD_H2C_READ_OFLD_REQ_ENTRY_NUM) | req->ls
			   );

	write_ptr->dword1 =
		cpu_to_le32(SET_WORD(req->offset,
				     FWCMD_H2C_READ_OFLD_REQ_OFFSET)
			   );

	ofld_info->buf_wptr += sizeof(struct mac_ax_read_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_read_req);
	ofld_info->used_size += sizeof(struct mac_ax_read_req);
	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_read_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_read_ofld_value(struct mac_ax_adapter *adapter,
			u8 **val_buf, u16 *val_len)
{
	struct mac_ax_read_ofld_value *value_info = &adapter->read_ofld_value;
	*val_buf = NULL;

	if (adapter->sm.read_h2c != MAC_AX_OFLD_H2C_DONE) {
		return MACPROCERR;
	}

	*val_buf = (u8 *)PLTFM_MALLOC(value_info->len);
	if (!*val_buf) {
		return MACBUFALLOC;
	}

	PLTFM_MEMCPY(*val_buf, value_info->buf, value_info->len);

	*val_len = value_info->len;

	adapter->sm.read_h2c = MAC_AX_OFLD_H2C_IDLE;

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
	fwcmd_tbl.data4 = 0;	// unknown here.
	fwcmd_tbl.data5 = ids->cts2self;
	fwcmd_tbl.data6 = 0;	// unknown here.
	ret = rtw_hal_mac_send_h2c_ameba(hal_com, fwcmd_tbl.cmd_id, sizeof(struct fwcmd_rsvdpage) - 1, &fwcmd_tbl.data0);

	return ret;
}

static u32 add_cmd(struct mac_ax_adapter *adapter, struct rtw_mac_cmd *cmd)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 chk_cmd_ofld_reg(struct mac_ax_adapter *adapter)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 chk_cmd_ofld_pkt(struct mac_ax_adapter *adapter)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 chk_cmd_ofld(struct mac_ax_adapter *adapter, u8 rx_ok)
{
	u32 ret;

	if (rx_ok) {
		ret = chk_cmd_ofld_pkt(adapter);
	} else {
		ret = chk_cmd_ofld_reg(adapter);
	}

	return ret;
}

static u32 cmd_ofld(struct mac_ax_adapter *adapter)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_cmd_ofld(struct mac_ax_adapter *adapter)
{
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	struct mac_ax_state_mach *sm = &adapter->sm;
	u32 ret = MACSUCCESS;

	PLTFM_MUTEX_LOCK(&ofld_info->cmd_ofld_lock);
	if (sm->cmd_state != MAC_AX_CMD_OFLD_IDLE) {
		PLTFM_MSG_ERR("%s: IO offload is busy\n", __func__);
		PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);
		return MACPROCERR;
	}
	sm->cmd_state = MAC_AX_CMD_OFLD_PROC;
	PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);

	ret = cmd_ofld(adapter);

	PLTFM_MUTEX_LOCK(&ofld_info->cmd_ofld_lock);
	sm->cmd_state = MAC_AX_CMD_OFLD_IDLE;
	PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);

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

	ret = mac_add_pkt_ofld(adapter, pkt, pkt_len, &pkt_id);
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

	for (poll_cnt = 1000; poll_cnt > 0; poll_cnt--) {
		state = adapter->sm.pkt_ofld;
		if (state == MAC_AX_OFLD_H2C_DONE) {
			break;
		}
		PLTFM_DELAY_MS(1);
	}
	if (poll_cnt == 0) {
		PLTFM_MSG_ERR("Packet ofld self test timout at READ\n");
		return MACPOLLTO;
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

u32 mac_ch_switch_ofld(struct mac_ax_adapter *adapter, struct mac_ax_ch_switch_parm parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_get_ch_switch_rpt(struct mac_ax_adapter *adapter, struct mac_ax_ch_switch_rpt *rpt)
{
	struct mac_ax_state_mach *sm = &adapter->sm;

	if (sm->ch_switch != MAC_AX_CH_SWITCH_GET_RPT) {
		return MACPROCERR;
	}
	PLTFM_MEMCPY(rpt, adapter->ch_switch_rpt, sizeof(struct mac_ax_ch_switch_rpt));
	sm->ch_switch = MAC_AX_OFLD_H2C_IDLE;
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

