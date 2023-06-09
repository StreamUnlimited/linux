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
#define _HAL_TRX_8730E_C_
#include "../hal_headers.h"
#include "rtl8730e_hal.h"

static void _hal_trx_8730e_dump_rxcnt(struct hal_ppdu_sts *ppdu_sts)
{
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "ppdu_sts->rx_cnt_size = 0x%x\n", ppdu_sts->rx_cnt_size);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_OFDM_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_OFDM_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_OFDM_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_OFDM_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_OFDM_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_OFDM_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_CCK_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_CCK_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_CCK_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_CCK_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_CCK_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_CCK_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HT_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HT_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HT_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HT_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HT_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HT_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HT_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HT_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTSU_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTSU_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTSU_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTSU_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTSU_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTSU_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTSU_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTSU_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTMU_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTMU_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTMU_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTMU_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTMU_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTMU_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_VHTMU_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_VHTMU_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HESU_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HESU_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HESU_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HESU_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HESU_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HESU_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HESU_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HESU_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HEMU_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HEMU_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HEMU_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HEMU_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HEMU_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HEMU_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HEMU_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HEMU_FAM]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HETB_OK = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HETB_OK]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HETB_FAIL = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HETB_FAIL]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HETB_PPDU = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HETB_PPDU]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_HETB_FAM = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_HETB_FAM]);

	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_FULLDRP = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_FULLDRP]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_FULLDRP_PKT = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_FULLDRP_PKT]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_PKTFLTR_DRP = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_PKTFLTR_DRP]);
	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_, "HAL_RXCNT_RXDMA = 0x%x\n",
		  ppdu_sts->rx_cnt.ppdu_cnt[HAL_RXCNT_RXDMA]);
}

static void _hal_dump_rxdesc(u8 *buf, struct rtw_r_meta_data *mdata)
{
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "%s ==>\n", __FUNCTION__);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pktlen = 0x%X\n", mdata->pktlen);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->shift = 0x%X\n", mdata->shift);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->wl_hd_iv_len = 0x%X\n",
		  mdata->wl_hd_iv_len);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->mac_info_vld = 0x%X\n",
		  mdata->mac_info_vld);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rpkt_type = 0x%X\n",
		  mdata->rpkt_type);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->sniffer_len = 0x%X\n",
		  mdata->sniffer_len);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_rate = 0x%X\n",
		  mdata->rx_rate);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_gi_ltf = 0x%X\n",
		  mdata->rx_gi_ltf);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->inter_ppdu = 0x%X\n",
		  mdata->inter_ppdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bw = 0x%X\n",
		  mdata->bw);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->freerun_cnt = 0x%X\n",
		  mdata->freerun_cnt);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->a1_match = 0x%X\n",
		  mdata->a1_match);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->sw_dec = 0x%X\n",
		  mdata->sw_dec);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->ampdu = 0x%X\n",
		  mdata->ampdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->amsdu = 0x%X\n",
		  mdata->amsdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->amsdu_cut = 0x%X\n",
		  mdata->amsdu_cut);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->last_msdu = 0x%X\n",
		  mdata->last_msdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->crc32 = 0x%X\n",
		  mdata->crc32);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->icverr = 0x%X\n",
		  mdata->icverr);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->magic_wake = 0x%X\n",
		  mdata->magic_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->unicast_wake = 0x%X\n",
		  mdata->unicast_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pattern_wake = 0x%X\n",
		  mdata->pattern_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_statistics = 0x%X",
		  mdata->rx_statistics);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pattern_idx = 0x%X",
		  mdata->pattern_idx);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->frame_type = 0x%X",
		  mdata->frame_type);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->mc = 0x%X",
		  mdata->mc);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bc = 0x%X",
		  mdata->bc);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->more_data = 0x%X",
		  mdata->more_data);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->more_frag = 0x%X",
		  mdata->more_frag);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pwr_bit = 0x%X",
		  mdata->pwr_bit);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->qos = 0x%X",
		  mdata->qos);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->tid = 0x%X",
		  mdata->tid);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->eosp = 0x%X",
		  mdata->eosp);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->htc = 0x%X",
		  mdata->htc);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->seq = 0x%X",
		  mdata->seq);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->frag_num = 0x%X",
		  mdata->frag_num);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->macid = 0x%X\n",
		  mdata->macid);
}

static enum rtw_hal_status
hal_handle_chaninfo_8730e(struct hal_info_t *hal) {
	return RTW_HAL_STATUS_SUCCESS;
}

