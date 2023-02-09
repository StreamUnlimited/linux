/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _HAL_BTC_FW_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_def.h"

#ifdef CONFIG_BTCOEX

void _chk_btc_err(struct btc_t *btc, u8 type, u32 cnt)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;

	switch (type) {
	case BTC_DCNT_RPT_HANG:
		if (wl->status.map.lps == BTC_LPS_RF_OFF ||
		    wl->status.map.rf_off) {
			return;
		}

		if (dm->cnt_dm[BTC_DCNT_RPT] == cnt && btc->fwinfo.rpt_en_map) {
			dm->cnt_dm[BTC_DCNT_RPT_HANG]++;
		} else {
			dm->cnt_dm[BTC_DCNT_RPT_HANG] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_RPT_HANG] >= BTC_CHK_HANG_MAX) {
			dm->error.map.wl_fw_hang = true;
		} else {
			dm->error.map.wl_fw_hang = false;
		}

		dm->cnt_dm[BTC_DCNT_RPT] = cnt;
		break;
	case BTC_DCNT_CYCLE_HANG:
		if (dm->cnt_dm[BTC_DCNT_CYCLE] == cnt &&
		    (dm->tdma_now.type != CXTDMA_OFF ||
		     dm->tdma_now.ext_ctrl == CXECTL_EXT)) {
			dm->cnt_dm[BTC_DCNT_CYCLE_HANG]++;
		} else {
			dm->cnt_dm[BTC_DCNT_CYCLE_HANG] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_CYCLE_HANG] >= BTC_CHK_HANG_MAX) {
			dm->error.map.cycle_hang = true;
		} else {
			dm->error.map.cycle_hang = false;
		}

		dm->cnt_dm[BTC_DCNT_CYCLE] = cnt;
		break;
	case BTC_DCNT_W1_HANG:
		if (dm->cnt_dm[BTC_DCNT_W1] == cnt &&
		    dm->tdma_now.type != CXTDMA_OFF) {
			dm->cnt_dm[BTC_DCNT_W1_HANG]++;
		} else {
			dm->cnt_dm[BTC_DCNT_W1_HANG] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_W1_HANG] >= BTC_CHK_HANG_MAX) {
			dm->error.map.w1_hang = true;
		} else {
			dm->error.map.w1_hang = false;
		}

		dm->cnt_dm[BTC_DCNT_W1] = cnt;
		break;
	case BTC_DCNT_B1_HANG:
		if (dm->cnt_dm[BTC_DCNT_B1] == cnt &&
		    dm->tdma_now.type != CXTDMA_OFF) {
			dm->cnt_dm[BTC_DCNT_B1_HANG]++;
		} else {
			dm->cnt_dm[BTC_DCNT_B1_HANG] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_B1_HANG] >= BTC_CHK_HANG_MAX) {
			dm->error.map.b1_hang = true;
		} else {
			dm->error.map.b1_hang = false;
		}

		dm->cnt_dm[BTC_DCNT_B1] = cnt;
		break;
	case BTC_DCNT_TDMA_NONSYNC:
		if (cnt != 0) { /* if tdma not sync between drv/fw  */
			dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC]++;
		} else {
			dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC] >= BTC_CHK_HANG_MAX) {
			dm->error.map.tdma_no_sync = true;
		} else {
			dm->error.map.tdma_no_sync = false;
		}
		break;
	case BTC_DCNT_SLOT_NONSYNC:
		if (cnt != 0) { /* if slot not sync between drv/fw  */
			dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC]++;
		} else {
			dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC] >= BTC_CHK_HANG_MAX) {
			dm->error.map.tdma_no_sync = true;
		} else {
			dm->error.map.tdma_no_sync = false;
		}
		break;
	case BTC_DCNT_BTCNT_HANG:
		cnt = cx->cnt_bt[BTC_BCNT_HIPRI_RX] +
		      cx->cnt_bt[BTC_BCNT_HIPRI_TX] +
		      cx->cnt_bt[BTC_BCNT_LOPRI_RX] +
		      cx->cnt_bt[BTC_BCNT_LOPRI_TX];

		if (cnt == 0) {
			dm->cnt_dm[BTC_DCNT_BTCNT_HANG]++;
		} else {
			dm->cnt_dm[BTC_DCNT_BTCNT_HANG] = 0;
		}

		if ((dm->cnt_dm[BTC_DCNT_BTCNT_HANG] >= BTC_CHK_HANG_MAX &&
		     bt->enable.now) || (!dm->cnt_dm[BTC_DCNT_BTCNT_HANG] &&
					 !bt->enable.now)) {
			_update_bt_scbd(btc, false);
		}
		break;
	case BTC_DCNT_WL_SLOT_DRIFT:
		if (cnt >= BTC_CHK_WLSLOT_DRIFT_MAX) {
			dm->cnt_dm[BTC_DCNT_WL_SLOT_DRIFT]++;
		} else {
			dm->cnt_dm[BTC_DCNT_WL_SLOT_DRIFT] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_WL_SLOT_DRIFT] >= BTC_CHK_HANG_MAX) {
			dm->error.map.wl_slot_drift = true;
		} else {
			dm->error.map.wl_slot_drift = false;
		}
		break;
	case BTC_DCNT_BT_SLOT_DRIFT:
		if (cnt >= BTC_CHK_BTSLOT_DRIFT_MAX) {
			dm->cnt_dm[BTC_DCNT_BT_SLOT_DRIFT]++;
		} else {
			dm->cnt_dm[BTC_DCNT_BT_SLOT_DRIFT] = 0;
		}

		if (dm->cnt_dm[BTC_DCNT_BT_SLOT_DRIFT] >= BTC_CHK_HANG_MAX) {
			dm->error.map.bt_slot_drift = true;
		} else {
			dm->error.map.bt_slot_drift = false;
		}
		break;
	}
}

