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

#include "fwcmd.h"
#include "mcc.h"
#include "mac_priv.h"
#include "twt.h"
#include "mac/pltfm_cfg.h"

#if MAC_AX_FEATURE_HV
#include "../hv_ax/dbgpkg_hv.h"
#endif

/* 8852A/8852B: the format of H2C/DLFW descriptor: WD Body
 * 8852C: the format of H2C/DLFW descriptor: RX Descriptor
 * WD body max len: 24 bytes
 * RX descriptor max len: 32 bytes
 * We use the max RX descriptor size as the header size
 * WD_BODY_LEN_V1 = RX descriptor max len = 32 bytes
 */

#define H2CB_CMD_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN_V1)
#define H2CB_CMD_SIZE		(H2C_CMD_LEN - FWCMD_HDR_LEN)
#define H2CB_CMD_QLEN		8

#define H2CB_DATA_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN_V1)
#define H2CB_DATA_SIZE		(H2C_DATA_LEN - FWCMD_HDR_LEN)
#define H2CB_DATA_QLEN		4

#define H2CB_LONG_DATA_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN)
#define H2CB_LONG_DATA_SIZE	(H2C_LONG_DATA_LEN - FWCMD_HDR_LEN)
#define H2CB_LONG_DATA_QLEN	1

#define FWCMD_WQ_MAX_JOB_NUM	5

#define FWCMD_LMT		12

#define MAC_AX_H2C_LMT_EN	0

#define FWCMD_H2CREG_BYTE0_SH 0
#define FWCMD_H2CREG_BYTE0_MSK 0xFF
#define FWCMD_H2CREG_BYTE1_SH 8
#define FWCMD_H2CREG_BYTE1_MSK 0xFF
#define FWCMD_H2CREG_BYTE2_SH 16
#define FWCMD_H2CREG_BYTE2_MSK 0xFF
#define FWCMD_H2CREG_BYTE3_SH 24
#define FWCMD_H2CREG_BYTE3_MSK 0xFF

#define BCN_GRPIE_OFST_EN BIT(7)

#define SCANOFLD_RSP_EVT_ID 0
#define SCANOFLD_RSP_EVT_PARSE 0

static u32 c2h_fwi_cmd_log(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
#if MAC_AX_FEATURE_DBGDEC
	u8 syntax_1 = 0, syntax_2 = 0;

	if ((len - FWCMD_HDR_LEN) >= 11) {
		syntax_1 = *(buf + FWCMD_HDR_LEN);
		syntax_2 = *(buf + FWCMD_HDR_LEN + 1);
	}
	//PLTFM_MSG_WARN("C2H encoded log syntax_1 %x, syntax_2 %x", syntax_1, syntax_2);
	{
		if (buf[len - 1] != '\0') {
			buf[len - 1] = '\0';
		}
		PLTFM_MSG_WARN("C2H log: %s", (char *)(buf + FWCMD_HDR_LEN));
	}
#else
	if (buf[len - 1] != '\0') {
		buf[len - 1] = '\0';
	}
	PLTFM_MSG_WARN("C2H log: %s", (char *)(buf + FWCMD_HDR_LEN));
#endif
	return MACSUCCESS;
}

static u32 c2h_wow_rcv_ack_hdl(struct mac_ax_adapter *adapter,
			       struct rtw_c2h_info *info)
{
	u8 *state;

	switch (info->c2h_func) {
	case FWCMD_H2C_FUNC_AOAC_REPORT_REQ:
		state = &adapter->sm.aoac_rpt;
		PLTFM_MSG_ERR("aoac_rpt rcv ack success\n");
		break;

	default:
		return MACSUCCESS;
	}

	if (*state == MAC_AX_AOAC_RPT_H2C_SENDING) {
		*state = MAC_AX_AOAC_RPT_H2C_RCVD;
	}

	return MACSUCCESS;
}

static u32 c2h_fwofld_rcv_ack_hdl(struct mac_ax_adapter *adapter,
				  struct rtw_c2h_info *info)
{
	u8 *state;

	switch (info->c2h_func) {
	case FWCMD_H2C_FUNC_WRITE_OFLD:
		state = &adapter->sm.write_h2c;
		break;

	case FWCMD_H2C_FUNC_CONF_OFLD:
		state = &adapter->sm.conf_h2c;
		break;

	case FWCMD_H2C_FUNC_PACKET_OFLD:
		state = &adapter->sm.pkt_ofld;
		break;

	case FWCMD_H2C_FUNC_READ_OFLD:
		state = &adapter->sm.read_h2c;
		break;

	case FWCMD_H2C_FUNC_DUMP_EFUSE:
		state = &adapter->sm.efuse_ofld;
		break;
	case FWCMD_H2C_FUNC_CH_SWITCH:
		state = &adapter->sm.ch_switch;
		break;

	default:
		return MACSUCCESS;
	}

	if (*state == MAC_AX_OFLD_H2C_SENDING) {
		*state = MAC_AX_OFLD_H2C_RCVD;
	}

	return MACSUCCESS;
}

