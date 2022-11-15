/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __AX_MAC_BCN_H__
#define __AX_MAC_BCN_H__

#include "../type.h"
#include "../fw_ax/inc_hdr/fwcmd_intf.h"
#include "fwcmd_intf_f2p.h"
#include "fwcmd_intf_f2p_v1.h"
#include "trx_desc.h"
#include "fwofld.h"
#include "p2p.h"
#include "flash.h"
#include "dbg_cmd.h"

#ifdef RTW_PHL_BCN_IOT
#define	AX_TXDESC_QSEL_BCN 0x10

struct mac_ax_bcn_priv {
	u8 port;
	u8 mbssid;
	u8 band;
	u8 grp_ie_ofst;
	u8 macid;
	u8 ssn_sel;
	u8 ssn_mode;
	u16 rate_sel;
	u8 txpwr;
	u8 txinfo_ctrl_en;
	u8 ntx_path_en;
	u8 path_map_a;
	u8 path_map_b;
	u8 path_map_c;
	u8 path_map_d;
	u8 antsel_a;
	u8 antsel_b;
	u8 antsel_c;
	u8 antsel_d;
	u8 sw_tsf;
	u16 csa_ofst;
	u8 *bcn_vir_addr;
	u32 bcn_phy_addr;
	u32 bcn_len;
	u8 *desc_vir_addr;
	u32 desc_phy_addr;
	u32 desc_len;
	u8 tx_bcn;
	_os_lock lock;
};

u32 mac_send_bcn(struct mac_ax_adapter *adapter, struct mac_ax_bcn_info *info);
#endif

#endif /* __AX_MAC_BCN_H__ */
