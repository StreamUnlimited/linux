#include <rtw_cfg80211_fullmac.h>

static struct wireless_dev *cfg80211_rtw_add_virtual_intf(struct wiphy *wiphy, const char *name,
		unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params)
{
	return 0;
}

static int cfg80211_rtw_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	return 0;
}

#if defined(CONFIG_RTW_MACADDR_ACL) && (CONFIG_RTW_MACADDR_ACL == 1)
static int cfg80211_rtw_set_mac_acl(struct wiphy *wiphy, struct net_device *ndev, const struct cfg80211_acl_data *params)
{
	return 0;
}
#endif /* CONFIG_RTW_MACADDR_ACL && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)) */

static int cfg80211_rtw_add_station(struct wiphy *wiphy, struct net_device *ndev, const u8 *mac, struct station_parameters *params)
{
	return 0;
}

static int cfg80211_rtw_del_station(struct wiphy *wiphy, struct net_device *ndev, struct station_del_parameters *params)
{
	int ret = 0;
	u8 wlan_idx = rtw_netdev_idx(ndev);
	u8 *mac_vir = NULL;
	dma_addr_t mac_phy;

	if (!params->mac) {
		/*null means delete all sta, not implement right now*/
		return ret;
	}
	dev_dbg(global_idev.fullmac_dev, "[fullmac]:%s %x:%x:%x:%x:%x:%x", __func__,
			params->mac[0], params->mac[1], params->mac[2], params->mac[3], params->mac[4], params->mac[5]);
	mac_vir = dmam_alloc_coherent(global_idev.fullmac_dev, 6, &mac_phy, GFP_KERNEL);
	if (!mac_vir) {
		dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
		return -ENOMEM;
	}
	memcpy(mac_vir, params->mac, 6);
	ret = llhw_ipc_wifi_del_sta(wlan_idx, (u8 *) mac_phy);

	if (mac_vir) {
		dma_free_coherent(global_idev.fullmac_dev, 6, mac_vir, mac_phy);
	}
	return ret;
}

static int cfg80211_rtw_change_station(struct wiphy *wiphy, struct net_device *ndev, const u8 *mac, struct station_parameters *params)
{
	return 0;
}

static int cfg80211_rtw_dump_station(struct wiphy *wiphy, struct net_device *ndev, int idx, u8 *mac, struct station_info *sinfo)
{
	return 0;
}

static int cfg80211_rtw_change_bss(struct wiphy *wiphy, struct net_device *ndev, struct bss_parameters *params)
{
	return 0;
}

static int cfg80211_rtw_set_txq_params(struct wiphy *wiphy, struct net_device *ndev, struct ieee80211_txq_params *params)
{
	int ret = 0;
	/*
	 * @AC_param: format is shown as in below ,
	 * +--------------------------+-------------+-------------+
	 * |        TXOP Limit        |ECWmin/ECWmax|  ACI/AIFSN  |
	 * +--------------------------+-------------+-------------+
	 * 	BIT31~16 corresponding to TXOP Limit, BIT15~8 corresponding
	 * 	to ECWmin/ECWmax, BIT7~0 corresponding to ACI/AIFSN.
	*/
	unsigned int AC_param = 0;
	u8	shift_count = 0;
	u8 aifsn, aci = 0, ECWMin, ECWMax;
	u16 TXOP;

	switch (params->ac) {
	case NL80211_AC_VO:
		aci = 3;
		break;
	case NL80211_AC_VI:
		aci = 2;
		break;
	case NL80211_AC_BK:
		aci = 1;
		break;
	case NL80211_AC_BE:
		aci = 0;
		break;
	default:
		break;
	}

	while ((params->cwmin + 1) >> shift_count != 1) {
		shift_count++;
		if (shift_count == 15) {
			break;
		}
	}

	ECWMin = shift_count;

	shift_count = 0;
	while ((params->cwmax + 1) >> shift_count != 1) {
		shift_count++;
		if (shift_count == 15) {
			break;
		}
	}

	ECWMax = shift_count;

	TXOP = params->txop;
	aifsn = params->aifs;

	AC_param = (aifsn & 0xf) | ((aci & 0x3) << 5) | ((ECWMin & 0xf) << 8) | ((ECWMax & 0xf) << 12) | ((TXOP & 0xffff) << 16);

	dev_dbg(global_idev.fullmac_dev, "=>"FUNC_NDEV_FMT" - Set TXQ params: aifsn=%d aci=%d ECWmin=%d, ECWmax=%d, TXOP=%d, AC_param=0x%x\n",
			FUNC_NDEV_ARG(ndev), aifsn, aci, ECWMin, ECWMax, TXOP, AC_param);

	ret = llhw_ipc_wifi_set_EDCA_params(&AC_param);

	return ret;
}

