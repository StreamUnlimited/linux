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

#include <drv_types.h>

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <rtw_trx_axi.h>
#include <linux/of_address.h>
#include <linux/io.h>

#ifndef CONFIG_AXI_HCI
#error "CONFIG_AXI_HCI shall be on!\n"
#endif

#ifdef CONFIG_GLOBAL_UI_PID
int ui_pid[3] = {0, 0, 0};
#endif

#ifdef CONFIG_PM
static int rtw_axi_suspend(struct platform_device *pdev, pm_message_t state);
static int rtw_axi_resume(struct platform_device *pdev);
#endif

static int rtw_dev_probe(struct platform_device *pdev);
static int rtw_dev_remove(struct platform_device *pdev);
static void rtw_dev_shutdown(struct platform_device *pdev);



struct axi_drv_priv {
	struct platform_driver rtw_axi_drv;
	int drv_registered;
};

/* Match table for of_platform binding */
static const struct of_device_id rtw_axi_of_match[] = {
	{ .compatible = "realtek,rtl8730e_nic", },
	{},
};

static struct axi_drv_priv axi_drvpriv = {
	.rtw_axi_drv.probe = rtw_dev_probe,
	.rtw_axi_drv.remove = rtw_dev_remove,
	.rtw_axi_drv.shutdown = rtw_dev_shutdown,
#ifdef CONFIG_PM
	.rtw_axi_drv.suspend = rtw_axi_suspend,
	.rtw_axi_drv.resume = rtw_axi_resume,
#endif
	.rtw_axi_drv.driver = {
		.name = (char *)DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = rtw_axi_of_match
	},
};

MODULE_DEVICE_TABLE(of, rtw_axi_of_match);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)) || (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18))
#define rtw_axi_interrupt(x, y, z) rtw_axi_interrupt(x, y)
#endif

static irqreturn_t rtw_axi_interrupt(int irq, void *priv, struct pt_regs *regs)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)priv;
	PAXI_DATA axi_data = dvobj_to_axi(dvobj);
	enum rtw_phl_status pstatus =  RTW_PHL_STATUS_SUCCESS;
	unsigned long sp_flags;

	if (axi_data->irq_enabled == 0 || !dvobj || !axi_data || !dvobj->phl) {
		return IRQ_HANDLED;
	}

	_rtw_spinlock_irq(&axi_data->irq_th_lock, &sp_flags);
	if (rtw_phl_recognize_interrupt(dvobj->phl)) {
		pstatus = rtw_phl_interrupt_handler(dvobj->phl);
	}
	_rtw_spinunlock_irq(&axi_data->irq_th_lock, &sp_flags);

	if (pstatus == RTW_PHL_STATUS_FAILURE) {
		return IRQ_HANDLED;
	}
	/* return IRQ_NONE; */

	return IRQ_HANDLED;
}

static u32 rtw_fw_ipc_recv(aipc_ch_t *ch, ipc_msg_struct_t *pmsg)
{
	return 0;
}

/**
 * @brief entity of struct aipc_ch_ops. It will associate the channel_recv to recv
 */
static struct aipc_ch_ops rtw_fw_ipc_ops = {
	.channel_recv = rtw_fw_ipc_recv,
};