static u32 c2h_fwi_rev_ack(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
	u32 data = *(u32 *)(buf + FWCMD_HDR_LEN);
	u32 ret;
	u32 cat;

	data = le32_to_cpu(data);

	cat = GET_FIELD(data, FWCMD_C2H_REC_ACK_CAT);
	if (cat == FWCMD_H2C_CAT_OUTSRC || cat == FWCMD_H2C_CAT_TEST) {
		return MACSUCCESS;
	}

	info->c2h_cat = GET_FIELD(data, FWCMD_C2H_REC_ACK_CAT);
	info->c2h_class = GET_FIELD(data, FWCMD_C2H_REC_ACK_CLASS);
	info->c2h_func = GET_FIELD(data, FWCMD_C2H_REC_ACK_FUNC);
	info->h2c_seq = GET_FIELD(data, FWCMD_C2H_REC_ACK_H2C_SEQ);
	adapter->fw_info.rec_seq = info->h2c_seq;
	info->type_rec_ack = 1;

	if (info->c2h_cat == FWCMD_H2C_CAT_MAC) {
		switch (info->c2h_class) {
		case FWCMD_H2C_CL_WOW:
			ret = c2h_wow_rcv_ack_hdl(adapter, info);
			if (ret) {
				return ret;
			}
			break;

		case FWCMD_H2C_CL_FW_OFLD:
			ret = c2h_fwofld_rcv_ack_hdl(adapter, info);
			if (ret) {
				return ret;
			}
			break;

		default:
			return MACSUCCESS;
		}
	}

	return MACSUCCESS;
}

static u32 c2h_fwofld_done_ack_hdl(struct mac_ax_adapter *adapter,
				   struct rtw_c2h_info *info)
{
	struct mac_ax_state_mach *sm = &adapter->sm;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;
	struct mac_ax_scanofld_info *scanofld_info = &adapter->scanofld_info;

	switch (info->c2h_func) {
	case FWCMD_H2C_FUNC_WRITE_OFLD:
		if (sm->write_h2c == MAC_AX_OFLD_H2C_RCVD) {
			if (info->h2c_return == MACSUCCESS) {
				sm->write_h2c = MAC_AX_OFLD_H2C_IDLE;
			} else {
				sm->write_h2c = MAC_AX_OFLD_H2C_ERROR;
			}
		}
		break;

	case FWCMD_H2C_FUNC_CONF_OFLD:
		if (sm->conf_h2c == MAC_AX_OFLD_H2C_RCVD) {
			if (info->h2c_return == MACSUCCESS) {
				sm->conf_h2c = MAC_AX_OFLD_H2C_IDLE;
			} else {
				sm->conf_h2c = MAC_AX_OFLD_H2C_ERROR;
			}
		}
		break;

	case FWCMD_H2C_FUNC_PACKET_OFLD:
		if (sm->pkt_ofld == MAC_AX_OFLD_H2C_RCVD) {
			if (info->h2c_return == MACSUCCESS) {
				if (ofld_info->last_op == PKT_OFLD_OP_READ) {
					sm->pkt_ofld = MAC_AX_OFLD_H2C_DONE;
				} else {
					sm->pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
				}
			} else {
				sm->pkt_ofld = MAC_AX_OFLD_H2C_ERROR;
			}
		} else {
			PLTFM_MSG_ERR("cant set pkt ofld state since no recv ack is received.");
		}
		break;

	case FWCMD_H2C_FUNC_READ_OFLD:
		if (sm->read_h2c == MAC_AX_OFLD_H2C_RCVD) {
			if (info->h2c_return == MACSUCCESS) {
				sm->read_h2c = MAC_AX_OFLD_H2C_DONE;
			} else {
				sm->read_h2c = MAC_AX_OFLD_H2C_ERROR;
			}
		}
		break;

	case FWCMD_H2C_FUNC_DUMP_EFUSE:
		if (sm->efuse_ofld == MAC_AX_OFLD_H2C_RCVD) {
			if (info->h2c_return == MACSUCCESS) {
				sm->efuse_ofld = MAC_AX_OFLD_H2C_DONE;
			} else {
				sm->efuse_ofld = MAC_AX_OFLD_H2C_ERROR;
			}
		}
		break;
	case FWCMD_H2C_FUNC_ADD_SCANOFLD_CH:
		if (scanofld_info->clear_drv_ch_list && scanofld_info->list->head) {
			mac_scanofld_ch_list_clear(adapter, scanofld_info->list);
		}
		PLTFM_MSG_TRACE("[scnofld] got add scanofld done ack. clear chlist busy\n");

		PLTFM_MUTEX_LOCK(&scanofld_info->drv_chlist_state_lock);
		adapter->scanofld_info.drv_chlist_busy = 0;
		PLTFM_MUTEX_UNLOCK(&scanofld_info->drv_chlist_state_lock);

		PLTFM_MUTEX_LOCK(&scanofld_info->fw_chlist_state_lock);
		adapter->scanofld_info.fw_chlist_busy = 0;
		PLTFM_MUTEX_UNLOCK(&scanofld_info->fw_chlist_state_lock);

		PLTFM_MSG_TRACE("[scnofld] drv_chlist_state = %d, fw_chlist_state = %d\n",
				adapter->scanofld_info.drv_chlist_busy,
				adapter->scanofld_info.fw_chlist_busy);
		break;
	case FWCMD_H2C_FUNC_SCANOFLD:
		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_TRACE("[scnofld] scan func fail,revert fwchlist and fw status\n");

			PLTFM_MUTEX_LOCK(&scanofld_info->fw_chlist_state_lock);
			scanofld_info->fw_chlist_busy = scanofld_info->last_fw_chlist_busy;
			PLTFM_MUTEX_UNLOCK(&scanofld_info->fw_chlist_state_lock);

			scanofld_info->fw_scan_busy = scanofld_info->last_fw_scan_busy;
		}
		PLTFM_MSG_TRACE("[scnofld] fw_scan_busy = %d, fw_chlist_state = %d\n",
				scanofld_info->fw_scan_busy,
				scanofld_info->fw_chlist_busy);
		break;
	default:
		break;
	}
	return MACSUCCESS;
}

static u32 c2h_role_done_ack_hdl(struct mac_ax_adapter *adapter,
				 struct rtw_c2h_info *info)
{
	struct mac_ax_state_mach *sm = &adapter->sm;

