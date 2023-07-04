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

#include "init_8730e.h"
#include "../pwr.h"
#include "../efuse.h"
#include "../init.h"
#include "../trxcfg.h"
#include "pwr_seq_8730e.h"
#include "../hw.h"
#include "../security_cam.h"
#include "../trx_desc.h"
#include "../../feature_cfg.h"
#include "../fwcmd.h"
#include "../bcn.h"
#include "../fwdl.h"
#include "../fwofld.h"
#include "../role.h"
#include "../tblupd.h"
#include "../rx_filter.h"
#include "../phy_rpt.h"
#include "../status.h"
#include "../hw_seq.h"
#include "gpio_8730e.h"
#include "../gpio.h"
#include "../cpuio.h"
#include "../sounding.h"
#include "../power_saving.h"
#include "../wowlan.h"
#include "../tcpip_checksum_offload.h"
#include "../la_mode.h"
#include "../dle.h"
#include "../coex.h"
#include "../mcc.h"
#include "../twt.h"
#include "../mport.h"
#include "../p2p.h"
#include "../flash.h"
#include "../dbg_cmd.h"
#include "../phy_misc.h"
#include "../h2c_agg.h"
#include "mac_priv_8730e.h"
#include "coex_8730e.h"
#include "phy_rpt_8730e.h"
#include "hdr_conv_rx_8730e.h"
#include "dle_8730e.h"
#include "efuse_8730e.h"
#include "halmac_hwimg_8730e.h"

#if MAC_AX_AXI_SUPPORT
#include "../_axi.h"
#include "_axi_8730e.h"
#include "mac/fw_ax/rtl8730e/hal8730e_fw.h"
#endif
#if MAC_AX_FEATURE_DBGPKG
#include "../dbgpkg.h"
#include "../dbgport_hw.h"
#endif
#if MAC_AX_8730E_SUPPORT

#if MAC_AX_AXI_SUPPORT
static struct mac_ax_intf_ops mac8730e_axi_ops = {
	reg_read8_axi, /* reg_read8 */
	reg_write8_axi, /* reg_write8 */
	reg_read16_axi, /* reg_read16 */
	reg_write16_axi, /* reg_write16 */
	reg_read32_axi, /* reg_read32 */
	reg_write32_axi, /* reg_write32 */
	sys_reg_read8_axi, /* sys_reg_read8 */
	sys_reg_write8_axi, /* sys_reg_write8 */
	sys_reg_read16_axi, /* sys_reg_read16 */
	sys_reg_write16_axi, /* sys_reg_write16 */
	sys_reg_read32_axi, /* sys_reg_read32 */
	sys_reg_write32_axi, /* sys_reg_write32 */
	NULL, /* tx_allow_sdio */
	NULL, /* tx_cmd_addr_sdio */
	NULL, /* intf_pre_init */
	NULL, /* intf_init */
	NULL, /* intf_deinit */
	NULL, /* reg_read_n_sdio */
	NULL, /*get_bulkout_id*/
	NULL, /* ltr_set_pcie */
	NULL, /*u2u3_switch*/
	NULL, /*get_usb_mode*/
	NULL,/*get_usb_support_ability*/
	NULL, /*usb_tx_agg_cfg*/
	NULL, /*usb_rx_agg_cfg*/
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
	rtl8730e_set_wowlan_ctrl_cmd, /*set_wowlan*/
#else
	NULL, /*set_wowlan*/
#endif
	NULL, /*ctrl_txdma_ch*/
	mac_clr_idx_all, /*clr_idx_all*/
	NULL, /*poll_txdma_ch_idle*/
	NULL, /*poll_rxdma_ch_idle*/
	NULL, /*ctrl_txhci*/
	mac_ctrl_rxhci, /*ctrl_rxhci*/
	NULL, /*ctrl_dma_io*/
	NULL, /* get_io_stat */
	NULL, /*get_txagg_num*/
	NULL, /*get_avail_txbd*/
	NULL, /*get_avail_rxbd*/
	NULL, /*trigger_txdma*/
	NULL, /*notify_rxdone*/
	NULL, /*get_usb_rx_state*/
	NULL, /* dbcc_hci_ctrl */
	NULL, /* pcie_autok_counter_avg */
	NULL, /* tp_adjust */
};
#endif

