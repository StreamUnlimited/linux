/******************************************************************************
 * Copyright (c) 2007-2022 Realtek Semiconductor Corp. All Rights Reserved.
 *
 * This program is dual-licensed under both the GPL version 2 and BSD
 * license. Either license may be used at your option.
 *
 *
 * License
 *
 *
 * GPL v2:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *
 * Alternatively, this software may be distributed, used, and modified
 * under the terms of BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#define  _IOCTL_CFG80211_C_

#include <rtw_drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>
#include <rtw_netdev_intf.h>
#include <uapi/inic/inic_wireless.h>

#define RTW_SSID_SCAN_AMOUNT 1 //LINUX_TODO
#define RTW_CHANNEL_SCAN_AMOUNT (14+37)

extern int inic_ipc_wifi_scan(rtw_scan_param_t *scan_param, u32 ssid_len, u32 block);

static int cfg80211_rtw_add_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
								, int link_id
#endif
								, u8 key_index
								, bool pairwise
								, const u8 *mac_addr, struct key_params *params)
{
	return 0;
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
	return 0;
}

static int cfg80211_rtw_del_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
								, int link_id
#endif
								, u8 key_index, bool pairwise, const u8 *mac_addr)

{
	return 0;
}

static int cfg80211_rtw_set_default_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
										, int link_id
#endif
										, u8 key_index
										, bool unicast, bool multicast
									   )
{
	return 0;
}

int cfg80211_rtw_set_default_mgmt_key(struct wiphy *wiphy, struct net_device *ndev
#ifdef CONFIG_MLD_KERNEL_PATCH
									  , int link_id
#endif
									  , u8 key_index)
{
	return 0;
}


#if defined(CONFIG_GTK_OL)
static int cfg80211_rtw_set_rekey_data(struct wiphy *wiphy,
									   struct net_device *ndev,
									   struct cfg80211_gtk_rekey_data *data)
{
	return 0;
}
#endif /*CONFIG_GTK_OL*/


static int cfg80211_rtw_get_station(struct wiphy *wiphy,
									struct net_device *ndev,
									const u8 *mac,
									struct station_info *sinfo)
{
	return 0;
}

static int cfg80211_rtw_change_iface(struct wiphy *wiphy,
									 struct net_device *ndev,
									 enum nl80211_iftype type,
									 struct vif_params *params)
{
	return 0;
}

static int cfg80211_rtw_scan(struct wiphy *wiphy
							 , struct cfg80211_scan_request *request)
{
	int i;
	u8 _status = _FALSE;
	int ret = 0;
	struct cfg80211_ssid *ssids = request->ssids;
	bool need_indicate_scan_done = _FALSE;
	struct wireless_dev *wdev;
	u32 wlan_idx = 3;
	struct net_device *pnetdev = NULL;
	rtw_scan_param_t scan_param = {0};
	u8 *channel_list = NULL;
	u32 ssid_len = 0;

	wdev = request->wdev;
	pnetdev = wdev_to_ndev(wdev);
	if (pnetdev) {
		wlan_idx = rtw_netdev_idx(pnetdev);
	} else {
		ret = -EINVAL;
		goto exit;
	}

	_rtw_memset(&scan_param, 0, sizeof(rtw_scan_param_t));

	/* parsing request ssids, n_ssids */ //LINUX_TODO: do we need to support multi ssid scan?
	for (i = 0; i < request->n_ssids && ssids && i < RTW_SSID_SCAN_AMOUNT; i++) {
		if (ssids[i].ssid_len) {
			scan_param.ssid = _rtw_malloc(ssids[0].ssid_len);
			_rtw_memcpy(scan_param.ssid, ssids[0].ssid, ssids[0].ssid_len);
			ssid_len = ssids[0].ssid_len;
		}
	}

	channel_list = (u8 *)_rtw_malloc(request->n_channels);
	for (i = 0; i < request->n_channels && i < RTW_CHANNEL_SCAN_AMOUNT; i++) {
		*(channel_list + i) = request->channels[i]->hw_value;
		//LINUX_TODO: process request->channels[i]->flags
	}
	scan_param.channel_list_num = i;
	scan_param.channel_list = channel_list;

	ret = inic_ipc_wifi_scan(&scan_param, 0, ssid_len);
	if (ret < 0) {
		//_rtw_cfg80211_surveydone_event_callback(padapter, request);
		cfg80211_scan_done(request, 0);
	}

exit:

	return ret;
}


static void cfg80211_rtw_abort_scan(struct wiphy *wiphy,
									struct wireless_dev *wdev)
{
}

static int cfg80211_rtw_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	return 0;
}


static int cfg80211_rtw_join_ibss(struct wiphy *wiphy, struct net_device *ndev,
								  struct cfg80211_ibss_params *params)
{
	return 0;
}

static int cfg80211_rtw_leave_ibss(struct wiphy *wiphy, struct net_device *ndev)
{
	return 0;
}

static int cfg80211_rtw_connect(struct wiphy *wiphy, struct net_device *ndev,
								struct cfg80211_connect_params *sme)
{
	return 0;
}

