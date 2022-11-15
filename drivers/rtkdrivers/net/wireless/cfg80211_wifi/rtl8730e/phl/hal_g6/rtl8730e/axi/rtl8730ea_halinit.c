/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation.
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
#define _RTL8730EA_HALINIT_C_
#include "../../hal_headers.h"
#include "../rtl8730e_hal.h"
#include "hal_trx_8730ea.h"
static void _hal_pre_init_8730ea(struct rtw_phl_com_t *phl_com,
				 struct hal_info_t *hal_info,
				 struct hal_init_info_t *init_30ea)
{
	struct mac_ax_trx_info *trx_info = &init_30ea->trx_info;
	struct mac_ax_intf_info *intf_info = &init_30ea->intf_info;
	struct mac_ax_host_rpr_cfg *rpr_cfg = (struct mac_ax_host_rpr_cfg *)hal_info->rpr_cfg;
	void *txbd_buf = NULL;
	void *rxbd_buf = NULL;

	/* trx_info */
	if (true == phl_com->dev_cap.tx_mu_ru) {
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	} else {
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;
	}

	if (hal_info->hal_com->dbcc_en == false) {
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	} else {
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	}

#ifdef RTW_WKARD_LAMODE
	PHL_INFO("%s : la_mode %d\n", __func__,	phl_com->dev_cap.la_mode);
	if (phl_com->dev_cap.la_mode) {
		trx_info->qta_mode = MAC_AX_QTA_LAMODE;
	}
#endif

	if (phl_com->dev_cap.rpq_agg_num) {
		rpr_cfg->agg_def = 0;
		rpr_cfg->agg = phl_com->dev_cap.rpq_agg_num;
	} else {
		rpr_cfg->agg_def = 1;
	}

	rpr_cfg->tmr_def = 1;
	rpr_cfg->txok_en = MAC_AX_FUNC_DEF;
	rpr_cfg->rty_lmt_en = MAC_AX_FUNC_DEF;
	rpr_cfg->lft_drop_en = MAC_AX_FUNC_DEF;
	rpr_cfg->macid_drop_en = MAC_AX_FUNC_DEF;

	trx_info->rpr_cfg = rpr_cfg;

	/* intf_info */
	txbd_buf = rtw_phl_get_txbd_buf(phl_com);
	rxbd_buf = rtw_phl_get_rxbd_buf(phl_com);

	intf_info->txbd_trunc_mode = MAC_AX_BD_TRUNC;
	intf_info->rxbd_trunc_mode = MAC_AX_BD_TRUNC;

	intf_info->rxbd_mode = MAC_AX_RXBD_PKT;
	intf_info->tag_mode = MAC_AX_TAG_MULTI;
	intf_info->tx_burst = MAC_AX_TX_BURST_DEF;
	intf_info->rx_burst = MAC_AX_RX_BURST_DEF;
	intf_info->wd_dma_idle_intvl = MAC_AX_WD_DMA_INTVL_DEF;
	intf_info->wd_dma_act_intvl = MAC_AX_WD_DMA_INTVL_DEF;
	intf_info->multi_tag_num = MAC_AX_TAG_NUM_DEF;
	intf_info->rx_sep_append_len = 0;
	intf_info->txbd_buf = txbd_buf;
	intf_info->rxbd_buf = rxbd_buf;
	intf_info->skip_all = false;

	//intf_info->txch_map = (struct mac_ax_txdma_ch_map *)hal_info->txch_map;

	/* others */
	init_30ea->ic_name = "rtl8730ea";
}

void init_hal_spec_8730ea(struct rtw_phl_com_t *phl_com,
			  struct hal_info_t *hal)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;

	init_hal_spec_8730e(phl_com, hal);
	hal_spec->rx_bd_info_sz = RX_BD_INFO_SIZE;
	hal_spec->rx_tag[0] = 0;
	hal_spec->rx_tag[1] = 0;

	bus_hw_cap->max_txbd_num = 32;
	bus_hw_cap->max_rxbd_num = 32;
	bus_hw_cap->max_rpbd_num = 0;

	bus_hw_cap->max_wd_page_size = 128;
	bus_hw_cap->txbd_len = ((TX_BUFFER_SEG_NUM == 0) ? 16 : ((TX_BUFFER_SEG_NUM == 1) ? 32 : 64));
	bus_hw_cap->rxbd_len = 8;
	bus_hw_cap->addr_info_size = 8;
	bus_hw_cap->seq_info_size = 8;
	/* phyaddr num = (wd page - (wdb + wdi + seqinfo)) / addrinfo */