static struct mac_ax_ops mac8730e_ops = {
	NULL, /* intf_ops */
	/*System level*/
	mac_hal_init, /* hal_init */
	mac_hal_fast_init, /* hal_fast_init */
	mac_hal_deinit, /* hal_deinit */
	mac_hal_fast_deinit, /*hal_fast_deinit*/
	mac_add_role, /* add_role */
	mac_remove_role, /* remove_role */
	mac_change_role, /* change_role */
	cfg_macid_8730e, /* cfg_macid */
	mac_pwr_switch, /* pwr_switch */
	mac_pre_sys_init, /*pre_sys_init*/
	mac_sys_init, /* sys_init */
	mac_trx_init, /*trx_init */
	mac_cfg_sec, /*cfg_sec*/
	init_pwr_reg_8730e, /* init_pwr_reg */
#if MAC_AX_AXI_SUPPORT
	enable_fw_8730e, /* enable_fw */
	mac_send_h2c_hmebox, /* send_h2c_hmebox */
#else
	mac_romdl, /* romdl */
	mac_enable_cpu, /* enable_cpu */
	mac_disable_cpu, /* disable_cpu */
	mac_fwredl, /* fwredl */
	mac_fwdl, /* fwdl */
	mac_query_fw_buff, /* query_fw_buff */
	mac_enable_fw, /* enable_fw */
#endif
	NULL, /* lv1_rcvy */
	mac_get_macaddr,
	mac_build_txdesc_8730e, /* build_txdesc */
	NULL, /* refill_txdesc */
	mac_parse_rxdesc_8730e, /* parse_rxdesc */
	mac_watchdog, /* watchdog */
	/*FW offload related*/
	mac_reset_fwofld_state,
	NULL,
	mac_read_pkt_ofld,
	mac_del_pkt_ofld,
	mac_add_pkt_ofld,
	mac_pkt_ofld_packet,
	mac_dump_efuse_ofld,
	NULL, /* efuse_ofld_map */
	NULL, /*update dmac ctrl info, upd_dctl_info*/
	NULL, /*update cmac ctrl info, upd_cctl_info*/
	mac_ie_cam_upd, /* ie_cam_upd */
	mac_twt_info_upd_h2c, /* twt info update h2c */
	mac_twt_act_h2c, /* twt act h2c */
	mac_twt_staanno_h2c, /* twt anno h2c */
	mac_twt_wait_anno,
	mac_host_getpkt_h2c,
	mac_p2p_act_h2c, /* p2p_act_h2c */
	mac_p2p_macid_ctrl_h2c, /* p2p_macid_ctrl_h2c */
	mac_get_p2p_stat, /* get_p2p_stat */
	NULL, /* tsf32_togl_h2c */
	mac_get_t32_togl_rpt, /* get_t32_togl_rpt */
	mac_ccxrpt_parsing, /* ccxrpt_parsing */
	mac_host_efuse_rec,
	/*Association, de-association related*/
	mac_sta_add_key, /* add station key */
	mac_sta_del_key, /* del station key */
	mac_sta_search_key_idx, /* sta_search_key_idx */
	mac_write_security_cam,/*write_security_cam*/
	mac_sta_hw_security_support, /* sta_hw_security_support */
	NULL, /*set mu score table, set_mu_table*/
	NULL, /* update SS dl group info, ss_dl_grp_upd*/
	NULL, /* update SS ul group info, ss_ul_grp_upd*/
	NULL, /* add sta into SS ul link, ss_ul_sta_upd*/
	NULL, /*update BA CAM info, mac_bacam_info*/
	/*TRX related*/
	mac_txdesc_len_8730e, /* txdesc_len */
	NULL,/*update short cut mac header*/
	NULL, /* enable_hwmasdu */
	NULL,
	NULL,
	NULL,
	NULL,/*mac_hwamsdu_max_len*/
	NULL,/*mac_hwamsdu_get_max_len*/
	NULL, /* enable_cut_hwamsdu */
	NULL, /* enable cut-amsdu chk mpdu size*/
	NULL, /* enable mac hdr conv, hdr_conv_en*/
	NULL, /* set eth type, hdr_conv_tx_set_eth_type */
	NULL, /* get eth type */
	NULL, /* get oui */
	NULL, /* get oui */
	NULL, /* enable mac hdr conv for specifical macid */
	NULL, /* enable vlantag valid */
	NULL, /* get vlantag valid*/
	NULL, /* enable qos control field translation */
	NULL, /* get qos control field translation */
	NULL, /* setup qos control field bit 8-15*/
	NULL, /* setup target header length */
	NULL, /* get target header length */
	mac_hdr_conv_rx_en_8730e, /* enable rx mac hdr conversion*/
	mac_hdr_conv_rx_en_driv_info_hdr_8730e, /*en rx hdr conv driver info*/
	mac_set_hwseq_reg, /* set hw seq by reg */
	NULL, /*for set hw seq content*/
	mac_get_hwseq_cfg, /*for get hw seq content*/
	mac_process_c2h, /* process_c2h */
	mac_parse_dfs, /* parse_dfs */
	mac_parse_ppdu, /* parse_ppdu */
	mac_cfg_phy_rpt, /* cfg_phy_rpt */
	NULL,/* rx_forwarding */
	mac_get_rx_fltr_opt, /* get_rx_fltr_opt */
	mac_set_rx_fltr_opt, /* set_rx_fltr_opt */
	mac_set_typ_fltr_opt, /* set_rx_fltr_typ_opt */
	mac_set_typsbtyp_fltr_opt, /* set_rx_fltr_typstyp_opt */
	mac_set_typsbtyp_fltr_detail, /* set detail type subtype filter config*/
	NULL, /* get addrcam setting */
	NULL, /* get addrcam disable default setting */
	NULL, /* config addrcam setting */
	NULL, /* config addrcam disable default setting */
	mac_sr_update, /* set sr parameter */
	mac_two_nav_cfg,  /* config 2NAV hw setting */
	mac_wde_pkt_drop, /* pkt_drop */
#ifdef RTW_PHL_BCN_IOT
	mac_send_bcn, /* send beacon h2c */
#else
	NULL,
#endif
	NULL, /*tx_mode_sel*/
	NULL, /* tcpip_chksum_ofd */
	NULL, /* chk_rx_tcpip_chksum_ofd */
	NULL, /*chk_allq_empty*/
	mac_is_txq_empty_8730e, /*is_txq_empty*/
	NULL, /*is_rxq_empty*/
	mac_parse_bcn_stats_c2h, /*parse tx bcn statistics*/
	NULL, /*tx_idle_poll*/
	mac_sifs_chk_cca_en, /* check cca in sifs enable/disable */
	NULL, /*for patch rate, patch_rx_rate*/
	NULL, /* wd offload for wp_offset, called while forming metadata, get_wp_offset */
	mac_set_ax_pkt_extension, /*set packet extension*/
	/*frame exchange related*/
	NULL, /* upd_ba_infotbl */
	NULL, /* upd_mu_sta */
	NULL, /* upd_ul_fixinfo */
	NULL, /*f2p_test_cmd*/
	NULL,
	NULL, /* snd_test_cmd */
	NULL, /* set_fixmode_mib */
	NULL,/*dumpwlanc*/
	NULL,/*dumpwlans*/
	NULL,/*dumpwland*/
	/*outsrcing related */
	mac_outsrc_h2c_common, /* outsrc common h2c */
	mac_read_pwr_reg, /* for read_pwr_reg*/
	mac_write_pwr_reg, /* for write tx power reg*/
	mac_write_msk_pwr_reg, /* for write tx power reg*/
	mac_write_pwr_ofst_mode, /* for write tx power mode offset reg*/
	mac_write_pwr_ofst_bw, /* for write tx power BW offset reg*/
	mac_write_pwr_ref_reg, /* for write tx power ref reg*/
	mac_write_pwr_limit_en, /* for write tx power limit enable reg*/
	mac_write_pwr_limit_rua_reg, /* for write tx power limit rua reg*/
	mac_write_pwr_limit_reg, /* for write tx power limit reg*/
	mac_write_pwr_by_rate_reg, /* for write tx power by rate reg*/
	mac_lamode_cfg, /*lamode_cfg*/
	NULL, /*trigger la mode start*/
	NULL, /*la mode buf size cfg */
	mac_get_lamode_st, /*get la mode status*/
	mac_read_xcap_reg, /*read xcap xo/xi reg*/
	mac_write_xcap_reg, /*write xcap xo/xi reg*/
	mac_write_bbrst_reg, /*write bb rst reg*/
	NULL, /*for BB control TX PATH, tx_path_map_cfg*/
	/*sounding related*/
	mac_get_csi_buffer_index, /* get CSI buffer index */
	mac_set_csi_buffer_index, /* set CSI buffer index */
	NULL, /* get MACID SND status */
	NULL, /* set SND status MACID */
	NULL,/* init SND MER */
	mac_init_snd_mee,/* init SND MEE,init_snd_mee */
	mac_csi_force_rate, /*CSI fix rate reg*/
	mac_csi_rrsc, /*CSI RRSC, csi_rrsc*/
	NULL, /*set sound parameter, set_snd_para*/
	mac_set_csi_para_reg, /*set reg csi para*/
	NULL, /*set csi para in cmac ctrl info, set_csi_para_cctl*/
	NULL, /*HW SND pause release, hw_snd_pause_release*/
	NULL, /*bypass SND status*/
	mac_deinit_mee, /*deinit_mee*/
	mac_snd_sup, /*bf entry num and SU MU buffer num*/
	mac_gid_pos, /*VHT MU GID position setting*/
	/*ps related*/
	mac_cfg_lps, /*config LPS*/
	mac_ps_pwr_state, /*set or check lps power state*/
	mac_chk_leave_lps, /*check already leave protocol ps*/
	mac_cfg_ips, /*config IPS*/
	mac_chk_leave_ips, /*check already leave IPS protocol*/
	mac_ps_notify_wake, /*send RPWM to wake up HW/FW*/
	mac_ps_set_32k, /* ps_set_32k */
	mac_ps_store_axi_regs, /* ps_store_axi_regs */
	mac_cfg_ps_advance_parm, /*config advance parameter for power saving*/
	mac_periodic_wake_cfg, /*config ips periodic wake*/
	/* Wowlan related*/
	mac_cfg_wow_wake, /*config wowlan wake*/
	mac_cfg_disconnect_det, /*config disconnect detective*/
	mac_cfg_keep_alive, /*config keep alive*/
	mac_cfg_gtk_ofld, /*config gtk ofld*/
	mac_cfg_arp_ofld, /*config arp ofld*/
	mac_cfg_ndp_ofld, /*config ndp ofld*/
	mac_cfg_realwow, /*config realwow*/
	mac_cfg_nlo, /*config nlo*/
	mac_cfg_dev2hst_gpio, /*config dev2hst gpio*/
	mac_cfg_uphy_ctrl, /*config uphy ctrl*/
	mac_cfg_wowcam_upd, /*config wowcam update*/
	mac_get_wow_wake_rsn, /* Get wowlan wakeup reason with reset option */
	mac_cfg_wow_sleep, /*config wowlan before sleep/after wake*/
	mac_get_wow_fw_status, /*get wowlan fw status*/
	mac_request_aoac_report, /* request_aoac_report */
	mac_read_aoac_report, /* read_aoac_report */
	mac_check_aoac_report_done, /* check_aoac_report_done */
	mac_wow_stop_trx, /* wow_stop_trx */
	mac_cfg_wow_auto_test, /* cfg_wow_auto_test */
	/*system related*/
	NULL, /*dbcc_enable */
	NULL, /* dbcc_trx_ctrl */
	mac_port_cfg, /* cofig port para, port_cfg*/
	mac_port_init, /* init port para, port_init*/
	NULL, /* enable CMAC/DMAC IMR */
	mac_dump_efuse_map, /* dump_efuse_map*/
	NULL, /* dump_bt_efuse */
	mac_write_efuse, /* write_wl_bt_efuse */
	mac_read_efuse, /* read_wl_bt_efuse */
	NULL, /* read_hidden_efuse */
	mac_get_efuse_avl_size, /* get_available_efuse_size */
	NULL, /* get_available_efuse_size_bt */
	dump_log_efuse_8730e, /* dump_log_efuse */
	read_log_efuse_8730e, /* read_log_efuse */
	write_log_efuse_8730e, /* write_logical_efuse */
	NULL, /* dump_logical_efuse_bt */
	NULL, /* read_logical_efuse_bt */
	NULL, /* write_logical_efuse_bt */
	NULL, /* program_efuse_map */
	NULL, /* program_efuse_map_bt */
	NULL, /* mask_logical_efuse_map */
	NULL, /* program_secure_data_map */
	NULL, /* compare_secure_data_map */
	mac_get_efuse_info, /* get_efuse_info */
	NULL, /* set_efuse_info */
	NULL, /* read_efuse_hidden_report */
	mac_check_efuse_autoload, /* check_efuse_autoload */
	NULL, /* efuse pg simulator */
	NULL, /* checksum update */
	NULL, /*report checksum comparison result, checksum_rpt*/
	NULL, /* Disable RF Offload,disable_rf_ofld_by_info */
	mac_set_efuse_ctrl, /*set efuse ctrl 0x30 or 0xC30*/
	NULL, /*efuse OTP test R/W to 0x7ff*/
	mac_get_ft_status, /* get_mac_ft_status */
	mac_fw_log_cfg, /* fw_log_cfg */
	NULL, /* pinmux_set_func */
	NULL, /* pinmux_free_func */
	NULL, /* sel_uart_tx_pin */
	NULL, /* sel_uart_rx_pin */
	NULL, /* set_gpio_func */
	NULL, /* get_gpio_val */
	mac_get_hw_info, /* get_hw_info */
	mac_set_hw_value, /* set_hw_value */
	mac_get_hw_value, /* get_hw_value */
	NULL, /* get_err_status */
	NULL, /* set_err_status */
	mac_general_pkt_ids, /*general_pkt_ids */
	mac_coex_init_8730e, /* coex_init */
	mac_read_coex_reg, /* coex_read */
	mac_write_coex_reg, /* coex_write */
	mac_get_scbd_8730e, /* get_scbd */
	mac_set_scbd_8730e, /* set_scbd */
	NULL, /*trigger_cmac_err*/
	NULL, /*trigger_cmac1_err*/
	NULL, /*trigger_dmac_err*/
	mac_tsf_sync, /*tsf_sync ask FW to Reset sync register at Beacon early interrupt. TODO in km*/
	NULL, /*8730E not support xtal_si*/
	NULL, /*8730E not support xtal_si*/
	NULL, /*io_chk_access */
	NULL, /* chk_ser_status */
	/* mcc */
	mac_reset_mcc_group,
	mac_reset_mcc_request,
	mac_add_mcc, /* add_mcc */
	mac_start_mcc, /* start_mcc */
	mac_stop_mcc, /* stop_mcc */
	mac_del_mcc_group, /* del_mcc_group */
	mac_mcc_request_tsf, /* mcc_request_tsf */
	mac_mcc_macid_bitmap, /* mcc_macid_bitmap */
	mac_mcc_sync_enable, /* mcc_sync_enable */
	mac_mcc_set_duration, /* mcc_set_duration */
	mac_get_mcc_tsf_rpt,
	mac_get_mcc_status_rpt,
	mac_get_mcc_group,
	mac_check_add_mcc_done,
	mac_check_start_mcc_done,
	mac_check_stop_mcc_done,
	mac_check_del_mcc_group_done,
	mac_check_mcc_request_tsf_done,
	mac_check_mcc_macid_bitmap_done,
	mac_check_mcc_sync_enable_done,
	mac_check_mcc_set_duration_done,
	/* not mcc */
	NULL,/*mac_check_access*/
	NULL, /* set_led_mode */
	NULL, /* led_ctrl */
	NULL, /* set_sw_gpio_mode */
	NULL, /* sw_gpio_ctrl */
	mac_get_c2h_event, /* get_c2h_event */
	NULL, /* cfg_wps */
	NULL, /* get_wl_dis_val */
	mac_cfg_per_pkt_phy_rpt_8852a, /* cfg_per_pkt_phy_rpt */
#if MAC_AX_FEATURE_DBGPKG
	NULL, /* fwcmd_lb */
	mac_mem_dump, /* mem_dump */
	mac_get_mem_size, /* get_mem_size */
	NULL, /* mac dbg_status_dump */
	mac_reg_dump, /* debug reg_dump for MAC/BB/RF*/
	mac_rx_cnt,/*rx_cnt*/
	NULL,/*dump_fw_rsvd_ple*/
	NULL,/*fw_dbg_dump*/
	NULL,/*event_notify*/
	NULL, /* Set debug port for LA, dbgport_hw_set */
#endif
#if MAC_AX_FEATURE_HV
	mac_ram_boot, /* ram_boot */
	/*fw offload related*/
	mac_clear_write_request, /* clear_write_request */
	mac_add_write_request, /* add_write_request */
	mac_write_ofld, /* write_ofld */
	mac_clear_conf_request, /* clear_conf_request */
	mac_add_conf_request, /* add_conf_request */
	mac_conf_ofld, /* conf_ofld */
	mac_clear_read_request, /* clear_read_request */
	mac_add_read_request, /* add_read_request */
	mac_read_ofld, /* read_ofld */
	mac_read_ofld_value, /* read_ofld_value */
#endif
	mac_add_cmd_ofld, /* add_cmd_ofld */
	NULL, /* cmd_ofld */
	mac_flash_erase,
	mac_flash_read,
	mac_flash_write,
	NULL, /* fw_status_cmd */
	mac_tx_duty, /* tx_duty */
	mac_tx_duty_stop, /* tx_duty _stop */
	NULL, /* fwc2h_ofdma_sts_parse */
	NULL, /* fw_ofdma_sts_en, send fw sts en to fw */
	mac_get_phy_rpt_cfg, /* get_phy_rpt_cfg */
#if MAC_AX_FEATURE_DBGCMD
	NULL, /* halmac_cmd */
	NULL, /* halmac_cmd_parser */
#endif
	mac_fast_ch_sw,
	mac_fast_ch_sw_done,
	mac_get_fast_ch_sw_rpt,
	mac_h2c_agg_enable,
	mac_h2c_agg_flush,
	mac_h2c_agg_tx,
	NULL,/*fw_dbg_dle_cfg*/
	mac_add_scanofld_ch,
	mac_scanofld,
	mac_scanofld_fw_busy,
	mac_scanofld_chlist_busy,
#if MAC_AX_FEATURE_DBGDEC
	NULL,/*fw_log_set_array*/
	NULL,/*fw_log_unset_array*/
#endif
	NULL,/*get_fw_status*/
	mac_role_sync,
	NULL,
	NULL,
	mac_cfg_bcn_filter,
	mac_bcn_filter_rssi,
	mac_bcn_filter_tp
};

