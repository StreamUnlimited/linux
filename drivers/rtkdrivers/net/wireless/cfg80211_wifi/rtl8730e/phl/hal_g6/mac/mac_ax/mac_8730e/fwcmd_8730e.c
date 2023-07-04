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

#include "../../type.h"
#include "../mac_priv.h"
#include "mac/halmac_reg2.h"

#if MAC_AX_8730E_SUPPORT

static struct mac_ax_h2creg_offset h2creg_offset = {
	REG_HMEBOX0, /* cmd + data 0~3  */
	REG_HMEBOX_E0, /* data 4~6 */
	REG_HMEBOX1,
	REG_HMEBOX_E1,
	REG_HMETFR,
};

static struct mac_ax_c2hreg_offset c2hreg_offset = {
	0, /* data0 */
	0, /* data1 */
	0, /* data2 */
	REG_MCUTST_II, /* data3 */
	0, /* ctrl */
};

struct mac_ax_h2creg_offset *
get_h2creg_offset_8730e(struct mac_ax_adapter *adapter)
{
	return &h2creg_offset;
}

struct mac_ax_c2hreg_offset *
get_c2hreg_offset_8730e(struct mac_ax_adapter *adapter)
{
	return &c2hreg_offset;
}
#endif /* #if MAC_AX_8852A_SUPPORT */
