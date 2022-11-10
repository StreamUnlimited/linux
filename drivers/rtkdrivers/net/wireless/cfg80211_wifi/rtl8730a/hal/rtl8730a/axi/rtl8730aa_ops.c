/******************************************************************************
 *
 * Copyright(c) 2015 - 2017 Realtek Corporation.
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

#define _AXI_OPS_OS_C_

#include <drv_types.h>		/* PADAPTER, basic_types.h and etc. */
#include <hal_data.h>		/* HAL_DATA_TYPE, GET_HAL_DATA() and etc. */
#include <hal_intf.h>		/* struct hal_ops */
#include "../rtl8730a.h"
#include "rtl8730aa.h"
#include "rtl8730a_spec.h"
#include "rtl8730a_recv.h"
#include "rtw_mi.h"

static void init_bd_ring_var(_adapter *padapter)
{
	struct recv_priv *r_priv = &padapter->recvpriv;
	struct xmit_priv *t_priv = &padapter->xmitpriv;
	u8 i = 0;

	for (i = 0; i < HW_QUEUE_ENTRY; i++) {
		t_priv->txringcount[i] = TX_BD_NUM_8730AA;
	}

	/*
	 * we just alloc 2 desc for beacon queue,
	 * because we just need first desc in hw beacon.
	 */
	t_priv->txringcount[BCN_QUEUE_INX] = TX_BD_NUM_8730AA_BCN;

	t_priv->txringcount[TXCMD_QUEUE_INX] = TX_BD_NUM_8730AA_CMD;
	t_priv->txringcount[BE_QUEUE_INX] = TX_BD_NUM_8730AA_BEQ;

	/*
	 * BE queue need more descriptor for performance consideration
	 * or, No more tx desc will happen, and may cause mac80211 mem leakage.
	 */
	r_priv->rxbuffersize = MAX_RECVBUF_SZ;
	r_priv->rxringcount = RX_BD_NUM_8730AA;
}

static void rtl8730aa_reset_bd(_adapter *padapter)
{
	_irqL	irqL;
	struct xmit_priv *t_priv = &padapter->xmitpriv;
	struct recv_priv *r_priv = &padapter->recvpriv;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct xmit_buf	*pxmitbuf = NULL;
	u8 *tx_bd, *rx_bd;
	int i, rx_queue_idx;
	dma_addr_t mapping;

	InitMAC_TRXBD_8730AA(padapter);

	for (rx_queue_idx = 0; rx_queue_idx < 1; rx_queue_idx++) {
		if (r_priv->rx_ring[rx_queue_idx].buf_desc) {
			rx_bd = NULL;
			for (i = 0; i < r_priv->rxringcount; i++) {
				rx_bd = (u8 *)
					&r_priv->rx_ring[rx_queue_idx].buf_desc[i];
			}
			r_priv->rx_ring[rx_queue_idx].idx = 0;
		}
	}

	_enter_critical(&pdvobjpriv->irq_th_lock, &irqL);
	for (i = 0; i < AXI_MAX_TX_QUEUE_COUNT; i++) {
		if (t_priv->tx_ring[i].buf_desc) {
			struct rtw_tx_ring *ring = &t_priv->tx_ring[i];

			while (ring->qlen) {
				tx_bd = (u8 *)(&ring->buf_desc[ring->idx]);
				SET_TX_BD_OWN(tx_bd, 0);

				if (i != BCN_QUEUE_INX)
					ring->idx =
						(ring->idx + 1) % ring->entries;

				pxmitbuf = rtl8730aa_dequeue_xmitbuf(ring);
				if (pxmitbuf) {
					mapping = GET_TX_BD_PHYSICAL_ADDR0_LOW(tx_bd);
					dma_unmap_single(&pdvobjpriv->pdev->dev,
							 mapping,
							 pxmitbuf->len, DMA_TO_DEVICE);
					rtw_free_xmitbuf(t_priv, pxmitbuf);
				} else {
					RTW_INFO("%s(): qlen(%d) is not zero, but have xmitbuf in pending queue\n",
						 __func__, ring->qlen);
					break;
				}
			}
			ring->idx = 0;
		}
	}
	_exit_critical(&pdvobjpriv->irq_th_lock, &irqL);
}

static void intf_chip_configure(PADAPTER padapter)
{

	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(pdvobjpriv);

	pwrpriv->reg_rfoff = 0;
	pwrpriv->rfoff_reason = 0;
}