static inline void _get_bt_patch_ver(struct btc_t *btc)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;
	struct fbtc_btver *pver = &fwinfo->rpt_fbtc_btver.finfo;

	pver->fw_ver = le32_to_cpu(*(u32 *)mpinfo->rsp_data);

	return;
}

static inline void _get_bt_feature(struct btc_t *btc)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;
	struct fbtc_btver *pver = &fwinfo->rpt_fbtc_btver.finfo;

	pver->feature = le32_to_cpu(*(u32 *)mpinfo->rsp_data);

	return;
}

static inline void _get_bt_coex_ver(struct btc_t *btc)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;
	struct fbtc_btver *pver = &fwinfo->rpt_fbtc_btver.finfo;

	pver->coex_ver = le32_to_cpu(*(u32 *)mpinfo->rsp_data);

	return;
}

static inline void _get_bt_scan_param(struct btc_t *btc, void *pscan_type)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;
	struct fbtc_btscan *pscan = &fwinfo->rpt_fbtc_btscan.finfo;
	u8 scan_type = 0;

	if (!pscan_type) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_,
			  "[BTC], %s(): pscan_type is NULL!\n",
			  __func__);
		return;
	}
	scan_type = *(u8 *)pscan_type;
	pscan->scan[scan_type] = le32_to_cpu(*(u32 *)mpinfo->rsp_data);
	bt->scan_info[scan_type].bt_scan_para = pscan->scan[scan_type];

	return;
}

static inline void _get_bt_dev_info(struct btc_t *btc)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;
	struct fbtc_btdevinfo *pdev = &fwinfo->rpt_fbtc_btdev.finfo;
	u32 val = 0;
	
	val = le32_to_cpu(*(u32 *)mpinfo->rsp_data);
	pdev->dev_name = (val & 0xFFFFFF00) >> 8;
	pdev->vendor_id = (u16)(val & 0xFF);

	return;
}

