#ifndef __RTW_NETDEV_OPS_H__
#define __RTW_NETDEV_OPS_H__

#define WIFI_MP_MSG_BUF_SIZE (4096)

struct netdev_priv_t {
	u32				wlan_idx;
	bool				priv_is_on;
};

#define rtw_netdev_priv_is_on(netdev)	(((struct netdev_priv_t *)netdev_priv(netdev))->priv_is_on)
#define rtw_netdev_idx(netdev)		(((struct netdev_priv_t *)netdev_priv(netdev))->wlan_idx)
#define wdev_to_ndev(w)			((w)->netdev)
#define ndev_to_wdev(n) 		((n)->ieee80211_ptr)

#endif //__RTW_NETDEV_OPS_H__