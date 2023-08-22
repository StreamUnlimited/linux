#include <rtw_cfg80211_fullmac.h>

/* Cipher suite type: params->cipher in add key. */
/*
 * OUI		Suite type	Meaning
 * 00-0F-AC	0		Use group cipher suite
 * 00-0F-AC	1		WEP-40
 * 00-0F-AC	2		TKIP
 * 00-0F-AC	3		Reserved
 * 00-0F-AC	4		CCMP – default in an RSNA
 * 00-0F-AC	5		WEP-104
 * 00-0F-AC	6–255		Reserved
 * Vendor OUI	Other		Vendor specific
*/

/* AKM suites: crypto.akm_suites while connect. */
/* OUI		Suite type	Authentication type					Key management type
 * 00-0F-AC	0		Reserved						Reserved
 * 00-0F-AC	1		IEEE 802.1X or using PMKSA caching – RSNA default	RSNA key management or using PMKSA caching – RSNA default
 * 00-0F-AC	2		PSK							RSNA key management, using PSK
 * 00-0F-AC	3–255		Reserved						Reserved
 * Vendor OUI	Any		Vendor specific						Vendor specific
*/

static int cfg80211_rtw_add_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
								, int link_id
#endif
								, u8 key_index
								, bool pairwise
								, const u8 *mac_addr, struct key_params *params)
{
	struct rtw_crypt_info crypt;
	int ret = 0;

	dev_dbg(global_idev.fullmac_dev, "--- %s ---", __func__);
	memset(&crypt, 0, sizeof(struct rtw_crypt_info));
	if (ndev) {
		crypt.wlan_idx = rtw_netdev_idx(ndev);
	} else {
		ret = -EINVAL;
		goto exit;
	}

	dev_dbg(global_idev.fullmac_dev, "[fullmac]: netdev = %d", crypt.wlan_idx);
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: key_index = %d", key_index);
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: pairwise= %d", pairwise);
	if (pairwise) {
		dev_dbg(global_idev.fullmac_dev, "[fullmac]: mac addr = %x %x %x %x %x %x", *mac_addr, *(mac_addr + 1), *(mac_addr + 2), *(mac_addr + 3), *(mac_addr + 4),
				*(mac_addr + 5));
	}
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: key material. key len = %d.\n", params->key_len);
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: cipher suite = %08x", params->cipher);
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: key mode = %d", params->mode);

	if (((params->cipher & 0xff) == 1) || ((params->cipher & 0xff) == 5)) {
		/* Set WEP key by rtos. */
		dev_dbg(global_idev.fullmac_dev, "--- %s --- return: set key by rtos self. ", __func__);
		return 0;
	}

	if (params->key_len && params->key) {
		crypt.key_len = params->key_len;
		memcpy(crypt.key, (u8 *)params->key, params->key_len);
	}
	crypt.pairwise = pairwise;
	crypt.key_idx = key_index;
	crypt.cipher = params->cipher;
	if (pairwise && mac_addr) {
		memcpy(crypt.mac_addr, mac_addr, 6);
	}

	ret = llhw_ipc_wifi_add_key(&crypt);
exit:

	return ret;
}

static int cfg80211_rtw_get_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
								, int link_id
#endif
								, u8 keyid
								, bool pairwise
								, const u8 *mac_addr, void *cookie
								, void (*callback)(void *cookie, struct key_params *))
{
	dev_dbg(global_idev.fullmac_dev, "--- %s --- !!!!!!!!!!", __func__);
	return 0;
}

static int cfg80211_rtw_del_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
								, int link_id
#endif
								, u8 key_index, bool pairwise, const u8 *mac_addr)

{
	dev_dbg(global_idev.fullmac_dev, "--- %s --- !!!!!!!!!!!!", __func__);
	return 0;
}

static int cfg80211_rtw_set_default_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
										, int link_id
#endif
										, u8 key_index, bool unicast, bool multicast)
{
	dev_dbg(global_idev.fullmac_dev, "--- %s ---", __func__);
	return 0;
}

static int cfg80211_rtw_set_default_mgmt_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
		, int link_id
#endif
		, u8 key_index)
{
	dev_dbg(global_idev.fullmac_dev, "--- %s ---", __func__);
	return 0;
}

#if defined(CONFIG_GTK_OL)
static int cfg80211_rtw_set_rekey_data(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_gtk_rekey_data *data)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s", __func__);
	return 0;
}
#endif /*CONFIG_GTK_OL*/

static int cfg80211_rtw_set_pmksa(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_pmksa *pmksa)
{
	dev_dbg(global_idev.fullmac_dev, "--- %s ---", __func__);
	return 0;
}

static int cfg80211_rtw_del_pmksa(struct wiphy *wiphy, struct net_device *ndev, struct cfg80211_pmksa *pmksa)
{
	dev_dbg(global_idev.fullmac_dev, "--- %s ---", __func__);
	return 0;
}

static int cfg80211_rtw_flush_pmksa(struct wiphy *wiphy, struct net_device *ndev)
{
	dev_dbg(global_idev.fullmac_dev, "--- %s ---!!!! ", __func__);
	return 0;
}

void cfg80211_rtw_ops_key_init(void)
{
	struct cfg80211_ops *ops = &global_idev.rtw_cfg80211_ops;

	ops->add_key = cfg80211_rtw_add_key;
	ops->get_key = cfg80211_rtw_get_key;
	ops->del_key = cfg80211_rtw_del_key;
	ops->set_default_key = cfg80211_rtw_set_default_key;
	ops->set_default_mgmt_key = cfg80211_rtw_set_default_mgmt_key;
	ops->set_pmksa = cfg80211_rtw_set_pmksa;
	ops->del_pmksa = cfg80211_rtw_del_pmksa;
	ops->flush_pmksa = cfg80211_rtw_flush_pmksa;
#if defined(CONFIG_GTK_OL)
	ops->set_rekey_data = cfg80211_rtw_set_rekey_data;
#endif /*CONFIG_GTK_OL*/
}