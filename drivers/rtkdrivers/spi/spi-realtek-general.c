/**
  ******************************************************************************
  * @file    spi-realtek-general.c
  * @author
  * @version V1.0.0
  * @date    2021-11-11
  * @brief   This file contains all the functions prototypes for Serial peripheral interface (SPI):
  *		- Initialization
  *		- Clock polarity and phase setting
  *		- SPI data frame size setting
  *		- SPI baud rate setting
  *		- Receive/Send data interface
  *		- Get TRx FIFO valid entries
  *		- check SPI device busy status
  *		- SPI device pinmux initialization and deinitialization
  *		- DMA transfers management
  *		- Interrupts and management
  * @attention
  *		- for master tx, slave rx, slave prepare first and wait for master's tx data.
  *		- for master rx, slave tx, slave tx to its fifo first and wait master's rx signal and dummy data.
  *		- spi master trx can choose interrupt mode or poll mode by configuring dts.
  *		- spi slave rx support interrupt mode only, so tx is also in interrupt mode.
  ******************************************************************************
  * @attention
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
  ******************************************************************************
  */
#include "spi-realtek-general.h"
#include <linux/of_gpio.h>

/* Mapping address used by both SPI0 and SPI1. */
static void __iomem *role_set_base = 0;

static void rtk_spi_writel(
	void __iomem *ptr, u32 reg, u32 value)
{
	writel(value, (void __iomem *)((u32)ptr + reg));
}

static u32 rtk_spi_readl(void __iomem *ptr, u32 reg)
{
	return readl((void __iomem *)((u32)ptr + reg));
}

void rtk_spi_reg_update(
	void __iomem *ptr,
	u32 reg, u32 mask, u32 value)
{
	u32 temp;

	temp = rtk_spi_readl(ptr, reg);
	temp |= mask;
	temp &= (~mask | value);
	rtk_spi_writel(ptr, reg, temp);
}

#if RTK_SPI_REG_DUMP
void rtk_spi_reg_dump(struct rtk_spi_controller *rtk_spi)
{
	pr_info("SPI_CTRLR0[%x] = %x", SPI_CTRLR0, rtk_spi_readl(rtk_spi->base, SPI_CTRLR0));
	pr_info("SPI_CTRLR1[%x] = %x", SPI_CTRLR1, rtk_spi_readl(rtk_spi->base, SPI_CTRLR1));
	pr_info("SPI_SSIENR[%x] = %x", SPI_SSIENR, rtk_spi_readl(rtk_spi->base, SPI_SSIENR));
	pr_info("RSVD0[%x] = %x", RSVD0, rtk_spi_readl(rtk_spi->base, RSVD0));
	pr_info("SPI_SER[%x] = %x", SPI_SER, rtk_spi_readl(rtk_spi->base, SPI_SER));
	pr_info("SPI_BAUDR[%x] = %x", SPI_BAUDR, rtk_spi_readl(rtk_spi->base, SPI_BAUDR));
	pr_info("SPI_TXFTLR[%x] = %x", SPI_TXFTLR, rtk_spi_readl(rtk_spi->base, SPI_TXFTLR));
	pr_info("SPI_RXFTLR[%x] = %x", SPI_RXFTLR, rtk_spi_readl(rtk_spi->base, SPI_RXFTLR));
	pr_info("SPI_TXFLR[%x] = %x", SPI_TXFLR, rtk_spi_readl(rtk_spi->base, SPI_TXFLR));
	pr_info("SPI_RXFLR[%x] = %x", SPI_RXFLR, rtk_spi_readl(rtk_spi->base, SPI_RXFLR));
	pr_info("RTK_SPI_SR[%x] = %x", RTK_SPI_SR, rtk_spi_readl(rtk_spi->base, RTK_SPI_SR));
	pr_info("SPI_IMR[%x] = %x", SPI_IMR, rtk_spi_readl(rtk_spi->base, SPI_IMR));
	pr_info("SPI_ISR[%x] = %x", SPI_ISR, rtk_spi_readl(rtk_spi->base, SPI_ISR));
	pr_info("SPI_RISR[%x] = %x", SPI_RISR, rtk_spi_readl(rtk_spi->base, SPI_RISR));
}
#endif // RTK_SPI_REG_DUMP

static void assert_param(u8 check_result)
{
	if (!check_result) {
		pr_info("error: illegal parameter.");
	}
}

static void rtk_get_dts_info(
	struct rtk_spi_controller *rtk_spi,
	struct device_node *np,
	int *param_to_set, int default_value,
	char *dts_name)
{
	int nr_requests, ret;

	/* get dts params. */
	ret = of_property_read_u32(np, dts_name, &nr_requests);
	if (ret) {
		dev_err(rtk_spi->dev, "can't get %s", dts_name);
		*param_to_set = default_value;
	} else {
		dev_dbg(rtk_spi->dev, "%s = %d", dts_name, nr_requests);
		*param_to_set = nr_requests;
	}
}

void rtk_spi_struct_init(
	struct rtk_spi_controller *rtk_spi,
	struct device_node *np)
{
	/* dts rtk defined params. */
	char s0[] = "reg";
	char s1[] = "rtk,spi-default-cs";
	char s2[] = "rtk,spi-slave-mode";
	char s3[] = "rtk,max-cs-num";
	char s4[] = "rtk,spi-index";
	char s5[] = "rtk,spi-dma-en";
	char s6[] = "rtk,spi-master-poll-mode";
	char s7[] = "rtk,spi-for-kernel";
	int ret = 0;
	enum of_gpio_flags flags;

	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.dma_params.spi_phy_addr, 0, s0);

	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.spi_default_cs, 0, s1);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.is_slave, 0, s2);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.max_cs_num, 0xFFFF, s3);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.spi_index, 0, s4);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.dma_enabled, 0, s5);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.spi_poll_mode, 0, s6);
	rtk_get_dts_info(rtk_spi, np, &rtk_spi->spi_manage.spi_for_kernel, 0, s7);

	rtk_spi->spi_param.rx_threshold_level = 0;
	rtk_spi->spi_param.tx_threshold_level = 1;

	rtk_spi->spi_param.dma_rx_data_level = 3;
	rtk_spi->spi_param.dma_tx_data_level = 56;

	rtk_spi->spi_param.slave_select_enable = 0;
	rtk_spi->spi_param.data_frame_num = 0;
	rtk_spi->spi_param.clock_divider = 2;
	rtk_spi->spi_param.data_frame_format = FRF_MOTOROLA_SPI;
	rtk_spi->spi_param.data_frame_size = DFS_8_BITS;
	rtk_spi->spi_param.interrupt_mask = 0x0;
	rtk_spi->spi_param.transfer_mode = TMOD_TR;
	/* Should keep the same for master and slave. */
	rtk_spi->spi_param.sclk_phase = SCPH_TOGGLES_IN_MIDDLE;
	rtk_spi->spi_param.sclk_polarity = SCPOL_INACTIVE_IS_LOW;

	rtk_spi->spi_manage.spi_prepared = SPI_NOT_SETUP;
	rtk_spi->spi_manage.dma_params.gdma_status = RTK_SPI_GDMA_UNPREPARED;

	rtk_spi->spi_manage.spi_cs_pin = of_get_named_gpio_flags(np, "rtk,spi-cs-gpios", 0, &flags);
	ret = gpio_request(rtk_spi->spi_manage.spi_cs_pin, NULL);
	if (ret != 0) {
		dev_err(rtk_spi->dev, "Fail to request spi cs pin\n");
		goto fail;
	}

	ret = gpio_direction_output(rtk_spi->spi_manage.spi_cs_pin, 0);
	if (IS_ERR_VALUE(ret)) {
		dev_err(rtk_spi->dev, "Fail to request spi cs to output direction\n");
		goto fail;
	}

	gpio_set_value(rtk_spi->spi_manage.spi_cs_pin, 1);

	if (rtk_spi->spi_manage.dma_enabled) {
		rtk_spi->spi_manage.dma_params.chan = NULL;
		rtk_spi->spi_manage.dma_params.config = NULL;
		mutex_init(&rtk_spi->spi_manage.dma_params.dma_lock);
	}

#if RTK_SPI_DMA_HELP
	rtk_spi->spi_manage.dma_params.help_dma_addr = 0;
#endif // RTK_SPI_DMA_HELP

fail:
	gpio_free(rtk_spi->spi_manage.spi_cs_pin);
}

void rtk_spi_enable_cmd(
	struct rtk_spi_controller *rtk_spi, u32 new_status)
{
	dev_dbg(rtk_spi->dev, "%s, status : %s", __FUNCTION__, new_status ? "enable" : "disable");

	if (new_status != DISABLE) {
		rtk_spi_reg_update(rtk_spi->base, SPI_SSIENR, SPI_BIT_SSI_EN, SPI_BIT_SSI_EN);
	} else {
		rtk_spi_reg_update(rtk_spi->base, SPI_SSIENR, SPI_BIT_SSI_EN, ~SPI_BIT_SSI_EN);
	}
}

