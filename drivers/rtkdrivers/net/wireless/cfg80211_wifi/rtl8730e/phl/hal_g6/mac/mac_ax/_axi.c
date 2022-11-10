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
#include "_axi.h"
#include "pwr.h"
#include "mac_priv.h"

#if MAC_AX_AXI_SUPPORT

static struct mac_ax_processor_id proc_id_long_dly[PROC_ID_LIST_NUM] = {
	/* Cezanne & Barcelo */ {0x178BFBFF, 0x00A50F00},
	/* Rembrandt */ {0x178BFBFF, 0x00A40F40}
};

u8 reg_read8_axi(struct mac_ax_adapter *adapter, u32 addr)
{
	return PLTFM_REG_R8(addr);
}

u8 sys_reg_read8_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr)
{
	return PLTFM_SYS_REG_R8(base, addr);
}

void reg_write8_axi(struct mac_ax_adapter *adapter, u32 addr, u8 val)
{
	PLTFM_REG_W8(addr, val);
}

void sys_reg_write8_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u8 val)
{
	PLTFM_SYS_REG_W8(base, addr, val);
}

u16 reg_read16_axi(struct mac_ax_adapter *adapter, u32 addr)
{
	return PLTFM_REG_R16(addr);
}

u16 sys_reg_read16_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr)
{
	return PLTFM_SYS_REG_R16(base, addr);
}

void reg_write16_axi(struct mac_ax_adapter *adapter, u32 addr, u16 val)
{
	PLTFM_REG_W16(addr, val);
}

void sys_reg_write16_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u16 val)
{
	PLTFM_SYS_REG_W16(base, addr, val);
}


u32 reg_read32_axi(struct mac_ax_adapter *adapter, u32 addr)
{
	return PLTFM_REG_R32(addr);
}

u32 sys_reg_read32_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr)
{
	return PLTFM_SYS_REG_R32(base, addr);
}


void reg_write32_axi(struct mac_ax_adapter *adapter, u32 addr, u32 val)
{
	PLTFM_REG_W32(addr, val);
}

void sys_reg_write32_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u32 val)
{
	PLTFM_SYS_REG_W32(base, addr, val);
}

u16 calc_avail_wptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_wptr = 0;

	if (rptr > wptr) {
		avail_wptr = rptr - wptr - 1;
	} else {
		avail_wptr = rptr + (bndy - wptr) - 1;
	}

	return avail_wptr;
}

u16 calc_avail_rptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_rptr = 0;

	if (wptr >= rptr) {
		avail_rptr = wptr - rptr;
	} else {
		avail_rptr = wptr + (bndy - rptr);
	}

	return avail_rptr;
}

u32 axi_pwr_switch(void *vadapter, u8 pre_switch, u8 on)
{
	struct mac_ax_adapter *adapter = (struct mac_ax_adapter *)vadapter;

	if (pre_switch == PWR_PRE_SWITCH) {
		adapter->mac_pwr_info.pwr_seq_proc = 1;
	} else if (pre_switch == PWR_POST_SWITCH) {
		adapter->mac_pwr_info.pwr_seq_proc = 0;
	}

	return MACSUCCESS;
}

u32 axi_get_txagg_num(struct mac_ax_adapter *adapter, u8 band)
{
	return AXI_DEFAULT_AGG_NUM;
}

#endif /* #if MAC_AX_AXI_SUPPORT */

