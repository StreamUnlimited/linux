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
#define _HAL_TRX_8730EA_C_
#include "../../hal_headers.h"
#include "../rtl8730e_hal.h"
#include "hal_trx_8730ea.h"

/**
 * this function will query total hw tx dma channels number
 *
 * returns the number of hw tx dma channel
 */
static u8 hal_query_txch_num_8730ea(void)
{
	u8 ch_num = 0;

	ch_num = TX_DMA_CHANNEL_ENTRY_8730EA;

	return ch_num;
}

/**
 * this function will query total hw rx dma channels number
 *
 * returns the number of hw rx dma channel
 */
static u8 hal_query_rxch_num_8730ea(void)
{
	u8 ch_num = 0;

	ch_num = RX_DMA_CHANNEL_ENTRY_8730EA;

	return ch_num;
}
static u8 hal_qsel_to_tid_8730ea(struct hal_info_t *hal, u8 qsel_id, u8 tid_indic)
{
	u8 tid = 0;

	switch (qsel_id) {
	case RTW_TXDESC_QSEL_BE:
		tid = (1 == tid_indic) ? RTW_PHL_RING_CAT_TID3 : RTW_PHL_RING_CAT_TID0;
		break;
	case RTW_TXDESC_QSEL_BK:
		tid = (1 == tid_indic) ? RTW_PHL_RING_CAT_TID2 : RTW_PHL_RING_CAT_TID1;
		break;
	case RTW_TXDESC_QSEL_VI:
		tid = (1 == tid_indic) ? RTW_PHL_RING_CAT_TID5 : RTW_PHL_RING_CAT_TID4;
		break;
	case RTW_TXDESC_QSEL_VO:
		tid = (1 == tid_indic) ? RTW_PHL_RING_CAT_TID7 : RTW_PHL_RING_CAT_TID6;
		break;
	case RTW_TXDESC_QSEL_MGT:
		tid = RTW_PHL_RING_CAT_MGNT;
		break;
	case RTW_TXDESC_QSEL_HIGH:
		tid = RTW_PHL_RING_CAT_HIQ;
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown qsel_id (%d)\n",
			  qsel_id);
		tid = 0;
		break;
	}

	return tid;
}

/**
 * Get target DMA channel's BD num hw register of rtl8730e
 * @dma_ch: input target dma channel with the hw definition of rtl8730ea
 * return the BD num hw register
 */
static u32 _hal_get_bd_num_reg_8730ea(u8 dma_ch)
{
	u32 reg = 0;

	switch (dma_ch) {
	case MGT_QUEUE_IDX_8730EA:
		reg = REG_MGQ_TXBD_NUM;
		break;
	case RX_QUEUE_IDX_8730EA:
		reg = REG_RX_RXBD_NUM;
		break;
	case VO_QUEUE_IDX_8730EA:
		reg = REG_VOQ_TXBD_NUM;
		break;
	case VI_QUEUE_IDX_8730EA:
		reg = REG_VIQ_TXBD_NUM;
		break;
	case BE_QUEUE_IDX_8730EA:
		reg = REG_BEQ_TXBD_NUM;
		break;
	case BK_QUEUE_IDX_8730EA:
		reg = REG_BKQ_TXBD_NUM;
		break;
	case HIGH_QUEUE_IDX_8730EA:
		reg = REG_MQ0_TXBD_NUM;
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown channel (%d)\n",
			  dma_ch);
		reg = 0xFFFF;
		break;
	}

	return reg;
}


/**
 * Get target DMA channel's BD Desc hw register of rtl8730e
 * @dma_ch: input target dma channel with the hw definition of rtl8730ea
 * return the BD Desc hw register
 */
static void _hal_get_bd_desc_reg_8730ea(u8 dma_ch, u32 *addr_l, u32 *addr_h)
{
	u32 reg = 0;

	switch (dma_ch) {
	case BCN_QUEUE_IDX_8730EA:
		*addr_l = REG_BCNQ_TXBD_DESA;
		break;
	case MGT_QUEUE_IDX_8730EA:
		*addr_l = REG_MGQ_TXBD_DESA;
		break;
	case VO_QUEUE_IDX_8730EA:
		*addr_l = REG_VOQ_TXBD_DESA;
		break;
	case VI_QUEUE_IDX_8730EA:
		*addr_l = REG_VIQ_TXBD_DESA;
		break;
	case BE_QUEUE_IDX_8730EA:
		*addr_l = REG_BEQ_TXBD_DESA;
		break;
	case BK_QUEUE_IDX_8730EA:
		*addr_l = REG_BKQ_TXBD_DESA;
		break;
	case RX_QUEUE_IDX_8730EA:
		*addr_l = REG_RXQ_RXBD_DESA;
		break;
	case HIGH_QUEUE_IDX_8730EA:
		*addr_l = REG_MQ0_TXBD_DESA;
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown channel (%d)\n",
			  dma_ch);
		reg = 0xFFFF;
		break;
	}
	*addr_h = 0;

}



/**
 * Get target DMA channel's BD Index hw register of rtl8730e
 * @dma_ch: input target dma channel with the hw definition of rtl8730ea
 * return the BD Index hw register
 */
