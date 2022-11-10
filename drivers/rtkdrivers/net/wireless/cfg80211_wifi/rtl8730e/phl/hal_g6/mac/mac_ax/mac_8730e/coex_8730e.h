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

#ifndef _MAC_AX_COEX_8730E_H_
#define _MAC_AX_COEX_8730E_H_

#include "../../mac_def.h"
#if MAC_AX_8730E_SUPPORT

#define DBG_GNT_WL_ID 0x1B
#define DBG_GNT_BT_ID 0x1B

/**
 * @addtogroup Common
 * @{
 * @addtogroup BTCoex
 * @{
 */

/**
 * @brief mac_coex_init
 *
 * @param *adapter
 * @param *coex
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_coex_init_8730e(struct mac_ax_adapter *adapter,
			struct mac_ax_coex *coex);
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
 * @brief mac_cfg_gnt
 *
 * @param *adapter
 * @param *gnt_cfg
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_cfg_gnt_8730e(struct mac_ax_adapter *adapter,
		      struct mac_ax_coex_gnt *gnt_cfg);
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
 * @brief mac_get_gnt
 *
 * @param *adapter
 * @param *gnt_cfg
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_get_gnt_8730e(struct mac_ax_adapter *adapter,
		      struct mac_ax_coex_gnt *gnt_cfg);
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
 * @brief mac_cfg_plt
 *
 * @param *adapter
 * @param *plt
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_cfg_plt_8852a(struct mac_ax_adapter *adapter, struct mac_ax_plt *plt);
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


#endif /* #if MAC_AX_8730E_SUPPORT */
#endif