int axi_alloc_irq(struct dvobj_priv *dvobj)
{
	PAXI_DATA axi_data = dvobj_to_axi(dvobj);
	struct platform_device *pdev = axi_data->pdev;
	int err = _SUCCESS;

	/* get the irq nember from device tree */
	axi_data->irq_dma = platform_get_irq(pdev, 0);
	if (axi_data->irq_dma < 0) {
		RTW_INFO("Error allocating DMA IRQ %d", axi_data->irq_dma);
		err = _FAIL;
		goto exit;
	}

	axi_data->irq_protocol = platform_get_irq(pdev, 1);
	if (axi_data->irq_protocol < 0) {
		RTW_INFO("Error allocating protocol IRQ %d", axi_data->irq_protocol);
		err = _FAIL;
		goto exit;
	}

#if defined(IRQF_SHARED)
	err = request_irq(axi_data->irq_dma, &rtw_axi_interrupt, IRQF_SHARED, DRV_NAME, dvobj);
#else
	err = request_irq(axi_data->irq_dma, &rtw_axi_interrupt, SA_SHIRQ, DRV_NAME, dvobj);
#endif
	if (err) {
		RTW_INFO("Error allocating IRQ %d", axi_data->irq_dma);
		goto exit;
	} else {
		RTW_INFO("Request_irq OK, IRQ %d\n", axi_data->irq_dma);
	}

#if defined(IRQF_SHARED)
	err = request_irq(axi_data->irq_protocol, &rtw_axi_interrupt, IRQF_SHARED, DRV_NAME, dvobj);
#else
	err = request_irq(axi_data->irq_protocol, &rtw_axi_interrupt, SA_SHIRQ, DRV_NAME, dvobj);
#endif
	if (err) {
		RTW_INFO("Error allocating IRQ %d", axi_data->irq_protocol);
		free_irq(axi_data->irq_dma, dvobj);
	} else {
		axi_data->irq_alloc = 1;
		RTW_INFO("Request_irq OK, IRQ %d\n", axi_data->irq_protocol);
	}

exit:
	return err ? _FAIL : _SUCCESS;
}

static struct dvobj_priv *axi_dvobj_init(struct platform_device *pdev)
{
	int err;
	u32 status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	struct pci_priv	*pcipriv = NULL;
	struct resource *res_mem = NULL;
	struct device_node *sys_node = NULL, *ocp_node = NULL;
	unsigned long pmem_len = 0;
	struct resource res_sys = {0};
	int i;
	PAXI_DATA axi_data;

	dvobj = devobj_init();
	if (dvobj == NULL) {
		goto exit;
	}

	ocp_node = of_get_parent(pdev->dev.of_node);
	sys_node = of_get_compatible_child(ocp_node, "realtek,amebad2-system-ctrl-ls");
	if (!sys_node) {
		RTW_ERR("Can't get sys_node\n");
		goto free_dvobj;
	}

	axi_data = dvobj_to_axi(dvobj);

	axi_data->pdev = pdev;
	platform_set_drvdata(pdev, dvobj);

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

	axi_data->fw_ipc = ameba_ipc_alloc_ch(sizeof(PAXI_DATA));
	if (!axi_data->fw_ipc) {
		RTW_ERR("no memory for ipc channel\n");
		goto iounmap;
	}

	/* initialize the ipc channel */
	axi_data->fw_ipc->port_id = AIPC_PORT_LP;
	axi_data->fw_ipc->ch_id = 1; /* <  NP -->  LP FW Info channel 1 */
	axi_data->fw_ipc->ch_config = AIPC_CONFIG_NOTHING;
	axi_data->fw_ipc->ops = &rtw_fw_ipc_ops;
	axi_data->fw_ipc->priv_data = axi_data;

	if (ameba_ipc_channel_register(axi_data->fw_ipc)) {
		RTW_ERR("register ipc channel failed.\n");
		goto free_ipc;
	}

	/*step 1-1., decide the chip_type via device info*/
	dvobj->interface_type = RTW_HCI_AXI;
	dvobj->ic_id = RTL8730E;
	dvobj->intf_ops = &axi_ops;


	status = _SUCCESS;

free_ipc:
	if (status != _SUCCESS && (axi_data->fw_ipc)) {
		kfree(axi_data->fw_ipc);
	}

iounmap:
	if (status != _SUCCESS && axi_data->axi_mem_start != 0) {
		if (axi_data->axi_sys_mem_start) {
			iounmap((void *)axi_data->axi_sys_mem_start);
			axi_data->axi_sys_mem_start = 0;
		}

		if (axi_data->axi_mem_start) {
			devm_iounmap(&pdev->dev, (void *)axi_data->axi_mem_start);
			axi_data->axi_mem_start = 0;
		}
	}

free_dvobj:
	if (status != _SUCCESS && dvobj) {
		platform_set_drvdata(pdev, NULL);
		devobj_deinit(dvobj);
		dvobj = NULL;
	}
exit:
	return dvobj;
}


