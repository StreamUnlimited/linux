// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek I2C master support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include "i2c-realtek.h"
#include <linux/mfd/rtk-timer.h>

void rtk_i2c_master_send(
	struct rtk_i2c_hw_params *i2c_param,
	u8 *pbuf, u8  i2c_cmd,
	u8 i2c_stop, u8  i2c_restart)
{
	rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD,
				   * (pbuf) | (i2c_restart << 10) |
				   (i2c_cmd << 8) | (i2c_stop << 9));
}

#if RTK_I2C_TODO
void rtk_i2c_master_send_null_data(
	struct rtk_i2c_hw_params *i2c_param,
	u8 *pbuf, u8 i2c_cmd, u8 i2c_stop, u8 i2c_restart)
{
	rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD,
				   * (pbuf) | (1 << 11) | (i2c_restart << 10) |
				   (i2c_cmd << 8) | (i2c_stop << 9));
}
#endif //RTK_I2C_TODO

void rtk_i2c_set_slave_addr(
	struct rtk_i2c_hw_params *i2c_param, u16 address)
{
	u32 tar;

	tar = rtk_i2c_readl(i2c_param->i2c_dev->base, IC_TAR) & ~(I2C_MASK_IC_TAR);

	/*set target address to generate start signal*/
	rtk_i2c_writel(i2c_param->i2c_dev->base, IC_TAR, (address & I2C_MASK_IC_TAR) | tar);
}

static hal_status rtk_i2c_is_timeout(struct rtk_i2c_dev *i2c_dev)
{
	hal_status status;

	if (i2c_dev->i2c_manage.timeout == 0) {
		status = HAL_TIMEOUT;
	} else {
		status = HAL_OK;
	}
	return status;
}

bool rtk_i2c_master_irq_wait_timeout(struct rtk_i2c_dev *i2c_dev)
{
	if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
		pr_err("I2C-%d timeout. Reason: wait irq timeout.\n", i2c_dev->i2c_param.i2c_index);
		rtk_i2c_interrupt_config(&i2c_dev->i2c_param, 0x1FFF, DISABLE);
		rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);
		rtk_i2c_flow_deinit(i2c_dev);
		rtk_i2c_reg_update(i2c_dev->base, IC_DATA_CMD, I2C_BIT_CMD_STOP, I2C_BIT_CMD_STOP);
		rtk_i2c_init_hw(&i2c_dev->i2c_param);
		i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
		return 1;
	} else {
        return 0;
    }
}

static void rtk_i2c_hw_timer_timeout(struct rtk_i2c_dev *i2c_dev)
{
	pr_err("%s: %s i2c-%d timeout. ", __FUNCTION__, (i2c_dev->i2c_param.i2c_master == I2C_SLAVE_MODE) ? "slave" : "master", i2c_dev->i2c_param.i2c_index);
	i2c_dev->i2c_manage.timeout = 0;
	rtk_gtimer_int_clear(i2c_dev->i2c_manage.timer_index);
}

static void rtk_i2c_dev_status_handler(
	hal_status dev_status, int dev_id)
{
	switch (dev_status) {
	case HAL_TIMEOUT:
		pr_err("I2C-%d system waits timeout.", dev_id);
		return;
	case HAL_ERR_HW:
		pr_err("I2C-%d system occurs hw error.", dev_id);
		return;
	case HAL_ERR_PARA:
		pr_err("I2C-%d system occurs parameter error.", dev_id);
		return;
	case HAL_ERR_MEM:
		pr_err("I2C-%d system occurs memory error.", dev_id);
		return;
	case HAL_ERR_UNKNOWN:
		pr_err("I2C-%d system occurs unknown error.", dev_id);
		return;
	default:
		pr_debug("This status is not expected to handle.");
		return;
	}
}

static int rtk_i2c_release_bus(struct i2c_adapter *i2c_adap)
{
	struct rtk_i2c_dev *i2c_dev = i2c_get_adapdata(i2c_adap);

	dev_warn(i2c_dev->dev, "Trying to recover i2c-%d\n", i2c_dev->i2c_param.i2c_index);

	rtk_i2c_enable_cmd(&i2c_dev->i2c_param, DISABLE);
	rtk_i2c_init_hw(&i2c_dev->i2c_param);
	i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;

	return 0;
}

