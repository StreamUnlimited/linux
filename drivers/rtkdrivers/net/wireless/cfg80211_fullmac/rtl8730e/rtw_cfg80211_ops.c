#include <rtw_cfg80211_fullmac.h>

static int cfg80211_rtw_get_station(struct wiphy *wiphy, struct net_device *ndev, const u8 *mac, struct station_info *sinfo)
{
	int ret = 0;
	rtw_phy_statistics_t *statistic_vir = NULL;
	dma_addr_t statistic_phy;

	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);

	if (rtw_netdev_idx(ndev) != 0) {
		dev_dbg(global_idev.fullmac_dev, "Only net device-0 is used for STA.");
	}

	statistic_vir = dmam_alloc_coherent(global_idev.fullmac_dev, sizeof(rtw_phy_statistics_t), &statistic_phy, GFP_KERNEL);
	if (!statistic_vir) {
		dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
		return -ENOMEM;
	}

	ret = llhw_ipc_wifi_get_statistics(statistic_phy);

	sinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL);
	sinfo->signal = statistic_vir->rssi;

	sinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
	sinfo->txrate.legacy = statistic_vir->supported_max_rate;

	dma_free_coherent(global_idev.fullmac_dev, sizeof(rtw_phy_statistics_t), statistic_vir, statistic_phy);

	return ret;
}

static int cfg80211_rtw_change_iface(struct wiphy *wiphy, struct net_device *ndev, enum nl80211_iftype type, struct vif_params *params)
{
	ndev->ieee80211_ptr->iftype = type;
	return 0;
}

int cfg80211_rtw_scan_done_indicate(unsigned int scanned_AP_num, void *user_data)
{
	//LINUX_TODO: aborted need to be realized
	struct cfg80211_scan_info info;

	if (!rtw_netdev_priv_is_on(global_idev.pndev[0])) {
		dev_dbg(global_idev.fullmac_dev, "sta is down, finish scan.");
		return -1;
	}

	if (!global_idev.mlme_priv.pscan_req_global) {
		dev_dbg(global_idev.fullmac_dev, "Last scan req has been finished. Wait for next. ");
		return -1;
	}

	memset(&info, 0, sizeof(info));
	info.aborted = 0;
	dev_dbg(global_idev.fullmac_dev, "%s: scan request(%x) done.", __FUNCTION__, (u32)global_idev.mlme_priv.pscan_req_global);
	cfg80211_scan_done(global_idev.mlme_priv.pscan_req_global, &info);

	/* cfg80211_scan_done will clear last request. Clean global scan request as well. */
	global_idev.mlme_priv.pscan_req_global = NULL;
	return 0;
}

