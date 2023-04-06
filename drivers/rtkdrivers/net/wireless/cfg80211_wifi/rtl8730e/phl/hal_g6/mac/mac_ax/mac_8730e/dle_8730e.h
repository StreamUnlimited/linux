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

#ifndef _MAC_AX_DLE_8730E_H_
#define _MAC_AX_DLE_8730E_H_

#include "../dle.h"

#if MAC_AX_8730E_SUPPORT

/*--------------------Define ----------------------------------------*/

/*--------------------Define Enum------------------------------------*/

/*--------------------Define MACRO----------------------------------*/

/*--------------------Define Struct-----------------------------------*/

/*--------------------Export global variable----------------------------*/

/*--------------------Function declaration-----------------------------*/

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */


/**
 * @brief preload_cfg_set_8852a
 *
 * @param *adapter
 * @param band
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 preload_cfg_set_8730e(struct mac_ax_adapter *adapter, enum mac_ax_band band,
			  struct mac_ax_preld_cfg *cfg);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief preload_init_set_8852a
 *
 * @param *adapter
 * @param band
 * @param mode
 * @return Please Place Description here.
 * @retval u32
 */
u32 preload_init_set_8730e(struct mac_ax_adapter *adapter, enum mac_ax_band band,
			   enum mac_ax_qta_mode mode);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief dle_func_en_8852a
 *
 * @param *adapter
 * @param en
 * @return Please Place Description here.
 * @retval u32
 */
u32 dle_func_en_8852a(struct mac_ax_adapter *adapter, u8 en);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief dle_clk_en_8852a
 *
 * @param *adapter
 * @param en
 * @return Please Place Description here.
 * @retval u32
 */
u32 dle_clk_en_8852a(struct mac_ax_adapter *adapter, u8 en);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief dle_mix_cfg_8852a
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 dle_mix_cfg_8852a(struct mac_ax_adapter *adapter, struct dle_mem_t *cfg);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief wde_quota_cfg_8852a
 *
 * @param *adapter
 * @param *min_cfg
 * @param *max_cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 wde_quota_cfg_8852a(struct mac_ax_adapter *adapter,
			struct wde_quota_t *min_cfg,
			struct wde_quota_t *max_cfg);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief ple_quota_cfg_8852a
 *
 * @param *adapter
 * @param *min_cfg
 * @param *max_cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 ple_quota_cfg_8852a(struct mac_ax_adapter *adapter,
			struct ple_quota_t *min_cfg,
			struct ple_quota_t *max_cfg);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup DLE
 * @{
 */
/**
 * @brief chk_dle_rdy_8852a
 *
 * @param *adapter
 * @param type
 * @return Please Place Description here.
 * @retval u32
 */
u32 chk_dle_rdy_8852a(struct mac_ax_adapter *adapter, enum DLE_CTRL_TYPE type);
/**
 * @}
 * @}
 */

u32 mac_is_txq_empty_8730e(struct mac_ax_adapter *adapter,
			   struct mac_ax_tx_queue_empty *val);
#endif
#endif