/**
* SW Parsing Rx Desc - hal_parsing_rx_wd_8730e
* description:
* 	Parsing Rx WiFi Desc by Halmac or SW Manually
* input:
* 	hal : hal ic adapter
* 	desc : pointer of the start of rx desc
* output:
* 	pkt : pointer of the start of pkt;
* 	pkt_len : the pkt length
* 	rxwd : rx desc
*/
static enum rtw_hal_status
_hal_parsing_rx_wd_8730e(struct hal_info_t *hal, u8 *desc,
			 struct rtw_r_meta_data *mdata) {
	/* ToDo: Parse word by word with byte swap once for
	 * each word
	 */
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u8 hw_sec_type = HAL_SEC_TYPE_NONE, ext_sectype = 0;

	mdata->shift = GET_RX_AX_DESC_MH_SHIFT_8730E(desc);
	mdata->sniffer_len = GET_RX_AX_DESC_SNIFFER_LEN_8730E(desc);
	mdata->mac_info_vld = 0;

	mdata->c2h = GET_RX_AX_DESC_C2H_8730E(desc);
	if (mdata->c2h) {
		mdata->rpkt_type = RX_8730E_DESC_PKT_T_C2H;
	} else {
		mdata->rpkt_type = GET_RX_AX_DESC_PKT_INDICATE_8730E(desc);
	}

	switch (mdata->rpkt_type) {
	case RX_8730E_DESC_PKT_T_WIFI:
		mdata->pwr_lv = GET_RX_AX_DESC_RX_PWR_LV_8730E(desc);
		mdata->central_ch = GET_RX_AX_DESC_RX_CENTRAL_CH_8730E(desc);
		mdata->pktlen = GET_RX_AX_DESC_PKT_LEN_8730E(desc);
		/*https://jira.realtek.com/browse/AMEBALITE-190*/
		mdata->pktlen -= 4;
		break;
	case RX_8730E_DESC_PKT_T_WIFI_NDPA_CH_INFO:
	case RX_8730E_DESC_PKT_T_RESP_CH_INFO:
		mdata->pktlen = GET_RX_AX_DESC_CH_INFO_CSI_LEN_8730E(desc) * 8;
		mdata->rx_bw = GET_RX_AX_DESC_CH_INFO_BW_8730E(desc);
		break;
	case RX_8730E_DESC_PKT_T_PPDU_STATUS:
		mdata->pktlen = GET_RX_AX_DESC_PHYSTS_LEN_8730E(desc) * 8;
		mdata->rx_is_data = GET_RX_AX_DESC_PHYSTS_OF_DATA_8730E(desc);
		break;
	default:
		mdata->pktlen = GET_RX_AX_DESC_PKT_LEN_8730E(desc);
		break;
	}

	mdata->wl_hd_iv_len = GET_RX_AX_DESC_WLANHD_IV_LEN_8730E(desc);

	mdata->rx_rate = GET_RX_AX_DESC_RX_RATE_8730E(desc);
	mdata->rx_gi_ltf = GET_RX_AX_DESC_GI_LTF_8730E(desc);
	mdata->rxmpdu_bssid_fit = GET_RX_AX_DESC_BSSID_FIT_8730E(desc);
	mdata->inter_ppdu = GET_RX_AX_DESC_INTER_PPDU_8730E(desc);
	mdata->bw = GET_RX_AX_DESC_BW_8730E(desc);

	mdata->freerun_cnt = GET_RX_AX_DESC_TSFL_8730E(desc);

	mdata->a1_match = GET_RX_AX_DESC_PAM_8730E(desc);
	mdata->sw_dec = GET_RX_AX_DESC_SWDEC_8730E(desc);
	mdata->ampdu = GET_RX_AX_DESC_AMPDU_8730E(desc);
	mdata->amsdu = GET_RX_AX_DESC_AMSDU_8730E(desc);
	mdata->crc32 = GET_RX_AX_DESC_CRC32_ERR_8730E(desc);
	mdata->icverr = GET_RX_AX_DESC_ICV_MIC_ERR_8730E(desc);
	mdata->magic_wake = GET_RX_AX_DESC_MAGIC_WAKE_8730E(desc);
	mdata->unicast_wake = GET_RX_AX_DESC_UNICAST_WAKE_8730E(desc);
	mdata->pattern_wake = GET_RX_AX_DESC_PATTERN_MATCH_8730E(desc);
	mdata->rx_statistics = GET_RX_AX_DESC_STATISTICS_8730E(desc);
	mdata->pattern_idx = GET_RX_AX_DESC_PATTERN_IDX_8730E(desc);

	mdata->frame_type = GET_RX_AX_DESC_FRAME_TYPE_8730E(desc);
	mdata->mc = GET_RX_AX_DESC_MC_8730E(desc);
	mdata->bc = GET_RX_AX_DESC_BC_8730E(desc);
	mdata->more_data = GET_RX_AX_DESC_MD_8730E(desc);
	mdata->more_frag = GET_RX_AX_DESC_MF_8730E(desc);
	mdata->pwr_bit = GET_RX_AX_DESC_PWR_8730E(desc);
	mdata->qos = GET_RX_AX_DESC_QOS_8730E(desc);

	mdata->tid = GET_RX_AX_DESC_TID_8730E(desc);
	mdata->eosp = GET_RX_AX_DESC_EOSP_8730E(desc);
	mdata->htc = GET_RX_AX_DESC_HTC_8730E(desc);

	mdata->seq = GET_RX_AX_DESC_SEQ_8730E(desc);
	mdata->frag_num = GET_RX_AX_DESC_FRAG_8730E(desc);

	mdata->macid_vld = GET_RX_AX_DESC_MACID_VLD_8730E(desc);
	hw_sec_type = GET_RX_AX_DESC_SEC_TYPE_8730E(desc);
	ext_sectype = GET_RX_AX_DESC_EXT_SECTYPE_8730E(desc);
	switch (hw_sec_type) {
	case HAL_SEC_TYPE_NONE:
		mdata->sec_type = RTW_ENC_NONE;
		break;
	case HAL_SEC_TYPE_WEP40:
		mdata->sec_type = RTW_ENC_WEP40;
		break;
	case HAL_SEC_TYPE_TKIP_NO_MIC:
	case HAL_SEC_TYPE_TKIP_MIC:
		mdata->sec_type = RTW_ENC_TKIP;
		break;
	case HAL_SEC_TYPE_AES:
		if (ext_sectype) {
			mdata->sec_type = RTW_ENC_CCMP256;
		} else {
			mdata->sec_type = RTW_ENC_CCMP;
		}
		break;
	case HAL_SEC_TYPE_WEP104:
		mdata->sec_type = RTW_ENC_WEP104;
		break;
	case HAL_SEC_TYPE_WAPI_SMS4:
		if (ext_sectype) {
			mdata->sec_type = RTW_ENC_GCMSMS4;
		} else {
			mdata->sec_type = RTW_ENC_WAPI;
		}
		break;
	case HAL_SEC_TYPE_GCMP:
		if (ext_sectype) {
			mdata->sec_type = RTW_ENC_GCMP256;
		} else {
			mdata->sec_type = RTW_ENC_GCMP;
		}
		break;
	}

	mdata->macid = GET_RX_AX_DESC_MACID_8730E(desc);
	mdata->macid_rxdesc = GET_RX_AX_DESC_TXRPTMID_SRCH_8730E(desc);

	if (mdata->pktlen == 0)
	{
		hstatus = RTW_HAL_STATUS_FAILURE;
	} else
	{
		hstatus = RTW_HAL_STATUS_SUCCESS;
	}

	return hstatus;
}

