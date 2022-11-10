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

#include "trx_desc_8730e.h"
#include "rtl8730e/axi/hal_trx_8730ea.h"
#if MAC_AX_8730E_SUPPORT

#define RXD_RPKT_TYPE_INVALID	0xFF
#define TXD_AC_TYPE_MSK		0x3
#define TXD_TID_IND_SH		2
#define TID_MAX_NUM		8

#define TID_0_QSEL 0
#define TID_1_QSEL 1
#define TID_2_QSEL 1
#define TID_3_QSEL 0
#define TID_4_QSEL 2
#define TID_5_QSEL 2
#define TID_6_QSEL 3
#define TID_7_QSEL 3
#define TID_0_IND 0
#define TID_1_IND 0
#define TID_2_IND 1
#define TID_3_IND 1
#define TID_4_IND 0
#define TID_5_IND 1
#define TID_6_IND 0
#define TID_7_IND 1

#define WP_OFFSET_UNIT_8852A 0x1
#define SEC_HDR_SIZE_4B 0x4
#define SEC_HDR_SIZE_8B 0x8
#define HDR_SIZE_WLAN_MAX 0x24
#define HDR_SIZE_802P3 0xE
#define HDR_SIZE_LLC 0x8
#define HDR_SIZE_AMSDU 0xE

enum wd_info_pkt_type {
	WD_INFO_PKT_NORMAL,

	/* keep last */
	WD_INFO_PKT_LAST,
	WD_INFO_PKT_MAX = WD_INFO_PKT_LAST,
};

static u8 qsel_l[TID_MAX_NUM] = {
	TID_0_QSEL, TID_1_QSEL, TID_2_QSEL, TID_3_QSEL,
	TID_4_QSEL, TID_5_QSEL, TID_6_QSEL, TID_7_QSEL
};

static u8 tid_ind[TID_MAX_NUM] = {
	TID_0_IND, TID_1_IND, TID_2_IND, TID_3_IND,
	TID_4_IND, TID_5_IND, TID_6_IND, TID_7_IND
};

u32 mac_txdesc_len_8730e(struct mac_ax_adapter *adapter,
			 struct rtw_t_meta_data *info)
{
	return TX_DESC_LEN;
}
/*
static u32 txdes_proc_h2c_fwdl_8730e(struct mac_ax_adapter *adapter,
				     struct rtw_t_meta_data *info, u8 *buf, u32 len)
{
	struct wd_body_t *wdb;

	if (len != mac_txdesc_len_8852a(adapter, info)) {
		PLTFM_MSG_ERR("[ERR] illegal len %d\n", len);
		return MACBUFSZ;
	}

	if (info->pktlen > AX_TXD_TXPKTSIZE_MSK || !info->pktlen) {
		PLTFM_MSG_ERR("[ERR] illegal txpktsize %d\n", info->pktlen);
		return MACFUNCINPUT;
	}

	wdb = (struct wd_body_t *)buf;
	wdb->dword0 = cpu_to_le32(SET_WORD(MAC_AX_DMA_H2C, AX_TXD_CH_DMA) |
			(info->type == RTW_PHL_PKT_TYPE_FWDL ? AX_TXD_FWDL_EN : 0));
	wdb->dword1 = 0;
	wdb->dword2 = cpu_to_le32(SET_WORD(info->pktlen, AX_TXD_TXPKTSIZE));
	wdb->dword3 = 0;
	wdb->dword4 = 0;
	wdb->dword5 = 0;

	return MACSUCCESS;
}


#if MAC_AX_FEATURE_HV
static u32 txdes_proc_hv_8852a(struct mac_ax_adapter *adapter,
			       struct rtw_t_meta_data *info,
			       struct wd_body_t *wdb,
			       struct wd_info_t *wdi)
{
	struct hv_txpkt_info *hv_info = (struct hv_txpkt_info *)info->mac_priv;

	wdb->dword0 |= cpu_to_le32((hv_info->chk_en ? AX_TXD_CHK_EN : 0));
	wdi->dword3 |=
		cpu_to_le32((hv_info->null_1 ? AX_TXD_NULL_1 : 0) |
			    (hv_info->null_0 ? AX_TXD_NULL_0 : 0) |
			    (hv_info->tri_frame ? AX_TXD_TRI_FRAME : 0) |
			    (hv_info->ht_data_snd ? AX_TXD_HT_DATA_SND : 0));
	wdi->dword5 |= cpu_to_le32(SET_WORD(hv_info->ndpa_dur, AX_TXD_NDPA_DURATION));

	return MACSUCCESS;
}
#endif
*/
static u32 txdes_proc_data_8730e(struct mac_ax_adapter *adapter,
				 struct rtw_t_meta_data *info, u8 *buf, u32 len)
{
	struct tx_desc_t *txdesc;
	u32 ret;
	u8 qsel, sec_type;

