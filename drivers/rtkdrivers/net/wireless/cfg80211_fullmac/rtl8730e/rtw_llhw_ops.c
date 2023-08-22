
#include <rtw_cfg80211_fullmac.h>

int llhw_ipc_send_msg(u32 id, u32 *param_buf, u32 buf_len)
{
	struct event_priv_t *event_priv = &global_idev.event_priv;
	int ret = 0;

	if (!global_idev.event_ch) {
		dev_err(global_idev.fullmac_dev, "%s: event ch is NULL when to send msg!\n",  "event");
		ret = -1;
		goto func_exit;
	}

	mutex_lock(&(event_priv->iiha_send_mutex));

	memset((u8 *)(event_priv->preq_msg), 0, sizeof(struct inic_ipc_host_req_msg));
	event_priv->preq_msg->api_id = id;
	if (param_buf) {
		memcpy(event_priv->preq_msg->param_buf, param_buf, buf_len * sizeof(u32));
	}

	memset((u8 *) & (event_priv->api_ipc_msg), 0, sizeof(ipc_msg_struct_t));
	event_priv->api_ipc_msg.msg = (u32)event_priv->req_msg_phy_addr;
	event_priv->api_ipc_msg.msg_type = IPC_USER_POINT;
	event_priv->api_ipc_msg.msg_len = sizeof(struct inic_ipc_host_req_msg);

	dev_dbg(global_idev.fullmac_dev, "-----LINUX SEND IPC\n");

	ret = ameba_ipc_channel_send(global_idev.event_ch, &(event_priv->api_ipc_msg));
	if (ret < 0) {
		dev_err(global_idev.fullmac_dev, "LINUX IPC SEND FAIL!!!! ret = %d", ret);
	}

	while (event_priv->preq_msg->api_id != IPC_WIFI_API_PROCESS_DONE) {
		udelay(10);
	}

	mutex_unlock(&(event_priv->iiha_send_mutex));

	ret = event_priv->preq_msg->ret;

func_exit:
	return ret;
}

void llhw_ipc_wifi_on(void)
{
	u32 param_buf[1];

	param_buf[0] = 0;

	llhw_ipc_send_msg(IPC_API_WIFI_ON, param_buf, 1);
}

int llhw_ipc_wifi_scan(rtw_scan_param_t *scan_param, u32 ssid_length, u32 block)
{
	int ret = 0;
	u32 param_buf[3];
	dma_addr_t dma_addr_scan_param = 0;
	struct device *pdev = global_idev.ipc_dev;

	dma_addr_scan_param = dma_map_single(pdev, scan_param, sizeof(rtw_scan_param_t), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_scan_param)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}

	param_buf[0] = (u32)dma_addr_scan_param;
	param_buf[1] = block;
	param_buf[2] = ssid_length;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_SCAN_NETWROKS, param_buf, 3);
	dma_unmap_single(pdev, dma_addr_scan_param, sizeof(rtw_scan_param_t), DMA_TO_DEVICE);

	return ret;
}