static struct mac_ax_hw_info mac8730e_hw_info = {
	0, /* done */
	MAC_AX_CHIP_ID_8730E, /* chip_id */
	0xFF, /* cv */
	0, /* acv */
	MAC_AX_INTF_INVALID, /* intf */
	19, /* tx_ch_num */
	10, /* tx_data_ch_num */
	TX_DESC_LEN, /* tx_desc_len */
	pwr_on_seq_8730e, /* pwr_on_seq */
	pwr_off_seq_8730e, /* pwr_off_seq */
	PWR_SEQ_VER_8730E, /* pwr_seq_ver */
	MAC_STA_NUM, /* macid_num */
	2, /* port_num */
	8, /* mbssid_num */
	16, /* bssid_num */
	0x800, /* efuse_size */
	0x200, /* log_efuse_size */
	NULL, /* sec_cam_table_t pointer */
	NULL, /* dctl_sec_info_t pointer */
	0, /* efuse_start_addr */
	1, /*checksum_en*/
	0, /* sw_amsdu_max_size */
	0, /* ind_aces_cnt */
	0, /* dbg_port_cnt */
	0, /* core_swr_volt */
	{{{0}, 0}}, /* cust_proc_id */
	MAC_AX_SWR_NORM, /* core_swr_volt_sel */
	MAC_AX_DRV_INFO_NONE, /* cmac0_drv_info */
	//MAC_AX_DRV_INFO_NONE, /* cmac1_drv_info */
};

struct mac_ax_ft_status mac_8730e_ft_status[] = {
	{MAC_AX_FT_DUMP_EFUSE, MAC_AX_STATUS_IDLE, NULL, 0},
	{MAC_AX_FT_MAX, MAC_AX_STATUS_ERR, NULL, 0},
};

static struct mac_ax_adapter mac_8730e_adapter = {
	&mac8730e_ops, /* ops */
	NULL, /* drv_adapter */
	NULL, /* phl_adapter */
	NULL, /* pltfm_cb */
	MAC_AX_DFLT_SM, /* sm */
	NULL, /* hw_info */
	{0}, /* fw_info */
	{0}, /* efuse_param */
	{0}, /* mac_pwr_info */
	mac_8730e_ft_status, /* ft_stat */
	NULL, /* hfc_param */
	{MAC_AX_QTA_SCC, 64, 128, 0, 0, 0, 0, 0, 0}, /* dle_info */
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, DFLT_GPIO_STATE, DFLT_SW_IO_MODE
	}, /* gpio_info */
	NULL, /* role table */
	{RSVD_PAGE_PROB_RSP, RSVD_PAGE_PS_POLL, RSVD_PAGE_NULL_DATA,
		RSVD_PAGE_QOS_NULL, RSVD_PAGE_BT_QOS_NULL, RSVD_PAGE_CTS2SELF,
		RSVD_PAGE_LTECOEX_QOSNULL, RSVD_PAGE_ARP_RSP, 0}, /* pkt_ofld_info */
	{0, 0, 0, NULL}, /* pkt_ofld_pkt */
	{NULL, NULL, NULL, 0, 0, 0, 0, 0}, /* cmd_ofld_info */
	{{{0}, {0}, {0}, {0}}}, /* mcc_group_info */
	{NULL}, /* wowlan_info */
	NULL, /* p2p_info */
	NULL, /* t32_togl_rpt */
	NULL, /* port_info */
	{0}, /* struct mac_ax_int_stats stats */
	{0}, /*h2c_agg_info*/
	{0, 0}, /* struct mac_ax_drv_stats drv_stats */
#if MAC_AX_AXI_SUPPORT
	{0}, /* axi_info */
#endif
	{0, 0, 0, 0, 0, 0, 0, 0, 0, NULL}, /*flash_info */
	{0, 0}, /* fast_ch_sw_info */
#if MAC_AX_FEATURE_HV
	NULL, /*hv_ax_ops*/
	HV_AX_ASIC, /* env */
#endif
#if MAC_AX_FEATURE_DBGCMD
	{NULL}, /*fw_dbgcmd*/
#endif
#if MAC_AX_FEATURE_DBGDEC
	NULL, /*fw_log_array*/
	NULL,
	0,
#endif
	{NULL}, /*scan ofld info*/
	{0}, /*log_cfg*/
	NULL, /* twt_info */
	NULL, /* dbcc_info */
};

static u8 _crc5_generate(unsigned char *in, unsigned long byte_num)
{
	int a, b;
	unsigned long i, j;
	u8 mask, smask = 0x1;
	int crc_mask = 0x00000010;
	int poly = 0x05; //CRC5 100101 ==>00101
	int crc_inter = 0x1f; //initial value

	for (i = 0; i < byte_num; i++) {
		mask = smask;
		for (j = 0; j < 8; j++) {
			a = ((crc_inter & crc_mask) != 0);
			b = ((in[i] & mask) != 0);

			crc_inter <<= 1;
			mask <<= 1;

			if (a ^ b) {
				crc_inter ^= poly;
			}
		}
	}

	return (crc_inter ^ 0x1f) & 0x1f;
}

