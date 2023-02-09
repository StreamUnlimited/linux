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

#ifndef _MAC_AX_FW_CMD_H_
#define _MAC_AX_FW_CMD_H_

#include "../type.h"
#include "../fw_ax/inc_hdr/fwcmd_intf.h"
#include "fwcmd_intf_f2p.h"
#include "fwcmd_intf_f2p_v1.h"
#include "trx_desc.h"
#include "fwofld.h"
#include "p2p.h"
#include "flash.h"
#include "dbg_cmd.h"

#define FWCMD_H2C_MAX_SIZE (8)

#define FWCMD_HDR_LEN		8
#define C2HREG_HDR_LEN 2
#define H2CREG_HDR_LEN 2
#define C2HREG_CONTENT_LEN 14
#define H2CREG_CONTENT_LEN 14
#define C2HREG_LEN (C2HREG_HDR_LEN + C2HREG_CONTENT_LEN)
#define H2CREG_LEN (H2CREG_HDR_LEN + H2CREG_CONTENT_LEN)

#define H2C_CMD_LEN		64
#define H2C_DATA_LEN		256
#define H2C_LONG_DATA_LEN	2048

#define SET_FWCMD_ID(_t, _ca, _cl, _f)                                         \
		(SET_WORD(_t, H2C_HDR_DEL_TYPE) | SET_WORD(_ca, H2C_HDR_CAT) | \
		 SET_WORD(_cl, H2C_HDR_CLASS) | SET_WORD(_f, H2C_HDR_FUNC))
#define GET_FWCMD_TYPE(id)	(GET_FIELD(id, C2H_HDR_DEL_TYPE))
#define GET_FWCMD_CAT(id)	(GET_FIELD(id, C2H_HDR_CAT))
#define GET_FWCMD_CLASS(id)	(GET_FIELD(id, C2H_HDR_CLASS))
#define GET_FWCMD_FUNC(id)	(GET_FIELD(id, C2H_HDR_FUNC))

#define FWCMD_TYPE_H2C	0
#define FWCMD_TYPE_C2H	1

#define FWCMD_C2H_CL_NULL		0xFF
#define FWCMD_C2H_FUNC_NULL		0xFF
#define FWCMD_C2H_CAT_NULL		0xFF

/**
 * @struct h2c_buf_head
 * @brief h2c_buf_head
 *
 * @var h2c_buf_head::next
 * Please Place Description here.
 * @var h2c_buf_head::prev
 * Please Place Description here.
 * @var h2c_buf_head::pool
 * Please Place Description here.
 * @var h2c_buf_head::size
 * Please Place Description here.
 * @var h2c_buf_head::qlen
 * Please Place Description here.
 * @var h2c_buf_head::suspend
 * Please Place Description here.
 * @var h2c_buf_head::lock
 * Please Place Description here.
 */
struct h2c_buf_head {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	u8 *pool;
	u32 size;
	u32 qlen;
	u8 suspend;
	mac_ax_mutex lock;
};

/**
 * @struct fwcmd_wkb_head
 * @brief fwcmd_wkb_head
 *
 * @var fwcmd_wkb_head::next
 * Please Place Description here.
 * @var fwcmd_wkb_head::prev
 * Please Place Description here.
 * @var fwcmd_wkb_head::qlen
 * Please Place Description here.
 * @var fwcmd_wkb_head::lock
 * Please Place Description here.
 */
struct fwcmd_wkb_head {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	u32 qlen;
	mac_ax_mutex lock;
};

/**
 * @struct h2c_buf
 * @brief h2c_buf
 *
 * @var h2c_buf::next
 * Please Place Description here.
 * @var h2c_buf::prev
 * Please Place Description here.
 * @var h2c_buf::_class_
 * Please Place Description here.
 * @var h2c_buf::id
 * Please Place Description here.
 * @var h2c_buf::master
 * Please Place Description here.
 * @var h2c_buf::len
 * Please Place Description here.
 * @var h2c_buf::head
 * Please Place Description here.
 * @var h2c_buf::end
 * Please Place Description here.
 * @var h2c_buf::data
 * Please Place Description here.
 * @var h2c_buf::tail
 * Please Place Description here.
 * @var h2c_buf::hdr_len
 * Please Place Description here.
 * @var h2c_buf::flags
 * Please Place Description here.
 * @var h2c_buf::h2c_seq
 * Please Place Description here.
 */
struct h2c_buf {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	enum h2c_buf_class _class_;
	u32 id;
	u8 master;
	u32 len;
	u8 *head;
	u8 *end;
	u8 *data;
	u8 *tail;
	u32 hdr_len;
#define H2CB_FLAGS_FREED	BIT(0)
	u32 flags;
	u8 h2c_seq;
};

/**
 * @struct c2h_proc_class
 * @brief c2h_proc_class
 *
 * @var c2h_proc_class::id
 * Please Place Description here.
 * @var c2h_proc_class::handler
 * Please Place Description here.
 */
struct c2h_proc_class {
	u16 id;
	u32(*handler)(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		      struct rtw_c2h_info *info);
};

struct fw_status_proc_class {
	u16 id;
	u32(*handler)(struct mac_ax_adapter *adapter, u8 *buf, u32 len);
};

/**
 * @struct c2h_proc_func
 * @brief c2h_proc_func
 *
 * @var c2h_proc_func::id
 * Please Place Description here.
 * @var c2h_proc_func::handler
 * Please Place Description here.
 */
struct c2h_proc_func {
	u16 id;
	u32(*handler)(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		      struct rtw_c2h_info *info);
};

/**
 * @struct mac_ax_c2hreg_info
 * @brief mac_ax_c2hreg_info
 *
 * @var mac_ax_c2hreg_info::id
 * Please Place Description here.
 * @var mac_ax_c2hreg_info::total_len
 * Please Place Description here.
 * @var mac_ax_c2hreg_info::content
 * Please Place Description here.
 * @var mac_ax_c2hreg_info::c2hreg
 * Please Place Description here.
 */