void cfg80211_rtw_inform_bss(u32 channel, u32 frame_is_bcn, s32 rssi, u8 *mac_addr, u8 *IEs, u32 ie_len)
{
	struct ieee80211_channel *notify_channel = NULL;
	struct cfg80211_bss *bss = NULL;
	s32 notify_signal;
	u8 *pbuf;
	size_t buf_size = 1000;
	size_t len, bssinf_len = 0;
	struct rtw_ieee80211_hdr_3addr *pwlanhdr;
	unsigned short *fctrl;
	u8  bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct wiphy *wiphy = global_idev.pwiphy_global;
	struct ieee80211_supported_band *sband;
	int i, j;

	pbuf = kzalloc(buf_size, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	if (pbuf == NULL) {
		dev_dbg(global_idev.fullmac_dev, "%s pbuf allocate failed	!!\n", __FUNCTION__);
		return;
	}

	bssinf_len = ie_len + sizeof(struct rtw_ieee80211_hdr_3addr);
	if (bssinf_len > buf_size) {
		dev_dbg(global_idev.fullmac_dev, "%s IE Length too long > %zu byte\n", __FUNCTION__, buf_size);
		goto exit;
	}

	for (i = 0; i < 2; i++) {
		sband = wiphy->bands[i];
		if (!sband) {
			continue;
		}

		for (j = 0; j < sband->n_channels; j++) {
			if (sband->channels[j].hw_value == channel) {
				notify_channel = &sband->channels[j];
				break;
			}
		}
		if (notify_channel) {
			break;
		}
	}

	notify_signal = 100 * rssi;

	pwlanhdr = (struct rtw_ieee80211_hdr_3addr *)pbuf;
	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if (frame_is_bcn) { /* WIFI_BEACON */
		memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
		set_frame_sub_type(pbuf, BIT(7));
	} else {
		memcpy(pwlanhdr->addr1, global_idev.pndev[0]->dev_addr, ETH_ALEN);
		set_frame_sub_type(pbuf, BIT(6) | BIT(4));
	}

	memcpy(pwlanhdr->addr2, mac_addr, ETH_ALEN);
	memcpy(pwlanhdr->addr3, mac_addr, ETH_ALEN);

	len = sizeof(struct rtw_ieee80211_hdr_3addr);
	memcpy((pbuf + len), IEs, ie_len);
	len += ie_len;

	bss = cfg80211_inform_bss_frame(wiphy, notify_channel, (struct ieee80211_mgmt *)pbuf, len, notify_signal, GFP_ATOMIC);

	if (!bss) {
		goto exit;
	}

	cfg80211_put_bss(wiphy, bss);

exit:
	if (pbuf) {
		kfree(pbuf);
	}
}

static int cfg80211_rtw_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
	int i;
	int ret = 0;
	struct cfg80211_ssid *ssids = request->ssids;
	struct wireless_dev *wdev;
	u32 wlan_idx = 0;
	struct net_device *pnetdev = NULL;
	rtw_scan_param_t scan_param = {0};

	/* coherent alloc: revise vir addr will be mapped to phy addr. Send phy addr to rtos by ipc. */
	u8 *channel_list_vir = NULL;
	dma_addr_t channel_list_phy;

	/* coherent alloc: revise vir addr will be mapped to phy addr. Send phy addr to rtos by ipc. */
	u8 *ssid_vir = NULL;
	dma_addr_t ssid_phy;

	u32 ssid_len = 0;
	u32 channel_num = 0;

	wdev = request->wdev;
	pnetdev = wdev_to_ndev(wdev);
	if (pnetdev) {
		wlan_idx = rtw_netdev_idx(pnetdev);
	} else {
		ret = -EINVAL;
		goto exit;
	}

	dev_dbg(global_idev.fullmac_dev, "cfg80211_rtw_scan enter\n");

	memset(&scan_param, 0, sizeof(rtw_scan_param_t));

	/* Add fake callback to inform rots give scan indicate when scan done. */
	scan_param.scan_user_callback = (long int (*)(unsigned int,  void *))0xffffffff;
	scan_param.ssid = NULL;

	//LINUX_TODO: do we need to support multi ssid scan?
	for (i = 0; i < request->n_ssids && ssids && i < RTW_SSID_SCAN_AMOUNT; i++) {
		if (ssids[i].ssid_len) {
			ssid_vir = dmam_alloc_coherent(global_idev.fullmac_dev, ssids[0].ssid_len, &ssid_phy, GFP_KERNEL);
			if (!ssid_vir) {
				dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
				return -ENOMEM;
			}
			memcpy(ssid_vir, ssids[0].ssid, ssids[0].ssid_len);
			ssid_len = ssids[0].ssid_len;
			/* Only hidden ssid scan has ssid param. */
			scan_param.ssid = (char *)ssid_phy;
			/* Multi ssid need other operation here. Only support one here. */
			break;
		}
	}

	/* no ssid entry, set the scan type as passive */
	if (request->n_ssids == 0) {
		scan_param.options = RTW_SCAN_PASSIVE;
	}

	if (request->n_channels) {
		channel_list_vir = dmam_alloc_coherent(global_idev.fullmac_dev, request->n_channels, &channel_list_phy, GFP_KERNEL);
		if (!channel_list_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			if (ssid_vir) {
				dma_free_coherent(global_idev.fullmac_dev, ssids[0].ssid_len, ssid_vir, ssid_phy);
			}
			return -ENOMEM;
		}
	}

	for (i = 0; i < request->n_channels && i < RTW_CHANNEL_SCAN_AMOUNT; i++) {
		if (request->channels[i]->flags & IEEE80211_CHAN_DISABLED) {
			continue;
		}
		*(channel_list_vir + channel_num) = request->channels[i]->hw_value;
		channel_num++;
	}
	scan_param.channel_list_num = channel_num;
	if (request->n_channels) {
		/* If channel list exists. */
		scan_param.channel_list = (unsigned char *)channel_list_phy;
	}

	ret = llhw_ipc_wifi_scan(&scan_param, ssid_len, 0);
	if (ret < 0) {
		//_rtw_cfg80211_surveydone_event_callback(padapter, request);
		struct cfg80211_scan_info info;
		memset(&info, 0, sizeof(info));
		info.aborted = 0;
		cfg80211_scan_done(request, &info);
		dev_dbg(global_idev.fullmac_dev, "%s: scan request(%x) fail.", __FUNCTION__, (u32)request);
		global_idev.mlme_priv.pscan_req_global = NULL;
	} else {
		global_idev.mlme_priv.pscan_req_global = request;
		dev_dbg(global_idev.fullmac_dev, "%s: scan request(%x) start.", __FUNCTION__, (u32)request);
	}

	if (ssid_vir) {
		dma_free_coherent(global_idev.fullmac_dev, ssids[0].ssid_len, ssid_vir, ssid_phy);
	}
	if (channel_list_vir) {
		dma_free_coherent(global_idev.fullmac_dev, request->n_channels, channel_list_vir, channel_list_phy);
	}

exit:
	return ret;
}