static void rtl8730aa_enable_dma(PADAPTER Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);

	rtw_write32(Adapter, REG_AXI_INTERRUPT_MASK, pHalData->IntrMask[1] & 0xFFFFFFFF);
}

static void rtl8730aa_enable_interrupt(PADAPTER Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(Adapter);

	pdvobjpriv->irq_enabled = 1;

	rtw_write32(Adapter, REG_AXI_INTERRUPT_MASK, pHalData->IntrMask[1] & 0xFFFFFFFF);
	rtw_write32(Adapter, REG_HIMR0, pHalData->IntrMask[0] & 0xFFFFFFFF);
	rtw_write32(Adapter, REG_HEMR, pHalData->IntrMask[2] & 0xFFFFFFFF);
	rtw_write32(Adapter, REG_HIMR2, pHalData->IntrMask[3] & 0xFFFFFFFF);
}


static void rtl8730aa_clear_interrupt(PADAPTER Adapter)
{
	u32 u32b;
	HAL_DATA_TYPE   *pHalData = GET_HAL_DATA(Adapter);

	u32b = rtw_read32(Adapter, REG_AXI_INTERRUPT);
	rtw_write32(Adapter, REG_AXI_INTERRUPT, u32b);
	pHalData->IntArray[1] = 0;

	u32b = rtw_read32(Adapter, REG_HISR0);
	rtw_write32(Adapter, REG_HISR0, u32b);
	pHalData->IntArray[0] = 0;

	u32b = rtw_read32(Adapter, REG_HESR);
	rtw_write32(Adapter, REG_HESR, u32b);
	pHalData->IntArray[2] = 0;

	u32b = rtw_read32(Adapter, REG_HISR2);
	rtw_write32(Adapter, REG_HISR2, u32b);
	pHalData->IntArray[3] = 0;
}

static void rtl8730aa_disable_dma(PADAPTER Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);

	rtw_write32(Adapter, REG_AXI_INTERRUPT_MASK, 0);
}

static void rtl8730aa_disable_interrupt(PADAPTER Adapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(Adapter);

	rtw_write32(Adapter, REG_AXI_INTERRUPT_MASK, 0x0);
	rtw_write32(Adapter, REG_HIMR0, 0x0);
	rtw_write32(Adapter, REG_HEMR, 0x0);
	rtw_write32(Adapter, REG_HIMR2, 0x0);
	pdvobjpriv->irq_enabled = 0;
}

static BOOLEAN rtl8730aa_InterruptRecognized(PADAPTER Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	BOOLEAN bRecognized = _FALSE;

	rtl8730aa_disable_interrupt(Adapter);

	pHalData->IntArray[0] = rtw_read32(Adapter, REG_HISR0);
	pHalData->IntArray[0] &= pHalData->IntrMask[0];

	if (pHalData->IntArray[0] & BIT_HISR1_INT) {
		pHalData->IntArray[2] = rtw_read32(Adapter, REG_HESR);
		pHalData->IntArray[2] &= pHalData->IntrMask[2];
		/*hesr has to be cleared before hisr0, otherwise hisr0[BIT_HISR1_INT] will be 1 again*/
		rtw_write32(Adapter, REG_HESR, pHalData->IntArray[2]);
	}

	if (pHalData->IntArray[0] & BIT_HISR2_INT) {
		pHalData->IntArray[3] = rtw_read32(Adapter, REG_HISR2);
		pHalData->IntArray[3] &= pHalData->IntrMask[3];
		/*hisr2 has to be cleared before hisr0, otherwise hisr0[BIT_HISR2_INT] will be 1 again*/
		rtw_write32(Adapter, REG_HISR2, pHalData->IntArray[3]);
	}

	rtw_write32(Adapter, REG_HISR0, pHalData->IntArray[0]);
	pHalData->IntArray[0] &= ~(BIT_HISR1_INT | BIT_HISR2_INT);

	/*Interrput form AXI*/
	pHalData->IntArray[1] = rtw_read32(Adapter, REG_AXI_INTERRUPT);
	pHalData->IntArray[1] &= pHalData->IntrMask[1];
	rtw_write32(Adapter, REG_AXI_INTERRUPT, pHalData->IntArray[1]);

	if (((pHalData->IntArray[0]) & pHalData->IntrMask[0]) != 0
	    || ((pHalData->IntArray[1]) & pHalData->IntrMask[1]) != 0
	    || ((pHalData->IntArray[2]) & pHalData->IntrMask[2]) != 0
	    || ((pHalData->IntArray[3]) & pHalData->IntrMask[3]) != 0) {
		bRecognized = _TRUE;
	}

	rtl8730aa_enable_interrupt(Adapter);

	return bRecognized;
}

