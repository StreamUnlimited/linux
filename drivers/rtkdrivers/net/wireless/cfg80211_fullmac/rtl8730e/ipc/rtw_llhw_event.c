// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek wireless local area network IC driver.
*   This is an interface between cfg80211 and firmware in other core. The
*   commnunication between driver and firmware is IPC（Inter Process
*   Communication）bus.
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <rtw_cfg80211_fullmac.h>

static void llhw_event_scan_report_indicate(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	u32 channel = p_ipc_msg->param_buf[0];
	u32 frame_is_bcn = p_ipc_msg->param_buf[1];
	s32 rssi = (s32)p_ipc_msg->param_buf[2];
	unsigned char *mac_addr = km4_phys_to_virt(p_ipc_msg->param_buf[3]);
	unsigned char *IEs = km4_phys_to_virt(p_ipc_msg->param_buf[4]);
	u32 ie_len = p_ipc_msg->param_buf[5];

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s,%s: event_priv_t is NULL in!\n", "event", __func__);
		goto func_exit;
	}

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	cfg80211_rtw_inform_bss(channel, frame_is_bcn, rssi, mac_addr, IEs, ie_len);

func_exit:
	return;
}

static void llhw_event_join_status_indicate(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	rtw_event_indicate_t event = (rtw_event_indicate_t)p_ipc_msg->param_buf[0];
	char *buf = km4_phys_to_virt(p_ipc_msg->param_buf[1]);
	int buf_len = (int)p_ipc_msg->param_buf[2];
	int flags = (int)p_ipc_msg->param_buf[3];
	struct device *pdev = NULL;
	u16 disassoc_reason;

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s,%s: event_priv_t is NULL in!\n", "event", __func__);
		goto func_exit;
	}

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	if (event == WIFI_EVENT_JOIN_STATUS) {
		cfg80211_rtw_connect_indicate(flags, buf, buf_len);
	}

	if (event == WIFI_EVENT_DISCONNECT) {
		memcpy(&disassoc_reason, buf + ETH_ALEN, 2);
		dev_dbg(global_idev.fullmac_dev, "%s: disassoc_reason=%d \n", __func__, disassoc_reason);
		/* LINUX_TODO: locally_generated always set to 1, need test, whether exists bug */
		if (global_idev.mlme_priv.rtw_join_status == RTW_JOINSTATUS_DISCONNECT) {
			cfg80211_rtw_disconnect_indicate(disassoc_reason, 1);
		}
	}
	if (event == WIFI_EVENT_STA_ASSOC) {
		dev_dbg(global_idev.fullmac_dev, "%s: sta assoc \n", __func__);
		cfg80211_rtw_sta_assoc_indicate(buf, buf_len);
	}

	if (event == WIFI_EVENT_STA_DISASSOC) {
		dev_dbg(global_idev.fullmac_dev, "%s: sta disassoc \n", __func__);
		cfg80211_del_sta(global_idev.pndev[1], buf, GFP_ATOMIC);
	}

	if (event == WIFI_EVENT_EXTERNAL_AUTH_REQ) {
		dev_dbg(global_idev.fullmac_dev, "%s: auth req \n", __func__);
		cfg80211_rtw_external_auth_request(buf, buf_len);
	}

	if (event == WIFI_EVENT_RX_MGNT) {
		dev_dbg(global_idev.fullmac_dev, "%s: rx mgnt \n", __func__);
		/*channel need get, force 6 seems ok temporary*/
		cfg80211_rx_mgmt(ndev_to_wdev(global_idev.pndev[0]), 6, 0, buf, buf_len, 0);
	}

	if (event == WIFI_EVENT_RX_MGNT_AP) {
		dev_dbg(global_idev.fullmac_dev, "%s: rx mgnt \n", __func__);
		/*channel need get, force 6 seems ok temporary*/
		cfg80211_rx_mgmt(ndev_to_wdev(global_idev.pndev[1]), 6, 0, buf, buf_len, 0);
	}

func_exit:
	return;
}

