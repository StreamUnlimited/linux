#include <rtw_cfg80211_fullmac.h>

#define DRIVERVERSION	"v1.15.12-27-g7f6d5a49a.20220627"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

static int rtw_ndev_set_mac_address(struct net_device *pnetdev, void *addr)
{
	dev_dbg(global_idev.fullmac_dev, "[netdev]: %s", __func__);
	return 0;
}

static struct net_device_stats *rtw_ndev_get_stats(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[netdev]: %s", __func__);
	return &global_idev.stats[rtw_netdev_idx(pnetdev)];
}

/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263) {
		return skb->priority - 256;
	}

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

u8 qos_acm(u8 acm_mask, u8 priority)
{
	u8 change_priority = priority;

	switch (priority) {
	case 0:
	case 3:
		if (acm_mask & BIT(1)) {
			change_priority = 1;
		}
		break;
	case 1:
	case 2:
		break;
	case 4:
	case 5:
		if (acm_mask & BIT(2)) {
			change_priority = 0;
		}
		break;
	case 6:
	case 7:
		if (acm_mask & BIT(3)) {
			change_priority = 5;
		}
		break;
	default:
		dev_warn(global_idev.fullmac_dev, "qos_acm(): invalid pattrib->priority: %d!!!\n", priority);
		break;
	}

	return change_priority;
}

/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static u16 rtw_ndev_select_queue(struct net_device *pnetdev, struct sk_buff *skb, struct net_device *sb_dev)
{
	int acm_mask = 0;
	u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

	skb->priority = rtw_classify8021d(skb);

	/* acm_mask != 0 is qos packet. tmp. */
	if (acm_mask != 0) {
		skb->priority = qos_acm(acm_mask, skb->priority);
	}

	return rtw_1d_to_queue[skb->priority];
}

int rtw_ndev_init(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));
	return 0;
}

void rtw_ndev_uninit(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));
}

int rtw_ndev_open(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));

	rtw_netdev_priv_is_on(pnetdev) = true;
	netif_tx_start_all_queues(pnetdev);
	netif_tx_wake_all_queues(pnetdev);
	return 0;
}

static int rtw_ndev_close(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));
	netif_tx_stop_all_queues(pnetdev);
	netif_carrier_off(pnetdev);
	rtw_netdev_priv_is_on(pnetdev) = false;
	return 0;
}

int rtw_ndev_open_ap(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));

	/* if2 init(SW + part of HW) */
	llhw_ipc_wifi_init_ap();

	netif_tx_start_all_queues(pnetdev);
	netif_tx_wake_all_queues(pnetdev);
	return 0;
}

static int rtw_ndev_close_ap(struct net_device *pnetdev)
{
	dev_dbg(global_idev.fullmac_dev, "[fullmac]: %s %d\n", __func__, rtw_netdev_idx(pnetdev));
	netif_tx_stop_all_queues(pnetdev);
	netif_carrier_off(pnetdev);

	/* if2 deinit (SW) */
	llhw_ipc_wifi_deinit_ap();
	return 0;
}

static enum netdev_tx rtw_xmit_entry(struct sk_buff *skb, struct net_device *pnetdev)
{
	int ret = 0;
	int wlan_idx = 0;

	if (pnetdev) {
		wlan_idx = rtw_netdev_idx(pnetdev);
	} else {
		ret = -EINVAL;
		goto func_exit;
	}

	if (llhw_ipc_xmit_entry(wlan_idx, skb) == 0) {
		ret = NETDEV_TX_OK;
	} else {
		ret = NETDEV_TX_BUSY;
	}

func_exit:
	return ret;
}

static const struct net_device_ops rtw_ndev_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = rtw_ndev_open,
	.ndo_stop = rtw_ndev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_select_queue = rtw_ndev_select_queue,
	.ndo_set_mac_address = rtw_ndev_set_mac_address,
	.ndo_get_stats = rtw_ndev_get_stats,
	.ndo_do_ioctl = NULL,
};