int llhw_ipc_wifi_connect(rtw_network_info_t *connect_param, unsigned char block)
{
	int ret = 0;
	internal_join_block_param_t *block_param = NULL;
	u32 param_buf[1] = {0};
	dma_addr_t dma_addr_connect_param = 0;
	struct device *pdev = global_idev.ipc_dev;
	int is_connected = -1;

	/* step1: check if there's ongoing connect*/
	if ((global_idev.mlme_priv.rtw_join_status > RTW_JOINSTATUS_UNKNOWN)
		&& (global_idev.mlme_priv.rtw_join_status < RTW_JOINSTATUS_SUCCESS)) {
		dev_err(global_idev.fullmac_dev, "[fullmac]: there is ongoing wifi connect!rtw_join_status=%d\n", global_idev.mlme_priv.rtw_join_status);
		return -EBUSY;
	}

	/*clear for last connect status */
	global_idev.mlme_priv.rtw_join_status = RTW_JOINSTATUS_STARTING;
	cfg80211_rtw_connect_indicate(RTW_JOINSTATUS_STARTING, NULL, 0);

	/* step2: malloc and set synchronous connection related variables*/
	if (block) {
		block_param = (internal_join_block_param_t *)kzalloc(sizeof(internal_join_block_param_t), GFP_KERNEL);
		if (!block_param) {
			ret = -ENOMEM;
			global_idev.mlme_priv.rtw_join_status = RTW_JOINSTATUS_FAIL;
			goto error;
		}
		block_param->block = block;
		/* initialize join_sema. */
		init_completion(&block_param->join_sema);
	}

	/* step3: set connect cmd to driver*/
	dma_addr_connect_param = dma_map_single(pdev, connect_param, sizeof(rtw_network_info_t), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_connect_param)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}
	param_buf[0] = (u32)dma_addr_connect_param;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_CONNECT, param_buf, 1);
	dma_unmap_single(pdev, dma_addr_connect_param, sizeof(rtw_network_info_t), DMA_TO_DEVICE);

	/* step4: wait connect finished for synchronous connection*/
	if (block) {
		global_idev.mlme_priv.join_block_param = block_param;
		block_param->join_timeout = RTW_JOIN_TIMEOUT;

		if (wait_for_completion_timeout(&block_param->join_sema, block_param->join_timeout) == 0) {
			dev_err(global_idev.fullmac_dev, "%s: Join bss timeout!\n", __func__);
			global_idev.mlme_priv.rtw_join_status = RTW_JOINSTATUS_FAIL;
			ret = -EINVAL;
			goto error;
		} else {
			is_connected = llhw_ipc_wifi_is_connected_to_ap();
			if (is_connected != 0) {
				ret = -EINVAL;
				global_idev.mlme_priv.rtw_join_status = RTW_JOINSTATUS_FAIL;
				goto error;
			}
		}
	}

error:
	if (block_param) {
		complete_release(&block_param->join_sema);
		kfree((u8 *)block_param);
		global_idev.mlme_priv.join_block_param = NULL;
	}

	if (global_idev.mlme_priv.rtw_join_status == RTW_JOINSTATUS_FAIL) {
		cfg80211_rtw_connect_indicate(RTW_JOINSTATUS_FAIL, NULL, 0);
	}

	return ret;
}

int llhw_ipc_wifi_disconnect(void)
{
	int ret = 0;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_DISCONNECT, NULL, 0);

	return ret;
}

int llhw_ipc_wifi_is_connected_to_ap(void)
{
	int ret = 0;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_IS_CONNECTED_TO_AP, NULL, 0);
	return ret;
}

int llhw_ipc_wifi_get_channel(u32 wlan_idx, u8 *ch)
{
	int ret = -1;
	u32 param_buf[2];
	int *channel_temp = NULL;
	dma_addr_t dma_addr = 0;
	struct device *pdev = global_idev.ipc_dev;

	channel_temp = kzalloc(sizeof(u8), GFP_KERNEL);
	if (channel_temp == NULL) {
		goto func_exit;
	}

	dma_addr = dma_map_single(pdev, channel_temp, sizeof(u8), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		ret = -EINVAL;
		goto free_buf;
	}
	param_buf[0] = wlan_idx;
	param_buf[1] = dma_addr;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_GET_CHANNEL, param_buf, 2);
	/* need do cache invalidate before get value */
	dma_unmap_single(pdev, dma_addr, sizeof(u8), DMA_FROM_DEVICE);
	*ch = *channel_temp;

free_buf:
	kfree((u8 *)channel_temp);

func_exit:
	return ret;
}

int llhw_ipc_wifi_init_ap(void)
{
	int ret = 0;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_INIT_AP, NULL, 0);
	return ret;
}

int llhw_ipc_wifi_deinit_ap(void)
{
	int ret = 0;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_DEINIT_AP, NULL, 0);
	return ret;
}