/**
 * hal_parsing_rx_wd_8730e
 * description:
 * 	Parsing Rx WiFi Desc by Halmac or SW Manually
 * input:
 * 	hal : hal ic adapter
 * 	desc : pointer of the start of rx desc
 * output:
 * 	pkt : pointer of the start of pkt;
 * 	pkt_len : the pkt length
 * 	rxwd : rx desc
 */

enum rtw_hal_status
hal_parsing_rx_wd_8730e(struct rtw_phl_com_t *phl_com,
			struct hal_info_t *hal,
			u8 *buf, u8 **pkt, u16 *pkt_len,
			struct rtw_r_meta_data *mdata) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	u8 halmac_rx = 0;
	u8 shift = 0;
	u8 *desc = NULL;
	u8 desc_l = 0;

	do
	{
		if (NULL == buf) {
			break;
		}

		desc = buf;

		if (!halmac_rx) {
			hstatus = _hal_parsing_rx_wd_8730e(hal, desc, mdata);
		} else {
			/* halmac_ax_ops->parse_rxdesc( */
			/* hm_info->halmac_ax_apter, */
			/* rxpkt_info, */
			/* buf, */
			/* len); */
		}


		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			break;
		}
		/* TODO :: Need Double Check*/
		desc_l = RX_DESC_SIZE_8730E;

		shift = (u8)(mdata->shift * 2 + desc_l);

		if ((1 == mdata->mac_info_vld) &&
		    (RX_8730E_DESC_PKT_T_PPDU_STATUS != mdata->rpkt_type)) {
			*pkt = desc + shift + RX_PPDU_MAC_INFO_SIZE_8730E;
		} else {
			*pkt = desc + shift;
		}

		*pkt_len = (u16)mdata->pktlen;

	} while (false);

	_hal_dump_rxdesc(desc, mdata);

	return hstatus;
}