#ifdef RTW_WKARD_BUSCAP_IN_HALSPEC
	hal_spec->phyaddr_num = 9;
#endif
}

enum rtw_hal_status hal_get_efuse_8730ea(struct rtw_phl_com_t *phl_com,
		struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_30ea;
	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));
	_hal_pre_init_8730ea(phl_com, hal_info, &init_30ea);

	return hal_get_efuse_8730e(phl_com, hal_info, &init_30ea);
}

enum rtw_hal_status hal_init_8730ea(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;
	void *drv = phlcom_to_drvpriv(phl_com);
	u32 len = 0;
	u8 txch_num = 0;

	txch_num = rtw_hal_query_txch_num(hal_info);

	len = sizeof(struct rtw_wp_rpt_stats) * txch_num;
	hal_com->trx_stat.wp_rpt_stats = _os_mem_alloc(drv, len);
	if (hal_com->trx_stat.wp_rpt_stats == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc wp_rpt_stats failed\n", __func__);
		goto error_trx_stats_wp_rpt;
	}

	_os_mem_set(hal_to_drvpriv(hal_info), hal_info->txdone_ch_map, 0,
		    4 * sizeof(u32));
	hal_info->tx_dma_ch_map[0] = (BIT_TXBCN1_ERR_INT | BIT_TXBCN1_OK_INT);
	hal_info->tx_dma_ch_map[1] = IMR_TX_MASK;
	hal_info->tx_dma_ch_map[2] = (BIT_BCNERLY0_INT
#ifdef CONFIG_CONCURRENT_MODE
				      | BIT_TXBCNERR9_INT
				      | BIT_TXBCNOK9_INT
				      | BIT_BCNERLY8_INT
#endif
				      );

	hal_info->rpr_cfg = _os_mem_alloc(drv,
					  sizeof(struct mac_ax_host_rpr_cfg));
	if (hal_info->rpr_cfg == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc rpr_cfg failed\n", __func__);
		goto error_rpr_cfg;
	}

	hal_status = RTW_HAL_STATUS_SUCCESS;

	return hal_status;

error_rpr_cfg:
	_os_mem_free(drv,
		     hal_com->trx_stat.wp_rpt_stats,
		     sizeof(struct rtw_wp_rpt_stats) * txch_num);
error_trx_stats_wp_rpt:
	return hal_status;
}

void hal_deinit_8730ea(struct rtw_phl_com_t *phl_com,
		       struct hal_info_t *hal_info)
{
	u8 txch_num = 0;

	txch_num = rtw_hal_query_txch_num(hal_info);

	_os_mem_set(hal_to_drvpriv(hal_info), hal_info->txdone_ch_map, 0,
		    4 * sizeof(u32));
	_os_mem_set(hal_to_drvpriv(hal_info), hal_info->tx_dma_ch_map, 0,
		    4 * sizeof(u32));
	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->hal_com->trx_stat.wp_rpt_stats,
		     sizeof(struct rtw_wp_rpt_stats) * txch_num);
	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->rpr_cfg,
		     sizeof(struct mac_ax_host_rpr_cfg));
}

enum rtw_hal_status hal_start_8730ea(struct rtw_phl_com_t *phl_com,
				     struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_30ea;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));
	_hal_pre_init_8730ea(phl_com, hal_info, &init_30ea);

	hal_status = hal_start_8730e(phl_com, hal_info, &init_30ea);
	if (RTW_HAL_STATUS_SUCCESS != hal_status) {

		PHL_ERR("hal_init_8852c: status = %u\n", hal_status);
		return hal_status;
	}

	return hal_status;
}

enum rtw_hal_status hal_stop_8730ea(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = hal_stop_8730e(phl_com, hal);
	return hal_status;
}

#ifdef CONFIG_WOWLAN

enum rtw_hal_status
hal_wow_init_8730ea(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
		    struct rtw_phl_stainfo_t *sta) {
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_30ea;
	struct mac_ax_trx_info *trx_info = &init_30ea.trx_info;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));
	if (true == phl_com->dev_cap.tx_mu_ru)
	{
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	} else
	{
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;
	}
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_30ea.ic_name = "rtl8730ea";

	hal_status = hal_wow_init_8730e(phl_com, hal_info, sta, &init_30ea);

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