struct mac_ax_adapter *get_mac_8730e_adapter(enum mac_ax_intf intf,
		u8 cv, void *drv_adapter,
		struct mac_ax_pltfm_cb *pltfm_cb)
{
	struct mac_ax_adapter *adapter = NULL;
	struct mac_ax_hw_info *hw_info = NULL;
	struct mac_ax_priv_ops **p;
	struct mac_ax_mac_pwr_info *pwr_info;
	u32 priv_size;

	if (!pltfm_cb) {
		return NULL;
	}

	priv_size = get_mac_ax_priv_size();
	adapter = (struct mac_ax_adapter *)pltfm_cb->rtl_malloc(drv_adapter,
			sizeof(struct mac_ax_adapter) + priv_size);
	if (!adapter) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "Malloc adapter fail\n");
		return NULL;
	}

	pltfm_cb->rtl_memcpy(drv_adapter, adapter, &mac_8730e_adapter,
			     sizeof(struct mac_ax_adapter));

	/*Alloc HW INFO */
	hw_info = (struct mac_ax_hw_info *)pltfm_cb->rtl_malloc(drv_adapter,
			sizeof(struct mac_ax_hw_info));

	if (!hw_info) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "Malloc hw info fail\n");
		return NULL;
	}

	pltfm_cb->rtl_memcpy(drv_adapter, hw_info, &mac8730e_hw_info,
			     sizeof(struct mac_ax_hw_info));

	pwr_info = &adapter->mac_pwr_info;

	adapter->drv_adapter = drv_adapter;
	adapter->pltfm_cb = pltfm_cb;
	adapter->hw_info = hw_info;
	adapter->hw_info->cv = cv;
	adapter->hw_info->intf = intf;
	adapter->hw_info->done = 1;

	p = get_priv(adapter);
	*p = get_mac_8730e_priv_ops(intf);

	switch (intf) {
#if MAC_AX_AXI_SUPPORT
	case MAC_AX_INTF_AXI:
		adapter->ops->intf_ops = &mac8730e_axi_ops;
		pwr_info->intf_pwr_switch = axi_pwr_switch;
		break;
#endif
	default:
		return NULL;
	}

	return adapter;
}

/*AXI RQPN Mapping*/
static struct halmac_rqpn HALMAC_RQPN_AXI_8730E[] = {
	/* { mode, vo_map, vi_map, be_map, bk_map, mg_map, hi_map } */
	{
		HALMAC_TRX_MODE_NORMAL,
		HALMAC_MAP2_NQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_LQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
	{
		HALMAC_TRX_MODE_TRXSHARE,
		HALMAC_MAP2_NQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_LQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
	{
		HALMAC_TRX_MODE_WMM,
		HALMAC_MAP2_HQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_NQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
	{
		HALMAC_TRX_MODE_P2P,
		HALMAC_MAP2_NQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_LQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
	{
		HALMAC_TRX_MODE_LOOPBACK,
		HALMAC_MAP2_NQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_LQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
	{
		HALMAC_TRX_MODE_DELAY_LOOPBACK,
		HALMAC_MAP2_NQ, HALMAC_MAP2_NQ, HALMAC_MAP2_LQ, HALMAC_MAP2_LQ,
		HALMAC_MAP2_EXQ, HALMAC_MAP2_HQ
	},
};

/*AXI Page Number*/
static struct halmac_pg_num HALMAC_PG_NUM_AXI_8730E[] = {
	/* { mode, hq_num, nq_num, lq_num, exq_num, gap_num} */
	{HALMAC_TRX_MODE_NORMAL, 8, 8, 8, 8, 1},
	{HALMAC_TRX_MODE_TRXSHARE, 8, 8, 8, 8, 1},
	{HALMAC_TRX_MODE_WMM, 8, 8, 8, 8, 1},
	{HALMAC_TRX_MODE_P2P, 8, 8, 8, 8, 1},
	{HALMAC_TRX_MODE_LOOPBACK, 8, 8, 8, 8, 1},
	{HALMAC_TRX_MODE_DELAY_LOOPBACK, 8, 8, 8, 8, 1},
};

static void mac_dis_clock_gate_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 value32 = 0;

	/*RXDMA CG*/
	value32 = MAC_REG_R32(REG_RXDMA_MODE);
	value32 &= ~BIT_RXDMA_GCLK_EN;
	MAC_REG_W32(REG_RXDMA_MODE, value32);

	/*TXDMA CG*/
	value32 = MAC_REG_R32(REG_TDE_GCK_CTRL);
	value32 &= ~BIT_TDE_GCLK_EN;
	MAC_REG_W32(REG_TDE_GCK_CTRL, value32);

	/*PTCL CG*/
	value32 = MAC_REG_R32(REG_GCLK_CFG);
	value32 &= ~BIT_EN_PTCL_GCLK;
	MAC_REG_W32(REG_GCLK_CFG, value32);

	/*WSEC CG*/
	value32 = MAC_REG_R32(REG_SECCFG);
	value32 |= (BIT_DIS_GCLK_TKIP | BIT_DIS_GCLK_AES | BIT_DIS_GCLK_WAPI);
	MAC_REG_W32(REG_SECCFG, value32);

	/*TX CG*/
	value32 = MAC_REG_R32(REG_TCR);
	value32 |= BIT_WMAC_TCR_DISGCLK;
	MAC_REG_W32(REG_TCR, value32);

	/*RX CG*/
	value32 = MAC_REG_R32(REG_RCR);
	value32 |= BIT_DISGCLK;
	MAC_REG_W32(REG_RCR, value32);
}

u32 init_trx_cfg_8730e(struct mac_ax_adapter *adapter,
		       enum halmac_trx_mode mode)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;

	ret = txdma_queue_mapping_8730e(adapter, mode);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]queue mapping %d\n", ret);
		return ret;
	}

	ret = priority_queue_cfg_8730e(adapter, mode);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]priority queue cfg\n");
		return ret;
	}

	return MACSUCCESS;
}

u32 init_protocol_cfg_8730e(struct mac_ax_adapter *adapter)
{
	u32 max_agg_num;
	u32 max_rts_agg_num;
	u32 val32;
	u16 pre_txcnt;
	u8 val8;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;

	init_txq_ctrl_8730e(adapter);
	init_sifs_ctrl_8730e(adapter);
	init_rate_fallback_ctrl_8730e(adapter);

	MAC_REG_W8(REG_BCN_AMPDU_CTCL + 2, WLAN_AMPDU_MAX_TIME);
	MAC_REG_W32(REG_NDPA_CTRL_LBK_ACQ_STOP,
		    MAC_REG_R32(REG_NDPA_CTRL_LBK_ACQ_STOP) | BIT_R_EOF_EN);

	pre_txcnt = (u16)(WLAN_PRE_TXCNT_TIME_TH | BIT_PRETX_AGGR_EN);
	val32 = MAC_REG_R32(REG_PRETX_DROP_CTRL);
	val32 |= (WLAN_PRE_TXCNT_TIME_TH << 8) | BIT_PRETX_AGGR_EN;
	MAC_REG_W32(REG_PRETX_DROP_CTRL, val32);

	max_agg_num = WLAN_MAX_AGG_PKT_LIMIT;
	max_rts_agg_num = WLAN_RTS_MAX_AGG_PKT_LIMIT;
	val32 = WLAN_RTS_LEN_TH | (WLAN_RTS_TX_TIME_TH << 8) |
		(max_agg_num << 16) | (max_rts_agg_num << 24);
	MAC_REG_W32(REG_PROT_MODE_CTRL, val32);

	MAC_REG_W16(REG_BAR_MODE_CTRL + 2,
		    WLAN_BAR_RETRY_LIMIT | WLAN_RETRY_PKT_LIMIT << 8);

	/*close A/B/C/D-cut BA parser*/
	val8 = MAC_REG_R8(REG_BNDY_LIFETIME_TAIL + 2);
	val8 = val8 & (~ BIT(5));
	MAC_REG_W8(REG_BNDY_LIFETIME_TAIL + 2, val8);

	/* disable all port */
	MAC_REG_W16(REG_MACID_H + 2, 0);

	/*Bypass TXBF error protection due to sounding failure*/
	val32 = MAC_REG_R32(REG_BF0_TIME_SETTING) & (~BIT_EN_BF0_UPDATE);//bit29
	MAC_REG_W32(REG_BF0_TIME_SETTING, val32 | BIT_EN_BF0_TIMER);
	val32 = MAC_REG_R32(REG_BF1_TIME_SETTING) & (~BIT_EN_BF1_UPDATE);//bit29
	MAC_REG_W32(REG_BF1_TIME_SETTING, val32 | BIT_EN_BF1_TIMER);
	val32 = MAC_REG_R32(REG_BF_TIMEOUT_EN) & (~BIT_BF0_TIMEOUT_EN) &
		(~BIT_BF1_TIMEOUT_EN);
	MAC_REG_W32(REG_BF_TIMEOUT_EN, val32);

	/*Fix incorrect HW default value of RSC*/
	val32 = BIT_CLEAR_RRSR_RSC(MAC_REG_R32(REG_RRSR));
	val32 = BIT_SET_R_RATE_CTRL_TABLE(val32, WLAN_RATE_RRSR_CCK_ONLY_1M);
	MAC_REG_W32(REG_RRSR, val32);

	val8 = MAC_REG_R8(REG_CFEND_RATE_SC_CTRL);
	MAC_REG_W8(REG_CFEND_RATE_SC_CTRL, val8 | BIT(5));

	/*for driver not update beacon every TBTT, jerry_zhou*/
	val8 = MAC_REG_R8(REG_BCN_AMPDU_CTCL);
	val8 = val8 & (~(BIT(3))) & (~(BIT(6)));
	MAC_REG_W8(REG_BCN_AMPDU_CTCL, val8);

	val8 = MAC_REG_R8(REG_FWHW_TXQ_HWSSN_CTRL + 2);
	val8 = val8 | BIT(2);
	MAC_REG_W8(REG_FWHW_TXQ_HWSSN_CTRL + 2, val8);

	/*for HW update SSN,if tx desc EN_HWSEQ=1,seq update by hw,
	else,seq update by sw;
	beacon update seq by hw only according 0x423[6]=1
	*/
	MAC_REG_W8(REG_FWHW_TXQ_HWSSN_CTRL + 3, 0xFF);

	return MACSUCCESS;
}