struct mac_ax_c2hreg_info {
	u8 id;
	u8 content_len;
	u8 *content;
	u8 c2hreg[C2HREG_LEN];
};

struct mac_ax_c2hreg_cont {
	u8 id;
	u8 content_len;
	struct fwcmd_c2hreg c2h_content;
};

struct mac_ax_c2hreg_poll {
	u8 polling_id;
	u32 retry_cnt;
	u32 retry_wait_us;
	struct mac_ax_c2hreg_cont c2hreg_cont;
};

struct mac_ax_h2creg_info {
	u8 id;
	u8 content_len;
	struct fwcmd_h2creg h2c_content;
};

/**
 * @brief mac_send_h2creg
 *
 * @param *adapter
 * @param *content
 * @param len
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_send_h2creg(struct mac_ax_adapter *adapter, u32 *content, u8 len);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Firmware
 * @{
 * @addtogroup C2H
 * @{
 */

/**
 * @brief mac_process_c2hreg
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_process_c2hreg(struct mac_ax_adapter *adapter,
		       struct mac_ax_c2hreg_info *info);

/**
 * @addtogroup Firmware
 * @{
 * @addtogroup C2H
 * @{
 */

/**
 * @brief mac_get_c2h_event
 *
 * Get the phl_msg_evt_id from C2H packet
 *
 * @param *adapter
 * @param *c2h
 * @param *id
 * @return Return 0 when getting event ID successfully.
 * @retval u32
 */

u32 mac_get_c2h_event(struct mac_ax_adapter *adapter,
		      struct rtw_c2h_info *c2h,
		      enum phl_msg_evt_id *id,
		      u8 *c2h_info);

/**
 * @}
 * @}
 */

u32 mac_notify_fw_dbcc(struct mac_ax_adapter *adapter, u8 en);
#endif



/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __COMMON_H2C_H__
#define __COMMON_H2C_H__

/* ---------------------------------------------------------------------------------------------------------
 * ----------------------------------    H2C CMD DEFINITION    ------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 88e, 8723b, 8812, 8821, 92e use the same FW code base */
enum h2c_cmd {
	/* Common Class: 000 */
	H2C_RSVD_PAGE = 0x00,
	H2C_MEDIA_STATUS_RPT = 0x01,
	H2C_SCAN_ENABLE = 0x02,
	H2C_KEEP_ALIVE = 0x03,
	H2C_DISCON_DECISION = 0x04,
	H2C_PSD_OFFLOAD = 0x05,
	H2C_TCP_KEEP_ALIVE = 0x07,
	H2C_AP_OFFLOAD = 0x08,
	H2C_BCN_RSVDPAGE = 0x09,
	H2C_PROBERSP_RSVDPAGE = 0x0A,
	H2C_FCS_RSVDPAGE = 0x10,
	H2C_FCS_INFO = 0x11,
	H2C_AP_WOW_GPIO_CTRL = 0x13,
#ifdef CONFIG_MCC_MODE
	H2C_MCC_UPDATE_PARAM = 0x15,
	H2C_MCC_MACID_BITMAP = 0x16,
	H2C_MCC_LOCATION = 0x10,
	H2C_MCC_CTRL = 0x18,
	H2C_MCC_NOA_PARAM = 0x19,
	H2C_MCC_IQK_PARAM = 0x1A,
#endif /* CONFIG_MCC_MODE */
	H2C_CHNL_SWITCH_OPER_OFFLOAD = 0x1C,

	/* PoweSave Class: 001 */
	H2C_SET_PWR_MODE = 0x20,
	H2C_PS_TUNING_PARA = 0x21,
	H2C_PS_TUNING_PARA2 = 0x22,
	H2C_P2P_LPS_PARAM = 0x23,
	H2C_P2P_PS_OFFLOAD = 0x24,
	H2C_PS_SCAN_ENABLE = 0x25,
	H2C_SAP_PS_ = 0x26,
	H2C_INACTIVE_PS_ = 0x27, /* Inactive_PS */
	H2C_FWLPS_IN_IPS_ = 0x28,
#ifdef CONFIG_LPS_PG
	H2C_LPS_PG_INFO = 0x2B,
#endif
	H2C_DEFAULT_PORT_ID = 0x2C,
	/* Dynamic Mechanism Class: 010 */
	H2C_MACID_CFG = 0x40,
	H2C_TXBF = 0x41,
	H2C_RSSI_SETTING = 0x42,
	H2C_AP_REQ_TXRPT = 0x43,
	H2C_INIT_RATE_COLLECT = 0x44,
	H2C_IQ_CALIBRATION	= 0x45,

	H2C_RA_MASK_3SS = 0x46,/* for 8814A */
	H2C_RA_PARA_ADJUST = 0x47,/* CONFIG_RA_DBG_CMD */
	H2C_DYNAMIC_TX_PATH = 0x48,/* for 8814A */

	H2C_FW_TRACE_EN = 0x49,

	/* BT Class: 011 */
	H2C_B_TYPE_TDMA = 0x60,
	H2C_BT_INFO = 0x61,
	H2C_FORCE_BT_TXPWR = 0x62,
	H2C_BT_IGNORE_WLANACT = 0x63,
	H2C_DAC_SWING_VALUE = 0x64,
	H2C_ANT_SEL_RSV = 0x65,
	H2C_WL_OPMODE = 0x66,
	H2C_BT_MP_OPER = 0x67,
	H2C_BT_CONTROL = 0x68,
	H2C_BT_WIFI_CTRL = 0x69,
	H2C_BT_FW_PATCH = 0x6A,