enum rtw_hal_status
hal_wow_deinit_8730ea(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
		      struct rtw_phl_stainfo_t *sta) {
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_30ea;
	struct mac_ax_trx_info *trx_info = &init_30ea.trx_info;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));
	if (true == phl_com->dev_cap.tx_mu_ru)
	{
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	} else
	{
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;
	}
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_30ea.ic_name = "rtl8730ea";

	hal_status = hal_wow_deinit_8730e(phl_com, hal_info, sta, &init_30ea);

	if (RTW_HAL_STATUS_SUCCESS != hal_status)
	{

		PHL_ERR("hal_wow_deinit_8730ea: status = %u\n", hal_status);
		return hal_status;
	}

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

#endif /* CONFIG_WOWLAN */

u32 hal_hci_cfg_8730ea(struct rtw_phl_com_t *phl_com,
		       struct hal_info_t *hal, struct rtw_ic_info *ic_info)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	hal_spec->cts2_thres_en = true;
	hal_spec->cts2_thres = 1792;

	hal_spec->txbd_multi_tag = 8;
#ifdef RTW_WKARD_TXBD_UPD_LMT
	hal_spec->txbd_upd_lmt = true;
#else
	hal_spec->txbd_upd_lmt = false;
#endif
	return RTW_HAL_STATUS_SUCCESS;
}

void hal_init_default_value_8730ea(struct hal_info_t *hal, struct hal_intr_mask_cfg *cfg)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	init_default_value_8730e(hal);

	hal_com->int_mask_default[0] = (u32)(
					       BIT_HISR1_INT_EN |
					       BIT_HISR2_INT_EN |
					       BIT_TXFF_FIFO_INT_EN |  /* TXFOVW */
					       BIT_RXFF_FULL_INT_EN |
					       BIT_HCPWM_INT_EN |
					       0);

	hal_com->int_mask_default[1] = (u32)(
					       IMR_ROK_8730E |
					       /* IMR_RDU_8730E | */
					       IMR_VODOK_8730E |
					       IMR_VIDOK_8730E |
					       IMR_BEDOK_8730E |
					       IMR_BKDOK_8730E |
					       IMR_MGNTDOK_8730E |
					       /* IMR_H2COK_8730E | */
					       IMR_H0DOK_8730E |
					       0);

	hal_com->int_mask_default[2] = 0;
	hal_com->int_mask_default[3] = 0;


	/*
	 * Set default value of Interrupt Mask Register3
	 */
	hal_com->int_mask[0] = hal_com->int_mask_default[0];
	hal_com->int_mask[1] = hal_com->int_mask_default[1];
	hal_com->int_mask[2] = hal_com->int_mask_default[2];
	hal_com->int_mask[3] = hal_com->int_mask_default[3];

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : %08X, %08X, %08x, %08x\n", __func__,
		  hal_com->int_mask[0], hal_com->int_mask[1], hal_com->int_mask[2],
		  hal_com->int_mask[3]);
}

void hal_enable_int_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, REG_HIMR0, hal_com->int_mask[0]);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, hal_com->int_mask[1]);
	hal_write32(hal_com, REG_HEMR, hal_com->int_mask[2]);
	hal_write32(hal_com, REG_HIMR2, hal_com->int_mask[3]);

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : \n%04X : %08X\n%04X : %08X\n%04X : %08X\n%04X : %08X\n",
		  __func__,
		  REG_HIMR0, hal_read32(hal_com, REG_HIMR0),
		  REG_AXI_INTERRUPT_MASK, hal_read32(hal_com, REG_AXI_INTERRUPT_MASK),
		  REG_HEMR, hal_read32(hal_com, REG_HEMR),
		  REG_HIMR2, hal_read32(hal_com, REG_HIMR2)
		 );
}

void hal_disable_int_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, REG_HIMR0, 0);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, 0);
	hal_write32(hal_com, REG_HEMR, 0);
	hal_write32(hal_com, REG_HIMR2, 0);
}