static u32 _hal_get_bd_idx_reg_8730ea(u8 dma_ch)
{
	u32 reg = 0;

	switch (dma_ch) {
	case VO_QUEUE_IDX_8730EA:
		reg = REG_VOQ_TXBD_IDX;
		break;
	case VI_QUEUE_IDX_8730EA:
		reg = REG_VIQ_TXBD_IDX;
		break;
	case BE_QUEUE_IDX_8730EA:
		reg = REG_BEQ_TXBD_IDX;
		break;
	case BK_QUEUE_IDX_8730EA:
		reg = REG_BKQ_TXBD_IDX;
		break;
	case MGT_QUEUE_IDX_8730EA:
		reg = REG_MGQ_TXBD_IDX;
		break;
	case HIGH_QUEUE_IDX_8730EA:
		reg = REG_MQ0_TXBD_IDX;
		break;
	case RX_QUEUE_IDX_8730EA:
		reg = REG_RXQ_RXBD_IDX;
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown channel (%d)\n",
			  dma_ch);
		reg = 0xFFFF;
		break;
	}

	return reg;
}


/**
 * this function maps the sw xmit ring identified by macid, tid and band
 * to rtl8730ea hw tx dma channel
 * @macid: input target macid range is 0 ~ 127
 * @cat: input target packet category, see enum rtw_phl_ring_cat
 * @band: input target band, 0 for band 0 / 1 for band 1
 *
 * returns the mapping hw dma channel defined by XXX_QUEUE_IDX_8730EA
 * if the input parameter is unknown value, returns BE_QUEUE_IDX_8730EA
 */
static u8 hal_mapping_hw_tx_chnl_8730ea(u16 macid, enum rtw_phl_ring_cat cat,
					u8 band)
{
	u8 dma_ch = 0;

	/* hana_todo, decided by cat only currently,
		we should consider more situation later */

	if (0 == band) {
		switch (cat) {
		case RTW_PHL_RING_CAT_TID0:
		case RTW_PHL_RING_CAT_TID3:
			dma_ch = BE_QUEUE_IDX_8730EA;
			break;
		case RTW_PHL_RING_CAT_TID1:
		case RTW_PHL_RING_CAT_TID2:
			dma_ch = BK_QUEUE_IDX_8730EA;
			break;
		case RTW_PHL_RING_CAT_TID4:
		case RTW_PHL_RING_CAT_TID5:
			dma_ch = VI_QUEUE_IDX_8730EA;
			break;
		case RTW_PHL_RING_CAT_TID6:
		case RTW_PHL_RING_CAT_TID7:
			dma_ch = VO_QUEUE_IDX_8730EA;
			break;
		case RTW_PHL_RING_CAT_MGNT:
			dma_ch = MGT_QUEUE_IDX_8730EA;
			break;
		case RTW_PHL_RING_CAT_HIQ:
			dma_ch = HIGH_QUEUE_IDX_8730EA;
			break;
		default:
			dma_ch = BE_QUEUE_IDX_8730EA;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown category (%d)\n",
				  cat);
			break;
		}
	} else {
		dma_ch = BE_QUEUE_IDX_8730EA;
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown band (%d)\n",
			  band);
	}

	return dma_ch;
}

/**
 * this function will return total trx ring number of target dma channel
 * @ch_idx: input, target dma channel index
 *
 * NOTE, input host_idx and hw_idx ptr shall NOT be NULL
 */
static int hal_get_trxring_num_8730ea(struct rtw_hal_com_t *hal_com, u8 ch_idx)
{
	int ring_num = 0;

	switch (ch_idx) {
	case MGT_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_MGQ;
		break;
	case RX_QUEUE_IDX_8730EA:
		ring_num = RX_BD_NUM_8730EA;
		break;
	case VO_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_VOQ;
		break;
	case VI_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_VIQ;
		break;
	case BE_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_BEQ;
		break;
	case BK_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_BKQ;
		break;
	case HIGH_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_H0Q;
		break;
	case TXCMD_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_CMD;
		break;
	case BCN_QUEUE_IDX_8730EA:
		ring_num = TX_BD_NUM_8730EA_BCN;
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown channel (%d)\n",
			  ch_idx);
		ring_num = 0;
		break;
	}

	return ring_num;
}

/**
 * this function will return available txbd number of target dma channel
 * @ch_idx: input, target dma channel index
 * @host_idx: the ptr of current host index of this channel
 * @hw_idx: the ptr of current hw index of this channel
 *
 * NOTE, input host_idx and hw_idx ptr shall NOT be NULL
 */
static u16 hal_get_avail_txbd_8730ea(struct rtw_hal_com_t *hal_com, u8 ch_idx,
				     u16 *host_idx, u16 *hw_idx)
{
	struct bus_cap_t *bus_cap = &hal_com->bus_cap;
	u16 avail_txbd = 0;
	u32 tmp32 = 0, reg = 0;
	u8 tx_dma_ch = 0;

	tx_dma_ch = BE_QUEUE_IDX_8730EA + ch_idx;

	reg = _hal_get_bd_idx_reg_8730ea(tx_dma_ch);
	if (0xFFFF == reg) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			  "[WARNING]get dma channel register fail\n");
		avail_txbd = 0;
	} else {
		tmp32 = hal_read32(hal_com, reg);

		*host_idx = (u16)(tmp32 & 0x0FFF);
		*hw_idx = (u16)((tmp32 >> 16) & 0x0FFF);

		avail_txbd = hal_calc_avail_wptr(*hw_idx, *host_idx,
						 (u16)bus_cap->txbd_num);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "hal_get_avail_txbd_8730ea => dma_ch %d, host_idx %d, "
			  "hw_idx %d, avail_txbd %d\n",
			  ch_idx, *host_idx, *hw_idx, avail_txbd);
	}

	return avail_txbd;
}

