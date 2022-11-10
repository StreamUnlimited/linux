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

#ifndef _MAC_AX_TXDESC_H_
#define _MAC_AX_TXDESC_H_

#if MAC_AX_8730E_SUPPORT
/* dword0 */   /* offset0 */
#define AX_TXD_GF		BIT(30)
#define AX_TXD_NO_ACM		BIT(29)
#define AX_TXD_BCNPKT_TSF_C		BIT(28)
#define AX_TXD_BMC		BIT(24)
#define AX_TXD_OFFSET_SH		16
#define AX_TXD_OFFSET_MSK		0xff
#define AX_TXD_TXPKTSIZE_SH		0
#define AX_TXD_TXPKTSIZE_MSK		0xffff

/* dword1 */   /* offset4 */
#define AX_TXD_HW_AESIV		BIT(31)
#define AX_TXD_FTM_EN		BIT(30)
#define AX_TXD_MOREDATA		BIT(29)
#define AX_TXD_PKT_OFFSET_SH		24
#define AX_TXD_PKT_OFFSET_MSK		0x1f
#define AX_TXD_SEC_TYPE_SH		22
#define AX_TXD_SEC_TYPE_MSK		0x3
#define AX_TXD_EN_DESC_ID		BIT(21)
#define AX_TXD_HTC		BIT(20)
#define AX_TXD_A_CTRL_BQR		BIT(19)
#define AX_TXD_A_CTRL_UPH		BIT(18)
#define AX_TXD_A_CTRL_BSR		BIT(17)
#define AX_TXD_A_CTRL_CAS		BIT(16)
#define AX_TXD_PIFS		BIT(15)
#define AX_TXD_LSIG_TXOP_EN		BIT(14)
#define AX_TXD_FORCE_TXOP		BIT(13)
#define AX_TXD_QSEL_SH		8
#define AX_TXD_QSEL_MSK		0x1f
#define AX_TXD_BIP_KEYID_SEL		BIT(7)
#define AX_TXD_MACID_SH		7
#define AX_TXD_MACID_MSK		0x7f

/* dword2 */   /* offset8 */
#define AX_TXD_MU_SND_FLAG		BIT(27)
#define AX_TXD_SPE_FRAME_INDICATE_SH		24
#define AX_TXD_SPE_FRAME_INDICATE_MSK		0x7
#define AX_TXD_BT_INT		BIT(23)
#define AX_TXD_AMPDU_DENSITY_SH		20
#define AX_TXD_AMPDU_DENSITY_MSK		0x7
#define AX_TXD_CCX		BIT(19)
#define AX_TXD_RAW		BIT(18)
#define AX_TXD_TM_EN		BIT(17)
#define AX_TXD_BK		BIT(16)
#define AX_TXD_NULL_1		BIT(15)
#define AX_TXD_NULL_0		BIT(14)
#define AX_TXD_ACK_CH_INFO		BIT(13)
#define AX_TXD_AGG_EN		BIT(12)
#define AX_TXD_CCA_RTS_SH		10
#define AX_TXD_CCA_RTS_MSK		0x3
#define AX_TXD_TRI_FRAME		BIT(9)
#define AX_TXD_P_AID_SH		0
#define AX_TXD_P_AID_MSK		0x1ff

/* dword3 */   /* offset12 */
#define AX_TXD_AMPDU_MAX_TIME_SH		24
#define AX_TXD_AMPDU_MAX_TIME_MSK		0xff
#define AX_TXD_NDPA_SH		22
#define AX_TXD_NDPA_MSK		0x3
#define AX_TXD_MAX_AGG_NUM_SH		17
#define AX_TXD_MAX_AGG_NUM_MSK		0x1f
#define AX_TXD_USE_MAX_TIME		BIT(16)
#define AX_TXD_NAVUSEHDR		BIT(15)
#define AX_TXD_CHKSUM_EN		BIT(14)
#define AX_TXD_HW_RTS_EN		BIT(13)
#define AX_TXD_RTS_EN		BIT(12)
#define AX_TXD_CTS_2_SELF		BIT(11)
#define AX_TXD_DISDATAFB		BIT(10)
#define AX_TXD_DISRTSFB		BIT(9)
#define AX_TXD_USEDESRATE		BIT(8)
#define AX_TXD_HW_SSN_SEL_SH		6
#define AX_TXD_HW_SSN_SEL_MSK		0x3
#define AX_TXD_TXDESC_INFO_EN		BIT(5)
#define AX_TXD_WHEADER_LEN_SH		0
#define AX_TXD_WHEADER_LEN_MSK		0x1f