void hal_config_int_8730ea(struct hal_info_t *hal, enum rtw_phl_config_int int_mode)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u8 index = 0;

	hal_disable_int_8730ea(hal);

	switch (int_mode) {
#ifdef RTW_PHL_BCN_IOT
	case RTW_PHL_EN_AP_MODE_INT:
		hal_com->int_mask[2] |= BIT_BCNERLY0_INT;
#ifdef CONFIG_CONCURRENT_MODE
		hal_com->int_mask[2] |= BIT_BCNERLY8_INT_EN;
#endif
		break;
	case RTW_PHL_DIS_AP_MODE_INT:
		hal_com->int_mask[2] &= ~BIT_BCNERLY0_INT;
#ifdef CONFIG_CONCURRENT_MODE
		hal_com->int_mask[2] &= ~BIT_BCNERLY8_INT_EN;
#endif
		break;
	case RTW_PHL_EN_TX_BCN_INT:
		hal_com->int_mask[0] |= (BIT_TXBCN1_ERR_INT_EN
					| BIT_TXBCN1_OK_INT_EN);
#ifdef CONFIG_CONCURRENT_MODE
		hal_com->int_mask[2] |= (BIT_TXBCNERR9_INT
					| BIT_TXBCNOK9_INT);
#endif
		break;
	case RTW_PHL_DIS_TX_BCN_INT:
		hal_com->int_mask[0] &= ~(BIT_TXBCN1_ERR_INT_EN
					| BIT_TXBCN1_OK_INT_EN);
#ifdef CONFIG_CONCURRENT_MODE
		hal_com->int_mask[2] &= ~(BIT_TXBCNERR9_INT
					 | BIT_TXBCNOK9_INT);
#endif
		break;
#endif
	default:
		PHL_WARN("Not support interrupt mode %d.\n", int_mode);
		break;
	}

	hal_enable_int_8730ea(hal);
}

bool hal_recognize_int_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	bool recognized = false;

#ifndef CONFIG_SYNC_INTERRUPT
	/* disable imr before cleaning isr */
	hal_write32(hal_com, REG_HIMR0, 0);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, 0);
	hal_write32(hal_com, REG_HEMR, 0);
	hal_write32(hal_com, REG_HIMR2, 0);
#endif /* CONFIG_SYNC_INTERRUPT */

	/* save isr for sw usage, handling interrupt */
	/* isr00 */
	hal_com->int_array[0] = hal_read32(hal_com, REG_HISR0);
	hal_com->int_array[0] &= hal_com->int_mask[0];

	/*Interrput form AXI*/
	hal_com->int_array[1] = hal_read32(hal_com, REG_AXI_INTERRUPT);
	hal_com->int_array[1] &= hal_com->int_mask[1];

	/*hesr has to be cleared before hisr0, otherwise hisr0[BIT_HISR1_INT] will be 1 again*/
	if (hal_com->int_array[0] & BIT_HISR1_INT) {
		hal_com->int_array[2] = hal_read32(hal_com, REG_HESR);
		hal_com->int_array[2] &= hal_com->int_mask[2];
	}

	/*hisr2 has to be cleared before hisr0, otherwise hisr0[BIT_HISR2_INT] will be 1 again*/
	if (hal_com->int_array[0] & BIT_HISR2_INT) {
		hal_com->int_array[3] = hal_read32(hal_com, REG_HISR2);
		hal_com->int_array[3] &= hal_com->int_mask[3];
	}

	if (hal_com->int_array[0] || hal_com->int_array[1] || hal_com->int_array[2] || hal_com->int_array[3]) {
		recognized = true;
	}

#ifndef CONFIG_SYNC_INTERRUPT
	/* clear isr */
	hal_write32(hal_com, REG_HISR0, hal_com->int_array[0]);
	hal_write32(hal_com, REG_AXI_INTERRUPT, hal_com->int_array[1]);
	hal_write32(hal_com, REG_HESR, hal_com->int_array[2]);
	hal_write32(hal_com, REG_HISR2, hal_com->int_array[3]);

	/* restore imr */
	hal_write32(hal_com, REG_HIMR0, hal_com->int_mask[0] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, hal_com->int_mask[1] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_HEMR, hal_com->int_mask[2] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_HIMR2, hal_com->int_mask[3] & 0xFFFFFFFF);
#endif /* CONFIG_SYNC_INTERRUPT */
	return recognized;
}

void hal_clear_int_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, REG_HISR0, hal_com->int_array[0]);
	hal_write32(hal_com, REG_AXI_INTERRUPT, hal_com->int_array[1]);
	hal_write32(hal_com, REG_HESR, hal_com->int_array[2]);
	hal_write32(hal_com, REG_HISR2, hal_com->int_array[3]);
}