/**
 * this function will return available txbd number of target dma channel
 * @ch_idx: input, target dma channel index
 * @host_idx: the ptr of current host index of this channel
 * @hw_idx: the ptr of current hw index of this channel
 *
 * NOTE, input host_idx and hw_idx ptr shall NOT be NULL
 */
static u16 hal_get_avail_rxbd_8730ea(struct rtw_hal_com_t *hal_com, u8 ch_idx,
				     u16 *host_idx, u16 *hw_idx)
{
	struct bus_cap_t *bus_cap = &hal_com->bus_cap;
	u16 avail_rxbd = 0;
	u32 tmp32 = 0, reg = 0;
	u8 rx_dma_ch = 0;

	rx_dma_ch = RX_QUEUE_IDX_8730EA + ch_idx;

	reg = _hal_get_bd_idx_reg_8730ea(rx_dma_ch);
	if (0xFFFF == reg) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]get dma channel register fail\n");
		avail_rxbd = 0;
	} else {
		tmp32 = hal_read32(hal_com, reg);

		*host_idx = (u16)(tmp32 & 0x0FFF);
		*hw_idx = (u16)((tmp32 >> 16) & 0x0FFF);

		avail_rxbd = hal_calc_avail_rptr(*host_idx, *hw_idx,
						 (u16)bus_cap->rxbd_num);
	}

	return avail_rxbd;
}


void _hal_fill_wp_addr_info_8730ea(struct rtw_hal_com_t *hal_com,
				   u8 *addr_info, struct rtw_pkt_buf_list *pkt,
				   u8 num, u8 mpdu_ls, u8 msdu_ls, bool lastwp)
{
	SET_ADDR_INFO_LEN(addr_info, pkt->length);
	SET_ADDR_INFO_ADDR_LOW_LSB(addr_info, pkt->phy_addr_l);

	//if (lastwp) {
	//	SET_ADDR_INFO_LS(addr_info, 1);
	//} else {
	//	SET_ADDR_INFO_LS(addr_info, 0);
	//}
}

u8 _hal_get_tid_indic_8730ea(u8 tid)
{
	u8 tid_indic = 0;
	switch (tid) {
	case RTW_PHL_RING_CAT_TID0:
	case RTW_PHL_RING_CAT_TID1:
	case RTW_PHL_RING_CAT_TID4:
	case RTW_PHL_RING_CAT_TID6:
	case RTW_PHL_RING_CAT_MGNT:
	case RTW_PHL_RING_CAT_HIQ:
		tid_indic = 0;
		break;
	case RTW_PHL_RING_CAT_TID3:
	case RTW_PHL_RING_CAT_TID2:
	case RTW_PHL_RING_CAT_TID5:
	case RTW_PHL_RING_CAT_TID7:
		tid_indic = 1;
		break;
	default:
		PHL_ERR("unknown tid %d\n", tid);
		break;
	}

	return tid_indic;
}

#ifdef CONFIG_PHL_TXSC

static u8 qsel_tbl[] = {
	TID_0_QSEL/*0*/, TID_1_QSEL/*1*/, TID_2_QSEL/*1*/, TID_3_QSEL/*0*/,
	TID_4_QSEL/*2*/, TID_5_QSEL/*2*/, TID_6_QSEL/*3*/, TID_7_QSEL/*2*/
};

static u8 tid_ind[] = {
	TID_0_IND, TID_1_IND, TID_2_IND, TID_3_IND,
	TID_4_IND, TID_5_IND, TID_6_IND, TID_7_IND
};

static enum rtw_hal_status
_hal_txsc_update_wd(struct hal_info_t *hal,
		    struct rtw_phl_pkt_req *req, u32 *wd_len) {
	struct rtw_xmit_req *tx_req = req ? req->tx_req : NULL;
	struct rtw_t_meta_data	*mdata;
	u32 *wd_words;
	u32 w0, w2, w3, w8, w9;

	if (NULL == tx_req)
	{
		return RTW_HAL_STATUS_FAILURE;
	}

	if (req->wd_len == 0)
	{
		rtw_hal_mac_ax_fill_txdesc(hal->mac, tx_req, req->wd_page,
					   wd_len);
		req->wd_len = (u8) * wd_len;
	} else
	{
		mdata = &tx_req->mdata;
		mdata->dma_ch = hal_mapping_hw_tx_chnl_8730ea(mdata->macid, mdata->tid, mdata->band);

		wd_words = (u32 *)req->wd_page;
		w0 = le32_to_cpu(wd_words[0])
		& ~(AX_TXD_TXPKTSIZE_MSK << AX_TXD_TXPKTSIZE_SH);
		w2 = le32_to_cpu(wd_words[2])
		& ~((AX_TXD_QSEL_MSK << AX_TXD_QSEL_SH)
		    | (AX_TXD_AGG_EN | AX_TXD_BK));
		w3 = le32_to_cpu(wd_words[3])
		& ~(AX_TXD_HW_SSN_SEL_MSK << AX_TXD_HW_SSN_SEL_SH);
		w8 = le32_to_cpu(wd_words[8])
		& ~(AX_TXD_EN_HWSEQ);
		w9 = le32_to_cpu(wd_words[3])
		& ~(AX_TXD_SEQ_MSK << AX_TXD_SEQ_SH);

		/* Update SSN SEL, DMA CH, QSEL, and TID indicator in WD cache */
		w0 |= (mdata->pktlen & AX_TXD_TXPKTSIZE_MSK) << AX_TXD_TXPKTSIZE_SH;
		wd_words[0] = cpu_to_le32(w0);

		if (mdata->hw_seq_mode == 0) {
			w9 |= (mdata->sw_seq & AX_TXD_HW_SSN_SEL_MSK) << AX_TXD_HW_SSN_SEL_SH;
			wd_words[9] = cpu_to_le32(w9);
		} else {
			w8 |= AX_TXD_EN_HWSEQ;
			wd_words[8] = cpu_to_le32(w8);
		}

		w3 |= (mdata->hw_ssn_sel & AX_TXD_HW_SSN_SEL_MSK) << AX_TXD_HW_SSN_SEL_SH;
		wd_words[3] = cpu_to_le32(w3);

		if (mdata->ampdu_en) {
			w2 |= AX_TXD_AGG_EN;
		}

		if (mdata->bk || mdata->ack_ch_info) {
			w2 |= AX_TXD_BK;
		}

		w2 |= (qsel_tbl[mdata->tid] & AX_TXD_QSEL_MSK) << AX_TXD_QSEL_SH;
		wd_words[2] = cpu_to_le32(w2);

		*wd_len = req->wd_len;
	}

	return RTW_HAL_STATUS_SUCCESS;
}
#endif

