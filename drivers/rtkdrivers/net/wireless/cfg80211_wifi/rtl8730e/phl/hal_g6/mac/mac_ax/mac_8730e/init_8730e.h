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

#ifndef _MAC_AX_INIT_8730E_H_
#define _MAC_AX_INIT_8730E_H_

#include "mac/type.h"
#if MAC_AX_8730E_SUPPORT

#define TX_FIFO_SIZE_8730E	32768
#define RX_FIFO_SIZE_8730E	16384
#define TRX_SHARE_SIZE0_8730E	8192
#define TRX_SHARE_SIZE1_8730E	8192
#define TRX_SHARE_SIZE2_8730E	8192

#define CTRL_INFO_ENTRY_SIZE_8730E 40
#define CRC5_ENTRY_SIZE_8730E 8
#define CTRL_INFO_BASE_8730E 0x40000 /* 0x40040000 */
#define CRC5_BASE_8730E 0x40A80 /* 0x40040A80 */

#define TX_FIFO_SIZE_LA_8730E	(TX_FIFO_SIZE_8730E >>  1)
#define TX_FIFO_SIZE_RX_EXPAND_1BLK_8730E	\
	(TX_FIFO_SIZE_8730E - TRX_SHARE_SIZE0_8730E)
#define RX_FIFO_SIZE_RX_EXPAND_1BLK_8730E	\
	(RX_FIFO_SIZE_8730E + TRX_SHARE_SIZE0_8730E)
#define TX_FIFO_SIZE_RX_EXPAND_2BLK_8730E	\
	(TX_FIFO_SIZE_8730E - TRX_SHARE_SIZE2_8730E)
#define RX_FIFO_SIZE_RX_EXPAND_2BLK_8730E	\
	(RX_FIFO_SIZE_8730E + TRX_SHARE_SIZE2_8730E)
#define TX_FIFO_SIZE_RX_EXPAND_3BLK_8730E	\
	(TX_FIFO_SIZE_8730E - TRX_SHARE_SIZE2_8730E - TRX_SHARE_SIZE0_8730E)
#define RX_FIFO_SIZE_RX_EXPAND_3BLK_8730E	\
	(RX_FIFO_SIZE_8730E + TRX_SHARE_SIZE2_8730E + TRX_SHARE_SIZE0_8730E)
#define TX_FIFO_SIZE_RX_EXPAND_4BLK_8730E	\
	(TX_FIFO_SIZE_8730E - (2 * TRX_SHARE_SIZE2_8730E))
#define RX_FIFO_SIZE_RX_EXPAND_4BLK_8730E	\
	(RX_FIFO_SIZE_8730E + (2 * TRX_SHARE_SIZE2_8730E))

#define RX_DMA_RESERVED_SIZE_8730E 0x80
#define RX_DMA_BOUNDARY_8730E (RX_FIFO_SIZE_8730E - RX_DMA_RESERVED_SIZE_8730E - 1)

#define TX_PAGE_SIZE_8730e		128
#define TX_PAGE_SIZE_SHIFT_8730e	7 /* 128 = 2^7 */

#define WLAN_TXQ_RPT_EN			0x3F

#define RSVD_PG_DRV_NUM			16
#define RSVD_PG_CPU_INSTRUCTION_NUM	0
#define RSVD_PG_FW_TXBUF_NUM		0
#define RSVD_PG_CSIBUF_NUM		4
#define RSVD_PG_DLLB_NUM		(TX_FIFO_SIZE_8730E / 3 >> \
					TX_PAGE_SIZE_SHIFT_8730e)

#define RSVD_PG_BCNQ_NUM		0x08
#ifdef CONFIG_CONCURRENT_MODE
#define RSVD_PG_BCNQ1_NUM		0x04
#else
#define RSVD_PG_BCNQ1_NUM		0x00
#endif
#define RSVD_PG_BCNQ2_NUM		0x00

/* page 0-4 for bcn, 4 for null data, 5 for bt qos null, 6 for qos null */
#define RSVD_PAGE_NULL_DATA (4)
#define RSVD_PAGE_BT_QOS_NULL (RSVD_PAGE_NULL_DATA + 1)
#define RSVD_PAGE_QOS_NULL (RSVD_PAGE_BT_QOS_NULL + 1)
#define RSVD_PAGE_PROB_RSP (0)
#define RSVD_PAGE_PS_POLL (0)
#define RSVD_PAGE_CTS2SELF (0)
#define RSVD_PAGE_LTECOEX_QOSNULL (0)

/* For WoWLan , more reserved page */
#ifdef CONFIG_WOWLAN
#define RSVD_PG_WOWLAN_NUM		0x07
#else
#define RSVD_PG_WOWLAN_NUM		0x00
#endif