	if (len != mac_txdesc_len_8730e(adapter, info)) {
		PLTFM_MSG_ERR("[ERR] illegal len %d\n", len);
		return MACBUFSZ;
	}

	if (info->pktlen > AX_TXD_TXPKTSIZE_MSK || !info->pktlen) {
		PLTFM_MSG_ERR("[ERR] illegal txpktsize %d\n", info->pktlen);
		return MACFUNCINPUT;
	}

	txdesc = (struct tx_desc_t *)buf;

	txdesc->txdw0 = cpu_to_le32(SET_WORD(info->pktlen, AX_TXD_TXPKTSIZE) |
				    SET_WORD(info->offset, AX_TXD_OFFSET) |
				    (info->bc || info->mc ? AX_TXD_BMC : 0) |
				    (info->bcnpkt_tsf_c ? AX_TXD_BCNPKT_TSF_C : 0) |
				    (info->no_acm ? AX_TXD_NO_ACM : 0) |
				    (info->gf ? AX_TXD_GF : 0));
	if (info->sec_hw_enc) {
		switch (info->sec_type) {
		case RTW_ENC_WEP40:
		case RTW_ENC_WEP104:
		case RTW_ENC_TKIP:
			sec_type = 0x1;
			break;
		case RTW_ENC_WAPI:
		case RTW_ENC_GCMP:
		case RTW_ENC_GCMP256:
		case RTW_ENC_GCMSMS4:
		case RTW_ENC_CCMP256:
			sec_type = 0x2;
			break;
		case RTW_ENC_CCMP:
			sec_type = 0x3;
			break;
		case HAL_SECURITY_TYPE_BIP:
			if (1 == info->bc) {
				sec_type = 0x0;
			} else {
				sec_type = 0x3;
			}
			break;
		default:
			sec_type = 0x0;
			break;

		}
	}

	qsel = qsel_l[info->tid];
	info->pkt_offset = 0;
	txdesc->txdw1 = cpu_to_le32(SET_WORD(info->macid, AX_TXD_MACID) |
				    (info->bip_keyid_sel ? AX_TXD_BIP_KEYID_SEL : 0) |
				    SET_WORD(qsel, AX_TXD_QSEL) |
				    (info->force_txop ? AX_TXD_FORCE_TXOP : 0) |
				    (info->lsig_txop_en ? AX_TXD_LSIG_TXOP_EN : 0) |
				    (info->pifs ? AX_TXD_PIFS : 0) |
				    (info->a_ctrl_cas ? AX_TXD_A_CTRL_CAS : 0) |
				    (info->a_ctrl_bsr ? AX_TXD_A_CTRL_BSR : 0) |
				    (info->a_ctrl_uph ? AX_TXD_A_CTRL_UPH : 0) |
				    (info->a_ctrl_bqr ? AX_TXD_A_CTRL_BQR : 0) |
				    (info->htc ? AX_TXD_HTC : 0) |
				    (info->en_desc_id ? AX_TXD_EN_DESC_ID : 0) |
				    SET_WORD(sec_type, AX_TXD_SEC_TYPE) |
				    SET_WORD(info->pkt_offset, AX_TXD_PKT_OFFSET) |
				    (info->moredata ? AX_TXD_MOREDATA : 0) |
				    (info->ftm_en ? AX_TXD_FTM_EN : 0) |
				    (info->hw_aesiv ? AX_TXD_HW_AESIV : 0));
	txdesc->txdw2 = cpu_to_le32(SET_WORD(info->p_aid, AX_TXD_P_AID) |
				    (info->tri_frame ? AX_TXD_TRI_FRAME : 0) |
				    (SET_WORD(info->rts_cca_mode, AX_TXD_CCA_RTS)) |
				    (info->ampdu_en ? AX_TXD_AGG_EN : 0) |
				    (info->ack_ch_info ? AX_TXD_ACK_CH_INFO : 0) |
				    (info->null0 ? AX_TXD_NULL_0 : 0) |
				    (info->null1 ? AX_TXD_NULL_1 : 0) |
				    (info->bk ? AX_TXD_BK : 0) |
				    (info->tm_en ? AX_TXD_TM_EN : 0) |
				    (info->raw ? AX_TXD_RAW : 0) |
				    (info->spe_rpt ? AX_TXD_CCX : 0) |
				    (SET_WORD(info->ampdu_density, AX_TXD_AMPDU_DENSITY)) |
				    (info->bt_int ? AX_TXD_BT_INT : 0) |
				    (SET_WORD(info->spe_frame_indicate, AX_TXD_SPE_FRAME_INDICATE)) |
				    (info->mu_snd_flag ? AX_TXD_MU_SND_FLAG : 0));