void rtl8730aa_update_interrupt_mask(PADAPTER Adapter, u32 *pAddMSR, u32 *pRemoveMSR)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);
	u8  LoopIndex = 0;

	rtl8730aa_disable_interrupt(Adapter);

	for (LoopIndex = 0; LoopIndex < 4; LoopIndex++) {
		if (pAddMSR && pAddMSR[LoopIndex]) {
			pHalData->IntrMask[LoopIndex] |= pAddMSR[LoopIndex];
		}

		if (pRemoveMSR && pRemoveMSR[LoopIndex]) {
			pHalData->IntrMask[LoopIndex] &= (~pRemoveMSR[LoopIndex]);
		}
	}

	rtl8730aa_enable_interrupt(Adapter);
}

static void rtl8730aa_bcn_handler(PADAPTER Adapter, u32 handled[])
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);
	PADAPTER bcn_adapter = rtw_mi_get_ap_adapter(Adapter);

	if ((pHalData->IntArray[0] & (BIT_TXBCN1_ERR_INT | BIT_TXBCN1_OK_INT)
#ifdef CONFIG_CONCURRENT_MODE
	     || (pHalData->IntArray[2] & (BIT_TXBCNERR9_INT | BIT_TXBCNOK9_INT))
#endif
	    )) {
#ifndef CONFIG_AXI_BCN_POLLING
		/* Modify for MI temporary,
		 * this processor cannot apply to multi-ap */

		if (bcn_adapter->xmitpriv.beaconDMAing) {
			bcn_adapter->xmitpriv.beaconDMAing = _FAIL;
			rtl8730aa_tx_isr(Adapter, BCN_QUEUE_INX);
		}
#endif
		handled[0] |= (pHalData->IntArray[0] & (BIT_TXBCN1_ERR_INT \
							| BIT_TXBCN1_OK_INT));
#ifdef CONFIG_CONCURRENT_MODE
		handled[2] |= (pHalData->IntArray[2] & (BIT_TXBCNERR9_INT \
							| BIT_TXBCNOK9_INT));
#endif
	}

	if (pHalData->IntArray[2] & (BIT_BCNERLY0_INT
#ifdef CONFIG_CONCURRENT_MODE
				     | BIT_BCNERLY8_INT
#endif
				    )) {
#ifndef CONFIG_AXI_BCN_POLLING
		_tasklet *bcn_tasklet;

		if (bcn_adapter->mlmepriv.update_bcn == _TRUE) {
			bcn_tasklet = &bcn_adapter->recvpriv.irq_prepare_beacon_tasklet;
			tasklet_hi_schedule(bcn_tasklet);
		}
#endif
		handled[2] |= (pHalData->IntArray[2] & (BIT_BCNERLY0_INT
#ifdef CONFIG_CONCURRENT_MODE
							| BIT_BCNERLY8_INT
#endif
						       ));
	}
}

static void rtl8730aa_rx_handler(PADAPTER Adapter, u32 handled[])
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);

	if ((pHalData->IntArray[0] & (BIT_RXFF_FULL_INT_EN)) ||
	    (pHalData->IntArray[1] & (IMR_ROK_8730A | IMR_RDU_8730A))) {
		pHalData->IntrMask[0] &= (~(BIT_RXFF_FULL_INT_EN));
		pHalData->IntrMask[1] &= (~(IMR_ROK_8730A | IMR_RDU_8730A));
		rtw_write32(Adapter, REG_HIMR0, pHalData->IntrMask[0]);
		rtw_write32(Adapter, REG_AXI_INTERRUPT_MASK, pHalData->IntrMask[1]);

		if ((pHalData->IntArray[1] & (IMR_RDU_8730A))\
		    || (pHalData->IntArray[0] & BIT_RXFF_FULL_INT)) {
			RTW_WARN("[RXFOVW]\n");
		}

		tasklet_hi_schedule(&Adapter->recvpriv.recv_tasklet);
		handled[0] |= (pHalData->IntArray[0] & BIT_RXFF_FULL_INT_EN);
		handled[1] |= (pHalData->IntArray[1] & (IMR_ROK_8730A | IMR_RDU_8730A));
	}
}