/* ARP Rsp:1, RWC:1, GTK Info:1, GTK RSP:2, GTK EXT MEM:2 */
#define RSVD_PAGE_GTK_WOW_START (RSVD_PG_BCNQ_NUM + RSVD_PG_BCNQ1_NUM + RSVD_PG_BCNQ2_NUM)
#define RSVD_PAGE_REMOTE_CTRL_INFO (RSVD_PAGE_GTK_WOW_START)
#define RSVD_PAGE_ARP_RSP (RSVD_PAGE_REMOTE_CTRL_INFO + 1)
#define RSVD_PAGE_GTK_INFO (RSVD_PAGE_ARP_RSP + 1)
#define RSVD_PAGE_GTK_RSP (RSVD_PAGE_GTK_INFO + 1)
#define RSVD_PAGE_GTK_EXT_MEM (RSVD_PAGE_GTK_INFO + 2)
#define RSVD_PAGE_GTK_WOW_END (RSVD_PAGE_GTK_EXT_MEM + 2)

#define C2H_PKT_BUF_8730e		256

#define WLAN_SIFS_CCK_DUR_TUNE	0x0A
#define WLAN_SIFS_OFDM_DUR_TUNE	0x10
#define WLAN_SIFS_CCK_CONT_TX	0x0A
#define WLAN_SIFS_OFDM_CONT_TX	0x10
#define WLAN_SIFS_CCK_TRX	0x0A
#define WLAN_SIFS_OFDM_TRX	0x10
#define WLAN_SIFS_CCK_T2T	0x50
#define WLAN_SIFS_CCK_R2T	0x50
#define WLAN_SIFS_OFDM_T2T	0x75
#define WLAN_SIFS_OFDM_R2T	0x75		/*HE_TRIG meets spec timing requirements, https://jira.realtek.com/browse/AMEBALITE-230*/
#define WLAN_DATA_RATE_FB_CNT_1_4	0x01000000
#define WLAN_DATA_RATE_FB_CNT_5_8	0x08070504
#define WLAN_RTS_RATE_FB_CNT_5_8	0x08070605
#define WLAN_AMPDU_MAX_TIME		0x5E
#define WLAN_PRE_TXCNT_TIME_TH		0x8C
#define WLAN_MAX_AGG_PKT_LIMIT		0x3F
#define WLAN_RTS_MAX_AGG_PKT_LIMIT	0x20
#define WLAN_RTS_LEN_TH			0xFF
#define WLAN_RTS_TX_TIME_TH		0x08
#define WLAN_BAR_RETRY_LIMIT		0x01
#define WLAN_RETRY_PKT_LIMIT		0x02
#define WALN_FAST_EDCA_VO_TH		0x06
#define WLAN_FAST_EDCA_VI_TH		0x06
#define WLAN_FAST_EDCA_BE_TH		0x06
#define WLAN_FAST_EDCA_BK_TH		0x06
#define WLAN_EDCA_VO_PARAM	0x002FA226
#define WLAN_EDCA_VI_PARAM	0x005EA324
#define WLAN_EDCA_BE_PARAM	0x005EA42B
#define WLAN_EDCA_BK_PARAM	0x0000A44F
#define WLAN_PIFS_TIME		0x1C
#define WLAN_SLOT_TIME		0x09
#define WLAN_RDG_NAV		0x180
#define WLAN_TXOP_NAV		0x40
#define WLAN_CCK_RX_TSF		0x4f
#define WLAN_OFDM_RX_TSF	0x2f
#define WLAN_TBTT_PROHIBIT	0x04 /* unit : 32us */
#define WLAN_TBTT_HOLD_TIME	0x064 /* unit : 32us */
#define WLAN_DRV_EARLY_INT	0x04
#define WLAN_BCN_CTRL_CLT0	0x10
#define WLAN_BCN_DMA_TIME	0x02
#define WLAN_BCN_MAX_ERR	0xFF
#define MAC_CLK_SPEED	0x28 /* 40M */
#define WLAN_RESP_TXRATE		0x4
#define WLAN_ACK_TO_OFDM			0x26
#define WLAN_ACK_TO_CCK			0x41
#define WLAN_EIFS_DUR_TUNE	0x13A
#define WLAN_NAV_MAX		0xeb
#define WLAN_RX_FILTER0		0x0404FFFF/*RX BC trigger is for ul ofdma cfo tracking, udp tx may only rx beacon and trigger but need cfo < 370Hz */
#define WLAN_RX_FILTER2		0xFFFF
#define WLAN_RCR_CFG		0x741c40ce
#define WLAN_RXPKT_MAX_SZ	11460
#define WLAN_TX_FUNC_CFG1		0x30
#define WLAN_TX_FUNC_CFG2		0x30
#define WLAN_MAC_OPT_FUNC2		0xB1830041
#define WLAN_MAC_OPT_NORM_FUNC1		0x98
#define WLAN_MAC_OPT_LB_FUNC1		0x80
#define COEX_GNT_WLAN  0x14
#define WLAN_RETRY_LIMIT		0x30