	txdesc->txdw3 = cpu_to_le32(SET_WORD(info->wheader_len, AX_TXD_WHEADER_LEN) |
				    (info->txdesc_info_en ? AX_TXD_TXDESC_INFO_EN : 0) |
				    (SET_WORD(info->hw_ssn_sel, AX_TXD_HW_SSN_SEL)) |
				    (info->userate_sel ? AX_TXD_USEDESRATE : 0) |
				    (info->dis_rts_rate_fb ? AX_TXD_DISRTSFB : 0) |
				    (info->dis_data_rate_fb ? AX_TXD_DISDATAFB : 0) |
				    (info->cts2self ? AX_TXD_CTS_2_SELF : 0) |
				    (info->rts_en ? AX_TXD_RTS_EN : 0) |
				    (info->hw_rts_en ? AX_TXD_HW_RTS_EN : 0) |
				    (info->chksum_en ? AX_TXD_CHKSUM_EN : 0) |
				    (info->nav_use_hdr ? AX_TXD_NAVUSEHDR : 0) |
				    (info->use_max_time ? AX_TXD_USE_MAX_TIME : 0) |
				    (SET_WORD(info->max_agg_num, AX_TXD_MAX_AGG_NUM)) |
				    (SET_WORD(info->ndpa, AX_TXD_NDPA)) |
				    (SET_WORD(info->ampdu_max_time, AX_TXD_AMPDU_MAX_TIME)));

	txdesc->txdw4 = cpu_to_le32(SET_WORD(info->f_rate, AX_TXD_DATARATE) |
				    (SET_WORD(info->data_rty_lowest_rate, AX_TXD_DATA_RTY_LOWEST_RATE)) |
				    (SET_WORD(info->retry_limit, AX_TXD_RETRY_LIMIT)) |
				    (SET_WORD(info->obw_cts2self_dup_type, AX_TXD_OBW_CTS2SELF_DUP_TYPE)) |
				    (info->try_rate ? AX_TXD_TRY_RATE : 0) |
				    (SET_WORD(info->f_gi_ltf, AX_TXD_GI_LTF)));

	txdesc->txdw5 = cpu_to_le32(SET_WORD(info->data_sc, AX_TXD_DATA_SC) |
				    (info->f_dcm ? AX_TXD_DATA_DCM : 0) |
				    (SET_WORD(info->f_bw, AX_TXD_DATA_BW)) |
				    (info->f_ldpc ? AX_TXD_DATA_LDPC : 0) |
				    (SET_WORD(info->f_stbc, AX_TXD_DATA_STBC)) |
				    (SET_WORD(info->rts_stbc, AX_TXD_RTS_STBC)) |
				    (info->rts_short ? AX_TXD_RTS_SHORT : 0) |
				    (SET_WORD(info->rts_sc, AX_TXD_RTS_SC)) |
				    (info->signaling_ta ? AX_TXD_SIGNALING_TA : 0) |
				    (info->data_bw_er ? AX_TXD_DATA_BW_ER : 0) |
				    (info->f_er ? AX_TXD_DATA_ER : 0) |
				    (SET_WORD(info->mbssid, AX_TXD_MBSSID)) |
				    (info->port_id ? AX_TXD_PORT_ID : 0) |
				    (info->polluted ? AX_TXD_POLLUTED : 0));

	txdesc->txdw6 = cpu_to_le32((SET_WORD(info->rtsrate, AX_TXD_RTSRATE)) |
				    (SET_WORD(info->data_rty_lowest_rate, AX_TXD_RTS_RTY_LOWEST_RATE)) |
				    (info->rty_use_des ? AX_TXD_RTY_USE_DES : 0));



