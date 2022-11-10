/******************************************************************************
 *
 * Copyright(c) 2015 - 2018 Realtek Corporation.
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
#ifndef _RTL8730A_H_
#define _RTL8730A_H_

#include <drv_types.h>		/* PADAPTER */
#include <rtw_rf.h>		/* CHANNEL_WIDTH */
#include <rtw_xmit.h>		/* struct pkt_attrib, struct xmit_frame */
#include <rtw_recv.h>		/* struct recv_frame */
#include <hal_intf.h>		/* HAL_DEF_VARIABLE */
#include "hal8730a_fw.h"	/* FW array */

#define DRIVER_EARLY_INT_TIME_8730A 0x05
#define BCN_DMA_ATIME_INT_TIME_8730A 0x02

/* 8730A TXDESC_SIZE is 16 + 32 (non-burst mode and packet machine is disabled)
 * where 32 is TX desc part 2 that is included in packet offset.
 * Since the offset is used by 8730A TX, it is safe to leave TXDESC_SIZE as 48
 * for common code.
 */
#define TX_DESC_PKT_OFFSET_8730A (0) /* SHL 3 bit in tx desc */

/* rtl8730a_ops.c */
struct hw_port_reg {
	u32 net_type; /*reg_offset*/
	u8 net_type_shift;
	u32 macaddr; /*reg_offset*/
	u32 bssid; /*reg_offset*/
	u32 ps_aid; /*reg_offset*/
	u32 ta; /*reg_offset*/
};


/* rtl8730a_halinit.c */
void rtl8730a_init_hal_spec(PADAPTER);
u32 rtl8730a_power_on(PADAPTER);
void rtl8730a_power_off(PADAPTER);
u8 rtl8730a_hal_init(PADAPTER);
u8 rtl8730a_mac_verify(PADAPTER);
void rtl8730a_init_misc(PADAPTER padapter);
u32 rtl8730a_init(PADAPTER);
u32 rtl8730a_deinit(PADAPTER);
void rtl8730a_init_default_value(PADAPTER);

/* rtl8730a_mac.c */
/* RXERR_RPT */
enum rx_rpt_type {
	OFDM_MPDU_OK = 0,	/* 0 */
	OFDM_MPDU_FAIL,
	OFDM_FALSE_ALARM,
	CCK_MPDU_OK,
	CCK_MPDU_FAIL,
	CCK_FALSE_ALARM,
	HT_MPDU_OK,
	HT_MPDU_FAIL,
	HT_PPDU,
	RX_FULL_DROP,		/* 10 */
	FWFF_FULL_DROP,
	VHT_SU_MPDU_OK = 16,	/* 16 */
	VHT_SU_MPDU_FAIL,
	VHT_SU_PPDU,
	VHT_FALSE_ALARM,
	VHT_MU_MPDU_OK,		/* 20 */
	VHT_MU_MPDU_FAIL,
	VHT_MU_PPDU		/* 22 */
};

u8 rtl8730a_rcr_config(PADAPTER, u32 rcr);
u8 rtl8730a_rx_ba_ssn_appended(PADAPTER);
u8 rtl8730a_rx_fcs_append_switch(PADAPTER, u8 enable);
u8 rtl8730a_rx_fcs_appended(PADAPTER);
u8 rtl8730a_rx_tsf_addr_filter_config(PADAPTER p, u8 config);
u8 rtl8730a_get_rx_drv_info_size(struct _ADAPTER *a);
u32 rtl8730a_get_tx_desc_size(struct _ADAPTER *a);
u32 rtl8730a_get_rx_desc_size(struct _ADAPTER *a);
u16 rtl8730a_rx_report_get(struct _ADAPTER *a, enum rx_rpt_type type);
void rtl8730a_rx_report_reset(struct _ADAPTER *a, enum rx_rpt_type type);

/* rtl8730a_ops.c */
u8 rtl8730a_read_efuse(PADAPTER);
void rtl8730a_run_thread(PADAPTER);
void rtl8730a_cancel_thread(PADAPTER);
u8 rtl8730a_sethwreg(PADAPTER, u8 variable, u8 *pval);
void rtl8730a_gethwreg(PADAPTER, u8 variable, u8 *pval);
u8 rtl8730a_sethaldefvar(PADAPTER, HAL_DEF_VARIABLE, void *pval);
u8 rtl8730a_gethaldefvar(PADAPTER, HAL_DEF_VARIABLE, void *pval);
void rtl8730a_set_hal_ops(PADAPTER);