static int rtk_i2c_wait_free_bus(struct rtk_i2c_dev *i2c_dev)
{
	int ret;

	if (i2c_dev->i2c_manage.dev_status == I2C_STS_IDLE || i2c_dev->i2c_manage.dev_status == I2C_STS_UNINITIAL) {
		dev_dbg(i2c_dev->dev, "i2c-%d is free to use.", i2c_dev->i2c_param.i2c_index);
		return 0;
	}

	dev_dbg(i2c_dev->dev, "i2c-%d is busy, current status = %x", i2c_dev->i2c_param.i2c_index, i2c_dev->i2c_manage.dev_status);

	ret = rtk_i2c_release_bus(&i2c_dev->adap);
	if (ret) {
		dev_err(i2c_dev->dev, "Failed to recover the bus (%d)\n", ret);
		return ret;
	}

	return -EBUSY;
}

void rtk_i2c_isr_master_handle_tx_empty(struct rtk_i2c_dev *i2c_dev)
{
    int retry = 0;
    u8 i2c_stop = 0;

	/* To check I2C master TX data length. If all the data are transmitted,
	mask all the interrupts and invoke the user callback */
	if (!i2c_dev->i2c_manage.tx_info.data_len) {
		retry = 0;
		while (0 == rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFE)) {
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 1", __FUNCTION__);
				return;
			}
		}

		/* I2C Disable TX Related Interrupts */
		rtk_i2c_interrupt_config(&i2c_dev->i2c_param, (I2C_BIT_M_TX_ABRT | I2C_BIT_M_TX_EMPTY |
								 I2C_BIT_M_TX_OVER), DISABLE);

		/* Clear all I2C pending interrupts */
		rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);
		/* Update I2C device status */
		i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;
		complete(&i2c_dev->xfer_completion);
	}

	if (i2c_dev->i2c_manage.tx_info.data_len > 0) {
#if RTK_I2C_DEBUG_EXTEND
		dev_dbg(i2c_dev->dev, "left tx data = %d", i2c_dev->i2c_manage.tx_info.data_len);
#endif // RTK_I2C_DEBUG_EXTEND
		/* Update I2C device status */
		i2c_dev->i2c_manage.dev_status = I2C_STS_TX_ING;

		/* Check I2C TX FIFO status. If it's not full, one byte data will be written into it. */
		if (rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFNF)) {
			i2c_stop = I2C_STOP_DIS;
			if ((i2c_dev->i2c_manage.tx_info.data_len == 1) &&
				((i2c_dev->i2c_manage.i2c_extend & I2C_EXD_MTR_HOLD_BUS) == 0)) {
				i2c_stop = I2C_STOP_EN;
			}

			rtk_i2c_master_send(&i2c_dev->i2c_param, i2c_dev->i2c_manage.tx_info.p_data_buf,
								I2C_WRITE_CMD, i2c_stop, 0);

			i2c_dev->i2c_manage.tx_info.p_data_buf++;
			i2c_dev->i2c_manage.tx_info.data_len--;

			retry = 0;
			while (!rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFE)) {
				if (rtk_i2c_get_raw_interrupt(&i2c_dev->i2c_param) & I2C_BIT_TX_ABRT) {
					return;
				}
				retry++;
				if (retry > SOWFTWARE_MAX_RETRYTIMES) {
					pr_err("retry overflow: %s - 2", __FUNCTION__);
					return;
				}
			}
		}
	}
}