	txdesc->txdw7 = cpu_to_le32((SET_WORD(0, AX_TXD_CHECK_SUM)) |
				    (SET_WORD(info->sw_define_l, AX_TXD_SW_DEFINE_L)) |
				    (SET_WORD(info->final_data_rate, AX_TXD_FINAL_DATA_RATE)));

	if (adapter->hw_info->checksum_en) {
		ret = mac_txdesc_checksum_8730e(adapter, info, buf);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] mac_wd_checksum %d\n", ret);
			return ret;
		}
	}

	txdesc->txdw8 = cpu_to_le32((SET_WORD(info->rts_rc, AX_TXD_RTS_RC)) |
				    (SET_WORD(info->bar_rc, AX_TXD_BAR_RC)) |
				    (SET_WORD(info->data_rc, AX_TXD_DATA_RC)) |
				    (info->tx_vendo ? AX_TXD_TX_VENDO : 0) |
				    (info->hw_seq_mode ? AX_TXD_EN_HWSEQ : 0));

	txdesc->txdw9 = cpu_to_le32((SET_WORD(info->group_bit_ie_offset, AX_TXD_GROUP_BIT_IE_OFFSET)) |
				    (SET_WORD(info->sw_define_h, AX_TXD_SW_DEFINE_H)) |
				    (SET_WORD(info->sw_seq, AX_TXD_SEQ)));

	return MACSUCCESS;
}

static u32 txdes_proc_mgnt_8730e(struct mac_ax_adapter *adapter,
				 struct rtw_t_meta_data *info, u8 *buf, u32 len)
{
	struct tx_desc_t *txdesc;
	u32 ret;
	u8 qsel = RTW_TXDESC_QSEL_MGT, sec_type;

	if (len != mac_txdesc_len_8730e(adapter, info)) {
		PLTFM_MSG_ERR("[ERR] illegal len %d\n", len);
		return MACBUFSZ;
	}

	if (info->pktlen > AX_TXD_TXPKTSIZE_MSK || !info->pktlen) {
		PLTFM_MSG_ERR("[ERR] illegal txpktsize %d\n", info->pktlen);
		return MACFUNCINPUT;
	}

	txdesc = (struct tx_desc_t *)buf;

	txdesc->txdw0 = cpu_to_le32(SET_WORD(info->pktlen, AX_TXD_TXPKTSIZE) |
				    SET_WORD(info->offset, AX_TXD_OFFSET) |
				    (info->bc || info->mc ? AX_TXD_BMC : 0) |
				    (info->bcnpkt_tsf_c ? AX_TXD_BCNPKT_TSF_C : 0) |
				    (info->no_acm ? AX_TXD_NO_ACM : 0) |
				    (info->gf ? AX_TXD_GF : 0));
	if (info->sec_hw_enc) {
		switch (info->sec_type) {
		case RTW_ENC_WEP40:
		case RTW_ENC_WEP104:
		case RTW_ENC_TKIP:
			sec_type = 0x1;
			break;
		case RTW_ENC_WAPI:
		case RTW_ENC_GCMP:
		case RTW_ENC_GCMP256:
		case RTW_ENC_GCMSMS4:
		case RTW_ENC_CCMP256:
			sec_type = 0x2;
			break;
		case RTW_ENC_CCMP:
			sec_type = 0x3;
			break;
		case HAL_SECURITY_TYPE_BIP:
			if (1 == info->bc) {
				sec_type = 0x0;
			} else {
				sec_type = 0x3;
			}
			break;
		default:
			sec_type = 0x0;
			break;

		}
	}

