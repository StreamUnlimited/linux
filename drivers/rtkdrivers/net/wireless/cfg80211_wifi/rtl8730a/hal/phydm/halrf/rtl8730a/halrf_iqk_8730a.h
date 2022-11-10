/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __HALRF_IQK_8730A_H__
#define __HALRF_IQK_8730A_H__

#if (RTL8730A_SUPPORT == 1)
/*--------------------------Define Parameters-------------------------------*/
#define IQK_8730A_TONE 1
#define IQK_8730A_SHOW 0

#define MAC_REG_NUM_8730A 1
#define BB_REG_NUM_8730A 12
#define RF_REG_NUM_8730A 9
#define IQK_DELAY_8730A 2
#define IQK_STEP_8730A 3
//#define RXK_STEP_8730A 6
//#define IQK_CMD_8730A 0x8

#define TX_IQK 0
#define RX_IQK 1
#define RF_PATH_MAX_8730A 1
/*---------------------------End Define Parameters-------------------------------*/

void do_iqk_8730a(void *dm_void,
		  u8 delta_thermal_index,
		  u8 thermal_value,
		  u8 threshold);

void phy_iq_calibrate_8730a(void *dm_void, boolean is_recovery);

void _iqk_fill_iqk_xy_8730a(void *dm_void);

#endif
#endif /*__HALRF_IQK_8730A_H__*/
