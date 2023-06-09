/******************************************************************************
 *
 * Copyright(c) 2007 - 2021 Realtek Corporation.
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
#define _OS_INTFS_C_

#include <rtw_drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>
#include <rtw_xmit.h>
#include <rtw_netdev_intf.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DRIVERVERSION	"v1.15.12-27-g7f6d5a49a.20220627"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

//LINUX_TODO
#define RTW_SSID_SCAN_AMOUNT 1 /* for WEXT_CSCAN_AMOUNT 9 */
#define RTW_SCAN_IE_LEN_MAX      2304
#define RTW_MAX_NUM_PMKIDS 4
#define	MAX_CHANNEL_NUM_2G		14
#define	MAX_CHANNEL_NUM_5G		28
#define RTW_G_RATES_NUM	12
#define RTW_A_RATES_NUM	8

#define WIFI_CIPHER_SUITE_GCMP		0x000FAC08
#define WIFI_CIPHER_SUITE_GCMP_256	0x000FAC09
#define WIFI_CIPHER_SUITE_CCMP_256	0x000FAC0A
#define WIFI_CIPHER_SUITE_BIP_GMAC_128	0x000FAC0B
#define WIFI_CIPHER_SUITE_BIP_GMAC_256	0x000FAC0C
#define WIFI_CIPHER_SUITE_BIP_CMAC_256	0x000FAC0D

static const u32 rtw_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef CONFIG_IEEE80211W
	WLAN_CIPHER_SUITE_AES_CMAC,
	WIFI_CIPHER_SUITE_GCMP,
	WIFI_CIPHER_SUITE_GCMP_256,
	WIFI_CIPHER_SUITE_CCMP_256,
	WIFI_CIPHER_SUITE_BIP_GMAC_128,
	WIFI_CIPHER_SUITE_BIP_GMAC_256,
	WIFI_CIPHER_SUITE_BIP_CMAC_256,
#endif /* CONFIG_IEEE80211W */
};

#define CHAN2G(_channel, _freq, _flags) {			\
		.band			= NL80211_BAND_2GHZ,		\
		.center_freq		= (_freq),			\
		.hw_value		= (_channel),			\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}

#define CHAN5G(_channel, _flags) {				\
		.band			= NL80211_BAND_5GHZ,		\
		.center_freq		= 5000 + (5 * (_channel)),	\
		.hw_value		= (_channel),			\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}

static const struct ieee80211_channel rtw_2ghz_channels[MAX_CHANNEL_NUM_2G] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static const struct ieee80211_channel rtw_5ghz_a_channels[MAX_CHANNEL_NUM_5G] = {
	CHAN5G(36, 0),	CHAN5G(40, 0),	CHAN5G(44, 0),	CHAN5G(48, 0),

	CHAN5G(52, 0),	CHAN5G(56, 0),	CHAN5G(60, 0),	CHAN5G(64, 0),

	CHAN5G(100, 0),	CHAN5G(104, 0),	CHAN5G(108, 0),	CHAN5G(112, 0),
	CHAN5G(116, 0),	CHAN5G(120, 0),	CHAN5G(124, 0),	CHAN5G(128, 0),
	CHAN5G(132, 0),	CHAN5G(136, 0),	CHAN5G(140, 0),	CHAN5G(144, 0),

	CHAN5G(149, 0),	CHAN5G(153, 0),	CHAN5G(157, 0),	CHAN5G(161, 0),
	CHAN5G(165, 0),	CHAN5G(169, 0),	CHAN5G(173, 0),	CHAN5G(177, 0),
};

#define RATETAB_ENT(_rate, _rateid, _flags) \
	{								\
		.bitrate	= (_rate),				\
		.hw_value	= (_rateid),				\
		.flags		= (_flags),				\
	}

static const struct ieee80211_rate rtw_rates[] = {
	RATETAB_ENT(10,  0x1,   0),
	RATETAB_ENT(20,  0x2,   0),
	RATETAB_ENT(55,  0x4,   0),
	RATETAB_ENT(110, 0x8,   0),
	RATETAB_ENT(60,  0x10,  0),
	RATETAB_ENT(90,  0x20,  0),
	RATETAB_ENT(120, 0x40,  0),
	RATETAB_ENT(180, 0x80,  0),
	RATETAB_ENT(240, 0x100, 0),
	RATETAB_ENT(360, 0x200, 0),
	RATETAB_ENT(480, 0x400, 0),
	RATETAB_ENT(540, 0x800, 0),
};