static void cfg80211_rtw_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
}

static int cfg80211_rtw_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_join_ibss(struct wiphy *wiphy, struct net_device *ndev,
								  struct cfg80211_ibss_params *params)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_leave_ibss(struct wiphy *wiphy, struct net_device *ndev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

void cfg80211_rtw_connect_indicate(fullmac_join_status join_status, void *user_data, size_t user_data_len)
{
	struct mlme_priv_t *mlme_priv = &global_idev.mlme_priv;

	mlme_priv->rtw_join_status = join_status;

	/* Merge from wifi_join_status_indicate. */
	if (join_status == RTW_JOINSTATUS_SUCCESS) {
		/* if Synchronous connection, up sema when connect success */
		if (mlme_priv->join_block_param && mlme_priv->join_block_param->block) {
			complete(&mlme_priv->join_block_param->join_sema);
		}
	}

	/* step 1: internal process for different status*/
	if (join_status == RTW_JOINSTATUS_STARTING) {
		dev_dbg(global_idev.fullmac_dev, "[fullmac] --- %s --- join starting", __func__);
		memset(mlme_priv->assoc_req_ie, 0, ASSOC_IE_MAX_LEN);
		memset(mlme_priv->assoc_rsp_ie, 0, ASSOC_IE_MAX_LEN);
		mlme_priv->assoc_req_ie_len = 0;
		mlme_priv->assoc_rsp_ie_len = 0;
		return;
	}

	if (join_status == RTW_JOINSTATUS_ASSOCIATING) {
		if (user_data_len > 0) {
			memcpy(mlme_priv->assoc_req_ie, (u8 *)user_data, user_data_len);
			mlme_priv->assoc_req_ie_len = user_data_len;
		}
		return;
	}

	if (join_status == RTW_JOINSTATUS_ASSOCIATED) {
		if (user_data_len > 0) {
			memcpy(mlme_priv->assoc_rsp_ie, (u8 *)user_data, user_data_len);
			mlme_priv->assoc_rsp_ie_len = user_data_len;
			dev_dbg(global_idev.fullmac_dev, "[fullmac] --- %s --- join associated", __func__);
			/* Inform connect information. */
			/* Different between cfg80211_connect_result and cfg80211_connect_bss are described in net/cfg80211.h. */
			/* if connect_result warning, that means get_bss fail (need check), one reason is WPA_S calls disconnect ops, which resulting in wdev->ssid_len = 0 */
			cfg80211_connect_result(global_idev.pndev[0], mlme_priv->assoc_rsp_ie + 16,
									mlme_priv->assoc_req_ie + WLAN_HDR_A3_LEN + 2, mlme_priv->assoc_req_ie_len - WLAN_HDR_A3_LEN - 2,
									mlme_priv->assoc_rsp_ie + WLAN_HDR_A3_LEN + 6, mlme_priv->assoc_rsp_ie_len -  WLAN_HDR_A3_LEN - 6,
									WLAN_STATUS_SUCCESS, GFP_ATOMIC);
			netif_carrier_on(global_idev.pndev[0]);
		}
		return;
	}

	if (join_status == RTW_JOINSTATUS_FAIL) {
		dev_dbg(global_idev.fullmac_dev, "[fullmac] --- %s --- join failed up sema.", __func__);
		/* merge from wifi_join_status_indicate if synchronous connection, up sema when connect fail*/
		if (mlme_priv->join_block_param && mlme_priv->join_block_param->block) {
			complete(&mlme_priv->join_block_param->join_sema);
		}
		dev_dbg(global_idev.fullmac_dev, "[fullmac] --- %s --- join failed inform cfg80211.", __func__);
		cfg80211_connect_result(global_idev.pndev[0], NULL, NULL, 0, NULL, 0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_ATOMIC);
		return;
	}
}

void cfg80211_rtw_disconnect_indicate(u16 reason, u8 locally_generated)
{
	/* Do it first for tx broadcast pkt after disconnection issue! */
	netif_carrier_off(global_idev.pndev[0]);
	dev_dbg(global_idev.fullmac_dev, "%s reason:%d\n", __func__, reason);
	cfg80211_disconnected(global_idev.pndev[0], reason, NULL, 0, locally_generated, GFP_ATOMIC);
}

void cfg80211_rtw_external_auth_request(char *buf, int buf_len)
{
	struct cfg80211_external_auth_params *auth_ext_para = &global_idev.mlme_priv.auth_ext_para;

	auth_ext_para->action = NL80211_EXTERNAL_AUTH_START;
	memcpy(auth_ext_para->bssid, buf, ETH_ALEN);
	auth_ext_para->key_mgmt_suite = 0x8ac0f00;

	/*ap mode doesn't need call this ops, sta will trigger auth*/
	cfg80211_external_auth_request(global_idev.pndev[0], auth_ext_para, GFP_ATOMIC);
	dev_dbg(global_idev.fullmac_dev, "%s, ssid=%s, len=%d\n", __func__, auth_ext_para->ssid.ssid, auth_ext_para->ssid.ssid_len);
}

static int cfg80211_rtw_connect(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_connect_params *sme)
{
	int ret = 0;
	rtw_network_info_t connect_param = {0};
	u32 wlan_idx = 0;
	/* coherent alloc: revise vir addr will be mapped to phy addr. Send phy addr to rtos by ipc. */
	u8 *vir_pwd = NULL;
	dma_addr_t phy_pwd;
	int rsnx_ielen = 0;
	u8 *prsnx;
	struct cfg80211_external_auth_params *auth_ext_para = &global_idev.mlme_priv.auth_ext_para;

	if (ndev) {
		wlan_idx = rtw_netdev_idx(ndev);
	} else {
		ret = -EINVAL;
		goto exit;
	}

	dev_dbg(global_idev.fullmac_dev, "=>"FUNC_NDEV_FMT" - Start to Connection\n", FUNC_NDEV_ARG(ndev));
	dev_dbg(global_idev.fullmac_dev,
			"ssid=%s, ssid_len=%d, freq=%d, bssid=[0x%x:0x%x:0x%x:0x%x:0x%x:0x%x], privacy=%d, key=%p, key_len=%d, key_idx=%d, auth_type=%d\n",
			sme->ssid, sme->ssid_len, sme->channel->center_freq,
			sme->bssid[0], sme->bssid[1], sme->bssid[2], sme->bssid[3], sme->bssid[4], sme->bssid[5],
			sme->privacy, sme->key, sme->key_len, sme->key_idx, sme->auth_type);
	dev_dbg(global_idev.fullmac_dev, " ciphers_pairwise=0x%x, cipher_group=0x%x,, akm_suites=0x%x\n",
			sme->crypto.ciphers_pairwise[0], sme->crypto.cipher_group, sme->crypto.akm_suites[0]);

	memset(&connect_param, 0, sizeof(rtw_network_info_t));

	memcpy(connect_param.ssid.val, (u8 *)sme->ssid, sme->ssid_len);
	connect_param.ssid.len = sme->ssid_len;

	if (sme->key_len) {
		vir_pwd = dmam_alloc_coherent(global_idev.fullmac_dev, sme->key_len, &phy_pwd, GFP_KERNEL);
		if (!vir_pwd) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			return -ENOMEM;
		}
		memcpy(vir_pwd, (u8 *)sme->key, sme->key_len);
		connect_param.password = (unsigned char *)phy_pwd;
	} else {
		connect_param.password = NULL;
	}

	if (sme->crypto.akm_suites[0] ==  WIFI_AKM_SUITE_SAE) {
		/*SAE need request wpa_suppilcant to do auth*/
		memcpy(auth_ext_para->ssid.ssid, (u8 *)sme->ssid, sme->ssid_len);
		auth_ext_para->ssid.ssid_len = sme->ssid_len;
	}

	connect_param.password_len = sme->key_len;
	connect_param.key_id = sme->key_idx;
	/* Set security type as open in linux. RTOS will check saved scan list and find the actually security type. */
	connect_param.security_type = RTW_SECURITY_OPEN;

	/* set channel to let low level do scan on specific channel */
	connect_param.channel = rtw_freq2ch(sme->channel->center_freq);
	connect_param.pscan_option = 0x2;

	/* do connect by bssid */
	memset(connect_param.bssid.octet, 0, ETH_ALEN);
	if (sme->bssid) {
		memcpy(connect_param.bssid.octet, sme->bssid, ETH_ALEN);
	}

	/* set rsnxe*/
	prsnx = rtw_get_ie((u8 *)sme->ie, WLAN_EID_RSNX, &rsnx_ielen, sme->ie_len);
	if (prsnx && (rsnx_ielen > 0)) {
		if ((rsnx_ielen + 2) <= RSNXE_MAX_LEN) {
			memcpy(connect_param.wpa_supp.rsnxe_ie, prsnx, rsnx_ielen + 2);
		} else {
			printk("%s:no more buf to save RSNX Cap!rsnx_ielen=%d\n", __func__, rsnx_ielen + 2);
		}
	}

	connect_param.joinstatus_user_callback = NULL;

	ret = llhw_ipc_wifi_connect(&connect_param, 0);
	if (ret < 0) {
		/* KM4 connect failed */
		cfg80211_connect_result(ndev, NULL, NULL, 0, NULL, 0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_ATOMIC);
	}

	if (sme->key_len) {
		dma_free_coherent(global_idev.fullmac_dev, strlen(vir_pwd), vir_pwd, phy_pwd);
	}

exit:
	return ret;
}

