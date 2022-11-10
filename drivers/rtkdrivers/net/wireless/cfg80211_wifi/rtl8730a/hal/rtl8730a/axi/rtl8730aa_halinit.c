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

#define _RTL8730AA_HALINIT_C_
#include <drv_types.h>          /* PADAPTER, basic_types.h and etc. */
#include <hal_data.h>		/* HAL_DATA_TYPE */
#include "../../hal_halmac.h"	/* HALMAC API */
#include "../rtl8730a.h"
#include "rtl8730aa.h"
#include <rtl8730a_spec.h>

u32 InitMAC_TRXBD_8730AA(PADAPTER Adapter)
{
	u8 tmpU1b;
	u16 tmpU2b;
	u32 tmpU4b;
	u16 val16;
	int q_idx;
	struct recv_priv *precvpriv = &Adapter->recvpriv;
	struct xmit_priv *pxmitpriv = &Adapter->xmitpriv;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);

	RTW_INFO("=======>InitMAC_TXBD_8730AA()\n");

	/* Disable Stop RX DMA */
	tmpU4b = rtw_read32(Adapter, REG_AXI_CTRL);
	tmpU4b &= ~BIT_STOPRX;
	rtw_write32(Adapter, REG_AXI_CTRL, tmpU4b);

	/* Clear the pending ISR */
	rtw_write32(Adapter, REG_AXI_INTERRUPT, 0xFFFFFFFF);

	/*
	 * Set CMD TX BD (buffer descriptor) physical address(from OS API).
	 */
	//rtw_write32(Adapter, REG_H2CQ_TXBD_DESA,\
	//	    BIT_H2CQDESA(pxmitpriv->tx_ring[TXCMD_QUEUE_INX].dma));
	//rtw_write32(Adapter, REG_H2CQ_TXBD_NUM,\
	//	    BIT_H2CQ_DESC_MODE(RTL8730AA_SEG_NUM)\
	//	    | BIT_H2CQDES_NUM(TX_BD_NUM_8730AA_CMD));

	/*
	 * Set TX/RX BD (buffer descriptor) physical address(from OS API).
	 */
	rtw_write32(Adapter, REG_BCNQ_TXBD_DESA, \
		    BIT_BPQDESA(pxmitpriv->tx_ring[BCN_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_MGQ_TXBD_DESA, \
		    BIT_MGQDESA(pxmitpriv->tx_ring[MGT_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_VOQ_TXBD_DESA, \
		    BIT_VOQDESA(pxmitpriv->tx_ring[VO_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_VIQ_TXBD_DESA, \
		    BIT_VIQDESA(pxmitpriv->tx_ring[VI_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_BEQ_TXBD_DESA, \
		    BIT_BEQDESA(pxmitpriv->tx_ring[BE_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_BKQ_TXBD_DESA, \
		    BIT_BKQDESA(pxmitpriv->tx_ring[BK_QUEUE_INX].dma));
	rtw_write32(Adapter, REG_MQ0_TXBD_DESA, \
		    BIT_MQ0DESA(pxmitpriv->tx_ring[HIGH_QUEUE_INX].dma));

	rtw_write32(Adapter, REG_RXQ_RXBD_DESA, \
		    BIT_RXNRML_DESA(precvpriv->rx_ring[RX_MPDU_QUEUE].dma));

	/* pci buffer descriptor mode: Reset the Read/Write point to 0 */
	rtw_write32(Adapter, REG_BD_RWPTR_CLR, 0x007f007f);

	/* Reset the H2CQ R/W point index to 0 */
	tmpU4b = rtw_read32(Adapter, REG_H2CQ_CSR);
	rtw_write32(Adapter, REG_H2CQ_CSR, (tmpU4b | BIT_CLR_H2CQ_HW_IDX | BIT_CLR_H2CQ_HOST_IDX));

	tmpU1b = rtw_read8(Adapter, REG_AXI_CTRL + 3);
	rtw_write8(Adapter, REG_AXI_CTRL + 3, (tmpU1b | 0x33));

	/* 20100318 Joseph: Reset interrupt migration setting
	 * when initialization. Suggested by SD1. */
	rtw_write32(Adapter, REG_INT_MIG, 0);
	pHalData->bInterruptMigration = _FALSE;

	/* 2009.10.19. Reset H2C protection register. by tynli. */
	rtw_write32(Adapter, REG_MCUTST_I, 0x0);

#if MP_DRIVER == 1
	if (Adapter->registrypriv.mp_mode == 1) {
		rtw_write32(Adapter, REG_MACID, 0x87654321);
		rtw_write32(Adapter, 0x0700, 0x87654321);
	}
#endif

	/* axi buffer descriptor mode: */
	/* ---- rx */
	rtw_write16(Adapter, REG_RX_RXBD_NUM, \
		    BIT_BCNQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_RXDES_NUM(RX_BD_NUM_8730AA));

	/* ---- tx */
	rtw_write16(Adapter, REG_MGQ_TXBD_NUM, \
		    BIT_MGQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MGQDES_NUM(TX_MGQ_DESC_NUM));
	rtw_write16(Adapter, REG_VOQ_TXBD_NUM, \
		    BIT_VOQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_VOQDES_NUM(TX_BD_NUM_8730AA_VOQ));
	rtw_write16(Adapter, REG_VIQ_TXBD_NUM, \
		    BIT_VIQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_VIQDES_NUM(TX_BD_NUM_8730AA_VIQ));
	rtw_write16(Adapter, REG_BEQ_TXBD_NUM, \
		    BIT_BEQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_BEQDES_NUM(TX_BD_NUM_8730AA_BEQ));
	rtw_write16(Adapter, REG_BKQ_TXBD_NUM, \
		    BIT_BKQ_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_BKQDES_NUM(TX_BD_NUM_8730AA_BKQ));
	rtw_write16(Adapter, REG_MQ0_TXBD_NUM, \
		    BIT_MQ0_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ0DES_NUM(TX_H0Q_DESC_NUM));
#if 0
	rtw_write16(Adapter, REG_MQ1_TXBD_NUM, \
		    BIT_MQ1_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ1DES_NUM(TX_H1Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ2_TXBD_NUM, \
		    BIT_MQ2_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ2DES_NUM(TX_H2Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ3_TXBD_NUM, \
		    BIT_MQ3_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ3DES_NUM(TX_H3Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ4_TXBD_NUM, \
		    BIT_MQ4_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ4DES_NUM(TX_H4Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ5_TXBD_NUM, \
		    BIT_MQ5_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ5DES_NUM(TX_H5Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ6_TXBD_NUM, \
		    BIT_MQ6_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ6DES_NUM(TX_H6Q_DESC_NUM));
	rtw_write16(Adapter, REG_MQ7_TXBD_NUM, \
		    BIT_MQ7_DESC_MODE(RTL8730AA_SEG_NUM)\
		    | BIT_MQ7DES_NUM(TX_H7Q_DESC_NUM));
#endif

	/* testchip cannot open clock gating of txdma */
	tmpU4b = rtw_read32(Adapter, REG_TDE_GCK_CTRL);
	tmpU4b |= BIT_TDE_GCLK_EN;
	rtw_write32(Adapter, REG_TDE_GCK_CTRL, tmpU4b);

	/* reset read/write point */
	rtw_write32(Adapter, REG_BD_RWPTR_CLR, 0xFFFFFFFF);

	RTW_INFO("InitMAC_TXBD_8730AA() <====\n");

	return _SUCCESS;
}

#ifdef CONFIG_RTW_LED
static void init_hwled(PADAPTER adapter, u8 enable)
{
	u8 mode = 0;
	struct led_priv *ledpriv = adapter_to_led(adapter);

	if (ledpriv->LedStrategy != HW_LED) {
		return;
	}

	rtw_halmac_led_cfg(adapter_to_dvobj(adapter), enable, mode);
}
#endif /* CONFIG_RTW_LED */

static void hal_init_misc(PADAPTER adapter)
{
#ifdef CONFIG_RTW_LED
	struct led_priv *ledpriv = adapter_to_led(adapter);
#ifdef CONFIG_SW_LED
	pledpriv->bRegUseLed = _TRUE;
	ledpriv->LedStrategy = SW_LED_MODE1;
#else /* HW LED */
	ledpriv->LedStrategy = HW_LED;
#endif /* CONFIG_SW_LED */
	init_hwled(adapter, 1);
#endif
}

u32 rtl8730aa_init(PADAPTER padapter)
{
	u8 ok = _TRUE;
	u8 val8 = 0;
	u32 val32 = 0;
	PHAL_DATA_TYPE hal;
	struct registry_priv  *registry_par = &padapter->registrypriv;

	hal = GET_HAL_DATA(padapter);

	ok = rtl8730a_hal_init(padapter);
	if (_FALSE == ok) {
		return _FAIL;
	}

	rtw_write16(padapter, REG_AXI_CTRL, 0x0000);
	rtw_write8(padapter, REG_RX_DRVINFO_SZ, 0x4);

	rtl8730a_phy_init_haldm(padapter);
#ifdef CONFIG_BEAMFORMING
	rtl8730a_phy_bf_init(padapter);
#endif

#ifdef CONFIG_FW_MULTI_PORT_SUPPORT
	/*HW /FW init*/
	rtw_hal_set_default_port_id_cmd(padapter, 0);
#endif

#ifdef CONFIG_BT_COEXIST
	/* Init BT hw config. */
	if (hal->EEPROMBluetoothCoexist == _TRUE) {
		rtw_btcoex_HAL_Initialize(padapter, _FALSE);
#ifdef CONFIG_FW_MULTI_PORT_SUPPORT
		rtw_hal_set_wifi_btc_port_id_cmd(padapter);
#endif
	} else
#endif /* CONFIG_BT_COEXIST */
		rtw_btcoex_wifionly_hw_config(padapter);

	rtl8730a_init_misc(padapter);
	hal_init_misc(padapter);

	/* TX interrupt migration - 3pkts or 0.448 ms */
	/* SD1 Jong: follow 8822b's definition, and
	 * TX/RX should turn on simultaneously
	 */

	val8 = rtw_read8(padapter, REG_TCR_8730A);
	val8 |= BIT_WMAC_TCR_UPD_HGQMD_8730A;
	rtw_write8(padapter, REG_TCR_8730A, val8);

	val32 = rtw_read32(padapter, REG_SCOREBOARD_CTRL);
	val32 |= BIT(31);
	rtw_write32(padapter, REG_SCOREBOARD_CTRL, val32);

	return _SUCCESS;
}

void rtl8730aa_init_default_value(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;


	pHalData = GET_HAL_DATA(padapter);

	rtl8730a_init_default_value(padapter);

	/* interface related variable */
	pHalData->CurrentWirelessMode = WIRELESS_MODE_AUTO;
	pHalData->bDefaultAntenna = 1;
	pHalData->TransmitConfig = BIT_WMAC_TCR_ERRSTEN_3;
	/*BIT_CFEND_FORMAT isn't existing in 8730A */

	/* Set RCR-Receive Control Register .
	 * The value is set in InitializeAdapter8190Pci().
	 */

	pHalData->ReceiveConfig = (
#ifdef CONFIG_RX_PACKET_APPEND_FCS
					  BIT_APP_FCS		|
#endif
					  BIT_APP_MIC		|
					  BIT_APP_ICV		|
					  BIT_DISGCLK		|
					  BIT_VHT_DACK		|
					  BIT_HTCBFMC		|
					  /* BIT_AMF		| */
					  BIT_CBSSID_DATA		|
					  BIT_CBSSID_MGNT		|
					  /* BIT_ACF		| */
					  /* BIT_ADF		| */ /* PS-Poll filter */
					  BIT_AB			|
					  BIT_AB			|
					  BIT_APM			|
					  0);

	/*
	 * Set default value of Interrupt Mask Register0
	 */
	pHalData->IntrMaskDefault[0] = (u32)(
					       BIT_HISR1_INT_EN |
					       BIT_HISR2_INT_EN |
					       BIT_TXFF_FIFO_INT_EN |  /* TXFOVW */
					       BIT_RXFF_FULL_INT_EN |
					       BIT_HCPWM_INT_EN |
					       0);

	/*
	 * Set default value of Interrupt Mask Register1
	 */
	pHalData->IntrMaskDefault[1] = (u32)(
					       IMR_ROK_8730A |
					       IMR_RDU_8730A |
					       IMR_VODOK_8730A |
					       IMR_VIDOK_8730A |
					       IMR_BEDOK_8730A |
					       IMR_BKDOK_8730A |
					       IMR_MGNTDOK_8730A |
					       //IMR_H2COK_8730A |
					       IMR_H0DOK_8730A |
					       0);

	pHalData->IntrMaskDefault[2] = 0;
	/*
	 * Set default value of Interrupt Mask Register3
	 */
	pHalData->IntrMaskDefault[3] = 0;

	pHalData->IntrMask[0] = pHalData->IntrMaskDefault[0];
	pHalData->IntrMask[1] = pHalData->IntrMaskDefault[1];
	pHalData->IntrMask[2] = pHalData->IntrMaskDefault[2];
	pHalData->IntrMask[3] = pHalData->IntrMaskDefault[3];
}
