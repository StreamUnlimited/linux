/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#ifndef __RTL8730A_XMIT_H__
#define __RTL8730A_XMIT_H__

#define MAX_TID (15)


#ifndef __INC_HAL8730ADESC_H
#define __INC_HAL8730ADESC_H

#define RX_STATUS_DESC_SIZE_8730A		24
#define RX_DRV_INFO_SIZE_UNIT_8730A 8

#define SET_EARLYMODE_PKTNUM_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr, 0, 4, __Value)
#define SET_EARLYMODE_LEN0_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr, 4, 15, __Value)
#define SET_EARLYMODE_LEN1_1_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr, 19, 13, __Value)
#define SET_EARLYMODE_LEN1_2_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr+4, 0, 2, __Value)
#define SET_EARLYMODE_LEN2_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr+4, 2, 15,	__Value)
#define SET_EARLYMODE_LEN3_8730A(__pAddr, __Value)					SET_BITS_TO_LE_4BYTE(__pAddr+4, 17, 15, __Value)


/*-----------------------------------------------------------------*/
/*	RTL8730A TX BUFFER DESC                                      */
/*-----------------------------------------------------------------*/
#ifdef CONFIG_64BIT_DMA
#define SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+((__Offset)*16), 0, 16, __Valeu)
#define SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+((__Offset)*16), 31, 1, __Valeu)
#define SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+((__Offset)*16)+4, 0, 32, __Valeu)
#define SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+((__Offset)*16)+8, 0, 32, __Valeu)
#else
#define SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset*8), 0, 16, __Valeu)
#define SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset*8), 31, 1, __Valeu)
#define SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, __Offset, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset*8)+4, 0, 32, __Valeu)
#define SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, __Offset, __Valeu)	/* 64 BIT mode only */
#endif
/* ********************************************************* */

/* 64 bits  -- 32 bits */
/* =======     ======= */
/* Dword 0     0 */
#define SET_TX_BUFF_DESC_LEN_0_8730A(__pTxDesc, __Valeu) SET_BITS_TO_LE_4BYTE(__pTxDesc, 0, 14, __Valeu)
#define SET_TX_BUFF_DESC_PSB_8730A(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc, 16, 15, __Value)
#define SET_TX_BUFF_DESC_OWN_8730A(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc, 31, 1, __Value)

/* Dword 1     1 */
#define SET_TX_BUFF_DESC_ADDR_LOW_0_8730A(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 0, 32, __Value)
#define GET_TX_BUFF_DESC_ADDR_LOW_0_8730A(__pTxDesc) LE_BITS_TO_4BYTE(__pTxDesc+4, 0, 32)
/* Dword 2     NA */
#define SET_TX_BUFF_DESC_ADDR_HIGH_0_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, 0, __Value)
#ifdef CONFIG_64BIT_DMA
#define GET_TX_BUFF_DESC_ADDR_HIGH_0_8730A(__pTxDesc) LE_BITS_TO_4BYTE(__pTxDesc+8, 0, 32)
#else
#define GET_TX_BUFF_DESC_ADDR_HIGH_0_8730A(__pTxDesc) 0
#endif
/* Dword 3     NA */
/* RESERVED 0 */
/* Dword 4     2 */
#define SET_TX_BUFF_DESC_LEN_1_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, 1, __Value)
#define SET_TX_BUFF_DESC_AMSDU_1_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(__pTxDesc, 1, __Value)
/* Dword 5     3 */
#define SET_TX_BUFF_DESC_ADDR_LOW_1_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, 1, __Value)
/* Dword 6     NA */
#define SET_TX_BUFF_DESC_ADDR_HIGH_1_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, 1, __Value)
/* Dword 7     NA */
/*RESERVED 0 */
/* Dword 8     4 */
#define SET_TX_BUFF_DESC_LEN_2_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, 2, __Value)
#define SET_TX_BUFF_DESC_AMSDU_2_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(__pTxDesc, 2, __Value)
/* Dword 9     5 */
#define SET_TX_BUFF_DESC_ADDR_LOW_2_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, 2, __Value)
/* Dword 10    NA */
#define SET_TX_BUFF_DESC_ADDR_HIGH_2_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, 2, __Value)
/* Dword 11    NA */
/*RESERVED 0 */
/* Dword 12    6 */
#define SET_TX_BUFF_DESC_LEN_3_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, 3, __Value)
#define SET_TX_BUFF_DESC_AMSDU_3_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(__pTxDesc, 3, __Value)
/* Dword 13    7 */
#define SET_TX_BUFF_DESC_ADDR_LOW_3_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, 3, __Value)
/* Dword 14    NA */
#define SET_TX_BUFF_DESC_ADDR_HIGH_3_8730A(__pTxDesc, __Value) SET_TXBUFFER_DESC_ADD_HIGT_WITH_OFFSET(__pTxDesc, 3, __Value)
/* Dword 15    NA */
/*RESERVED 0 */


