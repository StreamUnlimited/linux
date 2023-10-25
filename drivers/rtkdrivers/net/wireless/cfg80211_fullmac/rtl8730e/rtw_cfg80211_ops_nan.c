#include <rtw_cfg80211_fullmac.h>
#ifdef CONFIG_NAN
void dump_cfg80211_nan_func_filter(u8 num,
								   struct cfg80211_nan_func_filter *nan_func_filter)
{
	u8 i;
	printk("=======================\n");
	printk("dump_cfg80211_nan_func_filter:\n");
	for (i = 0; i < num; i++) {
		printk("filer %d:\n", i);
		rtw_dump_buf("filter content:", nan_func_filter[i].filter, nan_func_filter[i].len);
	}
	printk("=======================\n");
}

void dump_cfg80211_nan_func(struct cfg80211_nan_func *func)
{
	int i;

	printk("=======================\n");
	printk("dump_cfg80211_nan_func:\n");
	switch (func->type) {
	case NL80211_NAN_FUNC_PUBLISH:
		printk("type: publish\n");
		if (func->publish_type & NL80211_NAN_SOLICITED_PUBLISH) {
			printk("publish_type: solicited\n");
			printk("publish_bcast: %s\n", (func->publish_bcast) ? "true" : "false");
		}
		if (func->publish_type & NL80211_NAN_UNSOLICITED_PUBLISH) {
			printk("publish_type: unsolicited\n");
		}
		break;
	case NL80211_NAN_FUNC_SUBSCRIBE:
		printk("type: subscribe\n");
		printk("subscribe_active: %s\n", (func->subscribe_active) ? "true" : "false");
		break;
	case NL80211_NAN_FUNC_FOLLOW_UP:
		printk("type: follow up\n");
		printk("followup_id: %d\n", func->followup_id);
		printk("followup_reqid: %d\n", func->followup_reqid);
		printk("followup_dest: "MAC_FMT"\n", MAC_ARG(func->followup_dest.addr));
		break;
	default:
		printk("type: unknown\n");

	}

	printk("service_id: %s\n", func->service_id);
	printk("close_range: %s\n", (func->close_range) ? "true" : "false");

	if (func->type == NL80211_NAN_FUNC_FOLLOW_UP) {
		goto end;
	}

	printk("ttl: %d\n", func->ttl);
	if (func->serv_spec_info_len > 0) {
		rtw_dump_buf("serv_spec_info: ", func->serv_spec_info, func->serv_spec_info_len);
	}
	printk("srf_include: %s\n", (func->srf_include) ? "true" : "false");
	if (func->srf_bf_len > 0) {
		rtw_dump_buf("srf_bf: ", func->srf_bf, func->srf_bf_len);
		printk("srf_bf_idx: %d\n", func->srf_bf_idx);
	}
	if (func->srf_num_macs > 0) {
		printk("srf_num_macs: %d\n", func->srf_num_macs);
		for (i = 0; i < func->srf_num_macs; i++) {
			printk("srf_mac: "MAC_FMT"\n", MAC_ARG(func->srf_macs[i].addr));
		}
	}
	if (func->num_rx_filters > 0) {
		printk("num_rx_filters: %d\n", func->num_rx_filters);
		dump_cfg80211_nan_func_filter(func->num_rx_filters, func->rx_filters);
	}
	if (func->num_tx_filters > 0) {
		printk("num_tx_filters: %d\n", func->num_tx_filters);
		dump_cfg80211_nan_func_filter(func->num_tx_filters, func->tx_filters);
	}

end:
	printk("instance_id: %d\n", func->instance_id);
	printk("cookie: %llu\n", func->cookie);
	printk("=======================\n");
}


static int cfg80211_rtw_start_nan(struct wiphy *wiphy,
								  struct wireless_dev *wdev,
								  struct cfg80211_nan_conf *conf)
{
	int ret = 0;
	u8 band_support;

	printk(" => %s\n", __func__);

	printk("master_pref = %d, bands = %d\n", conf->master_pref, conf->bands);

	/* prepare neccessary parameters */
	band_support = ((conf->bands & BIT(NL80211_BAND_2GHZ)) ? BAND_CAP_2G : 0)
				   | ((conf->bands & BIT(NL80211_BAND_5GHZ)) ? BAND_CAP_5G : 0);