#ifdef CONFIG_CFG80211_SME_OFFLOAD
static int cfg80211_rtw_probe_client(struct wiphy *wiphy, struct net_device *ndev, const u8 *peer, u64 *cookie)
{
	return 0;
}
#endif // CONFIG_CFG80211_SME_OFFLOAD

static int cfg80211_rtw_change_beacon(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_beacon_data *info)
{
	return 0;
}

static int cfg80211_rtw_channel_switch(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_csa_settings *params)
{
	return 0;
}

void cfg80211_rtw_sta_assoc_indicate(char *buf, int buf_len)
{
	struct station_info sinfo;
	u8 frame_styp, ie_offset = 0;

	memset(&sinfo, 0, sizeof(sinfo));
	frame_styp = le16_to_cpu(((struct rtw_ieee80211_hdr_3addr *)buf)->frame_ctl) & IEEE80211_FCTL_STYPE;
	if (frame_styp == IEEE80211_STYPE_ASSOC_REQ) {
		ie_offset = 4;
	} else { /* WIFI_REASSOCREQ */
		ie_offset = 10;
	}
	sinfo.filled = 0;
	sinfo.assoc_req_ies = buf + WLAN_HDR_A3_LEN + 4;
	sinfo.assoc_req_ies_len = buf_len - WLAN_HDR_A3_LEN - 4;
	cfg80211_new_sta(global_idev.pndev[1], get_addr2_ptr(buf), &sinfo, GFP_ATOMIC);
}