void hal_restore_int_8730ea(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, REG_HIMR0, hal_com->int_mask[0] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, hal_com->int_mask[1] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_HEMR, hal_com->int_mask[2] & 0xFFFFFFFF);
	hal_write32(hal_com, REG_HIMR2, hal_com->int_mask[3] & 0xFFFFFFFF);
}

static u32 hal_bcn_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->int_array[0] & BIT_TXBCN1_ERR_INT) {
		handled[0] |= BIT_TXBCN1_ERR_INT;
		ret = 1;
	}

	if (hal_com->int_array[0] & BIT_TXBCN1_OK_INT) {
		handled[0] |= BIT_TXBCN1_OK_INT;
		ret = 1;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (hal_com->int_array[2] & BIT_TXBCNERR9_INT) {
		handled[2] |= BIT_TXBCNERR9_INT;
		ret = 1;
	}

	if (hal_com->int_array[2] & BIT_TXBCNOK9_INT) {
		handled[2] |= BIT_TXBCNOK9_INT;
		ret = 1;
	}
#endif

	if (hal_com->int_array[2] & BIT_BCNERLY0_INT) {
		handled[2] |= BIT_BCNERLY0_INT;
		ret = 1;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (hal_com->int_array[2] & BIT_BCNERLY8_INT) {
		handled[2] |= BIT_BCNERLY8_INT;
		ret = 1;
	}
#endif

	return ret;
}

static u32 hal_rx_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	static const u32 rx_handle_irq0 = (BIT_RXFF_FULL_INT_EN);
	static const u32 rx_handle_irq1 = (IMR_ROK_8730E | IMR_RDU_8730E);
	u32	handled0 = (hal_com->int_array[0] & rx_handle_irq0);
	u32	handled1 = (hal_com->int_array[1] & rx_handle_irq1);

	if ((handled0 == 0) && (handled1 == 0)) {
		return ret;
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "RX IRQ REG_HIMR0 : %08X (%08X)\n", handled0, hal_com->int_array[0]);
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "RX IRQ REG_AXI_INTERRUPT_MASK : %08X (%08X)\n", handled1, hal_com->int_array[1]);
	/* Disable RX interrupts, RX tasklet will enable them after processed RX */
	hal_com->int_mask[0] &= ~rx_handle_irq0;
	hal_com->int_mask[1] &= ~rx_handle_irq1;
#ifndef CONFIG_SYNC_INTERRUPT
	hal_write32(hal_com, REG_HIMR0, hal_com->int_mask[0]);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, hal_com->int_mask[1]);
#endif /* CONFIG_SYNC_INTERRUPT */

	handled[0] |= handled0;
	handled[1] |= handled1;
	ret = 1;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "RX IRQ A4 REG_HIMR0: %08X (%08X)\n", handled0, hal_com->int_array[0]);
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "RX IRQ A4 REG_AXI_INTERRUPT_MASK : %08X (%08X)\n", handled1, hal_com->int_array[1]);

	return ret;

}

static u32 hal_tx_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 event0 = 0x0, event1 = 0x0;

	if (hal_com->int_array[0] & BIT_TXFF_FIFO_INT) {
		if (printk_ratelimit()) {
			PHL_WARN("[TXFOVW]\n");
		}
	}

	event1 = hal_com->int_array[1] & (
			 IMR_VODOK_8730E |
			 IMR_VIDOK_8730E |
			 IMR_BEDOK_8730E |
			 IMR_BKDOK_8730E |
			 IMR_MGNTDOK_8730E |
			 IMR_H0DOK_8730E
		 );

	if ((event1 != 0) || (event0 != 0)) {
		ret = 1;
	}

	handled[0] |= event0;
	handled[1] |= event1;

	return ret;
}

static u32 hal_cmd_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

#if 0
	if (hal_com->int_array[1] & IMR_H2COK_8730E) {
		handled[1] |= IMR_H2COK_8730E;
		ret = 1;
	}
#endif
	return ret;
}

static u32 hal_halt_c2h_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
#if 0
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	if (hal_com->intr.halt_c2h_int.intr & B_AX_HALT_C2H_INT_EN) {
		handled[0] |= B_AX_HALT_C2H_INT_EN;
		ret = 1;
	}
#endif
	return ret;
}