u32 init_edca_cfg_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret = MACSUCCESS;
	u8 val8;


	MAC_REG_W32(REG_EDCA_VO_PARAM, WLAN_EDCA_VO_PARAM);
	MAC_REG_W32(REG_EDCA_VI_PARAM, WLAN_EDCA_VI_PARAM);
	MAC_REG_W32(REG_EDCA_BE_PARAM, WLAN_EDCA_BE_PARAM);
	MAC_REG_W32(REG_EDCA_BK_PARAM, WLAN_EDCA_BK_PARAM);

	MAC_REG_W16(REG_BCN_CFG_PIFS, 0x660F);
	val32 = MAC_REG_R32(REG_BCN_CFG_PIFS);
	val32 = BIT_SET_PIFS(val32, WLAN_PIFS_TIME);
	MAC_REG_W32(REG_BCN_CFG_PIFS, val32);

	/*sifs break enable, can finish the process of gen_cmd
		when enable pretx, set to 0*/
	HALMAC_REG_W8_CLR(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL + 1, BIT(4));

	val8 = MAC_REG_R8(REG_NOA0_MBSSID_RD_CTRL + 1);
	val8 = (val8 | BIT(0) | BIT(1) | BIT(2) | BIT(6)) & (~ BIT(7));
	MAC_REG_W8(REG_NOA0_MBSSID_RD_CTRL + 1, val8);

	cfg_mac_clk_8730e(adapter);

	cfg_32k_clk_8730e(adapter);

	/*enable freerun counter*/
	val8 = MAC_REG_R8(REG_FREERUN_CNT_P2POFF_DIS_TXTIME + 1);
	val8 = (val8 | BIT(5));
	MAC_REG_W8(REG_FREERUN_CNT_P2POFF_DIS_TXTIME + 1, val8);

	/*not according secondary_cca20/cca40 when tx,  modify later*/
	val32 = MAC_REG_R32(REG_CCA_NAV_CHK);
	val32 &= ~(BIT(18) | BIT(17) | BIT(9) | BIT(8) | BIT(2) | BIT(1));
	MAC_REG_W32(REG_CCA_NAV_CHK, val32);

	/* Clear TX pause */
	MAC_REG_W16(REG_TXPAUSE_TXPTCL_DISTXREQ_CTRL + 2, 0x0000);

	val32 = MAC_REG_R32(REG_TSFT_SYN_OFFSET_SLOT);
	MAC_REG_W32(REG_TSFT_SYN_OFFSET_SLOT, BIT_SET_SLOTTIME(val32, WLAN_SLOT_TIME));

	MAC_REG_W32(REG_NAV_CTRL, WLAN_NAV_CFG);
	MAC_REG_W16(REG_BCNERR_TSF_CTRL + 2, WLAN_RX_TSF_CFG);

	/* Set beacon cotnrol - enable TSF and other related functions */
	MAC_REG_W8(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1,
		   (u8)(MAC_REG_R8(REG_TSTRST_P2PRST_BCN_CTRL_PORT0_1) |
			BIT_P0_EN_BCN_FUNCTION));

	/* Set send beacon related registers */
	MAC_REG_W32(REG_NOA0_TBTT_PROHIBIT, WLAN_TBTT_TIME);
	MAC_REG_W8(REG_ATIMWND0_P0_BCN_ERLY_DMA_CTRL, WLAN_DRV_EARLY_INT);
	MAC_REG_W8(REG_ATIMWND0_P0_BCN_ERLY_DMA_CTRL + 1, WLAN_BCN_DMA_TIME);
	MAC_REG_W8(REG_BCNERR_TSF_CTRL + 1, WLAN_BCN_MAX_ERR);

	/* MU primary packet fail, BAR packet will not issue */
	//HALMAC_REG_W8_SET(REG_BAR_TX_CTRL, BIT(0));   //yx_qi

	return MACSUCCESS;
}

u32 init_wmac_cfg_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u8 val8;
	u16 val16;
	u8 DA[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
	u8 sec_cfg;
	u32 start_page, val32;   //crc5_addr,addr,wdata_L,wdata_H,
	u8 i = 0, j = 0;
	start_page = 0;

	MAC_REG_W32(REG_MAR, 0xFFFFFFFF);
	MAC_REG_W32(REG_MAR + 4, 0xFFFFFFFF);

	MAC_REG_W8(REG_CSI_CTRL + 2, WLAN_RESP_TXRATE);
	MAC_REG_W32(REG_RESP_SIFS_CCK, WLAN_ACKTO);
	MAC_REG_W16(REG_ACKTO + 2, WLAN_EIFS_DUR_TUNE);

	MAC_REG_W8(REG_NAV_THRSHOLD + 2, WLAN_NAV_MAX);

	/*0x668[21:20]=2'b10  define RX busy according to PHYRXON*/
	val8 = MAC_REG_R8(REG_WMAC_TRXPTCL_CTL + 2);
	val8 = (val8 & (~ BIT(4))) | BIT(5);
	MAC_REG_W8(REG_WMAC_TRXPTCL_CTL + 2, val8);

	/*0x66C[8]=0  0x66C[1:0]=2'b00  in itself txop can send CTS*/
	val16 = MAC_REG_R16(REG_WMAC_TRXPTCL_CTL_H);
	val16 = (u16)((val16 & (~(BIT_EN_TXCTS_INTXOP_V1 | BIT_EN_TXCTS_INRXNAV_V1 |
		      BIT_EN_TXCTS_TO_TXOPOWNER_INRXNAV_V1))));
	MAC_REG_W16(REG_WMAC_TRXPTCL_CTL_H, val16);

	MAC_REG_W32(REG_RXFLTMAP0, WLAN_RX_FILTER0);
	MAC_REG_W16(REG_RXFLTMAP, WLAN_RX_FILTER2);

	/* enable bssid check for trigger/ndpa/ba */
	MAC_REG_W32(REG_RX_CFG, (MAC_REG_R32(REG_RX_CFG) | BIT_R_CHK_CTL_BSSID));

	MAC_REG_W32(REG_RCR, WLAN_RCR_CFG);
	val8 = MAC_REG_R8(REG_RXPSF_CTRL + 2);
	val8 = (val8 & 0xF0) | 0xa;	/*set [3:1]=5 to solve macrx clk gating issue, suggested by Darren_liu*/
	MAC_REG_W8(REG_RXPSF_CTRL + 2, val8);

	MAC_REG_W8(REG_RXPKT_CTL, WLAN_RXPKT_MAX_SZ_512);
	MAC_REG_W8(REG_RXPKT_CTL + 1, 0x05);

	/* enable hw to update beacon's time field */
	val8 = MAC_REG_R8(REG_TCR);
	val8 |= BIT_WMAC_TCR_UPD_TIMIE;
	MAC_REG_W8(REG_TCR, val8);
	/*TX CG*/
	MAC_REG_W8(REG_TCR + 2, WLAN_TX_FUNC_CFG2);
	MAC_REG_W8(REG_TCR + 1, WLAN_TX_FUNC_CFG1);

	/*csi_report put in page 252,0x169C[11:0]=0xFC , jerry_zhou */
	val16 = MAC_REG_R16(REG_WMAC_CSIDMA_CFG);
	val16 = (val16 & 0xF000) | 0xFC;
	MAC_REG_W16(REG_WMAC_CSIDMA_CFG, val16);

	/* append MACID in the rx descriptor of rx response packet */
	MAC_REG_W32(REG_ANTTRN_CTR_1_V1, BIT_APPEND_MACID_IN_RESP_EN);

	//HALMAC_REG_W16_SET(REG_GENERAL_OPTION, BIT_DUMMY_FCS_READY_MASK_EN); yx_qi

	//jerry_zhou 20201202
	//for driver handle 802.11w management frame encrypt and decode by SW
	sec_cfg = MAC_REG_R8(REG_WSEC_OPTION + 2);
	sec_cfg &= ~(BIT(4) | BIT(6) | BIT(3) | BIT(5));
	MAC_REG_W8(REG_WSEC_OPTION + 2, sec_cfg);

	HALMAC_REG_W8_SET(REG_SND_PTCL_CTRL, BIT_R_DISABLE_CHECK_VHTSIGB_CRC);

	/*0x7d8[31]=1 dlk timeout enable, fix rx hang
	  0x7d8[24]=1 fix rx fifo full hang
	*/
	MAC_REG_W32(REG_RX_FILTER_FUNCTION, WLAN_MAC_OPT_FUNC2);

	/*0x16f6=0x4  new rx coming, fix rx hang*/
	MAC_REG_W8(REG_CHINFO_TRIG_TYPE1 + 2, 0x4);

	val32 = MAC_REG_R32(REG_WMAC_OPTION_FUNCTION_1);
	val32 |= (BIT_R_WMAC_FIL_FCPROVER_1 | BIT_R_WMAC_FIL_FCTYPE_1 \
		 | BIT_R_WMAC_SRCH_TXRPT_MID_1 | BIT_R_WMAC_SRCH_TXRPT_PERPKT_1);
	MAC_REG_W32(REG_WMAC_OPTION_FUNCTION_1, val32);

	/*https://jira.realtek.com/browse/AMEBALITE-190*/
	val32 = MAC_REG_R32(REG_RCR);
	val32 |= BIT_APP_FCS;
	MAC_REG_W32(REG_RCR, val32);

	ret = init_low_pwr_8730e(adapter);
	if (ret != MACSUCCESS) {
		return ret;
	}

	/*To solve not update basic nav issue, https://jira.realtek.com/browse/AMEBALITE-234*/
	val32 = MAC_REG_R32(REG_RESP_CONTROL_1);
	val32 &= ~BIT_RESP_EARLY_START;
	MAC_REG_W32(REG_RESP_CONTROL_1, val32);

	/*To solve orig tx blocked when mu sounding, https://jira.realtek.com/browse/AMEBAD2-1382*/
	val32 = MAC_REG_R32(REG_BEAMFORMING_CTRL);
	val32 &= ~BIT_WMAC_CSI_BFRP_STOPTX;
	MAC_REG_W32( REG_BEAMFORMING_CTRL, val32);

	/*To solve txtb data fail cause tx resp hang issue, https://jira.realtek.com/browse/AMEBAD2-1446*/
	val32 = MAC_REG_R32(REG_RXAI_CTRL);
	val32 |= (BIT_RXAI_ADDR_CHKEN | BIT_RXAI_PRTCT_SEL);
	MAC_REG_W32(REG_RXAI_CTRL, val32);

	/*To solve the issue that not resp hetb when bsrp/bfrp/bqrp/murts with cs_required=1*/
	val32 = MAC_REG_R32(REG_RESP_CONTROL);
	val32 &= ~BIT_RESP_RXTRIG_CHK_INSIFS;
	MAC_REG_W32(REG_RESP_CONTROL, val32);

	/*gnt wlan*/
	MAC_REG_W8(REG_BT_COEX_ENH + 1, COEX_GNT_WLAN);

	mac_dis_clock_gate_8730e(adapter);

	return MACSUCCESS;
}