#define rtw_a_rates		(rtw_rates + 4)
#define rtw_g_rates		(rtw_rates + 0)

struct wireless_dev *pwdev_global[TOTAL_IFACE_NUM] = {0};
struct net_device	*pnetdev_global[2] = {0};

int wdev_num = 0;
int netdev_num = 0;
int iface_nums_global = TOTAL_IFACE_NUM;
int current_iface_num = 0;
struct wiphy *pwiphy_global = NULL;
struct device *dev_global = NULL;

extern u8 mac_addr_from_adapter[6];
extern struct cfg80211_ops rtw_cfg80211_ops;

extern int inic_host_init(void);

/**
 * rtw_net_set_mac_address
 * This callback function is used for the Media Access Control address
 * of each net_device needs to be changed.
 *
 * Arguments:
 * @pnetdev: net_device pointer.
 * @addr: new MAC address.
 *
 * Return:
 * ret = 0: Permit to change net_device's MAC address.
 * ret = -1 (Default): Operation not permitted.
 *
 * Auther: Arvin Liu
 * Date: 2015/05/29
 */
static int rtw_ndev_set_mac_address(struct net_device *pnetdev, void *addr)
{
	return 0;
}

static struct net_device_stats *rtw_ndev_get_stats(struct net_device *pnetdev)
{
	return &(pnetdev->stats);
}

static u16 rtw_ndev_select_queue(struct net_device *dev, struct sk_buff *skb, struct net_device *sb_dev)
{

	return 0;
}

int rtw_ndev_init(struct net_device *dev)
{
	return 0;
}

void rtw_ndev_uninit(struct net_device *dev)
{
}

int rtw_ndev_open(struct net_device *pnetdev)
{
	return 0;
}

static int rtw_ndev_close(struct net_device *pnetdev)
{
	return 0;
}

static const struct net_device_ops rtw_ndev_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = rtw_ndev_open,
	.ndo_stop = rtw_ndev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_select_queue	= rtw_ndev_select_queue,
	.ndo_set_mac_address = rtw_ndev_set_mac_address,
	.ndo_get_stats = rtw_ndev_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};

/* For ethtool +++ */
static void rtw_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
}

static int rtw_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	return 0;
}

static void rtw_ethtool_get_strings(struct net_device *dev, u32 sset, u8 *data)
{
}

static void rtw_ethtool_get_stats(struct net_device *dev,
								  struct ethtool_stats *stats,
								  u64 *data)
{
}

static const struct ethtool_ops rtw_ethtool_ops = {
	.get_drvinfo = rtw_ethtool_get_drvinfo,
	.get_link = ethtool_op_get_link,
	.get_strings = rtw_ethtool_get_strings,
	.get_ethtool_stats = rtw_ethtool_get_stats,
	.get_sset_count = rtw_ethtool_get_sset_count,
};

int rtw_wiphy_band_init(struct wiphy *pwiphy, u32 band_type)
{
	int n_channels, n_bitrates;
	struct ieee80211_supported_band *band = NULL;

	if (band_type == NL80211_BAND_2GHZ) {
		n_channels = MAX_CHANNEL_NUM_2G;
		n_bitrates = RTW_G_RATES_NUM;
	} else if (band_type == NL80211_BAND_5GHZ) {
		n_channels = MAX_CHANNEL_NUM_5G;
		n_bitrates = RTW_A_RATES_NUM;
	}

	band = (struct ieee80211_supported_band *)rtw_zmalloc(
			   sizeof(struct ieee80211_supported_band)
			   + sizeof(struct ieee80211_channel) * n_channels
			   + sizeof(struct ieee80211_rate) * n_bitrates
			   + sizeof(struct ieee80211_sband_iftype_data) * 2
		   );

	if (!band) {
		printk("init wiphy band failed\n");
		return _FAIL;
	}

	band->channels = (struct ieee80211_channel *)(((u8 *)band) + sizeof(struct ieee80211_supported_band));
	band->bitrates = (struct ieee80211_rate *)(((u8 *)band->channels) + sizeof(struct ieee80211_channel) * n_channels);
	band->band = NL80211_BAND_2GHZ;
	band->n_channels = n_channels;
	band->n_bitrates = n_bitrates;
	band->iftype_data = (struct ieee80211_sband_iftype_data *)(((u8 *)band->bitrates)
						+ sizeof(struct ieee80211_rate) * n_bitrates);
	band->n_iftype_data = 0;

	if (band_type == NL80211_BAND_2GHZ) {
		_rtw_memcpy((void *)band->channels, (void *)rtw_2ghz_channels, sizeof(rtw_2ghz_channels));
		_rtw_memcpy(band->bitrates, rtw_g_rates, sizeof(struct ieee80211_rate) * RTW_G_RATES_NUM);
	} else if (band_type == NL80211_BAND_5GHZ) {
		_rtw_memcpy((void *)band->channels, (void *)rtw_5ghz_a_channels, sizeof(rtw_5ghz_a_channels));
		_rtw_memcpy(band->bitrates, rtw_a_rates, sizeof(struct ieee80211_rate) * RTW_A_RATES_NUM);
	}

	pwiphy->bands[band_type] = band;
	return _SUCCESS;

}