static void llhw_event_set_netif_info(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	int idx = (u32)p_ipc_msg->param_buf[0];
	unsigned char *dev_addr = km4_phys_to_virt(p_ipc_msg->param_buf[1]);
	int softap_addr_offset_idx = global_idev.wifi_user_config.softap_addr_offset_idx;

	dev_dbg(global_idev.fullmac_dev, "[fullmac]: set netif info.");

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s,%s: event_priv_t is NULL in!\n", "event", __func__);
		goto func_exit;
	}

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	if (idx >= INIC_MAX_NET_PORT_NUM) {
		dev_dbg(global_idev.fullmac_dev, "%s: interface %d not exist!\n", __func__, idx);
		goto func_exit;
	}

	if (!dev_addr) {
		dev_dbg(global_idev.fullmac_dev, "%s: mac address is NULL!\n", __func__);
		goto func_exit;
	}

	memcpy(global_idev.pndev[idx]->dev_addr, dev_addr, ETH_ALEN);
	dev_dbg(global_idev.fullmac_dev, "MAC ADDR [%02x:%02x:%02x:%02x:%02x:%02x]", *global_idev.pndev[idx]->dev_addr,
			*(global_idev.pndev[idx]->dev_addr + 1), *(global_idev.pndev[idx]->dev_addr + 2),
			*(global_idev.pndev[idx]->dev_addr + 3), *(global_idev.pndev[idx]->dev_addr + 4),
			*(global_idev.pndev[idx]->dev_addr + 5));

	/*set ap port mac address*/
	memcpy(global_idev.pndev[1]->dev_addr, global_idev.pndev[0]->dev_addr, ETH_ALEN);
	global_idev.pndev[1]->dev_addr[softap_addr_offset_idx] = global_idev.pndev[0]->dev_addr[softap_addr_offset_idx] + 1;

func_exit:
	return;
}

static void llhw_event_get_network_info(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	uint32_t type = (uint32_t)p_ipc_msg->param_buf[0];
	/* input is used for INIC_WLAN_IS_VALID_IP, not used now. */
	/* uint8_t *input = (uint8_t *)phys_to_virt(p_ipc_msg->param_buf[1]); */
	int idx = p_ipc_msg->param_buf[2];
	uint32_t *rsp_ptr = NULL;
	uint32_t rsp_len = 0;
	struct in_ifaddr *ifa = NULL;
	uint32_t inic_ip_addr[INIC_MAX_NET_PORT_NUM] = {0};
	uint32_t inic_ip_mask[INIC_MAX_NET_PORT_NUM] = {0};

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s: event_priv_t is NULL!\n", "event");
		goto func_exit;
	}

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s: device is NULL!\n", "event");
		goto func_exit;
	}

	switch (type) {
	case INIC_WLAN_GET_IP:
		rcu_read_lock();
		in_dev_for_each_ifa_rcu(ifa, global_idev.pndev[idx]->ip_ptr)
		memcpy(&inic_ip_addr[idx], &ifa->ifa_address, 4);
		rcu_read_unlock();
		rsp_ptr = &inic_ip_addr[idx];
		rsp_len = 4;
		break;
	case INIC_WLAN_GET_GW:
		dev_warn(global_idev.fullmac_dev, "INIC_WLAN_GET_GW is not supported. Add into global_idev if needed.");
		break;
	case INIC_WLAN_GET_GWMSK:
		rcu_read_lock();
		in_dev_for_each_ifa_rcu(ifa, global_idev.pndev[idx]->ip_ptr)
		memcpy(&inic_ip_mask[idx], &ifa->ifa_mask, 4);
		rcu_read_unlock();
		rsp_ptr = &inic_ip_mask[idx];
		rsp_len = 4;
		break;
	case INIC_WLAN_GET_HW_ADDR:
		rsp_ptr = (uint32_t *)global_idev.pndev[idx]->dev_addr;
		rsp_len = ETH_ALEN;
		break;
	case INIC_WLAN_IS_VALID_IP:
		/* todo in future */
		return;
	}

	memcpy(global_idev.event_priv.dev_req_network_info, rsp_ptr, rsp_len);
	p_ipc_msg->ret = (u32)global_idev.event_priv.dev_req_network_info_phy;

