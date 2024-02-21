/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
/******************************************************************************
 * history *
 * 2020.11.20 | Andrew Su | Modification after first review.
 * 1, Add IPC_WIFI_MSG_XMIT_DONE in IPC_WIFI_CTRL_TYPE to notice the tx is
 * done in device port.
******************************************************************************/

#ifndef __INIC_IPC_H__
#define __INIC_IPC_H__

#include "inic_def.h"
/* -------------------------------- Includes -------------------------------- */
#ifndef CONFIG_FULLMAC
#include "platform_opts.h"
#include "ameba_soc.h"
#include "osdep_service.h"
#include "rtw_wifi_constants.h"
#include "rtw_skbuff.h"
#include "wifi_conf.h"
#include "inic_ipc_cfg.h"
#endif

/* -------------------------------- Defines --------------------------------- */

/* -------------------------------- Macros ---------------------------------- */
#define FLAG_WLAN_IF_NOT_RUNNING		0xFFFFFFFF

#define HOST_MSG_PARAM_NUM		(9)
#define HOST_MSG_DUMY_NUM		(64 - (HOST_MSG_PARAM_NUM + 2) * 4)
#define DEV_MSG_PARAM_NUM		(7)
#define DEV_MSG_DUMY_NUM		(64 - (DEV_MSG_PARAM_NUM + 2) * 4)

#define INIC_MP_MSG_BUF_SIZE 4096

/* ------------------------------- Data Types ------------------------------- */
enum IPC_WIFI_CTRL_TYPE {
	IPC_WIFI_MSG_READ_DONE = 0,
	IPC_WIFI_MSG_MEMORY_NOT_ENOUGH,
	IPC_WIFI_MSG_RECV_DONE,
	IPC_WIFI_CMD_XIMT_PKTS,
	IPC_WIFI_EVT_RECV_PKTS,
	IPC_WIFI_EVT_TX_DONE
};

typedef struct inic_ipc_dev_req_msg {
	u32	enevt_id;
	u32	param_buf[DEV_MSG_PARAM_NUM];
	int	ret;
	u8	dummy[DEV_MSG_DUMY_NUM]; /* add for 64B size alignment */
} inic_ipc_dev_req_t;

typedef struct inic_ipc_host_req_msg {
	u32	api_id;
	u32	param_buf[HOST_MSG_PARAM_NUM];
	int	ret;
	u8 dummy[HOST_MSG_DUMY_NUM];//add for 64B size alignment
} inic_ipc_host_req_t;

typedef struct inic_ipc_ex_msg {
	u32	event_num;
	u32	msg_addr;
	u32 msg_queue_status;
	u32	wlan_idx;
	u32 rsvd[12]; /* keep total size 64B aligned */
} inic_ipc_ex_msg_t;

struct ipc_msg_node {
	struct list_head		list;
	struct inic_ipc_ex_msg		ipc_msg; /* to store ipc message */
	bool				is_used; /* sign whether to be used */
};

extern const char *TAG_WLAN_INIC;

/* -------------------------- Function declaration -------------------------- */
/*for ipc host*/
void inic_host_init(void);
void inic_host_event_int_hdl(void *Data, u32 IrqStatus, u32 ChanNum);

/*for ipc device*/
void inic_dev_init(void);
void inic_dev_event_int_hdl(void *Data, u32 IrqStatus, u32 ChanNum);

/*for ipc host api*/
void inic_api_init_host(void);
void inic_host_deinit_skb(void);
void inic_api_host_int_hdl(void *Data, u32 IrqStatus, u32 ChanNum);
int inic_api_host_message_send(u32 id, u32 *param_buf, u32 buf_len);

/*for ipc dev api*/
void inic_api_init_dev(void);
void inic_api_dev_int_hdl(void *Data, u32 IrqStatus, u32 ChanNum);
void inic_wifi_event_indicate(int event_cmd, char *buf, int buf_len, int flags);
void inic_autoreconnect_indicate(rtw_security_t security_type, u8 *ssid, int ssid_len, u8 *password, int password_len, int key_id, char is_wps_trigger);
void inic_eap_autoreconnect_indicate(u8 saved_eap_method);
void inic_scan_user_callback_indicate(unsigned int ap_num, void *user_data);
void inic_scan_each_report_user_callback_indicate(rtw_scan_result_t *scanned_ap_info, void *user_data);
void inic_promisc_callback_indicate(void *pkt_info);
void inic_ap_ch_switch_callback_indicate(unsigned char channel, rtw_channel_switch_res_t ret);
int inic_dev_set_netif_info(int idx_wlan, unsigned char *dev_addr);
int inic_get_lwip_info(u32 type, unsigned char *input, int index);
u64 inic_host_get_wifi_tsf(unsigned char port_id);
int inic_host_get_txbuf_pkt_num(void);
void inic_cfg80211_indicate_scan_report(u32 channel, u32 frame_is_bcn, s32 rssi, u8 *mac_addr, u8 *IEs, u32 ie_len);
void inic_host_init_skb(void);
#ifdef CONFIG_NAN
void inic_cfg80211_indicate_nan_match(u8 type, u8 inst_id, u8 peer_inst_id, u8 *addr, u32 info_len, u8 *info, u64 cookie);
void inic_cfg80211_nan_func_free(void  *data);
void inic_cfg80211_nan_cfgvendor_event_report(u16 event_id, void *event, int event_len);
void inic_cfg80211_cfgvendor_send_cmd_reply(void *data, int len);
#endif
int inic_ip_in_table_indicate(u8 gate, u8 ip);

#endif /* __INIC_IPC_H__ */