void rtk_i2c_isr_master_handle_rx_full(struct rtk_i2c_dev *i2c_dev)
{
	u8 i2c_stop;
	int retry = 0;

    /* Check if the receive transfer is NOT finished. If it is not, check if there
    is data in the RX FIFO and move the data from RX FIFO to user data buffer*/
    if (i2c_dev->i2c_manage.rx_info.data_len > 0) {

        /* Update I2C device status */
        i2c_dev->i2c_manage.dev_status = I2C_STS_RX_ING;

        while (1) {
            if (rtk_i2c_check_flag_state(&i2c_dev->i2c_param, (BIT_RFNE | BIT_RFF))) {

                *(i2c_dev->i2c_manage.rx_info.p_data_buf) = rtk_i2c_receive_data(&i2c_dev->i2c_param);

                i2c_dev->i2c_manage.rx_info.p_data_buf++;
                i2c_dev->i2c_manage.rx_info.data_len--;

                if (rtk_i2c_readl(i2c_dev->base, IC_RXFLR) == 0) {
                    dev_dbg(i2c_dev->dev, "Rx signal discontinuity.");
                    break;
                }
            } else if ((rtk_i2c_get_raw_interrupt(&i2c_dev->i2c_param)
                        & (I2C_BIT_RX_OVER | I2C_BIT_RX_UNDER)) != 0) {
                break;
            } else {
                if (!rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_RFNE)) {
                    break;
                }
            }
            retry++;
            if (retry > SOWFTWARE_MAX_RETRYTIMES) {
                pr_err("retry overflow: %s - 1", __FUNCTION__);
                return;
            }
        }
    }

    /* To check I2C master RX data length. If all the data are received,
    mask all the interrupts and invoke the user callback.
    Otherwise, the master should send another Read Command to slave for
    the next data byte receiving. */
    if (!i2c_dev->i2c_manage.rx_info.data_len) {
        /* I2C Disable RX Related Interrupts */
        rtk_i2c_interrupt_config(&i2c_dev->i2c_param, (I2C_BIT_M_RX_FULL | I2C_BIT_M_RX_OVER |
                                    I2C_BIT_M_RX_UNDER | I2C_BIT_M_TX_ABRT), DISABLE);
        /* Clear all I2C pending interrupts */
        rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);
        /* Update I2C device status */
        i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;
    } else {
        /* If TX FIFO is not full, another Read Command is written into it. */
        if (rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFNF)) {
            if (i2c_dev->i2c_manage.master_rd_cmd_cnt > 0) {
                i2c_stop = I2C_STOP_DIS;
                if ((i2c_dev->i2c_manage.master_rd_cmd_cnt == 1) && ((i2c_dev->i2c_manage.i2c_extend & I2C_EXD_MTR_HOLD_BUS) == 0)) {
                    i2c_stop = I2C_STOP_EN;
                }
                i2c_dev->i2c_manage.master_rd_cmd_cnt--;

                rtk_i2c_master_send(&i2c_dev->i2c_param, i2c_dev->i2c_manage.rx_info.p_data_buf,
                                    I2C_READ_CMD, i2c_stop, 0);
            }
        }
    }
}

void rtk_i2c_master_write(
	struct rtk_i2c_hw_params *i2c_param, u8 *pbuf, u8 len)
{
	u8 cnt = 0;
	int retry = 0;

	/* Write in the DR register the data to be sent */
	for (cnt = 0; cnt < len; cnt++) {
		retry = 0;
		while ((rtk_i2c_check_flag_state(i2c_param, BIT_TFNF)) == 0) {
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 1", __FUNCTION__);
				return;
			}
		}

		if (cnt >= len - 1) {
			/*generate stop signal*/
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*pbuf++) | (1 << 9));
		} else {
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*pbuf++));
		}
	}

	retry = 0;
	while ((rtk_i2c_check_flag_state(i2c_param, BIT_TFE)) == 0) {
		retry++;
		if (retry > SOWFTWARE_MAX_RETRYTIMES) {
			pr_err("retry overflow: %s - 2", __FUNCTION__);
			return;
		}
	}
}

u8 rtk_i2c_master_write_brk(
	struct rtk_i2c_hw_params *i2c_param, u8 *pbuf, u8 len)
{
	u8 cnt = 0;
	int retry = 0;

	/* Write in the DR register the data to be sent */
	for (cnt = 0; cnt < len; cnt++) {
		retry = 0;
		while ((rtk_i2c_check_flag_state(i2c_param, BIT_TFNF)) == 0) {
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 1", __FUNCTION__);
				return 0;
			}
		}

		if (cnt >= len - 1) {
			/*generate stop signal*/
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*pbuf++) | (1 << 9));
		} else {
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*pbuf++));
		}

		retry = 0;
		while ((rtk_i2c_check_flag_state(i2c_param, BIT_TFE)) == 0) {
			if (rtk_i2c_get_raw_interrupt(i2c_param) & I2C_BIT_TX_ABRT) {
				rtk_i2c_clear_all_interrupts(i2c_param);
				return cnt;
			}
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 2", __FUNCTION__);
				return 0;
			}
		}
	}

	return cnt;
}

#if RTK_I2C_TODO
void rtk_i2c_master_read_dw(
	struct rtk_i2c_hw_params *i2c_param, u8 *pbuf, u8 len)
{
	u8 cnt = 0;
	int retry = 0;