	/* WOWLAN Class: 100 */
	H2C_WOWLAN = 0x80,
	H2C_REMOTE_WAKE_CTRL = 0x81,
	H2C_AOAC_GLOBAL_INFO = 0x82,
	H2C_AOAC_RSVD_PAGE = 0x83,
	H2C_AOAC_RSVD_PAGE2 = 0x84,
	H2C_D0_SCAN_OFFLOAD_CTRL = 0x85,
	H2C_D0_SCAN_OFFLOAD_INFO = 0x86,
	H2C_CHNL_SWITCH_OFFLOAD = 0x87,
	H2C_AOAC_RSVDPAGE3 = 0x88,
	H2C_P2P_OFFLOAD_RSVD_PAGE = 0x8A,
	H2C_P2P_OFFLOAD = 0x8B,
	H2C_BA_OFFLOAD = 0x90,
	H2C_DHCP_RENEW = 0x91,
	H2C_DYNAMIC_TX_PWR = 0x92,
#ifdef CONFIG_WOWLAN_SSL_KEEP_ALIVE
	H2C_SSL_OFFLOAD = 0x93,
#endif
	H2C_PNO = 0x94,
	H2C_RESET_TSF = 0xC0,
	H2C_BCNHWSEQ = 0xC5,
	H2C_TSF_LATCH = 0xC9,
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && (CONFIG_UNSUPPORT_PLCPHDR_RPT)
	H2C_UNSUPPORT_PLCPHDR_RPT = 0xCC,
#endif
	H2C_MAXID,
};

/* C2H BT OP CODES, H2C_BT_MP_OPER 0x67. */
typedef enum _bt_op_code {
	BT_OP_GET_BT_VERSION = 0x00,
	BT_OP_WRITE_REG_ADDR = 0x0c,
	BT_OP_WRITE_REG_VALUE = 0x0d,

	BT_OP_READ_REG = 0x11,

	BT_LO_OP_GET_AFH_MAP_L = 0x1e,
	BT_LO_OP_GET_AFH_MAP_M = 0x1f,
	BT_LO_OP_GET_AFH_MAP_H = 0x20,

	BT_OP_GET_BT_COEX_SUPPORTED_FEATURE = 0x2a,
	BT_OP_GET_BT_COEX_SUPPORTED_VERSION = 0x2b,
	BT_OP_GET_BT_ANT_DET_VAL = 0x2c,
	BT_OP_GET_BT_BLE_SCAN_TYPE = 0x2d,
	BT_OP_GET_BT_BLE_SCAN_PARA = 0x2e,
	BT_OP_GET_BT_DEVICE_INFO = 0x30,
	BT_OP_GET_BT_FORBIDDEN_SLOT_VAL = 0x31,
	BT_OP_SET_BT_LANCONSTRAIN_LEVEL = 0x32,
	BT_OP_MAX
} BT_OP_CODE;

#define H2C_INACTIVE_PS_LEN		3
#define H2C_RSVDPAGE_LOC_LEN		5
#define H2C_MEDIA_STATUS_RPT_LEN		3

#define H2C_KEEP_ALIVE_CTRL_LEN	6
#define H2C_DISCON_DECISION_LEN		3
#define H2C_PNO_LEN		        4
#define H2C_TCP_KEEP_ALIVE_CTRL_LEN 6
#define H2C_DHCP_RENEW_CTRL_LEN 6

#define H2C_AP_OFFLOAD_LEN		3
#define H2C_AP_WOW_GPIO_CTRL_LEN	4
#define H2C_AP_PS_LEN			2
#define H2C_PWRMODE_LEN			7
#define H2C_PSTUNEPARAM_LEN			4
#define H2C_MACID_CFG_LEN		7
#define H2C_BTMP_OPER_LEN			5
#define H2C_WOWLAN_LEN			5
#define H2C_REMOTE_WAKE_CTRL_LEN	3
#define H2C_BA_OFFLOAD_LEN          2
#define H2C_AOAC_GLOBAL_INFO_LEN	2
#define H2C_AOAC_RSVDPAGE_LOC_LEN	7
#define H2C_SCAN_OFFLOAD_CTRL_LEN	4
#define H2C_BT_FW_PATCH_LEN			6
#define H2C_RSSI_SETTING_LEN		4
#define H2C_AP_REQ_TXRPT_LEN		2
#define H2C_FORCE_BT_TXPWR_LEN		3
#define H2C_BCN_RSVDPAGE_LEN		5
#define H2C_PROBERSP_RSVDPAGE_LEN	5
#define H2C_P2PRSVDPAGE_LOC_LEN	5
#define H2C_P2P_OFFLOAD_LEN	3
#define H2C_TSF_LATCH_LEN		2
#define H2C_DEFAULT_PORT_LEN	2
#define H2C_B_TYPE_TDMA_LEN	5
#define H2C_DYNAMIC_TX_PWR_LEN	5

#ifdef CONFIG_MCC_MODE
#define H2C_MCC_CTRL_LEN			7
#define H2C_MCC_LOCATION_LEN		3
#define H2C_MCC_MACID_BITMAP_LEN	6
#define H2C_MCC_UPDATE_INFO_LEN		4
#define H2C_MCC_NOA_PARAM_LEN		4
#define H2C_MCC_IQK_PARAM_LEN		7
#endif /* CONFIG_MCC_MODE */
#ifdef CONFIG_LPS_PG
#define H2C_LPS_PG_INFO_LEN		2
#define H2C_LPSPG_LEN			16
#endif
#define eq_mac_addr(a, b)						(((a)[0] == (b)[0] && (a)[1] == (b)[1] && (a)[2] == (b)[2] && (a)[3] == (b)[3] && (a)[4] == (b)[4] && (a)[5] == (b)[5]) ? 1 : 0)
#define cp_mac_addr(des, src)					((des)[0] = (src)[0], (des)[1] = (src)[1], (des)[2] = (src)[2], (des)[3] = (src)[3], (des)[4] = (src)[4], (des)[5] = (src)[5])
#define cpIpAddr(des, src)					((des)[0] = (src)[0], (des)[1] = (src)[1], (des)[2] = (src)[2], (des)[3] = (src)[3])