int llhw_ipc_wifi_del_sta(u8 wlan_idx, u8 *mac)
{
	int ret = 0;
	u32 param_buf[2];

	param_buf[0] = (u32)wlan_idx;
	param_buf[1] = (u32)mac;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_DEL_STA, param_buf, 2);
	return ret;
}

int llhw_ipc_wifi_start_ap(rtw_softap_info_t *softAP_config)
{
	int ret = 0;
	u32 param_buf[1];
	dma_addr_t dma_addr_password = 0;
	dma_addr_t dma_addr_softap_config = 0;
	struct device *pdev = global_idev.ipc_dev;

	dma_addr_softap_config = dma_map_single(pdev, softAP_config, sizeof(rtw_softap_info_t), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_password)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}

	param_buf[0] = (u32)dma_addr_softap_config;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_START_AP, param_buf, 1);

	dma_unmap_single(pdev, dma_addr_softap_config, sizeof(rtw_softap_info_t), DMA_FROM_DEVICE);

	return ret;
}

int llhw_ipc_wifi_stop_ap(void)
{
	int ret = 0;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_STOP_AP, NULL, 0);

	return ret;
}

int llhw_ipc_wifi_set_EDCA_params(unsigned int *AC_param)
{
	int ret = 0;
	dma_addr_t dma_addr_ac_param = 0;
	struct device *pdev = global_idev.ipc_dev;
	u32 param_buf[1] = {0};

	dma_addr_ac_param = dma_map_single(pdev, AC_param, sizeof(unsigned int), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_ac_param)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}
	param_buf[0] = (u32)dma_addr_ac_param;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_SET_EDCA_PARAM, param_buf, 1);
	dma_unmap_single(pdev, dma_addr_ac_param, sizeof(unsigned int), DMA_TO_DEVICE);

	return ret;
}

int llhw_ipc_wifi_add_key(struct rtw_crypt_info *crypt)
{
	int ret = 0;
	u32 param_buf[1];
	dma_addr_t dma_addr_crypt = 0;
	struct device *pdev = global_idev.ipc_dev;

	dma_addr_crypt = dma_map_single(pdev, crypt, sizeof(struct rtw_crypt_info), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_crypt)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}

	param_buf[0] = (u32)dma_addr_crypt;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_ADD_KEY, param_buf, 1);

	dma_unmap_single(pdev, dma_addr_crypt, sizeof(struct rtw_crypt_info), DMA_FROM_DEVICE);
	return ret;
}

int llhw_ipc_wifi_get_chplan(u8 *chplan)
{
	int ret = -1;
	u32 param_buf[1];
	int *chplan_temp = NULL;
	dma_addr_t dma_addr = 0;
	struct device *pdev = global_idev.ipc_dev;

	chplan_temp = kzalloc(sizeof(u8), GFP_KERNEL);
	if (chplan_temp == NULL) {
		goto func_exit;
	}

	dma_addr = dma_map_single(pdev, chplan_temp, sizeof(u8), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		ret = -EINVAL;
		goto free_buf;
	}
	param_buf[0] = dma_addr;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_GET_CHPLAN, param_buf, 1);
	/* need do cache invalidate before get value */
	dma_unmap_single(pdev, dma_addr, sizeof(u8), DMA_FROM_DEVICE);
	*chplan = *chplan_temp;

free_buf:
	kfree((u8 *)chplan_temp);

func_exit:
	return ret;
}