	/* read in the DR register the data to be received */
	for (cnt = 0; cnt < len; cnt++) {
		if (cnt >= len - 1) {
			/* generate stop singal */
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, 0x0003 << 8);
		} else {
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, 0x0001 << 8);
		}

		retry = 0;
		/* read data */
		if (cnt > 0) {
			/* wait for I2C_FLAG_RFNE flag */
			while ((rtk_i2c_check_flag_state(i2c_param, BIT_RFNE)) == 0) {
				retry++;
				if (retry > SOWFTWARE_MAX_RETRYTIMES) {
					pr_err("retry overflow: %s - 1", __FUNCTION__);
					return;
				}
			}
			*pbuf++ = (u8) rtk_i2c_readl(i2c_param->i2c_dev->base, IC_DATA_CMD);
		}
	}

	retry = 0;
	/* recv last data and NACK */
	while ((rtk_i2c_check_flag_state(i2c_param, BIT_RFNE)) == 0) {
		retry++;
		if (retry > SOWFTWARE_MAX_RETRYTIMES) {
			pr_err("retry overflow: %s - 2", __FUNCTION__);
			return;
		}
	}

	*pbuf++ = (u8) rtk_i2c_readl(i2c_param->i2c_dev->base, IC_DATA_CMD);
}
#endif // RTK_I2C_TODO

u8 rtk_i2c_master_read(
	struct rtk_i2c_hw_params *i2c_param, u8 *pbuf, u8 len)
{
	u8 cnt = 0;
	int retry = 0;

	/* read in the DR register the data to be received */
	for (cnt = 0; cnt < len; cnt++) {
		if (cnt >= len - 1) {
			/* generate stop singal */
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, 0x0003 << 8);
		} else {
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, 0x0001 << 8);
		}

		/* wait for I2C_FLAG_RFNE flag */
		while ((rtk_i2c_check_flag_state(i2c_param, BIT_RFNE)) == 0) {
			if (rtk_i2c_get_raw_interrupt(i2c_param) & I2C_BIT_TX_ABRT) {
				rtk_i2c_clear_all_interrupts(i2c_param);
				return cnt;
			}
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 1", __FUNCTION__);
				return 0;
			}
		}
		*pbuf++ = (u8) rtk_i2c_readl(i2c_param->i2c_dev->base, IC_DATA_CMD);
	}

	return cnt;
}

void rtk_i2c_master_repeat_read(
	struct rtk_i2c_hw_params *i2c_param,
	u8 *p_write_buf, u8 write_len,
	u8 *p_read_buf, u8 read_len)
{

	u8 cnt = 0;
	int retry = 0;

	/* write in the DR register the data to be sent */
	for (cnt = 0; cnt < write_len; cnt++) {
		while (!(rtk_i2c_check_flag_state(i2c_param, BIT_TFNF))) {
			retry++;
			if (retry > SOWFTWARE_MAX_RETRYTIMES) {
				pr_err("retry overflow: %s - 1", __FUNCTION__);
				return;
			}
		}
		retry = 0;

		if (cnt >= write_len - 1) {
			/*generate restart signal*/
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*p_write_buf++) | (1 << 10));
		} else {
			rtk_i2c_writel(i2c_param->i2c_dev->base, IC_DATA_CMD, (*p_write_buf++));
		}
	}

	retry = 0;
	/*Wait I2C TX FIFO not full*/
	while ((rtk_i2c_check_flag_state(i2c_param, BIT_TFNF)) == 0) {
		retry++;
		if (retry > SOWFTWARE_MAX_RETRYTIMES) {
			pr_err("retry overflow: %s - 1", __FUNCTION__);
			return;
		}
	}

	rtk_i2c_master_read(i2c_param, p_read_buf, read_len);
}

static hal_status rtk_i2c_send_poll_master(
	struct rtk_i2c_dev *i2c_dev)
{
	u32 data_send;
    int retry;

	/* Send data till the TX buffer data length is zero */
	i2c_dev->i2c_manage.dev_status = I2C_STS_TX_ING;

REOPERATION:
	data_send = rtk_i2c_master_write_brk(&i2c_dev->i2c_param, i2c_dev->i2c_manage.tx_info.p_data_buf,
										 i2c_dev->i2c_manage.tx_info.data_len);
	if (0 == data_send) {
        retry++;
        if (retry > SOWFTWARE_MAX_RETRYTIMES) {
            pr_err("retry overflow: %s - 1", __FUNCTION__);
            return HAL_TIMEOUT;
        }
		goto REOPERATION;
	} else if (data_send < i2c_dev->i2c_manage.tx_info.data_len) {
		pr_debug("Abort, Send %d data of %d %x!!!\n",
				data_send, i2c_dev->i2c_manage.tx_info.data_len,
				rtk_i2c_readl(i2c_dev->base, IC_TX_ABRT_SOURCE));
		rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);
		i2c_dev->i2c_manage.dev_status = I2C_STS_ERROR;
		return HAL_ERR_UNKNOWN;
	}

	i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;
	return HAL_OK;
}

