#include <rtw_cfg80211_fullmac.h>

void llhw_xmit_task_handler(void *buf)
{
	inic_msg_info_t *msg = (inic_msg_info_t *)((u8 *)buf + SIZE_TX_DESC);

	/* send to NP*/
	llhw_host_send((u8 *)buf, SIZE_TX_DESC + sizeof(inic_msg_info_t) + msg->pad_len + msg->data_len);

	/* free buffer */
	kfree(buf);

}

int llhw_xmit_entry(int idx, struct sk_buff *pskb)
{
	int ret = NETDEV_TX_OK;
	bool b_dropped = false;
	u8 *buf = NULL;
	inic_msg_info_t *msg = NULL;
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;
	struct net_device_stats *pstats = &global_idev.stats[idx];

	/* allocate memory and copy data. */
	buf = kzalloc(SIZE_TX_DESC + sizeof(inic_msg_info_t) + pskb->len, GFP_KERNEL);
	if (!buf) {
		b_dropped = true;
		goto exit;
	}

	msg = (inic_msg_info_t *)(buf + SIZE_TX_DESC);
	msg->event = INIC_WIFI_EVT_XIMT_PKTS;
	msg->wlan_idx = idx;
	msg->data_len = pskb->len;
	msg->pad_len = 0;

	memcpy((void *)(msg + 1), pskb->data, pskb->len);

	inic_msg_enqueue(&xmit_priv->tx_msg_priv, (void *)buf);

	pstats->tx_packets++;
	pstats->tx_bytes += msg->data_len;

exit:
	skb_tx_timestamp(pskb);
	if (b_dropped) {
		pstats->tx_dropped++;
		ret = NETDEV_TX_BUSY;
		/* requeue or free this skb in netdevice, not here. */
	} else {
		dev_kfree_skb(pskb);
	}

	return ret;
}

int llhw_xmit_init(void)
{
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;

	inic_msg_q_init(&xmit_priv->tx_msg_priv, llhw_xmit_task_handler);

	return 0;
}

int llhw_xmit_deinit(void)
{
	struct xmit_priv_t *xmit_priv = &global_idev.xmit_priv;

	inic_msg_q_deinit(&xmit_priv->tx_msg_priv);

	return 0;
}


