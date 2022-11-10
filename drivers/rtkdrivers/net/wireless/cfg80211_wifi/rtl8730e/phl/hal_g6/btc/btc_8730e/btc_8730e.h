/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation.
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
#ifndef __RTL8730E_BTC_H__
#define __RTL8730E_BTC_H__
/* rtl8730e_btc.c */

extern const struct btc_chip chip_8730e;
void _8730e_rfe_type(struct btc_t *btc);
void _8730e_init_cfg(struct btc_t *btc);
void _8730e_wl_pri(struct btc_t *btc, u8 map, bool state);
void _8730e_wl_tx_power(struct btc_t *btc, u32 level);
void _8730e_wl_rx_gain(struct btc_t *btc, u32 level);
void _8730e_wl_s1_standby(struct btc_t *btc, u32 state);
void _8730e_wl_req_mac(struct btc_t *btc, u8 mac_id);
void _8730e_update_bt_cnt(struct btc_t *btc);
u8 _8730e_bt_rssi(struct btc_t *btc, u8 val);
#endif /*__RTL8730E_PHY_H__*/