	if (llhw_ipc_wifi_start_nan(conf->master_pref, band_support) == -1) {
		ret = -ENOTCONN;
	}
exit:
	return ret;
}

void cfg80211_rtw_stop_nan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	printk(" => %s\n", __func__);

	llhw_ipc_wifi_stop_nan();
}

static int cfg80211_rtw_add_nan_func(struct wiphy *wiphy,
									 struct wireless_dev *wdev,
									 struct cfg80211_nan_func *func)
{
	int ret = 0;
	rtw_nan_func_info_t nan_param = {0};
	u8 *serv_spec_info_vir = NULL, *srf_bf_vir = NULL, *srf_macs_vir = NULL;
	dma_addr_t serv_spec_info_phy, srf_bf_phy, srf_macs_phy;
	u8 *rx_filters_vir = NULL, *tx_filters_vir = NULL;
	dma_addr_t rx_filters_phy, tx_filters_phy;

	printk(" => %s\n", __func__);

	rtw_memcpy(&nan_param, func, sizeof(rtw_nan_func_info_t));

	if (func->serv_spec_info) {
		serv_spec_info_vir = dmam_alloc_coherent(global_idev.fullmac_dev, func->serv_spec_info_len, &serv_spec_info_phy, GFP_KERNEL);
		if (!serv_spec_info_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			return -ENOMEM;
			goto exit;
		}
		rtw_memcpy(serv_spec_info_vir, func->serv_spec_info, func->serv_spec_info_len);
		nan_param.serv_spec_info = serv_spec_info_phy;
	}

	if (func->srf_bf) {
		srf_bf_vir = dmam_alloc_coherent(global_idev.fullmac_dev, func->srf_bf_len, &srf_bf_phy, GFP_KERNEL);
		if (!srf_bf_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			ret = -ENOMEM;
			goto exit;
		}
		rtw_memcpy(srf_bf_vir, func->srf_bf, func->srf_bf_len);
		nan_param.srf_bf = srf_bf_phy;
	}

	if (func->srf_macs) {
		srf_macs_vir = dmam_alloc_coherent(global_idev.fullmac_dev,
										   func->srf_num_macs * sizeof(struct mac_address), &srf_macs_phy, GFP_KERNEL);
		if (!srf_macs_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			ret = -ENOMEM;
			goto exit;
		}
		rtw_memcpy(srf_macs_vir, func->srf_macs, func->srf_num_macs * sizeof(struct mac_address));
		nan_param.srf_macs = srf_macs_phy;
	}

	if (func->rx_filters) {
		rx_filters_vir = dmam_alloc_coherent(global_idev.fullmac_dev,
											 func->num_rx_filters * sizeof(struct cfg80211_nan_func_filter), &rx_filters_phy, GFP_KERNEL);
		if (!rx_filters_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			ret = -ENOMEM;
			goto exit;
		}
		rtw_memcpy(rx_filters_vir, func->rx_filters, func->num_rx_filters * sizeof(struct cfg80211_nan_func_filter));
		nan_param.rx_filters = rx_filters_phy;
	}

