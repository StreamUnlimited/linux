/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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

#include <rtw_cfg80211_fullmac.h>

/* Match table for of_platform binding */
static const struct of_device_id rtw_axi_of_match[] = {
	{ .compatible = "realtek,rtl8730e_nic", },
	{},
};

MODULE_DEVICE_TABLE(of, rtw_axi_of_match);

PAXI_DATA paxi_data_global;

static void platform_device_init(struct platform_device *pdev)
{
	u32 status = false;
	struct resource *res_mem = NULL;
	struct device_node *sys_node = NULL, *ocp_node = NULL;
	unsigned long pmem_len = 0;
	struct resource res_sys = {0};
	PAXI_DATA axi_data;

	/* TODO: axi_data useless in fullmac, clear later. */
	axi_data = (PAXI_DATA)kzalloc(sizeof(AXI_DATA), in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	paxi_data_global = axi_data;

	ocp_node = of_get_parent(pdev->dev.of_node);
	sys_node = of_get_compatible_child(ocp_node, "realtek,amebad2-system-ctrl-ls");
	if (!sys_node) {
		pr_err("Can't get sys_node\n");
		goto free_dvobj;
	}

	axi_data->pdev = pdev;
	//platform_set_drvdata(pdev, dvobj); //LINUX_TODO

	/* TODO: SYS REG map, useless here. */
	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_mem) {
		pr_err("Can't get axi IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_mem->end - res_mem->start + 1;

	/* TODO: MAC REG map, useless here. */
	axi_data->axi_mem_start = (unsigned long)devm_ioremap_resource(&pdev->dev, res_mem);
	if (!axi_data->axi_mem_start) {
		pr_err("Can't map CTRL mem\n");
		goto exit;
	}
	axi_data->axi_mem_end = axi_data->axi_mem_start + pmem_len;

	pr_info("Memory mapped space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
			(unsigned long)res_mem->start, pmem_len, axi_data->axi_mem_start);

	if (of_address_to_resource(sys_node, 0, &res_sys)) {
		pr_err("Can't get sys IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_sys.end - res_sys.start + 1;

	axi_data->axi_sys_mem_start = 0;
	axi_data->axi_sys_mem_start = (unsigned long)ioremap(res_sys.start, pmem_len);
	if (!axi_data->axi_sys_mem_start) {
		pr_err("Can't map CTRL mem\n");
		goto exit;
	}
	axi_data->axi_sys_mem_end = axi_data->axi_sys_mem_start + pmem_len;

	pr_info("Memory mapped sys space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
			(unsigned long)res_sys.start, pmem_len, axi_data->axi_sys_mem_start);

	status = true;

free_dvobj:
	if (status != true) {
		platform_set_drvdata(pdev, NULL);
	}
exit:
	return ;
}

static void platform_device_deinit(struct platform_device *pdev)
{
	PAXI_DATA axi_data = paxi_data_global;

	platform_set_drvdata(pdev, NULL);
	if (axi_data) {
		if (axi_data->axi_sys_mem_start != 0) {
			iounmap((void *)axi_data->axi_sys_mem_start);
			axi_data->axi_sys_mem_start = 0;
		}

		if (axi_data->axi_mem_start != 0) {
			devm_iounmap(&pdev->dev, (void *)axi_data->axi_mem_start);
			axi_data->axi_mem_start = 0;
		}

		kfree((u8 *)axi_data);
		paxi_data_global = NULL;
	}
}

/* IPv4, IPv6 IP addr notifier */
static int rtw_inetaddr_notifier_call(struct notifier_block *nb,
									  unsigned long action, void *data)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)data;
	struct net_device *ndev;

	if (!ifa || !ifa->ifa_dev || !ifa->ifa_dev->dev) {
		return NOTIFY_DONE;
	}

	ndev = ifa->ifa_dev->dev;

	switch (action) {
	case NETDEV_UP:
		memcpy(global_idev.ip_addr, &ifa->ifa_address, RTW_IP_ADDR_LEN);
		dev_dbg(global_idev.fullmac_dev, "%s[%s]: up IP: [%pI4]\n", __func__, ifa->ifa_label, global_idev.ip_addr);
		break;
	case NETDEV_DOWN:
		memset(global_idev.ip_addr, 0, RTW_IP_ADDR_LEN);
		dev_dbg(global_idev.fullmac_dev, "%s[%s]: down IP: [%pI4]\n", __func__, ifa->ifa_label, global_idev.ip_addr);
		break;
	default:
		dev_dbg(global_idev.fullmac_dev, "%s: default action\n", __func__);
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block rtw_inetaddr_notifier = {
	.notifier_call = rtw_inetaddr_notifier_call
};

static void rtw_inetaddr_notifier_register(void)
{
	register_inetaddr_notifier(&rtw_inetaddr_notifier);
}

static void rtw_inetaddr_notifier_unregister(void)
{
	unregister_inetaddr_notifier(&rtw_inetaddr_notifier);
}

static int rtw_dev_probe(struct platform_device *pdev)
{
	int ret = false;

	dev_dbg(global_idev.fullmac_dev, "rtw_dev_probe start\n");
	memset(&global_idev, 0, sizeof(struct inic_device));

	/* Initialize axi_priv */
	platform_device_init(pdev);
	global_idev.fullmac_dev = &(pdev->dev);

	/*step1: alloc and init wiphy */
	ret = rtw_wiphy_init();
	if (ret == false) {
		dev_err(global_idev.fullmac_dev, "wiphy init fail");
		goto free_dvobj;
	}

	/*step3: register wiphy */
	if (wiphy_register(global_idev.pwiphy_global) != 0) {
		dev_err(global_idev.fullmac_dev, "wiphy register fail");
		goto os_ndevs_deinit;
	}

	/*step4: register netdev */
	ret = rtw_ndev_register();
	if (ret < 0) {
		dev_err(global_idev.fullmac_dev, "ndev register fail");
		goto os_ndevs_deinit;
	}

	ret = llhw_ipc_init();
	if (ret < 0) {
		dev_err(global_idev.fullmac_dev, "ipc init fail");
		goto free_dvobj;
	}

	rtw_regd_init();

	return 0; /* probe success */

os_ndevs_deinit:
	rtw_ndev_unregister();
	rtw_wiphy_deinit();

free_dvobj:
	platform_device_deinit(pdev);
	return -ENODEV;
}

static int rtw_dev_remove(struct platform_device *pdev)
{
	dev_dbg(global_idev.fullmac_dev, "rtw_dev_remove start.");

	rtw_ndev_unregister();
	dev_dbg(global_idev.fullmac_dev, "unregister netdev done.");

	wiphy_unregister(global_idev.pwiphy_global);

	rtw_wiphy_deinit();
	dev_dbg(global_idev.fullmac_dev, "unregister and deinit wiphy done.");

	llhw_ipc_deinit();
	dev_dbg(global_idev.fullmac_dev, "remove ipc done.");

	platform_device_deinit(pdev);
	dev_dbg(global_idev.fullmac_dev, "platform driver remove done.");

	pr_info("-%s done\n", __func__);
	memset(&global_idev, 0, sizeof(struct inic_device));

	return 0;
}

static void rtw_dev_shutdown(struct platform_device *pdev)
{
	dev_dbg(global_idev.fullmac_dev, "%s", __func__);
	rtw_dev_remove(pdev);
}

static int rtw_dev_suspend(struct platform_device *pdev, pm_message_t state)
{
	u32 ret = 0;

	dev_dbg(global_idev.fullmac_dev, "%s", __func__);

	ret = llhw_ipc_wifi_update_ip_addr_in_wowlan();
	if (ret == 0) {
		/* update ip address success, to suspend */
		/* set wowlan_state, to not schedule rx work */
		global_idev.wowlan_state = 1;
		netif_tx_stop_all_queues(global_idev.pndev[0]);
	} else {
		/* not suspend */
	}

	return ret;
}

static int rtw_dev_resume(struct platform_device *pdev)
{
	struct ipc_msg_q_priv *priv = &global_idev.msg_priv;

	dev_dbg(global_idev.fullmac_dev, "%s", __func__);

	netif_tx_start_all_queues(global_idev.pndev[0]);
	netif_tx_wake_all_queues(global_idev.pndev[0]);

	global_idev.wowlan_state = 0;

	/* wakeup recv work */
	if (!work_pending(&(priv->msg_work))) {
		schedule_work(&(priv->msg_work));
	}

	return 0;
}

static struct axi_drv_priv axi_drvpriv = {
	.rtw_axi_drv.probe = rtw_dev_probe,
	.rtw_axi_drv.remove = rtw_dev_remove,
	.rtw_axi_drv.shutdown = rtw_dev_shutdown,
	.rtw_axi_drv.suspend = rtw_dev_suspend,
	.rtw_axi_drv.resume = rtw_dev_resume,
	.rtw_axi_drv.driver = {
		.name = "fullmac-8730e",
		.owner = THIS_MODULE,
		.of_match_table = rtw_axi_of_match
	},
};

static int __init rtw_drv_entry(void)
{
	int ret = 0;

	rtw_inetaddr_notifier_register();

	ret = platform_driver_register(&axi_drvpriv.rtw_axi_drv);

	if (ret != 0) {
		rtw_inetaddr_notifier_unregister();
		goto exit;
	}

exit:
	pr_info("Fullmac module init ret=%d\n", ret);
	return ret;
}

static void __exit rtw_drv_halt(void)
{
	pr_info("Fullmac module exit start\n");
	dev_dbg(global_idev.fullmac_dev, "%s", __func__);
	platform_driver_unregister(&axi_drvpriv.rtw_axi_drv);
	pr_info("Fullmac module exit success\n");
}

module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
