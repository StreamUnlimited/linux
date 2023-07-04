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

#include "fwcmd.h"
#include "mcc.h"
#include "mac_priv.h"
#include "twt.h"
#include "mac/pltfm_cfg.h"

#if MAC_AX_FEATURE_HV
#include "../hv_ax/dbgpkg_hv.h"
#endif

/* 8852A/8852B: the format of H2C/DLFW descriptor: WD Body
 * 8852C: the format of H2C/DLFW descriptor: RX Descriptor
 * WD body max len: 24 bytes
 * RX descriptor max len: 32 bytes
 * We use the max RX descriptor size as the header size
 * WD_BODY_LEN_V1 = RX descriptor max len = 32 bytes
 */

#define H2CB_CMD_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN_V1)
#define H2CB_CMD_SIZE		(H2C_CMD_LEN - FWCMD_HDR_LEN)
#define H2CB_CMD_QLEN		8

#define H2CB_DATA_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN_V1)
#define H2CB_DATA_SIZE		(H2C_DATA_LEN - FWCMD_HDR_LEN)
#define H2CB_DATA_QLEN		4

#define H2CB_LONG_DATA_HDR_SIZE	(FWCMD_HDR_LEN + WD_BODY_LEN)
#define H2CB_LONG_DATA_SIZE	(H2C_LONG_DATA_LEN - FWCMD_HDR_LEN)
#define H2CB_LONG_DATA_QLEN	1

#define FWCMD_WQ_MAX_JOB_NUM	5

#define FWCMD_LMT		12

#define MAC_AX_H2C_LMT_EN	0

#define FWCMD_H2CREG_BYTE0_SH 0
#define FWCMD_H2CREG_BYTE0_MSK 0xFF
#define FWCMD_H2CREG_BYTE1_SH 8
#define FWCMD_H2CREG_BYTE1_MSK 0xFF
#define FWCMD_H2CREG_BYTE2_SH 16
#define FWCMD_H2CREG_BYTE2_MSK 0xFF
#define FWCMD_H2CREG_BYTE3_SH 24
#define FWCMD_H2CREG_BYTE3_MSK 0xFF

#define BCN_GRPIE_OFST_EN BIT(7)

#define SCANOFLD_RSP_EVT_ID 0
#define SCANOFLD_RSP_EVT_PARSE 0

u32 c2h_rx_dbg_hdl(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		   struct rtw_c2h_info *info)
{
	PLTFM_MSG_ERR("[ERR]%s: FW encounter Rx problem!\n", __func__);

	return MACSUCCESS;
}

u32 c2h_wps_rpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		struct rtw_c2h_info *info)
{
	PLTFM_MSG_TRACE("recevied wps report\n");
	return MACSUCCESS;
}

static u32 c2h_misc_ccxrpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
			   struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

int c2h_field_parsing(int id)
{

	switch (id) {
	case C2H_DBG:
	case C2H_LB:
	case C2H_DEFEATURE_DBG:
	case C2H_DEFEATURE_RSVD:
	case C2H_EXTEND:
		return C2H_CAT_TEST;

	case C2H_TXBF:
	case C2H_CCX_TX_RPT:
	case C2HID_AP_REQ_TXRPT:
	case C2H_FW_SCAN_COMPLETE:
	case C2HID_SPE_STATIS:
	case C2H_FW_CHNL_SWITCH_COMPLETE:
	case C2H_IQK_FINISH:
	case C2H_MAILBOX_STATUS:
	case C2H_P2P_RPORT:
	case C2H_MCC:
	case C2H_MAC_HIDDEN_RPT:
	case C2H_MAC_HIDDEN_RPT_2:
	case C2H_BCN_EARLY_RPT:
	case C2H_TSF_RPT:
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && (CONFIG_UNSUPPORT_PLCPHDR_RPT)
	case C2H_PLCPHDR_RPT:
#endif
		return C2H_CAT_MAC;
	case C2H_RA_RPT:
	case C2H_RA_PARA_RPT:
		return C2H_CAT_OUTSRC;

	case C2H_WLAN_INFO:
	case C2H_BT_INFO:
	case C2H_BT_MP_INFO:
		return C2H_CAT_BTCOEX;
	default:
		break;
	}
	return -EINVAL;
}