static void axi_dvobj_deinit(struct platform_device *pdev)
{
	struct dvobj_priv *dvobj = platform_get_drvdata(pdev);
	PAXI_DATA axi_data = dvobj_to_axi(dvobj);

	platform_set_drvdata(pdev, NULL);
	if (dvobj && axi_data) {
		if (axi_data->fw_ipc) {
			ameba_ipc_channel_unregister(axi_data->fw_ipc);
			axi_data->fw_ipc = NULL;
		}

		if (axi_data->irq_alloc) {
			free_irq(axi_data->irq_dma, dvobj);
			free_irq(axi_data->irq_protocol, dvobj);
			axi_data->irq_alloc = 0;
		}

		if (axi_data->axi_sys_mem_start != 0) {
			iounmap((void *)axi_data->axi_sys_mem_start);
			axi_data->axi_sys_mem_start = 0;
		}

		if (axi_data->axi_mem_start != 0) {
			devm_iounmap(&pdev->dev, (void *)axi_data->axi_mem_start);
			axi_data->axi_mem_start = 0;
		}

		devobj_deinit(dvobj);
		dvobj = NULL;
	}

}

#ifdef CONFIG_PM
static int rtw_axi_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	struct dvobj_priv *dvobj = platform_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);

exit:
	return ret;

}

static int rtw_resume_process(_adapter *padapter)
{
	return rtw_resume_common(padapter);
}

static int rtw_axi_resume(struct platform_device *pdev)
{
	struct dvobj_priv *dvobj = platform_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);
	struct net_device *pnetdev = padapter->pnetdev;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	int	err = 0;

	return err;
}
#endif/* CONFIG_PM */

_adapter *rtw_axi_primary_adapter_init(struct dvobj_priv *dvobj, \
				       struct platform_device *pdev)
{
	_adapter *padapter = NULL;
	int status = _FAIL;
	u8 hw_mac_addr[ETH_ALEN] = {0};

	padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (padapter == NULL) {
		goto exit;
	}

	/*registry_priv*/
	if (rtw_load_registry(padapter) != _SUCCESS) {
		goto free_adapter;
	}

	padapter->dvobj = dvobj;

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

	/* set adapter_type/iface type for primary padapter */
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;

	if (rtw_init_drv_sw(padapter) == _FAIL) {
		goto free_adapter;
	}

	/* get mac addr */
	rtw_hw_get_mac_addr(dvobj, hw_mac_addr);

	rtw_macaddr_cfg(adapter_mac_addr(padapter), hw_mac_addr);