#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
/*
* ARP packet
*
* LLC Header */
#define GET_ARP_PKT_LLC_TYPE(__pHeader)					ReadLE2Byte(((u8 *)(__pHeader)) + 6)

/* ARP element */
#define GET_ARP_PKT_OPERATION(__pHeader)				ReadLE2Byte(((u8 *)(__pHeader)) + 6)
#define GET_ARP_PKT_SENDER_MAC_ADDR(__pHeader, _val)	cp_mac_addr((u8 *)(_val), ((u8 *)(__pHeader))+8)
#define GET_ARP_PKT_SENDER_IP_ADDR(__pHeader, _val)		cpIpAddr((u8 *)(_val), ((u8 *)(__pHeader))+14)
#define GET_ARP_PKT_TARGET_MAC_ADDR(__pHeader, _val)	cp_mac_addr((u8 *)(_val), ((u8 *)(__pHeader))+18)
#define GET_ARP_PKT_TARGET_IP_ADDR(__pHeader, _val)	cpIpAddr((u8 *)(_val), ((u8 *)(__pHeader))+24)

#define SET_ARP_PKT_HW(__pHeader, __Value)					WriteEF2Byte(((u8 *)(__pHeader)) + 0, __Value)
#define SET_ARP_PKT_PROTOCOL(__pHeader, __Value)			WriteEF2Byte(((u8 *)(__pHeader)) + 2, __Value)
#define SET_ARP_PKT_HW_ADDR_LEN(__pHeader, __Value)			WriteEF1Byte(((u8 *)(__pHeader)) + 4, __Value)
#define SET_ARP_PKT_PROTOCOL_ADDR_LEN(__pHeader, __Value)	WriteEF1Byte(((u8 *)(__pHeader)) + 5, __Value)
#define SET_ARP_PKT_OPERATION(__pHeader, __Value)			WriteEF2Byte(((u8 *)(__pHeader)) + 6, __Value)
#define SET_ARP_PKT_SENDER_MAC_ADDR(__pHeader, _val)	cp_mac_addr(((u8 *)(__pHeader))+8, (u8 *)(_val))
#define SET_ARP_PKT_SENDER_IP_ADDR(__pHeader, _val)		cpIpAddr(((u8 *)(__pHeader))+14, (u8 *)(_val))
#define SET_ARP_PKT_TARGET_MAC_ADDR(__pHeader, _val)	cp_mac_addr(((u8 *)(__pHeader))+18, (u8 *)(_val))
#define SET_ARP_PKT_TARGET_IP_ADDR(__pHeader, _val)		cpIpAddr(((u8 *)(__pHeader))+24, (u8 *)(_val))

#define FW_WOWLAN_FUN_EN				BIT(0)
#define FW_WOWLAN_PATTERN_MATCH			BIT(1)
#define FW_WOWLAN_MAGIC_PKT				BIT(2)
#define FW_WOWLAN_UNICAST				BIT(3)
#define FW_WOWLAN_ALL_PKT_DROP			BIT(4)
#define FW_WOWLAN_GPIO_ACTIVE			BIT(5)
#define FW_WOWLAN_REKEY_WAKEUP			BIT(6)
#define FW_WOWLAN_DEAUTH_WAKEUP			BIT(7)

#define FW_WOWLAN_GPIO_WAKEUP_EN		BIT(0)
#define FW_FW_PARSE_MAGIC_PKT			BIT(1)

#define FW_REMOTE_WAKE_CTRL_EN			BIT(0)
#define FW_REALWOWLAN_EN				BIT(5)

#define FW_WOWLAN_KEEP_ALIVE_EN			BIT(0)
#define FW_ADOPT_USER					BIT(1)
#define FW_WOWLAN_KEEP_ALIVE_PKT_TYPE	BIT(2)

#define FW_REMOTE_WAKE_CTRL_EN			BIT(0)
#define FW_ARP_EN						BIT(1)
#define FW_REALWOWLAN_EN				BIT(5)
#define FW_WOW_FW_UNICAST_EN			BIT(7)

#endif /* CONFIG_WOWLAN */

/* _RSVDPAGE_LOC_CMD_0x00 */
#define SET_H2CCMD_RSVDPAGE_LOC_PROBE_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_PSPOLL(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_QOS_NULL_DATA(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_BT_QOS_NULL_DATA(__pH2CCmd, __Value)SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)

/* _MEDIA_STATUS_RPT_PARM_CMD_0x01 */
#define SET_H2CCMD_MSRRPT_PARM_OPMODE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)), 0, 1, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_MACID_IND(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)), 1, 1, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_MIRACAST(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)), 2, 1, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_MIRACAST_SINK(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)), 3, 1, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_ROLE(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)), 4, 4, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_MACID(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)) + 1, 0, 8, (__Value))
#define SET_H2CCMD_MSRRPT_PARM_MACID_END(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(((u8 *)(__pH2CCmd)) + 2, 0, 8, (__Value))

#define GET_H2CCMD_MSRRPT_PARM_OPMODE(__pH2CCmd)		LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 0, 1)
#define GET_H2CCMD_MSRRPT_PARM_MIRACAST(__pH2CCmd)		LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 2, 1)
#define GET_H2CCMD_MSRRPT_PARM_MIRACAST_SINK(__pH2CCmd)	LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 3, 1)
#define GET_H2CCMD_MSRRPT_PARM_ROLE(__pH2CCmd)			LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 4, 4)

extern const char *const _h2c_msr_role_str[];
#define h2c_msr_role_str(role) (((role) >= H2C_MSR_ROLE_MAX) ? _h2c_msr_role_str[H2C_MSR_ROLE_MAX] : _h2c_msr_role_str[(role)])