	if (info->c2h_class == FWCMD_H2C_CL_MEDIA_RPT &&
	    info->c2h_func == FWCMD_H2C_FUNC_FWROLE_MAINTAIN) {
		if (info->h2c_return == MACSUCCESS) {
			sm->role_stat = MAC_AX_ROLE_ALOC_SUCC;
		} else {
			PLTFM_MSG_ERR("[ERR]role_maintain: alloc failed\n");
			sm->role_stat = MAC_AX_ROLE_ALOC_FAIL;
			return MACROLEALOCFL;
		}
	} else if (info->c2h_class == FWCMD_H2C_CL_MEDIA_RPT &&
		   info->c2h_func == FWCMD_H2C_FUNC_JOININFO) {
		if (info->h2c_return == MACSUCCESS) {
			sm->role_stat = MAC_AX_ROLE_INIT_SUCC;
		} else {
			PLTFM_MSG_ERR("[ERR]role_join: init failed\n");
			sm->role_stat = MAC_AX_ROLE_INIT_FAIL;
			return MACROLEINITFL;
		}
	} else if (info->c2h_class == FWCMD_H2C_CL_ADDR_CAM_UPDATE &&
		   info->c2h_func == FWCMD_H2C_FUNC_ADDRCAM_INFO) {
		if (info->h2c_return == MACSUCCESS) {
			sm->role_stat = MAC_AX_ROLE_HW_UPD_SUCC;
		} else {
			PLTFM_MSG_ERR("[ERR]ADDR_CAM: upd failed\n");
			sm->role_stat = MAC_AX_ROLE_HW_UPD_FAIL;
			return MACROLEHWUPDFL;
		}
	}
	return MACSUCCESS;
}

static u32 c2h_ps_done_ack_hdl(struct mac_ax_adapter *adapter,
			       struct rtw_c2h_info *info)
{
	struct mac_ax_state_mach *sm = &adapter->sm;
	u8 p2pid;
	u32 ret;

	switch (info->c2h_func) {
	case FWCMD_H2C_FUNC_P2P_ACT:
		if (sm->p2p_stat != MAC_AX_P2P_ACT_BUSY) {
			PLTFM_MSG_ERR("[ERR]p2p act dack stat err %d\n",
				      sm->p2p_stat);
			return MACPROCERR;
		}

		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]p2p act dack ret %d\n",
				      info->h2c_return);
			sm->p2p_stat = MAC_AX_P2P_ACT_FAIL;
			break;
		}

		p2pid = P2PID_INVALID;
		ret = get_wait_dack_p2pid(adapter, &p2pid);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]p2p act dack get wait id %d\n", ret);
			return ret;
		}
		if (p2pid == P2PID_INVALID) {
			PLTFM_MSG_ERR("[ERR]p2p act dack no wait id\n");
			sm->p2p_stat = MAC_AX_P2P_ACT_IDLE;
			return MACPROCERR;
		}

		if (adapter->p2p_info[p2pid].wait_term &&
		    adapter->p2p_info[p2pid].wait_init) {
			PLTFM_MSG_ERR("[ERR]p2p act dack wait term & init\n");
		} else if (adapter->p2p_info[p2pid].wait_term) {
			PLTFM_MEMSET(&adapter->p2p_info[p2pid], 0,
				     sizeof(struct mac_ax_p2p_info));
		} else if (adapter->p2p_info[p2pid].wait_init) {
			adapter->p2p_info[p2pid].run = 1;
			adapter->p2p_info[p2pid].wait_init = 0;
			adapter->p2p_info[p2pid].wait_dack = 0;
		} else {
			adapter->p2p_info[p2pid].wait_dack = 0;
		}
		sm->p2p_stat = MAC_AX_P2P_ACT_IDLE;
		break;
	case FWCMD_H2C_FUNC_P2P_MACID_CTRL:
		if (sm->p2p_stat != MAC_AX_P2P_ACT_BUSY) {
			PLTFM_MSG_ERR("[ERR]p2p macid ctrl dack stat err %d\n",
				      sm->p2p_stat);
			return MACPROCERR;
		}

		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]p2p macid ctrl dack ret %d\n",
				      info->h2c_return);
			sm->p2p_stat = MAC_AX_P2P_ACT_FAIL;
			break;
		}

		p2pid = P2PID_INVALID;
		ret = get_wait_dack_p2pid(adapter, &p2pid);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]p2p macid ctrl dack get wait id %d\n",
				      ret);
			return ret;
		}
		if (p2pid != P2PID_INVALID) {
			if (adapter->p2p_info[p2pid].run) {
				adapter->p2p_info[p2pid].wait_dack = 0;
			} else {
				PLTFM_MSG_ERR("[ERR]p2p%d macid ctrl dack not run\n",
					      p2pid);
			}
		} else {
			PLTFM_MSG_ERR("[ERR]p2p macid ctrl dack no wait id\n");
		}
		sm->p2p_stat = MAC_AX_P2P_ACT_IDLE;
		break;
	case FWCMD_H2C_FUNC_IPS_CFG:
		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]fwips dack ret %d\n", info->h2c_return);
		}
		break;
	case FWCMD_H2C_FUNC_PS_ADVANCE_PARM:
		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]ps advanced parm dack ret %d\n", info->h2c_return);
		}
		break;
	case FWCMD_H2C_FUNC_PERIODIC_WAKE:
		if (info->h2c_return != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] periodic wake dack ret %d\n", info->h2c_return);
		}
		break;
	default:
		break;
	}

	return MACSUCCESS;
}