static void
_hal_rx_wlanhdr_check_8730e(void *drvpriv, void *hdr, struct rtw_r_meta_data *mdata)
{
	/*Check Retry BIT*/
	u8 retry = 0;
	u16 seq_num = 0;
	u8 type = 0;

	type = PHL_GET_80211_HDR_TYPE(hdr);
	retry = PHL_GET_80211_HDR_RETRY(hdr);
	seq_num = PHL_GET_80211_HDR_SEQUENCE(hdr);
	PHL_GET_80211_HDR_ADDRESS2(drvpriv, hdr, mdata->ta);
	PHL_GET_80211_HDR_ADDRESS1(drvpriv, hdr, mdata->ra);

	if (retry) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_DEBUG_, "[TP Debug] RETRY PKT!!!! seq_num = %d \n", seq_num);
	}
	if (type == 9) { /* trigger */
		PHL_TRACE(COMP_PHL_TRIG, _PHL_INFO_, "rx trigger\n");
		PHL_TRACE(COMP_PHL_TRIG, _PHL_INFO_,
			  "macid = %d\n", mdata->macid);
	}
}

static void
_hal_rx_sts_8730e(struct hal_info_t *hal, struct rtw_r_meta_data *meta)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct rtw_trx_stat *trx_stat = &hal_com->trx_stat;

	/* todo: shall drop rx in PHL if hw err */
	if (meta->crc32 || meta->icverr) {
		trx_stat->rx_err_cnt++;
	} else {
		trx_stat->rx_ok_cnt++;
	}
}

enum rtw_hal_status
hal_handle_rx_buffer_8730e(struct rtw_phl_com_t *phl_com,
			   struct hal_info_t *hal,
			   u8 *buf, u32 buf_len,
			   struct rtw_phl_rx_pkt *phl_rx) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	void *drv = hal_to_drvpriv(hal);
	struct rtw_recv_pkt *r = &phl_rx->r;
	struct rtw_pkt_buf_list *pkt = &r->pkt_list[0];
	struct rtw_r_meta_data *mdata = &r->mdata;
#ifdef CONFIG_PHL_TEST_SUITE
	struct test_bp_info bp_info;