static hal_status
rtk_i2c_receive_poll_master(
	struct rtk_i2c_dev *i2c_dev)
{
	u32 data_recv;
    int retry = 0;

	/* Send data till the TX buffer data length is zero */
	i2c_dev->i2c_manage.dev_status = I2C_STS_RX_ING;

REOPERATION:
	data_recv = rtk_i2c_master_read(&i2c_dev->i2c_param, i2c_dev->i2c_manage.rx_info.p_data_buf,
									i2c_dev->i2c_manage.rx_info.data_len);
	if (0 == data_recv) {
        retry++;
        if (retry > SOWFTWARE_MAX_RETRYTIMES) {
            pr_err("retry overflow: %s - 1", __FUNCTION__);
            return HAL_TIMEOUT;
        }
		goto REOPERATION;
	}
	if (data_recv < i2c_dev->i2c_manage.tx_info.data_len) {
		dev_err(i2c_dev->dev, "Abort, Send %d data of %d!!!\n", data_recv, i2c_dev->i2c_manage.rx_info.data_len);
		i2c_dev->i2c_manage.dev_status = I2C_STS_ERROR;
		return HAL_ERR_HW;
	}
	i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;

	return HAL_OK;
}

static hal_status rtk_i2c_receive_int_master(
	struct rtk_i2c_dev *i2c_dev)
{
	u32 i2c_local_len = 0;
	u8 i2c_stop = 0;

	/* Calculate user time out parameters */
	if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
		i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
		dev_err(i2c_dev->dev, "I2C-%d receive timeout. Reason: recv int triggered after timeout.", i2c_dev->i2c_param.i2c_index);
		return HAL_TIMEOUT;
	}

	/* I2C Device status Update */
	i2c_dev->i2c_manage.dev_status = I2C_STS_RX_READY;
	rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);

	/* To fill the Master Read Command into TX FIFO */
	i2c_dev->i2c_manage.master_rd_cmd_cnt = i2c_dev->i2c_manage.rx_info.data_len;
	i2c_local_len = 2;
	i2c_dev->i2c_manage.dev_status = I2C_STS_RX_READY;

	if (i2c_dev->i2c_manage.rx_info.data_len > 0) {
		rtk_i2c_interrupt_config(&i2c_dev->i2c_param, (I2C_BIT_M_RX_FULL | I2C_BIT_M_RX_OVER |
								 I2C_BIT_M_RX_UNDER | I2C_BIT_M_TX_ABRT), ENABLE);
	}

	/* In order for the DW_apb_i2c to continue acknowledging reads, a read command should be written for */
	/* every byte that is to be received; otherwise the DW_apb_i2c will stop acknowledging. */

	//two times read cmd:
	//flow is:
	//step 1: master request first data entry
	//step 2: slave send first data entry
	//step 3: master send seconed read cmd to ack first data and request second data
	//step 4: slave send second data
	//step 5: master rx full interrupt receive fisrt data and ack second data and request third data.
	//loop step 4 and step 5.
	//so last slave data have no ack, this is permitted by the spec.
	if (i2c_dev->i2c_manage.master_rd_cmd_cnt > 0) {
		i2c_stop = I2C_STOP_DIS;

		if ((i2c_dev->i2c_manage.master_rd_cmd_cnt == 1)
			&& ((i2c_dev->i2c_manage.i2c_extend & I2C_EXD_MTR_HOLD_BUS) == 0)) {
			i2c_stop = I2C_STOP_EN;
		}

		if (i2c_dev->i2c_manage.master_rd_cmd_cnt > 0) {
			i2c_dev->i2c_manage.master_rd_cmd_cnt--;
		}
		i2c_local_len--;
		rtk_i2c_master_send(&i2c_dev->i2c_param, i2c_dev->i2c_manage.rx_info.p_data_buf,
							I2C_READ_CMD, i2c_stop, 0);
	}

	return HAL_OK;
}

static hal_status rtk_i2c_send_int_master(
	struct rtk_i2c_dev *i2c_dev)
{
	/* I2C Device status Update */
	i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;
	rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);

	/* I2C Device status Update */
	i2c_dev->i2c_manage.dev_status = I2C_STS_TX_ING;

	/* Check I2C TX FIFO status */
	while (rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFE) == 0) {
		if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
			i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
			dev_err(i2c_dev->dev, "I2C-%d send timeout. Reason: wnd int triggered after timeout..", i2c_dev->i2c_param.i2c_index);
			break;
		}
	}

	/* I2C Enable TX Related Interrupts */
	rtk_i2c_interrupt_config(&i2c_dev->i2c_param, (I2C_BIT_M_TX_ABRT | I2C_BIT_M_TX_EMPTY |
							 I2C_BIT_M_TX_OVER), ENABLE);

	return HAL_OK;
}