static void rtl8730aa_tx_handler(PADAPTER Adapter, u32 events[], u32 handled[])
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);

	if (events[1] & IMR_MGNTDOK_8730A) {
		rtl8730aa_tx_isr(Adapter, MGT_QUEUE_INX);
		handled[1] |= IMR_MGNTDOK_8730A;
	}

	if (events[1] & IMR_H0DOK_8730A) {
		rtl8730aa_tx_isr(Adapter, HIGH_QUEUE_INX);
		handled[1] |= IMR_H0DOK_8730A;
	}

	if (events[1] & IMR_BKDOK_8730A) {
		rtl8730aa_tx_isr(Adapter, BK_QUEUE_INX);
		handled[1] |= IMR_BKDOK_8730A;
	}

	if (events[1] & IMR_BEDOK_8730A) {
		rtl8730aa_tx_isr(Adapter, BE_QUEUE_INX);
		handled[1] |= IMR_BEDOK_8730A;
	}

	if (events[1] & IMR_VIDOK_8730A) {
		rtl8730aa_tx_isr(Adapter, VI_QUEUE_INX);
		handled[1] |= IMR_VIDOK_8730A;
	}

	if (events[1] & IMR_VODOK_8730A) {
		rtl8730aa_tx_isr(Adapter, VO_QUEUE_INX);
		handled[1] |= IMR_VODOK_8730A;
	}
}

static void rtl8730aa_cmd_handler(PADAPTER Adapter, u32 handled[])
{
#if 0
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);

	if (pHalData->IntArray[1] & IMR_H2COK_8730A) {
		handled[1] |= IMR_H2COK_8730A;
	}
#endif
}

static s32 rtl8730aa_interrupt(PADAPTER Adapter)
{
	_irqL irqL;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(Adapter);
	struct xmit_priv *t_priv = &Adapter->xmitpriv;
	int ret = _SUCCESS;
	u32 handled[4] = {0};

	_enter_critical(&pdvobjpriv->irq_th_lock, &irqL);

	/* read ISR: 4/8bytes */
	if (rtl8730aa_InterruptRecognized(Adapter) == _FALSE) {
		RTW_ERR("Interrupt Recognized failed\n");
		ret = _FAIL;
		goto done;
	}

	/* <1> beacon related */
	rtl8730aa_bcn_handler(Adapter, handled);

	/* <2> Rx related */
	rtl8730aa_rx_handler(Adapter, handled);

	/* <3> Tx related */
	rtl8730aa_tx_handler(Adapter, pHalData->IntArray, handled);

	if (pHalData->IntArray[0] & BIT_TXFF_FIFO_INT) {
		if (printk_ratelimit()) {
			RTW_WARN("[TXFOVW]\n");
		}
		handled[0] |= BIT_TXFF_FIFO_INT;
	}

	/* <4> Cmd related */
	rtl8730aa_cmd_handler(Adapter, handled);

#ifdef CONFIG_LPS_LCLK
	if (pHalData->IntArray[0] & BIT_CPWM) {
		struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(Adapter);

		_set_workitem(&(pwrpriv->cpwm_event));
		handled[0] |= BIT_CPWM;
	}
#endif

	if ((pHalData->IntArray[0] & (~handled[0])) ||
	    (pHalData->IntArray[1] & (~handled[1])) ||
	    (pHalData->IntArray[2] & (~handled[2])) ||
	    (pHalData->IntArray[3] & (~handled[3]))) {

		if (printk_ratelimit()) {
			RTW_WARN("Unhandled ISR = 0x%08x, 0x%08x, 0x%08x, 0x%08x.\n",
				 (pHalData->IntArray[0] & (~handled[0])),
				 (pHalData->IntArray[1] & (~handled[1])),
				 (pHalData->IntArray[2] & (~handled[2])),
				 (pHalData->IntArray[3] & (~handled[3])));
		}
	}
done:
	pHalData->IntArray[0] = 0;
	pHalData->IntArray[1] = 0;
	pHalData->IntArray[2] = 0;
	pHalData->IntArray[3] = 0;

	_exit_critical(&pdvobjpriv->irq_th_lock, &irqL);
	return ret;
}

