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
#ifndef _HAL_TRX_8730E_H_
#define _HAL_TRX_8730E_H_

#define RX_DESC_SIZE_8730E 24

#define RX_BD_INFO_SIZE 8

#define RX_PPDU_MAC_INFO_SIZE_8730E 4

#define ACH0_QUEUE_IDX_8730E 0x0
#define ACH1_QUEUE_IDX_8730E 0x1
#define ACH2_QUEUE_IDX_8730E 0x2
#define ACH3_QUEUE_IDX_8730E 0x3
#define ACH4_QUEUE_IDX_8730E 0x4
#define ACH5_QUEUE_IDX_8730E 0x5
#define ACH6_QUEUE_IDX_8730E 0x6
#define ACH7_QUEUE_IDX_8730E 0x7
#define MGQ_B0_QUEUE_IDX_8730E 0x8
#define HIQ_B0_QUEUE_IDX_8730E 0x9
#define MGQ_B1_QUEUE_IDX_8730E 0xa
#define HIQ_B1_QUEUE_IDX_8730E 0xb
#define FWCMD_QUEUE_IDX_8730E 0xc

/* AX RX DESC */
/* DWORD 0 ; Offset 00h */
#define GET_RX_AX_DESC_PKT_LEN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 0, 14)
#define GET_RX_AX_DESC_CRC32_ERR_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 14, 1)
#define GET_RX_AX_DESC_ICV_MIC_ERR_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 15, 1)
#define GET_RX_AX_DESC_SNIFFER_LEN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 16, 3)
#define GET_RX_AX_DESC_EOSP_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 19, 1)
#define GET_RX_AX_DESC_SEC_TYPE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 20, 3)
#define GET_RX_AX_DESC_MH_SHIFT_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE( __pRxStatusDesc, 23, 2)
#define GET_RX_AX_DESC_SWDEC_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc, 25, 1)
#define GET_RX_AX_DESC_HE_MU_RU_SIZE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE( __pRxStatusDesc, 26, 3)
#define GET_RX_AX_DESC_BW_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE( __pRxStatusDesc, 29, 2)
#define GET_RX_AX_DESC_INTER_PPDU_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE( __pRxStatusDesc, 31, 1)

/* DWORD 1 ; Offset 04h */
#define GET_RX_AX_DESC_KEY_INDEX_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 0, 7)
#define GET_RX_AX_DESC_MACID_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 0, 7)
#define GET_RX_AX_DESC_EXT_SECTYPE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 7, 1)
#define GET_RX_AX_DESC_TID_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 8, 4)
#define GET_RX_AX_DESC_MACID_VLD_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 12, 1)
#define GET_RX_AX_DESC_AMSDU_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 13, 1)
#define GET_RX_AX_DESC_RXID_MATCH_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 14, 1)
#define GET_RX_AX_DESC_AMPDU_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 15, 1)
#define GET_RX_AX_DESC_PKT_INDICATE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 16, 3)
#define GET_RX_AX_DESC_R_SENT_RXCMD_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 19, 1)
#define GET_RX_AX_DESC_CHKSUMERR_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 20, 1)
#define GET_RX_AX_DESC_RX_IS_IPV4_IPV6_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 21, 1)
#define GET_RX_AX_DESC_RX_IS_TCP_UDP_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 22, 1)
#define GET_RX_AX_DESC_CHKSUM_EN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 23, 1)
#define GET_RX_AX_DESC_PAM_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 24, 1)
#define GET_RX_AX_DESC_PWR_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 25, 1)
#define GET_RX_AX_DESC_MD_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 26, 1)
#define GET_RX_AX_DESC_MF_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 27, 1)
#define GET_RX_AX_DESC_FRAME_TYPE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 28, 2)
#define GET_RX_AX_DESC_MC_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 30, 1)
#define GET_RX_AX_DESC_BC_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+4, 31, 1)