static void _update_bt_report(struct btc_t *btc, u8 op, void *prama)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_bt_a2dp_desc *a2dp = &bt_linfo->a2dp_desc;
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_btmpinfo *mpinfo = &fwinfo->btmpinfo;

	struct fbtc_btver *pver = &fwinfo->rpt_fbtc_btver.finfo;
	struct fbtc_btscan *pscan = &fwinfo->rpt_fbtc_btscan.finfo;
	struct fbtc_btafh *pafh = &fwinfo->rpt_fbtc_btafh.finfo;
	struct fbtc_btdevinfo *pdev = &fwinfo->rpt_fbtc_btdev.finfo;

	u8 scan_type = 0;

	switch (op) {
	case BT_OP_GET_BT_VERSION:
		_get_bt_patch_ver(btc);
		bt->ver_info.fw = pver->fw_ver;
		break;
	case BT_OP_GET_BT_COEX_SUPPORTED_FEATURE:
		_get_bt_feature(btc);
		bt->feature = pver->feature;
		break;
	case BT_OP_GET_BT_COEX_SUPPORTED_VERSION:
		_get_bt_coex_ver(btc);
		bt->ver_info.fw_coex = pver->coex_ver;
		break;
	case BT_OP_GET_BT_BLE_SCAN_TYPE:
		scan_type = bt->scan_type;
		bt->scan_type = mpinfo->rsp_data[0];
		if (scan_type != bt->scan_type) {
			hal_btc_fw_get_bt_rpt(btc, BT_OP_GET_BT_BLE_SCAN_PARA);
		}
		break;
	case BT_OP_GET_BT_BLE_SCAN_PARA:
		_get_bt_scan_param(btc, prama);
		break;
	case BT_LO_OP_GET_AFH_MAP_L:
		hal_mem_cpy(h, &pafh->afh_l[0], mpinfo->rsp_data, 4);
		hal_mem_cpy(h, &bt_linfo->afh_map[0], mpinfo->rsp_data, 4);
		break;
	case BT_LO_OP_GET_AFH_MAP_M:
		hal_mem_cpy(h, &pafh->afh_l[0], mpinfo->rsp_data, 4);
		hal_mem_cpy(h, &bt_linfo->afh_map[4], mpinfo->rsp_data, 4);
		break;
	case BT_LO_OP_GET_AFH_MAP_H:
		hal_mem_cpy(h, &pafh->afh_h[0], mpinfo->rsp_data, 2);
		hal_mem_cpy(h, &bt_linfo->afh_map[8], mpinfo->rsp_data, 2);
		break;
	case BT_OP_GET_BT_DEVICE_INFO:
		_get_bt_dev_info(btc);
		a2dp->device_name = pdev->dev_name;
		a2dp->vendor_id = pdev->vendor_id;
		break;
	default:
		break;
	}
}

static void _parse_btc_report(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			      u8 *pbuf, u32 buf_len)
{
	struct btc_btmpinfo *mpinfo = &pfwinfo->btmpinfo;
	struct rtw_hal_com_t *h = btc->hal;
	u8 extid = 0, status = 0, len = 0, seq = 0;

	if ((buf_len < 3) || (!pbuf)) {
		return;
	}

	extid = pbuf[0];
	switch (extid) {
	case EXTEND_C2H_TRIG_BY_BT_FW:
		mpinfo->rpt_sts = pbuf[1] & 0x0F;
		mpinfo->rsp_len = (pbuf[2] & 0xF0) >> 4;
		mpinfo->rpt_ver = pbuf[2] & 0x0F;
		seq = (pbuf[2] & 0xF0) >> 4;

		PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
			  "[BTC], %s(): seq %d.\n",
			  __func__, seq);
		if (seq != mpinfo->exp_seq) {
			PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
			  "[BTC], %s(): seq %d is not matched,"
			  " expected seq (%d)\n",
			  __func__, seq, mpinfo->exp_seq);
			mpinfo->exp_seq = seq + 1;
			mpinfo->exp_seq &= 0x0F;
		} else {
			mpinfo->exp_seq++;
			mpinfo->exp_seq &= 0x0F;
		}

		if (mpinfo->rpt_sts != BT_STS_OK) {
			PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_,
				  "[BTC], %s(): recv bt mp operate error (%d)!\n",
				  __func__, mpinfo->rpt_sts);
			break;
		}

		if (mpinfo->rsp_len) {
			hal_mem_cpy(h, mpinfo->rsp_data, pbuf + 3, buf_len - 3);
			_update_bt_report(btc, mpinfo->rpt_op[seq], &mpinfo->rpt_param[seq]);
		}
		break;
	case EXTEND_C2H_WIFI_FW_ACTIVE_RSP:
		break;
	default:
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_,
			  "[BTC], %s(): not support extid(%d)\n",
			  __func__, extid);
		return;
	}

#if 0
	mpinfo->rpt_sts = pbuf[1] & 0x0F;
	mpinfo->rsp_len = (pbuf[1] & 0xF0) >> 4;
	mpinfo->rpt_ver = pbuf[2] & 0x0F;
	mpinfo->rpt_seq = (pbuf[2] & 0xF0) >> 4;
	if (mpinfo->rsp_len) {
		hal_mem_cpy(h, mpinfo->rsp_data, pbuf + 3, mpinfo->rsp_len);
	}
	printk("%s, extid 0x%02x, rpt_sts 0x%02x, rpt_ver 0x%02x, rpt_seq 0x%02x.\n", __func__, extid, mpinfo->rpt_sts, mpinfo->rpt_ver, mpinfo->rpt_seq);
