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
#include "mac/halmac_bit2.h"
#include "mac/halmac_reg2.h"
#include "rtw_xmit.h"

#ifndef _HAL_TX_8730EA_H_
#define _HAL_TX_8730EA_H_

#define TXBD_PAGE_SIZE 128

#define TX_BD_NUM_8730EA 32
#define RX_BD_NUM_8730EA 32

#ifdef CONFIG_CONCURRENT_MODE
#define TX_BD_NUM_8730EA_BCN 4
#define TX_BD_NUM_8730EA_BEQ (TX_BD_NUM_8730EA * 2)
#else
#define TX_BD_NUM_8730EA_BCN 2
#define TX_BD_NUM_8730EA_BEQ TX_BD_NUM_8730EA
#endif /* CONFIG_CONCURRENT_MODE */

#define TX_BD_NUM_8730EA_VIQ TX_BD_NUM_8730EA
#define TX_BD_NUM_8730EA_VOQ TX_BD_NUM_8730EA
#define TX_BD_NUM_8730EA_BKQ TX_BD_NUM_8730EA
#define TX_BD_NUM_8730EA_CMD 4

#define TX_BD_NUM_8730EA_MGQ TX_BD_NUM_8730EA
#define TX_BD_NUM_8730EA_H0Q TX_BD_NUM_8730EA

#define RTL8730AE_SEG_NUM (TX_BUFFER_SEG_NUM) /* 0:2 seg, 1: 4 seg, 2: 8 seg */

#define MAX_RX_TAG_VALUE 0x1FFF

#define BE_QUEUE_IDX_8730EA 0x0
#define BK_QUEUE_IDX_8730EA 0x1
#define VI_QUEUE_IDX_8730EA 0x2
#define VO_QUEUE_IDX_8730EA 0x3
#define MGT_QUEUE_IDX_8730EA 0x4
#define HIGH_QUEUE_IDX_8730EA 0x5
#define BCN_QUEUE_IDX_8730EA 0x6
#define TXCMD_QUEUE_IDX_8730EA 0x7

#define RX_QUEUE_IDX_8730EA 0x10

#define	RTW_TXDESC_QSEL_BE 0x0
#define	RTW_TXDESC_QSEL_BK 0x2
#define	RTW_TXDESC_QSEL_VI 0x5
#define	RTW_TXDESC_QSEL_VO 0x7
#define	RTW_TXDESC_QSEL_BCN 0x10
#define	RTW_TXDESC_QSEL_HIGH 0x11
#define	RTW_TXDESC_QSEL_MGT 0x12
#define	RTW_TXDESC_QSEL_CMD 0x13


/* data * 4 + MGQ * 1 + HIQ * 1 */
#define TX_DMA_CHANNEL_ENTRY_8730EA 6
/* RXQ * 1 + RPQ * 1 */
#define RX_DMA_CHANNEL_ENTRY_8730EA 1

#define GET_RX_RP_PKT_POLLUTED(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 31, 1)
#define GET_RX_RP_PKT_PCIE_SEQ(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 16, 15)
#define GET_RX_RP_PKT_TX_STS(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 13, 3)
#define GET_RX_RP_PKT_QSEL(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 8, 5)
#define GET_RX_RP_PKT_MAC_ID(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 0, 8)

/* TX BD */
#define SET_TXBUFFER_DESC_LEN(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x00, 0, 16, __Value)
#define SET_TXBUFFER_DESC_PSB(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x00, 16, 8, __Value)
#define SET_TXBUFFER_DESC_OWN(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x00, 31, 1, __Value)
#define SET_TXBUFFER_DESC_LS(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x00, 30, 1, __Value)

#define SET_TXBUFFER_DESC_ADD_LOW(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x04, 0, 32, __Value)
#define SET_TXBUFFER_DESC_ADD_HIGH(__pTxDesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc + 0x08, 0, 32, __Value)

#define GET_TXBUFFER_DESC_LEN(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x00, 0, 16)
#define GET_TXBUFFER_DESC_PSB(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x00, 16, 8)
#define GET_TXBUFFER_DESC_OWN(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x00, 30, 1)
#define GET_TXBUFFER_DESC_LS(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x00, 30, 1)
#define GET_TXBUFFER_DESC_ADD_LOW(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x04, 0, 32)
#define GET_TXBUFFER_DESC_ADD_HIGH(_pTxDesc) \
	LE_BITS_TO_4BYTE(_pTxDesc + 0x08, 0, 32)


#define SET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset * 8), 0, 16, __Value)
#define SET_TXBUFFER_DESC_MODE_WITH_OFFSET(__pTxDesc, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset * 8), 31, 1, __Value)
#define SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset * 8) + 4, 0, 32, __Value)
#define SET_TXBUFFER_DESC_ADD_HIGH_WITH_OFFSET(__pTxDesc, __Offset, __Value) \
	SET_BITS_TO_LE_4BYTE(__pTxDesc+(__Offset * 8) + 8, 0, 32, __Value)

#define GET_TXBUFFER_DESC_LEN_WITH_OFFSET(__pTxDesc, __Offset) \
	LE_BITS_TO_4BYTE(__pTxDesc+(__Offset * 8), 0, 16)
#define GET_TXBUFFER_DESC_MODE_WITH_OFFSET(__pTxDesc, __Offset) \
	LE_BITS_TO_4BYTE(__pTxDesc+(__Offset * 8), 31, 1)
#define GET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(__pTxDesc, __Offset) \
	LE_BITS_TO_4BYTE(__pTxDesc+(__Offset * 8) + 4, 0, 32)