	txdesc->txdw1 = cpu_to_le32(SET_WORD(info->macid, AX_TXD_MACID) |
				    (info->bip_keyid_sel ? AX_TXD_BIP_KEYID_SEL : 0) |
				    SET_WORD(qsel, AX_TXD_QSEL) |
				    (info->force_txop ? AX_TXD_FORCE_TXOP : 0) |
				    (info->lsig_txop_en ? AX_TXD_LSIG_TXOP_EN : 0) |
				    (info->pifs ? AX_TXD_PIFS : 0) |
				    (info->a_ctrl_cas ? AX_TXD_A_CTRL_CAS : 0) |
				    (info->a_ctrl_bsr ? AX_TXD_A_CTRL_BSR : 0) |
				    (info->a_ctrl_uph ? AX_TXD_A_CTRL_UPH : 0) |
				    (info->a_ctrl_bqr ? AX_TXD_A_CTRL_BQR : 0) |
				    (info->htc ? AX_TXD_HTC : 0) |
				    (info->en_desc_id ? AX_TXD_EN_DESC_ID : 0) |
				    SET_WORD(sec_type, AX_TXD_SEC_TYPE) |
				    SET_WORD(info->pkt_offset, AX_TXD_PKT_OFFSET) |
				    (info->moredata ? AX_TXD_MOREDATA : 0) |
				    (info->ftm_en ? AX_TXD_FTM_EN : 0) |
				    (info->hw_aesiv ? AX_TXD_HW_AESIV : 0));
	txdesc->txdw2 = cpu_to_le32(SET_WORD(info->p_aid, AX_TXD_P_AID) |
				    (info->tri_frame ? AX_TXD_TRI_FRAME : 0) |
				    (SET_WORD(info->rts_cca_mode, AX_TXD_CCA_RTS)) |
				    (info->ampdu_en ? AX_TXD_AGG_EN : 0) |
				    (info->ack_ch_info ? AX_TXD_ACK_CH_INFO : 0) |
				    (info->null0 ? AX_TXD_NULL_0 : 0) |
				    (info->null1 ? AX_TXD_NULL_1 : 0) |
				    (info->bk ? AX_TXD_BK : 0) |
				    (info->tm_en ? AX_TXD_TM_EN : 0) |
				    (info->raw ? AX_TXD_RAW : 0) |
				    (info->spe_rpt ? AX_TXD_CCX : 0) |
				    (SET_WORD(info->ampdu_density, AX_TXD_AMPDU_DENSITY)) |
				    (info->bt_int ? AX_TXD_BT_INT : 0) |
				    (SET_WORD(info->spe_frame_indicate, AX_TXD_SPE_FRAME_INDICATE)) |
				    (info->mu_snd_flag ? AX_TXD_MU_SND_FLAG : 0));

	txdesc->txdw3 = cpu_to_le32(SET_WORD(info->wheader_len, AX_TXD_WHEADER_LEN) |
				    (info->txdesc_info_en ? AX_TXD_TXDESC_INFO_EN : 0) |
				    (SET_WORD(info->hw_ssn_sel, AX_TXD_HW_SSN_SEL)) |
				    (info->userate_sel ? AX_TXD_USEDESRATE : 0) |
				    (info->dis_rts_rate_fb ? AX_TXD_DISRTSFB : 0) |
				    (info->dis_data_rate_fb ? AX_TXD_DISDATAFB : 0) |
				    (info->cts2self ? AX_TXD_CTS_2_SELF : 0) |
				    (info->rts_en ? AX_TXD_RTS_EN : 0) |
				    (info->hw_rts_en ? AX_TXD_HW_RTS_EN : 0) |
				    (info->chksum_en ? AX_TXD_CHKSUM_EN : 0) |
				    (info->nav_use_hdr ? AX_TXD_NAVUSEHDR : 0) |
				    (info->use_max_time ? AX_TXD_USE_MAX_TIME : 0) |
				    (SET_WORD(info->max_agg_num, AX_TXD_MAX_AGG_NUM)) |
				    (SET_WORD(info->ndpa, AX_TXD_NDPA)) |
				    (SET_WORD(info->ampdu_max_time, AX_TXD_AMPDU_MAX_TIME)));

	txdesc->txdw4 = cpu_to_le32(SET_WORD(info->f_rate, AX_TXD_DATARATE) |
				    (SET_WORD(info->data_rty_lowest_rate, AX_TXD_DATA_RTY_LOWEST_RATE)) |
				    (SET_WORD(info->retry_limit, AX_TXD_RETRY_LIMIT)) |
				    (SET_WORD(info->obw_cts2self_dup_type, AX_TXD_OBW_CTS2SELF_DUP_TYPE)) |
				    (info->try_rate ? AX_TXD_TRY_RATE : 0) |
				    (SET_WORD(info->f_gi_ltf, AX_TXD_GI_LTF)));