#define H2C_MSR_FMT "%s %s%s"
#define H2C_MSR_ARG(h2c_msr) \
	GET_H2CCMD_MSRRPT_PARM_OPMODE((h2c_msr)) ? " C" : "", \
	h2c_msr_role_str(GET_H2CCMD_MSRRPT_PARM_ROLE((h2c_msr))), \
	GET_H2CCMD_MSRRPT_PARM_MIRACAST((h2c_msr)) ? (GET_H2CCMD_MSRRPT_PARM_MIRACAST_SINK((h2c_msr)) ? " MSINK" : " MSRC") : ""

/* _KEEP_ALIVE_CMD_0x03 */
#define SET_H2CCMD_KEEPALIVE_PARM_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_KEEPALIVE_PARM_ADOPT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_KEEPALIVE_PARM_PKT_TYPE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_H2CCMD_KEEPALIVE_PARM_CHECK_PERIOD(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)

/* _DISCONNECT_DECISION_CMD_0x04 */
#define SET_H2CCMD_DISCONDECISION_PARM_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_DISCONDECISION_PARM_ADOPT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_DISCONDECISION_PARM_TRY_OK_BCN_FAIL_COUNT_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_H2CCMD_DISCONDECISION_PARM_CHECK_PERIOD(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_H2CCMD_DISCONDECISION_PARM_TRY_PKT_NUM(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)

/* _PMO_CMD_0x94 */
#define SET_H2CCMD_PNO_PARM_ENABLE(__pH2CCmd, __Value)						SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_PNO_PARM_DTIM(__pH2CCmd, __Value)					    SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 4, __Value)
#define SET_H2CCMD_PNO_PARM_INTERVAL_L(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_H2CCMD_PNO_PARM_INTERVAL_H(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)
#define SET_H2CCMD_PNO_PARM_FORCE_WAKEUP(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+3, 0, 8, __Value)

