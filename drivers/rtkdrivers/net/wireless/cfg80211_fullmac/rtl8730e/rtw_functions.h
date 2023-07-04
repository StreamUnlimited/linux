#ifndef __RTW_FUNCTIONS_H__
#define __RTW_FUNCTIONS_H__

int rtw_wiphy_init(void);
void rtw_wiphy_deinit(void);
int rtw_wiphy_init_params(struct wiphy *pwiphy);
int rtw_wiphy_band_init(struct wiphy *pwiphy, u32 band_type);
void llhw_ipc_recv_task_from_msg(struct inic_ipc_ex_msg *p_ipc_msg);
int llhw_ipc_recv_init(struct inic_device *idev);
void llhw_ipc_xmit_deinit(void);
void llhw_ipc_event_task(unsigned long data);
int llhw_ipc_xmit_init(void);
void llhw_ipc_xmit_deinit(void);
void llhw_ipc_recv_task_from_msg_recv_pkts(int idx_wlan, struct dev_sk_buff *skb);
int llhw_ipc_xmit_entry(int idx, struct sk_buff *pskb);
void llhw_ipc_xmit_done(int idx_wlan);
int llhw_ipc_event_init(struct inic_device *idev);
void llhw_ipc_event_deinit(void);
int llhw_ipc_send_msg(u32 id, u32 *param_buf, u32 buf_len);
void llhw_ipc_wifi_on(void);
int llhw_ipc_wifi_scan(rtw_scan_param_t *scan_param, u32 ssid_len, u32 block);
int llhw_ipc_wifi_connect(rtw_network_info_t *connect_param, unsigned char block);
int llhw_ipc_wifi_disconnect(void);
int llhw_ipc_wifi_is_connected_to_ap(void);
int llhw_ipc_wifi_get_channel(u32 wlan_idx, u8 *ch);
int llhw_ipc_init(void);
int llhw_ipc_wifi_del_sta(u8 wlan_idx, u8* mac);
int llhw_ipc_wifi_init_ap(void);
int llhw_ipc_wifi_deinit_ap(void);
int llhw_ipc_wifi_start_ap(rtw_softap_info_t *softAP_config);
int llhw_ipc_wifi_stop_ap(void);
int llhw_ipc_wifi_add_key(struct rtw_crypt_info *crypt);
int llhw_ipc_wifi_set_EDCA_params(unsigned int *AC_param);
int llhw_ipc_wifi_get_chplan(u8 *chplan);
int llhw_ipc_wifi_tx_mgnt(u8 wlan_idx, const u8 *buf, size_t buf_len);
int llhw_ipc_wifi_sae_succ_start_assoc(void);
u32 llhw_ipc_wifi_update_ip_addr_in_wowlan(void);
int llhw_ipc_wifi_get_statistics(u32 statistic_phy);
int rtw_ndev_register(void);
void rtw_ndev_unregister(void);
void *rtw_get_ethtool_ops(void);
void llhw_ipc_deinit(void);
void cfg80211_rtw_ops_sta_init(void);
void cfg80211_rtw_ops_ap_init(void);
void cfg80211_rtw_ops_key_init(void);
void cfg80211_rtw_inform_bss(u32 channel, u32 frame_is_bcn, s32 rssi, u8 *mac_addr, u8 *IEs, u32 ie_len);
void cfg80211_rtw_disconnect_indicate(u16 reason, u8 locally_generated);
void cfg80211_rtw_sta_assoc_indicate(char *buf, int buf_len);
void cfg80211_rtw_external_auth_request(char *buf, int buf_len);
void cfg80211_rtw_connect_indicate(fullmac_join_status join_status, void *user_data, size_t user_data_len);
int cfg80211_rtw_scan_done_indicate(unsigned int scanned_AP_num, void *user_data);
int inic_ipc_msg_q_init(struct device *pdev, void (*task_hdl)(struct inic_ipc_ex_msg *));
int inic_ipc_msg_enqueue(struct inic_ipc_ex_msg *p_ipc_msg);
void inic_ipc_msg_q_deinit(void);
void llhw_ipc_send_packet(struct inic_ipc_ex_msg *p_ipc_msg);
int rtw_regd_init(void);
void rtw_ethtool_ops_init(void);

#endif // __RTW_FUNCTIONS_H__