hal_status rtk_i2c_send_master(struct rtk_i2c_dev *i2c_dev)
{
    /* Calculate user time out parameters */
	if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
		i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
		dev_err(i2c_dev->dev, "I2C-%d send timeout. Reason: i2c send timeout 1.", i2c_dev->i2c_param.i2c_index);
		return HAL_TIMEOUT;
	}
	dev_dbg(i2c_dev->dev, "mode check : %x (poll: 0, int: 2)", i2c_dev->i2c_manage.operation_type);

    /* Master run-time update  target address */
    if (i2c_dev->i2c_manage.i2c_extend & I2C_EXD_MTR_ADDR_UPD) {
        while (1) {
            /* Check Master activity status is 0 && TX FIFO status empty */
            if (!rtk_i2c_check_flag_state(&i2c_dev->i2c_param, I2C_BIT_MST_ACTIVITY) &&
                rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFE)) {
                break;
            }
            /* Time-Out check */
            if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
                i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
                dev_err(i2c_dev->dev, "I2C-%d send timeout. Reason: i2c send timeout 2.", i2c_dev->i2c_param.i2c_index);
                return HAL_TIMEOUT;
            }
        }

        /* Update Master Target address */
        rtk_i2c_set_slave_addr(&i2c_dev->i2c_param, i2c_dev->i2c_manage.tx_info.target_addr);
    }

    if (i2c_dev->i2c_manage.operation_type == I2C_POLL_TYPE) {
        rtk_i2c_send_poll_master(i2c_dev);
    }

    if (i2c_dev->i2c_manage.operation_type == I2C_INTR_TYPE) {
        rtk_i2c_send_int_master(i2c_dev);
    }
    return HAL_OK;
}

hal_status rtk_i2c_receive_master(struct rtk_i2c_dev *i2c_dev)
{
	/* Calculate user time out parameters */
	if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
		i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
		dev_err(i2c_dev->dev, "I2C-%d receive timeout. Reason: i2c recv timeout 1.\n", i2c_dev->i2c_param.i2c_index);
		return HAL_TIMEOUT;
	}

    /* Master run-time update target address */
    if (i2c_dev->i2c_manage.i2c_extend & I2C_EXD_MTR_ADDR_UPD) {
        while (1) {
            /* Check Master activity status && Check TX FIFO status */
            if (!rtk_i2c_check_flag_state(&i2c_dev->i2c_param, I2C_BIT_MST_ACTIVITY) &&
                rtk_i2c_check_flag_state(&i2c_dev->i2c_param, BIT_TFE)) {
                break;
            }
            rtk_i2c_clear_all_interrupts(&i2c_dev->i2c_param);

            /* Time-Out check */
            if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
                i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
                dev_err(i2c_dev->dev, "I2C-%d receive timeout. Reason: i2c recv timeout 2.", i2c_dev->i2c_param.i2c_index);
                return HAL_TIMEOUT;
            }
        }

        /* Update Master Target address */
        rtk_i2c_set_slave_addr(&i2c_dev->i2c_param, i2c_dev->i2c_manage.rx_info.target_addr);
    }

    /*I2C_POLL_OP_TYPE*/
    if (i2c_dev->i2c_manage.operation_type == I2C_POLL_TYPE) {
        rtk_i2c_receive_poll_master(i2c_dev);
    }

    /*I2C_INTR_OP_TYPE*/
    if (i2c_dev->i2c_manage.operation_type == I2C_INTR_TYPE) {
        rtk_i2c_receive_int_master(i2c_dev);
    }
    return HAL_OK;
}

static void rtk_print_i2c_input_data(struct i2c_msg *msg, bool direction)
{
	int i = 0;

	if (direction) {
		pr_debug("--- origin read buf input params ---");
	} else {
		pr_debug("--- check input params ---");
	}

	pr_debug("addr = %x", msg->addr);
	pr_debug("flags = %x", msg->flags);
	pr_debug("data len = %d", msg->len);
	pr_debug("data to transfer: ");
	for (i = 0; i < msg->len; i++) {
		pr_debug("%x", *(msg->buf + i));
	}
}

/* Linux Layar. */