static int cfg80211_rtw_disconnect(struct wiphy *wiphy, struct net_device *ndev, u16 reason_code)
{
	int ret = 0;

	dev_dbg(global_idev.fullmac_dev, "[fullmac] --- %s ---", __func__);

	ret = llhw_ipc_wifi_disconnect();

	/* KM4 will report WIFI_EVENT_DISCONNECT event to linux, after disconnect done */

	return ret;
}

static int cfg80211_rtw_set_txpower(struct wiphy *wiphy, struct wireless_dev *wdev, enum nl80211_tx_power_setting type, int mbm)
{
	dev_dbg(global_idev.fullmac_dev, "%s set %d %d", __func__, type, mbm);

	/* Operation not permitted */
	return -EPERM;
}

static int cfg80211_rtw_get_txpower(struct wiphy *wiphy, struct wireless_dev *wdev, int *dbm)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);

	/* BB REG 0x1C78[8:0] is tx_power * 4. Unit of tx_power is dBm. (axi_mem_start + 0x11c78) */
	/* Value sometimes 0, sometimes not realtime, return -EINVAL to make TX-Power disappear. */

	return -EPERM;
}

static int cfg80211_rtw_set_power_mgmt(struct wiphy *wiphy, struct net_device *ndev, bool enabled, int timeout)
{
	dev_dbg(global_idev.fullmac_dev, "%s enable = %d", __func__, enabled);

	/* Operation not permitted */
	return -EPERM;
}

