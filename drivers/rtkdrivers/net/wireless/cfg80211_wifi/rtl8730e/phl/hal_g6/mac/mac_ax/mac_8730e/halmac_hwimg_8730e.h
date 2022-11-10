/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#include "../../type.h"
#include "../../mac_ax.h"

#ifndef _HALMAC_HW_IMG_8730E_H_
#define _HALMAC_HW_IMG_8730E_H_



/******************************************************************************
 *                           pwr_reg_pxp
 ******************************************************************************/
#if MAC_AX_8730E_SUPPORT
void init_pwr_reg_8730e(struct mac_ax_adapter *adapter,  bool is_form_folder,
			u32 folder_len, u32 *folder_array);
#endif


#endif /* end of HWIMG_SUPPORT*/