static void rtk_i2c_xfer_msg(
	struct rtk_i2c_dev *i2c_dev,
	struct i2c_msg *msg)
{
	/*Local Variables*/
	u8 i2c_msg_addr_mode = I2C_ADDR_7BIT;
	hal_status dev_status = HAL_OK;

	dev_dbg(i2c_dev->dev, "%s", __FUNCTION__);
	rtk_print_i2c_input_data(msg, msg->flags & I2C_M_RD);

	i2c_dev->i2c_param.i2c_ack_addr = msg->addr;
	i2c_dev->i2c_manage.i2c_extend |= (I2C_EXD_MTR_ADDR_RTY | I2C_EXD_MTR_ADDR_UPD);

	if (msg->flags & I2C_M_TEN) {
		i2c_msg_addr_mode = I2C_ADDR_10BIT;
	}

	dev_dbg(i2c_dev->dev, "--->  I2C Speed Mode: %x\n", i2c_dev->i2c_param.i2c_speed_mode);
	dev_dbg(i2c_dev->dev, "--->  I2C Addr Mode: %x\n", i2c_msg_addr_mode);

	/*Assign I2C Address Mode Based on Address Mode*/
	i2c_dev->i2c_param.i2c_addr_mode = i2c_msg_addr_mode;

	/* To DeInitialize I2C Verification Master and Slave */
	dev_status = rtk_i2c_flow_deinit(i2c_dev);
	if (dev_status) {
		goto ERROR_CHECK;
	}

	if (msg->flags & I2C_M_RD) {
		/* i2c read */
		dev_dbg(i2c_dev->dev, "Master Read Start!!!\n");
		i2c_dev->i2c_manage.rx_info.target_addr = msg->addr;
		i2c_dev->i2c_manage.rx_info.p_data_buf = msg->buf;
		i2c_dev->i2c_manage.rx_info.data_len = msg->len;
		dev_status = rtk_i2c_flow_init(i2c_dev);
		if (dev_status) {
			goto ERROR_CHECK;
		}
		dev_status = rtk_i2c_receive_master(i2c_dev);
		if (dev_status) {
			goto ERROR_CHECK;
		}
		dev_dbg(i2c_dev->dev, "Master Read Done!!!\n");
		while ((i2c_dev->i2c_manage.dev_status != I2C_STS_IDLE) &&
			   (i2c_dev->i2c_manage.dev_status != I2C_STS_ERROR) &&
			   (i2c_dev->i2c_manage.dev_status != I2C_STS_TIMEOUT)) {
				/* Time-Out check */
				if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
					i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
					dev_err(i2c_dev->dev, "I2C-%d send timeout. Reason: cannot finish read.", i2c_dev->i2c_param.i2c_index);
					break;
				}
		}
	} else {
		/* i2c write */
		dev_dbg(i2c_dev->dev, "Master Write Start!!!\n");
		reinit_completion(&i2c_dev->xfer_completion);
		i2c_dev->i2c_manage.tx_info.target_addr = msg->addr;
		i2c_dev->i2c_manage.tx_info.p_data_buf = msg->buf;
		i2c_dev->i2c_manage.tx_info.data_len = msg->len;
		dev_status = rtk_i2c_flow_init(i2c_dev);
		if (dev_status) {
			goto ERROR_CHECK;
		}
		dev_status = rtk_i2c_send_master(i2c_dev);
		if (dev_status) {
			goto ERROR_CHECK;
		}
		dev_dbg(i2c_dev->dev, "Master Write Done!!!\n");
		while ((i2c_dev->i2c_manage.dev_status != I2C_STS_IDLE)
			   && (i2c_dev->i2c_manage.dev_status != I2C_STS_ERROR)
			   && (i2c_dev->i2c_manage.dev_status != I2C_STS_TIMEOUT)) {
				/* Directly sleep here will cause runtime touchscreen not smooth(us-level i2c done). */
				// /* No sleep here will cause audio xrun when i2c is not wired to board.(i2c cannot transfer). */
				if (wait_for_completion_timeout(&i2c_dev->xfer_completion, msecs_to_jiffies(3000)) == 0) {
					dev_err(i2c_dev->dev, "I2C wait for completion timeout");
					i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
					break;
				}

				if (rtk_i2c_is_timeout(i2c_dev) == HAL_TIMEOUT) {
					i2c_dev->i2c_manage.dev_status = I2C_STS_TIMEOUT;
					dev_err(i2c_dev->dev, "I2C-%d send timeout. Reason: cannot finish write.", i2c_dev->i2c_param.i2c_index);
					break;
				}
		}
	}

	if (i2c_dev->i2c_manage.dev_status == I2C_STS_ERROR) {
		dev_err(i2c_dev->dev, "I2C transfer with some error happened.");
		return;
	} else if (i2c_dev->i2c_manage.dev_status == I2C_STS_TIMEOUT) {
		dev_err(i2c_dev->dev, "I2C transfer timeout.");
		return;
	}
	return;

