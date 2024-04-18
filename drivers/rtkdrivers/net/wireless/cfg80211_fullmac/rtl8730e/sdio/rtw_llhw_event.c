#include <rtw_cfg80211_fullmac.h>

static void llhw_event_scan_report_indicate(struct event_priv_t *event_priv, u32 *param_buf)
{
	u32 channel = param_buf[0];
	u32 frame_is_bcn = param_buf[1];
	s32 rssi = (s32)param_buf[2];
	u32 ie_len = param_buf[3];
	unsigned char *mac_addr = (u8 *) &param_buf[4];
	unsigned char *IEs = mac_addr + ETH_ALEN;

	cfg80211_rtw_inform_bss(channel, frame_is_bcn, rssi, mac_addr, IEs, ie_len);

	return;
}

static void llhw_event_join_status_indicate(struct event_priv_t *event_priv, u32 *param_buf)
{
	rtw_event_indicate_t event = (rtw_event_indicate_t)param_buf[0];
	int flags = (int)param_buf[1];
	int buf_len = (int)param_buf[2];
	char *buf = (char *) &param_buf[3];
	u16 disassoc_reason;

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

	return;
}

static void llhw_event_set_netif_info(struct event_priv_t *event_priv, u32 *param_buf)
{
	int idx = (int)param_buf[0];
	unsigned char *dev_addr = (u8 *)&param_buf[1];
	unsigned char last;
	int softap_addr_offset_idx = global_idev.wifi_user_config.softap_addr_offset_idx;

	dev_dbg(global_idev.fullmac_dev, "[fullmac]: set netif info.");

	if (idx >= TOTAL_IFACE_NUM) {
		dev_dbg(global_idev.fullmac_dev, "%s: interface %d not exist!\n", __func__, idx);
		goto func_exit;
	}

	if (!dev_addr) {
		dev_dbg(global_idev.fullmac_dev, "%s: mac address is NULL!\n", __func__);
		goto func_exit;
	}

	memcpy((void *)global_idev.pndev[idx]->dev_addr, dev_addr, ETH_ALEN);
	dev_dbg(global_idev.fullmac_dev, "MAC ADDR [%02x:%02x:%02x:%02x:%02x:%02x]", *global_idev.pndev[idx]->dev_addr,
			*(global_idev.pndev[idx]->dev_addr + 1), *(global_idev.pndev[idx]->dev_addr + 2),
			*(global_idev.pndev[idx]->dev_addr + 3), *(global_idev.pndev[idx]->dev_addr + 4),
			*(global_idev.pndev[idx]->dev_addr + 5));

	/*set ap port mac address*/
	memcpy((void *)global_idev.pndev[1]->dev_addr, global_idev.pndev[0]->dev_addr, ETH_ALEN);

	last = global_idev.pndev[0]->dev_addr[softap_addr_offset_idx] + 1;
	memcpy((void *)&global_idev.pndev[1]->dev_addr[softap_addr_offset_idx], &last, 1);

func_exit:
	return;
}

static u8 llhw_event_get_network_info(struct event_priv_t *event_priv, u32 *param_buf)
{
	uint32_t type = (uint32_t)param_buf[0];
	int idx = param_buf[1];
	/* input is used for INIC_WLAN_IS_VALID_IP, not used now. */
	/* uint8_t *input = (uint8_t *)(&param_buf[2]); */
	uint32_t *rsp_ptr = NULL;
	uint32_t rsp_len = 0;
	struct in_ifaddr *ifa = NULL;
	uint32_t inic_ip_addr[INIC_MAX_NET_PORT_NUM] = {0};
	uint32_t inic_ip_mask[INIC_MAX_NET_PORT_NUM] = {0};
	inic_api_info_t *ret_msg;
	u8 *buf;
	u32 buf_len;

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
		return 0;
	}

	buf_len = SIZE_TX_DESC + sizeof(inic_api_info_t) + rsp_len;
	buf = kzalloc(buf_len, GFP_KERNEL);
	if (buf) {
		/* fill inic_api_info_t */
		ret_msg = (inic_api_info_t *)(buf + SIZE_TX_DESC);
		ret_msg->event = INIC_WIFI_EVT_API_RETURN;
		ret_msg->api_id = INIC_API_GET_LWIP_INFO;

		/* copy data */
		memcpy((u8 *)(ret_msg + 1), (u8 *)rsp_ptr, rsp_len);

		/* send */
		llhw_host_send(buf, buf_len);

		kfree(buf);
	}

	return 1;
}

#ifdef CONFIG_NAN
static void llhw_event_nan_match_indicate(struct event_priv_t *event_priv, u32 *param_buf)
{
	u8 type = param_buf[0];
	u8 inst_id = param_buf[1];
	u8 peer_inst_id = param_buf[2];
	u32 info_len = param_buf[3];
	u64 cookie = ((u64)param_buf[5] << 32) | param_buf[4];
	unsigned char *mac_addr = (u8 *)&param_buf[6];
	unsigned char *IEs = mac_addr + ETH_ALEN;

	cfg80211_rtw_nan_handle_sdf(type, inst_id, peer_inst_id, mac_addr, info_len, IEs, cookie);

	return;
}

