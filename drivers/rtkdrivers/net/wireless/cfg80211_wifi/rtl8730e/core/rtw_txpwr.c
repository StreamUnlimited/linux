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
#define _RTW_TXPWR_C_

#include <drv_types.h>

void dump_tx_power_ext_info(void *sel, struct dvobj_priv *dvobj)
{
	struct tx_power_ext_info info;
	struct {
		const char *str;
		struct txpwr_param_status *status;
	} params[] = {
		{"tx_power_by_rate", &info.by_rate},
		{"tx_power_limit", &info.lmt},
#ifdef CONFIG_80211AX_HE
		{"tx_power_limit_ru", &info.lmt_ru},
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
		{"tx_power_limit_6g", &info.lmt_6g},
		{"tx_power_limit_ru_6g", &info.lmt_ru_6g},
#endif
	};
	u8 num_of_param = sizeof(params) / sizeof(params[0]);
	u8 i;

	if (rtw_txpwr_hal_get_ext_info(dvobj, &info)) {
		for (i = 0; i < num_of_param; i++) {
			RTW_PRINT_SEL(sel, "%s: %s, %s, %s\n", params[i].str
				      , params[i].status->enable ? "enabled" : "disabled"
				      , params[i].status->loaded ? "loaded" : "unloaded"
				      , params[i].status->external_src ? "file" : "default"
				     );
		}
	} else {
		RTW_PRINT_SEL(sel, "not ready\n");
	}
}