/**
 * the function update wd page, including wd info, wd body, seq info, addr info
 * @hal: see struct hal_info_t
 * @phl_pkt_req: see struct rtw_phl_pkt_req
 */
static enum rtw_hal_status
hal_update_wd_8730ea(struct hal_info_t *hal,
		     struct rtw_phl_pkt_req *req) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;
	struct rtw_xmit_req *tx_req = NULL;
	struct rtw_pkt_buf_list *pkt_list = NULL;
	u32 wd_len = 0, seq_ofst = 0, addr_info_ofst = 0;
	u16 wp_seq = 0;
	u8 i = 0, wp_num = 0, mpdu_ls = 0, msdu_ls = 0, tid_indic = 0;
	u8 buf_len = 0;
	FUNCIN_WSTS(hstatus);
	do
	{
		if (NULL == req) {
			break;
		}

		tx_req = req->tx_req;
		pkt_list = (struct rtw_pkt_buf_list *)tx_req->pkt_list;

#ifdef CONFIG_PHL_TXSC
		_hal_txsc_update_wd(hal, req, &wd_len);
#else
		/* connect with halmac */
		rtw_hal_mac_ax_fill_txdesc(hal->mac, tx_req, req->wd_page,
					   &wd_len);
#endif
		tid_indic = _hal_get_tid_indic_8730ea(tx_req->mdata.tid);

		buf_len += (u8)wd_len;
		req->total_len = wd_len;
		req->seg_num = 0;
		addr_info_ofst = wd_len;

		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_, "after halmac update wd, wd_len = 0x%x, seq_info_size = 0x%x\n", wd_len, bus_hw_cap->seq_info_size);

		for (i = 0; i < tx_req->mdata.addr_info_num; i++) {
			if (0 == i) {
				wp_num = tx_req->mdata.addr_info_num;
			} else {
				wp_num = 0;
			}

			if ((tx_req->mdata.addr_info_num - 1) == i) {
				msdu_ls = 1;
			} else {
				msdu_ls = 0;
			}

			if (i == tx_req->mdata.addr_info_num - 1) {
				_hal_fill_wp_addr_info_8730ea(hal_com,
							      req->wd_page + addr_info_ofst,
							      &pkt_list[i], wp_num, mpdu_ls, msdu_ls, true);
			} else {
				_hal_fill_wp_addr_info_8730ea(hal_com,
							      req->wd_page + addr_info_ofst,
							      &pkt_list[i], wp_num, mpdu_ls, msdu_ls, false);
			}

			addr_info_ofst += bus_hw_cap->addr_info_size;
			buf_len += bus_hw_cap->addr_info_size;
			req->total_len += pkt_list[i].length;
			req->seg_num++;
		}
		req->wd_len = buf_len;
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_, "req->wd_len = 0x%x\n", req->wd_len);
	} while (false);

#if 0 /* remove this for saving cpu cycle */
	if (RTW_HAL_STATUS_SUCCESS == hstatus)
	{
		debug_dump_data(req->wd_page, (u16)addr_info_ofst, "dump wd page");
	}
#endif
	FUNCOUT_WSTS(hstatus);
	return hstatus;
}

/**
 * the function update txbd
 * @hal: see struct hal_info_t
 * @txbd_ring: the target txbd ring buffer going to update, see struct tx_base_desc
 * @wd: the wd page going to be filled in txbd, see struct rtw_wd_page
 */