/* _TCP_KEEP_ALIVE_CMD_0x07 */
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_ADOPT(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_DTIMTO_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_DTIMTO_RETRY_INC(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_DTIMTO_SSL_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_CHECK_PERIOD(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_LOC(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_RESEND_INTERVAL(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+3, 0, 8, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_FIRST_PKT_TIME(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+4, 0, 8, __Value)
#define SET_H2CCMD_TCP_KEEPALIVE_PARM_ACK_TIMEOUT(__pH2CCmd, __Value)

/* _DHCP_RENEW_CMD_0x91 */
#define SET_H2CCMD_DHCP_RENEW_PARM_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define	SET_H2CCMD_DHCP_RENEW_PARM_LOC(__pH2CCmd, __Value)                      SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define	SET_H2CCMD_DHCP_RENEW_PARM_T1_H(__pH2CCmd, __Value)              SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)
#define	SET_H2CCMD_DHCP_RENEW_PARM_T1_L(__pH2CCmd, __Value)              SET_BITS_TO_LE_1BYTE(__pH2CCmd+3, 0, 8, __Value)
#define	SET_H2CCMD_DHCP_RENEW_PARM_LEASE_H(__pH2CCmd, __Value)        SET_BITS_TO_LE_1BYTE(__pH2CCmd+4, 0, 8, __Value)
#define	SET_H2CCMD_DHCP_RENEW_PARM_LEASE_L(__pH2CCmd, __Value)        SET_BITS_TO_LE_1BYTE(__pH2CCmd+5, 0, 8, __Value)

/* _DYNAMIC_TX_PWR_CMD_0x92 */
#define SET_H2CCMD_DYNAMIC_TX_PWR_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define	SET_H2CCMD_DYNAMIC_TX_PWR_THS_L(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define	SET_H2CCMD_DYNAMIC_TX_PWR_THS_H(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)
#define	SET_H2CCMD_DYNAMIC_TX_PWR_OFS_L(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+3, 0, 8, __Value)
#define	SET_H2CCMD_DYNAMIC_TX_PWR_OFS_H(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd+4, 0, 8, __Value)
/* _AP_Offload 0x08 */
#define SET_H2CCMD_AP_WOWLAN_EN(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
/* _BCN_RsvdPage	0x09 */
#define SET_H2CCMD_AP_WOWLAN_RSVDPAGE_LOC_BCN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
/* _Probersp_RsvdPage 0x0a */
#define SET_H2CCMD_AP_WOWLAN_RSVDPAGE_LOC_ProbeRsp(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
/* _Probersp_RsvdPage 0x13 */
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_INDEX(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 4, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_C2H_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_PLUS(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_HIGH_ACTIVE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 6, 1, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 7, 1, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_DURATION(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_AP_WOW_GPIO_CTRL_C2H_DURATION(__pH2CCmd, __Value)SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
/* _AP_PS 0x26 */
#define SET_H2CCMD_AP_WOW_PS_EN(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_AP_WOW_PS_32K_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_AP_WOW_PS_RF(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_H2CCMD_AP_WOW_PS_DURATION(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)

#ifdef CONFIG_MCC_MODE
/* MCC LOC CMD 0x10 */
#define SET_H2CCMD_MCC_RSVDPAGE_LOC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)

/* MCC MAC ID CMD 0x16 */
#define SET_H2CCMD_MCC_MACID_BITMAP_L(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_H2CCMD_MCC_MACID_BITMAP_H(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)

/* MCC INFO CMD 0x18 */
#define SET_H2CCMD_MCC_CTRL_ORDER(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 4, __Value)
#define SET_H2CCMD_MCC_CTRL_TOTALNUM(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 4, __Value)
#define SET_H2CCMD_MCC_CTRL_CHIDX(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_MCC_CTRL_BW(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 2, __Value)
#define SET_H2CCMD_MCC_CTRL_BW40SC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 2, 3, __Value)
#define SET_H2CCMD_MCC_CTRL_BW80SC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 5, 3, __Value)
#define SET_H2CCMD_MCC_CTRL_DURATION(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_H2CCMD_MCC_CTRL_ROLE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 3, __Value)
#define SET_H2CCMD_MCC_CTRL_INCURCH(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 3, 1, __Value)
#define SET_H2CCMD_MCC_CTRL_RSVD0(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 4, 4, __Value)
#define SET_H2CCMD_MCC_CTRL_RSVD1(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 0, 8, __Value)
#define SET_H2CCMD_MCC_CTRL_RFETYPE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 0, 4, __Value)
#define SET_H2CCMD_MCC_CTRL_DISTXNULL(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 4, 1, __Value)
#define SET_H2CCMD_MCC_CTRL_C2HRPT(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 5, 2, __Value)
#define SET_H2CCMD_MCC_CTRL_CHSCAN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 7, 1, __Value)

/* MCC NoA CMD 0x19 */
#define SET_H2CCMD_MCC_NOA_FW_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_MCC_NOA_TSF_SYNC_OFFSET(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 7, __Value)
#define SET_H2CCMD_MCC_NOA_START_TIME(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_MCC_NOA_INTERVAL(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_MCC_EARLY_TIME(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)

/* MCC IQK CMD 0x1A */
#define SET_H2CCMD_MCC_IQK_READY(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_MCC_IQK_ORDER(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 4, __Value)
#define SET_H2CCMD_MCC_IQK_PATH(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 2, __Value)
#define SET_H2CCMD_MCC_IQK_RX_L(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_MCC_IQK_RX_M1(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 2, __Value)
#define SET_H2CCMD_MCC_IQK_RX_M2(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 2, 6, __Value)
#define SET_H2CCMD_MCC_IQK_RX_H(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 4, __Value)
#define SET_H2CCMD_MCC_IQK_TX_L(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)
#define SET_H2CCMD_MCC_IQK_TX_M1(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 0, 3, __Value)
#define SET_H2CCMD_MCC_IQK_TX_M2(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 3, 5, __Value)
#define SET_H2CCMD_MCC_IQK_TX_H(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 0, 6, __Value)
#endif /* CONFIG_MCC_MODE */

/* CHNL SWITCH OPER OFFLOAD 0x1C */
#define SET_H2CCMD_CH_SW_OPER_OFFLOAD_CH_NUM(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_H2CCMD_CH_SW_OPER_OFFLOAD_BW_MODE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd) + 1, 0, 2, __Value)
#define SET_H2CCMD_CH_SW_OPER_OFFLOAD_BW_40M_SC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd) + 1, 2, 3, __Value)
#define SET_H2CCMD_CH_SW_OPER_OFFLOAD_BW_80M_SC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd) + 1, 5, 3, __Value)
#define SET_H2CCMD_CH_SW_OPER_OFFLOAD_RFE_TYPE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd) + 2, 0, 4, __Value)

/* _WoWLAN PARAM_CMD_0x80 */
#define SET_H2CCMD_WOWLAN_FUNC_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_WOWLAN_PATTERN_MATCH_ENABLE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_WOWLAN_MAGIC_PKT_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_H2CCMD_WOWLAN_UNICAST_PKT_ENABLE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_H2CCMD_WOWLAN_ALL_PKT_DROP(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_H2CCMD_WOWLAN_GPIO_ACTIVE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_H2CCMD_WOWLAN_REKEY_WAKE_UP(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 6, 1, __Value)
#define SET_H2CCMD_WOWLAN_DISCONNECT_WAKE_UP(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 7, 1, __Value)
#define SET_H2CCMD_WOWLAN_GPIONUM(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 7, __Value)
#define SET_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 7, 1, __Value)
#define SET_H2CCMD_WOWLAN_GPIO_DURATION(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_WOWLAN_GPIO_PULSE_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 1, __Value)
#define SET_H2CCMD_WOWLAN_GPIO_PULSE_COUNT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 1, 7, __Value)
#define SET_H2CCMD_WOWLAN_LOWPR_RX(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 1, __Value)
#define SET_H2CCMD_WOWLAN_CHANGE_UNIT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 2, 1, __Value)
/* _REMOTE_WAKEUP_CMD_0x81 */
#define SET_H2CCMD_REMOTE_WAKECTRL_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_OFFLOAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_NDP_OFFLOAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_GTK_OFFLOAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_NLO_OFFLOAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_FW_UNICAST_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 7, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_P2P_OFFLAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_NBNS_FILTER_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 2, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_TKIP_OFFLOAD_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 3, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_ACTION(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_FW_PARSING_UNTIL_WAKEUP(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 4, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_POWER_BIT_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 7, 1, __Value)
#define SET_H2CCMD_REMOTE_WAKE_CTRL_ARP_RESPONSE_BLOCK(__pH2CCmd, __Value)    SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 3, 1, __Value)

/* AOAC_GLOBAL_INFO_0x82 */
#define SET_H2CCMD_AOAC_GLOBAL_INFO_PAIRWISE_ENC_ALG(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_H2CCMD_AOAC_GLOBAL_INFO_GROUP_ENC_ALG(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)

/* AOAC_RSVDPAGE_LOC_0x83 */
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_REMOTE_WAKE_CTRL_INFO(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd), 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_ARP_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_NEIGHBOR_ADV(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_INFO(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)
#ifdef CONFIG_GTK_OL
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_EXT_MEM(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 0, 8, __Value)
#endif /* CONFIG_GTK_OL */

/* AOAC_RSVDPAGE_2_0x84 */

/* AOAC_RSVDPAGE_3_0x88 */
#ifdef CONFIG_PNO_SUPPORT
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_NLO_INFO(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd), 0, 8, __Value)
#endif
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_AOAC_REPORT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd) + 1, 0, 8, __Value)

#ifdef CONFIG_PNO_SUPPORT
/* D0_Scan_Offload_Info_0x86 */
#define SET_H2CCMD_AOAC_NLO_FUN_EN(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd), 3, 1, __Value)
#define SET_H2CCMD_AOAC_NLO_IPS_EN(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd), 4, 1, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_PROBE_PACKET(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_SCAN_INFO(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_AOAC_RSVDPAGE_LOC_SSID_INFO(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#endif /* CONFIG_PNO_SUPPORT */

#ifdef CONFIG_P2P_WOWLAN
/* P2P_RsvdPage_0x8a */
#define SET_H2CCMD_RSVDPAGE_LOC_P2P_BCN(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_P2P_PROBE_RSP(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_P2P_NEGO_RSP(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_P2P_INVITE_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_H2CCMD_RSVDPAGE_LOC_P2P_PD_RSP(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)
#endif /* CONFIG_P2P_WOWLAN */

/* BA_offload 0x90 */
#define SET_H2CCMD_BA_OFFLOAD_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_BA_OFFLOAD_BA_RSP_LOC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)

#ifdef CONFIG_LPS_PG
#define SET_H2CCMD_LPSPG_SEC_CAM_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)/*SecurityCAM_En*/
#define SET_H2CCMD_LPSPG_MBID_CAM_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)/*BSSIDCAM_En*/
#define SET_H2CCMD_LPSPG_PMC_CAM_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)/*PatternMatchCAM_En*/
#define SET_H2CCMD_LPSPG_MACID_SEARCH_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)/*MACIDSearch_En*/
#define SET_H2CCMD_LPSPG_TXSC_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)/*TXSC_En*/
#define SET_H2CCMD_LPSPG_MU_RATE_TB_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)/*MURateTable_En*/
#define SET_H2CCMD_LPSPG_LOC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)/*Loc_LPS_PG*/
#endif

#ifdef CONFIG_WOWLAN_SSL_KEEP_ALIVE
#define SET_H2CCMD_MQTT_PUBLISH_WAKE(__pH2CCmd, __Value) SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)/*PUBLISH_WAKE EN*/
#define SET_H2CCMD_MQTT_SSL_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)/*SSL EN*/
#define SET_H2CCMD_MQTT_PATTERN_MATCH_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)/*PATTERN_MATCH_EN EN*/
#define SET_H2CCMD_SSL_INFO_LOC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)/*SSL_INFO_LOC*/
#define SET_H2CCMD_SSL_PATTERN_LOC(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)/*SSL_PATTERN_LOC*/
#endif

/* DEFAULT_PORT_CMD_0x2c */
#define SET_H2CCMD_DEFAULT_PORTNUM(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 4, __Value)

/* _TSF_LATCH_CMD_0xc6 */
#define SET_H2CCMD_TSFLATCH_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_H2CCMD_TSFLATCH_GPIO_DEFAULT_VAL(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_H2CCMD_TSFLATCH_PORTNUM(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 4, __Value)
#define SET_H2CCMD_TSFLATCH_GPIONUM(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)

/* ---------------------------------------------------------------------------------------------------------
 * -------------------------------------------    Structure    --------------------------------------------------
 * --------------------------------------------------------------------------------------------------------- */
typedef struct _RSVDPAGE_LOC {
	u8 LocProbeRsp;
	u8 LocPsPoll;
	u8 LocNullData;
	u8 LocQosNull;
	u8 LocBTQosNull;
#ifdef CONFIG_WOWLAN
	u8 LocRemoteCtrlInfo;
	u8 LocArpRsp;
	u8 LocAddbaRsp;
	u8 LocNbrAdv;
	u8 LocGTKRsp;
	u8 LocGTKInfo;
#ifdef CONFIG_GTK_OL
	u8 LocGTKEXTMEM;
#endif /* CONFIG_GTK_OL */
	u8 LocProbeReq;
	u8 LocNetList;
#ifdef CONFIG_WOW_PATTERN_HW_CAM
	u8 LocAOACReport;
#endif
#ifdef CONFIG_PNO_SUPPORT
	u8 LocPNOInfo;
	u8 LocScanInfo;
	u8 LocSSIDInfo;
	u8 LocProbePacket;
#endif /* CONFIG_PNO_SUPPORT */
#endif /* CONFIG_WOWLAN	 */
	u8 LocApOffloadBCN;
#ifdef CONFIG_P2P_WOWLAN
	u8 LocP2PBeacon;
	u8 LocP2PProbeRsp;
	u8 LocNegoRsp;
	u8 LocInviteRsp;
	u8 LocPDRsp;
#endif /* CONFIG_P2P_WOWLAN */
} RSVDPAGE_LOC, *PRSVDPAGE_LOC;

#endif
void dump_TX_FIFO(struct mac_ax_adapter padapter, u8 page_num, u16 page_size);
u8 rtw_hal_set_fw_media_status_cmd(struct mac_ax_adapter *adapter, u8 mstatus, u8 macid);
u32 mac_process_c2h(struct mac_ax_adapter *adapter, u8 *buf, u32 len, u8 *c2h_ret);
u32 mac_send_h2c_hmebox(struct mac_ax_adapter *adapter, u8 *h2c);
u32 mac_outsrc_h2c_common(struct mac_ax_adapter *adapter, struct rtw_g6_h2c_hdr *hdr, u32 *pvalue);
u32 mac_ie_cam_upd(struct mac_ax_adapter *adapter, struct mac_ax_ie_cam_cmd_info *info);
u32 mac_fw_log_cfg(struct mac_ax_adapter *adapter, struct mac_ax_fw_log *log_cfg);
u32 mac_host_getpkt_h2c(struct mac_ax_adapter *adapter, u8 macid, u8 pkttype);
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
/* WOW command function */
u8 rtl8730e_set_wowlan_ctrl_cmd(struct mac_ax_adapter *adapter, enum mac_ax_wow_ctrl w_c);
void rtw_hal_set_fw_wow_related_cmd(struct mac_ax_adapter *padapter, u8 enable);
#ifdef CONFIG_P2P_WOWLAN
/* H2C 0x8A */
u8 rtw_hal_set_FwP2PRsvdPage_cmd(struct mac_ax_adapter *adapter, PRSVDPAGE_LOC rsvdpageloc);
/* H2C 0x8B */
u8 rtw_hal_set_p2p_wowlan_offload_cmd(struct mac_ax_adapter *adapter);
#endif /* CONFIG_P2P_WOWLAN */
#endif


/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __COMMON_C2H_H__
#define __COMMON_C2H_H__

#define C2H_TYPE_REG 0
#define C2H_TYPE_PKT 1

/*
* C2H event format:
* Fields    TRIGGER    PAYLOAD    SEQ    PLEN    ID
* BITS     [127:120]    [119:16]   [15:8]  [7:4]  [3:0]
*/
#define C2H_ID(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)), 0, 4)
#define C2H_PLEN(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)), 4, 4)
#define C2H_SEQ(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)) + 1, 0, 8)
#define C2H_PAYLOAD(_c2h)	(((u8*)(_c2h)) + 2)

#define SET_C2H_ID(_c2h, _val)		SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)), 0, 4, _val)
#define SET_C2H_PLEN(_c2h, _val)	SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)), 4, 4, _val)
#define SET_C2H_SEQ(_c2h, _val)		SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)) + 1 , 0, 8, _val)

/*
* C2H event format:
* Fields    TRIGGER     PLEN      PAYLOAD    SEQ      ID
* BITS    [127:120]  [119:112]  [111:16]   [15:8]   [7:0]
*/
#define C2H_ID_88XX(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)), 0, 8)
#define C2H_SEQ_88XX(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)) + 1, 0, 8)
#define C2H_PAYLOAD_88XX(_c2h)	(((u8*)(_c2h)) + 2)
#define C2H_PLEN_88XX(_c2h)		LE_BITS_TO_1BYTE(((u8*)(_c2h)) + 14, 0, 8)
#define C2H_TRIGGER_88XX(_c2h)	LE_BITS_TO_1BYTE(((u8*)(_c2h)) + 15, 0, 8)

#define SET_C2H_ID_88XX(_c2h, _val)		SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)), 0, 8, _val)
#define SET_C2H_SEQ_88XX(_c2h, _val)	SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)) + 1, 0, 8, _val)
#define SET_C2H_PLEN_88XX(_c2h, _val)	SET_BITS_TO_LE_1BYTE(((u8*)(_c2h)) + 14, 0, 8, _val)

/* C2H_STATUS */
typedef enum _c2h_status {
	BT_STS_OK = 0x0,
	BT_STS_VER_MISMATCH = 0x1,
	BT_STS_UNKNOWN_OPCODE = 0x2,
	BT_STS_ERROR_PARAMETER = 0x3
} C2H_STATUS;

typedef enum _C2H_EVT {
	C2H_DBG = 0x00,
	C2H_LB = 0x01,
	C2H_TXBF = 0x02,
	C2H_CCX_TX_RPT = 0x03,
	C2HID_AP_REQ_TXRPT = 0x04,
	C2H_FW_SCAN_COMPLETE = 0x7,
	C2H_BT_INFO = 0x09,
	C2H_BT_MP_INFO = 0x0B,
	C2H_RA_RPT = 0x0C,
	C2HID_SPE_STATIS = 0x0D,
	C2H_RA_PARA_RPT = 0x0E,
	C2H_FW_CHNL_SWITCH_COMPLETE = 0x10,
	C2H_IQK_FINISH = 0x11,
	C2H_MAILBOX_STATUS = 0x15,
	C2H_P2P_RPORT = 0x16,
	C2H_MCC = 0x17,
	C2H_MAC_HIDDEN_RPT = 0x19,
	C2H_MAC_HIDDEN_RPT_2 = 0x1A,
	C2H_BCN_EARLY_RPT = 0x1E,
	C2H_DEFEATURE_DBG = 0x22,
	C2H_TSF_RPT = 0x24,
	C2H_WLAN_INFO = 0x27,
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && (CONFIG_UNSUPPORT_PLCPHDR_RPT)
	C2H_PLCPHDR_RPT = 0x2B,
#endif
	C2H_DEFEATURE_RSVD = 0xFD,
	C2H_EXTEND = 0xff,
} C2H_EVT;

typedef enum _EXTEND_C2H_EVT {
	EXTEND_C2H_WIFI_FW_ACTIVE_RSP = 0x00,
	EXTEND_C2H_TRIG_BY_BT_FW = 0x01
} EXTEND_C2H_EVT;

#define C2H_REG_LEN 16

/* C2H_IQK_FINISH, 0x11 */
#define IQK_OFFLOAD_LEN 1
void c2h_iqk_offload(struct mac_ax_adapter *adapter, u8 *data, u8 len);
int	c2h_iqk_offload_wait(struct mac_ax_adapter *adapter, u32 timeout_ms);
#define rtl8812_iqk_wait c2h_iqk_offload_wait /* TODO: remove this after phydm call c2h_iqk_offload_wait instead */

#ifdef CONFIG_RTW_MAC_HIDDEN_RPT
/* C2H_MAC_HIDDEN_RPT, 0x19 */
#define MAC_HIDDEN_RPT_LEN 8
int c2h_mac_hidden_rpt_hdl(struct mac_ax_adapter *adapter, u8 *data, u8 len);

/* C2H_MAC_HIDDEN_RPT_2, 0x1A */
#define MAC_HIDDEN_RPT_2_LEN 5
int c2h_mac_hidden_rpt_2_hdl(struct mac_ax_adapter *adapter, u8 *data, u8 len);
int hal_read_mac_hidden_rpt(struct mac_ax_adapter *adapter);
#endif /* CONFIG_RTW_MAC_HIDDEN_RPT */

/* C2H_DEFEATURE_DBG, 0x22 */
#define DEFEATURE_DBG_LEN 1
int c2h_defeature_dbg_hdl(struct mac_ax_adapter *adapter, u8 *data, u8 len);

int c2h_tsf_rpt_hdl(struct mac_ax_adapter *adapter, u8 *data, u8 len);
#endif /* __COMMON_C2H_H__ */