int rtw_wiphy_init(struct wiphy *pwiphy)
{
	int ret = _SUCCESS;

	//LINUX_TODO: set default value for other wiphy parameters
	pwiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	pwiphy->max_scan_ssids = RTW_SSID_SCAN_AMOUNT;
	pwiphy->max_scan_ie_len = RTW_SCAN_IE_LEN_MAX;
	pwiphy->max_num_pmkids = RTW_MAX_NUM_PMKIDS;
	pwiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	pwiphy->cipher_suites = rtw_cipher_suites;
	pwiphy->n_cipher_suites = sizeof(rtw_cipher_suites) / sizeof((rtw_cipher_suites)[0]);

	ret = rtw_wiphy_band_init(pwiphy, NL80211_BAND_2GHZ);
	if (ret == _FAIL) {
		return ret;
	}
	ret = rtw_wiphy_band_init(pwiphy, NL80211_BAND_5GHZ);
	if (ret == _FAIL) {
		return ret;
	}

	//HT cap-2.4G
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.ht_supported = 1;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
			IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 | IEEE80211_HT_CAP_MAX_AMSDU;
	//pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap->cap |= IEEE80211_HT_CAP_RX_STBC_1R;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[0] = 0xFF;
	pwiphy->bands[NL80211_BAND_2GHZ]->ht_cap.mcs.rx_highest = 72;

	//HT cap-5G
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.ht_supported = 1;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
			IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 | IEEE80211_HT_CAP_MAX_AMSDU;
	//pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap->cap |= IEEE80211_HT_CAP_RX_STBC_1R;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_16K;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[0] = 0xFF;
	pwiphy->bands[NL80211_BAND_5GHZ]->ht_cap.mcs.rx_highest = 72;

	//todo: VHT and HE cap init
	return ret;
}

/**
 * rtw_os_ndevs_init - Allocate and register OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 *
 * Returns:
 * _SUCCESS or _FAIL
 */