u32 rtk_spi_get_status(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, RTK_SPI_SR);
}

u32 rtk_spi_busy_check(
	struct rtk_spi_controller *rtk_spi)
{
	return (((rtk_spi_get_status(rtk_spi) & SPI_BIT_BUSY) != 0) ? 1 : 0);
}

void rtk_spi_set_slave_enable(
	struct rtk_spi_controller *rtk_spi, u32 slave_index)
{
	if (rtk_spi_busy_check(rtk_spi)) {
		dev_dbg(rtk_spi->dev, "SPI-%d is busy\n", rtk_spi->spi_manage.spi_index);
		if (rtk_spi->spi_manage.current_direction != RTK_SPI_TRX) {
			return;
		}
		dev_dbg(rtk_spi->dev, "but continue loopback test.");
	}

	rtk_spi_writel(rtk_spi->base, SPI_SER, 1 << slave_index);
}

void rtk_spi_set_dma_level(
	struct rtk_spi_controller *rtk_spi, u32 tx_level, u32 rx_level)
{
	/* Set TX FIFO water level to trigger Tx DMA transfer */
	rtk_spi_writel(rtk_spi->base, SPI_DMATDLR, tx_level);

	/* Set RX FIFO water level to trigger Rx DMA transfer */
	rtk_spi_writel(rtk_spi->base, SPI_DMARDLR, rx_level);
}

void rtk_spi_set_role(
	struct rtk_spi_controller *rtk_spi)
{
	if (rtk_spi->spi_manage.spi_index == 0) {
		if (!rtk_spi->spi_manage.is_slave) {
			/* set spi0 master mode. */
			rtk_spi_reg_update(role_set_base, REG_HSYS_HPLAT_CTRL, HSYS_BIT_SPI0_MST, HSYS_BIT_SPI0_MST);
		} else {
			/* set spi0 slave mode. */
			rtk_spi_reg_update(role_set_base, REG_HSYS_HPLAT_CTRL, HSYS_BIT_SPI0_MST, ~HSYS_BIT_SPI0_MST);
		}
	} else if (rtk_spi->spi_manage.spi_index == 1) {
		if (!rtk_spi->spi_manage.is_slave) {
			/* set spi1 master mode. */
			rtk_spi_reg_update(role_set_base, REG_HSYS_HPLAT_CTRL, HSYS_BIT_SPI1_MST, HSYS_BIT_SPI1_MST);
		} else {
			/* set spi1 slave mode. */
			rtk_spi_reg_update(role_set_base, REG_HSYS_HPLAT_CTRL, HSYS_BIT_SPI1_MST, ~HSYS_BIT_SPI1_MST);
		}
	} else {
		dev_err(rtk_spi->dev, "Error: spi index error\n");
	}
}

void rtk_spi_set_clk_polarity(
	struct rtk_spi_controller *rtk_spi, u32 sclk_polarity)
{
	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SCPOL, sclk_polarity << 7);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_clk_phase(
	struct rtk_spi_controller *rtk_spi, u32 sclk_phase)
{
	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SCPH, sclk_phase << 6);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_frame_size(
	struct rtk_spi_controller *rtk_spi, u32 data_frame_size)
{
	dev_dbg(rtk_spi->dev, "%s, data_frame_size = %d", __FUNCTION__, data_frame_size);

	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_MASK_DFS, data_frame_size);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_read_len(
	struct rtk_spi_controller *rtk_spi, u32 data_frame_num)
{
	assert_param((data_frame_num >= 1) && (data_frame_num <= 0x10000));

	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_writel(rtk_spi->base, SPI_CTRLR1, data_frame_num - 1);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_baud_div(
	struct rtk_spi_controller *rtk_spi, u32 clk_divider)
{
	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_writel(rtk_spi->base, SPI_BAUDR, clk_divider & SPI_MASK_SCKDV);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_dma_enable(
	struct rtk_spi_controller *rtk_spi,
	u32 new_state, u8 direction)
{
	if (new_state == DISABLE) {
		if (direction == SPI_TX_MODE) {
			rtk_spi_reg_update(rtk_spi->base, SPI_DMACR, SPI_BIT_TDMAE, ~SPI_BIT_TDMAE);
		} else {
			rtk_spi_reg_update(rtk_spi->base, SPI_DMACR, SPI_BIT_RDMAE, ~SPI_BIT_RDMAE);
		}
	} else {
		if (direction == SPI_TX_MODE) {
			rtk_spi_reg_update(rtk_spi->base, SPI_DMACR, SPI_BIT_TDMAE, SPI_BIT_TDMAE);
		} else {
			rtk_spi_reg_update(rtk_spi->base, SPI_DMACR, SPI_BIT_RDMAE, SPI_BIT_RDMAE);
		}
	}
}

void rtk_spi_init_hw(struct rtk_spi_controller *rtk_spi)
{
	u32 temp_value	= 0;

	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_set_role(rtk_spi);

	/* REG_DW_SSI_CTRLR0 */
	temp_value |= rtk_spi->spi_param.data_frame_size;
	temp_value |= (rtk_spi->spi_param.data_frame_format << 4);
	temp_value |= (rtk_spi->spi_param.sclk_phase << 6);
	temp_value |= (rtk_spi->spi_param.sclk_polarity << 7);
	temp_value |= (rtk_spi->spi_param.transfer_mode << 8);
	temp_value &= ~SPI_BIT_SLV_OE;
	rtk_spi_writel(rtk_spi->base, SPI_CTRLR0, temp_value);
	rtk_spi_writel(rtk_spi->base, SPI_TXFTLR, rtk_spi->spi_param.tx_threshold_level);
	rtk_spi_writel(rtk_spi->base, SPI_RXFTLR, rtk_spi->spi_param.rx_threshold_level);

	/* Master Only:REG_DW_SSI_CTRLR1, REG_DW_SSI_SER, REG_DW_SSI_BAUDR*/
	if (rtk_spi->spi_param.spi_role & SSI_MASTER) {
		rtk_spi_writel(rtk_spi->base, SPI_CTRLR1, rtk_spi->spi_param.data_frame_num);
		rtk_spi_set_slave_enable(rtk_spi, rtk_spi->spi_param.slave_select_enable);
		rtk_spi_writel(rtk_spi->base, SPI_BAUDR, rtk_spi->spi_param.clock_divider);
	}

	/* REG_DW_SSI_IMR */
	rtk_spi_writel(rtk_spi->base, SPI_IMR, rtk_spi->spi_param.interrupt_mask);

	/*DMA level set */
	if (rtk_spi->spi_manage.dma_enabled) {
		rtk_spi_set_dma_level(rtk_spi, rtk_spi->spi_param.dma_tx_data_level, rtk_spi->spi_param.dma_rx_data_level);
	}

	rtk_spi_enable_cmd(rtk_spi, ENABLE);

	rtk_spi_set_clk_polarity(rtk_spi, rtk_spi->spi_param.sclk_polarity);
	rtk_spi_set_clk_phase(rtk_spi, rtk_spi->spi_param.sclk_phase);
	rtk_spi_set_baud_div(rtk_spi, rtk_spi->spi_param.clock_divider);

}

void rtk_spi_interrupt_config(
	struct rtk_spi_controller *rtk_spi, u32 spi_int, u32 new_state)
{
	if (new_state == ENABLE) {
		/* Enable the selected SSI interrupts */
		rtk_spi_reg_update(rtk_spi->base, SPI_IMR, spi_int, spi_int);
	} else {
		/* Disable the selected SSI interrupts */
		rtk_spi_reg_update(rtk_spi->base, SPI_IMR, spi_int, ~spi_int);
	}
}

void rtk_spi_clean_interrupt(
	struct rtk_spi_controller *rtk_spi, u32 int_status)
{
	if (int_status & SPI_BIT_TXOIS) {
		rtk_spi_readl(rtk_spi->base, SPI_TXOICR);
	}

	if (int_status & SPI_BIT_RXUIS) {
		rtk_spi_readl(rtk_spi->base, SPI_RXUICR);
	}

	if (int_status & SPI_BIT_RXOIS) {
		rtk_spi_readl(rtk_spi->base, SPI_RXOICR);
	}

	if (int_status & SPI_BIT_MSTIS_FAEIS) {
		/* Another master is actively transferring data */
		/* TODO: Do reading data... */
		rtk_spi_readl(rtk_spi->base, SPI_MSTICR_FAEICR);
	}

	if (int_status & SPI_BIT_TXUIS) {
		/* For slave only. This register is used as TXUICR in slave mode*/
		rtk_spi_readl(rtk_spi->base, SPI_TXUICR);
	}

