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

#ifndef _MAC_AX_EFUSE_8730E_H_
#define _MAC_AX_EFUSE_8730E_H_

#include "../efuse.h"
#include "hal_headers.h"
#include "efuse/hal_efuse.h"
#include <misc/realtek-otp-core.h>

#if MAC_AX_8730E_SUPPORT

#define ENABLE  1
#define DISABLE 0

#define OTPC_EF_ADDR_NS(x)        ((u32)(((x) & 0x000007FF) << 8))
#define OTPC_GET_EF_ADDR_NS(x)    ((u32)(((x >> 8) & 0x000007FF)))
#define OTPC_BIT_EF_RD_WR_NS      ((u32)0x00000001 << 31)
#define OTP_POLL_TIMES					20000
#define OTPC_MASK_EF_DATA_NS      ((u32)0x000000FF << 0)

/**
 * @addtogroup Efuse
 * @{
 */

/**
 * @}
 */

/**
 * @brief efuse_read_8730e
 *
 * @param *adapter
 * @param addr
 * @param *data
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_read_8730e(struct mac_ax_adapter *adapter,
		      u32 addr, u8 *data, int len);

/**
 * @}
 */

/**
 * @brief efuse_write_8730e
 *
 * @param *adapter
 * @param addr
 * @param data
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_write_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 data, int len);

/**
 * @brief efuse_read8_8730e
 *
 * @param *adapter
 * @param addr
 * @param *data
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_read8_8730e(struct mac_ax_adapter *adapter,
		      u32 addr, u8 *data);

/**
 * @}
 */

/**
 * @brief efuse_write8_8730e
 *
 * @param *adapter
 * @param addr
 * @param data
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_write8_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 data);

/**
 * @}
 */

u32 read_log_efuse_8730e(struct mac_ax_adapter *adapter, u32 addr,
			 u32 size, u8 *val);
u32 write_log_efuse_8730e(struct mac_ax_adapter *adapter, u32 addr, u8 val);
u32 dump_log_efuse_8730e(struct mac_ax_adapter *adapter, u8 *efuse_map);

#endif /* MAC_AX_8730E_SUPPORT */
#endif