/* DWORD 2 ; Offset 08h */
#define GET_RX_AX_DESC_SEQ_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 0, 12)
#define GET_RX_AX_DESC_FRAG_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 12, 4)
#define GET_RX_AX_DESC_QOS_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 16, 1)
#define GET_RX_AX_DESC_STATISTICS_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 17, 1)
#define GET_RX_AX_DESC_WLANHD_IV_LEN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 18, 6)
#define GET_RX_AX_DESC_MAGIC_PKT_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 24, 1)
#define GET_RX_AX_DESC_GI_LTF_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 25, 3)
#define GET_RX_AX_DESC_C2H_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 28, 1)
#define GET_RX_AX_DESC_PPDU_FLUSH_CNT_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 29, 2)
#define GET_RX_AX_DESC_FCS_OK_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+8, 31, 1)

/* DWORD 3 ; Offset 12h */
#define GET_RX_AX_DESC_RX_RATE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 0, 9)
#define GET_RX_AX_DESC_HTC_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 9, 1)
#define GET_RX_AX_DESC_BSSID_FIT_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 10, 2)
/*[28:12] start, different for normal pkt, channel info pkt and ppdu status pkt*/
/*normal pkt:*/
#define GET_RX_AX_DESC_RX_PWR_LV_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 12, 9)
#define GET_RX_AX_DESC_RX_CENTRAL_CH_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 21, 8)
/*channel info pkt:*/
#define GET_RX_AX_DESC_CH_INFO_CSI_LEN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 16, 9)
#define GET_RX_AX_DESC_CH_INFO_BW_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 25, 2)
/*ppdu status pkt:*/
#define GET_RX_AX_DESC_PHYSTS_LEN_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 16, 8)
#define GET_RX_AX_DESC_PHYSTS_OF_DATA_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 24, 1)
/*[28:12] end*/
#define GET_RX_AX_DESC_PATTERN_MATCH_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 29, 1)
#define GET_RX_AX_DESC_UNICAST_WAKE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 30, 1)
#define GET_RX_AX_DESC_MAGIC_WAKE_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+12, 31, 1)

/* DWORD 4 ; Offset 16h */
#define GET_RX_AX_DESC_PATTERN_IDX_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 0, 8)
#define GET_RX_AX_DESC_RX_EOF_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 8, 1)
#define GET_RX_AX_DESC_RX_SCRAMBLER_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 9, 7)
#define GET_RX_AX_DESC_RX_PRE_NDP_OK_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 16, 1)
#define GET_RX_AX_DESC_TXRPTMID_SRCH_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 17, 7)
#define GET_RX_AX_DESC_MBSSID_FIT_INDEX_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 24, 5)
#define GET_RX_AX_DESC_TXRPTMID_CTL_MASK_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 29, 1)
#define GET_RX_AX_DESC_RX_SUP_A3_FIT_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 30, 1)
#define GET_RX_AX_DESC_FLAG_VENOR_CHECK_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+16, 31, 1)

/* DWORD 5 ; Offset 20h */
#define GET_RX_AX_DESC_TSFL_8730E(__pRxStatusDesc) LE_BITS_TO_4BYTE(__pRxStatusDesc+20, 0, 32)


/*
0000: normal rx packet
0001: normal rx/NDPA trigger channel info
0010: response trigger channel info
0011: ppdu status (phy status)
0100: C2H packet
 */
#define RX_8730E_DESC_PKT_T_WIFI 0
#define RX_8730E_DESC_PKT_T_WIFI_NDPA_CH_INFO 1
#define RX_8730E_DESC_PKT_T_RESP_CH_INFO 2
#define RX_8730E_DESC_PKT_T_PPDU_STATUS 4
#define RX_8730E_DESC_PKT_T_C2H 3


struct rx_ppdu_status {
	u32 mac_info_length;
	u32 phy_info_length;
	//struct mac_info macinfo;
	//struct phy_info phyinfo;
};

enum rtw_hal_status
hal_handle_rx_buffer_8730e(struct rtw_phl_com_t *phl_com,
			   struct hal_info_t *hal,
			   u8 *buf, u32 buf_len,
			   struct rtw_phl_rx_pkt *phl_rx);

#endif /*_HAL_TRX_8730E_H_*/