#endif

	return;
}

static void _append_tdma(struct btc_t *btc, bool force_exec)
{
	struct btc_dm *dm = &btc->dm;
	struct btf_tlv *tlv = NULL;
	struct fbtc_1tdma *v = NULL;
	u16 len = btc->policy_len;

	if (!force_exec && !_tdma_cmp(&dm->tdma, &dm->tdma_now)) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): tdma no change!\n",
			  __func__);
		return;
	}

	tlv = (struct btf_tlv *)&btc->policy[len];
	tlv->type = CXPOLICY_TDMA;
	tlv->len = sizeof(struct fbtc_1tdma);
	v = (struct fbtc_1tdma *)&tlv->val[0];
	v->fver = FCX_TDMA_VER;

	_tdma_cpy(&v->tdma, &dm->tdma);

	btc->policy_len = len + 2 + sizeof(struct fbtc_1tdma);

	PHL_INFO("[BTC], %s: type:%d, rxflctrl=%d, txflctrl=%d, wtgle_n=%d, leak_n=%d, ext_ctrl=%d, rxflctrl_role=0x%x\n",
		 __func__, dm->tdma.type, dm->tdma.rxflctrl, dm->tdma.txflctrl,
		 dm->tdma.wtgle_n, dm->tdma.leak_n, dm->tdma.ext_ctrl,
		 dm->tdma.rxflctrl_role);
}

static void _append_slot(struct btc_t *btc, bool force_exec)
{
	struct btc_dm *dm = &btc->dm;
	struct btf_tlv *tlv = NULL;
	struct fbtc_1slot *v = NULL;
	u16 len = 0;
	u8 i, cnt = 0;

	for (i = 0; i < CXST_MAX; i++) {
		if (!force_exec && !_slot_cmp(&dm->slot[i], &dm->slot_now[i])) {
			continue;
		}

		len = btc->policy_len;

		tlv = (struct btf_tlv *)&btc->policy[len];
		tlv->type = CXPOLICY_SLOT;
		tlv->len = sizeof(struct fbtc_1slot);
		v = (struct fbtc_1slot *)&tlv->val[0];

		v->fver = FCX_SLOT_VER;
		v->sid = i;
		_slot_cpy(&v->slot, &dm->slot[i]);

		PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
			  "[BTC], %s: slot-%d: dur=%d, table=0x%08x, type=%d\n",
			  __func__, i, dm->slot[i].dur, dm->slot[i].cxtbl,
			  dm->slot[i].cxtype);
		cnt++;
		btc->policy_len = len + 2 + sizeof(struct fbtc_1slot);
	}

	if (cnt > 0) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
			  "[BTC], %s: slot update (cnt=%d)!!\n", __func__, cnt);
	}
}