func_exit:
	return;
}

#ifdef CONFIG_NAN
static void llhw_event_nan_match_indicate(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	u8 type = p_ipc_msg->param_buf[0];
	u8 inst_id = p_ipc_msg->param_buf[1];
	u8 peer_inst_id = p_ipc_msg->param_buf[2];
	unsigned char *mac_addr = km4_phys_to_virt(p_ipc_msg->param_buf[3]);
	unsigned char *IEs = km4_phys_to_virt(p_ipc_msg->param_buf[4]);
	u32 info_len = p_ipc_msg->param_buf[5];
	u64 cookie = p_ipc_msg->param_buf[7] << 32 | p_ipc_msg->param_buf[6];

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	cfg80211_rtw_nan_handle_sdf(type, inst_id, peer_inst_id, mac_addr, info_len, dma_ie, cookie);

func_exit:
	return;
}

static void llhw_event_nan_cfgvendor_event_indicate(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	u8 event_id = p_ipc_msg->param_buf[0];
	unsigned char *event_addr = km4_phys_to_virt(p_ipc_msg->param_buf[1]);
	u32 size = p_ipc_msg->param_buf[2];

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	rtw_cfgvendor_nan_event_indication(event_id, event_addr, size);

func_exit:
	return;
}

static void llhw_event_nan_cfgvendor_cmd_reply(struct event_priv_t *event_priv, inic_ipc_dev_req_t *p_ipc_msg)
{
	struct device *pdev = NULL;
	unsigned char *data_addr = km4_phys_to_virt(p_ipc_msg->param_buf[0]);
	u32 size = p_ipc_msg->param_buf[1];

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s,%s: device is NULL in scan!\n", "event", __func__);
		goto func_exit;
	}

	rtw_cfgvendor_send_cmd_reply(data_addr, size);

func_exit:
	return;
}

#endif

void llhw_event_task(unsigned long data)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;
	struct device *pdev = NULL;
	inic_ipc_dev_req_t *p_recv_msg = NULL;
	int msg_len = 0;

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s: event_priv_t is NULL!\n", "event");
		goto func_exit;
	}

	pdev = global_idev.ipc_dev;
	if (!pdev) {
		dev_err(global_idev.fullmac_dev, "%s: device is NULL!\n", "event");
		goto func_exit;
	}

	msg_len = event_priv->recv_ipc_msg.msg_len;
	if (!event_priv->recv_ipc_msg.msg || !msg_len) {
		dev_err(global_idev.fullmac_dev, "%s: Invalid device message!\n", "event");
		goto func_exit;
	}

	p_recv_msg = km4_phys_to_virt(event_priv->recv_ipc_msg.msg);

	switch (p_recv_msg->enevt_id) {
	/* receive callback indication */
	case INIC_API_SCAN_USER_CALLBACK:
		/* If user callback provided as NULL, param_buf[1] appears NULL here. Do not make ptr. */
		spin_lock_bh(&event_priv->event_lock);
		/* https://jira.realtek.com/browse/AMEBAD2-1543 */
		cfg80211_rtw_scan_done_indicate(p_recv_msg->param_buf[0], NULL);
		spin_unlock_bh(&event_priv->event_lock);
		break;
	case INIC_API_SCAN_EACH_REPORT_USER_CALLBACK:
		//iiha_scan_each_report_cb_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_AUTO_RECONNECT:
		//iiha_autoreconnect_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_AP_CH_SWITCH:
		//iiha_ap_ch_switch_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_HDL:
		llhw_event_join_status_indicate(event_priv, p_recv_msg);
		break;
	case INIC_API_PROMISC_CALLBACK:
		//iiha_wifi_promisc_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_GET_LWIP_INFO:
		llhw_event_get_network_info(event_priv, p_recv_msg);
		break;
	case INIC_API_SET_NETIF_INFO:
		llhw_event_set_netif_info(event_priv, p_recv_msg);
		break;
	case INIC_API_CFG80211_SCAN_REPORT:
		llhw_event_scan_report_indicate(event_priv, p_recv_msg);
		break;
#ifdef CONFIG_NAN
	case INIC_API_CFG80211_NAN_REPORT_MATCH_EVENT:
		llhw_event_nan_match_indicate(event_priv, p_recv_msg);
		break;
	case INIC_API_CFG80211_NAN_DEL_FUNC:
		cfg80211_rtw_nan_func_free(p_recv_msg->param_buf[0]);
		break;
	case INIC_API_CFG80211_NAN_CFGVENDOR_EVENT:
		llhw_event_nan_cfgvendor_event_indicate(event_priv, p_recv_msg);
		break;
	case INIC_API_CFG80211_NAN_CFGVENDOR_CMD_REPLY:
		llhw_event_nan_cfgvendor_event_indicate(event_priv, p_recv_msg);
		break;
#endif
	default:
		dev_err(global_idev.fullmac_dev, "%s: Unknown Device event(%d)!\n\r", "event", p_recv_msg->enevt_id);
		break;
	}

	/*set enevt_id to 0 to notify NP that event is finished*/
	p_recv_msg->enevt_id = INIC_API_PROCESS_DONE;

