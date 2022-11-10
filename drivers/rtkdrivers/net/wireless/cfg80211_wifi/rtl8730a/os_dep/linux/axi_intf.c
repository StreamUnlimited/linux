/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#include <hal_data.h>

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>

#ifndef CONFIG_AXI_HCI
#error "CONFIG_AXI_HCI shall be on!\n"
#endif


#ifdef CONFIG_80211N_HT
extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;/* for enable tx_ampdu */
#endif

extern int pm_netdev_open(struct net_device *pnetdev, u8 bnormal);
int rtw_resume_process(_adapter *padapter);

#ifdef CONFIG_PM
static int rtw_axi_suspend(struct platform_device *pdev, pm_message_t state);
static int rtw_axi_resume(struct platform_device *pdev);
#endif

static int rtw_drv_init(struct platform_device *pdev);
static int rtw_dev_remove(struct platform_device *pdev);
static void rtw_dev_shutdown(struct platform_device *pdev);

/* Match table for of_platform binding */
static const struct of_device_id rtw_axi_of_match[] = {
	{ .compatible = "realtek,rtl8730e_nic", },
	{},
};

struct axi_drv_priv {
	struct platform_driver rtw_axi_drv;
	int drv_registered;
};


static struct axi_drv_priv axi_drvpriv = {
	.rtw_axi_drv.probe = rtw_drv_init,
	.rtw_axi_drv.remove = rtw_dev_remove,
	.rtw_axi_drv.shutdown = rtw_dev_shutdown,
#ifdef CONFIG_PM
	.rtw_axi_drv.suspend = rtw_axi_suspend,
	.rtw_axi_drv.resume = rtw_axi_resume,
#endif
	.rtw_axi_drv.driver = {
		.name = DRV_NAME,
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
	_adapter *adapter = dvobj_get_primary_adapter(dvobj);

	if (dvobj->irq_enabled == 0) {
		return IRQ_HANDLED;
	}

	if (rtw_hal_interrupt_handler(adapter) == _FAIL) {
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
 * entity of struct aipc_ch_ops. It will associate the channel_recv to recv
 */
static struct aipc_ch_ops rtw_fw_ipc_ops = {
	.channel_recv = rtw_fw_ipc_recv,
};


int axi_alloc_irq(struct dvobj_priv *dvobj)
{
	int err = _SUCCESS;
	struct platform_device *pdev = dvobj->pdev;

	/* get the irq nember from device tree */
	dvobj->irq_dma = platform_get_irq(pdev, 0);
	if (dvobj->irq_dma < 0) {
		RTW_INFO("Error allocating DMA IRQ %d", dvobj->irq_dma);
		err = _FAIL;
		goto exit;
	}

	dvobj->irq_protocol = platform_get_irq(pdev, 1);
	if (dvobj->irq_protocol < 0) {
		RTW_INFO("Error allocating protocol IRQ %d", dvobj->irq_protocol);
		err = _FAIL;
		goto exit;
	}

#if defined(IRQF_SHARED)
	err = request_irq(dvobj->irq_dma, &rtw_axi_interrupt, IRQF_SHARED, DRV_NAME, dvobj);
#else
	err = request_irq(dvobj->irq_dma, &rtw_axi_interrupt, SA_SHIRQ, DRV_NAME, dvobj);
#endif
	if (err) {
		RTW_INFO("Error allocating IRQ %d", dvobj->irq_dma);
		goto exit;
	} else {
		RTW_INFO("Request_irq OK, IRQ %d\n", dvobj->irq_dma);
	}

#if defined(IRQF_SHARED)
	err = request_irq(dvobj->irq_protocol, &rtw_axi_interrupt, IRQF_SHARED, DRV_NAME, dvobj);
#else
	err = request_irq(dvobj->irq_protocol, &rtw_axi_interrupt, SA_SHIRQ, DRV_NAME, dvobj);
#endif
	if (err) {
		RTW_INFO("Error allocating IRQ %d", dvobj->irq_protocol);
		free_irq(dvobj->irq_dma, dvobj);
	} else {
		dvobj->irq_alloc = 1;
		RTW_INFO("Request_irq OK, IRQ %d\n", dvobj->irq_protocol);
	}

exit:
	return err ? _FAIL : _SUCCESS;
}

static struct dvobj_priv *axi_dvobj_init(struct platform_device *pdev)
{
	int err;
	u32 status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	struct resource *res_mem = NULL;
	struct device_node *sys_node = NULL, *ocp_node = NULL;
	unsigned long pmem_len = 0;
	struct resource res_sys = {0};
	int i;


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

	dvobj->pdev = pdev;
	platform_set_drvdata(pdev, dvobj);

	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_mem) {
		RTW_ERR("Can't get axi IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_mem->end - res_mem->start + 1;

	dvobj->axi_mem_start = NULL;
	dvobj->axi_mem_start = (unsigned long)devm_ioremap_resource(&pdev->dev, res_mem);
	if (!dvobj->axi_mem_start) {
		RTW_ERR("Can't map CTRL mem\n");
		goto exit;
	}
	dvobj->axi_mem_end = dvobj->axi_mem_start + pmem_len;

	RTW_INFO("Memory mapped space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
		 (unsigned long)res_mem->start, pmem_len, dvobj->axi_mem_start);

	if (of_address_to_resource(sys_node, 0, &res_sys)) {
		RTW_ERR("Can't get sys IORESOURCE_MEM\n");
		goto free_dvobj;
	}

	pmem_len = res_sys.end - res_sys.start + 1;

	dvobj->axi_sys_mem_start = NULL;
	dvobj->axi_sys_mem_start = (unsigned long)ioremap(res_sys.start, pmem_len);
	if (!dvobj->axi_sys_mem_start) {
		RTW_ERR("Can't map CTRL mem\n");
		goto exit;
	}
	dvobj->axi_sys_mem_end = dvobj->axi_sys_mem_start + pmem_len;

	RTW_INFO("Memory mapped sys space start: 0x%08lx len:%08lx, after map:0x%08lx\n",
		 (unsigned long)res_sys.start, pmem_len, dvobj->axi_sys_mem_start);

	dvobj->fw_ipc = ameba_ipc_alloc_ch(sizeof(struct dvobj_priv *));
	if (!dvobj->fw_ipc) {
		RTW_ERR("no memory for ipc channel\n");
		goto iounmap;
	}

	/* initialize the ipc channel */
	dvobj->fw_ipc->port_id = AIPC_PORT_LP;
	dvobj->fw_ipc->ch_id = 1; /* <  NP -->  LP FW Info channel 1 */
	dvobj->fw_ipc->ch_config = AIPC_CONFIG_NOTHING;
	dvobj->fw_ipc->ops = &rtw_fw_ipc_ops;
	dvobj->fw_ipc->priv_data = dvobj;

	if (ameba_ipc_channel_register(dvobj->fw_ipc)) {
		RTW_ERR("register ipc channel failed.\n");
		goto free_ipc;
	}

	/*step 1-1., decide the chip_type via device info*/
	dvobj->interface_type = RTW_AXI;
#ifdef CONFIG_RTL8730A
	dvobj->chip_type = RTL8730A;
	dvobj->HardwareType = HARDWARE_TYPE_RTL8730AA;

#endif

	status = _SUCCESS;

free_ipc:
	if (status != _SUCCESS && (dvobj->fw_ipc)) {
		kfree(dvobj->fw_ipc);
	}

iounmap:
	if (status != _SUCCESS && dvobj->axi_mem_start != 0) {
		if (dvobj->axi_sys_mem_start) {
			iounmap(dvobj->axi_sys_mem_start);
		}

		if (dvobj->axi_mem_start) {
			devm_iounmap(&pdev->dev, (void *)dvobj->axi_mem_start);
		}
		dvobj->axi_mem_start = 0;
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

	platform_set_drvdata(pdev, NULL);
	if (dvobj) {
		if (dvobj->fw_ipc) {
			ameba_ipc_channel_unregister(dvobj->fw_ipc);
			dvobj->fw_ipc = NULL;
		}

		if (dvobj->irq_alloc) {
			free_irq(dvobj->irq_dma, dvobj);
			free_irq(dvobj->irq_protocol, dvobj);
			dvobj->irq_alloc = 0;
		}

		if (dvobj->axi_sys_mem_start != 0) {
			iounmap((void *)dvobj->axi_sys_mem_start);
			dvobj->axi_sys_mem_start = 0;
		}

		if (dvobj->axi_mem_start != 0) {
			devm_iounmap(&pdev->dev, (void *)dvobj->axi_mem_start);
			dvobj->axi_mem_start = 0;
		}

		devobj_deinit(dvobj);
		dvobj = NULL;
	}
}


u8 rtw_set_hal_ops(_adapter *padapter)
{
	/* alloc memory for HAL DATA */
	if (rtw_hal_data_init(padapter) == _FAIL) {
		return _FAIL;
	}

#ifdef CONFIG_RTL8730A
	if (rtw_get_chip_type(padapter) == RTL8730A) {
		rtl8730aa_set_hal_ops(padapter);
	}
#endif

	if (rtw_hal_ops_check(padapter) == _FAIL) {
		return _FAIL;
	}

	if (hal_spec_init(padapter) == _FAIL) {
		return _FAIL;
	}

	return _SUCCESS;
}

void axi_set_intf_ops(_adapter *padapter, struct _io_ops *pops)
{
#if defined(CONFIG_RTL8730A)
	if (rtw_get_chip_type(padapter) == RTL8730A) {
		rtl8730aa_set_intf_ops(pops);
	}
#endif

}

static void axi_intf_start(_adapter *padapter)
{
	u8 en_sw_bcn = _TRUE;

	RTW_INFO("+axi_intf_start\n");

	/* Enable hw interrupt */
	rtw_hal_enable_interrupt(padapter);
	rtw_hal_set_hwreg(padapter, HW_VAR_ENSWBCN, &en_sw_bcn);

#ifdef CONFIG_AXI_TX_POLLING
	rtw_tx_poll_init(padapter);
#endif

	RTW_INFO("-axi_intf_start\n");
}
static void rtw_mi_axi_tasklets_kill(_adapter *padapter)
{
	int i;
	_adapter *iface;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			tasklet_kill(&(padapter->recvpriv.recv_tasklet));
			tasklet_kill(&(padapter->recvpriv.irq_prepare_beacon_tasklet));
			tasklet_kill(&(padapter->xmitpriv.xmit_tasklet));
		}
	}
}

static void axi_intf_stop(_adapter *padapter)
{


	/* Disable hw interrupt */
	if (!rtw_is_surprise_removed(padapter)) {
		/* device still exists, so driver can do i/o operation */
		rtw_hal_disable_interrupt(padapter);
		rtw_mi_axi_tasklets_kill(padapter);

		rtw_hal_set_hwreg(padapter, HW_VAR_PCIE_STOP_TX_DMA, 0);

		rtw_hal_irp_reset(padapter);

	} else {
		/* Clear irq_enabled to prevent handle interrupt function. */
		adapter_to_dvobj(padapter)->irq_enabled = 0;
	}

#ifdef CONFIG_AXI_TX_POLLING
	rtw_tx_poll_timer_cancel(padapter);
#endif

}

static void disable_ht_for_spec_devid(const struct pci_device_id *pdid)
{

}

#ifdef CONFIG_PM
static int rtw_axi_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	struct dvobj_priv *dvobj = pci_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);

	ret = rtw_suspend_common(padapter);
	if (ret != 0) {
		RTW_INFO("%s Failed on rtw_suspend_common (%d)\n", __func__, ret);
		goto exit;
	}

exit:
	return ret;

}

int rtw_resume_process(_adapter *padapter)
{
	return rtw_resume_common(padapter);
}

static int rtw_axi_resume(struct platform_device *pdev)
{
	struct dvobj_priv *dvobj = pci_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);
	struct net_device *pnetdev = padapter->pnetdev;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	int	err = 0;