#define WLAN_RATE_RRSR_CCK_ONLY_1M	0xFFFF1

#define WLAN_SIFS_CFG	(WLAN_SIFS_CCK_CONT_TX | \
			(WLAN_SIFS_OFDM_CONT_TX << BIT_SHIFT_SIFS_OFDM_CTX) | \
			(WLAN_SIFS_CCK_TRX << BIT_SHIFT_SIFS_CCK_TRX) | \
			(WLAN_SIFS_OFDM_TRX << BIT_SHIFT_SIFS_OFDM_TRX))
#define WLAN_SIFS_DUR_TUNE	(WLAN_SIFS_CCK_DUR_TUNE | \
				(WLAN_SIFS_OFDM_DUR_TUNE << 8))
#define WLAN_NAV_CFG		(WLAN_RDG_NAV | (WLAN_TXOP_NAV << 16))
#define WLAN_RX_TSF_CFG		(WLAN_CCK_RX_TSF | (WLAN_OFDM_RX_TSF) << 8)
#define WLAN_TBTT_TIME	(WLAN_TBTT_PROHIBIT |\
			(WLAN_TBTT_HOLD_TIME << BIT_SHIFT_TBTT_HOLD_TIME))
#define WLAN_ACKTO   (WLAN_ACK_TO_OFDM | (WLAN_ACK_TO_CCK << 12))
#define WLAN_RXPKT_MAX_SZ_512	(WLAN_RXPKT_MAX_SZ >> 9)


#ifdef CONFIG_NEW_HALMAC_INTERFACE
struct mac_ax_adapter *get_mac_8730e_adapter(enum mac_ax_intf intf,
		u8 cv, void *phl_adapter,
		void *drv_adapter,
		struct mac_ax_pltfm_cb *pltfm_cb)
#else
struct mac_ax_adapter *get_mac_8730e_adapter(enum mac_ax_intf intf,
		u8 cv, void *drv_adapter,
		struct mac_ax_pltfm_cb *pltfm_cb);
#endif

u32 init_trx_cfg_8730e(struct mac_ax_adapter *adapter,
		       enum halmac_trx_mode mode);

u32 init_protocol_cfg_8730e(struct mac_ax_adapter *adapter);

u32 init_edca_cfg_8730e(struct mac_ax_adapter *adapter);

u32 init_wmac_cfg_8730e(struct mac_ax_adapter *adapter);

u32 txdma_queue_mapping_8730e(struct mac_ax_adapter *adapter,
			      enum halmac_trx_mode mode);

u32 rqpn_parser_8730e(struct mac_ax_adapter *adapter,
		      enum halmac_trx_mode mode, struct halmac_rqpn *tbl);

u32 priority_queue_cfg_8730e(struct mac_ax_adapter *adapter,
			     enum halmac_trx_mode mode);

u32 set_trx_fifo_info_8730e(struct mac_ax_adapter *adapter,
			    enum halmac_trx_mode mode);

u32 pg_num_parser_8730e(struct mac_ax_adapter *adapter,
			enum halmac_trx_mode mode, struct halmac_pg_num *tbl);

u32 init_txq_ctrl_8730e(struct mac_ax_adapter *adapter);

u32 init_rate_fallback_ctrl_8730e(struct mac_ax_adapter *adapter);

void cfg_mac_clk_8730e(struct mac_ax_adapter *adapter);

void cfg_32k_clk_8730e(struct mac_ax_adapter *adapter);

u32 init_sifs_ctrl_8730e(struct mac_ax_adapter *adapter);

u32 init_low_pwr_8730e(struct mac_ax_adapter *adapter);

u32 enable_port_8730e(struct mac_ax_adapter *adapter, u8 port, bool en);

u32 cfg_mac_addr_8730e(struct mac_ax_adapter *adapter, u8 port,
		       union halmac_wlan_addr *addr);

u32 cfg_bssid_8730e(struct mac_ax_adapter *adapter, u8 port,
		    union halmac_wlan_addr *addr);

u32 cfg_transmitter_addr_8730e(struct mac_ax_adapter *adapter,
			       u8 port, union halmac_wlan_addr *addr);

u32 cfg_net_type_8730e(struct mac_ax_adapter *adapter, u8 port,
		       enum mac_ax_net_type net_type);

u32 cfg_sta_aid_8730e(struct mac_ax_adapter *adapter, u8 port, u16 aid);

u32 cfg_macid_8730e(struct mac_ax_adapter *adapter,
		    struct rtw_phl_stainfo_t *sta);

#endif /* #if MAC_AX_8730E_SUPPORT */
#endif