static enum rtw_hal_status
hal_update_txbd_8730ea(struct hal_info_t *hal,
		       struct tx_base_desc *txbd_ring,
		       struct rtw_wd_page *wd_page,
		       u8 ch_idx, u16 wd_num) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;
	u8 *ring_head = 0;
	int i = 0;
	u8 *target_txbd = 0, *add_info = 0;
	u16 host_idx = 0, txbd_host_idx = 0, hw_idx = 0;
	u16 avail_txbd = 0, wcnt = 0;
	u16 txbd_num = (u16)hal_com->bus_cap.txbd_num;
	u16 seg_num = ((TX_BUFFER_SEG_NUM == 0) ? 2 : ((TX_BUFFER_SEG_NUM == 1) ? 4 : 8));
	int sub_seg_num = 0;

	do
	{
		if (NULL == wd_page) {
			break;
		}
		if (NULL == txbd_ring) {
			break;
		}

		/* connect with halmac */
		host_idx = txbd_ring[ch_idx].host_idx;
		avail_txbd = hal_get_avail_txbd_8730ea(hal->hal_com, ch_idx,
						       &txbd_host_idx, &hw_idx);

		wcnt = (wd_num > avail_txbd) ? avail_txbd : wd_num;

		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "hal_update_txbd_8730ea => ch_idx %d, host_idx %d, "
			  "hw_idx %d, avail_txbd %d, wcnt %d\n",
			  ch_idx, txbd_host_idx, hw_idx, avail_txbd, wcnt);

		while (wcnt > 0) {

			ring_head = txbd_ring[ch_idx].vir_addr;
			target_txbd = ring_head + (host_idx *
						   bus_hw_cap->txbd_len);

			SET_TXBUFFER_DESC_LEN(target_txbd, 0);
			SET_TXBUFFER_DESC_PSB(target_txbd, 0);
			SET_TXBUFFER_DESC_ADD_LOW(target_txbd, 0);

			for (i = 1 ; i <= seg_num ; i++) {
				SET_TXBUFFER_DESC_LEN_WITH_OFFSET(target_txbd, i, 0);
				SET_TXBUFFER_DESC_MODE_WITH_OFFSET(target_txbd, i, 0);
				SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(target_txbd, i, 0);
			}

			SET_TXBUFFER_DESC_LEN(target_txbd, TX_WIFI_INFO_SIZE);
			SET_TXBUFFER_DESC_PSB(target_txbd, RTW_DIV_ROUND_UP(wd_page->total_len, TXBD_PAGE_SIZE));
			SET_TXBUFFER_DESC_ADD_LOW(target_txbd, wd_page->phy_addr_l);

			add_info = wd_page->phy_addr_l + TX_WIFI_INFO_SIZE;
			sub_seg_num = (int)(wd_page->seg_num);

			SET_TXBUFFER_DESC_MODE_WITH_OFFSET(target_txbd, 1, 1);
			SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(target_txbd, 1, add_info);
			SET_TXBUFFER_DESC_LEN_WITH_OFFSET(target_txbd, 1, sub_seg_num * 8);

			PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_, "wd_page->buf_len = 0x%x\n", wd_page->buf_len);
			PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_, "wd_page->phy_addr_l = 0x%x\n", wd_page->phy_addr_l);

			host_idx = (host_idx + 1) % txbd_num;
			wd_page->host_idx = host_idx;
			wcnt--;
			//multi wd page in one update txbd
#if 0//S_TODO
			if (wcnt > 0) {
				wd_page = list_first_entry(wd_page->list,
							   struct rtw_wd_page,
							   wd_page->list);
				if (NULL == wd_page) {
					break;
				}
			}
#endif
		}

		txbd_ring[ch_idx].host_idx = host_idx;
	} while (false);

	return hstatus;
}

/**
 * the function trigger tx start
 * @hal: see struct hal_info_t
 * @txbd_ring: the target txbd ring buffer going to update, see struct tx_base_desc
 * @ch_idx: the dma channel index of this txbd_ring
 */
static enum rtw_hal_status
hal_trigger_txdma_8730ea(struct hal_info_t *hal,
			 struct tx_base_desc *txbd_ring, u8 ch_idx) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	u8 tx_dma_ch;
	u32 txbd_reg;

	do
	{
		/* connect with halmac */
		tx_dma_ch = BE_QUEUE_IDX_8730EA + ch_idx;
		txbd_reg = _hal_get_bd_idx_reg_8730ea(tx_dma_ch);
		if (0xFFFF == txbd_reg) {
			break;
		}

		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "hal_trigger_txdma_8730ea => dma_ch %d, host_idx %d.\n",
			  ch_idx, txbd_ring[ch_idx].host_idx);
		hal_write16(hal->hal_com, txbd_reg, txbd_ring[ch_idx].host_idx);
		hstatus = RTW_HAL_STATUS_SUCCESS;
	} while (false);

	return hstatus;
}


static enum rtw_hal_status hal_pltfm_tx_8730ea(void *hal, struct rtw_h2c_pkt *h2c_pkt)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
#if 0

	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct rtw_wd_page wd_page;
	struct tx_base_desc *txbd_ring = NULL;

	_os_mem_set(hal_to_drvpriv(hal_info), &wd_page, 0, sizeof(wd_page));

	txbd_ring = (struct tx_base_desc *)hal_info->hal_com->fw_txbd;
	wd_page.vir_addr = h2c_pkt->vir_addr;
	wd_page.phy_addr_l = h2c_pkt->phy_addr_l;
	wd_page.phy_addr_h = h2c_pkt->phy_addr_h;
	wd_page.buf_len = h2c_pkt->buf_len;
	wd_page.cache = 1;

	_os_spinlock(hal_to_drvpriv(hal_info), &txbd_ring[FWCMD_QUEUE_IDX_8730EA].txbd_lock, _ps, NULL);
	hstatus = hal_update_txbd_8730ea(hal_info, txbd_ring, &wd_page,
					 FWCMD_QUEUE_IDX_8730EA, 1);
	_os_spinunlock(hal_to_drvpriv(hal_info), &txbd_ring[FWCMD_QUEUE_IDX_8730EA].txbd_lock, _ps, NULL);

	/* enqueue busy queue */

	hstatus = hal_trigger_txdma_8730ea(hal_info, txbd_ring, FWCMD_QUEUE_IDX_8730EA);