	if (func->tx_filters) {
		tx_filters_vir = dmam_alloc_coherent(global_idev.fullmac_dev,
											 func->num_tx_filters * sizeof(struct cfg80211_nan_func_filter), &tx_filters_phy, GFP_KERNEL);
		if (!tx_filters_vir) {
			dev_dbg(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
			ret = -ENOMEM;
			goto exit;
		}
		rtw_memcpy(tx_filters_vir, func->tx_filters, func->num_tx_filters * sizeof(struct cfg80211_nan_func_filter));
		nan_param.tx_filters = tx_filters_phy;
	}

	if (llhw_ipc_wifi_add_nan_func(&nan_param, func) < 0) {
		ret = -ENOTCONN;
	}

	dump_cfg80211_nan_func(func);

exit:
	if (serv_spec_info_vir) {
		dma_free_coherent(global_idev.fullmac_dev, func->serv_spec_info_len, serv_spec_info_vir, serv_spec_info_phy);
	}
	if (srf_bf_vir) {
		dma_free_coherent(global_idev.fullmac_dev, func->srf_bf_len, srf_bf_vir, srf_bf_phy);
	}
	if (srf_macs_vir) {
		dma_free_coherent(global_idev.fullmac_dev, func->srf_num_macs, srf_macs_vir, srf_macs_phy);
	}
	if (rx_filters_vir) {
		dma_free_coherent(global_idev.fullmac_dev, func->num_rx_filters, rx_filters_vir, rx_filters_phy);
	}
	if (tx_filters_vir) {
		dma_free_coherent(global_idev.fullmac_dev, func->num_tx_filters, tx_filters_vir, tx_filters_phy);
	}

	return ret;
}

void cfg80211_rtw_nan_func_free(struct wireless_dev *nan_wdev, void *os_dep_data)
{
	gfp_t kflags;
	struct cfg80211_nan_func *func = os_dep_data;

	if (nan_wdev == NULL) {
		printk("%s: No nan wdev\n", __func__);
		return;
	}

	if (func == NULL) {
		return;
	}

	kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

	printk("%s: Remove NAN func cookie (%llu)\n", __func__, func->cookie);
	cfg80211_nan_func_terminated(nan_wdev, func->instance_id,
								 NL80211_NAN_FUNC_TERM_REASON_USER_REQUEST,
								 func->cookie, kflags);
	cfg80211_free_nan_func(func);
}

void cfg80211_rtw_del_nan_func(struct wiphy *wiphy,
							   struct wireless_dev *wdev, u64 cookie)
{
	void *func = NULL;

	printk("%s =>\n", __func__);

	llhw_ipc_wifi_del_nan_func(cookie);
}

static int cfg80211_rtw_nan_change_conf(struct wiphy *wiphy,
										struct wireless_dev *wdev,
										struct cfg80211_nan_conf *conf,
										u32 changes)
{
	int ret = 0;

	printk(" => %s\n", __func__);

	printk("master_pref = %d, bands = %d changes=%d\n", conf->master_pref, conf->bands, changes);

	//rtw_update_nan_conf(conf,changes);

exit:
	return ret;
}


void cfg80211_nan_handle_sdf(u8 type, u8 inst_id, u8 peer_inst_id, u8 *addr, u32 info_len, u8 *info, u64 cookie)
{
	struct cfg80211_nan_match_params match_param;

	gfp_t kflags;

	kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	memset(&match_param, 0, sizeof(match_param));
	match_param.type = type;
	match_param.inst_id = inst_id;
	match_param.peer_inst_id = peer_inst_id;
	match_param.addr = addr;
	match_param.info_len = info_len;
	match_param.info = info;
	match_param.cookie = cookie;
	RTW_INFO("%s: type = %d inst_id = %d peer_inst_id = %d addr = "MAC_FMT" cookie = %llu\n", __func__,
			 match_param.type, match_param.inst_id, match_param.peer_inst_id, MAC_ARG(match_param.addr), match_param.cookie);
	cfg80211_nan_match(global_idev.pwdev_global[1], &match_param, kflags);

}

void cfg80211_nan_func_free(void *os_dep_data)
{
	struct wireless_dev *wdev = global_idev.pwdev_global[2];
	gfp_t kflags;
	struct cfg80211_nan_func *func = os_dep_data;

	if (wdev == NULL) {
		RTW_WARN("%s: No nan wdev\n", __func__);
		return;
	}

	kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

	if (func != NULL) {
		RTW_INFO("%s: Remove NAN func cookie (%llu)\n", __func__, func->cookie);
		cfg80211_nan_func_terminated(wdev, func->instance_id, NL80211_NAN_FUNC_TERM_REASON_USER_REQUEST, func->cookie, kflags);
		cfg80211_free_nan_func(func);
	}
}

void cfg80211_rtw_ops_nan_init(void)
{
	struct cfg80211_ops *ops = &global_idev.rtw_cfg80211_ops;
	ops->start_nan = cfg80211_rtw_start_nan;
	ops->stop_nan = cfg80211_rtw_stop_nan;
	ops->add_nan_func = cfg80211_rtw_add_nan_func;
	ops->del_nan_func = cfg80211_rtw_del_nan_func;
	ops->nan_change_conf = cfg80211_rtw_nan_change_conf;
}
#endif