	if (int_status & SPI_BIT_SSRIS) {
		/* For slave only. This register is used as SSRICR in slave mode*/
		rtk_spi_readl(rtk_spi->base, SPI_SSRICR);
	}

}

void rtk_spi_write_data(
	struct rtk_spi_controller *rtk_spi, u32 value)
{
#if RTK_SPI_DEBUG_DETAILS
	pr_info("spi-%d value write = %x", rtk_spi->spi_manage.spi_index, value);
#endif // RTK_SPI_DEBUG_DETAILS
	rtk_spi_writel(rtk_spi->base, SPI_DATA_FIFO_ENRTY, value & SPI_MASK_DR);
}

void rtk_spi_set_rx_fifo_level(
	struct rtk_spi_controller *rtk_spi, u32 rx_threshold_level)
{
	rtk_spi_writel(rtk_spi->base, SPI_RXFTLR, rx_threshold_level);
}

void rtk_spi_set_tx_fifo_level(
	struct rtk_spi_controller *rtk_spi, u32 tx_threshold_level)
{
	rtk_spi_writel(rtk_spi->base, SPI_TXFTLR, tx_threshold_level);
}

u32 rtk_spi_read_data(
	struct rtk_spi_controller *rtk_spi)
{
	u32 temp;
	temp = rtk_spi_readl(rtk_spi->base, SPI_DATA_FIFO_ENRTY);
#if RTK_SPI_DEBUG_DETAILS
	pr_info("%x", temp);
#endif // RTK_SPI_DEBUG_DETAILS
	return temp;
}

u32 rtk_spi_get_rx_cnt(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, SPI_RXFLR) & SPI_MASK_RXTFL;
}

u32 rtk_spi_get_tx_cnt(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, SPI_TXFLR) & SPI_MASK_TXTFL;
}

u32 rtk_spi_get_data_frame_size(
	struct rtk_spi_controller *rtk_spi)
{

	u32 size;
	size = (rtk_spi_readl(rtk_spi->base, SPI_CTRLR0) & SPI_MASK_DFS) + 1;
	return size;
}

void rtk_spi_set_sample_delay(
	struct rtk_spi_controller *rtk_spi, u32 sample_delay)
{
	rtk_spi_writel(rtk_spi->base, SPI_RX_SAMPLE_DLY, sample_delay & SPI_MASK_RSD);
}

u32 rtk_spi_get_interrupt(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, SPI_ISR);
}

#if RTK_SPI_HW_CONTROL_FOR_FUTURE_USE
void rtk_spi_set_toggle_phase(
	struct rtk_spi_controller *rtk_spi, u32 toggle_phase)
{
	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SS_T, toggle_phase << 31);
	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

void rtk_spi_set_data_swap(
	struct rtk_spi_controller *rtk_spi, u32 swap_status, u32 new_state)
{
	rtk_spi_enable_cmd(rtk_spi, DISABLE);

	if (new_state == ENABLE) {
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, swap_status, swap_status);
	} else {
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, swap_status, ~swap_status);
	}

	rtk_spi_enable_cmd(rtk_spi, ENABLE);
}

u32 rtk_spi_get_raw_interrupt(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, SPI_RISR);
}

u32 rtk_spi_check_slave_enable(
	struct rtk_spi_controller *rtk_spi)
{
	return rtk_spi_readl(rtk_spi->base, SPI_SER);
}
#endif // RTK_SPI_HW_CONTROL_FOR_FUTURE_USE

u32 rtk_spi_get_readable(
	struct rtk_spi_controller *rtk_spi)
{
	u32 status = rtk_spi_get_status(rtk_spi);
	u32 readable = (((status & SPI_BIT_RFNE) != 0) ? 1 : 0);

	dev_dbg(rtk_spi->dev, "%s, fifo %s empty", __FUNCTION__, readable ? "is not" : "is");
	return readable;
}

u32 rtk_spi_get_writeable(
	struct rtk_spi_controller *rtk_spi)
{
	u8 status = 0;
	u32 value = 0;

#ifdef SPI_SLAVE_TXERR_WORK_AROUND
	value = rtk_spi_get_tx_cnt(rtk_spi);
	status = ((value < SSI_TX_FIFO_DEPTH - 1) ? 1 : 0);
#else
	value = rtk_spi_get_status(rtk_spi);
	status = (((value & SPI_BIT_TFNF) != 0) ? 1 : 0);
#endif

	return status;
}

void rtk_spi_status_switch(
	struct rtk_spi_controller *rtk_spi,
	enum rtk_spi_master_trans_status to_set)
{
	switch (to_set) {
	case RTK_SPI_RX_DONE:
		if (rtk_spi->spi_manage.transfer_status == RTK_SPI_ONGOING) {
			rtk_spi->spi_manage.transfer_status = RTK_SPI_RX_DONE;
		}
		if ((rtk_spi->spi_manage.transfer_status == RTK_SPI_TX_DONE) && (rtk_spi->spi_manage.current_direction == RTK_SPI_TRX)) {
			rtk_spi->spi_manage.transfer_status = RTK_SPI_TRX_DONE;
		}
		break;
	case RTK_SPI_TX_DONE:
		if (rtk_spi->spi_manage.transfer_status == RTK_SPI_ONGOING) {
			rtk_spi->spi_manage.transfer_status = RTK_SPI_TX_DONE;
		}
		if ((rtk_spi->spi_manage.transfer_status == RTK_SPI_RX_DONE) && (rtk_spi->spi_manage.current_direction == RTK_SPI_TRX)) {
			rtk_spi->spi_manage.transfer_status = RTK_SPI_TRX_DONE;
		}
		break;
	default:
		rtk_spi->spi_manage.transfer_status = to_set;
		break;
	}

	dev_dbg(rtk_spi->dev, "Transfer status set to %d.", rtk_spi->spi_manage.transfer_status);
}

u32 rtk_spi_receive_data(
	struct rtk_spi_controller *rtk_spi,
	void *rx_data,
	u32 length)
{
	u32 receive_level;
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);
	u32 readable = rtk_spi_get_readable(rtk_spi);
	u32 left_data = length;


	while (readable) {
		receive_level = rtk_spi_get_rx_cnt(rtk_spi);
		dev_dbg(rtk_spi->dev, "rx level = %d", receive_level);

		while (receive_level--) {
			if (rx_data != NULL) {
				if (data_frame_size > 8) {
					/*  16~9 bits mode */
					*((u16 *)(rx_data)) = (u16)rtk_spi_read_data(rtk_spi);
					rx_data = (void *)(((u16 *)rx_data) + 1);
				} else {
					/*  8~4 bits mode */
					*((u8 *)(rx_data)) = (u8)rtk_spi_read_data(rtk_spi);
					rx_data = (void *)(((u8 *)rx_data) + 1);
				}
			} else {
				/*  for Master mode, doing TX also will got RX data, so drop the dummy data */
				rtk_spi_read_data(rtk_spi);
			}

			if (left_data > 0) {
				left_data--;
			}
			if (left_data == 0) {
				break;
			}
		}

		if (left_data == 0) {
			break;
		}

		readable = rtk_spi_get_readable(rtk_spi);
	}

	if (left_data > RTK_SPI_FIFO_HALF) {
		rtk_spi_set_rx_fifo_level(rtk_spi, RTK_SPI_FIFO_HALF);
	} else if (left_data) {
		rtk_spi_set_rx_fifo_level(rtk_spi, left_data - 1);
	}

	return (length - left_data);
}

u32 rtk_spi_send_data(
	struct rtk_spi_controller *rtk_spi,
	const void *tx_data,
	u32 length,
	u32 is_slave)
{
	u32 writeable = rtk_spi_get_writeable(rtk_spi);
	u32 tx_write_max = SSI_TX_FIFO_DEPTH - rtk_spi_get_tx_cnt(rtk_spi);
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);
	u32 tx_length = length;

#ifdef SPI_SLAVE_TXERR_WORK_AROUND
	if (is_slave) {
		tx_write_max = (tx_write_max > 0) ? (tx_write_max - 1) : 0;
	}
#endif

	if (writeable) {
		/* Disable Tx FIFO Empty IRQ */
		rtk_spi_interrupt_config(rtk_spi, SPI_BIT_TXEIM, DISABLE);

		while (tx_write_max--) {
			if (data_frame_size > 8) {
				// 16~9 bits mode
				if (tx_data != NULL) {
					rtk_spi_write_data(rtk_spi, *((u16 *)(tx_data)));
					tx_data = (void *)(((u16 *)tx_data) + 1);
				} else {
					// For master mode: Push a dummy to TX FIFO for Read
					if (!is_slave) {
						rtk_spi_write_data(rtk_spi, (u16)0);// Dummy byte
					}
				}
			} else {
				// 8~4 bits mode
				if (tx_data != NULL) {
					rtk_spi_write_data(rtk_spi, *((u8 *)(tx_data)));
					tx_data = (void *)(((u8 *)tx_data) + 1);
				} else {
					// For master mode: Push a dummy to TX FIFO for Read
					if (!is_slave) {
						rtk_spi_write_data(rtk_spi, (u8)0);// Dummy byte
					}
				}
			}

			tx_length--;

			if (tx_length == 0) {
				break;
			}
		}

		/* Enable Tx FIFO Empty IRQ */
		rtk_spi_interrupt_config(rtk_spi, SPI_BIT_TXEIM, ENABLE);
	}

	return (length - tx_length);
}