	status = _SUCCESS;

free_adapter:
	if (status != _SUCCESS && padapter) {
		rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_axi_primary_adapter_deinit(_adapter *padapter)
{
	rtw_free_drv_sw(padapter);

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndev_free(padapter);

	rtw_vmfree((u8 *)padapter, sizeof(_adapter));
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_dev_probe(struct platform_device *pdev)
{
	_adapter *padapter = NULL;
	struct dvobj_priv *dvobj;

	RTW_INFO("+%s\n", __func__);

	/* Initialize dvobj_priv */
	dvobj = axi_dvobj_init(pdev);
	if (dvobj == NULL) {
		RTW_ERR("axi_dvobj_init Failed!\n");
		goto exit;
	}

	if (devobj_trx_resource_init(dvobj) == _FAIL) {
		goto free_dvobj;
	}

	/*init hw - register and get chip-info */
	if (rtw_hw_init(dvobj) == _FAIL) {
		RTW_ERR("rtw_hw_init Failed!\n");
		goto free_trx_reso;
	}

	/* Initialize primary adapter */
	padapter = rtw_axi_primary_adapter_init(dvobj, pdev);
	if (padapter == NULL) {
		RTW_ERR("rtw_axi_primary_adapter_init Failed!\n");
		goto free_hw;
	}

	/* Initialize virtual interface */
#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_drv_add_vir_ifaces(dvobj) == _FAIL) {
		goto free_if_vir;
	}
#endif

	/*init data of dvobj from registary and ic spec*/
	if (devobj_data_init(dvobj) == _FAIL) {
		RTW_ERR("devobj_data_init Failed!\n");
		goto free_devobj_data;
	}

#ifdef CONFIG_GLOBAL_UI_PID
	if (ui_pid[1] != 0) {
		RTW_INFO("ui_pid[1]:%d\n", ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	/* dev_alloc_name && register_netdev */
	if (rtw_os_ndevs_init(dvobj) != _SUCCESS) {
		RTW_ERR("rtw_os_ndevs_init Failed!\n");
		goto free_devobj_data;
	}

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

	/* alloc irq */
	if (axi_alloc_irq(dvobj) != _SUCCESS) {
		RTW_ERR("axi_alloc_irq Failed!\n");
		goto os_ndevs_deinit;
	}

	RTW_INFO("-%s success\n", __func__);
	return 0; /* _SUCCESS;*/


os_ndevs_deinit:
	rtw_os_ndevs_deinit(dvobj);

free_devobj_data:
	devobj_data_deinit(dvobj);

free_if_vir:
#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_stop_vir_ifaces(dvobj);
	rtw_drv_free_vir_ifaces(dvobj);
#endif
	rtw_axi_primary_adapter_deinit(padapter);

free_hw:
	rtw_hw_deinit(dvobj);

free_trx_reso:
	devobj_trx_resource_deinit(dvobj);

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
	struct dvobj_priv *dvobj = platform_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);
	struct net_device *pnetdev = padapter->pnetdev;

	if (dvobj->processing_dev_remove == _TRUE) {
		RTW_WARN("%s-line%d: Warning! device has been removed!\n", __func__, __LINE__);
		return -1;
	}

	RTW_INFO("+%s\n", __func__);

	dvobj->processing_dev_remove = _TRUE;

	if (unlikely(!padapter)) {
		return -1;
	}

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndevs_unregister(dvobj);

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(dvobj_to_pwrctl(dvobj));
#endif
#if 0 /*GEORGIA_TODO_FIXIT*/
	if (GET_PHL_COM(pdvobjpriv)->fw_ready == _TRUE) {
		rtw_pm_set_ips(padapter, IPS_NONE);
		rtw_pm_set_lps(padapter, PM_PS_MODE_ACTIVE);

		LeaveAllPowerSaveMode(padapter);
	}
#endif
	dev_set_drv_stopped(adapter_to_dvobj(padapter));	/*for stop thread*/
#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	rtw_stop_cmd_thread(padapter);
#endif
#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_stop_vir_ifaces(dvobj);
#endif

	rtw_drv_stop_prim_iface(padapter);

	rtw_hw_stop(dvobj);
	dev_set_surprise_removed(dvobj);

	rtw_axi_primary_adapter_deinit(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_free_vir_ifaces(dvobj);
#endif
	rtw_hw_deinit(dvobj);
	devobj_data_deinit(dvobj);
	devobj_trx_resource_deinit(dvobj);
	axi_dvobj_deinit(pdev);

	RTW_INFO("-%s done\n", __func__);
	return 0;
}

static void rtw_dev_shutdown(struct platform_device *pdev)
{
	rtw_dev_remove(pdev);
}

static int __init rtw_drv_entry(void)
{
	int ret = 0;

	RTW_PRINT("module init start\n");
	dump_drv_version(RTW_DBGDUMP);
#ifdef BTCOEXVERSION
	RTW_PRINT(DRV_NAME" BT-Coex version = %s\n", BTCOEXVERSION);
#endif /* BTCOEXVERSION */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
	/* console_suspend_enabled=0; */
#endif

	axi_drvpriv.drv_registered = _TRUE;
	rtw_suspend_lock_init();
	rtw_drv_proc_init();
	rtw_nlrtw_init();
	rtw_ndev_notifier_register();
	rtw_inetaddr_notifier_register();

	ret = platform_driver_register(&axi_drvpriv.rtw_axi_drv);

	if (ret != 0) {
		axi_drvpriv.drv_registered = _FALSE;
		rtw_suspend_lock_uninit();
		rtw_drv_proc_deinit();
		rtw_nlrtw_deinit();
		rtw_ndev_notifier_unregister();
		rtw_inetaddr_notifier_unregister();
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

	rtw_suspend_lock_uninit();
	rtw_drv_proc_deinit();
	rtw_nlrtw_deinit();
	rtw_ndev_notifier_unregister();
	rtw_inetaddr_notifier_unregister();

	RTW_PRINT("module exit success\n");

	rtw_mstat_dump(RTW_DBGDUMP);
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