bool rtw_hal_c2h_pkt_hdr_parse(struct mac_ax_adapter *adapter, u8 *buf, u16 len, u8 *id, u8 *seq, u8 *plen, u8 **payload)
{
	bool ret = _FAIL;

	if (!buf || len > 256 || len < 3) {
		goto exit;
	}

	*id = C2H_ID_88XX(buf);
	*seq = C2H_SEQ_88XX(buf);
	*plen = len - 2;
	*payload = C2H_PAYLOAD_88XX(buf);
	ret = _SUCCESS;

exit:
	return ret;
}

u32 mac_process_c2h(struct mac_ax_adapter *adapter, u8 *buf, u32 len, u8 *c2h_ret)
{
	u8 parse_fail = 0;
	u8 bypass = 0;
	s32 ret = _FAIL;
	u8 id, seq, plen;
	u8 *payload;
	struct rtw_c2h_info *c2h = (struct rtw_c2h_info *)c2h_ret;

	ret = rtw_hal_c2h_pkt_hdr_parse(adapter, buf, len, &id, &seq, &plen, &payload);
	c2h->c2h_cat = c2h_field_parsing(id);
	switch (id) {
	case C2H_BT_INFO:
		c2h->c2h_class = BTC_CLASS_FEV;
		c2h->c2h_func = BTC_FEV_BT_INFO;
		break;
	case C2H_BT_MP_INFO:
		c2h->c2h_class = BTC_CLASS_FEV;
		c2h->c2h_func = BTC_FEV_REPORT;
		break;
	case C2H_WLAN_INFO:
		c2h->c2h_class = BTC_CLASS_FEV;
		c2h->c2h_func = BTC_FEV_WL_DBG_INFO;
		break;
	default:
		c2h->c2h_class = id;
		c2h->c2h_func = 0;
		break;
	}

	c2h->content = payload;
	c2h->content_len = plen;
	c2h->h2c_return = ret;
	c2h->h2c_seq = seq;

	return ret;
}

/* km */
u32 rtl8730e_set_wowlan_ctrl_cmd(struct mac_ax_adapter *adapter, enum mac_ax_wow_ctrl w_c)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;

	u8 u1H2CWoWlanCtrlParm[H2C_WOWLAN_LEN] = {0};
	u8 discont_wake = 1, gpionum = 0, gpio_dur = 0;
	u8 hw_unicast = 0, gpio_pulse_cnt = 0, gpio_pulse_en = 0;
	u8 sdio_wakeup_enable = 1;
	u8 gpio_high_active = 0; //0: low active, 1: high active
	u8 magic_pkt = 0;
	u8 gpio_unit = 0; /*0: 64ns, 1: 8ms*/
	u8 ret = _FAIL;
	u8 enable = !w_c;

	/* change_unit not supported now. */
	u8 change_unit = 0;

#ifdef CONFIG_GPIO_WAKEUP
	gpionum = WAKEUP_GPIO_IDX;
	sdio_wakeup_enable = 0;
#endif //CONFIG_GPIO_WAKEUP

	if (w_c == MAC_AX_WOW_ENTER) {
		enable = 1;
	} else {
		enable = 0;
	}

	magic_pkt = enable;
	hw_unicast = 0;

#if 0
	struct pwrctrl_priv *ppwrpriv = adapter_to_pwrctl(adapter);
	if (psecpriv->dot11PrivacyAlgrthm == _WEP40_ || psecpriv->dot11PrivacyAlgrthm == _WEP104_) {
		hw_unicast = 1;
	} else {
		hw_unicast = 0;
	}
#endif

	PLTFM_MSG_TRACE("%s(): enable=%d change_unit=%d\n",
			 __func__, enable, change_unit);

	/* time = (gpio_dur/2) * gpio_unit, default:256 ms */
	if (enable && change_unit) {
		gpio_dur = 0x40;
		gpio_unit = 1;
		gpio_pulse_en = 1;
	}

#ifdef CONFIG_PLATFORM_ARM_RK3188
	if (enable) {
		gpio_pulse_en = 1;
		gpio_pulse_cnt = 0x04;
	}