int rtw_os_ndevs_init(void)
{
	int ret = _FAIL;
	int i;
	struct net_device *ndev = NULL;
	struct wireless_dev *wdev;
	char *name;
	struct ieee80211_supported_band *band;
	char *wlan_name[TOTAL_IFACE_NUM] = {"wlan0", "wlan1"};
	struct wiphy *pwiphy = NULL;

	printk("rtw_os_ndevs_init start\n");

	/*step1: alloc and init wiphy */
	pwiphy = wiphy_new(&rtw_cfg80211_ops, 0);
	if (!pwiphy) {
		printk("wiphy new failed\n");
		goto exit;
	}
	pwiphy_global = pwiphy;
	set_wiphy_dev(pwiphy, dev_global);
	/*init wiphy default values*/
	ret = rtw_wiphy_init(pwiphy);
	if (ret != _SUCCESS) {
		goto os_ndevs_free;
	}

	//LINUX_TODO: rtw_regd_init

	/* step2: alloc netdev and wireless device */
	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		/* alloc and init netdev */
		ndev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
		if (!ndev) {
			goto os_ndevs_free;
		}
		pnetdev_global[netdev_num++] = ndev;
		rtw_netdev_idx(ndev) = i;
		ndev->netdev_ops = &rtw_ndev_ops;
		ndev->watchdog_timeo = HZ * 3; /* 3 second timeout */
		SET_NETDEV_DEV(ndev, dev_global);

		/* alloc and init wireless_dev */
		wdev = (struct wireless_dev *)rtw_zmalloc(sizeof(struct wireless_dev));
		if (!wdev) {
			goto os_ndevs_free;
		}
		wdev->wiphy = pwiphy;
		wdev->netdev = ndev;
		wdev->iftype = NL80211_IFTYPE_STATION;
		ndev->ieee80211_ptr = wdev;
		pwdev_global[wdev_num++] = wdev;
	}

	/* tell NP to do wifi on and update mac addr*/
	inic_host_init();
	/* init wiphy's mac addr after get addr from NP*/
	_rtw_memcpy(pwiphy->perm_addr, pnetdev_global[0], ETH_ALEN);

	/*step3: register wiphy */
	if (wiphy_register(pwiphy) != 0) {
		goto os_ndevs_free;
	}
	//LINUX_TODO: chset_hook and cfgvendor attach
	//LINUX_TODO: rtw_get_chplan_cmd, rtw_regd_change_complete_sync

	/*step4: register netdev */
	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		netdev_set_default_ethtool_ops(pnetdev_global[i], &rtw_ethtool_ops);
		if (dev_alloc_name(pnetdev_global[i], wlan_name[i]) < 0) {
			printk("dev_alloc_name, fail!\n");
		}
		netif_carrier_off(pnetdev_global[i]);
		ret = (register_netdev(pnetdev_global[i]) == 0) ? _SUCCESS : _FAIL;

		if (ret != _SUCCESS) {
			//rtw_cfg80211_indicate_scan_done(adapter, _TRUE); //LINUX_TODO, need further check
			//rtw_cfg80211_indicate_disconnect(adapter, 0, 1); //LINUX_TODO, need further check
			printk("netdevice register fail!\n");
			break;
		}
	}

	/*failure process when netdev register fail */
	if (ret != _SUCCESS) {
		for (; i >= 0; i--) {
			if (pnetdev_global[i]) {
				unregister_netdevice(pnetdev_global[i]);
			}
		}
		//rtw_cfgvendor_detach(pwiphy);//LINUX_TODO, need further check
		wiphy_unregister(pwiphy);
		goto os_ndevs_free;
	}

	ret = _SUCCESS;

os_ndevs_free:
	if (ret != _SUCCESS) {
		for (i = 0; i < TOTAL_IFACE_NUM; i++) {
			if (pwdev_global[i]) { //wdev
				rtw_mfree((u8 *)pwdev_global[i], sizeof(struct wireless_dev));
				wdev_num--;
				pwdev_global[i] = NULL;
			}

			if (pnetdev_global[i]) {
				free_netdev(pnetdev_global[i]);
				pnetdev_global[i] = NULL;
				netdev_num--;
			}
		}

		if (pwiphy) {
			//rtw_regd_deinit(pwiphy);
			if (pwiphy->bands[NL80211_BAND_2GHZ]) {
				rtw_mfree(pwiphy->bands[NL80211_BAND_2GHZ], 0);
				pwiphy->bands[NL80211_BAND_2GHZ] = NULL;
			}
			if (pwiphy->bands[NL80211_BAND_5GHZ]) {
				rtw_mfree(pwiphy->bands[NL80211_BAND_5GHZ], 0);
				pwiphy->bands[NL80211_BAND_5GHZ] = NULL;
			}

			wiphy_free(pwiphy);
			pwiphy_global = NULL;
		}
	}
exit:
	return ret;
}

/**
 * rtw_os_ndevs_deinit - Unregister and free OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 */
void rtw_os_ndevs_deinit(void)
{
	int i;

	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		if (pnetdev_global[i]) {
			//rtw_cfg80211_indicate_scan_done(adapter, _TRUE); //LINUX_TODO
			//rtw_cfg80211_indicate_disconnect(adapter, 0, 1); //LINUX_TODO
			unregister_netdevice(pnetdev_global[i]);
		}
	}

	//rtw_cfgvendor_detach(dvobj_to_wiphy(dvobj));//LINUX_TODO, need further check
	wiphy_unregister(pwiphy_global);

	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		if (pwdev_global[i]) { //wdev
			rtw_mfree((u8 *)pwdev_global[i], sizeof(struct wireless_dev));
			wdev_num--;
			pwdev_global[i] = NULL;
		}

		if (pnetdev_global[i]) {
			free_netdev(pnetdev_global[i]);
			pnetdev_global[i] = NULL;
			netdev_num--;
		}
	}

	if (pwiphy_global) {
		//rtw_regd_deinit(pwiphy_global);
		//LINUX_TODO: free band of wiphy
		wiphy_free(pwiphy_global);
		pwiphy_global = NULL;

	}
}