#endif
	return hstatus;
}

static u8 hal_check_rxrdy_8730ea(struct rtw_phl_com_t *phl_com, u8 *rxbd_info,
				 u8 ch_idx)
{
	struct hal_spec_t *hal_spec = &phl_com->hal_spec;
	u8 res = true;
	u8 first_seg = 0, last_seg = 0;
	u16 total_len = 0, read_cnt = 0;
	do {
		if (rxbd_info == NULL) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				  "[WARNING] input rx bd info is NULL!\n");
			res = false;
			break;
		}

		total_len = (u16)GET_RX_BD_INFO_TOTALRXPKTSIZE(rxbd_info);
		first_seg = (u8)GET_RX_BD_INFO_FS(rxbd_info);
		last_seg = (u8)GET_RX_BD_INFO_LS(rxbd_info);

		while (total_len == 0) {
			read_cnt++;

			total_len = (u16)GET_RX_BD_INFO_TOTALRXPKTSIZE(rxbd_info);
			first_seg = (u8)GET_RX_BD_INFO_FS(rxbd_info);
			last_seg = (u8)GET_RX_BD_INFO_LS(rxbd_info);

			if (read_cnt > 10000) {
				res = false;
				break;
			}
		}

		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "RX:%s exit total_len=%d, rx_tag = %d, first_seg=%d, last_seg=%d, read_cnt %d\n",
			  __func__, total_len, hal_spec->rx_tag[ch_idx],
			  first_seg, last_seg, read_cnt);

	} while (false);

	return res;
}

static void _hal_show_tx_failure_rsn_8730ea(u8 txsts)
{

	switch (txsts) {

	case TX_STATUS_TX_FAIL_REACH_RTY_LMT:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "this wp is tx fail (REACH_RTY_LMT)\n");
		break;
	case TX_STATUS_TX_FAIL_LIFETIME_DROP:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "this wp is tx fail (LIFETIME_DROP)\n");
		break;
	case TX_STATUS_TX_FAIL_MACID_DROP:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "this wp is tx fail (MACID_DROP)\n");
		break;
	case TX_STATUS_TX_FAIL_SW_DROP:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "this wp is tx fail (SW_DROP)\n");
		break;
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "this wp is tx fail (UNKNOWN)\n");
		break;
	}

}

/**
 * Process Rx PPDU Status with HALMAC API and PHYDM API
 */
static enum rtw_hal_status
hal_handle_ppdusts_8730ea(void *hal, u8 *psbuf, u16 sz,
			  struct rtw_r_meta_data *mdata, struct rx_ppdu_status *rxps) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	if (mdata->mac_info_vld)
	{
		/*Call HALMAC API HALMAC API*/
		rxps->mac_info_length = 4; //To DO
	}

	rxps->phy_info_length = sz - rxps->mac_info_length;
	if (rxps->phy_info_length > 0)
	{
		/* the remaining length > 4  the phy info is valid */
		/* CALL PHYDM API Here*/
		//rx_desc->mac_id

	}
	hstatus = RTW_HAL_STATUS_SUCCESS;
	return hstatus;
}

/**
 * SW Parsing Rx Desc
 **/
static enum rtw_hal_status
_hal_parsing_rx_wd_8730ea(struct hal_info_t *hal, u8 *desc,
			  struct rtw_r_meta_data *mdata) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	hstatus = RTW_HAL_STATUS_SUCCESS;
	return hstatus;
}

static u8 hal_handle_rxbd_info_8730ea(struct hal_info_t *hal,
				      u8 *rxbd_info, u16 *size)
{
	u8 res = false;
	u16 pld_size = 0;
	u8 fs = 0, ls = 0;
	u8 pkt_rdy = false;

	do {
		if (NULL == rxbd_info) {
			break;
		}
		if (NULL == size) {
			break;
		}

		fs = (u8)GET_RX_BD_INFO_FS(rxbd_info);
		ls = (u8)GET_RX_BD_INFO_LS(rxbd_info);
		pld_size = (u16)GET_RX_BD_INFO_HW_W_SIZE(rxbd_info);

		if (fs == 1) {
			if (ls == 1) {
				pkt_rdy = true;
			} else {
				pkt_rdy = false;
			}

		} else if (fs == 0) {
			if (ls == 1) {
				pkt_rdy = false;
			} else {
				pkt_rdy = false;
			}
		}

		if (pkt_rdy) {
			*size = pld_size;
			res = true;
		} else {
			*size = 0;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING] need to handle RX FS/LS\n");
			res = false;
		}
	} while (false);
	return res;
}



/**
 * the function update rxbd
 */