static irqreturn_t rtk_spi_interrupt_handler(int irq, void *dev_id)
{
	struct rtk_spi_controller *rtk_spi = dev_id;

	u32 int_status = rtk_spi_get_interrupt(rtk_spi);
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);
	u32 trans_len = 0;

	rtk_spi_clean_interrupt(rtk_spi, int_status);

	if (int_status & (SPI_BIT_TXOIS | SPI_BIT_RXUIS | SPI_BIT_RXOIS | SPI_BIT_TXUIS)) {
		dev_dbg(rtk_spi->dev, "[INT] Tx/Rx Warning %x \n", int_status);
	}

	if (int_status & SPI_BIT_SSRIS) {
		dev_dbg(rtk_spi->dev, "[INT] SS_N Rising Edge Detect %x \n", int_status);
		rtk_spi_interrupt_config(rtk_spi, SPI_BIT_SSRIM, DISABLE);
	}

	if (int_status & SPI_BIT_MSTIS_FAEIS) {
		dev_dbg(rtk_spi->dev, "[INT] Multi Master Contention Interrupt / Slave Frame Alignment Interrupt %x \n", int_status);
	}

	if ((int_status & SPI_BIT_RXFIS)) {

		trans_len = rtk_spi_receive_data(rtk_spi, rtk_spi->spi_manage.rx_info.p_data_buf, rtk_spi->spi_manage.rx_info.data_len);
		rtk_spi->spi_manage.rx_info.data_len -= trans_len;
		if (data_frame_size > 8) {
			// 16~9 bits mode
			rtk_spi->spi_manage.rx_info.p_data_buf = (void *)(((u16 *)rtk_spi->spi_manage.rx_info.p_data_buf) + trans_len);
		} else {
			// 8~4 bits mode
			rtk_spi->spi_manage.rx_info.p_data_buf = (void *)(((u8 *)rtk_spi->spi_manage.rx_info.p_data_buf) + trans_len);
		}

		if (rtk_spi->spi_manage.rx_info.data_len == 0) {
			rtk_spi_interrupt_config(rtk_spi, (SPI_BIT_RXFIM | SPI_BIT_RXOIM | SPI_BIT_RXUIM), DISABLE);
			rtk_spi_status_switch(rtk_spi, RTK_SPI_RX_DONE);
			spi_finalize_current_transfer(rtk_spi->controller);
		}
	}

	if (int_status & SPI_BIT_TXEIS) {

		u32 trans_len = 0;
		dev_dbg(rtk_spi->dev, "[INT] Tx Empty %x \n", int_status);

		trans_len = rtk_spi_send_data(rtk_spi, rtk_spi->spi_manage.tx_info.p_data_buf, rtk_spi->spi_manage.tx_info.data_len, rtk_spi->spi_manage.is_slave);

		rtk_spi->spi_manage.tx_info.data_len -= trans_len;
		if (data_frame_size > 8) {
			// 16~9 bits mode
			rtk_spi->spi_manage.tx_info.p_data_buf = (void *)(((u16 *)rtk_spi->spi_manage.tx_info.p_data_buf) + trans_len);
		} else {
			// 8~4 bits mode
			rtk_spi->spi_manage.tx_info.p_data_buf = (void *)(((u8 *)rtk_spi->spi_manage.tx_info.p_data_buf) + trans_len);
		}

		if (!rtk_spi->spi_manage.tx_info.data_len) {
			rtk_spi_interrupt_config(rtk_spi, (SPI_BIT_TXOIM | SPI_BIT_TXEIM), DISABLE);
			rtk_spi_status_switch(rtk_spi, RTK_SPI_TX_DONE);
			if (rtk_spi->spi_manage.current_direction != RTK_SPI_TRX) {
				while(rtk_spi_busy_check(rtk_spi) == 1);
				spi_finalize_current_transfer(rtk_spi->controller);
			}
		}
	}

	return IRQ_HANDLED;
}

u32 rtk_spi_interrupt_read(
	struct rtk_spi_controller *rtk_spi,
	void *rx_data, u32 length)
{
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);

	assert_param(length != 0);

	if (data_frame_size > 8) {
		/*  16~9 bits mode */
		rtk_spi->spi_manage.rx_info.data_len = length >> 1; // 2 bytes(16 bit) every transfer
	} else {
		/*  8~4 bits mode */
		rtk_spi->spi_manage.rx_info.data_len = length; // 1 byte(8 bit) every transfer
	}

	rtk_spi->spi_manage.rx_info.p_data_buf = rx_data;

	rtk_spi_interrupt_config(rtk_spi, (SPI_BIT_RXFIM | SPI_BIT_RXOIM | SPI_BIT_RXUIM), ENABLE);

	return USE_SPI_CORE_WAIT;
}

u32 rtk_spi_interrupt_write(
	struct rtk_spi_controller *rtk_spi,
	const void *ptx_data, u32 length)
{
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);

	assert_param(length != 0);

#if RTK_SPI_REG_DUMP
	rtk_spi_reg_dump(rtk_spi);
#endif // RTK_SPI_REG_DUMP

	if (data_frame_size > 8) {
		/*  16~9 bits mode */
		rtk_spi->spi_manage.tx_info.data_len = length >> 1; // 2 bytes(16 bit) every transfer
	} else {
		/*  8~4 bits mode */
		rtk_spi->spi_manage.tx_info.data_len = length; // 1 byte(8 bit) every transfer
	}

	rtk_spi->spi_manage.tx_info.p_data_buf = (void *)ptx_data;
	rtk_spi_interrupt_config(rtk_spi, (SPI_BIT_TXOIM | SPI_BIT_TXEIM), ENABLE);

	return USE_SPI_CORE_WAIT;
}

u32 rtk_spi_poll_send(
	struct rtk_spi_controller *rtk_spi,
	const void *ptx_data, u32 length)
{
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);
	int ret = 0;

	while (length) {
		if (rtk_spi_get_status(rtk_spi) & SPI_BIT_TFNF) {
			if (data_frame_size > 8) {
				rtk_spi_write_data(rtk_spi, *(u16 *)(ptx_data));
				ptx_data = (void *)(((u16 *)ptx_data) + 1);
			} else {
				rtk_spi_write_data(rtk_spi, *(u8 *)(ptx_data));
				ptx_data = (void *)(((u8 *)ptx_data) + 1);
			}
			length--;
		}
	}

	while(rtk_spi_busy_check(rtk_spi) == 1);
	rtk_spi_status_switch(rtk_spi, RTK_SPI_TX_DONE);
	spi_finalize_current_transfer(rtk_spi->controller);

	return ret;
}

int rtk_spi_poll_receive(
	struct rtk_spi_controller *rtk_spi,
	void *prx_data, u32 length)
{
	int cnt_to_rx;
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);

	while (length) {
		if ((rtk_spi_get_status(rtk_spi)) & SPI_BIT_RFNE) {
			cnt_to_rx = rtk_spi_get_rx_cnt(rtk_spi);
			while (cnt_to_rx--) {
				if (data_frame_size > 8) {
					/*  16~9 bits mode */
					*((u16 *)(prx_data)) = (u16)rtk_spi_read_data(rtk_spi);
					prx_data = (void *)(((u16 *)prx_data) + 1);
				} else {
					/*  8~4 bits mode */
					*((u8 *)(prx_data)) = (u8)rtk_spi_read_data(rtk_spi);
					prx_data = (void *)(((u8 *)prx_data) + 1);
				}
				length--;
				if (length == 0) {
					rtk_spi_status_switch(rtk_spi, RTK_SPI_RX_DONE);
					spi_finalize_current_transfer(rtk_spi->controller);
					return 0;
				}
			}
		}
	}
	rtk_spi->spi_manage.rx_info.p_data_buf = prx_data;

	return length;
}