/* tx */
void rtl8730a_init_xmit_priv(_adapter *adapter);
void rtl8730a_fill_txdesc_sectype(struct pkt_attrib *, u8 *ptxdesc);
void rtl8730a_fill_txdesc_vcs(PADAPTER, struct pkt_attrib *, u8 *ptxdesc);
void rtl8730a_fill_txdesc_phy(PADAPTER, struct pkt_attrib *, u8 *ptxdesc);
void rtl8730a_fill_txdesc_force_bmc_camid(struct pkt_attrib *, u8 *ptxdesc);
void rtl8730a_fill_txdesc_bmc_tx_rate(struct pkt_attrib *pattrib, u8 *ptxdesc);
u8 rtl8730a_bw_mapping(PADAPTER, struct pkt_attrib *);
u8 rtl8730a_sc_mapping(PADAPTER, struct pkt_attrib *);
void rtl8730a_fill_txdesc_bf(struct xmit_frame *, u8 *desc);
void rtl8730a_fill_txdesc_mgnt_bf(struct xmit_frame *, u8 *desc);
void rtl8730a_cal_txdesc_chksum(PADAPTER, u8 *ptxdesc);
void rtl8730a_update_txdesc(struct xmit_frame *, u8 *pbuf);
void rtl8730a_dbg_dump_tx_desc(PADAPTER, int frame_tag, u8 *ptxdesc);

/* rx */
void rtl8730a_rxdesc2attribute(struct rx_pkt_attrib *a, u8 *desc);
void rtl8730a_query_rx_desc(union recv_frame *, u8 *pdesc);

/* rtl8730a_cmd.c */
s32 rtl8730a_fillh2ccmd(PADAPTER, u8 id, u32 buf_len, u8 *pbuf);
void _rtl8730a_set_FwPwrMode_cmd(PADAPTER adapter, u8 psmode, u8 rfon_ctrl);
void rtl8730a_set_FwPwrMode_cmd(PADAPTER adapter, u8 psmode);
void rtl8730a_set_FwPwrMode_rfon_ctrl_cmd(PADAPTER adapter, u8 rfon_ctrl);

void rtl8730a_set_FwPwrModeInIPS_cmd(PADAPTER adapter, u8 cmd_param);
#ifdef CONFIG_WOWLAN
void rtl8730a_set_fw_pwrmode_inips_cmd_wowlan(PADAPTER padapter, u8 ps_mode);
#endif /* CONFIG_WOWLAN */
void rtl8730a_req_txrpt_cmd(PADAPTER, u8 macid);
void rtl8730a_c2h_handler(PADAPTER, u8 *pbuf, u16 length);
#ifdef CONFIG_WOWLAN
void rtl8730a_set_fw_pwrmode_inips_cmd_wowlan(PADAPTER padapter, u8 ps_mode);
#endif
void rtl8730a_c2h_handler_no_io(PADAPTER, u8 *pbuf, u16 length);

#ifdef CONFIG_LPS_PWR_TRACKING
void rtl8730a_set_fw_thermal_rpt_cmd(_adapter *adapter, u8 enable, u8 thermal_value);
void rtw_lps_pwr_tracking(_adapter *adapter, u8 thermal_value);
#endif

#ifdef CONFIG_BT_COEXIST
void rtl8730a_download_BTCoex_AP_mode_rsvd_page(PADAPTER);
#endif /* CONFIG_BT_COEXIST */

/* rtl8730a_phy.c */
u8 rtl8730a_phy_init_mac_register(PADAPTER);
u8 rtl8730a_phy_init(PADAPTER);
void rtl8730a_phy_init_dm_priv(PADAPTER);
void rtl8730a_phy_deinit_dm_priv(PADAPTER);
void rtl8730a_phy_init_haldm(PADAPTER);
void rtl8730a_phy_haldm_watchdog(PADAPTER);
u32 rtl8730a_read_bb_reg(PADAPTER, u32 addr, u32 mask);
void rtl8730a_write_bb_reg(PADAPTER, u32 addr, u32 mask, u32 val);
u32 rtl8730a_read_rf_reg(PADAPTER adapter, enum rf_path path, u32 addr, u32 mask);
void rtl8730a_write_rf_reg(PADAPTER adapter, enum rf_path path, u32 addr, u32 mask, u32 val);
void rtl8730a_set_channel_bw(PADAPTER adapter, u8 center_ch, enum channel_width, u8 offset40, u8 offset80);
void rtl8730a_set_tx_power_level(PADAPTER, u8 channel);
void rtl8730a_set_txpwr_done(_adapter *adapter);
void rtl8730a_set_tx_power_index(PADAPTER adapter, u32 powerindex, enum rf_path rfpath, u8 rate);
void rtl8730a_notch_filter_switch(PADAPTER, bool enable);
#ifdef CONFIG_BEAMFORMING
void rtl8730a_phy_bf_init(PADAPTER);
void rtl8730a_phy_bf_enter(PADAPTER, struct sta_info *);
void rtl8730a_phy_bf_leave(PADAPTER, u8 *addr);
void rtl8730a_phy_bf_set_gid_table(PADAPTER, struct beamformer_entry *);
void rtl8730a_phy_bf_set_csi_report(PADAPTER, struct _RT_CSI_INFO *);
void rtl8730a_phy_bf_sounding_status(PADAPTER, u8 status);
#endif /* CONFIG_BEAMFORMING */

struct afrf_info {
	u8 arfr_cck_en;
	u8 arfr_ofdm_en;
	u8 arfr_ht_en;
};

#define BCN_VALID_ALL	((void *)0xFFFF)
#define BCN_VALID_PORT0	0xF0
#define BCN_SEL_PORT0	((void *)0xFF00)
#define MASK_ALL_PORTS	0x1F	/* for 5 ports */

#endif /* _RTL8730A_H_ */