static u32 c2h_fwi_done_ack(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			    struct rtw_c2h_info *info)
{
	u32 data = *(u32 *)(buf + FWCMD_HDR_LEN);
	u32 ret;
	u32 cat;

	data = le32_to_cpu(data);

	cat = GET_FIELD(data, FWCMD_C2H_REC_ACK_CAT);
	if (cat == FWCMD_H2C_CAT_OUTSRC || cat == FWCMD_H2C_CAT_TEST) {
		return MACSUCCESS;
	}

	info->c2h_cat = GET_FIELD(data, FWCMD_C2H_DONE_ACK_CAT);
	info->c2h_class = GET_FIELD(data, FWCMD_C2H_DONE_ACK_CLASS);
	info->c2h_func = GET_FIELD(data, FWCMD_C2H_DONE_ACK_FUNC);
	info->h2c_return = GET_FIELD(data, FWCMD_C2H_DONE_ACK_H2C_RETURN);
	info->h2c_seq = GET_FIELD(data, FWCMD_C2H_DONE_ACK_H2C_SEQ);
	info->type_done_ack = 1;

	if (info->c2h_cat == FWCMD_H2C_CAT_MAC) {
		if (info->c2h_class == FWCMD_H2C_CL_FW_OFLD) {
			ret = c2h_fwofld_done_ack_hdl(adapter, info);
			if (ret != MACSUCCESS) {
				return ret;
			}
		} else if (info->c2h_class == FWCMD_H2C_CL_PS) {
			ret = c2h_ps_done_ack_hdl(adapter, info);
			if (ret != MACSUCCESS) {
				return ret;
			}
		} else if (info->c2h_class == FWCMD_H2C_CL_MEDIA_RPT ||
			   info->c2h_class == FWCMD_H2C_CL_ADDR_CAM_UPDATE) {
			ret = c2h_role_done_ack_hdl(adapter, info);
			if (ret != MACSUCCESS) {
				return ret;
			}
		}
	}

	return MACSUCCESS;
}

static u32 c2h_fwi_bcn_stats(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			     struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

static u32 c2h_fwi_bcn_csazero(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			       struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

static u32 c2h_dump_efuse_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			      u32 len, struct rtw_c2h_info *info)
{
	struct mac_ax_efuse_ofld_info *ofld_info = &adapter->efuse_ofld_info;
	u32 size;

	if (adapter->sm.efuse_ofld != MAC_AX_OFLD_H2C_RCVD) {
		PLTFM_MSG_ERR("[ERR]not cmd sending\n");
		return MACPROCERR;
	}

	size = adapter->hw_info->efuse_size;

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(size);
		if (!ofld_info->buf) {
			adapter->sm.efuse = MAC_AX_EFUSE_IDLE;
			return MACBUFALLOC;
		}
	}

	PLTFM_MEMCPY(ofld_info->buf, buf + FWCMD_HDR_LEN, size);

	adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_DONE;

	return MACSUCCESS;
}

static u32 c2h_read_rsp_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			    u32 len, struct rtw_c2h_info *info)
{
	struct mac_ax_read_ofld_value *value_info = &adapter->read_ofld_value;
	u32 hdr1;
	u16 read_len;
	u8 *read_buff;

	if (value_info->buf) {
		PLTFM_FREE(value_info->buf, value_info->len);
	}

	hdr1 = ((struct fwcmd_hdr *)buf)->hdr1;
	hdr1 = le32_to_cpu(hdr1);

	read_len = GET_FIELD(hdr1, C2H_HDR_TOTAL_LEN) - FWCMD_HDR_LEN;

	read_buff = (u8 *)PLTFM_MALLOC(read_len);
	if (!read_buff) {
		return MACBUFALLOC;
	}

	PLTFM_MEMCPY(read_buff, buf + FWCMD_HDR_LEN, read_len);

	value_info->len = read_len;
	value_info->buf = read_buff;

	return MACSUCCESS;
}

static u32 c2h_pkt_ofld_rsp_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				u32 len, struct rtw_c2h_info *info)
{
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;
	struct mac_ax_pkt_ofld_pkt *ofld_pkt = &adapter->pkt_ofld_pkt;
	struct mac_ax_state_mach *sm = &adapter->sm;
	u32 c2h_content = *(u32 *)(buf + FWCMD_HDR_LEN);
	u16 pkt_len;
	u8 id, pkt_op;
	u8 *pkt_buff;
	u8 *pkt_content;

	c2h_content = le32_to_cpu(c2h_content);

	pkt_op = GET_FIELD(c2h_content, FWCMD_C2H_PKT_OFLD_RSP_PKT_OP);
	pkt_len = GET_FIELD(c2h_content, FWCMD_C2H_PKT_OFLD_RSP_PKT_LENGTH);
	id = GET_FIELD(c2h_content, FWCMD_C2H_PKT_OFLD_RSP_PKT_ID);

	PLTFM_MSG_TRACE("get pkt ofld rsp. pkt_op: %d, pkt_len: %d, id: %d\n", pkt_op, pkt_len, id);

	switch (pkt_op) {
	case PKT_OFLD_OP_ADD:
		if (pkt_len != 0) {
			if (!(ofld_info->id_bitmap[id >> 3] & (1 << (id & 7)))) {
				ofld_info->free_id_count--;
				ofld_info->used_id_count++;
			}
			ofld_info->id_bitmap[id >> 3] |= (1 << (id & 7));
		}

		break;

	case PKT_OFLD_OP_DEL:
		if (pkt_len != 0) {
			if (ofld_info->id_bitmap[id >> 3] & (1 << (id & 7))) {
				ofld_info->free_id_count++;
				ofld_info->used_id_count--;
			}
			ofld_info->id_bitmap[id >> 3] &= ~(1 << (id & 7));
		}

		break;

	case PKT_OFLD_OP_READ:
		if (pkt_len != 0) {
			if (ofld_pkt->pkt) {
				PLTFM_FREE(ofld_pkt->pkt, ofld_pkt->pkt_len);
			}

			pkt_buff = (u8 *)PLTFM_MALLOC(pkt_len);
			if (!pkt_buff) {
				return MACBUFALLOC;
			}

			pkt_content = buf + FWCMD_HDR_LEN;
			pkt_content += sizeof(struct mac_ax_pkt_ofld_hdr);
			PLTFM_MEMCPY(pkt_buff, pkt_content, pkt_len);
			ofld_pkt->pkt_id = id;
			ofld_pkt->pkt_len = pkt_len;
			ofld_pkt->pkt = pkt_buff;
		}
		break;

	default:
		PLTFM_MSG_ERR("[ERR]invalid packet offload op: %d", pkt_op);
		break;
	}
	if (sm->pkt_ofld == MAC_AX_OFLD_H2C_ERROR) {
		if (ofld_info->last_op == PKT_OFLD_OP_READ) {
			sm->pkt_ofld = MAC_AX_OFLD_H2C_DONE;
		} else {
			sm->pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		}
	}

	return MACSUCCESS;
}