u32 rtk_spi_poll_trx(
	struct rtk_spi_controller *rtk_spi,
	void *prx_data, const void *ptx_data, u32 length)
{
	u32 rx_tmp_cnt, tx_cnt = 0, rx_cnt = 0;
	u32 data_frame_size = rtk_spi_get_data_frame_size(rtk_spi);
	int ret = 0;

	while (tx_cnt < length || rx_cnt < length) {
		if (tx_cnt < length) {
			if (rtk_spi_get_status(rtk_spi) & SPI_BIT_TFNF) {
				if (data_frame_size > 8) {
					rtk_spi_write_data(rtk_spi, *(u16 *)(ptx_data));
					ptx_data = (void *)(((u16 *)ptx_data) + 1);
				} else {
					rtk_spi_write_data(rtk_spi, *(u8 *)(ptx_data));
					ptx_data = (void *)(((u8 *)ptx_data) + 1);
				}
				tx_cnt++;
			}
		}

		if (rx_cnt < length) {
			while (rtk_spi_get_status(rtk_spi) & SPI_BIT_RFNE) {
				rx_tmp_cnt = rtk_spi_get_rx_cnt(rtk_spi);
				while(rx_tmp_cnt) {
					if (data_frame_size > 8) {
						*(u16*)(prx_data) = (u16)rtk_spi_read_data(rtk_spi);
						prx_data = (void *)(((u16 *)prx_data) + 1);
					} else {
						*(u8*)(prx_data) = (u8)rtk_spi_read_data(rtk_spi);
						prx_data = (void *)(((u8 *)prx_data) + 1);
					}

					rx_cnt++;
					rx_tmp_cnt--;
				}
			}
		}
	}

	rtk_spi_status_switch(rtk_spi, RTK_SPI_TX_DONE);
	spi_finalize_current_transfer(rtk_spi->controller);

	return ret;
}

void rtk_spi_slave_tx_cmd(
	struct rtk_spi_controller *rtk_spi, u32 Status)
{
	if (Status) {
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SLV_OE, ~SPI_BIT_SLV_OE);
	} else {
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SLV_OE, SPI_BIT_SLV_OE);
	}
}

void rtk_spi_dma_done_callback(void *data)
{
	struct rtk_spi_controller *rtk_spi = data;
	int check_times = 0;

	while (!mutex_trylock(&rtk_spi->spi_manage.dma_params.dma_lock)) {
	}

	/* DMA done. But still need a little time for spi transfer. */
	/* Check times to prevent lost contact accidently. */
	while (rtk_spi_busy_check(rtk_spi)) {
		check_times++;
		dev_dbg(rtk_spi->dev, "still transferring, wait.");
		if (check_times > 10) {
			break;
		}
	}

	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		rtk_spi_set_dma_enable(rtk_spi, DISABLE, SPI_TX_MODE);
	} else {
		rtk_spi_set_dma_enable(rtk_spi, DISABLE, SPI_RX_MODE);
	}

	rtk_spi->spi_manage.dma_params.gdma_status = RTK_SPI_GDMA_DONE;

	mutex_unlock(&rtk_spi->spi_manage.dma_params.dma_lock);

	/* Inform upper layer that dma spi is completed. */
	spi_finalize_current_transfer(rtk_spi->controller);

}

int rtk_spi_gdma_prepare(struct rtk_spi_controller *rtk_spi)
{
	int ret = 0;
	const char *name = "dma_spi";

	/* request chan. */
	if (!rtk_spi->spi_manage.dma_params.chan) {
		rtk_spi->spi_manage.dma_params.chan = dma_request_chan(rtk_spi->dev, name);
	}

	if (!rtk_spi->spi_manage.dma_params.chan) {
		dev_err(rtk_spi->dev, "failed to request ledc dma channel.\n");
		rtk_spi->spi_manage.dma_params.gdma_status = RTK_SPI_GDMA_UNPREPARED;
		mutex_unlock(&rtk_spi->spi_manage.dma_params.dma_lock);
		return -1;
	}

	dev_dbg(rtk_spi->dev, "request chan success for spi-%d.", rtk_spi->spi_manage.spi_index);

	return ret;
}

void rtk_spi_gdma_deinit(struct rtk_spi_controller *rtk_spi)
{
	mutex_lock(&rtk_spi->spi_manage.dma_params.dma_lock);

	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		rtk_spi_set_dma_enable(rtk_spi, DISABLE, SPI_TX_MODE);
	} else {
		rtk_spi_set_dma_enable(rtk_spi, DISABLE, SPI_RX_MODE);
	}

	if (rtk_spi->spi_manage.dma_params.config) {
		kfree(rtk_spi->spi_manage.dma_params.config);
		rtk_spi->spi_manage.dma_params.config = NULL;
	}

#if RTK_SPI_DMA_HELP
	if (rtk_spi->spi_manage.dma_params.help_dma_addr) {
		dma_unmap_single(rtk_spi->dev, rtk_spi->spi_manage.dma_params.help_dma_addr, rtk_spi->spi_manage.dma_params.dma_length,
						 rtk_spi->spi_manage.current_direction ? DMA_MEM_TO_DEV : DMA_DEV_TO_MEM);
		rtk_spi->spi_manage.dma_params.help_dma_addr = 0;
	}
#endif // RTK_SPI_DMA_HELP

	if (rtk_spi->spi_manage.dma_enabled && rtk_spi->spi_manage.dma_params.chan) {
		dma_release_channel(rtk_spi->spi_manage.dma_params.chan);
		rtk_spi->spi_manage.dma_params.chan = NULL;
		rtk_spi->spi_manage.dma_params.gdma_status = RTK_SPI_GDMA_UNPREPARED;
		dev_dbg(rtk_spi->dev, "release chan success for spi-%d.", rtk_spi->spi_manage.spi_index);
	}
	mutex_unlock(&rtk_spi->spi_manage.dma_params.dma_lock);

}

#if RTK_SPI_DMA_HELP
dma_addr_t rtk_spi_prepare_dma_addr(
	struct rtk_spi_controller *rtk_spi,
	void *buf, int size)
{
	dma_addr_t temp;
	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		temp = dma_map_single(rtk_spi->dev, buf, size, DMA_MEM_TO_DEV);
	} else {
		temp = dma_map_single(rtk_spi->dev, buf, size, DMA_DEV_TO_MEM);
	}

	return temp;
}
#endif // RTK_SPI_DMA_HELP