static void rtl8730aa_unmap_beacon_icf(PADAPTER Adapter)
{
	_adapter *pri_adapter = GET_PRIMARY_ADAPTER(Adapter);
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(pri_adapter);
	struct xmit_priv	*pxmitpriv = &Adapter->xmitpriv;
	struct xmit_buf	*pxmitbuf;
	struct rtw_tx_ring	*ring = &pri_adapter->xmitpriv.tx_ring[BCN_QUEUE_INX];
	u8	*tx_bufdesc;

	tx_bufdesc = (u8 *)&ring->buf_desc[0];
	pxmitbuf = &pxmitpriv->pcmd_xmitbuf[CMDBUF_BEACON];
	if (!pxmitbuf) {
		RTW_INFO("%s, fail to get xmit_buf\n", __func__);
		return;
	}
//	RTW_INFO("FREE pxmitbuf: %p, buf_desc: %p, sz: %d\n", pxmitbuf, tx_bufdesc, pxmitbuf->len);
	dma_unmap_single(&pdvobjpriv->pdev->dev,
			 GET_TX_BD_PHYSICAL_ADDR0_LOW(tx_bufdesc),
			 pxmitbuf->len,
			 DMA_TO_DEVICE);
}

static void rtl8730aa_hci_flush(PADAPTER Adapter, u32 queue_idx)
{
	struct xmit_priv *t_priv = &Adapter->xmitpriv;
	struct rtw_tx_ring *ring = &t_priv->tx_ring[queue_idx];
	u32 i, tmp32, cur_rp, cur_wp;

	/* bcn queue should not enter this function */
	if (queue_idx == BCN_QUEUE_INX) {
		return;
	}

	for (i = 0; i < 30; i++) {
		tmp32 = rtw_read32(Adapter, get_txbd_rw_reg(queue_idx));
		cur_rp = (tmp32 >> 16) & 0x0FFF;
		cur_wp = tmp32 & 0x0FFF;

		if (cur_rp == cur_wp) {
			return;
		}

		rtw_udelay_os(1);
	}

	RTW_INFO("%s timeout!(%d-%d)\n", __func__, cur_rp, cur_wp);
}

u32 rtl8730aa_init_bd(_adapter *padapter)
{
	struct xmit_priv *t_priv = &padapter->xmitpriv;
	int	i, ret = _SUCCESS;

	init_bd_ring_var(padapter);
	ret = rtl8730aa_init_rxbd_ring(padapter);

	if (ret == _FAIL) {
		return ret;
	}

	/* general process for other queue */
	for (i = 0; i < AXI_MAX_TX_QUEUE_COUNT; i++) {
		ret = rtl8730aa_init_txbd_ring(padapter, i,
					       t_priv->txringcount[i]);
		if (ret == _FAIL) {
			goto err_free_rings;
		}
	}

	return ret;

err_free_rings:

	rtl8730aa_free_rxbd_ring(padapter);

	for (i = 0; i < AXI_MAX_TX_QUEUE_COUNT; i++)
		if (t_priv->tx_ring[i].buf_desc) {
			rtl8730aa_free_txbd_ring(padapter, i);
		}


	return ret;
}

u32 rtl8730aa_free_bd(_adapter *padapter)
{
	struct xmit_priv	*t_priv = &padapter->xmitpriv;
	u32 i;


	/* free rxbd rings */
	rtl8730aa_free_rxbd_ring(padapter);

	/* free txbd rings */
	for (i = 0; i < HW_QUEUE_ENTRY; i++) {
		rtl8730aa_free_txbd_ring(padapter, i);
	}


	return _SUCCESS;
}

static u8 sethwreg(PADAPTER padapter, u8 variable, u8 *val)
{
	u8 ret = _SUCCESS;

	switch (variable) {
#ifdef CONFIG_LPS_LCLK
	case HW_VAR_SET_RPWM: {
		u8 ps_state = *((u8 *)val);
		/* rpwm value only use BIT0(clock bit) ,BIT6(Ack bit), and BIT7(Toggle bit) for 88e. */
		/* BIT0 value - 1: 32k, 0:40MHz. */
		/* BIT6 value - 1: report cpwm value after success set, 0:do not report. */
		/* BIT7 value - Toggle bit change. */
		/* modify by Thomas. 2012/4/2. */
		ps_state = ps_state & 0xC1;
		/* RTW_INFO("##### Change RPWM value to = %x for switch clk #####\n",ps_state); */
		rtw_write8(padapter, REG_PCIE_HRPWM1_V1_8730A, ps_state);
		break;
	}
#endif
	default:
		ret = rtl8730a_sethwreg(padapter, variable, val);
		break;
	}

	return ret;
}