static void _convert_policy(struct btc_t *btc, bool force_exec,
			    struct btc_h2c_tdma *h2c, struct btc_table *tbl)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;
	struct fbtc_tdma *t = &dm->tdma;
	struct fbtc_slot *s = dm->slot;
	struct fbtc_slot s_wl = {0}, s_bt = {0};
	u8 h2c_para[2] = {0x0, 0x0};
	static u8 old_leak_n;

	PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
		  "[BTC], %s(): type %d, rxflctrl %d, txflctrl %d, "
		  "wtgle_n %d, leak_n %d, ext_ctrl %d.\n",
		  __func__, t->type, t->rxflctrl, t->txflctrl,
		  t->wtgle_n, t->leak_n, t->ext_ctrl);

	memset(h2c, 0, sizeof(struct btc_h2c_tdma));
	memcpy(&s_bt, &s[CXST_B1], sizeof(struct fbtc_slot));
	memcpy(&s_wl, &s[CXST_W1], sizeof(struct fbtc_slot));

	if ((t->type == CXTDMA_FIX) \
	    || (t->type == CXTDMA_AUTO) \
	    || (t->type == CXTDMA_AUTO2)) {
		h2c->en = 1;
		h2c->sco = 1;
		h2c->chg_coex_tbl = 1;
		if (t->type == CXTDMA_AUTO) {
			h2c->bt_auto_slot = 1;
		} else if (t->type == CXTDMA_AUTO2) {
			h2c->bt_auto_slot = 1;
			h2c->pan_en = 1;
		} else {
			h2c->bt_auto_slot = 0;
		}
	} else if (t->ext_ctrl == CXECTL_B2) {
		memcpy(&s_wl, &s[CXECTL_B2], sizeof(struct fbtc_slot));
		h2c->en = 1;
		h2c->sco = 1;
	} else {
		memcpy(&s_bt, &s[CXST_OFF], sizeof(struct fbtc_slot));
		memcpy(&s_wl, &s[CXST_OFF], sizeof(struct fbtc_slot));
		goto exit_set_table;
	}

	if (t->rxflctrl == CXFLC_NULLP) {
		h2c->auto_wkup = 1;
		h2c->no_ps = 0;
		h2c->ps_poll = 0;
		h2c->cts2self = 0;
		h2c->wl_dym_slot = 1;
	} else if (t->rxflctrl == CXFLC_QOSNULL) {
		h2c->auto_wkup = 1;
		h2c->no_ps = 0;
		h2c->ps_poll = 1;
		h2c->cts2self = 0;
	} else if (t->rxflctrl == CXFLC_CTS) {
		h2c->auto_wkup = 0;
		h2c->no_ps = 1;
		h2c->ps_poll = 0;
		h2c->cts2self = 1;
	} else {
		h2c->auto_wkup = 0;
		h2c->no_ps = 0;
		h2c->ps_poll = 0;
		h2c->cts2self = 0;
	}

	if (t->txflctrl == CXTPS_ON) {
		h2c->no_tx_pause = 0;
	} else {
		h2c->no_tx_pause = 1;
	}

	if (old_leak_n != t->leak_n) {
		if (t->leak_n) {
			h2c_para[0] = 0xC;
			h2c_para[1] = 0x0;
			ops->fw_cmd(btc, BTFC_SET, SET_H2C_TEST, h2c_para, 2);
		} else {
			h2c_para[0] = 0xC;
			h2c_para[1] = 0x1;
			ops->fw_cmd(btc, BTFC_SET, SET_H2C_TEST, h2c_para, 2);
		}
		old_leak_n = t->leak_n;
	}

	h2c->tbtt = s_wl.dur;
	if (s_wl.cxtype == SLOT_ISO) { /* 0x778 = d */
		h2c->allow_bt_hi_pri = 1;
		h2c->ext_r778 = 0;
	} else if (s_wl.cxtype == SLOT_MIX) { /* 0x778 = 1 */
		h2c->allow_bt_hi_pri = 0;
		h2c->ext_r778 = 1;
	}

	if (s_bt.cxtype == SLOT_ISO) { /* 0x778 = d */
		h2c->r778_in_bt = 1;
	} else if (s_bt.cxtype == SLOT_MIX) { /* 0x778 = 1 */
		h2c->r778_in_bt = 0;
	}

exit_set_table:
	tbl->bt = s_bt.cxtbl;
	tbl->wl = s_wl.cxtbl;
}