func_exit:
	return;
}

static u32 llhw_ipc_event_interrupt(aipc_ch_t *ch, ipc_msg_struct_t *pmsg)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;
	u32 ret = 0;

	if (!event_priv) {
		dev_err(global_idev.fullmac_dev, "%s: event_priv_t is NULL in interrupt!\n", "event");
		goto func_exit;
	}

	/* copy ipc_msg from temp memory in ipc interrupt. */
	memcpy((u8 *) & (event_priv->recv_ipc_msg), (u8 *)pmsg, sizeof(ipc_msg_struct_t));
	tasklet_schedule(&(event_priv->api_tasklet));

func_exit:
	return ret;
}

int llhw_event_init(struct inic_device *idev)
{
	struct event_priv_t	*event_priv = &global_idev.event_priv;
	aipc_ch_t		*event_ch = global_idev.event_ch;

	/* initialize the mutex to send event_priv message. */
	mutex_init(&(event_priv->iiha_send_mutex));
	spin_lock_init(&event_priv->event_lock);

	event_priv->preq_msg = dmam_alloc_coherent(event_ch->pdev, sizeof(struct inic_ipc_host_req_msg), &event_priv->req_msg_phy_addr, GFP_KERNEL);
	if (!event_priv->preq_msg) {
		dev_err(global_idev.fullmac_dev, "%s: allloc req_msg error.\n", "event");
		return -ENOMEM;
	}

	/* coherent alloc some non-cache memory for transmit network_info to NP */
	event_priv->dev_req_network_info = dmam_alloc_coherent(event_ch->pdev, DEV_REQ_NETWORK_INFO_MAX_LEN, &event_priv->dev_req_network_info_phy, GFP_KERNEL);
	if (!event_priv->dev_req_network_info) {
		dev_err(global_idev.fullmac_dev, "%s: allloc dev_req_network_info error.\n", "event");
		return -ENOMEM;
	}

	/* initialize event tasklet */
	tasklet_init(&(event_priv->api_tasklet), llhw_event_task, (unsigned long)event_priv);

	return 0;
}

void llhw_event_deinit(void)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;

	/* free sema to wakeup the message queue task */
	tasklet_kill(&(event_priv->api_tasklet));

	dma_free_coherent(global_idev.ipc_dev, DEV_REQ_NETWORK_INFO_MAX_LEN,
					  event_priv->dev_req_network_info, event_priv->dev_req_network_info_phy);
	dma_free_coherent(global_idev.ipc_dev, sizeof(struct inic_ipc_host_req_msg), event_priv->preq_msg, event_priv->req_msg_phy_addr);

	/* deinitialize the mutex to send event_priv message. */
	mutex_destroy(&(event_priv->iiha_send_mutex));

	return;
}

struct aipc_ch_ops llhw_ipc_event_ops = {
	.channel_recv = llhw_ipc_event_interrupt,
};