u32 txdma_queue_mapping_8730e(struct mac_ax_adapter *adapter,
			      enum halmac_trx_mode mode)
{
	u16 value16;
	struct halmac_rqpn *cur_rqpn_sel = NULL;
	u32 ret = MACSUCCESS;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (is_intf(adapter, MAC_AX_INTF_AXI)) {
		cur_rqpn_sel = HALMAC_RQPN_AXI_8730E;
	} else {
		return MACNOTSUP;
	}

	ret = rqpn_parser_8730e(adapter, mode, cur_rqpn_sel);
	if (ret != MACSUCCESS) {
		return ret;
	}

	value16 = 0;
	value16 |= BIT_TXDMA_HIQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_HI]);
	value16 |= BIT_TXDMA_MGQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_MG]);
	value16 |= BIT_TXDMA_BKQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_BK]);
	value16 |= BIT_TXDMA_BEQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_BE]);
	value16 |= BIT_TXDMA_VIQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_VI]);
	value16 |= BIT_TXDMA_VOQ_MAP(adapter->pq_map[HALMAC_PQ_MAP_VO]);
	MAC_REG_W16(REG_TXDMA_PQ_MAP, value16);

	return MACSUCCESS;
}

u32 rqpn_parser_8730e(struct mac_ax_adapter *adapter,
		      enum halmac_trx_mode mode, struct halmac_rqpn *tbl)
{
	u8 flag;
	u32 i;

	flag = 0;
	for (i = 0; i < HALMAC_TRX_MODE_MAX; i++) {
		if (mode == tbl[i].mode) {
			adapter->pq_map[HALMAC_PQ_MAP_VO] = tbl[i].dma_map_vo;
			adapter->pq_map[HALMAC_PQ_MAP_VI] = tbl[i].dma_map_vi;
			adapter->pq_map[HALMAC_PQ_MAP_BE] = tbl[i].dma_map_be;
			adapter->pq_map[HALMAC_PQ_MAP_BK] = tbl[i].dma_map_bk;
			adapter->pq_map[HALMAC_PQ_MAP_MG] = tbl[i].dma_map_mg;
			adapter->pq_map[HALMAC_PQ_MAP_HI] = tbl[i].dma_map_hi;
			flag = 1;
			break;
		}
	}

	if (flag == 0) {
		PLTFM_MSG_ERR("[ERR]trx mdoe!!\n");
		return MACNOTSUP;
	}

	return MACSUCCESS;
}

u32 priority_queue_cfg_8730e(struct mac_ax_adapter *adapter,
			     enum halmac_trx_mode mode)
{
	u8 transfer_mode = 0;
	u32 cnt, val32 = 0;
	struct halmac_txff_allocation *txff_info = &adapter->txff_alloc;
	u32 ret = MACSUCCESS;
	struct halmac_pg_num *cur_pg_num = NULL;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = set_trx_fifo_info_8730e(adapter, mode);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]set trx fifo!!\n");
		return ret;
	}

	if (is_intf(adapter, MAC_AX_INTF_AXI)) {
		cur_pg_num = HALMAC_PG_NUM_AXI_8730E;
	} else {
		return MACNOTSUP;
	}

	ret = pg_num_parser_8730e(adapter, mode, cur_pg_num);
	if (ret != MACSUCCESS) {
		return ret;
	}

	MAC_REG_W32(REG_RQPN_CTRL_HLPQ, ((txff_info->pub_queue_pg_num) << 16) + \
		    ((txff_info->low_queue_pg_num) << 8) + txff_info->high_queue_pg_num);
	MAC_REG_W32(REG_RQPN_NPQ, ((txff_info->extra_queue_pg_num) << 16) + txff_info->normal_queue_pg_num);
	MAC_REG_W8(REG_RQPN_CTRL_HLPQ + 3, 0x80);

	/*
	adapter->sdio_fs.hiq_pg_num = txff_info->high_queue_pg_num;
	adapter->sdio_fs.miq_pg_num = txff_info->normal_queue_pg_num;
	adapter->sdio_fs.lowq_pg_num = txff_info->low_queue_pg_num;
	adapter->sdio_fs.pubq_pg_num = txff_info->pub_queue_pg_num;
	adapter->sdio_fs.exq_pg_num = txff_info->extra_queue_pg_num;
	*/

	MAC_REG_W8(REG_BNDY_LIFETIME_TAIL + 1, (u8)(txff_info->rsvd_cpu_instr_addr));

	/*bcn head0*/
	MAC_REG_W8(REG_DWBCN0_CTRL + 1, (u8)(txff_info->rsvd_bcnq_addr));
	MAC_REG_W8(REG_BNDY_LIFETIME_TAIL, (u8)(txff_info->rsvd_bcnq_addr));
	/*bcn head1*/
	MAC_REG_W8(REG_BCN_AMPDU_CTCL + 3, (u8)(txff_info->rsvd_bcnq1_addr));
	MAC_REG_W8(REG_DWBCN1_CTRL + 1, (u8)(txff_info->rsvd_bcnq1_addr));
	/*bcn head2*/
	MAC_REG_W8(REG_BCN_AMPDU_CTCL + 1, (u8)(txff_info->rsvd_bcnq2_addr));
	MAC_REG_W8(REG_DWBCN1_CTRL + 3, (u8)(txff_info->rsvd_bcnq2_addr));

	val32 = MAC_REG_R32(REG_FWHW_TXQ_HWSSN_CTRL);
	val32 |= BIT_EN_BCNQ_DL;
	val32 &= ~BIT_EN_WR_FREE_TAIL;
	MAC_REG_W32(REG_FWHW_TXQ_HWSSN_CTRL, val32);

	MAC_REG_W16(0x116, (u16)(adapter->hw_cfg_info.rx_fifo_size - C2H_PKT_BUF_8730e - 1));

	/*init LLT*/
	MAC_REG_W32(REG_AUTO_LLT, MAC_REG_R32(REG_AUTO_LLT) | BIT(16));
	cnt = 1000;
	while (MAC_REG_R32(REG_AUTO_LLT) & BIT(16)) {
		cnt--;
		if (cnt == 0) {
			return MACINITLLTFAIL;
		}
	}

	if (mode == HALMAC_TRX_MODE_DELAY_LOOPBACK) {
		transfer_mode = HALMAC_TRNSFER_LOOPBACK_DELAY;
		MAC_REG_W8(REG_NDPA_CTRL_LBK_ACQ_STOP + 1,
			   (u8)(adapter->txff_alloc.rsvd_boundary & 0xff)); //REG_WMAC_LBK_BUF_HD_V1, yx_qi
	} else if (mode == HALMAC_TRX_MODE_LOOPBACK) {
		transfer_mode = HALMAC_TRNSFER_LOOPBACK_DIRECT;
	} else {
		transfer_mode = HALMAC_TRNSFER_NORMAL;
	}

	adapter->hw_cfg_info.trx_mode = transfer_mode;
	MAC_REG_W8(REG_CR + 3, (u8)transfer_mode);

	MAC_REG_W16((REG_TRXFF_BNDY + 2), RX_DMA_BOUNDARY_8730E);

	return MACSUCCESS;
}