static bool _set_tdma(struct btc_t *btc, const char *action, u8 byte1,
		      u8 byte2, u8 byte3, u8 byte4, u8 byte5)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u8 wl_link_mode = wl_rinfo->link_mode;
	u8 real_byte1 = byte1, real_byte5 = byte5;

	if (!ops || !ops->fw_cmd) {
		return false;
	}

	PHL_INFO("[BTC], enter %s(): h2c 0x60 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			 __func__, byte1, byte2, byte3,
			 byte4, byte5);

	if ((wl_link_mode == BTC_WLINK_2G_AP) \
	     && (byte1 & BIT(4) && !(byte1 & BIT(5)))) {
		PHL_INFO("[BTC], %s(): action=%s, AP mode.\n",
			 __func__, action);

		real_byte1 &= ~BIT(4);
		real_byte1 |= BIT(5);

		real_byte5 |= BIT(5);
		real_byte5 &= ~BIT(6);
	} else if (byte1 & BIT(4) && !(byte1 & BIT(5))) {
		PHL_INFO("[BTC], %s(): action=%s,  Force LPS (byte1 = 0x%x)\n",
			 __func__, action, byte1);
	} else {
		PHL_INFO("[BTC], %s(): action=%s,  native power save (byte1 = 0x%x)\n",
			 __func__, action, byte1);
	}

	dm->dr_tdma[0] = real_byte1;
	dm->dr_tdma[1] = byte2;
	dm->dr_tdma[2] = byte3;
	dm->dr_tdma[3] = byte4;
	dm->dr_tdma[4] = real_byte5;

	ops->fw_cmd(btc, BTFC_SET, SET_B_TYPE_TDMA, dm->dr_tdma, 5);

	PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
		  "[BTC], %s(): h2c 0x60 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		  __func__, dm->dr_tdma[0], dm->dr_tdma[1], dm->dr_tdma[2],
		  dm->dr_tdma[3], dm->dr_tdma[4]);

	/* Always forec excute rtw_btc_set_table To avoid
	 * coex table error if wl slot toggle mode on ->off
	 * ex: 5508031054 next state -> rtw_btc_table + 5108031054
	 * rtw_btc_table may be changed by 5508031054
	 */
	if (real_byte1 & BIT(2)) {
		wl->status.map.slot_toggle = 1;
		wl->status.map.slot_toggle_change = 0;
	} else {
		wl->status.map.slot_toggle_change =wl->status.map.slot_toggle;
		wl->status.map.slot_toggle = 0;
	}

	return true;
}

static bool _set_table(struct btc_t *btc, bool force_exec, u32 val0x6c0,
		       u32 val0x6c4)
{
	struct btc_dm *dm = &btc->dm;
	u32 val1 = 0x0, val2 = 0x0;

	rtw_hal_mac_coex_reg_read(btc->hal, REG_BT_COEX_TABLE_V1, &val1);
	rtw_hal_mac_coex_reg_read(btc->hal, REG_BT_COEX_TABLE2_V1, &val2);
	/* If last tdma is wl slot toggle, force write table*/
	if (!force_exec && !(run_rsn("_ntfy_power_off") ||
		run_rsn("_ntfy_radio_state"))) {
		if ((val0x6c0 == val1) && (val0x6c4 == val2)) {
			return false;
		}
	}

	rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_TABLE_V1, val0x6c0);
	rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_TABLE2_V1, val0x6c4);

	if (btc->chip->break_table_limit) {
		rtw_hal_mac_coex_reg_read(btc->hal, REG_BT_COEX_ENH, &val1);
		val1 = val1  & (~BIT_DIS_BT_DONT_CARE);
		rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_ENH, val1);
		rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_BREAK_TABLE, 0xF3FFFFFF);
	} else {
		rtw_hal_mac_coex_reg_read(btc->hal, REG_BT_COEX_ENH, &val1);
		val1 = val1 | BIT_DIS_BT_DONT_CARE;
		rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_ENH, val1);
		rtw_hal_mac_coex_reg_write(btc->hal, REG_BT_COEX_BREAK_TABLE, 0xFFFFFFFF);
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
		  "[BTC], %s(): 0x6c0 = %x, 0x6c4 = %x\n",
		   __func__, val0x6c0, val0x6c4);

	return true;
}

/*
 * extern functions
 */

void hal_btc_fw_btmp_init(struct btc_t *btc)
{
	struct btc_btmpinfo *mpinfo = &btc->fwinfo.btmpinfo;
	struct rtw_hal_com_t *h = btc->hal;

	mpinfo->oper_seq = 0;
	mpinfo->exp_seq = 0;
	mpinfo->rpt_sts = 0;

	mpinfo->rsp_len = 0;
	mpinfo->rpt_sts = 0;
}

void hal_btc_fw_btmp_deinit(struct btc_t *btc)
{
	struct btc_btmpinfo *mpinfo = &btc->fwinfo.btmpinfo;
	struct rtw_hal_com_t *h = btc->hal;
}