/* Accept only 8-bit-rule data here. */
int rtk_spi_can_dma(
	struct spi_controller *controller,
	struct spi_device *spi,
	struct spi_transfer *transfer)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);
	struct rtk_spi_gdma_parameters	*dma_params;
	int ret = USE_SPI_CORE_WAIT;

	if (transfer->len > MAX_DMA_LENGTH) {
		return 0;
	}

	dma_params = &rtk_spi->spi_manage.dma_params;

	mutex_lock(&dma_params->dma_lock);

	if (rtk_spi->spi_manage.dma_enabled && (dma_params->gdma_status == RTK_SPI_GDMA_UNPREPARED)) {
		ret = rtk_spi_gdma_prepare(rtk_spi);
		if (ret < 0) {
			dev_info(rtk_spi->dev, "DMA transfer is not prepared for spi-%d.\n", rtk_spi->spi_manage.spi_index);
			dma_params->gdma_status = RTK_SPI_GDMA_UNPREPARED;
			goto cannot_dma;
		}
		dma_params->gdma_status = RTK_SPI_GDMA_PREPARED;
	}

	if (dma_params->gdma_status == RTK_SPI_GDMA_UNPREPARED) {
		dev_info(rtk_spi->dev, "Dma has not been prepared for spi-%d.\n", rtk_spi->spi_manage.spi_index);
		goto cannot_dma;
	} else if (dma_params->gdma_status == RTK_SPI_GDMA_ONGOING) {
		dev_info(rtk_spi->dev, "Last dma is ongoing for spi-%d, please check.\n", rtk_spi->spi_manage.spi_index);
		goto cannot_dma;
	}

	if (!dma_params->config) {
		dma_params->config = kmalloc(sizeof(*dma_params->config), GFP_KERNEL);
	}

	dma_params->gdma_status = RTK_SPI_GDMA_ONGOING;
	dma_params->config->device_fc = 1;
	dma_params->dma_length = transfer->len;
	dma_params->config->dst_port_window_size = 0;
	dma_params->config->src_port_window_size = 0;

	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		/* DMA spi tx mode. */
		if (!transfer->tx_dma) {
			pr_err("There is no tx_dma address for spi-%d dma tx!", rtk_spi->spi_manage.spi_index);
#if RTK_SPI_DMA_HELP
			transfer->tx_dma = rtk_spi_prepare_dma_addr(rtk_spi, transfer->tx_buf, transfer->len);
			if (!transfer->tx_dma) {
				goto cannot_dma;
			}
			dma_params->help_dma_addr = transfer->tx_dma;
#else // RTK_SPI_DMA_HELP
			goto cannot_dma;
#endif // RTK_SPI_DMA_HELP
		}

		rtk_spi_enable_cmd(rtk_spi, DISABLE);
		rtk_spi_set_dma_level(rtk_spi, 1, 3);

		dma_params->config->src_addr = transfer->tx_dma;
		dma_params->config->direction = DMA_MEM_TO_DEV;
		dma_params->config->dst_addr = dma_params->spi_phy_addr + SPI_DATA_FIFO_ENRTY;
		dma_params->config->dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		dma_params->config->src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		dma_params->config->dst_maxburst = 4;
		dma_params->config->src_maxburst = 1;

		if (rtk_spi->spi_manage.spi_index == 0) {
			dma_params->config->slave_id = GDMA_HANDSHAKE_INTERFACE_SPI0_TX;
		} else {
			dma_params->config->slave_id = GDMA_HANDSHAKE_INTERFACE_SPI1_TX;
		}

	} else if (rtk_spi->spi_manage.current_direction == RTK_SPI_READ) {
		/* DMA spi rx mode. */
		if (!transfer->rx_dma) {
			pr_err("There is no rx_dma address for spi-%d dma rx!", rtk_spi->spi_manage.spi_index);
#if RTK_SPI_DMA_HELP
			transfer->rx_dma = rtk_spi_prepare_dma_addr(rtk_spi, transfer->rx_buf, transfer->len);
			if (!transfer->rx_dma) {
				goto cannot_dma;
			}
			dma_params->help_dma_addr = transfer->rx_dma;
#else // RTK_SPI_DMA_HELP
			goto cannot_dma;
#endif // RTK_SPI_DMA_HELP
		}

		rtk_spi_set_dma_level(rtk_spi, 56, 3);
		rtk_spi_set_sample_delay(rtk_spi, 1);

		dma_params->dma_length = transfer->len;
		dma_params->config->src_addr = rtk_spi->spi_manage.dma_params.spi_phy_addr + SPI_DATA_FIFO_ENRTY;
		dma_params->config->direction = DMA_DEV_TO_MEM;
		dma_params->config->dst_addr = transfer->rx_dma;
		dma_params->config->dst_port_window_size = 0;
		dma_params->config->dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		dma_params->config->src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		dma_params->config->dst_maxburst = 1;
		dma_params->config->src_maxburst = 4;

		if (rtk_spi->spi_manage.spi_index == 0) {
			dma_params->config->slave_id = GDMA_HANDSHAKE_INTERFACE_SPI0_RX;
		} else {
			dma_params->config->slave_id = GDMA_HANDSHAKE_INTERFACE_SPI1_RX;
		}
	} else {
		pr_err("Please confirm the transfer direction by setting one of the transfer buffer to NULL.");
		goto cannot_dma;
	}

	ret = dmaengine_slave_config(dma_params->chan, dma_params->config);
	if (ret < 0) {
		pr_info("dmaengine slave config fail. ");
		goto cannot_dma;
	}

	dma_params->txdesc = dmaengine_prep_dma_cyclic(dma_params->chan,
						 (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) ? dma_params->config->src_addr : dma_params->config->dst_addr,
						 dma_params->dma_length, transfer->len,
						 dma_params->config->direction, DMA_PREP_INTERRUPT);

	dma_params->txdesc->callback = rtk_spi_dma_done_callback;
	dma_params->txdesc->callback_param = rtk_spi;

	dmaengine_submit(dma_params->txdesc);
	dma_async_issue_pending(dma_params->chan);

	/* Open spi hw dma. */
	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		if (rtk_spi->spi_manage.is_slave) {
			rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SLV_OE, ~SPI_BIT_SLV_OE);
		}

		rtk_spi_enable_cmd(rtk_spi, ENABLE);
		rtk_spi_set_dma_enable(rtk_spi, ENABLE, SPI_TX_MODE);

	} else {
		rtk_spi_enable_cmd(rtk_spi, ENABLE);
		rtk_spi_set_dma_enable(rtk_spi, ENABLE, SPI_RX_MODE);
		if (!rtk_spi->spi_manage.is_slave) {
			/* Tx dummy0, if master rx is ongoing. */
			/* Freertos did not use dma tx dummy data in dma rx mode either.*/
			while (1) {
				if (rtk_spi_get_writeable(rtk_spi)) {
					rtk_spi_write_data(rtk_spi, 0);
					break;
				}
			}
		}
	}

cannot_dma:
	mutex_unlock(&dma_params->dma_lock);
	return 0;

}

int rtk_spi_transfer_slave(
	struct rtk_spi_controller *rtk_spi,
	struct spi_transfer *transfer,
	bool direction)
{
	/* Based on spi core mechenism, tx poll only, rx interrupt only. */
	int ret = USE_SPI_CORE_WAIT;

	rtk_spi_enable_cmd(rtk_spi, DISABLE);
	rtk_spi_set_tx_fifo_level(rtk_spi, rtk_spi->spi_param.tx_threshold_level);
	rtk_spi_set_rx_fifo_level(rtk_spi, rtk_spi->spi_param.rx_threshold_level);

	if (direction == SPI_TX_MODE) {
		/* slave spi tx mode. */
		/*  As a Slave mode, if the peer(Master) side is power off, the BUSY flag is always on */
		if (transfer->len >= RTK_SPI_FIFO_ALL) {
			rtk_spi_set_tx_fifo_level(rtk_spi, RTK_SPI_FIFO_HALF);
		} else {
			rtk_spi_set_tx_fifo_level(rtk_spi, transfer->len / 2);
		}

		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_BIT_SLV_OE, ~SPI_BIT_SLV_OE);
		rtk_spi_enable_cmd(rtk_spi, ENABLE);

		rtk_spi->spi_manage.tx_info.p_data_buf = transfer->tx_buf;
		rtk_spi->spi_manage.tx_info.data_len = transfer->len;
		rtk_spi_interrupt_write(rtk_spi, transfer->tx_buf, transfer->len);

#if RTK_SPI_REG_DUMP
		rtk_spi_reg_dump(rtk_spi);
#endif // RTK_SPI_REG_DUMP
		return ret;
	} else if (direction == SPI_RX_MODE) {
		/* slave spi rx mode. */
		if (transfer->len >= RTK_SPI_FIFO_ALL) {
			rtk_spi_set_rx_fifo_level(rtk_spi, RTK_SPI_FIFO_HALF);
		} else {
			rtk_spi_set_rx_fifo_level(rtk_spi, transfer->len / 2);
		}
		rtk_spi_enable_cmd(rtk_spi, ENABLE);
		rtk_spi_set_read_len(rtk_spi, transfer->len);

		rtk_spi->spi_manage.rx_info.p_data_buf = transfer->rx_buf;
		rtk_spi->spi_manage.rx_info.data_len = transfer->len;
		rtk_spi_interrupt_read(rtk_spi, transfer->rx_buf, transfer->len);

#if RTK_SPI_REG_DUMP
		rtk_spi_reg_dump(rtk_spi);
#endif // RTK_SPI_REG_DUMP
		return ret;
	} else {
		dev_info(rtk_spi->dev,
				 "Please confirm whether you want tx or rx by set another buf = NULL.");
		return -EINVAL;
	}
}