static u32 c2h_beacon_resend_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				 u32 len, struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

static u32 c2h_macid_pause_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			       u32 len, struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

static u32 c2h_tx_duty_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			   u32 len, struct rtw_c2h_info *info)
{
	u32 content = *(u32 *)(buf + FWCMD_HDR_LEN);
	struct mac_ax_tx_duty_ofld_info ofld_info;

	content = le32_to_cpu(content);

	ofld_info.timer_err =
		GET_FIELD(content, FWCMD_C2H_TX_DUTY_RPT_TIMER_ERR);

	if (ofld_info.timer_err) {
		PLTFM_MSG_ERR("[ERR]Tx duty FW timer error\n");
	}

	return MACSUCCESS;
}

static u32 c2h_tsf32_togl_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				  u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_cmd_ofld_rsp_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_scanofld_rsp_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_ch_switch_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				 u32 len, struct rtw_c2h_info *info)
{
	u32 *content = (u32 *)(buf + FWCMD_HDR_LEN);
	u8 *state;
	u32 rpt_size;
	struct mac_ax_ch_switch_rpt *rpt;

	state = &adapter->sm.ch_switch;
	if (*state != MAC_AX_OFLD_H2C_SENDING && *state != MAC_AX_OFLD_H2C_RCVD) {
		return MACPROCERR;
	}
	rpt_size = sizeof(struct mac_ax_ch_switch_rpt);
	if (!adapter->ch_switch_rpt) {
		adapter->ch_switch_rpt = (struct mac_ax_ch_switch_rpt *)PLTFM_MALLOC(rpt_size);
	}
	if (!adapter->ch_switch_rpt) {
		return MACPROCERR;
	}
	rpt = adapter->ch_switch_rpt;
	rpt->result = (u8)GET_FIELD(le32_to_cpu(*content), FWCMD_C2H_CH_SWITCH_RPT_RESULT);
	*state = MAC_AX_CH_SWITCH_GET_RPT;
	return MACSUCCESS;
}

static u32 c2h_bcn_filter_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf,
				  u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_fw_ofld(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_wait_announ_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			       u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_stat_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf,
			    u32 len, struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_twt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
	    struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_wow_aoac_report_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			    struct rtw_c2h_info *info)
{
	struct mac_ax_wowlan_info *wowlan_info = &adapter->wowlan_info;
	u8 *c2h_content = buf + FWCMD_HDR_LEN;
	u8 *rpt_buff;

	if (adapter->sm.aoac_rpt != MAC_AX_AOAC_RPT_H2C_RCVD) {
		return MACPROCERR;
	}

	if (wowlan_info->aoac_report) {
		PLTFM_FREE(wowlan_info->aoac_report,
			   sizeof(struct mac_ax_aoac_report));
	}

	rpt_buff = (u8 *)PLTFM_MALLOC(sizeof(struct mac_ax_aoac_report));
	if (!rpt_buff) {
		adapter->sm.aoac_rpt = MAC_AX_AOAC_RPT_ERROR;
		return MACBUFALLOC;
	}

	PLTFM_MEMCPY(rpt_buff, c2h_content, sizeof(struct mac_ax_aoac_report));
	wowlan_info->aoac_report = rpt_buff;

	adapter->sm.aoac_rpt = MAC_AX_AOAC_RPT_H2C_DONE;

	return MACSUCCESS;
}