bool hal_btc_fw_send_btmp_oper(struct btc_t *btc, u8 op, u8 op_ver,
			       u8* cmd, u8 size, u8 param)
{
	struct btc_btmpinfo *mpinfo = &btc->fwinfo.btmpinfo;
	struct rtw_hal_com_t *h = btc->hal;
	u8 buf[H2C_BTMP_OPER_LEN] = {0}, i = 0, seq = 0;
	bool tmout = true, ret = false;

	if (size + 2 > H2C_BTMP_OPER_LEN) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_,
			  "[BTC], %s(): size is exceeded!\n",
			  __func__);
		return false;
	}

	seq = (mpinfo->oper_seq & 0x0F);
	PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_,
			  "[BTC], %s(): op 0x%02x, op_ver 0x%02x, seq %d!\n",
			  __func__, op, op_ver, seq);

	mpinfo->oper_seq++;
	mpinfo->oper_seq &= 0x0F;
	buf[0] = (seq << 4) | (op_ver % 0x0F);
	buf[1] = op;
	mpinfo->rpt_op[seq] = op;
	mpinfo->rpt_param[seq] = param;
	hal_mem_cpy(h, buf + 2, cmd, size);
	hal_btc_fw_set_bt(btc, SET_BT_MP_OPER, H2C_BTMP_OPER_LEN, buf);

	ret = true;
exit:
	return ret;
}

void hal_btc_fw_get_bt_rpt(struct btc_t *btc, u8 op)
{
	struct btf_fwinfo *fwinfo = &btc->fwinfo;
	struct btc_bt_info *bt = &btc->cx.bt;
	u8 buf[H2C_BTMP_OPER_LEN] = {0};
	u8 *cmd = NULL;
	u8 prama = 0;
	u8 size = 0;
	bool ret = false;

	switch (op) {
	case BT_OP_GET_BT_BLE_SCAN_PARA:
		prama = bt->scan_type;
		break;
	case BT_OP_GET_BT_VERSION:
	case BT_OP_GET_BT_COEX_SUPPORTED_FEATURE:
	case BT_OP_GET_BT_COEX_SUPPORTED_VERSION:
	case BT_OP_GET_BT_BLE_SCAN_TYPE:
	case BT_LO_OP_GET_AFH_MAP_L:
	case BT_LO_OP_GET_AFH_MAP_M:
	case BT_LO_OP_GET_AFH_MAP_H:
	case BT_OP_GET_BT_DEVICE_INFO:
		break;
	default:
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_,
			  "[BTC], %s(): not support bt mp operate(%d)!\n",
			  __func__, op);
		return;
	}

	ret = hal_btc_fw_send_btmp_oper(btc, op, 0, cmd, size, prama);

	return;
}

bool hal_btc_fw_set_1tdma(struct btc_t *btc, u16 len, u8 *buf)
{
	/* for wlcli manual control  */
	struct btc_dm *dm = &btc->dm;

	if (len != sizeof(struct fbtc_tdma)) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_, "[BTC], %s(): return because len != %d\n",
			  __func__, (int)sizeof(struct fbtc_tdma));
		return false;
	} else if (buf[0] >= CXTDMA_MAX) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_, "[BTC], %s(): return because tdma_type >= %d\n",
			  __func__, CXTDMA_MAX);
		return false;
	}

	_tdma_cpy(&dm->tdma, buf);
	return true;
}

bool hal_btc_fw_set_1slot(struct btc_t *btc, u16 len, u8 *buf)
{
	/* for wlcli manual control  */
	struct btc_dm *dm = &btc->dm;

	if (len != sizeof(struct fbtc_slot) + 1) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_, "[BTC], %s(): return because len != %d\n",
			  __func__, (int)sizeof(struct fbtc_slot) + 1);
		return false;
	} else if (buf[0] >= CXST_MAX) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_ERR_, "[BTC], %s(): return because slot_id >= %d\n",
			  __func__, CXST_MAX);
		return false;
	}

	_slot_cpy(&dm->slot[buf[0]], &buf[1]);
	return true;
}

bool hal_btc_fw_set_policy(struct btc_t *btc, bool force_exec, u16 policy_type,
			   const char *action)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;
	struct btc_h2c_tdma h2c = {0};
	struct btc_table table = {0};
	u8 *h2c_buf = (u8 *)&h2c;

	if (!ops || !ops->fw_cmd) {
		return false;
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_INFO_, "[BTC], action %s():", action);
	_act_cpy(dm->run_action, (char *)action);
	_update_dm_step(btc, action);
	_update_dm_step(btc, id_to_str(BTC_STR_POLICY, (u32)policy_type));

	if (dm->tdma.rxflctrl == CXFLC_NULLP) {
		btc->hal->btc_ctrl.lps = 1;
	} else {
		btc->hal->btc_ctrl.lps = 0;
	}

	if (btc->hal->btc_ctrl.lps == 1) {
		hal_btc_notify_ps_tdma(btc, btc->hal->btc_ctrl.lps);
	}

	_convert_policy(btc, force_exec, &h2c, &table);
	_set_table(btc, force_exec, table.bt, table.wl);
	_set_tdma(btc, action, h2c_buf[0], h2c_buf[1],
			    h2c_buf[2], h2c_buf[3], h2c_buf[4]);

	_tdma_cpy(&dm->tdma_now, &dm->tdma);
	_slots_cpy(dm->slot_now, dm->slot);

	if (btc->hal->btc_ctrl.lps == 0) {
		hal_btc_notify_ps_tdma(btc, btc->hal->btc_ctrl.lps);
	}

	return true;
}