static int cfg80211_rtw_disconnect(struct wiphy *wiphy, struct net_device *ndev,
								   u16 reason_code)
{
	return 0;
}

static int cfg80211_rtw_set_txpower(struct wiphy *wiphy,
									struct wireless_dev *wdev,
									enum nl80211_tx_power_setting type, int mbm)
{

	return 0;
}

static int cfg80211_rtw_get_txpower(struct wiphy *wiphy,
									struct wireless_dev *wdev,
									int *dbm)
{
	return 0;
}


static int cfg80211_rtw_set_power_mgmt(struct wiphy *wiphy,
									   struct net_device *ndev,
									   bool enabled, int timeout)
{
	return 0;
}

static int cfg80211_rtw_set_pmksa(struct wiphy *wiphy,
								  struct net_device *ndev,
								  struct cfg80211_pmksa *pmksa)
{
	return 0;
}

static int cfg80211_rtw_del_pmksa(struct wiphy *wiphy,
								  struct net_device *ndev,
								  struct cfg80211_pmksa *pmksa)
{
	return 0;
}

static int cfg80211_rtw_flush_pmksa(struct wiphy *wiphy,
									struct net_device *ndev)
{
	return 0;
}

#ifdef CONFIG_AP_MODE


static struct wireless_dev *
cfg80211_rtw_add_virtual_intf(
	struct wiphy *wiphy,
	const char *name,
	unsigned char name_assign_type,
	enum nl80211_iftype type,
	struct vif_params *params)
{
	return 0;
}

static int cfg80211_rtw_del_virtual_intf(struct wiphy *wiphy,
		struct wireless_dev *wdev
										)
{
	return 0;
}

#if CONFIG_RTW_MACADDR_ACL
static int cfg80211_rtw_set_mac_acl(struct wiphy *wiphy, struct net_device *ndev,
									const struct cfg80211_acl_data *params)
{
	return 0;
}
#endif /* CONFIG_RTW_MACADDR_ACL && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)) */


static int	cfg80211_rtw_add_station(struct wiphy *wiphy, struct net_device *ndev,
									 const u8 *mac,
									 struct station_parameters *params)
{
	return 0;
}

static int	cfg80211_rtw_del_station(struct wiphy *wiphy, struct net_device *ndev,
									 struct station_del_parameters *params
								   )
{
	return 0;
}

static int cfg80211_rtw_change_station(struct wiphy *wiphy,
									   struct net_device *ndev,
									   const u8 *mac,
									   struct station_parameters *params)
{
	return 0;
}

static int	cfg80211_rtw_dump_station(struct wiphy *wiphy, struct net_device *ndev,
									  int idx, u8 *mac, struct station_info *sinfo)
{
	return 0;
}

static int	cfg80211_rtw_change_bss(struct wiphy *wiphy, struct net_device *ndev,
									struct bss_parameters *params)
{
	return 0;

}

static int	cfg80211_rtw_set_txq_params(struct wiphy *wiphy
										, struct net_device *ndev
										, struct ieee80211_txq_params *params)
{
	return 0;
}

static int cfg80211_rtw_set_monitor_channel(struct wiphy *wiphy
		, struct cfg80211_chan_def *chandef)
{
	return 0;
}

static int cfg80211_rtw_get_channel(struct wiphy *wiphy,
									struct wireless_dev *wdev,
#if (defined(CONFIG_MLD_KERNEL_PATCH))
									unsigned int link_id,
#endif
									struct cfg80211_chan_def *chandef)
{
	printk("****get channel return 1\n");
	return 1;
}

#ifdef CONFIG_CFG80211_SME_OFFLOAD
static int cfg80211_rtw_probe_client(struct wiphy *wiphy,
									 struct net_device *ndev, const u8 *peer,
									 u64 *cookie)
{
	return 0;
}

#endif	/* CONFIG_CFG80211_SME_OFFLOAD */
#endif


#ifdef CONFIG_CFG80211_SME_OFFLOAD
static int	cfg80211_rtw_auth(struct wiphy *wiphy, struct net_device *ndev,
							  struct cfg80211_auth_request *req)
{
	return 0;
}

static int	cfg80211_rtw_assoc(struct wiphy *wiphy, struct net_device *ndev,
							   struct cfg80211_assoc_request *req)
{
	return 0;
}

static int cfg80211_rtw_deauth(struct wiphy *wiphy, struct net_device *ndev,
							   struct cfg80211_deauth_request *req)
{
	return 0;
}

static int	cfg80211_rtw_disassoc(struct wiphy *wiphy, struct net_device *ndev,
								  struct cfg80211_disassoc_request *req)
{
	return 0;
}
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */

static s32 cfg80211_rtw_remain_on_channel(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		struct ieee80211_channel *channel,
		unsigned int duration, u64 *cookie)
{
	return 0;
}

static s32 cfg80211_rtw_cancel_remain_on_channel(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		u64 cookie)
{
	return 0;
}

static int cfg80211_rtw_mgmt_tx(struct wiphy *wiphy,
								struct wireless_dev *wdev,
								struct cfg80211_mgmt_tx_params *params,
								u64 *cookie)
{
	return 0;
}

