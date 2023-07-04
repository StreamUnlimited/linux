#include <rtw_cfg80211_fullmac.h>

int llhw_ipc_xmit_skb_free_number(void)
{
	int free_num = 0;
	u32 delta = 0;
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;

	if (xmit_priv->skb_used_num < xmit_priv->skb_free_num) {
		delta = 0xFFFFFFFF - xmit_priv->skb_free_num + xmit_priv->skb_used_num;
	} else {
		delta = xmit_priv->skb_used_num - xmit_priv->skb_free_num;
	}
	free_num = SKB_NUM_AP - delta;

//    dev_dbg(global_idev.fullmac_dev, "used:%d %d freed:%d %d\n", xmit_priv->skb_used_num, xmit_priv->skb_used_num%SKB_NUM_AP,
//        xmit_priv->skb_free_num, xmit_priv->skb_free_num%SKB_NUM_AP);

	return free_num;
}

void llhw_ipc_xmit_done(int idx_wlan)
{
	u32 free_num;

	global_idev.xmit_priv.skb_free_num++;
	free_num = llhw_ipc_xmit_skb_free_number();

	if (free_num >= QUEUE_WAKE_THRES) {
		//dev_dbg(global_idev.fullmac_dev, "wq %d\n", free_num);
		netif_tx_wake_all_queues(global_idev.pndev[0]);
		netif_tx_wake_all_queues(global_idev.pndev[1]);
	}
}

int llhw_ipc_xmit_entry(int idx, struct sk_buff *pskb)
{
	struct dev_sk_buff *skb = NULL;
	struct skb_data *skb_data = NULL;
	dma_addr_t skb_data_phy = 0, skb_phy = 0;
	int ret = 0;
	int skb_index = 0, i;
	struct net_device *pndev = global_idev.pndev[idx];
	struct net_device_stats *pstats = &global_idev.stats[idx];
	u8 b_dropped = 0;
	int size = 0;
	struct inic_ipc_ex_msg ipc_msg = {0};

	u16 queue = skb_get_queue_mapping(pskb);
	u32 free_num = llhw_ipc_xmit_skb_free_number();
	if (free_num < QUEUE_STOP_THRES) {
		netif_stop_subqueue(pndev, queue);
		//dev_dbg(global_idev.fullmac_dev, "sq %d %d\n", free_num, queue);
	}

	if (free_num == 0) {
		dev_warn(global_idev.fullmac_dev, "[DEBUG] free_num is zero\n");
		ret = -EBUSY;
		goto drop_packet;
	}

	skb_index = global_idev.xmit_priv.skb_used_num % SKB_NUM_AP;

	skb = &global_idev.xmit_priv.host_skb_info[skb_index].skb;

	if (skb->busy) {
		/* skb alloc and free sequence may not consistent
		for example, for softap mode, if associated-sta in power save mode, softap will enqueue current skb */
		//dev_dbg(global_idev.fullmac_dev, "skb is busy 0x%x %d\n", skb_phy, skb_index);
		for (i = 0; i < SKB_NUM_AP; ++i) {
			skb = &global_idev.xmit_priv.host_skb_info[i].skb;
			if (skb->busy == 0) {
				skb_index = i;
				//dev_dbg(global_idev.fullmac_dev, "find skb 0x%x %d %d\n", skb_phy, skb_index, free_num);
				break;
			}

			if (i == SKB_NUM_AP - 1) {
				dev_warn(global_idev.fullmac_dev, "[ERROR] skb_not_free!!!!! %d\n", free_num);
				ret = -EBUSY;
				goto drop_packet;
			}
		}
	}
	skb_phy = global_idev.xmit_priv.host_skb_info_phy + sizeof(struct skb_info) * skb_index + sizeof(struct list_head);

	memset(skb, '\0', sizeof(struct dev_sk_buff));
	size = SKB_DATA_ALIGN(pskb->len + SKB_DATA_ALIGN(SKB_WLAN_TX_EXTRA_LEN));

	skb_data = &global_idev.xmit_priv.host_skb_data[skb_index];
	//skb_data_phy = physical address of skb_data->buf
	skb_data_phy = global_idev.xmit_priv.host_skb_data_phy + sizeof(struct skb_data) * skb_index + ((u32)(skb_data->buf) - (u32)skb_data);
	//printk("skb_data=0x%x, skb_data->buf=0x%x, skb_data_phy=0x%x pskb->len=%d", skb_data, skb_data->buf, skb_data_phy, pskb->len);

	memcpy(skb_data->buf + SKB_DATA_ALIGN(SKB_WLAN_TX_EXTRA_LEN), pskb->data, pskb->len);
	skb->head = (unsigned char *)skb_data_phy;
	skb->end = (unsigned char *)(skb_data_phy + size);
	skb->data = (unsigned char *)(skb_data_phy + SKB_DATA_ALIGN(SKB_WLAN_TX_EXTRA_LEN));
	skb->tail = (unsigned char *)(skb_data_phy + SKB_DATA_ALIGN(SKB_WLAN_TX_EXTRA_LEN));
	skb->busy = 1;
	skb->no_free = 1;
	atomic_set(&skb_data->ref, 1);
	dev_skb_put(skb, pskb->len);
	global_idev.xmit_priv.skb_used_num++;

	ipc_msg.event_num = IPC_WIFI_CMD_XIMT_PKTS;
	ipc_msg.msg_addr = (u32)skb_phy;
	ipc_msg.wlan_idx = idx;
	llhw_ipc_send_packet(&ipc_msg);

	pstats->tx_packets++;
	pstats->tx_bytes += pskb->len;
	goto func_exit;

drop_packet:
	b_dropped = 1;
	pstats->tx_dropped++;

func_exit:

	skb_tx_timestamp(pskb);
	if (b_dropped) {
		dev_kfree_skb_any(pskb);
	} else {
		dev_kfree_skb(pskb);
	}

	return ret;
}