	txdesc->txdw5 = cpu_to_le32(SET_WORD(info->data_sc, AX_TXD_DATA_SC) |
				    (info->f_dcm ? AX_TXD_DATA_DCM : 0) |
				    (SET_WORD(info->f_bw, AX_TXD_DATA_BW)) |
				    (info->f_ldpc ? AX_TXD_DATA_LDPC : 0) |
				    (SET_WORD(info->f_stbc, AX_TXD_DATA_STBC)) |
				    (SET_WORD(info->rts_stbc, AX_TXD_RTS_STBC)) |
				    (info->rts_short ? AX_TXD_RTS_SHORT : 0) |
				    (SET_WORD(info->rts_sc, AX_TXD_RTS_SC)) |
				    (info->signaling_ta ? AX_TXD_SIGNALING_TA : 0) |
				    (info->data_bw_er ? AX_TXD_DATA_BW_ER : 0) |
				    (info->f_er ? AX_TXD_DATA_ER : 0) |
				    (SET_WORD(info->mbssid, AX_TXD_MBSSID)) |
				    (info->port_id ? AX_TXD_PORT_ID : 0) |
				    (info->polluted ? AX_TXD_POLLUTED : 0));

	txdesc->txdw6 = cpu_to_le32((SET_WORD(info->rtsrate, AX_TXD_RTSRATE)) |
				    (SET_WORD(info->data_rty_lowest_rate, AX_TXD_RTS_RTY_LOWEST_RATE)) |
				    (info->rty_use_des ? AX_TXD_RTY_USE_DES : 0));



	txdesc->txdw7 = cpu_to_le32((SET_WORD(0, AX_TXD_CHECK_SUM)) |
				    (SET_WORD(info->sw_define_l, AX_TXD_SW_DEFINE_L)) |
				    (SET_WORD(info->final_data_rate, AX_TXD_FINAL_DATA_RATE)));

	if (adapter->hw_info->checksum_en) {
		ret = mac_txdesc_checksum_8730e(adapter, info, buf);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] mac_wd_checksum %d\n", ret);
			return ret;
		}
	}

	txdesc->txdw8 = cpu_to_le32((SET_WORD(info->rts_rc, AX_TXD_RTS_RC)) |
				    (SET_WORD(info->bar_rc, AX_TXD_BAR_RC)) |
				    (SET_WORD(info->data_rc, AX_TXD_DATA_RC)) |
				    (info->tx_vendo ? AX_TXD_TX_VENDO : 0) |
				    (info->hw_seq_mode ? AX_TXD_EN_HWSEQ : 0));

	txdesc->txdw9 = cpu_to_le32((SET_WORD(info->group_bit_ie_offset, AX_TXD_GROUP_BIT_IE_OFFSET)) |
				    (SET_WORD(info->sw_define_h, AX_TXD_SW_DEFINE_H)) |
				    (SET_WORD(info->sw_seq, AX_TXD_SEQ)));

	return MACSUCCESS;
}

static struct txd_proc_type txdes_proc_mac_8730e[] = {
//8730e no PKT_TYPE_H2C & PKT_TYPE_FWDL
	{RTW_PHL_PKT_TYPE_DATA, txdes_proc_data_8730e},
	{RTW_PHL_PKT_TYPE_MGNT, txdes_proc_mgnt_8730e},
	{RTW_PHL_PKT_TYPE_MAX, NULL},
};

u32 mac_build_txdesc_8730e(struct mac_ax_adapter *adapter,
			   struct rtw_t_meta_data *info, u8 *buf, u32 len)
{
	struct txd_proc_type *proc = txdes_proc_mac_8730e;
	enum rtw_packet_type pkt_type = info->type;
	u32(*handler)(struct mac_ax_adapter * adapter,
		      struct rtw_t_meta_data * info, u8 * buf, u32 len) = NULL;

	for (; proc->type != RTW_PHL_PKT_TYPE_MAX; proc++) {
		if (pkt_type == proc->type) {
			handler = proc->handler;
			break;
		}
	}

	if (!handler) {
		PLTFM_MSG_ERR("[ERR]null type handler type: %X\n", proc->type);
		return MACNOITEM;
	}

	return handler(adapter, info, buf, len);
}
/*
u32 mac_refill_txdesc_8730e(struct mac_ax_adapter *adapter,
			    struct rtw_t_meta_data *txpkt_info,
			    struct mac_ax_refill_info *mask,
			    struct mac_ax_refill_info *info)
{
	u32 dw0 = ((struct wd_body_t *)info->pkt)->dword0;
	u32 dw1 = ((struct wd_body_t *)info->pkt)->dword1;
	u32 ret;

	if (mask->packet_offset)
		((struct wd_body_t *)info->pkt)->dword0 =
			dw0 | (info->packet_offset ? AX_TXD_PKT_OFFSET : 0);

	if (mask->agg_num == AX_TXD_DMA_TXAGG_NUM_MSK)
		((struct wd_body_t *)info->pkt)->dword1 =
			SET_CLR_WORD(dw1, info->agg_num, AX_TXD_DMA_TXAGG_NUM);

	if (adapter->hw_info->wd_checksum_en) {
		ret = mac_wd_checksum_8852a(adapter, txpkt_info, info->pkt);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR] mac_wd_checksum %d\n", ret);
			return ret;
		}
	}

	return MACSUCCESS;
}
*/
static u32 rxdes_parse_comm_8730e(struct mac_ax_adapter *adapter,
				  struct mac_ax_rxpkt_info *info, u8 *buf)
{
	u32 hdr_val = le32_to_cpu(((struct rx_desc_t *)buf)->rxdw0);