void hal_btc_fw_set_gpio_dbg(struct btc_t *btc, u8 type, u32 val)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_ops *ops = btc->ops;
	u8 data[7] = {0}, len = 0;

	if (!ops || !ops->fw_cmd || type >= CXDGPIO_MAX) {
		return;
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s()\n", __func__);

	data[0] = FCX_GPIODBG_VER;
	data[1] = 0;
	data[2] = type;

	switch (type) {
	case CXDGPIO_EN_MAP:
		len = sizeof(u32) + 3;
		hal_mem_cpy(h, &data[3], &val, sizeof(u32));
		break;
	case CXDGPIO_MUX_MAP:
		len = sizeof(8) * 2 + 3;
		data[3] = (u8)(val & bMASKB0);
		data[4] = (u8)((val & bMASKB1) >> 8);
		break;
	default:
		return;
	}

	ops->fw_cmd(btc, BTFC_SET, SET_GPIO_DBG, data, len);
}

void hal_btc_fw_set_drv_info(struct btc_t *btc, u8 type)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;
	u8 buf[256] = {0};
	u8 sz = 0, n = 0;

	if (!ops || !ops->fw_cmd || type >= CXDRVINFO_MAX) {
		return;
	}

	switch (type) {
	case CXDRVINFO_INIT:
		n = sizeof(dm->init_info);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_INIT;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &dm->init_info, n);
		break;
	case CXDRVINFO_ROLE:
		n = sizeof(wl->role_info);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_ROLE;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->role_info, n);
		break;
	case CXDRVINFO_CTRL:
		n = sizeof(btc->ctrl);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_CTRL;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &btc->ctrl, n);
		break;
#if 0
	case CXDRVINFO_RFK:
		n = sizeof(wl->rfk_info);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_RFK;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->rfk_info, n);
		break;
	case CXDRVINFO_DBCC:
		n = sizeof(wl->dbcc_info);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_DBCC;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->dbcc_info, n);
		break;
	case CXDRVINFO_SMAP:
		n = sizeof(wl->status);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_SMAP;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->status, n);
		break;
	case CXDRVINFO_RUN:
		n = BTC_RSN_MAXLEN;
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_RUN;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], dm->run_reason, n);
		break;
	case CXDRVINFO_SCAN:
		n = sizeof(wl->scan_info);
		sz = n + 2;

		if (sz > sizeof(buf)) {
			return;
		}

		buf[0] = CXDRVINFO_SCAN;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->scan_info, n);
		break;
#endif
	default:
		return;
	}

	ops->fw_cmd(btc, BTFC_SET, SET_DRV_INFO, (u8 *)buf, sz);
}

void hal_btc_fw_set_drv_event(struct btc_t *btc, u8 type)
{
	struct btc_ops *ops = btc->ops;

	if (!ops || !ops->fw_cmd) {
		return;
	}

	ops->fw_cmd(btc, BTFC_SET, SET_DRV_EVENT, &type, 1);
}

void hal_btc_fw_set_bt(struct btc_t *btc, u8 type, u16 len, u8 *buf)
{
	struct btc_ops *ops = btc->ops;

	if (!ops || !ops->fw_cmd) {
		return;
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s()\n", __func__);

	ops->fw_cmd(btc, BTFC_SET, type, buf, len);
}

void hal_btc_fw_event(struct btc_t *btc, u8 evt_id, void *data, u32 len)
{
	struct btf_fwinfo *pfwinfo = &btc->fwinfo;

	if (!len || !data) {
		return;
	}

	switch (evt_id) {
	case BTF_EVNT_RPT:
		_parse_btc_report(btc, pfwinfo, data, len);
		break;
	default:
		break;
	}
}

#endif
