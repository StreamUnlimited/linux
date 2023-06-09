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
#define _HCI_INTF_C_


#include <rtw_drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <ameba_ipc/ameba_ipc.h>
#include <linux/init.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <rtw_netdev_intf.h>

#ifndef CONFIG_AXI_HCI
#error "CONFIG_AXI_HCI shall be on!\n"
#endif

#ifdef CONFIG_GLOBAL_UI_PID
int ui_pid[3] = {0, 0, 0};
#endif

typedef struct axi_data {
	struct platform_device *pdev;

	/* AXI MEM map */
	unsigned long axi_mem_end; /* shared mem end */
	unsigned long axi_mem_start; /* shared mem start */

	/* AXI MEM map */
	unsigned long axi_sys_mem_end; /* shared mem end */
	unsigned long axi_sys_mem_start; /* shared mem start */

	u8 bdma64;
} AXI_DATA, *PAXI_DATA;

struct axi_drv_priv {
	struct platform_driver rtw_axi_drv;
	int drv_registered;
};

/* Match table for of_platform binding */
static const struct of_device_id rtw_axi_of_match[] = {
	{ .compatible = "realtek,rtl8730e_nic", },
	{},
};

MODULE_DEVICE_TABLE(of, rtw_axi_of_match);

//LINUX_TODO:
extern struct device *dev_global;
extern struct wiphy *pwiphy_global;
extern struct net_device	*pnetdev_global[2];

PAXI_DATA paxi_data_global;

static void axi_dvobj_init(struct platform_device *pdev)
{
	int err;
	u32 status = _FAIL;
	struct pci_priv	*pcipriv = NULL;
	struct resource *res_mem = NULL;
	struct device_node *sys_node = NULL, *ocp_node = NULL;
	unsigned long pmem_len = 0;
	struct resource res_sys = {0};
	int i;
	PAXI_DATA axi_data;

	axi_data = (PAXI_DATA)rtw_zmalloc(sizeof(AXI_DATA));
	paxi_data_global = axi_data;

	ocp_node = of_get_parent(pdev->dev.of_node);
	sys_node = of_get_compatible_child(ocp_node, "realtek,amebad2-system-ctrl-ls");
	if (!sys_node) {
		RTW_ERR("Can't get sys_node\n");
		goto free_dvobj;
	}

	axi_data->pdev = pdev;
	//platform_set_drvdata(pdev, dvobj); //LINUX_TODO

	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_mem) {
		RTW_ERR("Can't get axi IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_mem->end - res_mem->start + 1;

	axi_data->axi_mem_start = (unsigned long)devm_ioremap_resource(&pdev->dev, res_mem);
	if (!axi_data->axi_mem_start) {
		RTW_ERR("Can't map CTRL mem\n");
		goto exit;
	}
	axi_data->axi_mem_end = axi_data->axi_mem_start + pmem_len;

	RTW_INFO("Memory mapped space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
			 (unsigned long)res_mem->start, pmem_len, axi_data->axi_mem_start);

	if (of_address_to_resource(sys_node, 0, &res_sys)) {
		RTW_ERR("Can't get sys IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_sys.end - res_sys.start + 1;

	axi_data->axi_sys_mem_start = 0;
	axi_data->axi_sys_mem_start = (unsigned long)ioremap(res_sys.start, pmem_len);
	if (!axi_data->axi_sys_mem_start) {
		RTW_ERR("Can't map CTRL mem\n");
		goto exit;
	}
	axi_data->axi_sys_mem_end = axi_data->axi_sys_mem_start + pmem_len;

	RTW_INFO("Memory mapped sys space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
			 (unsigned long)res_sys.start, pmem_len, axi_data->axi_sys_mem_start);

	/* LINUX_TODO: initialize the ipc channel */

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS) {
		platform_set_drvdata(pdev, NULL);
	}
exit:
	return ;
}


static void axi_dvobj_deinit(struct platform_device *pdev)
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

		rtw_mfree((u8 *)axi_data, sizeof(AXI_DATA));
		paxi_data_global = NULL;
	}

}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_dev_probe(struct platform_device *pdev)
{
	printk("rtw_dev_probe start\n");

	/* Initialize axi_priv */
	axi_dvobj_init(pdev);
	dev_global = &(pdev->dev); //LINUX_TODO

#ifdef CONFIG_GLOBAL_UI_PID
	if (ui_pid[1] != 0) {
		RTW_INFO("ui_pid[1]:%d\n", ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	/* dev_alloc_name && register_netdev */
	if (rtw_os_ndevs_init() != _SUCCESS) {
		printk("rtw_os_ndevs_init Failed!\n");
		goto free_dvobj;
	}

	return 0; /* _SUCCESS;*/


os_ndevs_deinit:
	rtw_os_ndevs_deinit();

free_dvobj:
	axi_dvobj_deinit(pdev);

exit:
	return -ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
/* rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both */
static int rtw_dev_remove(struct platform_device *pdev)
{
	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
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

	axi_dvobj_deinit(pdev);

	RTW_INFO("-%s done\n", __func__);
	return 0;
}

static void rtw_dev_shutdown(struct platform_device *pdev)
{
	rtw_dev_remove(pdev);
}

static int rtw_dev_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int rtw_dev_resume(struct platform_device *pdev)
{
	return 0;
}

static struct axi_drv_priv axi_drvpriv = {
	.rtw_axi_drv.probe = rtw_dev_probe,
	.rtw_axi_drv.remove = rtw_dev_remove,
	.rtw_axi_drv.shutdown = rtw_dev_shutdown,
	.rtw_axi_drv.suspend = rtw_dev_suspend,
	.rtw_axi_drv.resume = rtw_dev_resume,
	.rtw_axi_drv.driver = {
		.name = (char *)DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = rtw_axi_of_match
	},
};

static int __init rtw_drv_entry(void)
{
	int ret = 0;

	printk("module init start\n");
	//dump_drv_version(RTW_DBGDUMP);

	axi_drvpriv.drv_registered = _TRUE;
	//rtw_drv_proc_init();
	//rtw_nlrtw_init();
	//rtw_ndev_notifier_register();
	//rtw_inetaddr_notifier_register();

	ret = platform_driver_register(&axi_drvpriv.rtw_axi_drv);

	if (ret != 0) {
		axi_drvpriv.drv_registered = _FALSE;
		//rtw_drv_proc_deinit();
		//rtw_nlrtw_deinit();
		//rtw_ndev_notifier_unregister();
		//rtw_inetaddr_notifier_unregister();
		goto exit;
	}

exit:
	RTW_PRINT("module init ret=%d\n", ret);
	return ret;
}

static void __exit rtw_drv_halt(void)
{
	RTW_PRINT("module exit start\n");

	axi_drvpriv.drv_registered = _FALSE;

	platform_driver_unregister(&axi_drvpriv.rtw_axi_drv);

	//rtw_drv_proc_deinit();
	//rtw_nlrtw_deinit();
	//rtw_ndev_notifier_unregister();
	//rtw_inetaddr_notifier_unregister();

	RTW_PRINT("module exit success\n");
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