	info->rxdlen = RX_DESC_LEN;
	info->pktsize = GET_FIELD(hdr_val, AX_RXD_PKT_LEN);
	info->shift = (u8)GET_FIELD(hdr_val, AX_RXD_MH_SHIFT);

	return MACSUCCESS;
}

static u32 rxdes_parse_wifi_8730e(struct mac_ax_adapter *adapter,
				  struct mac_ax_rxpkt_info *info, u8 *buf, u32 len)
{
	u32 hdr_val;

	info->type = MAC_AX_PKT_DATA;

	hdr_val = le32_to_cpu(((struct rxd_short_t *)buf)->dword3);
	info->u.data.crc_err = !!(hdr_val & AX_RXD_CRC32_ERROR);
	info->u.data.icv_err = !!(hdr_val & AX_RXD_ICV_ERROR);

	return MACSUCCESS;
}

/*
static u32 rxdes_parse_c2h_8730e(struct mac_ax_adapter *adapter,
				 struct mac_ax_rxpkt_info *info, u8 *buf, u32 len)
{
	info->type = MAC_AX_PKT_C2H;

	return MACSUCCESS;
}
*/
static u32 rxdes_parse_ch_info_8730e(struct mac_ax_adapter *adapter,
				     struct mac_ax_rxpkt_info *info, u8 *buf, u32 len)
{
	info->type = MAC_AX_PKT_CH_INFO;

	return MACSUCCESS;
}

static u32 rxdes_parse_physts_8730e(struct mac_ax_adapter *adapter,
				    struct mac_ax_rxpkt_info *info, u8 *buf, u32 len)
{
	u32 hdr_val = le32_to_cpu(((struct rxd_short_t *)buf)->dword0);

	info->type = MAC_AX_PKT_PHYSTS;
//	info->u.ppdu.mac_info = !!(hdr_val & AX_RXD_MAC_INFO_VLD);

	return MACSUCCESS;
}

static struct rxd_parse_type rxdes_parse_mac_8730e[] = {
	{RXD_S_RPKT_TYPE_WIFI, rxdes_parse_wifi_8730e},
	{RXD_S_RPKT_TYPE_CH_INFO_NORMAL, rxdes_parse_ch_info_8730e},
	{RXD_S_RPKT_TYPE_CH_INFO_RESP, rxdes_parse_ch_info_8730e},
	{RXD_S_RPKT_TYPE_PHYSTS, rxdes_parse_physts_8730e},
	{RXD_RPKT_TYPE_INVALID, NULL},
};

u32 mac_parse_rxdesc_8730e(struct mac_ax_adapter *adapter,
			   struct mac_ax_rxpkt_info *info, u8 *buf, u32 len)
{
	struct rxd_parse_type *parse = rxdes_parse_mac_8730e;
	u8 rpkt_type;
	u32 hdr_val;
	u32(*handler)(struct mac_ax_adapter * adapter,
		      struct mac_ax_rxpkt_info * info, u8 * buf, u32 len) = NULL;

	hdr_val = le32_to_cpu(((struct rx_desc_t *)buf)->rxdw1);
	rpkt_type = (u8)GET_FIELD(hdr_val, AX_RXD_PKT_INDICATE);

	rxdes_parse_comm_8730e(adapter, info, buf);

	hdr_val = le32_to_cpu(((struct rx_desc_t *)buf)->rxdw2);
	/*rx desc offset8[28] C2H*/
	if (hdr_val & AX_RXD_C2H) {
		info->type = MAC_AX_PKT_C2H;
		return MACSUCCESS;
	}
	/*rx desc offset4[18:16] PKT_INDICATE*/
	else {
		for (; parse->type != RXD_RPKT_TYPE_INVALID; parse++) {
			if (rpkt_type == parse->type) {
				handler = parse->handler;
				break;
			}
		}

		if (!handler) {
			PLTFM_MSG_ERR("[ERR]null type handler type: %X\n", parse->type);
			return MACNOITEM;
		}
		return handler(adapter, info, buf, len);
	}
}