u32 set_trx_fifo_info_8730e(struct mac_ax_adapter *adapter,
			    enum halmac_trx_mode mode)
{
	u16 cur_pg_addr;
	u32 txff_size = TX_FIFO_SIZE_8730E;
	u32 rxff_size = RX_FIFO_SIZE_8730E;
	struct halmac_txff_allocation *info = &adapter->txff_alloc;

	if (info->rx_fifo_exp_mode == HALMAC_RX_FIFO_EXPANDING_MODE_1_BLOCK) {
		txff_size = TX_FIFO_SIZE_RX_EXPAND_1BLK_8730E;
		rxff_size = RX_FIFO_SIZE_RX_EXPAND_1BLK_8730E;
	}

	adapter->hw_cfg_info.tx_fifo_size = txff_size;
	adapter->hw_cfg_info.rx_fifo_size = rxff_size;
	info->tx_fifo_pg_num = (u16)(txff_size >> TX_PAGE_SIZE_SHIFT_8730e);

	info->rsvd_drv_pg_num = RSVD_PG_DRV_NUM;

	info->rsvd_pg_num = info->rsvd_drv_pg_num +
			    RSVD_PG_CPU_INSTRUCTION_NUM +
			    RSVD_PG_FW_TXBUF_NUM +
			    RSVD_PG_CSIBUF_NUM +
			    RSVD_PG_BCNQ_NUM +
			    RSVD_PG_BCNQ1_NUM +
			    RSVD_PG_BCNQ2_NUM +
			    RSVD_PG_WOWLAN_NUM;

	if (mode == HALMAC_TRX_MODE_DELAY_LOOPBACK) {
		info->rsvd_pg_num += RSVD_PG_DLLB_NUM;
	}

	if (info->rsvd_pg_num > info->tx_fifo_pg_num) {
		return MACTXFIFOPAGEFAIL;
	}

	info->acq_pg_num = info->tx_fifo_pg_num - info->rsvd_pg_num;
	info->rsvd_boundary = info->tx_fifo_pg_num - info->rsvd_pg_num;

	cur_pg_addr = info->tx_fifo_pg_num;
	cur_pg_addr -= RSVD_PG_WOWLAN_NUM;
	info->rsvd_wowlan_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_BCNQ2_NUM;
	info->rsvd_bcnq2_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_BCNQ1_NUM;
	info->rsvd_bcnq1_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_BCNQ_NUM;
	info->rsvd_bcnq_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_CSIBUF_NUM;
	info->rsvd_csibuf_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_FW_TXBUF_NUM;
	info->rsvd_fw_txbuf_addr = cur_pg_addr;
	cur_pg_addr -= RSVD_PG_CPU_INSTRUCTION_NUM;
	info->rsvd_cpu_instr_addr = cur_pg_addr;
	cur_pg_addr -= info->rsvd_drv_pg_num;
	info->rsvd_drv_addr = cur_pg_addr;

	if (mode == HALMAC_TRX_MODE_DELAY_LOOPBACK) {
		info->rsvd_drv_addr -= RSVD_PG_DLLB_NUM;
	}

	if (info->rsvd_boundary != info->rsvd_drv_addr) {
		return MACTXFIFOPAGEFAIL;
	}

	return MACSUCCESS;
}

u32 pg_num_parser_8730e(struct mac_ax_adapter *adapter,
			enum halmac_trx_mode mode, struct halmac_pg_num *tbl)
{
	u8 flag;
	u16 hpq_num = 0;
	u16 lpq_num = 0;
	u16 npq_num = 0;
	u16 gapq_num = 0;
	u16 expq_num = 0;
	u16 pubq_num = 0;
	u32 i = 0;

	flag = 0;
	for (i = 0; i < HALMAC_TRX_MODE_MAX; i++) {
		if (mode == tbl[i].mode) {
			hpq_num = tbl[i].hq_num;
			lpq_num = tbl[i].lq_num;
			npq_num = tbl[i].nq_num;
			expq_num = tbl[i].exq_num;
			gapq_num = tbl[i].gap_num;
			pubq_num = adapter->txff_alloc.acq_pg_num - hpq_num -
				   lpq_num - npq_num - expq_num - gapq_num;
			flag = 1;
			PLTFM_MSG_TRACE("[TRACE]%s done\n", __func__);
			break;
		}
	}

	if (flag == 0) {
		PLTFM_MSG_ERR("[ERR]trx mode!!\n");
		return MACNOTSUP;
	}

	if (adapter->txff_alloc.acq_pg_num <
	    hpq_num + lpq_num + npq_num + expq_num + gapq_num) {
		PLTFM_MSG_ERR("[ERR]acqnum = %d\n",
			      adapter->txff_alloc.acq_pg_num);
		PLTFM_MSG_ERR("[ERR]hpq_num = %d\n", hpq_num);
		PLTFM_MSG_ERR("[ERR]LPQ_num = %d\n", lpq_num);
		PLTFM_MSG_ERR("[ERR]npq_num = %d\n", npq_num);
		PLTFM_MSG_ERR("[ERR]EPQ_num = %d\n", expq_num);
		PLTFM_MSG_ERR("[ERR]gapq_num = %d\n", gapq_num);
		return MACTXFIFOPAGEFAIL;
	}

	adapter->txff_alloc.high_queue_pg_num = hpq_num;
	adapter->txff_alloc.low_queue_pg_num = lpq_num;
	adapter->txff_alloc.normal_queue_pg_num = npq_num;
	adapter->txff_alloc.extra_queue_pg_num = expq_num;
	adapter->txff_alloc.pub_queue_pg_num = pubq_num;

	return MACSUCCESS;
}

u32 init_txq_ctrl_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u8 val8;

	val8 = MAC_REG_R8(REG_FWHW_TXQ_HWSSN_CTRL);
	val8 |= (BIT(7) & (~ BIT(1)) & (~ BIT(2)));
	MAC_REG_W8(REG_FWHW_TXQ_HWSSN_CTRL, val8);

	MAC_REG_W8(REG_FWHW_TXQ_HWSSN_CTRL + 1, WLAN_TXQ_RPT_EN);

	/*not generate report once retry*/
	val8 = MAC_REG_R8(REG_FWHW_TXQ_HWSSN_CTRL + 2);
	val8 |= BIT(2) | BIT(3);
	MAC_REG_W8(REG_FWHW_TXQ_HWSSN_CTRL + 2, val8);

	return MACSUCCESS;
}

u32 init_sifs_ctrl_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS, val32 = 0;

	MAC_REG_W32(REG_RETRY_LIMIT_SIFS, WLAN_SIFS_DUR_TUNE \
		    | BIT_SRL(WLAN_RETRY_LIMIT) \
		    | BIT_LRL(WLAN_RETRY_LIMIT));
	MAC_REG_W32(REG_SIFS, WLAN_SIFS_CFG);

	val32 = MAC_REG_R32(REG_SIFS_TIMING_CTRL_CCK);
	val32 = BIT_SET_R2T_SIFS_CCK(val32, WLAN_SIFS_CCK_R2T);
	val32 = BIT_SET_T2T_SIFS_CCK(val32, WLAN_SIFS_CCK_T2T);
	MAC_REG_W32(REG_SIFS_TIMING_CTRL_CCK, val32);

	val32 = MAC_REG_R32(REG_SIFS_TIMING_CTRL_OFDM);
	val32 = BIT_SET_T2T_SIFS_OFDM(val32, WLAN_SIFS_OFDM_T2T);
	val32 = BIT_SET_R2T_SIFS_OFDM(val32, WLAN_SIFS_OFDM_R2T);
	MAC_REG_W32(REG_SIFS_TIMING_CTRL_OFDM, val32);

	return MACSUCCESS;
}

u32 init_rate_fallback_ctrl_8730e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(REG_DARFRC0_L, WLAN_DATA_RATE_FB_CNT_1_4);
	MAC_REG_W32(REG_DARFRC0_H, WLAN_DATA_RATE_FB_CNT_5_8);
	MAC_REG_W32(REG_RARFRC_H, WLAN_RTS_RATE_FB_CNT_5_8);

	return MACSUCCESS;
}