static u32 hal_watchdog_timer_handler_8730ea(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
#if 0
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	if (hal_com->intr.watchdog_timer_int.intr & B_AX_WDT_PTFM_INT_EN) {
		handled[1] |= B_AX_WDT_PTFM_INT_EN;
		ret = 1;
	}
#endif
	return ret;
}

u32 hal_int_hdler_8730ea(struct hal_info_t *hal)
{
	u32 int_hdler_msk = 0x0;

	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 handled[4] = {0};
	u32 generalhandled[8] = {0};

	/* <1> beacon related */
	int_hdler_msk |= (hal_bcn_handler_8730ea(hal, handled) << 0);

	/* <2> Rx related */
	int_hdler_msk |= (hal_rx_handler_8730ea(hal, handled) << 1);

	/* <3> Tx related */
	int_hdler_msk |= (hal_tx_handler_8730ea(hal, handled) << 2);

	/* <4> Cmd related */
	int_hdler_msk |= (hal_cmd_handler_8730ea(hal, handled) << 3);

	/*Start General interrupt type*/
	/* <5> Halt C2H related */
	int_hdler_msk |= (hal_halt_c2h_handler_8730ea(hal, generalhandled) << 4);

	/* <6> watchdog timer related */
	int_hdler_msk |= (hal_watchdog_timer_handler_8730ea(hal, generalhandled) << 5);

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : int_hdler_msk = 0x%x\n", __func__, int_hdler_msk);

	if ((hal_com->int_array[0] & (~handled[0]))
	    || (hal_com->int_array[1] & (~handled[1]))
	    || (hal_com->int_array[2] & (~handled[2]))
	    || (hal_com->int_array[3] & (~handled[3]))) {
		/*if (printk_ratelimit()) */
		{
			PHL_WARN("Unhandled ISR => %x, %x, %x, %x\nIMR : %08X\nISR : %08X"
				 "\nIMR : %08X\nISR : %08X\n"
				 "\nIMR : %08X\nISR : %08X\n"
				 "\nIMR : %08X\nISR : %08X\n",
				 (hal_com->int_array[0] & (~handled[0])),
				 (hal_com->int_array[1] & (~handled[1])),
				 (hal_com->int_array[2] & (~handled[2])),
				 (hal_com->int_array[3] & (~handled[3])),
				 hal_read32(hal_com, REG_HIMR0),
				 hal_read32(hal_com, REG_HISR0),
				 hal_read32(hal_com, REG_AXI_INTERRUPT_MASK),
				 hal_read32(hal_com, REG_AXI_INTERRUPT),
				 hal_read32(hal_com, REG_HEMR),
				 hal_read32(hal_com, REG_HESR),
				 hal_read32(hal_com, REG_HIMR2),
				 hal_read32(hal_com, REG_HISR2)
				);
		}
	}

	//General interrupt type not handled.

	return int_hdler_msk;
}


void hal_rx_int_restore_8730ea(struct hal_info_t *hal)
{

	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_com->int_mask[0] |= (BIT_RXFF_FULL_INT_EN);
	hal_com->int_mask[1] |= (IMR_ROK_8730E | IMR_RDU_8730E);
#ifndef CONFIG_SYNC_INTERRUPT
	hal_write32(hal_com, REG_HIMR0, hal_com->int_mask[0]);
	hal_write32(hal_com, REG_AXI_INTERRUPT_MASK, hal_com->int_mask[1]);
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : \n%04X : %08X\n%04X : %08X\n",
		  __func__,
		  REG_HIMR0, hal_read32(hal_com, REG_HIMR0),
		  REG_AXI_INTERRUPT_MASK, hal_read32(hal_com, REG_AXI_INTERRUPT_MASK)
		 );
#endif /* CONFIG_SYNC_INTERRUPT */

}

enum rtw_hal_status
hal_mp_init_8730ea(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info) {
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_30ea;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));

	init_30ea.ic_name = "rtl8730ea";

	hal_status = hal_mp_init_8730e(phl_com, hal_info, &init_30ea);

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

enum rtw_hal_status
hal_mp_deinit_8730ea(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info) {
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_30ea;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_30ea, 0, sizeof(init_30ea));

	init_30ea.ic_name = "rtl8730ea";

	hal_status = hal_mp_deinit_8730e(phl_com, hal_info, &init_30ea);

	if (RTW_HAL_STATUS_SUCCESS != hal_status)
	{

		PHL_ERR("hal_mp_deinit_8730ea: status = %u\n", hal_status);
		return hal_status;
	}

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}