u32 mac_txdesc_checksum_8730e(struct mac_ax_adapter *adapter,
			      struct rtw_t_meta_data *info, u8 *txdesc)
{
	u16 chksum = 0;
	u32 txdesc_size;
	u16 *data;
	u32 i, txdw7;

	if (!txdesc) {
		PLTFM_MSG_ERR("[ERR]null pointer\n");
		return MACNPTR;
	}

	if (adapter->hw_info->checksum_en != 1) {
		PLTFM_MSG_TRACE("[TRACE]chksum disable\n");
	}

	txdw7 = ((struct tx_desc_t *)txdesc)->txdw7;

	((struct tx_desc_t *)txdesc)->txdw7 =
		SET_CLR_WORD(txdw7, 0x0, AX_TXD_CHECK_SUM);

	data = (u16 *)(txdesc);
	/*unit : 4 bytes*/
	txdesc_size = mac_txdesc_len_8730e(adapter, info) >> 2;
	for (i = 0; i < (txdesc_size - 2); i++) {
		chksum ^= (*(data + 2 * i) ^ * (data + (2 * i + 1)));
	}
	chksum ^= 0xffff;
	/* *(data + 2 * i) & *(data + (2 * i + 1) have endain issue*/
	/* Process eniadn issue after checksum calculation */
	((struct tx_desc_t *)txdesc)->txdw7 =
		SET_CLR_WORD(txdw7, (u16)(chksum), AX_TXD_CHECK_SUM);
	return MACSUCCESS;
}
/*
u32 mac_patch_rx_rate_8730e(struct mac_ax_adapter *adapter,
			    struct rtw_r_meta_data *info)
{
	u32 nss;

	if (is_cv(adapter, CBV)) {
		if (info->rpkt_type != RX_8852A_DESC_PKT_T_WIFI ||
		    info->ppdu_type != RX_8852A_DESC_PPDU_T_HE_SU ||
		    info->rx_gi_ltf != RX_8852A_DESC_RX_GI_LTF_4X_0_8)
			return MACSUCCESS;

		nss = GET_NSS_FROM_RX_RATE(info->rx_rate);

		if (nss == NSS_1) // real nss = 2
			info->rx_rate = SET_NSS_TO_RX_RATE(info->rx_rate, NSS_2);
		else if (nss == NSS_8) // real nss = 1
			info->rx_rate = SET_NSS_TO_RX_RATE(info->rx_rate, NSS_1);
	}

	return MACSUCCESS;
}

u32 mac_get_wp_offset_8730e(struct mac_ax_adapter *adapter,
			    struct mac_txd_ofld_wp_offset *ofld_conf, u16 *val)
{
	u16 ret_val = 0;

	if (ofld_conf->hw_amsdu_type == MAC_TXD_OFLD_HW_AMSDU_CONF_MISSING) {
		PLTFM_MSG_ERR("missing configurations: HW AMSDU type\n");
		return MACFUNCINPUT;
	}
	if (ofld_conf->hw_hdr_conv_type == MAC_TXD_OFLD_HW_HDR_CONV_CONF_MISSING) {
		PLTFM_MSG_ERR("missing configurations: HW HDR CONV type\n");
		return MACFUNCINPUT;
	}

	if (ofld_conf->hw_hdr_conv_type == MAC_TXD_OFLD_HW_HDR_CONV_ETHII_TO_WLAN)
		ret_val += (HDR_SIZE_WLAN_MAX + HDR_SIZE_LLC - HDR_SIZE_802P3);
	else if (ofld_conf->hw_hdr_conv_type == MAC_TXD_OFLD_HW_HDR_CONV_SNAP_TO_WLAN)
		ret_val += (HDR_SIZE_WLAN_MAX - HDR_SIZE_802P3);

	if (ofld_conf->hw_amsdu_type == MAC_TXD_OFLD_HW_AMSDU_ON)
		ret_val += HDR_SIZE_AMSDU;

	*val = (ret_val / WP_OFFSET_UNIT_8852A);

	return MACSUCCESS;
}*/

#endif /* #if MAC_AX_8730E_SUPPORT */
