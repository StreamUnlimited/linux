/******************************************************************************
 *
 * Copyright(c) 2015 - 2017 Realtek Corporation.
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
#ifndef _RTL8730AA_H_
#define _RTL8730AA_H_

#include <drv_types.h>		/* PADAPTER */
#include <rtl8730a_spec.h>

#define TX_BD_NUM_8730AA	32
#define RX_BD_NUM_8730AA	AXI_MAX_RX_COUNT /* TODO */

#ifdef CONFIG_CONCURRENT_MODE
#define TX_BD_NUM_8730AA_BCN	4
#define TX_BD_NUM_8730AA_BEQ	(TX_BD_NUM_8730AA * 2)
#else
#define TX_BD_NUM_8730AA_BCN	2
#define TX_BD_NUM_8730AA_BEQ	TX_BD_NUM_8730AA
#endif /* CONFIG_CONCURRENT_MODE */

#define TX_BD_NUM_8730AA_VIQ	TX_BD_NUM_8730AA
#define TX_BD_NUM_8730AA_VOQ	TX_BD_NUM_8730AA
#define TX_BD_NUM_8730AA_BKQ	TX_BD_NUM_8730AA
#define TX_BD_NUM_8730AA_CMD	4

#define TX_MGQ_DESC_NUM TX_BD_NUM_8730AA
#define TX_H0Q_DESC_NUM TX_BD_NUM_8730AA
#define TX_H1Q_DESC_NUM 2
#define TX_H2Q_DESC_NUM 2
#define TX_H3Q_DESC_NUM 2
#define TX_H4Q_DESC_NUM 2
#define TX_H5Q_DESC_NUM 2
#define TX_H6Q_DESC_NUM 2
#define TX_H7Q_DESC_NUM 2

#define RTL8730AA_SEG_NUM       1 /* 0:2 seg, 1: 4 seg, 2: 8 seg */

#define TX_BUFFER_SEG_NUM	1 /* 0:2 seg, 1: 4 seg, 2: 8 seg. */

#define MAX_RECVBUF_SZ_8730A	24576	/* 24k */


/* RX BD */
#define SET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x04, 0, 32, __Value)
#define SET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x08, 0, 32, __Value)

#define SET_RX_BD_RXBUFFSIZE(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 0, 14, __Value)
#define SET_RX_BD_LS(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 14, 1, __Value)
#define SET_RX_BD_FS(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 15, 1, __Value)
#define SET_RX_BD_RX_TAG(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 16, 13, __Value)

#define GET_RX_BD_OWN(__pRxBd) \
	LE_BITS_TO_4BYTE(__pRxBd + 0x00, 31, 1)
#define GET_RX_BD_TAG(__pRxBd) \
	LE_BITS_TO_4BYTE(__pRxBd + 0x00, 16, 13)


#define SET_TX_BD_LEN_WITH_OFFSET(__pRxBd, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + (__Offset * 8), 0, 16, __Value)
#define SET_TX_BD_AMSDU_WITH_OFFSET(__pRxBd, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + (__Offset * 8), 31, 1, __Value)
#define SET_TX_BD_ADD_LOW_WITH_OFFSET(__pRxBd, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + (__Offset * 8) + 4, 0, 32, __Value)
#define SET_TX_BD_ADD_HIGT_WITH_OFFSET(__pRxBd, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + (__Offset * 16) + 8, 0, 32, __Value)

#define SET_TX_BD_LEN_0(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 0, 16, __Value)

#define SET_TX_BD_ADDR_LOW_0(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x04, 0, 32, __Value)
#define GET_TX_BD_RXBUFFSIZE(__pRxBd, __Value) \
	LE_BITS_TO_4BYTE(__pRxBd + 0x04, 0, 32)

#define SET_TX_BD_ADDR_HIGH_0(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x08, 0, 32, __Value)

#define BIT_HIGHDOK_MSK_8730A	BIT_TXDMAOK_CHANNEL15_MSK_8730A
#define BIT_MGTDOK_MSK_8730A	BIT_TXDMAOK_CHANNEL14_MSK_8730A
#define BIT_VODOK_MSK_8730A	BIT_TXDMAOK_CHANNEL0_MSK_8730A
#define BIT_VIDOK_MSK_8730A	BIT_TXDMAOK_CHANNEL1_MSK_8730A
#define BIT_BEDOK_MSK_8730A	BIT_TXDMAOK_CHANNEL2_MSK_8730A
#define BIT_BKDOK_MSK_8730A	BIT_TXDMAOK_CHANNEL3_MSK_8730A

/* rtl8730aa_halinit.c */
u32 rtl8730aa_init(PADAPTER);
void rtl8730aa_init_default_value(PADAPTER);

/* rtl8730aa_halmac.c */
int rtl8730aa_halmac_init_adapter(PADAPTER);

/* rtl8730aa_io.c */

/* rtl8730aa_led.c */
void rtl8730aa_InitSwLeds(PADAPTER);
void rtl8730aa_DeInitSwLeds(PADAPTER);

/* rtl8730as_xmit.c */
#define OFFSET_SZ 0

s32 rtl8730aa_init_xmit_priv(PADAPTER);
void rtl8730aa_free_xmit_priv(PADAPTER);
struct xmit_buf *rtl8730aa_dequeue_xmitbuf(struct rtw_tx_ring *);
void rtl8730aa_fill_fake_txdesc(PADAPTER, u8 *pDesc, u32 BufferLen,
				u8 IsPsPoll, u8 IsBTQosNull, u8 bDataFrame);
int rtl8730aa_init_txbd_ring(PADAPTER, unsigned int q_idx,
			     unsigned int entries);
void rtl8730aa_free_txbd_ring(PADAPTER, unsigned int prio);

void rtl8730aa_tx_isr(PADAPTER, int prio);

s32 rtl8730aa_mgnt_xmit(PADAPTER, struct xmit_frame *);
s32 rtl8730aa_hal_xmit(PADAPTER, struct xmit_frame *);
#ifdef CONFIG_RTW_MGMT_QUEUE
s32 rtl8730aa_hal_mgmt_xmitframe_enqueue(PADAPTER, struct xmit_frame *);
#endif
s32 rtl8730aa_hal_xmitframe_enqueue(PADAPTER, struct xmit_frame *);

#ifdef CONFIG_XMIT_THREAD_MODE
s32 rtl8730aa_xmit_buf_handler(PADAPTER);
#endif
u32 InitMAC_TRXBD_8730AA(PADAPTER adapter);

void rtl8730aa_xmitframe_resume(PADAPTER);

/* rtl8730as_recv.c */
s32 rtl8730aa_init_recv_priv(PADAPTER);
void rtl8730aa_free_recv_priv(PADAPTER);
int rtl8730aa_init_rxbd_ring(PADAPTER);
void rtl8730aa_free_rxbd_ring(PADAPTER);

u16 get_txbd_rw_reg(u16 q_idx);

/* rtl8730as_ops.c */

#endif /* _RTL8730AA_H_ */
