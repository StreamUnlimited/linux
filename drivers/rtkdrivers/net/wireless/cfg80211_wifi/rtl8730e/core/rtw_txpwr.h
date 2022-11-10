/******************************************************************************
 *
 * Copyright(c) 2007 - 2022 Realtek Corporation.
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
#ifndef __RTW_TXPWR_H__
#define __RTW_TXPWR_H__

struct txpwr_param_status {
	bool enable;
	bool loaded;
	bool external_src;
};

#define SET_TXPWR_PARAM_STATUS(_status, _enable, _loaded, _ext_src) \
	do { \
		(_status)->enable = _enable; \
		(_status)->loaded = _loaded; \
		(_status)->external_src = _ext_src; \
	} while (0)

struct tx_power_ext_info {
	struct txpwr_param_status by_rate;
	struct txpwr_param_status lmt;
#ifdef CONFIG_80211AX_HE
	struct txpwr_param_status lmt_ru;
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
	struct txpwr_param_status lmt_6g;
	struct txpwr_param_status lmt_ru_6g;
#endif
};

void dump_tx_power_ext_info(void *sel, struct dvobj_priv *dvobj);

#endif /* __RTW_TXPWR_H__ */