#ifdef FULLMAC_TODO
static int cfg80211_rtw_get_channel(struct wiphy *wiphy, struct wireless_dev *wdev, struct cfg80211_chan_def *chandef)
{
	int retval = 1;
	int i, j = 0;
	struct ieee80211_supported_band *sband = NULL;
	int cfreq;
	struct ieee80211_channel *ieee_chan = NULL;
	int is_connected = -1;
	struct net_device *pnetdev = NULL;
	int wlan_idx = -1;
	u8 ch = 0;

	//LINUX_TODO: for monitor mode, just reutn retval=1
	pnetdev = wdev_to_ndev(wdev);
	if (pnetdev && rtw_netdev_priv_is_on(pnetdev)) {
		wlan_idx = rtw_netdev_idx(pnetdev);
		if (wlan_idx == 0) { //STA mode
			is_connected = llhw_ipc_wifi_is_connected_to_ap();
			if (is_connected != 0) { /* 0 correspond to RTW_SUCCESS*/
				return retval;
			}
		}
	} else {
		return retval;
	}

	llhw_ipc_wifi_get_channel(wlan_idx, &ch);
	dev_dbg(global_idev.fullmac_dev, "%s %d channel=%d\n", __FUNCTION__, __LINE__, ch);
	memset(chandef, 0, sizeof(*chandef));

	cfreq = rtw_ch2freq(ch);

	for (i = 0; i < 2; i++) {
		sband = wiphy->bands[i];
		if (!sband) {
			continue;
		}

		for (j = 0; j < sband->n_channels; j++) {
			if (sband->channels[j].hw_value == ch) {
				ieee_chan = &sband->channels[j];
				break;
			}
		}
		if (ieee_chan) {
			break;
		}
	}

	//LINUX_TODO: ht ? NL80211_CHAN_WIDTH_20 : NL80211_CHAN_WIDTH_20_NOHT;
	chandef->width = NL80211_CHAN_WIDTH_20;
	chandef->chan = ieee_chan;
	chandef->center_freq1 = cfreq;
	//dev_dbg(global_idev.fullmac_dev, "%s %d chandef=%d %d %d\n", __FUNCTION__, __LINE__, chandef->width, chandef->chan, chandef->center_freq1);
	retval = 0;

	return retval;
}
#endif // FULLMAC_TODO