void cfg_mac_clk_8730e(struct mac_ax_adapter *adapter)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	//mac clk 40M
	val32 = MAC_REG_R32(REG_CLKDIV_CKSEL);
	val32 = BIT_SET_MAC_CLK_SEL(val32, 0x01);
	MAC_REG_W32(REG_CLKDIV_CKSEL, val32);

	//sec clk 80M
	val32 = MAC_REG_R32(REG_CLKDIV_CKSEL);
	val32 = BIT_SET_SEC_CLK_SEL(val32, 0x01);
	MAC_REG_W32(REG_CLKDIV_CKSEL, val32);

	MAC_REG_W8(REG_BCNERR_TSF_CTRL, MAC_CLK_SPEED);
	MAC_REG_W8(REG_USTIME_EDCA, MAC_CLK_SPEED);
	MAC_REG_W8(REG_WSEC_OPTION, MAC_CLK_SPEED);
}

void cfg_32k_clk_8730e(struct mac_ax_adapter *adapter)
{
	u32 val32;
	u8 val8;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	/*32kselect sdm32k from mac external(32.768)*/
	val8 = MAC_REG_R8(REG_WL_CLK_CTRL);
	val8 &= ~BIT_CKSL_CK32K;
	val8 |= (BIT_WL_FQSEL_CK32K | BIT_WL_CKMCU_EN | BIT_WL_CKSEC_EN | BIT_WL_CKMAC_EN);
	MAC_REG_W8(REG_WL_CLK_CTRL, val8);

	/*32k Enable*/
	val8 = MAC_REG_R8(REG_WL_CLK_CTRL);
	val8 |= BIT_WL_CK32K_EN;
	MAC_REG_W8(REG_WL_CLK_CTRL, val8);

	/*CLK for GTimer:Unit: 32us, use 32k clk, 1cycle =32.768us*/
	MAC_REG_W32(REG_TCUNIT_BASE, 0x01);
}

u32 init_low_pwr_8730e(struct mac_ax_adapter *adapter)
{
	u16 value16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	/*RXGCK FIFO threshold CFG*/
	value16 = (MAC_REG_R16(REG_RXPSF_CTRL + 2) & 0xF00F);
	value16 |= BIT(10) | BIT(8) | BIT(6) | BIT(4);
	MAC_REG_W16(REG_RXPSF_CTRL + 2, value16);

	/*invalid_pkt CFG*/
	value16 = 0;
	value16 = BIT_SET_RXPSF_PKTLENTHR(value16, 1);
	value16 |= BIT_RXPSF_CTRLEN | BIT_RXPSF_VHTCHKEN | BIT_RXPSF_HTCHKEN
		   | BIT_RXPSF_OFDMCHKEN | BIT_RXPSF_CCKCHKEN
		   | BIT_RXPSF_OFDMRST;

	MAC_REG_W16(REG_RXPSF_CTRL, value16);
	MAC_REG_W32(REG_RXPSF_TYPE_CTRL, 0);

	return MACSUCCESS;
}

u32 enable_port_8730e(struct mac_ax_adapter *adapter, u8 port, bool en)
{
	u32 val32 = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	if (port >= MAC_AX_PORT_NUM - 1) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	val32 = MAC_REG_R32(REG_MACID_H);
	switch (port) {
	case MAC_AX_PORT_0:
		if (en) {
			val32 |= BIT_EN_PORT0;
		} else {
			val32 &= ~BIT_EN_PORT0;
		}
		break;
	case MAC_AX_PORT_1:
		if (en) {
			val32 |= BIT_EN_PORT1;
		} else {
			val32 &= ~BIT_EN_PORT1;
		}
		break;
	default:
		return MACNOITEM;
	}
	MAC_REG_W32(REG_MACID_H, val32);
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_mac_addr_8730e(struct mac_ax_adapter *adapter, u8 port,
		       union halmac_wlan_addr *addr)
{
	u32 offset;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	if (port >= MAC_AX_PORT_NUM - 1) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}

	switch (port) {
	case MAC_AX_PORT_0:
		offset = REG_MACID;
		break;
	case MAC_AX_PORT_1:
		offset = REG_MACID1;
		break;
	default:
		return MACNOITEM;
	}
	MAC_REG_W32(offset, le32_to_cpu(addr->addr_l_h.low));
	MAC_REG_W16(offset + 4, le16_to_cpu(addr->addr_l_h.high));
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_bssid_8730e(struct mac_ax_adapter *adapter, u8 port,
		    union halmac_wlan_addr *addr)
{
	u32 offset;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 *mac = (u8 *)addr;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	if (port >= MAC_AX_PORT_NUM - 1) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}
	switch (port) {
	case MAC_AX_PORT_0:
		offset = REG_BSSID;
		break;
	case MAC_AX_PORT_1:
		offset = REG_BSSID1;
		break;
	default:
		return MACNOITEM;
	}
	MAC_REG_W32(offset, le32_to_cpu(addr->addr_l_h.low));
	MAC_REG_W16(offset + 4, le16_to_cpu(addr->addr_l_h.high));
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_transmitter_addr_8730e(struct mac_ax_adapter *adapter, u8 port,
			       union halmac_wlan_addr *addr)
{
	u32 offset;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	if (port >= MAC_AX_PORT_NUM - 1) {
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACPORTCFGPORT;
	}
	switch (port) {
	case MAC_AX_PORT_0:
		offset = REG_TRANSMIT_ADDRSS_0;
		break;
	case MAC_AX_PORT_1:
		offset = REG_TRANSMIT_ADDRSS_1;
		break;
	default:
		return MACNOITEM;
	}
	MAC_REG_W32(offset, le32_to_cpu(addr->addr_l_h.low));
	MAC_REG_W16(offset + 4, le16_to_cpu(addr->addr_l_h.high));
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_net_type_8730e(struct mac_ax_adapter *adapter, u8 port,
		       enum mac_ax_net_type net_type)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 value8 = 0;
	u32 value32 = 0;
	u8 net_type_tmp = 0;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	switch (port) {
	case MAC_AX_PORT_0:
		net_type_tmp = net_type;
		value8 = ((MAC_REG_R8(REG_CR + 2) & 0xFC) | net_type_tmp);
		MAC_REG_W8(REG_CR + 2, value8);
		break;
	case MAC_AX_PORT_1:
		net_type_tmp = (net_type << 2);
		value8 = ((MAC_REG_R8(REG_CR + 2) & 0xF3) | net_type_tmp);
		MAC_REG_W8(REG_CR + 2, value8);
		break;
	default:
		break;
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_sta_aid_8730e(struct mac_ax_adapter *adapter, u8 port, u16 aid)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);
	switch (port) {
	case MAC_AX_PORT_0:
		val32 = MAC_REG_R32(REG_BCN_PSR_RPT0);
		val32 = BIT_SET_PS_AID_0(val32, aid);
		MAC_REG_W32(REG_BCN_PSR_RPT0 , val32);

		/* config mac aid for 11ax */
		val32 = MAC_REG_R32(REG_AID);
		val32 = BIT_SET_R_MAC_AID12_0(val32, aid);
		MAC_REG_W32(REG_AID, val32);
		break;
	case MAC_AX_PORT_1:
		val32 = MAC_REG_R32(REG_BCN_PSR_RPT1);
		val32 = BIT_SET_PS_AID_1(val32, aid);
		MAC_REG_W32(REG_BCN_PSR_RPT1 , val32);

		/* config mac aid for 11ax */
		val32 = MAC_REG_R32(REG_AID);
		val32 = BIT_SET_R_MAC_AID12_1(val32, aid);
		MAC_REG_W32(REG_AID, val32);
		break;
	default:
		PLTFM_MSG_ERR("[ERR] invalid port idx %d\n", port);
		return MACHWNOSUP;
	}
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}

u32 cfg_macid_8730e(struct mac_ax_adapter *adapter,
		    struct rtw_phl_stainfo_t *sta)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 addr = 0;
	u8 mac_addr[ETH_ALEN] = {0};
	u8 crc5 = 0;
	u32 entry = 0;
	u32 offset = 0, idx = 0;
	u8 entry_data[CRC5_ENTRY_SIZE_8730E + 1] = {0};
	u16 entry_val16 = 0;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	memcpy(mac_addr, sta->mac_addr, ETH_ALEN);

	/*write mac addr to control info*/
	addr = CTRL_INFO_BASE_8730E + CTRL_INFO_ENTRY_SIZE_8730E * sta->macid + 32;
	PLTFM_MEM_W(addr, mac_addr, ETH_ALEN);

	/*write mac addr to crc5*/
	crc5 = (u8)_crc5_generate(mac_addr, ETH_ALEN);
	/* each entry contain 12 macid*/
	entry = sta->macid / 12;
	/* offset in each entry(each macid contain 5 bits) */
	offset = 5 * (sta->macid % 12);
	idx = offset / 8;
	offset = offset % 8;
	/* each entry is 64 bits*/
	addr = CRC5_BASE_8730E + entry * CRC5_ENTRY_SIZE_8730E;
	PLTFM_MEM_R(addr, entry_data, CRC5_ENTRY_SIZE_8730E);
	entry_data[8] = 0;

	entry_val16 = ((u16)entry_data[idx + 1] << 8) | entry_data[idx];
	entry_val16 &= ~(((u16)0x1F) << offset);
	entry_val16 |= ((u16)crc5 << offset);
	entry_data[idx] = (u8)entry_val16;
	entry_data[idx + 1] = (u8)(entry_val16 >> 8);
	/* set entry valid bit*/
	entry_data[7] = entry_data[7] | BIT5;
	PLTFM_MEM_W(addr, entry_data, CRC5_ENTRY_SIZE_8730E);

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return MACSUCCESS;
}
#endif /* #if MAC_AX_8730E_SUPPORT */
