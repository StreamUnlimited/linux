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

#ifndef _MAC_AX_PWR_SEQ_FUNC_8730E_H_
#define _MAC_AX_PWR_SEQ_FUNC_8730E_H_

#include "../../mac_def.h"
#if MAC_AX_8730E_SUPPORT

u32 mac_pwr_on_8730e(struct mac_ax_adapter *adapter);
u32 mac_pwr_off_8730e(struct mac_ax_adapter *adapter);

#if MAC_AX_FEATURE_HV
u32 mac_enter_lps_sdio_8852c(struct mac_ax_adapter *adapter);
u32 mac_enter_lps_usb_8852c(struct mac_ax_adapter *adapter);
u32 mac_enter_lps_pcie_8852c(struct mac_ax_adapter *adapter);

u32 mac_leave_lps_sdio_8852c(struct mac_ax_adapter *adapter);
u32 mac_leave_lps_usb_8852c(struct mac_ax_adapter *adapter);
u32 mac_leave_lps_pcie_8852c(struct mac_ax_adapter *adapter);

#endif
#endif /* #if MAC_AX_8852C_SUPPORT */
#endif