static void gethwreg(PADAPTER padapter, u8 variable, u8 *val)
{
	switch (variable) {
#ifdef CONFIG_LPS_LCLK
	case HW_VAR_CPWM:
		*val = rtw_read8(padapter, REG_PCIE_HCPWM1_V1_8730A);
		break;
#endif
	default:
		rtl8730a_gethwreg(padapter, variable, val);
		break;
	}

}

/*
	Description:
		Query setting of specified variable.
*/
static u8 gethaldefvar(PADAPTER	padapter, HAL_DEF_VARIABLE eVariable, void *pValue)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	u8 bResult = _SUCCESS;

	switch (eVariable) {

	case HAL_DEF_MAX_RECVBUF_SZ:
		*((u32 *)pValue) = MAX_RECVBUF_SZ;
		break;

	case HW_VAR_MAX_RX_AMPDU_FACTOR:
		*(HT_CAP_AMPDU_FACTOR *)pValue = MAX_AMPDU_FACTOR_64K;
		break;
	default:
		bResult = rtl8730a_gethaldefvar(padapter, eVariable, pValue);
		break;
	}

	return bResult;
}

#ifdef CONFIG_RFKILL_POLL
static bool rtl8730aa_gpio_radio_on_off_check(_adapter *adapter, u8 *valid)
{
	u8 tmp8;
	bool ret;

	tmp8  = rtw_read32(adapter, REG_GPIO_EXT_CTRL_8730A);
	ret = (tmp8 & BIT(1)) ? _FALSE : _TRUE;	/* Power down pin output value, low active */
	*valid = 1;

	return ret;
}
#endif


void rtl8730aa_set_hal_ops(PADAPTER padapter)
{
	struct hal_ops *ops;
	int err;

	err = rtl8730aa_halmac_init_adapter(padapter);
	if (err) {
		RTW_INFO("%s: [ERROR]HALMAC initialize FAIL!\n", __func__);
		return;
	}

	rtl8730a_set_hal_ops(padapter);

	ops = &padapter->hal_func;

	ops->hal_init = rtl8730aa_init;
	ops->inirp_init = rtl8730aa_init_bd;
	ops->inirp_deinit = rtl8730aa_free_bd;
	ops->irp_reset = rtl8730aa_reset_bd;
	ops->init_xmit_priv = rtl8730aa_init_xmit_priv;
	ops->free_xmit_priv = rtl8730aa_free_xmit_priv;
	ops->init_recv_priv = rtl8730aa_init_recv_priv;
	ops->free_recv_priv = rtl8730aa_free_recv_priv;

#ifdef CONFIG_RTW_SW_LED
	ops->InitSwLeds = rtl8730aa_InitSwLeds;
	ops->DeInitSwLeds = rtl8730aa_DeInitSwLeds;
#endif

	ops->init_default_value = rtl8730aa_init_default_value;
	ops->intf_chip_configure = intf_chip_configure;

	ops->enable_interrupt = rtl8730aa_enable_interrupt;
	ops->disable_interrupt = rtl8730aa_disable_interrupt;
	ops->interrupt_handler = rtl8730aa_interrupt;
	/*
		ops->check_ips_status = check_ips_status;
	*/
	ops->clear_interrupt = rtl8730aa_clear_interrupt;
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN) ||\
	defined(CONFIG_AXI_HCI)
	/*
		ops->clear_interrupt = clear_interrupt_all;
	*/
#endif
	ops->get_hal_def_var_handler = gethaldefvar;

	ops->set_hw_reg_handler = sethwreg;
	ops->GetHwRegHandler = gethwreg;

	ops->hal_xmit = rtl8730aa_hal_xmit;
	ops->mgnt_xmit = rtl8730aa_mgnt_xmit;
#ifdef CONFIG_RTW_MGMT_QUEUE
	ops->hal_mgmt_xmitframe_enqueue = rtl8730aa_hal_mgmt_xmitframe_enqueue;
#endif
	ops->hal_xmitframe_enqueue = rtl8730aa_hal_xmitframe_enqueue;
#ifdef CONFIG_HOSTAPD_MLME
	ops->hostap_mgnt_xmit_entry = rtl8730aa_hostap_mgnt_xmit_entry;
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	/* vincent TODO */
	ops->xmit_thread_handler = rtl8730aa_xmit_buf_handler;
#endif

#ifdef CONFIG_RFKILL_POLL
	ops->hal_radio_onoff_check = rtl8730aa_gpio_radio_on_off_check;
#endif
	ops->unmap_beacon_icf = rtl8730aa_unmap_beacon_icf;

	ops->hci_flush = rtl8730aa_hci_flush;
}