	if (pwrpriv->wowlan_mode || pwrpriv->wowlan_ap_mode) {
		rtw_resume_lock_suspend();
		err = rtw_resume_process(padapter);
		rtw_resume_unlock_suspend();
	} else {
#ifdef CONFIG_RESUME_IN_WORKQUEUE
		rtw_resume_in_workqueue(pwrpriv);
#endif
		if (rtw_is_earlysuspend_registered(pwrpriv)) {
			/* jeff: bypass resume here, do in late_resume */
			rtw_set_do_late_resume(pwrpriv, _TRUE);
		} else {
			rtw_resume_lock_suspend();
			err = rtw_resume_process(padapter);
			rtw_resume_unlock_suspend();
		}
	}
exit:

	return err;
}
#endif/* CONFIG_PM */

_adapter *rtw_axi_primary_adapter_init(struct dvobj_priv *dvobj, struct platform_device *pdev)
{
	_adapter *padapter = NULL;
	int status = _FAIL;

	padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (padapter == NULL) {
		goto exit;
	}

	if (loadparam(padapter) != _SUCCESS) {
		goto free_adapter;
	}

	padapter->dvobj = dvobj;

	rtw_set_drv_stopped(padapter);/*init*/

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

	/* set adapter_type/iface type for primary padapter */
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;
#ifdef CONFIG_MI_WITH_MBSSID_CAM
	padapter->hw_port = HW_PORT0;
#else
	padapter->hw_port = HW_PORT0;
#endif

	if (rtw_init_io_priv(padapter, axi_set_intf_ops) == _FAIL) {
		goto free_adapter;
	}

	/* step 2.	hook HalFunc, allocate HalData */
	/* hal_set_hal_ops(padapter); */
	if (rtw_set_hal_ops(padapter) == _FAIL) {
		goto free_hal_data;
	}

	/* step 3. */
	padapter->intf_start = &axi_intf_start;
	padapter->intf_stop = &axi_intf_stop;

	/* .3 */
	rtw_hal_read_chip_version(padapter);

	/* .4 */
	rtw_hal_chip_configure(padapter);

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_Initialize(padapter);
#endif
	rtw_btcoex_wifionly_initialize(padapter);

	/* step 4. read efuse/eeprom data and get mac_addr */
	if (rtw_hal_read_chip_info(padapter) == _FAIL) {
		goto free_hal_data;
	}

	/* step 5. */
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		goto free_hal_data;
	}

	if (rtw_hal_inirp_init(padapter) == _FAIL) {
		goto free_hal_data;
	}

	rtw_macaddr_cfg(adapter_mac_addr(padapter),  get_hal_mac_addr(padapter));