#define GET_TXBUFFER_DESC_ADD_HIGH_WITH_OFFSET(__pTxDesc, __Offset) \
	LE_BITS_TO_4BYTE(__pTxDesc+(__Offset * 8) + 8, 0, 32)


/* Addr Info */
#define SET_ADDR_INFO_LEN(__addr_info, __value) \
	SET_BITS_TO_LE_4BYTE(__addr_info + 0x00, 0, 16, __value)
#define SET_ADDR_INFO_LS(__addr_info, __value)				\
	SET_BITS_TO_LE_4BYTE(__addr_info + 0x00, 31, 1, __value)
#define SET_ADDR_INFO_ADDR_LOW_LSB(__addr_info, __value) \
	SET_BITS_TO_LE_4BYTE(__addr_info + 0x04, 0, 32, __value)

/* Addr Info */
#define GET_ADDR_INFO_LEN(__addr_info) \
	LE_BITS_TO_4BYTE(__addr_info + 0x00, 0, 16)
#define GET_ADDR_INFO_LS(__addr_info) \
	LE_BITS_TO_4BYTE(__addr_info + 0x00, 31, 1)
#define GET_ADDR_INFO_ADDR_LOW(__addr_info) \
	LE_BITS_TO_4BYTE(__addr_info + 0x04, 0, 32)


#define RX_RP_PACKET_SIZE 4

/* RX BD */
#define SET_RX_BD_RXBUFFSIZE(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x00, 0, 14, __Value)
#define SET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x04, 0, 32, __Value)
#define SET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd, __Value) \
	SET_BITS_TO_LE_4BYTE(__pRxBd + 0x08, 0, 32, __Value)

/* RX RD INFO */
#define SET_RX_BD_INFO_TOTALRXPKTSIZE(rxdesc, value) \
	SET_BITS_TO_LE_4BYTE(rxdesc + 0x00, 16, 13, value)
#define SET_RX_BD_INFO_TAG(rxdesc, value) \
	SET_BITS_TO_LE_4BYTE(rxdesc + 0x00, 16, 13, value)
#define SET_RX_BD_INFO_FS(rxdesc, value) \
	SET_BITS_TO_LE_4BYTE(rxdesc + 0x00, 15, 1, value)
#define SET_RX_BD_INFO_LS(rxdesc, value) \
	SET_BITS_TO_LE_4BYTE(rxdesc + 0x00, 14, 1, value)
#define SET_RX_BD_INFO_HW_W_SIZE(rxdesc, value) \
	SET_BITS_TO_LE_4BYTE(rxdesc + 0x00, 0, 14, value)

#define GET_RX_BD_INFO_TOTALRXPKTSIZE(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 16, 13)
#define GET_RX_BD_INFO_TAG(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 16, 13)
#define GET_RX_BD_INFO_FS(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 15, 1)
#define GET_RX_BD_INFO_LS(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 14, 1)
#define GET_RX_BD_INFO_HW_W_SIZE(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 0, 14)

/* RX RP PACKET */
#define SET_RX_RP_PKT_POLLUTED(rppkt, value) \
	SET_BITS_TO_LE_4BYTE(rppkt + 0x00, 31, 1, value)
#define SET_RX_RP_PKT_PCIE_SEQ(rppkt, value) \
	SET_BITS_TO_LE_4BYTE(rppkt + 0x00, 16, 15, value)
#define SET_RX_RP_PKT_TX_STS(rppkt, value) \
	SET_BITS_TO_LE_4BYTE(rppkt + 0x00, 13, 3, value)
#define SET_RX_RP_PKT_QSEL(rppkt, value) \
	SET_BITS_TO_LE_4BYTE(rppkt + 0x00, 8, 5, value)
#define SET_RX_RP_PKT_MAC_ID(rppkt, value) \
	SET_BITS_TO_LE_4BYTE(rppkt + 0x00, 0, 8, value)

#define GET_RX_RP_PKT_POLLUTED(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 31, 1)
#define GET_RX_RP_PKT_PCIE_SEQ(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 16, 15)
#define GET_RX_RP_PKT_TX_STS(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 13, 3)
#define GET_RX_RP_PKT_QSEL(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 8, 5)
#define GET_RX_RP_PKT_MAC_ID(rppkt) LE_BITS_TO_4BYTE(rppkt + 0x00, 0, 8)

/* CONFIG_PHL_TXSC */
#define TID_0_QSEL RTW_TXDESC_QSEL_BE
#define TID_1_QSEL RTW_TXDESC_QSEL_BK
#define TID_2_QSEL RTW_TXDESC_QSEL_BK
#define TID_3_QSEL RTW_TXDESC_QSEL_BE
#define TID_4_QSEL RTW_TXDESC_QSEL_VI
#define TID_5_QSEL RTW_TXDESC_QSEL_VI
#define TID_6_QSEL RTW_TXDESC_QSEL_VO
#define TID_7_QSEL RTW_TXDESC_QSEL_VO
#define TID_0_IND 0
#define TID_1_IND 0
#define TID_2_IND 1
#define TID_3_IND 1
#define TID_4_IND 0
#define TID_5_IND 1
#define TID_6_IND 0
#define TID_7_IND 1

enum rxbd_mode_8730ea {
	RXBD_MODE_PACKET = 0,
	RXBD_MODE_SEPARATION = 1,
	RXBD_MODE_MAX = 0xFF
};

struct bd_ram {
	u8 sidx;
	u8 max;
	u8 min;
};

#endif