static void cfg80211_rtw_mgmt_frame_register(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		u16 frame_type, bool reg)
{

}

#ifdef CONFIG_AP_MODE
static int cfg80211_rtw_channel_switch(struct wiphy *wiphy,
									   struct net_device *dev,
									   struct cfg80211_csa_settings *params)
{
	return 0;
}


static int cfg80211_rtw_start_ap(struct wiphy *wiphy, struct net_device *ndev,
								 struct cfg80211_ap_settings *settings)
{
	return 0;
}

static int cfg80211_rtw_change_beacon(struct wiphy *wiphy, struct net_device *ndev,
									  struct cfg80211_beacon_data *info)
{
	return 0;
}

static int cfg80211_rtw_stop_ap(struct wiphy *wiphy, struct net_device *ndev)
{
	return 0;
}

#endif /* CONFIG_AP_MODE */

struct cfg80211_ops rtw_cfg80211_ops = {
	.change_virtual_intf = cfg80211_rtw_change_iface,
	.add_key = cfg80211_rtw_add_key,
	.get_key = cfg80211_rtw_get_key,
	.del_key = cfg80211_rtw_del_key,
	.set_default_key = cfg80211_rtw_set_default_key,
	.set_default_mgmt_key = cfg80211_rtw_set_default_mgmt_key,
#if defined(CONFIG_GTK_OL)
	.set_rekey_data = cfg80211_rtw_set_rekey_data,
#endif /*CONFIG_GTK_OL*/
	.get_station = cfg80211_rtw_get_station,
	.scan = cfg80211_rtw_scan,
	.abort_scan = cfg80211_rtw_abort_scan,
	.set_wiphy_params = cfg80211_rtw_set_wiphy_params,
	.connect = cfg80211_rtw_connect,
	.disconnect = cfg80211_rtw_disconnect,
	.join_ibss = cfg80211_rtw_join_ibss,
	.leave_ibss = cfg80211_rtw_leave_ibss,
	.set_tx_power = cfg80211_rtw_set_txpower,
	.get_tx_power = cfg80211_rtw_get_txpower,
	.set_power_mgmt = cfg80211_rtw_set_power_mgmt,
	.set_pmksa = cfg80211_rtw_set_pmksa,
	.del_pmksa = cfg80211_rtw_del_pmksa,
	.flush_pmksa = cfg80211_rtw_flush_pmksa,

#ifdef CONFIG_CFG80211_SME_OFFLOAD
	.auth = cfg80211_rtw_auth,
	.deauth = cfg80211_rtw_deauth,
	.assoc = cfg80211_rtw_assoc,
	.disassoc = cfg80211_rtw_disassoc,
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */

#ifdef CONFIG_AP_MODE
	.add_virtual_intf = cfg80211_rtw_add_virtual_intf,
	.del_virtual_intf = cfg80211_rtw_del_virtual_intf,
	.start_ap = cfg80211_rtw_start_ap,
	.change_beacon = cfg80211_rtw_change_beacon,
	.stop_ap = cfg80211_rtw_stop_ap,

#if CONFIG_RTW_MACADDR_ACL
	.set_mac_acl = cfg80211_rtw_set_mac_acl,
#endif

	.add_station = cfg80211_rtw_add_station,
	.del_station = cfg80211_rtw_del_station,
	.change_station = cfg80211_rtw_change_station,
	.dump_station = cfg80211_rtw_dump_station,
	.change_bss = cfg80211_rtw_change_bss,
	.set_txq_params = cfg80211_rtw_set_txq_params,

#ifdef CONFIG_CFG80211_SME_OFFLOAD
	/*
	 * This is required by AP SAE, otherwise wpa_driver_nl80211_capa() would
	 * set use_monitor to 1 because poll_command_supported is false and
	 * hostap::nl80211_setup_ap would not call nl80211_mgmt_subscribe_ap()
	 * (which SAE AP shall use).
	 */
	.probe_client = cfg80211_rtw_probe_client,
#endif	/* CONFIG_CFG80211_SME_OFFLOAD */
	/* .auth = cfg80211_rtw_auth, */
	/* .assoc = cfg80211_rtw_assoc,	 */
#endif /* CONFIG_AP_MODE */

	.set_monitor_channel = cfg80211_rtw_set_monitor_channel,
	.get_channel = cfg80211_rtw_get_channel,

	.remain_on_channel = cfg80211_rtw_remain_on_channel,
	.cancel_remain_on_channel = cfg80211_rtw_cancel_remain_on_channel,

#ifdef CONFIG_RTW_80211R
	.update_ft_ies = cfg80211_rtw_update_ft_ies,
#endif

	.mgmt_tx = cfg80211_rtw_mgmt_tx,
	.mgmt_frame_register = cfg80211_rtw_mgmt_frame_register,

#ifdef CONFIG_AP_MODE
	.channel_switch = cfg80211_rtw_channel_switch,
#endif /* #ifdef CONFIG_AP_MODE */
};