#endif
/* -----------------------------------------------------------
 *
 *	Rate
 *
 * -----------------------------------------------------------
 * CCK Rates, TxHT = 0 */
#define DESC8730A_RATE1M				0x00
#define DESC8730A_RATE2M				0x01
#define DESC8730A_RATE5_5M				0x02
#define DESC8730A_RATE11M				0x03

/* OFDM Rates, TxHT = 0 */
#define DESC8730A_RATE6M				0x04
#define DESC8730A_RATE9M				0x05
#define DESC8730A_RATE12M				0x06
#define DESC8730A_RATE18M				0x07
#define DESC8730A_RATE24M				0x08
#define DESC8730A_RATE36M				0x09
#define DESC8730A_RATE48M				0x0a
#define DESC8730A_RATE54M				0x0b

/* MCS Rates, TxHT = 1 */
#define DESC8730A_RATEMCS0				0x0c
#define DESC8730A_RATEMCS1				0x0d
#define DESC8730A_RATEMCS2				0x0e
#define DESC8730A_RATEMCS3				0x0f
#define DESC8730A_RATEMCS4				0x10
#define DESC8730A_RATEMCS5				0x11
#define DESC8730A_RATEMCS6				0x12
#define DESC8730A_RATEMCS7				0x13
#define DESC8730A_RATEMCS8				0x14
#define DESC8730A_RATEMCS9				0x15
#define DESC8730A_RATEMCS10		0x16
#define DESC8730A_RATEMCS11		0x17
#define DESC8730A_RATEMCS12		0x18
#define DESC8730A_RATEMCS13		0x19
#define DESC8730A_RATEMCS14		0x1a
#define DESC8730A_RATEMCS15		0x1b
#define DESC8730A_RATEVHTSS1MCS0		0x2c
#define DESC8730A_RATEVHTSS1MCS1		0x2d
#define DESC8730A_RATEVHTSS1MCS2		0x2e
#define DESC8730A_RATEVHTSS1MCS3		0x2f
#define DESC8730A_RATEVHTSS1MCS4		0x30
#define DESC8730A_RATEVHTSS1MCS5		0x31
#define DESC8730A_RATEVHTSS1MCS6		0x32
#define DESC8730A_RATEVHTSS1MCS7		0x33
#define DESC8730A_RATEVHTSS1MCS8		0x34
#define DESC8730A_RATEVHTSS1MCS9		0x35
#define DESC8730A_RATEVHTSS2MCS0		0x36
#define DESC8730A_RATEVHTSS2MCS1		0x37
#define DESC8730A_RATEVHTSS2MCS2		0x38
#define DESC8730A_RATEVHTSS2MCS3		0x39
#define DESC8730A_RATEVHTSS2MCS4		0x3a
#define DESC8730A_RATEVHTSS2MCS5		0x3b
#define DESC8730A_RATEVHTSS2MCS6		0x3c
#define DESC8730A_RATEVHTSS2MCS7		0x3d
#define DESC8730A_RATEVHTSS2MCS8		0x3e
#define DESC8730A_RATEVHTSS2MCS9		0x3f