static int cfg80211_rtw_set_monitor_channel(struct wiphy *wiphy, struct cfg80211_chan_def *chandef)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_external_auth_status(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_external_auth_params *params)
{
	if (rtw_netdev_idx(dev) == 0) {
		dev_dbg(global_idev.fullmac_dev, "[STA]: %s: auth_status=%d\n", __func__, params->status);
		/* interface change later. */
		if (params->status == WLAN_STATUS_SUCCESS) {
			llhw_ipc_wifi_sae_succ_start_assoc();
		}
	} else {
		dev_dbg(global_idev.fullmac_dev, "[SoftAP]: %s: auth_status=%d\n", __func__, params->status);
		/* SoftAP is supposed to go into this interface instead of private one. */
	}

	return 0;
}

#ifdef CONFIG_CFG80211_SME_OFFLOAD
static int cfg80211_rtw_auth(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_auth_request *req)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_assoc(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_assoc_request *req)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_deauth(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_deauth_request *req)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_disassoc(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_disassoc_request *req)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */

static s32 cfg80211_rtw_remain_on_channel(struct wiphy *wiphy, struct wireless_dev *wdev, struct ieee80211_channel *channel, unsigned int duration, u64 *cookie)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static s32 cfg80211_rtw_cancel_remain_on_channel(struct wiphy *wiphy, struct wireless_dev *wdev, u64 cookie)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}

