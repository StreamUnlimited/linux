/** @file */
/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#ifndef _MAC_AX_COEX_H_
#define _MAC_AX_COEX_H_

#include "../mac_def.h"

#define MAC_AX_NOTIFY_TP_MAJOR 0x81
#define MAC_AX_NOTIFY_PWR_MAJOR 0x80

/**
 * @addtogroup Common
 * @{
 * @addtogroup BTCoex
 * @{
 */

/**
 * @brief mac_cfg_plt
 *
 * @param *adapter
 * @param *plt
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_cfg_plt(struct mac_ax_adapter *adapter, struct mac_ax_plt *plt);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup BTCoex
 * @{
 */



/**
 * @addtogroup Common
 * @{
 * @addtogroup BTCoex
 * @{
 */


/**
 * @brief mac_write_coex_mask
 *
 * Write coex reg with mask
 *
 * @param *adapter
 * @param offset
 * @param mask
 * @param val
 * @return 0 for success, others are fail
 * @retval u32
 */

u32 mac_write_coex_mask(struct mac_ax_adapter *adapter,
			u32 offset, u32 mask, u32 val);

/**
 * @}
 * @}
 */

u32 mac_read_coex_reg(struct mac_ax_adapter *adapter,
		      const u32 offset, u32 *val);

u32 mac_write_coex_reg(struct mac_ax_adapter *adapter,
		       const u32 offset, const u32 val);

#endif