ERROR_CHECK:
	rtk_i2c_dev_status_handler(dev_status, i2c_dev->i2c_param.i2c_index);
}

static int rtk_i2c_xfer(struct i2c_adapter *i2c_adap,
						struct i2c_msg msgs[], int num)
{
	struct rtk_i2c_dev *i2c_dev = i2c_get_adapdata(i2c_adap);
	int ret, msg_id;

	/* Wait for slave hardware read/write flip. */
	udelay(50);

#if RTK_I2C_TODO
	ret = pm_runtime_get_sync(i2c_dev->dev);
	if (ret < 0) {
		return ret;
	}
#endif //RTK_I2C_TODO

	ret = rtk_i2c_wait_free_bus(i2c_dev);
	if (ret) {
		goto pm_free;
	}

	/* Start transfer i2c msgs. */
	for (msg_id = 0; msg_id < num; msg_id++) {
		dev_dbg(i2c_dev->dev, "order msg id = %d", msg_id);
		i2c_dev->i2c_manage.timeout = i2c_adap->timeout * 1000000;
		rtk_gtimer_change_period(i2c_dev->i2c_manage.timer_index, i2c_adap->timeout * 1000000);
		rtk_gtimer_int_config(i2c_dev->i2c_manage.timer_index, 1);
		rtk_gtimer_start(i2c_dev->i2c_manage.timer_index, 1);

		/* give hw i2c. */
		rtk_i2c_xfer_msg(i2c_dev, &msgs[msg_id]);
		dev_dbg(i2c_dev->dev, "msg id %d transfer done", msg_id);

		if (i2c_dev->i2c_manage.dev_status == I2C_STS_TIMEOUT) {
			rtk_gtimer_start(i2c_dev->i2c_manage.timer_index, 0);
			i2c_dev->i2c_manage.dev_status = I2C_STS_IDLE;
			rtk_i2c_flow_deinit(i2c_dev);
			dev_err(i2c_dev->dev, "Wait slave 0x%x timeout.\n", msgs[msg_id].addr);
			return -ETIMEDOUT;
		} else if (i2c_dev->i2c_manage.dev_status == I2C_STS_ERROR) {
			dev_err(i2c_dev->dev, "Some error happened when contacting slave 0x%x.\n", msgs[msg_id].addr);
			ret = -ETIMEDOUT;
		}
		rtk_gtimer_start(i2c_dev->i2c_manage.timer_index, 0);
	}

pm_free:
#if RTK_I2C_TODO
	pm_runtime_mark_last_busy(i2c_dev->dev);
	pm_runtime_put_autosuspend(i2c_dev->dev);
#endif // RTK_I2C_TODO

	return (ret < 0) ? ret : num;
}

static u32 rtk_i2c_master_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR;
}

static const struct i2c_algorithm rtk_i2c_master_algo = {
	.master_xfer = rtk_i2c_xfer,
	.functionality = rtk_i2c_master_func,
};

int rtk_i2c_master_probe(
    struct platform_device *pdev, struct rtk_i2c_dev *i2c_dev, struct i2c_adapter *adap)
{
    int r;

	i2c_dev->i2c_param.i2c_master = I2C_MASTER_MODE;
    adap->algo = &rtk_i2c_master_algo;

	r= rtk_gtimer_dynamic_init(i2c_dev->i2c_manage.user_set_timeout * 1000000,
        rtk_i2c_hw_timer_timeout, i2c_dev);
	if (r < 0) {
		dev_err(i2c_dev->dev, "timer get failed.");
		return r;
	} else {
		i2c_dev->i2c_manage.timer_index = r;
	}

    /* Master parameters init. */
	i2c_dev->i2c_manage.rx_info.p_data_buf = devm_kzalloc(&pdev->dev, RTK_I2C_T_RX_FIFO_MAX, GFP_KERNEL);
	i2c_dev->i2c_manage.tx_info.p_data_buf = devm_kzalloc(&pdev->dev, RTK_I2C_T_RX_FIFO_MAX, GFP_KERNEL);

	rtk_i2c_init_hw(&i2c_dev->i2c_param);
	init_completion(&i2c_dev->xfer_completion);
    return 0;
}