#ifdef CONFIG_MI_WITH_MBSSID_CAM
	rtw_mbid_camid_alloc(padapter, adapter_mac_addr(padapter));
#endif
#ifdef CONFIG_P2P
	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));
#endif /* CONFIG_P2P */


	rtw_hal_disable_interrupt(padapter);

//	rtw_hal_power_off(padapter);

	RTW_INFO("bDriverStopped:%s, bSurpriseRemoved:%s, bup:%d, hw_init_completed:%s\n"
		 , rtw_is_drv_stopped(padapter) ? "True" : "False"
		 , rtw_is_surprise_removed(padapter) ? "True" : "False"
		 , padapter->bup
		 , rtw_is_hw_init_completed(padapter) ? "True" : "False"
		);

	status = _SUCCESS;

free_hal_data:

	if (status != _SUCCESS && padapter->HalData) {
		rtw_hal_free_data(padapter);
	}

free_adapter:
	if (status != _SUCCESS && padapter) {
#ifdef RTW_HALMAC
		rtw_halmac_deinit_adapter(dvobj);
#endif
		rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_axi_primary_adapter_deinit(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	/*	padapter->intf_stop(padapter); */

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);
	}

#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
		free_mlme_ap_info(padapter);
#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(padapter);
#endif
	}
#endif

	/*rtw_cancel_all_timer(padapte);*/
