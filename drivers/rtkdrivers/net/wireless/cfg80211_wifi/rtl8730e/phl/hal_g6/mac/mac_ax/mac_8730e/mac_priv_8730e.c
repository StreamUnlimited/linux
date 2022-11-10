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

#include "../mac_priv.h"
#include "init_8730e.h"
#include "cmac_tx_8730e.h"
#include "fwcmd_8730e.h"
#include "rrsr_8730e.h"
#include "coex_8730e.h"
#include "phy_rpt_8730e.h"
#include "dbgpkg_8730e.h"
#include "efuse_8730e.h"
#include "dle_8730e.h"
#include "pwr_seq_func_8730e.h"
#include "mac_priv_8730e.h"
#include "../_axi.h"
#if MAC_AX_AXI_SUPPORT
#include "_axi_8730e.h"
#endif

#if MAC_AX_8730E_SUPPORT

static struct mac_ax_priv_ops mac8730e_priv_ops = {
	NULL, /* pwr_off, 8730e DO NOT USE this pwr_off function */
	NULL, /* pwr_on, 8730e DO NOT USE this pwr_on function */
	NULL, /* intf_pwr_switch */
	NULL, /* dmac_func_en */
	NULL, /* dmac_func_pre_en */
	init_trx_cfg_8730e, /*init_trx_cfg*/
	init_protocol_cfg_8730e, /*init_protocol_cfg*/
	init_edca_cfg_8730e, /*init_edca_cfg*/
	init_wmac_cfg_8730e, /*init_wmac_cfg_8730e*/
	enable_port_8730e, /*enable_port*/
	cfg_mac_addr_8730e,/*cfg_mac_addr*/
	cfg_bssid_8730e,/*cfg_bssid*/
	cfg_transmitter_addr_8730e,/*cfg_transmitter_addr*/
	cfg_net_type_8730e, /*cfg_net_type*/
	NULL, /*cmac module init, cmac_init*/
	NULL,/* cmac_func_en */
	NULL, /* macid_idle_ck */
	stop_sch_tx_8730e, /* stop_sch_tx */
	NULL, /* switch_efuse_bank */
	NULL, /* enable_efuse_sw_pwr_cut */
	NULL, /* disable_efuse_sw_pwr_cut */
	NULL, /*efuse_get_pwr_state*/
	efuse_read8_8730e, /*efuse_read8*/
	efuse_write8_8730e, /*efuse_write*/
	get_h2creg_offset_8852a, /* get_h2creg_offset */
	get_c2hreg_offset_8852a, /* get_c2hreg_offset */
	NULL, /*ser_imr_config*/
	NULL, /* disconnect_flush_key */
	sec_info_tbl_init, /* sec_info_tbl_init */
	free_sec_info_tbl, /* free_sec_info_tbl */
	NULL, /* mac_wowlan_secinfo */
	mac_get_rrsr_cfg_8730e, /*get RRSR related config,get_rrsr_cfg*/
	mac_set_rrsr_cfg_8730e, /*set RRSR related config, set_rrsr_cfg*/
	mac_get_csi_rrsr_cfg_8730e, /*get CSI RRSR related config, get_csi_rrsr_cfg*/
	mac_set_csi_rrsr_cfg_8730e, /*set CSI RRSR related config, set_csi_rrsr_cfg*/
	mac_cfg_gnt_8730e, /* cfg_gnt*/
	NULL, /* cfg_ctrl_path */
	mac_get_gnt_8730e, /* get_gnt */
	NULL, /* get_ctrl_path */
	NULL, /*get_bbrpt_dle_cfg*/
	NULL, /*for mac debug port, dbg_port_sel_8852a*/
	NULL, /*for mac tx flow ptcl dbg, tx_flow_ptcl_dbg_port_8852a*/
	NULL, /*for mac tx schdueler ptcl dbg, tx_flow_sch_dbg_port_8852a*/
	NULL, /*for mac station scheduler check*/
	NULL, /*for dmac debug dump, dmac_dbg_dump_8852a*/
	NULL, /*for cmac debug dump,cmac_dbg_dump_8852a*/
	NULL, /*for system critical debug dump,crit_dbg_dump_8852a*/
	NULL, /*for tx flow debug dump,tx_dbg_dump_8852a*/
	NULL, /* coex_mac_init */
	NULL, /* set_fc_page_ctrl_reg */
	NULL, /* get_fc_page_info */
	NULL, /* set_fc_pubpg */
	NULL, /* get_fc_mix_info */
	NULL, /* set_fc_h2c */
	NULL, /* set_fc_mix_cfg */
	NULL, /* set_fc_func_en */
	NULL, /* dle_dfi_ctrl */
	NULL, /* dle_is_txq_empty */
	NULL, /* dle_is_rxq_empty */
	preload_cfg_set_8730e, /* preload_cfg_set */
	preload_init_set_8730e, /* preload_init_set */
	NULL, /* dle_func_en */
	NULL, /* dle_clk_en */
	NULL, /* dle_mix_cfg */
	NULL, /* wde_quota_cfg */
	NULL, /* ple_quota_cfg */
	NULL, /* chk_dle_rdy */
	NULL, /* is_dbg_port_not_valid */
	NULL, /* dbg_port_sel_rst */
	NULL, /* dle_dfi_sel */
#if MAC_AX_PCIE_SUPPORT
	NULL, /* get_pcie_info_def */
	NULL, /* get_bdram_tbl_pcie */
	NULL, /* mio_w32_pcie */
	NULL, /* mio_r32_pcie */
	NULL, /* get_txbd_reg_pcie */
	NULL, /* set_txbd_reg_pcie */
	NULL, /* get_rxbd_reg_pcie */
	NULL, /* set_rxbd_reg_pcie */
	NULL, /* ltr_sw_trigger */
	NULL, /* pcie_cfgspc_write */
	NULL, /* pcie_cfgspc_read */
	NULL, /* pcie_ltr_write */
	NULL, /* pcie_ltr_read */
	NULL, /* ctrl_hci_dma_en_pcie */
	NULL, /* ctrl_trxdma_pcie */
	NULL, /* ctrl_wpdma_pcie */
	NULL, /* poll_io_idle_pcie */
	NULL, /* poll_dma_all_idle_pcie */
	NULL, /* clr_idx_ch_pcie */
	NULL, /* rst_bdram_pcie */
	NULL, /* trx_mit_pcie */
	NULL, /* mode_op_pcie */
	get_err_flag_pcie_8852a, /* get_err_flag_pcie */
	NULL, /* mac_auto_refclk_cal_pcie */
#ifdef RTW_WKARD_GET_PROCESSOR_ID
	NULL, /* chk_proc_long_ldy_pcie */
#endif
	NULL, /* sync_trx_bd_idx */
#endif
#if MAC_AX_AXI_SUPPORT
	NULL, /* get_txbd_reg_axi */
	NULL, /* set_txbd_reg_axi */
	NULL, /* get_rxbd_reg_axi */
	NULL, /* set_rxbd_reg_axi */
	NULL, /* poll_dma_all_idle_axi */
	NULL, /* clr_idx_ch_axi */
	NULL, /* get_err_flag_axi */
	NULL, /* sync_trx_bd_idx */
#ifdef RTW_WKARD_GET_PROCESSOR_ID
	NULL, /* chk_proc_long_ldy_axi */
#endif
#endif
#if MAC_AX_SDIO_SUPPORT
	r_indir_cmd52_sdio_8852a, /* r_indir_cmd52_sdio */
	_r_indir_cmd52_sdio_8852a, /* _r_indir_cmd52_sdio */
	_r_indir_cmd53_sdio_8852a, /* _r_indir_cmd53_sdio */
	r16_indir_sdio_8852a, /* r16_indir_sdio */
	r32_indir_sdio_8852a, /* r32_indir_sdio */
	w_indir_cmd52_sdio_8852a, /* w_indir_cmd52_sdio */
	w_indir_cmd53_sdio_8852a, /* w_indir_cmd53_sdio */
	ud_fs_8852a, /* ud_fs */
	sdio_pre_init_8852a, /* sdio_pre_init */
	tx_mode_cfg_sdio_8852a, /* tx_mode_cfg_sdio */
	leave_suspend_sdio_8852a, /* leave_suspend_sdio */
	get_int_latency_sdio_8852a, /* get_int_latency_sdio */
	get_clk_cnt_sdio_8852a, /* get_clk_cnt_sdio */
	set_wt_cfg_sdio_8852a, /* set_wt_cfg_sdio */
	set_clk_mon_sdio_8852a, /* set_clk_mon_sdio */
	sdio_pwr_switch_8852a, /* sdio_pwr_switch */
	rx_agg_cfg_sdio_8852a, /* rx_agg_cfg_sdio */
	aval_page_cfg_sdio_8852a, /* aval_page_cfg_sdio */
#endif
#if MAC_AX_USB_SUPPORT
	usb_ep_cfg_8852a, /* USB endpoint pause release */
#endif
};

struct mac_ax_priv_ops *get_mac_8730e_priv_ops(enum mac_ax_intf intf)
{
	switch (intf) {
#if MAC_AX_AXI_SUPPORT
	case MAC_AX_INTF_AXI:
		mac8730e_priv_ops.pwr_on = mac_pwr_on_8730e;
		mac8730e_priv_ops.pwr_off = mac_pwr_off_8730e;
		mac8730e_priv_ops.intf_pwr_switch = axi_pwr_switch;
		break;
#endif
	default:
		return NULL;
	}

	return &mac8730e_priv_ops;
}
#endif /* #if MAC_AX_8730E_SUPPORT */
