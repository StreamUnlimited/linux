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

#ifndef _TABLEUPD_H2C_8730E_H_
#define _TABLEUPD_H2C_8730E_H_

#include "../fwcmd.h"
#if MAC_AX_8730E_SUPPORT
#if 0
/**
 * @brief mac_f2p_test_cmd_8852c
 *
 * @param *adapter
 * @param *info
 * @param *f2pwd
 * @param *ptxcmd
 * @param *psigb_addr
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_f2p_test_cmd_8852c(struct mac_ax_adapter *adapter,
			   struct mac_ax_f2p_test_para_v1 *info,
			   struct mac_ax_f2p_wd *f2pwd,
			   struct mac_ax_f2p_tx_cmd_v1 *ptxcmd,
			   u8 *psigb_addr);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup DMAC_Table
 * @{
 */

/**
 * @brief mac_upd_dctl_info_8852c
 *
 * @param *adapter
 * @param *info
 * @param *mask
 * @param macid
 * @param operation
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_upd_dctl_info_8852c(struct mac_ax_adapter *adapter,
			    struct mac_ax_dctl_info *info,
			    struct mac_ax_dctl_info *mask, u8 macid, u8 operation);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup CMAC_Table
 * @{
 */

/**
 * @brief mac_upd_cctl_info_8852c
 *
 * @param *adapter
 * @param *info
 * @param *mask
 * @param macid
 * @param operation
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_upd_cctl_info_8852c(struct mac_ax_adapter *adapter,
			    struct rtw_hal_mac_ax_cctl_info *info,
			    struct rtw_hal_mac_ax_cctl_info *mask, u8 macid, u8 operation);
/**
 * @}
 * @}
 */

#if MAC_AX_FEATURE_DBGPKG
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup CMAC_Table
 * @{
 */

/**
 * @brief cctl_info_debug_write_8852c
 *
 * @param *adapter
 * @param *tbl
 * @param macid
 * @return Please Place Description here.
 * @retval u32
 */

u32 cctl_info_debug_write_8852c(struct mac_ax_adapter *adapter,
				struct fwcmd_cctlinfo_ud_v1 *tbl, u8 macid);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup DMAC_Table
 * @{
 */

/**
 * @brief dctl_info_debug_write_v1_8852c
 *
 * @param *adapter
 * @param *tbl
 * @param macid
 * @return Please Place Description here.
 * @retval u32
 */
u32 dctl_info_debug_write_8852c(struct mac_ax_adapter *adapter,
				struct fwcmd_dctlinfo_ud_v1 *tbl, u8 macid);
/**
 * @}
 * @}
 */

/**
 * @brief mac_ss_dl_grp_upd
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_ss_dl_grp_upd_8852c(struct mac_ax_adapter *adapter,
			    struct mac_ax_ss_dl_grp_upd *info);
#endif
#endif /* #if MAC_AX_8852C_SUPPORT */
#endif
#endif