#define	RX_HAL_IS_CCK_RATE_8730A(pDesc)\
	(GET_RX_STATUS_DESC_RX_RATE_8730A(pDesc) == DESC8730A_RATE1M || \
	 GET_RX_STATUS_DESC_RX_RATE_8730A(pDesc) == DESC8730A_RATE2M || \
	 GET_RX_STATUS_DESC_RX_RATE_8730A(pDesc) == DESC8730A_RATE5_5M || \
	 GET_RX_STATUS_DESC_RX_RATE_8730A(pDesc) == DESC8730A_RATE11M)

#ifdef CONFIG_TRX_BD_ARCH
struct tx_desc;
#endif

void rtl8730a_update_txdesc(struct xmit_frame *pxmitframe, u8 *pmem);
void rtl8730a_fill_fake_txdesc(PADAPTER padapter, u8 *pDesc, u32 BufferLen, u8 IsPsPoll, u8 IsBTQosNull, u8 bDataFrame);

void fill_txdesc_force_bmc_camid(struct pkt_attrib *pattrib, u8 *ptxdesc);
void fill_txdesc_bmc_tx_rate(struct pkt_attrib *pattrib, u8 *ptxdesc);

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
s32 rtl8730as_init_xmit_priv(PADAPTER padapter);
void rtl8730as_free_xmit_priv(PADAPTER padapter);
s32 rtl8730as_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe);
s32 rtl8730as_mgnt_xmit(PADAPTER padapter, struct xmit_frame *pmgntframe);
#ifdef CONFIG_RTW_MGMT_QUEUE
s32 rtl8730as_hal_mgmt_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
#endif
s32	rtl8730as_hal_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
s32 rtl8730as_xmit_buf_handler(PADAPTER padapter);
thread_return rtl8730as_xmit_thread(thread_context context);
#define hal_xmit_handler rtl8730as_xmit_buf_handler
#endif

#ifdef CONFIG_USB_HCI
s32 rtl8730au_xmit_buf_handler(PADAPTER padapter);
#define hal_xmit_handler rtl8730au_xmit_buf_handler
s32 rtl8730au_init_xmit_priv(PADAPTER padapter);
void rtl8730au_free_xmit_priv(PADAPTER padapter);
s32 rtl8730au_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe);
s32 rtl8730au_mgnt_xmit(PADAPTER padapter, struct xmit_frame *pmgntframe);
#ifdef CONFIG_RTW_MGMT_QUEUE
s32 rtl8730au_hal_mgmt_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
#endif
s32	 rtl8730au_hal_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
void rtl8730au_xmit_tasklet(unsigned long priv);
s32 rtl8730au_xmitframe_complete(_adapter *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
void _dbg_dump_tx_info(_adapter	*padapter, int frame_tag, struct tx_desc *ptxdesc);
#endif

#ifdef CONFIG_AXI_HCI
s32 rtl8730aa_init_xmit_priv(PADAPTER padapter);
void rtl8730aa_free_xmit_priv(PADAPTER padapter);
struct xmit_buf *rtl8730aa_dequeue_xmitbuf(struct rtw_tx_ring *ring);
void	rtl8730aa_xmitframe_resume(_adapter *padapter);
s32 rtl8730aa_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe);
s32 rtl8730aa_mgnt_xmit(PADAPTER padapter, struct xmit_frame *pmgntframe);
#ifdef CONFIG_RTW_MGMT_QUEUE
s32 rtl8730aa_hal_mgmt_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
#endif
s32	rtl8730aa_hal_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
#endif

u8	BWMapping_8730A(PADAPTER Adapter, struct pkt_attrib *pattrib);
u8	SCMapping_8730A(PADAPTER Adapter, struct pkt_attrib	*pattrib);

#endif
