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
#ifndef __RTL8730A_RECV_H__
#define __RTL8730A_RECV_H__

#define MAX_RECVBUF_SZ (4000)

/* Rx smooth factor */
#define	Rx_Smooth_Factor (20)

/*-----------------------------------------------------------------*/
/*	RTL8730A RX BUFFER DESC                                      */
/*-----------------------------------------------------------------*/
/*DWORD 0*/
#define SET_RX_BUFFER_DESC_DATA_LENGTH_8730A(__pRxStatusDesc, __Value)		SET_BITS_TO_LE_4BYTE(__pRxStatusDesc, 0, 14, __Value)
#define SET_RX_BUFFER_DESC_LS_8730A(__pRxStatusDesc, __Value)	SET_BITS_TO_LE_4BYTE(__pRxStatusDesc, 15, 1, __Value)
#define SET_RX_BUFFER_DESC_FS_8730A(__pRxStatusDesc, __Value)		SET_BITS_TO_LE_4BYTE(__pRxStatusDesc, 16, 1, __Value)
#define SET_RX_BUFFER_DESC_TOTAL_LENGTH_8730A(__pRxStatusDesc, __Value)		SET_BITS_TO_LE_4BYTE(__pRxStatusDesc, 16, 15, __Value)

#define GET_RX_BUFFER_DESC_OWN_8730A(__pRxStatusDesc)		LE_BITS_TO_4BYTE(__pRxStatusDesc, 31, 1)
#define GET_RX_BUFFER_DESC_LS_8730A(__pRxStatusDesc)		LE_BITS_TO_4BYTE(__pRxStatusDesc, 15, 1)
#define GET_RX_BUFFER_DESC_FS_8730A(__pRxStatusDesc)		LE_BITS_TO_4BYTE(__pRxStatusDesc, 16, 1)
#define GET_RX_BUFFER_DESC_RX_TAG_8730A(__pRxStatusDesc)		LE_BITS_TO_4BYTE(__pRxStatusDesc, 16, 13)

/*DWORD 1*/
#define SET_RX_BUFFER_PHYSICAL_LOW_8730A(__pRxStatusDesc, __Value)		SET_BITS_TO_LE_4BYTE(__pRxStatusDesc+4, 0, 32, __Value)

/*DWORD 2*/
#ifdef CONFIG_64BIT_DMA
#define SET_RX_BUFFER_PHYSICAL_HIGH_8730A(__pRxStatusDesc, __Value)		SET_BITS_TO_LE_4BYTE(__pRxStatusDesc+8, 0, 32, __Value)
#else
#define SET_RX_BUFFER_PHYSICAL_HIGH_8730A(__pRxStatusDesc, __Value)
#endif


#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
s32 rtl8730as_init_recv_priv(PADAPTER padapter);
void rtl8730as_free_recv_priv(PADAPTER padapter);
s32 rtl8730as_recv_hdl(_adapter *padapter);
#endif

#ifdef CONFIG_USB_HCI
int rtl8730au_init_recv_priv(_adapter *padapter);
void rtl8730au_free_recv_priv(_adapter *padapter);
void rtl8730au_init_recvbuf(_adapter *padapter, struct recv_buf *precvbuf);
#endif

#if defined(CONFIG_PCI_HCI) || defined(CONFIG_AXI_HCI)
s32 rtl8730aa_init_recv_priv(PADAPTER padapter);
void rtl8730aa_free_recv_priv(PADAPTER padapter);
#endif

void rtl8730a_query_rx_desc_status(union recv_frame *precvframe, u8 *pdesc);

#endif /* __RTL8730A_RECV_H__ */