#endif
	hstatus = hal_parsing_rx_wd_8730e(phl_com, hal, buf,
					  &pkt->vir_addr, &pkt->length, mdata);

	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
		return hstatus;
	}
	if ((pkt->vir_addr + pkt->length) > (buf + buf_len)) {
		return RTW_HAL_STATUS_FAILURE;
	}

	/* hana_todo */
	r->pkt_cnt = 1;

	switch (mdata->rpkt_type) {
	case RX_8730E_DESC_PKT_T_WIFI: {
		phl_rx->type = RTW_RX_TYPE_WIFI;
		_hal_rx_wlanhdr_check_8730e(drv, pkt->vir_addr, mdata);
		_hal_rx_sts_8730e(hal, mdata);
		hal_rx_ppdu_sts_normal_data(phl_com, pkt->vir_addr, mdata);
	}
	break;
	case RX_8730E_DESC_PKT_T_PPDU_STATUS: {
		struct hal_ppdu_sts ppdu_sts = {0};
		u8 is_su = 1;

		phl_rx->type = RTW_RX_TYPE_PPDU_STATUS;

		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "==> RX_8730E_DESC_PKT_T_PPDU_STATUS : ppdu_cnt  %d ; freerun_cnt %d \n",
			mdata->ppdu_cnt, mdata->freerun_cnt);
		rtw_hal_mac_ax_parse_ppdu_sts(hal, mdata->mac_info_vld,
					      pkt->vir_addr, mdata->pktlen,
					      (void *)&ppdu_sts,
					      (void *)mdata);
		if (ppdu_sts.rx_cnt_size != 0) {
			_hal_trx_8730e_dump_rxcnt(&ppdu_sts);
		}
		if (ppdu_sts.phy_st_size != 0) {
			if (rtw_hal_bb_parse_phy_sts(hal,
						     (void *)&ppdu_sts,
						     phl_rx,
						     is_su) != RTW_HAL_STATUS_SUCCESS)
				PHL_TRACE(COMP_PHL_PSTS, _PHL_DEBUG_,
					  "rtw_hal_bb_parse_phy_sts fail\n");


			hal_rx_ppdu_sts(phl_com, phl_rx, &ppdu_sts);
#ifdef CONFIG_PHL_TEST_SUITE
			bp_info.type = BP_INFO_TYPE_MP_RX_PHYSTS;
			bp_info.ptr = (void *)(ppdu_sts.phy_st_ptr);
			bp_info.len = ppdu_sts.phy_st_size;
			rtw_phl_test_setup_bp(phl_com, &bp_info, TEST_SUB_MODULE_MP);
#endif
		}
	}
	break;
	case RX_8730E_DESC_PKT_T_WIFI_NDPA_CH_INFO:
	case RX_8730E_DESC_PKT_T_RESP_CH_INFO: {
#ifdef  CONFIG_PHL_CHANNEL_INFO
		enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;
		u8 *buf_addr;
		struct ch_rpt_hdr_info ch_hdr_rpt = {0};
		struct phy_info_rpt phy_rpt = {0};
		struct ch_info_drv_rpt drv_rpt = {0};
		u32 idle_num = CHAN_INFO_PKT_TOTAL;
		struct chan_info_t *chan_info_old = NULL;

		phl_rx->type = RTW_RX_TYPE_CHANNEL_INFO;
		/* Channel Report */
		/* TODO: need to discuss the final csi header format further.*/
		idle_num = rtw_phl_get_chaninfo_idle_number(drv, phl_com);

		if (idle_num == CHAN_INFO_PKT_TOTAL) {
			phl_com->chan_info = rtw_phl_query_idle_chaninfo(drv, phl_com);
		}

		if (phl_com->chan_info == NULL) {
			/*hstatus = RTW_HAL_STATUS_SUCCESS is expected*/
			PHL_INFO("channel info packet not avaialbe due to no pakcet handle\n");
			break;
		}
		buf_addr = phl_com->chan_info->chan_info_buffer;
		status = rtw_hal_bb_ch_info_parsing(hal, pkt->vir_addr,
						    mdata->pktlen,
						    buf_addr + phl_com->chan_info->length,
						    &ch_hdr_rpt, &phy_rpt, &drv_rpt);

		if (status == RTW_HAL_STATUS_FAILURE) {
			phl_com->chan_info->length = 0;
		} else {
			phl_com->chan_info->length += drv_rpt.raw_data_len;
		}
		/* store phy info if seg#0 is success*/
		if (drv_rpt.seg_idx_curr == 0 && status != RTW_HAL_STATUS_FAILURE)
			_hal_fill_csi_header_phy_info(hal, &(phl_com->chan_info->csi_header),
						      &ch_hdr_rpt, &phy_rpt);
		if (status == RTW_HAL_STATUS_BB_CH_INFO_LAST_SEG) {
			/* Fill remain csi header to buffer  */
			_hal_fill_csi_header_remain(hal,
						    &(phl_com->chan_info->csi_header), mdata);
			/* push compelete channel info resourecs to busy queue */
			chan_info_old = rtw_phl_recycle_busy_chaninfo(drv, phl_com, phl_com->chan_info);
			if (chan_info_old) {
				rtw_phl_enqueue_idle_chaninfo(drv, phl_com, chan_info_old);
			}
			phl_com->chan_info = rtw_phl_query_idle_chaninfo(drv, phl_com);
			if (phl_com->chan_info == NULL) {
				PHL_INFO("channel info packet not avaialbe after recycle\n");
			}
		}
#else
		phl_rx->type = RTW_RX_TYPE_CHANNEL_INFO;
#endif /* CONFIG_PHL_CHANNEL_INFO */
	}
	break;
	case RX_8730E_DESC_PKT_T_C2H: {
		struct rtw_c2h_info c = {0};

		phl_rx->type = RTW_RX_TYPE_C2H;
		rtw_hal_mac_parse_c2h(hal, pkt->vir_addr, mdata->pktlen, (void *)&c);

		hal_c2h_post_process(phl_com, hal, (void *)&c);
	}
	break;

	default:
		break;
	}

	return hstatus;
}