#endif

	SET_H2CCMD_WOWLAN_FUNC_ENABLE(u1H2CWoWlanCtrlParm, enable);
	SET_H2CCMD_WOWLAN_PATTERN_MATCH_ENABLE(u1H2CWoWlanCtrlParm, 0);
	SET_H2CCMD_WOWLAN_MAGIC_PKT_ENABLE(u1H2CWoWlanCtrlParm, magic_pkt);
	SET_H2CCMD_WOWLAN_UNICAST_PKT_ENABLE(u1H2CWoWlanCtrlParm, 1);
	SET_H2CCMD_WOWLAN_ALL_PKT_DROP(u1H2CWoWlanCtrlParm, 0);
	SET_H2CCMD_WOWLAN_GPIO_ACTIVE(u1H2CWoWlanCtrlParm, gpio_high_active);
#ifndef CONFIG_GTK_OL
	SET_H2CCMD_WOWLAN_REKEY_WAKE_UP(u1H2CWoWlanCtrlParm, enable);
#endif
	SET_H2CCMD_WOWLAN_DISCONNECT_WAKE_UP(u1H2CWoWlanCtrlParm, discont_wake);
	SET_H2CCMD_WOWLAN_GPIONUM(u1H2CWoWlanCtrlParm, gpionum);
	SET_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(u1H2CWoWlanCtrlParm, sdio_wakeup_enable);

	SET_H2CCMD_WOWLAN_GPIO_DURATION(u1H2CWoWlanCtrlParm, gpio_dur);
	SET_H2CCMD_WOWLAN_CHANGE_UNIT(u1H2CWoWlanCtrlParm, gpio_unit);

	SET_H2CCMD_WOWLAN_GPIO_PULSE_EN(u1H2CWoWlanCtrlParm, gpio_pulse_en);
	SET_H2CCMD_WOWLAN_GPIO_PULSE_COUNT(u1H2CWoWlanCtrlParm, gpio_pulse_cnt);

	ret = rtw_hal_mac_send_h2c_ameba(hal_com, H2C_WOWLAN, H2C_WOWLAN_LEN, u1H2CWoWlanCtrlParm);

	return ret;
}



u32 mac_outsrc_h2c_common(struct mac_ax_adapter *adapter,
			  struct rtw_g6_h2c_hdr *hdr, u32 *pvalue)
{
	/* Confirmed: bb will not use h2c to write rf regs. */
	return 0;
}

u32 mac_fw_log_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_fw_log *log_cfg)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_host_getpkt_h2c(struct mac_ax_adapter *adapter, u8 macid, u8 pkttype)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