static const struct net_device_ops rtw_ndev_ops_ap = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = rtw_ndev_open_ap,
	.ndo_stop = rtw_ndev_close_ap,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_select_queue = rtw_ndev_select_queue,
	.ndo_set_mac_address = rtw_ndev_set_mac_address,
	.ndo_get_stats = rtw_ndev_get_stats,
	.ndo_do_ioctl = NULL,
};

int rtw_ndev_register(void)
{
	int i, ret = false;
	/* TODO: Customer names, get them from dts instead. */
	char *wlan_name[TOTAL_IFACE_NUM] = {"wlan0", "wlan1"};
	struct net_device *ndev = NULL;
	struct wireless_dev *wdev;

	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		/* alloc and init netdev */
		ndev = alloc_etherdev_mq(sizeof(struct netdev_priv_t), 4);
		if (!ndev) {
			goto dev_fail;
		}
		global_idev.pndev[i] = ndev;
		rtw_netdev_idx(ndev) = i;
		ndev->netdev_ops = (i ? &rtw_ndev_ops_ap : &rtw_ndev_ops);
		ndev->watchdog_timeo = HZ * 3; /* 3 second timeout */
		SET_NETDEV_DEV(ndev, global_idev.fullmac_dev);

		/* alloc and init wireless_dev */
		wdev = (struct wireless_dev *)kzalloc(sizeof(struct wireless_dev), in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
		if (!wdev) {
			goto dev_fail;
		}
		wdev->wiphy = global_idev.pwiphy_global;
		wdev->netdev = ndev;
		/* TODO: iftype. */
		wdev->iftype = (i ? NL80211_IFTYPE_AP : NL80211_IFTYPE_STATION);
		ndev->ieee80211_ptr = wdev;
		global_idev.pwdev_global[i] = wdev;

		rtw_ethtool_ops_init();
		netdev_set_default_ethtool_ops(global_idev.pndev[i], &global_idev.rtw_ethtool_ops);
		if (dev_alloc_name(global_idev.pndev[i], wlan_name[i]) < 0) {
			dev_err(global_idev.fullmac_dev, "dev_alloc_name, fail!\n");
		}
		netif_carrier_off(global_idev.pndev[i]);
		ret = (register_netdev(global_idev.pndev[i]) == 0) ? true : false;
		if (ret != true) {
			dev_err(global_idev.fullmac_dev, "netdevice register fail!\n");
			goto dev_fail;
		}
	}

	return ret;

dev_fail:
	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		if (global_idev.pwdev_global[i]) { //wdev
			kfree((u8 *)global_idev.pwdev_global[i]);
			global_idev.pwdev_global[i] = NULL;
		}

		if (global_idev.pndev[i]) {
			free_netdev(global_idev.pndev[i]);
			global_idev.pndev[i] = NULL;
		}
	}
	return -ENODEV;

}

void rtw_ndev_unregister(void)
{
	int i;

	for (i = 0; i < TOTAL_IFACE_NUM; i++) {
		if (global_idev.pndev[i]) {
			/* hold rtnl_lock in unregister_netdev. */
			unregister_netdev(global_idev.pndev[i]);
		}
		dev_dbg(global_idev.fullmac_dev, "remove netdev done for interface %d.", i);

		if (global_idev.pwdev_global[i]) { //wdev
			kfree((u8 *)global_idev.pwdev_global[i]);
			global_idev.pwdev_global[i] = NULL;
			/* remove wireless_dev in ndev. */
			global_idev.pndev[i]->ieee80211_ptr = NULL;
		}

		if (global_idev.pndev[i]) {
			free_netdev(global_idev.pndev[i]);
			global_idev.pndev[i] = NULL;
		}
		dev_dbg(global_idev.fullmac_dev, "free netdev %d ok.", i);
	}
}