static enum rtw_hal_status
hal_update_rxbd_8730ea(struct hal_info_t *hal, struct rx_base_desc *rxbd,
		       struct rtw_rx_buf *rx_buf) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;
	u8 *ring_head = NULL;
	u8 *target_rxbd = NULL;
	u16 rxbd_num = (u16)hal_com->bus_cap.rxbd_num;

	do
	{
		if (NULL == rxbd) {
			break;
		}
		if (NULL == rx_buf) {
			break;
		}

		ring_head = rxbd->vir_addr;
		target_rxbd = ring_head + (rxbd->host_idx *
					   bus_hw_cap->rxbd_len);
		/* connect with halmac */
		SET_RX_BD_INFO_FS(target_rxbd, 0);
		SET_RX_BD_INFO_LS(target_rxbd, 0);
		SET_RX_BD_RXBUFFSIZE(target_rxbd, rx_buf->buf_len);
		SET_RX_BD_PHYSICAL_ADDR_LOW(target_rxbd,
					    (u32)rx_buf->phy_addr_l);

		rxbd->host_idx = (rxbd->host_idx + 1) % rxbd_num;
	} while (false);

	return hstatus;
}


/**
 * the function notify rx done
 */
static enum rtw_hal_status
hal_notify_rxdone_8730ea(struct hal_info_t *hal,
			 struct rx_base_desc *rxbd, u8 ch, u16 rxcnt) {
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	u32 reg = 0;
	u8 rx_dma_ch = 0;

	do
	{
		rx_dma_ch = RX_QUEUE_IDX_8730EA + ch;
		reg = _hal_get_bd_idx_reg_8730ea(rx_dma_ch);
		/* connect with halmac */
		if (0xFFFF == reg) {
			break;
		}
		hal_write16(hal->hal_com, reg, rxbd->host_idx);
		hstatus = RTW_HAL_STATUS_SUCCESS;
	} while (false);

	return hstatus;
}

static void _hal_trx_init_bd_num_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u16 txbd_num = (u16)hal_com->bus_cap.txbd_num;
	u16 rxbd_num = (u16)hal_com->bus_cap.rxbd_num;
	u32 reg = 0;
	u16 value = 0;
	u8 i = 0;

	for (i = 0; i < TX_DMA_CHANNEL_ENTRY_8730EA; i++) {
		if ((TXCMD_QUEUE_IDX_8730EA == i) \
		    || (BCN_QUEUE_IDX_8730EA == i)) {
			continue;
		}

		value = ((RTL8730AE_SEG_NUM & 0x3) << 12)
			| (txbd_num & 0x3F);

		reg = _hal_get_bd_num_reg_8730ea(i);
		if (0 != reg) {
			hal_write16(hal->hal_com, reg, value);
		} else {
			PHL_ERR("query txbd num reg fail (ch_idx = %d)\n", i);
		}
	}

	value = ((RTL8730AE_SEG_NUM & 0x3) << 12)
		| (rxbd_num & 0x3F);

	reg = _hal_get_bd_num_reg_8730ea(RX_QUEUE_IDX_8730EA);
	if (0 != reg) {
		hal_write16(hal->hal_com, reg, value);
	} else {
		PHL_ERR("query rxbd num reg fail (ch_idx = %d)\n",\
			RX_QUEUE_IDX_8730EA);
	}

	hal_write32(hal->hal_com, REG_BD_RWPTR_CLR, 0xFFFFFFFF);
}


static void _hal_trx_init_bd_8730ea(struct hal_info_t *hal, u8 *txbd_buf, u8 *rxbd_buf)
{
	struct tx_base_desc *txbd = NULL;
	struct rx_base_desc *rxbd = NULL;
	u32 reg_addr_l = 0, reg_addr_h = 0, value32 = 0;
	u8 i = 0, rxch_idx = 0;

	/* Disable Stop RX DMA */
	value32 = hal_read32(hal->hal_com, REG_AXI_CTRL);
	value32 &= ~BIT_STOPRX;
	hal_write32(hal->hal_com, REG_AXI_CTRL, value32);

	/* Clear the pending ISR */
	hal_write32(hal->hal_com, REG_AXI_INTERRUPT, 0xFFFFFFFF);

	if (NULL != txbd_buf && NULL != rxbd_buf) {
		txbd = (struct tx_base_desc *)txbd_buf;
		rxbd = (struct rx_base_desc *)rxbd_buf;

		/* data queue + mgnt queue + high queue */
		for (i = 0; i < TX_DMA_CHANNEL_ENTRY_8730EA; i++) {
			if (TXCMD_QUEUE_IDX_8730EA == i) {
				continue;
			}

			switch (i) {
			case MGT_QUEUE_IDX_8730EA:
				value32 = BIT_MGQDESA(txbd[i].phy_addr_l);
				break;
			case VO_QUEUE_IDX_8730EA:
				value32 = BIT_VOQDESA(txbd[i].phy_addr_l);
				break;
			case VI_QUEUE_IDX_8730EA:
				value32 = BIT_VIQDESA(txbd[i].phy_addr_l);
				break;
			case BE_QUEUE_IDX_8730EA:
				value32 = BIT_BEQDESA(txbd[i].phy_addr_l);
				break;
			case BK_QUEUE_IDX_8730EA:
				value32 = BIT_BKQDESA(txbd[i].phy_addr_l);
				break;
			case HIGH_QUEUE_IDX_8730EA:
				value32 = BIT_MQ0DESA(txbd[i].phy_addr_l);
				break;
			case BCN_QUEUE_IDX_8730EA:
				value32 = BIT_BPQDESA(txbd[i].phy_addr_l);
				break;
			default:
				PHL_ERR("not support channel (ch_idx = %d)\n", i);
				break;
			}

			_hal_get_bd_desc_reg_8730ea(i, &reg_addr_l, &reg_addr_h);
			if (0 != reg_addr_l) {
				hal_write32(hal->hal_com, reg_addr_l, value32);
			} else {
				PHL_ERR("query txbd desc reg_addr_l fail (ch_idx = %d)\n", i);
			}
		}

		/* RX queue */
		value32 = BIT_RXNRML_DESA(rxbd[rxch_idx].phy_addr_l);
		_hal_get_bd_desc_reg_8730ea(RX_QUEUE_IDX_8730EA, &reg_addr_l, &reg_addr_h);
		if (0 != reg_addr_l) {
			hal_write32(hal->hal_com, reg_addr_l, value32);
		} else {
			PHL_ERR("query rxbd desc reg_addr_l fail (ch_idx = %d)\n", i);
		}
	}

	value32 = hal_read32(hal->hal_com, REG_AXI_CTRL);
	value32 |= BIT_MAX_RXDMA(0x03) | BIT_MAX_TXDMA(0x03);
	/* enable total packet length int rxbd[16:31] */
	value32 &= ~BIT_EN_WT_RXTAG;
	hal_write32(hal->hal_com, REG_AXI_CTRL, value32);
}