static void llhw_event_nan_cfgvendor_event_indicate(struct event_priv_t *event_priv, u32 *param_buf)
{
	u8 event_id = param_buf[0];
	u32 size = param_buf[1];
	unsigned char *event_addr = (u8 *)&param_buf[2];

	rtw_cfgvendor_nan_event_indication(event_id, event_addr, size);

	return;
}

static void llhw_event_nan_cfgvendor_cmd_reply(struct event_priv_t *event_priv, u32 *param_buf)
{
	u32 size = param_buf[0];
	unsigned char *data_addr = (u8 *)&param_buf[1];

	rtw_cfgvendor_send_cmd_reply(data_addr, size);

	return;
}

#endif

void llhw_event_task(struct work_struct *data)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;
	u8 already_ret = 0;
	inic_api_info_t *p_recv_msg = (inic_api_info_t *)event_priv->rx_api_msg;
	u32 *param_buf = (u32 *)(p_recv_msg + 1);
	inic_api_info_t *ret_msg;
	u8 *buf;
	u32 buf_len;

	switch (p_recv_msg->api_id) {

	/* receive callback indication */
	case INIC_API_SCAN_USER_CALLBACK:
		/* If user callback provided as NULL, param_buf[1] appears NULL here. Do not make ptr. */
		spin_lock_bh(&event_priv->event_lock);
		/* https://jira.realtek.com/browse/AMEBAD2-1543 */
		cfg80211_rtw_scan_done_indicate(param_buf[0], NULL);
		spin_unlock_bh(&event_priv->event_lock);
		break;
	case INIC_API_SCAN_EACH_REPORT_USER_CALLBACK:
		//iiha_scan_each_report_cb_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_AUTO_RECONNECT:
		//iiha_autoreconnect_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_WIFI_AP_CH_SWITCH:
		//iiha_ap_ch_switch_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_HDL:
		llhw_event_join_status_indicate(event_priv, param_buf);
		break;
	case INIC_API_PROMISC_CALLBACK:
		//iiha_wifi_promisc_hdl(event_priv, p_recv_msg);
		break;
	case INIC_API_GET_LWIP_INFO:
		already_ret = llhw_event_get_network_info(event_priv, param_buf);
		break;
	case INIC_API_SET_NETIF_INFO:
		llhw_event_set_netif_info(event_priv, param_buf);
		break;
	case INIC_API_CFG80211_SCAN_REPORT:
		llhw_event_scan_report_indicate(event_priv, param_buf);
		break;
#ifdef CONFIG_NAN
	case INIC_API_CFG80211_NAN_REPORT_MATCH_EVENT:
		llhw_event_nan_match_indicate(event_priv, param_buf);
		break;
	case INIC_API_CFG80211_NAN_DEL_FUNC:
		cfg80211_rtw_nan_func_free(param_buf[0]);
		break;
	case INIC_API_CFG80211_NAN_CFGVENDOR_EVENT:
		llhw_event_nan_cfgvendor_event_indicate(event_priv, param_buf);
		break;
	case INIC_API_CFG80211_NAN_CFGVENDOR_CMD_REPLY:
		llhw_event_nan_cfgvendor_cmd_reply(event_priv, param_buf);
		break;
#endif
	default:
		dev_err(global_idev.fullmac_dev, "%s: Unknown Device event(%d)!\n\r", "event", p_recv_msg->event);
		break;
	}

	if (already_ret == 0) {
		buf_len = SIZE_TX_DESC + sizeof(inic_api_info_t);
		buf = kzalloc(buf_len, GFP_KERNEL);
		if (buf) {
			/* fill and send ret_msg */
			ret_msg = (inic_api_info_t *)(buf + SIZE_TX_DESC);
			ret_msg->event = INIC_WIFI_EVT_API_RETURN;
			ret_msg->api_id = p_recv_msg->api_id;
			llhw_host_send(buf, buf_len);

			kfree(buf);
		}
	}

	/* free rx_event_msg */
	llhw_free_rxbuf((u8 *)p_recv_msg);

	return;
}

int llhw_event_init(struct inic_device *idev)
{
	struct event_priv_t	*event_priv = &global_idev.event_priv;

	/* initialize the mutex to send event_priv message. */
	mutex_init(&(event_priv->send_mutex));
	spin_lock_init(&event_priv->event_lock);
	init_completion(&event_priv->api_ret_sema);

	/* initialize event tasklet */
	INIT_WORK(&(event_priv->api_work), llhw_event_task);

	return 0;
}

void llhw_event_deinit(void)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;

	/* deinitialize the mutex to send event_priv message. */
	mutex_destroy(&(event_priv->send_mutex));

	complete_release(&event_priv->api_ret_sema);

	return;
}