static int cfg80211_rtw_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev, struct cfg80211_mgmt_tx_params *params, u64 *cookie)
{
	struct net_device *ndev = wdev_to_ndev(wdev);
	u8 wlan_idx = rtw_netdev_idx(ndev);
	struct ieee80211_channel *chan = params->chan;
	const u8 *buf = params->buf;
	size_t len = params->len;
	int ret = 0;
	bool ack = true;
	u8 tx_ch;
	u8 frame_styp;
	static u32 mgmt_tx_cookie = 0;

	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);

	if (chan == NULL) {
		ret = -EINVAL;
		goto exit;
	}

	tx_ch = (u8)ieee80211_frequency_to_channel(chan->center_freq);

	/* cookie generation */
	*cookie = mgmt_tx_cookie++;

	/* indicate ack before issue frame to avoid racing with rsp frame */
	cfg80211_mgmt_tx_status(wdev, *cookie, buf, len, ack, GFP_KERNEL);
	frame_styp = le16_to_cpu(((struct rtw_ieee80211_hdr_3addr *)buf)->frame_ctl) & IEEE80211_FCTL_STYPE;

	if (frame_styp == IEEE80211_STYPE_AUTH) {
		dev_dbg(global_idev.fullmac_dev, "wpa_s tx auth\n");
		//dev_dbg(global_idev.fullmac_dev, "tx_ch=%d, no_cck=%u, da="MAC_FMT"\n", tx_ch, no_cck, MAC_ARG(GetAddr1Ptr(buf)));
		/*LINUX_TODO, AP mode needs queue confirm frame until external auth status update*/
		goto dump;
	} else {
		dev_dbg(global_idev.fullmac_dev, "mgmt tx todo, frame_type:0x%x\n", frame_styp);
		return ret;
	}

	/*LINUX_TODO, action frame, probe response*/

dump:
	/*ignore tx_ch/ no_cck/ wait_ack temporary*/
	llhw_ipc_wifi_tx_mgnt(wlan_idx, buf, len);

exit:
	return ret;
}

static void cfg80211_rtw_mgmt_frame_register(struct wiphy *wiphy, struct wireless_dev *wdev, u16 frame_type, bool reg)
{
	//dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s reg = %d", __func__, reg);
}

void cfg80211_rtw_ops_sta_init(void)
{
	struct cfg80211_ops *ops = &global_idev.rtw_cfg80211_ops;

	ops->change_virtual_intf = cfg80211_rtw_change_iface;
	ops->get_station = cfg80211_rtw_get_station;
	ops->scan = cfg80211_rtw_scan;
	ops->abort_scan = cfg80211_rtw_abort_scan;
	ops->set_wiphy_params = cfg80211_rtw_set_wiphy_params;
	ops->connect = cfg80211_rtw_connect;
	ops->disconnect = cfg80211_rtw_disconnect;
	ops->join_ibss = cfg80211_rtw_join_ibss;
	ops->leave_ibss = cfg80211_rtw_leave_ibss;
	ops->set_tx_power = cfg80211_rtw_set_txpower;
	ops->get_tx_power = cfg80211_rtw_get_txpower;
	ops->set_power_mgmt = cfg80211_rtw_set_power_mgmt;
#ifdef FULLMAC_TODO
	ops->get_channel = cfg80211_rtw_get_channel;
#endif
	ops->set_monitor_channel = cfg80211_rtw_set_monitor_channel;
	ops->remain_on_channel = cfg80211_rtw_remain_on_channel;
	ops->cancel_remain_on_channel = cfg80211_rtw_cancel_remain_on_channel;
	ops->mgmt_tx = cfg80211_rtw_mgmt_tx;
	ops->mgmt_frame_register = cfg80211_rtw_mgmt_frame_register;
	ops->external_auth = cfg80211_rtw_external_auth_status;
#ifdef CONFIG_CFG80211_SME_OFFLOAD
	ops->auth = cfg80211_rtw_auth;
	ops->deauth = cfg80211_rtw_deauth;
	ops->assoc = cfg80211_rtw_assoc;
	ops->disassoc = cfg80211_rtw_disassoc;
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */
}