u32 c2h_wow(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
	    struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_mcc_rcv_ack_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			struct rtw_c2h_info *info)
{
	struct mac_ax_state_mach *sm = &adapter->sm;
	u32 c2h_content = *(u32 *)(buf + FWCMD_HDR_LEN);
	u8 group, h2c_func;

	c2h_content = le32_to_cpu(c2h_content);
	group = GET_FIELD(c2h_content, FWCMD_C2H_MCC_RCV_ACK_GROUP);
	h2c_func = GET_FIELD(c2h_content, FWCMD_C2H_MCC_RCV_ACK_H2C_FUNC);

	if (h2c_func <= FWCMD_H2C_FUNC_RESET_MCC_GROUP) {
		PLTFM_MSG_TRACE("[TRACE]%s: MCC group H2C rcv ack\n",
				__func__);

		if (sm->mcc_group[group] == MAC_AX_MCC_STATE_H2C_SENT) {
			sm->mcc_group[group] = MAC_AX_MCC_STATE_H2C_RCVD;

			PLTFM_MSG_TRACE("[TRACE]%s: MCC group %d state: %d\n",
					__func__, group,
					MAC_AX_MCC_STATE_H2C_RCVD);
		}
	} else if (h2c_func <= FWCMD_H2C_FUNC_MCC_SET_DURATION) {
		PLTFM_MSG_TRACE("[TRACE]%s: MCC request H2C rcv ack\n",
				__func__);

		if (sm->mcc_request[group] == MAC_AX_MCC_REQ_H2C_SENT) {
			sm->mcc_request[group] = MAC_AX_MCC_REQ_H2C_RCVD;

			PLTFM_MSG_TRACE("[TRACE]%s: MCC group %d state: %d\n",
					__func__, group,
					MAC_AX_MCC_REQ_H2C_RCVD);
		}
	} else {
		PLTFM_MSG_ERR("[ERR]%s: invalid MCC H2C func %d\n",
			      __func__, h2c_func);
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 c2h_mcc_req_ack_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			struct rtw_c2h_info *info)
{
	struct mac_ax_state_mach *sm = &adapter->sm;
	u32 c2h_content = *(u32 *)(buf + FWCMD_HDR_LEN);
	u8 group, h2c_func, h2c_return;

	c2h_content = le32_to_cpu(c2h_content);
	group = GET_FIELD(c2h_content, FWCMD_C2H_MCC_REQ_ACK_GROUP);
	h2c_func = GET_FIELD(c2h_content, FWCMD_C2H_MCC_REQ_ACK_H2C_FUNC);
	h2c_return = GET_FIELD(c2h_content, FWCMD_C2H_MCC_REQ_ACK_H2C_RETURN);

	PLTFM_MSG_TRACE("[TRACE]%s: group: %d, h2c_func: %d, h2c_return: %d\n",
			__func__, group, h2c_func, h2c_return);

	if (h2c_func < FWCMD_H2C_FUNC_MCC_REQ_TSF) {
		PLTFM_MSG_ERR("[ERR]%s: invalid MCC H2C func: %d\n",
			      __func__, h2c_func);
		return MACNOITEM;
	}

	PLTFM_MSG_TRACE("[TRACE]%s: group %d curr req state: %d\n",
			__func__, group, sm->mcc_request[group]);

	if (sm->mcc_request[group] == MAC_AX_MCC_REQ_H2C_RCVD) {
		if (h2c_return == 0) {
			if (h2c_func == FWCMD_H2C_FUNC_MCC_REQ_TSF) {
				sm->mcc_request[group] = MAC_AX_MCC_REQ_DONE;
			} else {
				sm->mcc_request[group] = MAC_AX_MCC_REQ_IDLE;
			}
		} else {
			sm->mcc_request[group] = MAC_AX_MCC_REQ_FAIL;
			PLTFM_MSG_ERR("[ERR]%s: MCC H2C func %d fail: %d\n",
				      __func__, h2c_func, h2c_return);
		}
	}

	return MACSUCCESS;
}

u32 c2h_mcc_tsf_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_mcc_status_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_mcc(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
	    struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_rx_dbg_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		   struct rtw_c2h_info *info)
{
	PLTFM_MSG_ERR("[ERR]%s: FW encounter Rx problem!\n", __func__);

	return MACSUCCESS;
}

u32 c2h_fw_dbg(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
	       struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_wps_rpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		struct rtw_c2h_info *info)
{
	PLTFM_MSG_TRACE("recevied wps report\n");
	return MACSUCCESS;
}

static u32 c2h_misc_ccxrpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

static u32 c2h_cl_misc(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		       struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_fast_ch_sw_rpt_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_fast_ch_sw(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		   struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_port_init_stat(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		       struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 c2h_port_cfg_stat(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		      struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 c2h_cl_mport(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			struct rtw_c2h_info *info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static inline struct c2h_proc_class *c2h_proc_sel(u8 cat)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

int c2h_field_parsing(int id)
{

	switch (id) {
	case C2H_DBG:
	case C2H_LB:
	case C2H_DEFEATURE_DBG:
	case C2H_DEFEATURE_RSVD:
	case C2H_EXTEND:
		return C2H_CAT_TEST;

	case C2H_TXBF:
	case C2H_CCX_TX_RPT:
	case C2HID_AP_REQ_TXRPT:
	case C2H_FW_SCAN_COMPLETE:
	case C2H_RA_RPT:
	case C2HID_SPE_STATIS:
	case C2H_RA_PARA_RPT:
	case C2H_FW_CHNL_SWITCH_COMPLETE:
	case C2H_IQK_FINISH:
	case C2H_MAILBOX_STATUS:
	case C2H_P2P_RPORT:
	case C2H_MCC:
	case C2H_MAC_HIDDEN_RPT:
	case C2H_MAC_HIDDEN_RPT_2:
	case C2H_BCN_EARLY_RPT:
	case C2H_TSF_RPT:
	case C2H_WLAN_INFO:
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && (CONFIG_UNSUPPORT_PLCPHDR_RPT)
	case C2H_PLCPHDR_RPT:
#endif
		return C2H_CAT_MAC;

	case C2H_BT_INFO:
	case C2H_BT_MP_INFO:
		return C2H_CAT_BTCOEX;
	default:
		break;
	}
	return -EINVAL;
}

bool rtw_hal_c2h_pkt_hdr_parse(struct mac_ax_adapter *adapter, u8 *buf, u16 len, u8 *id, u8 *seq, u8 *plen, u8 **payload)
{
	bool ret = _FAIL;

	if (!buf || len > 256 || len < 3) {
		goto exit;
	}

	*id = C2H_ID_88XX(buf);
	*seq = C2H_SEQ_88XX(buf);
	*plen = len - 2;
	*payload = C2H_PAYLOAD_88XX(buf);
	ret = _SUCCESS;

exit:
	return ret;
}

u32 mac_process_c2h(struct mac_ax_adapter *adapter, u8 *buf, u32 len, u8 *c2h_ret)
{
	u8 parse_fail = 0;
	u8 bypass = 0;
	s32 ret = _FAIL;
	u8 id, seq, plen;
	u8 *payload;
	struct rtw_c2h_info *c2h = (struct rtw_c2h_info *)c2h_ret;

	ret = rtw_hal_c2h_pkt_hdr_parse(adapter, buf, len, &id, &seq, &plen, &payload);

	c2h->c2h_cat = c2h_field_parsing(id);
	c2h->c2h_class = id;
	c2h->c2h_func = 0;
	c2h->content = payload;
	c2h->content_len = plen;
	c2h->h2c_return = ret;
	c2h->h2c_seq = seq;

	return ret;
}

/* km */
u8 rtl8730e_set_wowlan_ctrl_cmd(struct mac_ax_adapter *adapter, enum mac_ax_wow_ctrl w_c)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;

	u8 u1H2CWoWlanCtrlParm[H2C_WOWLAN_LEN] = {0};
	u8 discont_wake = 1, gpionum = 0, gpio_dur = 0;
	u8 hw_unicast = 0, gpio_pulse_cnt = 0, gpio_pulse_en = 0;
	u8 sdio_wakeup_enable = 1;
	u8 gpio_high_active = 0; //0: low active, 1: high active
	u8 magic_pkt = 0;
	u8 gpio_unit = 0; /*0: 64ns, 1: 8ms*/
	u8 ret = _FAIL;
	u8 enable = !w_c;

	/* change_unit not supported now. */
	u8 change_unit = 0;

#ifdef CONFIG_GPIO_WAKEUP
	gpionum = WAKEUP_GPIO_IDX;
	sdio_wakeup_enable = 0;
#endif //CONFIG_GPIO_WAKEUP

	if (w_c == MAC_AX_WOW_ENTER) {
		enable = 1;
	} else {
		enable = 0;
	}

	magic_pkt = enable;
	hw_unicast = 0;

#if 0
	struct pwrctrl_priv *ppwrpriv = adapter_to_pwrctl(adapter);
	if (psecpriv->dot11PrivacyAlgrthm == _WEP40_ || psecpriv->dot11PrivacyAlgrthm == _WEP104_) {
		hw_unicast = 1;
	} else {
		hw_unicast = 0;
	}
#endif

	printk("%s(): enable=%d change_unit=%d\n",
			 __func__, enable, change_unit);

	/* time = (gpio_dur/2) * gpio_unit, default:256 ms */
	if (enable && change_unit) {
		gpio_dur = 0x40;
		gpio_unit = 1;
		gpio_pulse_en = 1;
	}

#ifdef CONFIG_PLATFORM_ARM_RK3188
	if (enable) {
		gpio_pulse_en = 1;
		gpio_pulse_cnt = 0x04;
	}
#endif

	SET_H2CCMD_WOWLAN_FUNC_ENABLE(u1H2CWoWlanCtrlParm, enable);
	SET_H2CCMD_WOWLAN_PATTERN_MATCH_ENABLE(u1H2CWoWlanCtrlParm, 0);
	SET_H2CCMD_WOWLAN_MAGIC_PKT_ENABLE(u1H2CWoWlanCtrlParm, magic_pkt);
	SET_H2CCMD_WOWLAN_UNICAST_PKT_ENABLE(u1H2CWoWlanCtrlParm, 1);
	SET_H2CCMD_WOWLAN_ALL_PKT_DROP(u1H2CWoWlanCtrlParm, 0);
	SET_H2CCMD_WOWLAN_GPIO_ACTIVE(u1H2CWoWlanCtrlParm, gpio_high_active);
#ifndef CONFIG_GTK_OL
	SET_H2CCMD_WOWLAN_REKEY_WAKE_UP(u1H2CWoWlanCtrlParm, enable);
#endif
	SET_H2CCMD_WOWLAN_DISCONNECT_WAKE_UP(u1H2CWoWlanCtrlParm, discont_wake);
	SET_H2CCMD_WOWLAN_GPIONUM(u1H2CWoWlanCtrlParm, gpionum);
	SET_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(u1H2CWoWlanCtrlParm, sdio_wakeup_enable);

	SET_H2CCMD_WOWLAN_GPIO_DURATION(u1H2CWoWlanCtrlParm, gpio_dur);
	SET_H2CCMD_WOWLAN_CHANGE_UNIT(u1H2CWoWlanCtrlParm, gpio_unit);

	SET_H2CCMD_WOWLAN_GPIO_PULSE_EN(u1H2CWoWlanCtrlParm, gpio_pulse_en);
	SET_H2CCMD_WOWLAN_GPIO_PULSE_COUNT(u1H2CWoWlanCtrlParm, gpio_pulse_cnt);

	ret = rtw_hal_mac_send_h2c_ameba(hal_com, H2C_WOWLAN, H2C_WOWLAN_LEN, u1H2CWoWlanCtrlParm);

	return ret;
}



u32 mac_outsrc_h2c_common(struct mac_ax_adapter *adapter,
			  struct rtw_g6_h2c_hdr *hdr, u32 *pvalue)
{
	/* Confirmed: bb will not use h2c to write rf regs. */
	return 0;
}

u32 mac_fw_log_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_fw_log *log_cfg)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_host_getpkt_h2c(struct mac_ax_adapter *adapter, u8 macid, u8 pkttype)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

#if MAC_AX_PHL_H2C
u32 __ie_cam_set_cmd(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb,
		     struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

#else
u32 __ie_cam_set_cmd(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb,
		     struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}
#endif
u32 mac_ie_cam_upd(struct mac_ax_adapter *adapter,
		   struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_send_h2c_hmebox(struct mac_ax_adapter *adapter, u8 *h2c)
{
#define MAX_H2C_BOX_NUMS 4
#define MESSAGE_BOX_SIZE 4
#define EX_MESSAGE_BOX_SIZE 4
#define MAC_AX_H2CHMEBOX_CNT 100
#define MAC_AX_H2CHMEBOX_MS 1
	u32 cnt = MAC_AX_H2CHMEBOX_CNT;
	u8 len, byte0, byte1, val;
	u8 h2c_box_num = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct fwcmd_h2creg h2creg;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_h2creg_offset *h2creg_offset;
	struct mac_ax_fw_info *fw = &adapter->fw_info;
	u32 h2c_cmd = 0;
	u32 h2c_cmd_ex = 0;
	u32 msgbox_addr = 0;
	u32 msgbox_ex_addr = 0;

	if (!h2c) {
		return MACSUCCESS;
	}

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		PLTFM_MSG_ERR("FW is not ready\n");
		return MACFWNONRDY;
	}

	if (adapter->sm.mac_rdy != MAC_AX_MAC_RDY) {
		PLTFM_MSG_TRACE("MAC is not ready\n");
		adapter->stats.h2c_reg_uninit++;
	}

	h2creg_offset = p_ops->get_h2creg_offset(adapter);
	if (!h2creg_offset) {
		PLTFM_MSG_ERR("Get H2CREG offset FAIL\n");
		return MACNPTR;
	}

	PLTFM_MUTEX_LOCK(&adapter->fw_info.msg_reg);
	h2c_box_num = fw->last_hmebox_num;

	do {
		if (!(MAC_REG_R8(h2creg_offset->ctrl) & BIT(h2c_box_num))) {
			break;
		}
		PLTFM_DELAY_MS(MAC_AX_H2CHMEBOX_MS);
		cnt--;
	} while (cnt);

	if (!cnt) {
		PLTFM_MSG_WARN("%s, reg_1cc(%x), msg_box(%d)...\n", __func__, \
			       MAC_REG_R8(h2creg_offset->ctrl), h2c_box_num);
		PLTFM_MSG_ERR("fw read cmd failed...\n");
		PLTFM_MUTEX_UNLOCK(&adapter->fw_info.msg_reg);
		return MACPOLLTO;
	}

	/* Write Ext command. data 3~6. */
	msgbox_ex_addr = h2creg_offset->data1 + (h2c_box_num * EX_MESSAGE_BOX_SIZE);
	PLTFM_MEMCPY((u8 *)(&h2c_cmd_ex), h2c + 4, EX_MESSAGE_BOX_SIZE);
	h2c_cmd_ex = le32_to_cpu(h2c_cmd_ex);
	MAC_REG_W32(msgbox_ex_addr, h2c_cmd_ex);

	/* Write command. CMD ID + data 0~2. */
	msgbox_addr = h2creg_offset->data0 + (h2c_box_num * MESSAGE_BOX_SIZE);
	PLTFM_MEMCPY((u8 *)(&h2c_cmd), h2c, 4);
	h2c_cmd = le32_to_cpu(h2c_cmd);
	MAC_REG_W32(msgbox_addr, h2c_cmd);

	/* update last msg box number */
	fw->last_hmebox_num = (h2c_box_num + 1) % MAX_H2C_BOX_NUMS;

	PLTFM_MUTEX_UNLOCK(&adapter->fw_info.msg_reg);

	return MACSUCCESS;
}

u32 _mac_send_h2creg(struct mac_ax_adapter *adapter,
		     struct mac_ax_h2creg_info *h2c)
{

	return MACSUCCESS;
}

static u32 get_wps_rpt_event_id(struct mac_ax_adapter *adapter,
				struct rtw_c2h_info *c2h,
				enum phl_msg_evt_id *id,
				u8 *c2h_info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 get_bcn_resend_event(struct mac_ax_adapter *adapter,
				struct rtw_c2h_info *c2h,
				enum phl_msg_evt_id *id,
				u8 *c2h_info)
{
	*id = MSG_EVT_BCN_RESEND;

	return MACSUCCESS;
}

static u32 get_tsf32_togl_rpt_event(struct mac_ax_adapter *adapter,
				    struct rtw_c2h_info *c2h,
				    enum phl_msg_evt_id *id,
				    u8 *c2h_info)
{
	*id = HAL_C2H_EV_MAC_TSF32_TOG;

	return MACSUCCESS;
}

static u32 get_fw_rx_dbg_event(struct mac_ax_adapter *adapter,
			       struct rtw_c2h_info *c2h,
			       enum phl_msg_evt_id *id,
			       u8 *c2h_info)
{
	*id = MSG_EVT_DBG_RX_DUMP;

	return MACSUCCESS;
}

static u32 get_scanofld_event(struct mac_ax_adapter *adapter, struct rtw_c2h_info *c2h,
			      enum phl_msg_evt_id *id, u8 *c2h_info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 mac_get_c2h_event(struct mac_ax_adapter *adapter,
		      struct rtw_c2h_info *c2h,
		      enum phl_msg_evt_id *id,
		      u8 *c2h_info)
{
	struct c2h_event_id_proc *proc;
	/* write specific handler later. */
	u32(*hdl)(struct mac_ax_adapter * adapter, struct rtw_c2h_info * c2h,
		  enum phl_msg_evt_id * id, u8 * c2h_info) = NULL;

	/*for C2H ack bit no need to process*/
	if (c2h->type_done_ack == 1 || c2h->type_rec_ack == 1) {
		return MACSUCCESS;
	}

	/* Deal with specific mac C2H cmd. */

	return MACSUCCESS;
}