int rtk_spi_transfer_one(
	struct spi_controller *controller,
	struct spi_device *spi,
	struct spi_transfer *transfer)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);
	int ret = USE_SPI_CORE_WAIT;

	if (!transfer->tx_buf) {
		rtk_spi->spi_manage.current_direction = RTK_SPI_READ;
	} else if (!transfer->rx_buf) {
		rtk_spi->spi_manage.current_direction = RTK_SPI_WRITE;
	} else if (transfer->rx_buf && transfer->tx_buf) {
		rtk_spi->spi_manage.current_direction = RTK_SPI_TRX;
	} else {
		dev_info(rtk_spi->dev, "Please confirm whether you want tx or rx by set another buf = NULL.");
		rtk_spi->controller->cur_msg_prepared = 0;
		rtk_spi->spi_manage.current_direction = RTK_SPI_INVALID;
		return -EINVAL;
	}
	dev_dbg(rtk_spi->dev, "Message prepare direction = %d", rtk_spi->spi_manage.current_direction);

	if (rtk_spi->spi_manage.spi_prepared == SPI_NOT_SETUP) {
		dev_info(rtk_spi->dev, "Please call prepare_transfer_hardware for spi-%d first.\n", rtk_spi->spi_manage.spi_index);
		return -EINVAL;
	}

	if (transfer->bits_per_word) {
		dev_dbg(rtk_spi->dev, "upper level given bits_per_word = %d", transfer->bits_per_word);
		rtk_spi_set_frame_size(rtk_spi, transfer->bits_per_word - 1);
	} else {
		dev_info(rtk_spi->dev, "Please set bits_per_word for spi-%d.\n", rtk_spi->spi_manage.spi_index);
		return -EINVAL;
	}

	if (transfer->speed_hz) {
		rtk_spi->spi_param.clock_divider = MAX_SSI_CLOCK / transfer->speed_hz + (MAX_SSI_CLOCK % transfer->speed_hz == 0 ? 0 : 1);
		if (rtk_spi->spi_param.clock_divider % 2) {
			rtk_spi->spi_param.clock_divider++;
		}
		dev_dbg(rtk_spi->dev, "upper level given speed_hz = %d", transfer->speed_hz);
		dev_dbg(rtk_spi->dev, "clock divider caculated = %d", rtk_spi->spi_param.clock_divider);
		dev_dbg(rtk_spi->dev, "actual speed_hz = %d", MAX_SSI_CLOCK / rtk_spi->spi_param.clock_divider);
		rtk_spi_set_baud_div(rtk_spi, rtk_spi->spi_param.clock_divider);
	}

	if (rtk_spi->spi_manage.dma_enabled) {
		ret = rtk_spi_can_dma(controller, spi, transfer);
		if (ret) {
			ret = USE_SPI_CORE_WAIT;
		} else {
			ret = -EINVAL;
		}
		return ret;
	}

	dev_dbg(rtk_spi->dev, "slave mode = %d", rtk_spi->spi_manage.is_slave);
	if (rtk_spi->spi_manage.is_slave) {
		if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
			ret = rtk_spi_transfer_slave(rtk_spi, transfer, SPI_TX_MODE);
			return ret;
		} else if (rtk_spi->spi_manage.current_direction == RTK_SPI_READ) {
			ret = rtk_spi_transfer_slave(rtk_spi, transfer, SPI_RX_MODE);
			return ret;
		} else {
			pr_info("Spi slave read or write mode is not clear.");
			return -EINVAL;
		}
	}

	rtk_spi->spi_manage.transfer_status = RTK_SPI_ONGOING;
	rtk_spi->current_trans = transfer;
	if (rtk_spi->spi_manage.current_direction == RTK_SPI_WRITE) {
		/* spi tx mode. */
		rtk_spi_enable_cmd(rtk_spi, DISABLE);
		if (transfer->len >= RTK_SPI_FIFO_ALL) {
			rtk_spi_set_tx_fifo_level(rtk_spi, RTK_SPI_FIFO_HALF);
		} else {
			rtk_spi_set_tx_fifo_level(rtk_spi, transfer->len / 2);
		}
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_MASK_TMOD, SPI_TMOD(1));
		rtk_spi_enable_cmd(rtk_spi, ENABLE);

		rtk_spi->spi_manage.tx_info.p_data_buf = transfer->tx_buf;
		rtk_spi->spi_manage.tx_info.data_len = transfer->len;
		if (rtk_spi->spi_manage.spi_poll_mode) {
			ret = rtk_spi_poll_send(rtk_spi, transfer->tx_buf, transfer->len);
		} else {
			ret = rtk_spi_interrupt_write(rtk_spi, transfer->tx_buf, transfer->len);
		}
	} else if (rtk_spi->spi_manage.current_direction == RTK_SPI_READ) {
		/* spi rx mode. */
		rtk_spi_enable_cmd(rtk_spi, DISABLE);
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_MASK_TMOD, SPI_TMOD(2));
		rtk_spi_set_read_len(rtk_spi, transfer->len);

		if (transfer->len >= RTK_SPI_FIFO_ALL) {
			rtk_spi_set_rx_fifo_level(rtk_spi, RTK_SPI_FIFO_HALF);
		} else {
			rtk_spi_set_rx_fifo_level(rtk_spi, transfer->len / 2);
		}

		rtk_spi->spi_manage.rx_info.p_data_buf = transfer->rx_buf;
		rtk_spi->spi_manage.rx_info.data_len = transfer->len;
		if (rtk_spi->spi_manage.spi_poll_mode) {
			/* tx dummy0 before poll read. */
			while (1) {
				if (rtk_spi_get_writeable(rtk_spi)) {
					rtk_spi_write_data(rtk_spi, 0);
					break;
				}
			}
			rtk_spi->spi_manage.rx_info.data_len =  rtk_spi_poll_receive(rtk_spi, rtk_spi->spi_manage.rx_info.p_data_buf, rtk_spi->spi_manage.rx_info.data_len);
		} else {
			ret = rtk_spi_interrupt_read(rtk_spi, transfer->rx_buf, transfer->len);
			/* tx dummy0 once to start the rx process. */
			while (1) {
				if (rtk_spi_get_writeable(rtk_spi)) {
					rtk_spi_write_data(rtk_spi, 0);
					break;
				}
			}
		}
	} else if (rtk_spi->spi_manage.current_direction == RTK_SPI_TRX) {
		/* spi loopback mode. */
		rtk_spi_enable_cmd(rtk_spi, DISABLE);
		rtk_spi_set_rx_fifo_level(rtk_spi, rtk_spi->spi_param.rx_threshold_level);
		rtk_spi_set_tx_fifo_level(rtk_spi, rtk_spi->spi_param.rx_threshold_level);
		rtk_spi_reg_update(rtk_spi->base, SPI_CTRLR0, SPI_MASK_TMOD, SPI_TMOD(0));
		rtk_spi_set_read_len(rtk_spi, transfer->len);

		rtk_spi->spi_manage.tx_info.p_data_buf = transfer->tx_buf;
		rtk_spi->spi_manage.tx_info.data_len = transfer->len;
		rtk_spi->spi_manage.rx_info.p_data_buf = transfer->rx_buf;
		rtk_spi->spi_manage.rx_info.data_len = transfer->len;
		if (rtk_spi->spi_manage.spi_poll_mode) {
			ret = rtk_spi_poll_trx(rtk_spi, transfer->rx_buf, transfer->tx_buf, transfer->len);
		} else {
			ret = rtk_spi_interrupt_read(rtk_spi, transfer->rx_buf, transfer->len);
			ret = rtk_spi_interrupt_write(rtk_spi, transfer->tx_buf, transfer->len);
		}
	} else {
		dev_info(rtk_spi->dev,
				 "Please confirm whether you want tx or rx by set another buf = NULL.");
		return -EINVAL;
	}

#if RTK_SPI_REG_DUMP
	rtk_spi_reg_dump(rtk_spi);
#endif // RTK_SPI_REG_DUMP

	return ret;
}

static void spi_init_board_info(
	struct rtk_spi_controller *rtk_spi,
	struct spi_board_info *rtk_spi_chip)
{
	strcpy(rtk_spi_chip->modalias, "spidev");
	rtk_spi_chip->chip_select = rtk_spi->spi_manage.spi_default_cs;
	rtk_spi_chip->bus_num = rtk_spi->spi_manage.spi_index;
	rtk_spi_chip->controller_data = rtk_spi->controller;
	rtk_spi_chip->max_speed_hz = 100000000;
	rtk_spi_chip->mode = 0;
	rtk_spi_chip->platform_data = rtk_spi;
}

static int rtk_spi_slave_abort(struct spi_controller *controller)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);

	dev_dbg(rtk_spi->dev, "rtk spi-%d slave abort.", rtk_spi->spi_manage.spi_index);
	rtk_spi_enable_cmd(rtk_spi, DISABLE);

	if (rtk_spi->controller && rtk_spi->controller->cur_msg_prepared) {
		/* Slave transfer abort manually. */

		if (rtk_spi->spi_manage.dma_enabled) {
			rtk_spi_gdma_deinit(rtk_spi);
		}

		spi_finalize_current_transfer(rtk_spi->controller);
	}

	return 0;
}

static void rtk_spi_handle_err(struct spi_controller *controller,
							   struct spi_message *msg)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);
	/* Disable the SPI */

	/* Clear and disable interrupts and service requests */

	/*
	 * Stop the DMA if running. Note DMA callback handler may have unset
	 * the dma_running already, which is fine as stopping is not needed
	 * then but we shouldn't rely this flag for anything else than
	 * stopping. For instance to differentiate between PIO and DMA
	 * transfers.
	 */

	rtk_spi->spi_manage.transfer_status = RTK_SPI_DONE_WITH_ERROR;

}

/* spi.c spi_set_cs: input signal enbale: 0 means enable, 1 means disable. */
void rtk_spi_set_cs(
	struct spi_device *spi,
	bool enable)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(spi->controller);

	dev_dbg(rtk_spi->dev, "enter %s to %s cs", __FUNCTION__, !enable ? "enable" : "disable");
	if (spi->chip_select) {
		dev_warn(rtk_spi->dev, "set cs id = %d", spi->chip_select);
		dev_warn(rtk_spi->dev, "The hardware slave-select line is dedicated for general spi. One master supports only one slave actually.");
	}

	if (!rtk_spi->spi_manage.is_slave && !enable) {
		rtk_spi_set_slave_enable(rtk_spi, spi->chip_select);
		gpio_set_value(rtk_spi->spi_manage.spi_cs_pin, 0);
	} else if (!rtk_spi->spi_manage.is_slave && enable) {
		/* disbale cs is not recommended, change cs id directly. */
		gpio_set_value(rtk_spi->spi_manage.spi_cs_pin, 1);
	}
}

