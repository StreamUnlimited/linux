/** @file */
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

#ifndef _MAC_AX_RXDESC_H_
#define _MAC_AX_RXDESC_H_
#if MAC_AX_8730E_SUPPORT
/* dword0 */   /* offset0 */
#define AX_RXD_INTER_PPDU		BIT(31)
#define AX_RXD_RX_BW_SH		29
#define AX_RXD_RX_BW_MSK		0x3
#define AX_RXD_HE_MU_RU_SIZE_SH		26
#define AX_RXD_HE_MU_RU_SIZE_MSK		0x7
#define AX_RXD_SWDEC		BIT(25)
#define AX_RXD_MH_SHIFT_SH		23
#define AX_RXD_MH_SHIFT_MSK		0x3
#define AX_RXD_SECURITY_TYPE_SH		20
#define AX_RXD_SECURITY_TYPE_MSK		0x7
#define AX_RXD_EOSP		BIT(19)
#define AX_RXD_SNIFFER_LEN_SH		16
#define AX_RXD_SNIFFER_LEN_MSK		0x7
#define AX_RXD_ICV_ERROR		BIT(15)  //ICV/MIC ERR
#define AX_RXD_CRC32_ERROR		BIT(14)
#define AX_RXD_PKT_LEN_SH		0
#define AX_RXD_PKT_LEN_MSK		0x3fff

/* dword1 */   /* offset4 */
#define AX_RXD_BC		BIT(31)
#define AX_RXD_MC		BIT(30)
#define AX_RXD_FRAME_TYPE_SH		28
#define AX_RXD_FRMAE_TYPE_MSK		0x3
#define AX_RXD_MF		BIT(27)
#define AX_RXD_MD		BIT(26)
#define AX_RXD_PWR		BIT(25)
#define AX_RXD_PAM		BIT(24)
#define AX_RXD_CHKSUM_EN		BIT(23)
#define AX_RXD_RX_IS_TCP_UDP		BIT(22)
#define AX_RXD_RX_IS_IPV4_IPV6		BIT(21)
#define AX_RXD_CHKSUMERR		BIT(20)
#define AX_RXD_R_SENT_RXCMD		BIT(19)
#define AX_RXD_PKT_INDICATE_SH		16
#define AX_RXD_PKT_INDICATE_MSK		0x7
#define AX_RXD_AMPDU		BIT(15)
#define AX_RXD_RXID_MATCH		BIT(14)
#define AX_RXD_AMSDU		BIT(13)
#define AX_RXD_KEY_INDEX		BIT(12)  //KEY_INDEX/MACID_VLD
#define AX_RXD_TID_SH		8
#define AX_RXD_TID_MSK		0xf
#define AX_RXD_EXT_SECTYPE		BIT(7)
#define AX_RXD_KEY_INDEX_SH		0  //KEY_INDEX/MACID
#define AX_RXD_KEY_INDEX_MSK		0x7f

/* dword2 */   /* offset8 */
#define AX_RXD_FCS_OK		BIT(31)  //FCS_OK & PAM
#define AX_RXD_PPDU_FLUSH_CNT_SH		29
#define AX_RXD_PPDU_FLUSH_CNT_MSK		0x3
#define AX_RXD_C2H		BIT(28)
#define AX_RXD_GI_LTF_SH		25
#define AX_RXD_GI_LTF_MSK		0x7
#define AX_RXD_MAGIC_PKT		BIT(24)
#define AX_RXD_WLANHD_IV_LEN_SH		18
#define AX_RXD_WLANHD_IV_LEN_MSK		0x3f
#define AX_RXD_IFS_CNT0_PHY		BIT(17)
#define AX_RXD_RX_IS_QOS		BIT(16)
#define AX_RXD_FRAGMENT_NUMBER_SH		12
#define AX_RXD_FRAGMENT_NUMBER_MSK		0xf
#define AX_RXD_SEQ_SH		0
#define AX_RXD_SEQ_MSK		0xfff

/* dword3 */   /* offset12 */
#define AX_RXD_MAGIC_WAKE		BIT(31)
#define AX_RXD_UNICAST_WAKE		BIT(30)
#define AX_RXD_PATTERN_MATCH		BIT(29)
#define AX_RXD_RX_CENTRAL_CH_SH		21
#define AX_RXD_RX_CENTRAL_CH_MSK		0xff
#define AX_RXD_RX_PWR_LV_SH		12
#define AX_RXD_RX_PWR_LV_MSK		0x1ff
#define AX_RXD_BSSID_FIT_SH		10
#define AX_RXD_BSSID_FIT_MSK		0x3
#define AX_RXD_HTC		BIT(9)
#define AX_RXD_RX_RATE_SH		0
#define AX_RXD_RX_RATE_MSK		0x1ff

/* dword0 */   /* offset16 */
#define AX_RXD_FLAG_VENDOR_CHECK		BIT(31)
#define AX_RXD_RX_SUP_ADDR3_FIT		BIT(30)
#define AX_RXD_TXRPTMID_CTL_MASK		BIT(29)
#define AX_RXD_MBSSID_FIT_INDEX_SH		24
#define AX_RXD_MBSSID_FIT_INDEX_MSK		0x1f
#define AX_RXD_TXRPTMID_SRCH_SH		24
#define AX_RXD_TXRPTMID_SRCH_MSK		0x1f
#define AX_RXD_RX_PRE_NDP_OK		BIT(16)
#define AX_RXD_RX_SCRAMBLER_SH		9
#define AX_RXD_RX_SCRAMBLER_MSK		0x7f
#define AX_RXD_RX_EOF		BIT(8)
#define AX_RXD_PATTERN_MATCH_INDEX_SH		0
#define AX_RXD_PATTERN_MATCH_INDEX_MSK		0xff

/* dword0 */   /* offset20 */
#define AX_RXD_TSFL_SH		0
#define AX_RXD_TSFL_MSK		0xffffffff

#define RXD_S_RPKT_TYPE_WIFI		0
#define RXD_S_RPKT_TYPE_CH_INFO_NORMAL		1
#define RXD_S_RPKT_TYPE_CH_INFO_RESP		2
#define RXD_S_RPKT_TYPE_C2H		3
#define RXD_S_RPKT_TYPE_PHYSTS	4
#endif
#endif