int llhw_ipc_xmit_init(void)
{
	struct device *pdev = global_idev.ipc_dev;
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;

	if (xmit_priv->host_skb_data || xmit_priv->host_skb_info) {
		dev_err(global_idev.fullmac_dev, "host_skb_info or host_skb_data not mfree|\n");
		return -ENOMEM;
	}

	xmit_priv->host_skb_info = (struct skb_info *)dmam_alloc_coherent(pdev, sizeof(struct skb_info) * SKB_NUM_AP, &xmit_priv->host_skb_info_phy, GFP_KERNEL);
	if (!xmit_priv->host_skb_info) {
		dev_err(global_idev.fullmac_dev, "%s: malloc failed.", __func__);
		return -ENOMEM;
	}

	xmit_priv->host_skb_data = (struct skb_data *)dmam_alloc_coherent(pdev, sizeof(struct skb_data) * SKB_NUM_AP, &xmit_priv->host_skb_data_phy, GFP_KERNEL);
	if (!xmit_priv->host_skb_data) {
		dev_err(global_idev.fullmac_dev, "%s: malloc failed, free former one and return -ENOMEM.", __func__);
		dma_free_coherent(pdev, sizeof(struct skb_info) * SKB_NUM_AP, xmit_priv->host_skb_info, xmit_priv->host_skb_info_phy);
		return -ENOMEM;
	}

	memset(xmit_priv->host_skb_info, 0, sizeof(struct skb_info) * SKB_NUM_AP);
	memset(xmit_priv->host_skb_data, 0, sizeof(struct skb_data) * SKB_NUM_AP);
	return 0;
}

void llhw_ipc_xmit_deinit(void)
{
	struct device *pdev = global_idev.ipc_dev;
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;

	if (xmit_priv->host_skb_data || xmit_priv->host_skb_info) {
		dma_free_coherent(pdev, sizeof(struct skb_info) * SKB_NUM_AP, xmit_priv->host_skb_info, xmit_priv->host_skb_info_phy);
		dma_free_coherent(pdev, sizeof(struct skb_data) * SKB_NUM_AP, xmit_priv->host_skb_data, xmit_priv->host_skb_data_phy);
		memset(&global_idev.xmit_priv, 0, sizeof(struct xmit_priv_t));
	}
}