int llhw_ipc_wifi_tx_mgnt(u8 wlan_idx, const u8 *buf, size_t buf_len)
{
	int ret = 0;
	u32 param_buf[1];
	dma_addr_t dma_addr_buf = 0;
	dma_addr_t dma_addr_desc = 0;
	raw_data_desc_t raw_data_desc = {0};

	struct device *pdev = global_idev.ipc_dev;

	dma_addr_buf = dma_map_single(pdev, (void *)buf, buf_len, DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_buf)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}

	raw_data_desc.buf = (unsigned char *)dma_addr_buf;
	raw_data_desc.buf_len = buf_len;
	raw_data_desc.wlan_idx = wlan_idx;

	dma_addr_desc = dma_map_single(pdev, &raw_data_desc, sizeof(raw_data_desc_t), DMA_TO_DEVICE);
	if (dma_mapping_error(pdev, dma_addr_desc)) {
		dev_err(global_idev.fullmac_dev, "%s: mapping dma error!\n", __func__);
		return -1;
	}

	param_buf[0] = (u32)dma_addr_desc;
	ret = llhw_ipc_send_msg(IPC_API_WIFI_SEND_MGNT, param_buf, 1);

	dma_unmap_single(pdev, dma_addr_desc, sizeof(raw_data_desc_t), DMA_FROM_DEVICE);
	dma_unmap_single(pdev, dma_addr_buf, buf_len, DMA_FROM_DEVICE);

	return ret;
}

int llhw_ipc_wifi_sae_succ_start_assoc(void)
{
	int ret = 0;
	u32 param_buf[1];

	param_buf[0] = 0;
	ret = llhw_ipc_send_msg(IPC_API_WIFI_SAE_STATUS, param_buf, 1);
	return ret;
}