static int rtk_spi_prepare_message(
	struct spi_controller *controller,
	struct spi_message *msg)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);
	int ret = 0;

	rtk_spi->controller->cur_msg_prepared = 1;

	return ret;
}

static int rtk_spi_unprepare_message(
	struct spi_controller *controller,
	struct spi_message *msg)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(controller);

	if (rtk_spi->spi_manage.dma_enabled) {
		rtk_spi_gdma_deinit(rtk_spi);
	}

	if (rtk_spi->controller && rtk_spi->controller->cur_msg_prepared) {
		rtk_spi->controller->cur_msg_prepared = 0;
	}

	return 0;
}

static int rtk_spi_setup(struct spi_device *spi)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(spi->controller);
	int ret = 0;

	if (rtk_spi->spi_manage.spi_prepared) {
		dev_info(rtk_spi->dev, "spi-%d has already been prepared.\n", rtk_spi->spi_manage.spi_index);
		return -EINVAL;
	}

	ret = clk_prepare_enable(rtk_spi->clk);
	if (ret) {
		dev_err(rtk_spi->dev, "Failed to prepare_enable clock\n");
		return ret;
	}

	rtk_spi_init_hw(rtk_spi);
	rtk_spi->spi_manage.spi_prepared = SPI_SETUP_DONE;

	return ret;
}

static void rtk_spi_cleanup(struct spi_device *spi)
{
	struct rtk_spi_controller *rtk_spi = spi_controller_get_devdata(spi->controller);

	clk_disable_unprepare(rtk_spi->clk);

	rtk_spi->spi_manage.spi_prepared = SPI_NOT_SETUP;
}

static int rtk_spi_probe(struct platform_device *pdev)
{
	struct device			*dev = &pdev->dev;
	struct rtk_spi_controller	*rtk_spi;
	struct spi_controller		*controller;
	struct resource			*res;
	struct device_node		*np = pdev->dev.of_node;

	int ret, status;

	rtk_spi = devm_kzalloc(&pdev->dev, sizeof(struct rtk_spi_controller), GFP_KERNEL);
	if (!rtk_spi) {
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rtk_spi->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(rtk_spi->base)) {
		return PTR_ERR(rtk_spi->base);
	}

	if (!role_set_base) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		role_set_base = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(role_set_base)) {
			return PTR_ERR(role_set_base);
		}
	}

	rtk_spi->clk = devm_clk_get(&pdev->dev, "rtk_spi_clk");
	if (IS_ERR(rtk_spi->clk)) {
		dev_err(&pdev->dev, "Error: Missing controller clock\n");
		return PTR_ERR(rtk_spi->clk);
	}

	rtk_spi->dev = &pdev->dev;
	rtk_spi_struct_init(rtk_spi, np);

	if (rtk_spi->spi_manage.is_slave) {
		controller = spi_alloc_slave(dev, sizeof(struct rtk_spi_controller));
	} else {
		controller = spi_alloc_master(dev, sizeof(struct rtk_spi_controller));
	}

	if (!controller) {
		dev_err(&pdev->dev, "cannot alloc spi_controller\n");
		return -ENOMEM;
	}

	rtk_spi->controller = controller;

	controller->dev.of_node = pdev->dev.of_node;
	/* the spi->mode bits understood by this driver: */
	controller->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_LOOP;

	controller->bus_num = rtk_spi->spi_manage.spi_index;
	controller->dma_alignment = DMA_ALIGNMENT;
	controller->cleanup = rtk_spi_cleanup;
	controller->setup = rtk_spi_setup;
	controller->set_cs = rtk_spi_set_cs;
	controller->transfer_one = rtk_spi_transfer_one;
	controller->slave_abort = rtk_spi_slave_abort;
	controller->handle_err = rtk_spi_handle_err;
	controller->prepare_message = rtk_spi_prepare_message;
	controller->unprepare_message = rtk_spi_unprepare_message;
	controller->auto_runtime_pm = false;
	controller->num_chipselect = rtk_spi->spi_manage.max_cs_num;
	controller->flags = SPI_CONTROLLER_MUST_RX | SPI_CONTROLLER_MUST_TX;
#if RTK_LINUX_SPI_DMA_ENTRY
	/* If spi driver support dma mode, controller will call can_dma prior to transfer one. */
	controller->can_dma = rtk_spi_can_dma;
#endif // RTK_LINUX_SPI_DMA_ENTRY
	controller->max_speed_hz = 50000000;
	controller->min_speed_hz = 762;

	rtk_spi->irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(&pdev->dev, rtk_spi->irq, rtk_spi_interrupt_handler, 0, dev_name(&pdev->dev), rtk_spi);
	if (ret) {
		dev_err(&pdev->dev, "unable to request IRQ\n");
		return ret;
	}

#if RTK_SPI_TODO
	pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);
#endif // RTK_SPI_TODO

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, rtk_spi);
	spi_controller_set_devdata(controller, rtk_spi);

	/* Attention!!! If spi controller is registered for kernel device, register board inifo in child-node probe. */
	if (!rtk_spi->spi_manage.spi_for_kernel) {
		/* Register board info for userspace spidev. */
		spi_init_board_info(rtk_spi, &rtk_spi->rtk_spi_chip);
		spi_register_board_info(&rtk_spi->rtk_spi_chip, 1);
	}

	status = spi_register_controller(controller);
	if (status != 0) {
		dev_err(&pdev->dev, "problem registering spi controller\n");
#if RTK_SPI_TODO
		goto out_error_pm_runtime_enabled;
#else
		goto out_error_controller_alloc;
#endif // RTK_SPI_TODO
	}

	return status;

#if RTK_SPI_TODO
out_error_pm_runtime_enabled:
	pm_runtime_disable(&pdev->dev);
#endif // RTK_SPI_TODO

out_error_controller_alloc:
	spi_controller_put(controller);
	return status;
}

static int rtk_spi_remove(struct platform_device *pdev)
{
	struct rtk_spi_controller *rtk_spi = platform_get_drvdata(pdev);

	if (!rtk_spi) {
		return 0;
	}

	rtk_spi_enable_cmd(rtk_spi, DISABLE);

	if (rtk_spi->spi_manage.dma_enabled) {
		rtk_spi_gdma_deinit(rtk_spi);
		mutex_destroy(&rtk_spi->spi_manage.dma_params.dma_lock);
	}

	if (rtk_spi->controller) {
		spi_unregister_controller(rtk_spi->controller);
	}

	return 0;
}

#if RTK_SPI_TODO
static int rtk_spi_suspend(struct device *dev)
{
	struct rtk_spi_controller *rtk_spi = dev_get_drvdata(dev);
	int status;

	status = spi_controller_suspend(rtk_spi->controller);
	if (status != 0) {
		return status;
	}
	//rtk_spi_write(rtk_spi, REG, 0);

	if (!pm_runtime_suspended(dev)) {
		clk_disable_unprepare(rtk_spi->clk);
	}

	return 0;
}

static int rtk_spi_resume(struct device *dev)
{
	struct rtk_spi_controller *rtk_spi = dev_get_drvdata(dev);
	int status;

	/* Enable the SSP clock */
	if (!pm_runtime_suspended(dev)) {
		status = clk_prepare_enable(rtk_spi->clk);
		if (status) {
			return status;
		}
	}

	/* Start the queue running */
	return spi_controller_resume(rtk_spi->controller);
}

static int rtk_spi_runtime_suspend(struct device *dev)
{
	struct rtk_spi_controller *rtk_spi = dev_get_drvdata(dev);

	clk_disable_unprepare(rtk_spi->clk);
	return 0;
}

static int rtk_spi_runtime_resume(struct device *dev)
{
	struct rtk_spi_controller *rtk_spi = dev_get_drvdata(dev);
	int status;

	status = clk_prepare_enable(rtk_spi->clk);
	return status;
}

static const struct dev_pm_ops rtk_spi_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(rtk_spi_suspend, rtk_spi_resume)
	SET_RUNTIME_PM_OPS(rtk_spi_runtime_suspend,
					   rtk_spi_runtime_resume, NULL)
};
#endif // RTK_SPI_TODO

static const struct of_device_id rtk_spi_of_match[] = {
	{.compatible = "realtek,amebad2-rtk-spi"},
	{},
};
MODULE_DEVICE_TABLE(of, rtk_spi_of_match);

static struct platform_driver rtk_spi_driver = {
	.driver = {
		.name	= "rtk-spi",
#if RTK_SPI_TODO
		.pm	= &rtk_spi_pm_ops,
#endif // RTK_SPI_TODO
		.of_match_table = of_match_ptr(rtk_spi_of_match),
	},
	.probe = rtk_spi_probe,
	.remove = rtk_spi_remove,
};

module_platform_driver(rtk_spi_driver);
MODULE_AUTHOR("realtek");
MODULE_DESCRIPTION("RTK SPI driver general.");
MODULE_LICENSE("GPL v2");
