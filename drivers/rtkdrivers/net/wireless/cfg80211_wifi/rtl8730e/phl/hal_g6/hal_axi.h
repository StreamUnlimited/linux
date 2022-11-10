/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef _HAL_AXI_H_
#define _HAL_AXI_H_

#ifdef CONFIG_AXI_HCI
void hal_axi_set_io_ops(struct rtw_hal_com_t *hal, struct hal_io_ops *pops);

#ifdef CONFIG_RTL8730E
#include "rtl8730e/rtl8730e.h"
#endif

static inline void hal_set_ops_axi(struct rtw_phl_com_t *phl_com,
				   struct hal_info_t *hal)
{
#ifdef CONFIG_RTL8730E
	if (hal_get_chip_id(hal->hal_com) == CHIP_WIFI6_8730E) {
		hal_set_ops_8730ea(phl_com, hal);
		hal_hook_trx_ops_8730ea(phl_com, hal);
	}
#endif
}
#endif /*CONFIG_AXI_HCI*/
#endif /*_HAL_AXI_H_*/