static int cfg80211_rtw_start_ap(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_ap_settings *settings)
{
	int ret = 0;
	rtw_softap_info_t softAP_config = {0};
	char fake_pwd[] = "12345678";
	u8 *pwd_vir = NULL;
	dma_addr_t pwd_phy;

	dev_dbg(global_idev.fullmac_dev, "=>"FUNC_NDEV_FMT" - Start Softap\n", FUNC_NDEV_ARG(ndev));

	memcpy(softAP_config.ssid.val, (u8 *)settings->ssid, settings->ssid_len);
	softAP_config.ssid.len = settings->ssid_len;
	softAP_config.channel = (u8) ieee80211_frequency_to_channel(settings->chandef.chan->center_freq);

	dev_dbg(global_idev.fullmac_dev, "wpa_versions=%d\n", settings->crypto.wpa_versions);
	dev_dbg(global_idev.fullmac_dev, "n_ciphers_pairwise=%d\n", settings->crypto.n_ciphers_pairwise);
	dev_dbg(global_idev.fullmac_dev, "ciphers_pairwise=0x%x\n", settings->crypto.ciphers_pairwise[0]);
	dev_dbg(global_idev.fullmac_dev, "n_akm_suites=%d\n", settings->crypto.n_akm_suites);
	dev_dbg(global_idev.fullmac_dev, "akm_suites=0x%x\n", settings->crypto.akm_suites[0]);
	dev_dbg(global_idev.fullmac_dev, "cipher_group=0x%x\n", settings->crypto.cipher_group);
	dev_dbg(global_idev.fullmac_dev, "wep_tx_key=%d\n", settings->crypto.wep_tx_key);
	dev_dbg(global_idev.fullmac_dev, "sae_pwd_len=%d\n", settings->crypto.sae_pwd_len);

	if (settings->privacy) {
		if (settings->crypto.n_ciphers_pairwise > 1 || settings->crypto.n_akm_suites > 1) {
			dev_dbg(global_idev.fullmac_dev, "wpa mixed mode, not support right now!\n");
			return -EPERM;
		}
		if ((settings->crypto.wpa_versions == 2) && ((u8)settings->crypto.akm_suites[0] == 0x08)) {
			softAP_config.security_type = RTW_SECURITY_WPA3_AES_PSK;
		} else if ((settings->crypto.wpa_versions == 2) && ((u8)settings->crypto.ciphers_pairwise[0] == 0x04)) {
			softAP_config.security_type = RTW_SECURITY_WPA2_AES_PSK;
		} else if ((settings->crypto.wpa_versions == 2) && ((u8)settings->crypto.ciphers_pairwise[0] == 0x02)) {
			softAP_config.security_type = RTW_SECURITY_WPA2_TKIP_PSK;
		} else if (settings->crypto.wpa_versions == 1) {
			dev_dbg(global_idev.fullmac_dev, "wpa_versions=1, not support right now!\n");
			return -EPERM;
		} else {
			softAP_config.security_type = RTW_SECURITY_WEP_PSK;
			dev_err(global_idev.fullmac_dev, "ERR: AP in WEP security mode is not supported!!");
			return -EPERM;
		}

		pwd_vir = dmam_alloc_coherent(global_idev.fullmac_dev, strlen(fake_pwd), &pwd_phy, GFP_KERNEL);
		if (!pwd_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			return -ENOMEM;
		}
		/* If not fake, copy from upper layer, like WEP(unsupported). */
		memcpy(pwd_vir, fake_pwd, strlen(fake_pwd));
		softAP_config.password = (unsigned char *)pwd_phy;
		softAP_config.password_len = strlen(fake_pwd);
		//dev_dbg(global_idev.fullmac_dev, "security_type=0x%x, password=%s, len=%d \n", softAP_config.security_type, softAP_config.password, softAP_config.password_len);
	} else {
		softAP_config.security_type = RTW_SECURITY_OPEN;
	}

	ret = llhw_ipc_wifi_start_ap(&softAP_config);

	netif_carrier_on(ndev);

	if (pwd_vir) {
		dma_free_coherent(global_idev.fullmac_dev, strlen(pwd_vir), pwd_vir, pwd_phy);
	}

	return ret;
}

static int cfg80211_rtw_stop_ap(struct wiphy *wiphy, struct net_device *ndev)
{
	int ret = 0;
	dev_dbg(global_idev.fullmac_dev, "=>"FUNC_NDEV_FMT" - Stop Softap\n", FUNC_NDEV_ARG(ndev));

	ret = llhw_ipc_wifi_stop_ap();

	netif_carrier_off(ndev);
	return ret;
}

void cfg80211_rtw_ops_ap_init(void)
{
	struct cfg80211_ops *ops = &global_idev.rtw_cfg80211_ops;

	ops->channel_switch = cfg80211_rtw_channel_switch;
	ops->add_virtual_intf = cfg80211_rtw_add_virtual_intf;
	ops->del_virtual_intf = cfg80211_rtw_del_virtual_intf;
	ops->start_ap = cfg80211_rtw_start_ap;
	ops->change_beacon = cfg80211_rtw_change_beacon;
	ops->stop_ap = cfg80211_rtw_stop_ap;
#if defined(CONFIG_RTW_MACADDR_ACL) && (CONFIG_RTW_MACADDR_ACL == 1)
	ops->set_mac_acl = cfg80211_rtw_set_mac_acl;
#endif
	ops->add_station = cfg80211_rtw_add_station;
	ops->del_station = cfg80211_rtw_del_station;
	ops->change_station = cfg80211_rtw_change_station;
	ops->dump_station = cfg80211_rtw_dump_station;
	ops->change_bss = cfg80211_rtw_change_bss;
	ops->set_txq_params = cfg80211_rtw_set_txq_params;
#ifdef CONFIG_CFG80211_SME_OFFLOAD
	/*
	 * This is required by AP SAE, otherwise wpa_driver_nl80211_capa() would
	 * set use_monitor to 1 because poll_command_supported is false and
	 * hostap::nl80211_setup_ap would not call nl80211_mgmt_subscribe_ap()
	 * (which SAE AP shall use).
	 */
	ops->probe_client = cfg80211_rtw_probe_client;
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */
}