#ifdef CONFIG_WOWLAN
	adapter_to_pwrctl(padapter)->wowlan_mode = _FALSE;
#endif /* CONFIG_WOWLAN */
	rtw_dev_unload(padapter);

	RTW_INFO("%s, hw_init_completed=%s\n", __func__, rtw_is_hw_init_completed(padapter) ? "_TRUE" : "_FALSE");

	rtw_hal_inirp_deinit(padapter);
	rtw_free_drv_sw(padapter);

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndev_free(padapter);

#ifdef RTW_HALMAC
	rtw_halmac_deinit_adapter(adapter_to_dvobj(padapter));
#endif /* RTW_HALMAC */

	rtw_vmfree((u8 *)padapter, sizeof(_adapter));

#ifdef CONFIG_PLATFORM_RTD2880B
	RTW_INFO("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_drv_init(struct platform_device *pdev)
{
	int i, err = -ENODEV;

	int status = _FAIL;
	_adapter *padapter = NULL;
	struct dvobj_priv *dvobj;

	/* RTW_INFO("+rtw_drv_init\n"); */

	/* step 0. */
	disable_ht_for_spec_devid(NULL);

	/* Initialize dvobj_priv */
	dvobj = axi_dvobj_init(pdev);
	if (dvobj == NULL) {
		goto exit;
	}

	/* Initialize primary adapter */
	padapter = rtw_axi_primary_adapter_init(dvobj, pdev);
	if (padapter == NULL) {
		RTW_INFO("rtw_axi_primary_adapter_init Failed!\n");
		goto free_dvobj;
	}

	/* Initialize virtual interface */
#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->registrypriv.virtual_iface_num > (CONFIG_IFACE_NUMBER - 1)) {
		padapter->registrypriv.virtual_iface_num = (CONFIG_IFACE_NUMBER - 1);
	}

	for (i = 0; i < padapter->registrypriv.virtual_iface_num; i++) {
		if (rtw_drv_add_vir_if(padapter, axi_set_intf_ops) == NULL) {
			RTW_INFO("rtw_drv_add_iface failed! (%d)\n", i);
			goto free_if_vir;
		}
	}
#endif

#ifdef CONFIG_GLOBAL_UI_PID
	if (ui_pid[1] != 0) {
		RTW_INFO("ui_pid[1]:%d\n", ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	/* dev_alloc_name && register_netdev */
	if (rtw_os_ndevs_init(dvobj) != _SUCCESS) {
		goto free_if_vir;
	}

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

	/* alloc irq */
	if (axi_alloc_irq(dvobj) != _SUCCESS) {
		goto os_ndevs_deinit;
	}

	RTW_INFO("-871x_drv - drv_init, success!\n");

	status = _SUCCESS;

os_ndevs_deinit:
	if (status != _SUCCESS) {
		rtw_os_ndevs_deinit(dvobj);
	}
free_if_vir:
	if (status != _SUCCESS) {
#ifdef CONFIG_CONCURRENT_MODE
		rtw_drv_stop_vir_ifaces(dvobj);
		rtw_drv_free_vir_ifaces(dvobj);
#endif
	}

	if (status != _SUCCESS && padapter) {
		rtw_axi_primary_adapter_deinit(padapter);
	}

free_dvobj:
	if (status != _SUCCESS) {
		axi_dvobj_deinit(pdev);
	}
exit:
	return status == _SUCCESS ? 0 : -ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
/* rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both */
static int rtw_dev_remove(struct platform_device *pdev)
{
	struct dvobj_priv *pdvobjpriv = platform_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(pdvobjpriv);
	struct net_device *pnetdev = padapter->pnetdev;

	if (pdvobjpriv->processing_dev_remove == _TRUE) {
		RTW_WARN("%s-line%d: Warning! device has been removed!\n", __func__, __LINE__);
		return -1;
	}

	RTW_INFO("+rtw_dev_remove\n");

	pdvobjpriv->processing_dev_remove = _TRUE;

	if (unlikely(!padapter)) {
		return -1;
	}

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndevs_unregister(pdvobjpriv);


#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(dvobj_to_pwrctl(pdvobjpriv));
#endif

	if (GET_HAL_DATA(padapter)->bFWReady == _TRUE) {
		rtw_pm_set_ips(padapter, IPS_NONE);
		rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

		LeaveAllPowerSaveMode(padapter);
	}

	rtw_set_drv_stopped(padapter);	/*for stop thread*/
	rtw_stop_cmd_thread(padapter);
#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_stop_vir_ifaces(pdvobjpriv);
#endif

#ifdef CONFIG_BT_COEXIST
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	if (GET_HAL_DATA(padapter)->EEPROMBluetoothCoexist) {
		rtw_btcoex_close_socket(padapter);
	}
#endif
	rtw_btcoex_HaltNotify(padapter);
#endif

	rtw_axi_primary_adapter_deinit(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_free_vir_ifaces(pdvobjpriv);
#endif

	axi_dvobj_deinit(pdev);

	RTW_INFO("-r871xu_dev_remove, done\n");

	return 0;
}

static void rtw_dev_shutdown(struct platform_device *pdev)
{
	struct dvobj_priv *pdvobjpriv = platform_get_drvdata(pdev);
	_adapter *padapter = dvobj_get_primary_adapter(pdvobjpriv);
	struct net_device *pnetdev = padapter->pnetdev;

#ifdef CONFIG_RTL8730A
	if (IS_HARDWARE_TYPE_8730AA(padapter)) {
		u8 u1Tmp;

		u1Tmp = PlatformEFIORead1Byte(padapter, 0x75 /*REG_HCI_OPT_CTRL_8730A+1*/);
		PlatformEFIOWrite1Byte(padapter, 0x75 /*REG_HCI_OPT_CTRL_8730A+1*/, (u1Tmp | BIT0)); /*Disable USB Suspend Signal*/
	}
#endif

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
#ifdef CONFIG_PLATFORM_CMAP_INTFS
	cmap_intfs_init();
#endif
	rtw_ndev_notifier_register();
	rtw_inetaddr_notifier_register();

	ret = platform_driver_register(&axi_drvpriv.rtw_axi_drv);

	if (ret != 0) {
		axi_drvpriv.drv_registered = _FALSE;
		rtw_suspend_lock_uninit();
		rtw_drv_proc_deinit();
		rtw_nlrtw_deinit();
#ifdef CONFIG_PLATFORM_CMAP_INTFS
		cmap_intfs_deinit();
#endif
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
#ifdef CONFIG_PLATFORM_CMAP_INTFS
	cmap_intfs_deinit();
#endif
	rtw_ndev_notifier_unregister();
	rtw_inetaddr_notifier_unregister();

	RTW_PRINT("module exit success\n");

	rtw_mstat_dump(RTW_DBGDUMP);
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