/* dword4 */   /* offset16 */
#define AX_TXD_GI_LTF_SH		29
#define AX_TXD_GI_LTF_MSK		0x7
#define AX_TXD_TRY_RATE		BIT(28)
#define AX_TXD_OBW_CTS2SELF_DUP_TYPE_SH		24
#define AX_TXD_OBW_CTS2SELF_DUP_TYPE_MSK		0xf
#define AX_TXD_RETRY_LIMIT_SH		18
#define AX_TXD_RETRY_LIMIT_MSK		0x3f
#define AX_TXD_DATA_RTY_LOWEST_RATE_SH		9
#define AX_TXD_DATA_RTY_LOWEST_RATE_MSK		0x1ff
#define AX_TXD_DATARATE_SH		0
#define AX_TXD_DATARATE_MSK		0x1ff


/* dword5 */   /* offset20 */
#define AX_TXD_POLLUTED		BIT(31)
#define AX_TXD_PORT_ID		BIT(30)
#define AX_TXD_MBSSID_SH		20
#define AX_TXD_MBSSID_MSK		0xf
#define AX_TXD_DATA_ER		BIT(19)
#define AX_TXD_DATA_BW_ER		BIT(18)
#define AX_TXD_SIGNALING_TA		BIT(17)
#define AX_TXD_RTS_SC_SH		13
#define AX_TXD_RTS_SC_MSK		0xf
#define AX_TXD_RTS_SHORT		BIT(12)
#define AX_TXD_RTS_STBC_SH		10
#define AX_TXD_RTS_STBC_MSK		0x3
#define AX_TXD_DATA_STBC_SH		8
#define AX_TXD_DATA_STBC_MSK		0x3
#define AX_TXD_DATA_LDPC		BIT(7)
#define AX_TXD_DATA_BW_SH		5
#define AX_TXD_DATA_BW_MSK		0x3
#define AX_TXD_DATA_DCM		BIT(4)
#define AX_TXD_DATA_SC_SH		0
#define AX_TXD_DATA_SC_MSK		0xf

/* dword6 */   /* offset24 */
#define AX_TXD_RTY_USE_DES		BIT(19)
#define AX_TXD_RTS_RTY_LOWEST_RATE_SH		9
#define AX_TXD_RTS_RTY_LOWEST_RATE_MSK		0x1ff
#define AX_TXD_RTSRATE_SH		0
#define AX_TXD_RTSRATE_MSK		0x1ff


/* dword7 */   /* offset28 */
#define AX_TXD_FINAL_DATA_RATE_SH		23
#define AX_TXD_FINAL_DATA_RATE_MSK		0x1ff
#define AX_TXD_SW_DEFINE_L_SH		16
#define AX_TXD_SW_DEFINE_L_MSK		0x7f
#define AX_TXD_CHECK_SUM_SH		0
#define AX_TXD_CHECK_SUM_MSK		0xffff

/* dword8 */   /* offset32 */
#define AX_TXD_TAILPAGE_L_SH		24
#define AX_TXD_TAILPAGE_L_MSK		0xff
#define AX_TXD_NEXTHEADPAGE_L_SH		16
#define AX_TXD_NEXTHEADPAGE_L_MSK		0xff
#define AX_TXD_EN_HWSEQ		BIT(15)
#define AX_TXD_TX_VENDO		BIT(14)
#define AX_TXD_DATA_RC_SH		8
#define AX_TXD_DATA_RC_MSK		0x3f
#define AX_TXD_BAR_RC_SH		6
#define AX_TXD_BAR_RC_MSK		0x3
#define AX_TXD_RTS_RC_SH		0
#define AX_TXD_RTS_RC_MSK		0x3f

/* dword9 */   /* offset36 */

#define AX_TXD_TAILPAGE_H_SH		28
#define AX_TXD_TAILPAGE_H_MSK		0xf
#define AX_TXD_NEXTHEADPAGE_H_SH		24
#define AX_TXD_NEXTHEADPAGE_H_MSK		0xf
#define AX_TXD_MU_SND_FLAG		BIT(27)
#define AX_TXD_SEQ_SH		12
#define AX_TXD_SEQ_MSK		0xfff
#define AX_TXD_SW_DEFINE_H_SH		8
#define AX_TXD_SW_DEFINE_H_MSK		0xf
#define AX_TXD_GROUP_BIT_IE_OFFSET_SH		0
#define AX_TXD_GROUP_BIT_IE_OFFSET_MSK		0xff
#endif

#endif