static void hal_cfg_wow_txdma_8730ea(struct hal_info_t *hal, u8 en)
{

}
static u8 hal_poll_txdma_idle_8730ea(struct hal_info_t *hal)
{
	return true;
}

static bool hal_check_tx_done_8730ea(struct hal_info_t *hal, u8 dma_ch)
{
	bool ret = false;

	switch (dma_ch) {
	case MGT_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_MGNTDOK_8730E) {
			hal->txdone_ch_map &= ~IMR_MGNTDOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case VO_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_VODOK_8730E) {
			hal->txdone_ch_map &= ~IMR_VODOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case VI_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_VIDOK_8730E) {
			hal->txdone_ch_map &= ~IMR_VIDOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case BE_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_BEDOK_8730E) {
			hal->txdone_ch_map &= ~IMR_BEDOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case BK_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_BKDOK_8730E) {
			hal->txdone_ch_map &= ~IMR_BKDOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case HIGH_QUEUE_IDX_8730EA:
		if (hal->txdone_ch_map & IMR_H0DOK_8730E) {
			hal->txdone_ch_map &= ~IMR_H0DOK_8730E;
			ret = true;
		} else {
			ret = false;
		}
		break;
	case BCN_QUEUE_IDX_8730EA:
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown channel (%d)\n",
			  dma_ch);
		ret = false;
		break;
	}

	return ret;
}

/**
 * the function will deinitializing 8730ea specific data and hw configuration
 */
static void hal_trx_deinit_8730ea(struct hal_info_t *hal)
{
	/*struct rtw_hal_com_t *hal_com = hal->hal_com;*/
}

/**
 * the function will initializing 8730ea specific data and hw configuration
 */
static enum rtw_hal_status hal_trx_init_8730ea(struct hal_info_t *hal, u8 *txbd_buf, u8 *rxbd_buf)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	/* Set AMPDU max agg number to 128 */
	/* CR setting*/
	rtw_hal_mac_set_hw_ampdu_cfg(hal, 0, 0x7F, 0xAB);

	_hal_trx_init_bd_8730ea(hal, txbd_buf, rxbd_buf);

	_hal_trx_init_bd_num_8730ea(hal);

	return hstatus;
}

static struct hal_trx_ops ops = {0};
void hal_trx_ops_init_8730ea(void)
{
	ops.init = hal_trx_init_8730ea;
	ops.deinit = hal_trx_deinit_8730ea;
	ops.query_trxring_num = hal_get_trxring_num_8730ea;
	ops.query_tx_res = hal_get_avail_txbd_8730ea;
	ops.query_rx_res = hal_get_avail_rxbd_8730ea;
	ops.cfg_wow_txdma = hal_cfg_wow_txdma_8730ea;
	ops.poll_txdma_idle = hal_poll_txdma_idle_8730ea;
	ops.map_hw_tx_chnl = hal_mapping_hw_tx_chnl_8730ea;
	ops.qsel_to_tid = hal_qsel_to_tid_8730ea;
	ops.query_txch_num = hal_query_txch_num_8730ea;
	ops.query_rxch_num = hal_query_rxch_num_8730ea;
	ops.update_wd = hal_update_wd_8730ea;
	ops.update_txbd = hal_update_txbd_8730ea;
	ops.tx_start = hal_trigger_txdma_8730ea;
	ops.check_rxrdy = hal_check_rxrdy_8730ea;
	ops.check_tx_done = hal_check_tx_done_8730ea;
	ops.handle_rxbd_info = hal_handle_rxbd_info_8730ea;
	ops.handle_rx_buffer = hal_handle_rx_buffer_8730e;
	ops.update_rxbd = hal_update_rxbd_8730ea;
	ops.notify_rxdone = hal_notify_rxdone_8730ea;
}



u32 hal_hook_trx_ops_8730ea(struct rtw_phl_com_t *phl_com,
			    struct hal_info_t *hal_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	if (NULL != hal_info) {
		hal_trx_ops_init_8730ea();
		hal_info->trx_ops = &ops;
		hstatus = RTW_HAL_STATUS_SUCCESS;
	}

	return hstatus;
}