u32 llhw_ipc_wifi_update_ip_addr_in_wowlan(void)
{
	int ret = 0;
	struct event_priv_t *event_priv = &global_idev.event_priv;
	u32 param_buf[1] = {0};
	u32 try_cnt = 5000;//wait 10ms
	static u8 *ip_addr = NULL;
	static dma_addr_t ip_addr_phy = 0;

	if (ip_addr == NULL) {
		ip_addr = dmam_alloc_coherent(global_idev.fullmac_dev, sizeof(u32), &ip_addr_phy, GFP_KERNEL);
		if (!ip_addr) {
			dev_err(global_idev.fullmac_dev, "%s: allloc ip_addr error.\n", __func__);
			return -ENOMEM;
		}
		memcpy(ip_addr, global_idev.ip_addr, 4);
	}
	dev_dbg(global_idev.fullmac_dev, "%s ip=[%d.%d.%d.%d]\n", __func__, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

	param_buf[0] = (u32)ip_addr_phy;

	mutex_lock(&(event_priv->iiha_send_mutex));

	memset((u8 *)(event_priv->preq_msg), 0, sizeof(struct inic_ipc_host_req_msg));
	event_priv->preq_msg->api_id = IPC_API_WIFI_IP_UPDATE;
	event_priv->preq_msg->ret = -1;
	memcpy(event_priv->preq_msg->param_buf, param_buf, sizeof(u32));

	memset((u8 *) & (event_priv->api_ipc_msg), 0, sizeof(ipc_msg_struct_t));
	event_priv->api_ipc_msg.msg = (u32)event_priv->req_msg_phy_addr;
	event_priv->api_ipc_msg.msg_type = IPC_USER_POINT;
	event_priv->api_ipc_msg.msg_len = sizeof(struct inic_ipc_host_req_msg);

	ameba_ipc_channel_send(global_idev.event_ch, &(event_priv->api_ipc_msg));

	while (try_cnt) {
		if (event_priv->preq_msg->api_id != IPC_WIFI_API_PROCESS_DONE) {
			try_cnt --;
			udelay(2);
		} else {
			break;
		}
	}

	if (try_cnt == 0) {
		dev_err(global_idev.fullmac_dev, "wowlan update ip address fail");
	}

	mutex_unlock(&(event_priv->iiha_send_mutex));
	ret = event_priv->preq_msg->ret;

	if (ip_addr) {
		dma_free_coherent(global_idev.fullmac_dev, sizeof(u32), ip_addr, ip_addr_phy);
		ip_addr = NULL;
	}
	return ret;
}

int llhw_ipc_wifi_get_statistics(u32 statistic_phy)
{
	int ret = 0;
	u32 param_buf[1];

	/* ptr of statistics to fullfill. */
	param_buf[0] = (u32)statistic_phy;

	ret = llhw_ipc_send_msg(IPC_API_WIFI_GET_PHY_STATISTIC, param_buf, 1);

	return ret;
}

int llhw_ipc_wifi_mp_cmd(dma_addr_t cmd_phy, unsigned int cmd_len, dma_addr_t user_phy)
{
	u32 param_buf[4];

	param_buf[0] = (u32)cmd_phy;
	param_buf[1] = (u32)cmd_len;
	param_buf[2] = (u32)1;
	param_buf[3] = (u32)user_phy;

	return llhw_ipc_send_msg(IPC_API_WIFI_MP_CMD, param_buf, 4);
}

int llhw_ipc_wifi_iwpriv_cmd(dma_addr_t cmd_phy, unsigned int cmd_len, dma_addr_t user_phy)
{
	u32 param_buf[3];

	param_buf[0] = (u32)cmd_phy;
	param_buf[1] = (u32)cmd_len;
	param_buf[2] = (u32)1;

	return llhw_ipc_send_msg(IPC_API_WIFI_IWPRIV_INFO, param_buf, 3);
}

void llhw_ipc_send_packet(struct inic_ipc_ex_msg *p_ipc_msg)
{
	struct ipc_msg_q_priv *msg_priv = &global_idev.msg_priv;
	struct inic_ipc_ex_msg *pmsg = msg_priv->p_inic_ipc_msg;
	ipc_msg_struct_t ipc_msg = {0};
	int try_cnt = 100000;

	/* Get the warning of queue's depth not enough in peer, delay send the
	 * the next message.
	 */
	if (p_ipc_msg->msg_queue_status == IPC_WIFI_MSG_MEMORY_NOT_ENOUGH) {
		dev_err(global_idev.fullmac_dev, "[AP]%s: p_ipc_msg->msg_queue_status not enough. Do not msleep here.\n", __func__);
		/* TX SKBBUF 10 + RX DONE 10 = MSG POOL 20. If msg memory is still not enough, need debug.  */
	}

	spin_lock_bh(&msg_priv->ipc_send_msg_lock);

	/* Wait for another port ack acknowledgement last message sending */
	while (pmsg->event_num != IPC_WIFI_MSG_READ_DONE) {
		udelay(2);
		try_cnt--;
		if (try_cnt == 0) {
			dev_warn(global_idev.fullmac_dev, "[AP] inic ipc wait timeout %d\n", pmsg->event_num);
			break;
		}
	}

	pmsg->event_num = p_ipc_msg->event_num;
	pmsg->msg_addr = p_ipc_msg->msg_addr;
	pmsg->msg_queue_status = p_ipc_msg->msg_queue_status;
	pmsg->wlan_idx = p_ipc_msg->wlan_idx;

	/* Send the new message after last one acknowledgement */
	ipc_msg.msg_type = IPC_USER_POINT;
	ipc_msg.msg = msg_priv->ipc_msg_phy_addr;
	ipc_msg.msg_len = sizeof(struct inic_ipc_ex_msg);

	ameba_ipc_channel_send(global_idev.data_ch, &ipc_msg);
	spin_unlock_bh(&msg_priv->ipc_send_msg_lock);
}

u64 llhw_wifi_get_tsft(u8 iface_type)
{
	u8 *wifi_base_vir = NULL;
	u32 *tsf_base_vir = NULL;
	u32 reg_tsf_low = 0, reg_tsf_high = 0;
	u64 tsft_val = 0;

	wifi_base_vir = (u8 *)paxi_data_global->axi_mem_start;
	tsf_base_vir = (u32 *)(wifi_base_vir + 0x560);

	if (iface_type == 0) {
		reg_tsf_low = tsf_base_vir[0];
		reg_tsf_high = tsf_base_vir[1];
	} else if (iface_type == 1) {
		reg_tsf_low = tsf_base_vir[2];
		reg_tsf_high = tsf_base_vir[3];
	} else {
		dev_warn(global_idev.fullmac_dev, "[AP] unknown port(%d)!\n", iface_type);
	}

	tsft_val = ((u64)reg_tsf_high << 32) | reg_tsf_low;
	return tsft_val;
}

