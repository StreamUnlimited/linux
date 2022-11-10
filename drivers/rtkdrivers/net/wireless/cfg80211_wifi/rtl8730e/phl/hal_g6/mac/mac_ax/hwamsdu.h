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

#ifndef _MAC_AX_HWAMSDU_H_
#define _MAC_AX_HWAMSDU_H_

#include "../type.h"
#include "../mac_ax.h"

#define MAX_LENGTH_ENUM 7

/**
 * @struct mac_ax_en_amsdu_cut
 * @brief mac_ax_en_amsdu_cut
 *
 * @var mac_ax_en_amsdu_cut::enable
 * Please Place Description here.
 * @var mac_ax_en_amsdu_cut::low_th
 * Please Place Description here.
 * @var mac_ax_en_amsdu_cut::high_th
 * Please Place Description here.
 * @var mac_ax_en_amsdu_cut::aligned
 * Please Place Description here.
 * @var mac_ax_en_amsdu_cut::rsvd0
 * Please Place Description here.
 */
struct mac_ax_en_amsdu_cut {
	/* dword0 */
	u32 enable: 1;
	u32 low_th: 8;
	u32 high_th: 16;
	u32 aligned: 2;
	u32 rsvd0: 5;
};

/**
 * @struct mac_ax_en_hwamsdu
 * @brief mac_ax_en_hwamsdu
 *
 * @var mac_ax_en_hwamsdu::enable
 * Please Place Description here.
 * @var mac_ax_en_hwamsdu::max_num
 * Please Place Description here.
 * @var mac_ax_en_hwamsdu::en_single_amsdu
 * Please Place Description here.
 * @var mac_ax_en_hwamsdu::en_last_amsdu_padding
 * Please Place Description here.
 * @var mac_ax_en_hwamsdu::rsvd0
 * Please Place Description here.
 */
struct mac_ax_en_hwamsdu {
	/* dword0 */
	u32 enable: 1;
	u32 max_num: 2;
	u32 en_single_amsdu: 1;
	u32 en_last_amsdu_padding: 1;
	u32 rsvd0: 27;
};

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_ShortCut
 * @{
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_ShortCut
 * @{
 */
#endif