#if MAC_AX_PHL_H2C
u32 __ie_cam_set_cmd(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb,
		     struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

#else
u32 __ie_cam_set_cmd(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb,
		     struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}
#endif
u32 mac_ie_cam_upd(struct mac_ax_adapter *adapter,
		   struct mac_ax_ie_cam_cmd_info *info)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_send_h2c_hmebox(struct mac_ax_adapter *adapter, u8 *h2c)
{
#define MAX_H2C_BOX_NUMS 4
#define MESSAGE_BOX_SIZE 4
#define EX_MESSAGE_BOX_SIZE 4
#define MAC_AX_H2CHMEBOX_CNT 100
#define MAC_AX_H2CHMEBOX_MS 1
	u32 cnt = MAC_AX_H2CHMEBOX_CNT;
	u8 len, byte0, byte1, val;
	u8 h2c_box_num = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct fwcmd_h2creg h2creg;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_h2creg_offset *h2creg_offset;
	struct mac_ax_fw_info *fw = &adapter->fw_info;
	u32 h2c_cmd = 0;
	u32 h2c_cmd_ex = 0;
	u32 msgbox_addr = 0;
	u32 msgbox_ex_addr = 0;

	if (!h2c) {
		return MACSUCCESS;
	}

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		PLTFM_MSG_ERR("FW is not ready\n");
		return MACFWNONRDY;
	}

	if (adapter->sm.mac_rdy != MAC_AX_MAC_RDY) {
		PLTFM_MSG_TRACE("MAC is not ready\n");
		adapter->stats.h2c_reg_uninit++;
	}

	h2creg_offset = p_ops->get_h2creg_offset(adapter);
	if (!h2creg_offset) {
		PLTFM_MSG_ERR("Get H2CREG offset FAIL\n");
		return MACNPTR;
	}

	PLTFM_MUTEX_LOCK(&adapter->fw_info.msg_reg);
	h2c_box_num = fw->last_hmebox_num;

	do {
		if (!(MAC_REG_R8(h2creg_offset->ctrl) & BIT(h2c_box_num))) {
			break;
		}
		PLTFM_DELAY_MS(MAC_AX_H2CHMEBOX_MS);
		cnt--;
	} while (cnt);

	if (!cnt) {
		PLTFM_MSG_WARN("%s, reg_1cc(%x), msg_box(%d)...\n", __func__, \
			       MAC_REG_R8(h2creg_offset->ctrl), h2c_box_num);
		PLTFM_MSG_ERR("fw read cmd failed...\n");
		PLTFM_MUTEX_UNLOCK(&adapter->fw_info.msg_reg);
		return MACPOLLTO;
	}

	/* Write Ext command. data 3~6. */
	msgbox_ex_addr = h2creg_offset->data1 + (h2c_box_num * EX_MESSAGE_BOX_SIZE);
	PLTFM_MEMCPY((u8 *)(&h2c_cmd_ex), h2c + 4, EX_MESSAGE_BOX_SIZE);
	h2c_cmd_ex = le32_to_cpu(h2c_cmd_ex);
	MAC_REG_W32(msgbox_ex_addr, h2c_cmd_ex);

	/* Write command. CMD ID + data 0~2. */
	msgbox_addr = h2creg_offset->data0 + (h2c_box_num * MESSAGE_BOX_SIZE);
	PLTFM_MEMCPY((u8 *)(&h2c_cmd), h2c, 4);
	h2c_cmd = le32_to_cpu(h2c_cmd);
	MAC_REG_W32(msgbox_addr, h2c_cmd);

	/* update last msg box number */
	fw->last_hmebox_num = (h2c_box_num + 1) % MAX_H2C_BOX_NUMS;

	PLTFM_MUTEX_UNLOCK(&adapter->fw_info.msg_reg);

	return MACSUCCESS;
}

u32 _mac_send_h2creg(struct mac_ax_adapter *adapter,
		     struct mac_ax_h2creg_info *h2c)
{

	return MACSUCCESS;
}

static u32 get_wps_rpt_event_id(struct mac_ax_adapter *adapter,
				struct rtw_c2h_info *c2h,
				enum phl_msg_evt_id *id,
				u8 *c2h_info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

static u32 get_bcn_resend_event(struct mac_ax_adapter *adapter,
				struct rtw_c2h_info *c2h,
				enum phl_msg_evt_id *id,
				u8 *c2h_info)
{
	*id = MSG_EVT_BCN_RESEND;

	return MACSUCCESS;
}

static u32 get_tsf32_togl_rpt_event(struct mac_ax_adapter *adapter,
				    struct rtw_c2h_info *c2h,
				    enum phl_msg_evt_id *id,
				    u8 *c2h_info)
{
	*id = HAL_C2H_EV_MAC_TSF32_TOG;

	return MACSUCCESS;
}

static u32 get_fw_rx_dbg_event(struct mac_ax_adapter *adapter,
			       struct rtw_c2h_info *c2h,
			       enum phl_msg_evt_id *id,
			       u8 *c2h_info)
{
	*id = MSG_EVT_DBG_RX_DUMP;

	return MACSUCCESS;
}

static u32 get_scanofld_event(struct mac_ax_adapter *adapter, struct rtw_c2h_info *c2h,
			      enum phl_msg_evt_id *id, u8 *c2h_info)
{
	printk("%s: function not supported.", __FUNCTION__);
	return 0;
}

u32 mac_get_c2h_event(struct mac_ax_adapter *adapter,
		      struct rtw_c2h_info *c2h,
		      enum phl_msg_evt_id *id,
		      u8 *c2h_info)
{
	struct c2h_event_id_proc *proc;
	/* write specific handler later. */
	u32(*hdl)(struct mac_ax_adapter * adapter, struct rtw_c2h_info * c2h,
		  enum phl_msg_evt_id * id, u8 * c2h_info) = NULL;

	/*for C2H ack bit no need to process*/
	if (c2h->type_done_ack == 1 || c2h->type_rec_ack == 1) {
		return MACSUCCESS;
	}

	/* Deal with specific mac C2H cmd. */

	return MACSUCCESS;
}
