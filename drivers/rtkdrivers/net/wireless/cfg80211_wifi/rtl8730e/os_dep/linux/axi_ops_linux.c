/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
 *****************************************************************************/
#define _AXI_OPS_LINUX_C_

#include <drv_types.h>
#include <mac/mac_def.h>

u8 os_axi_read8(struct dvobj_priv *dvobj, u32 addr)
{
	return 0xff & readb((u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
}

u16 os_axi_read16(struct dvobj_priv *dvobj, u32 addr)
{
	return readw((u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
}

u32 os_axi_read32(struct dvobj_priv *dvobj, u32 addr)
{
	return readl((u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
}

int os_axi_write8(struct dvobj_priv *dvobj, u32 addr, u8 val)
{
	writeb(val, (u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
	return 1;
}

int os_axi_write16(struct dvobj_priv *dvobj, u32 addr, u16 val)
{
	writew(val, (u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
	return 2;
}

int os_axi_write32(struct dvobj_priv *dvobj, u32 addr, u32 val)
{
	writel(val, (u8 *)dvobj_to_axi(dvobj)->axi_mem_start + addr);
	return 4;
}

u8 os_axi_sys_read8(struct dvobj_priv *dvobj, u32 base, u32 addr)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		return 0xff & readb((u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

u16 os_axi_sys_read16(struct dvobj_priv *dvobj, u32 base, u32 addr)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		return readw((u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

u32 os_axi_sys_read32(struct dvobj_priv *dvobj, u32 base, u32 addr)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		return readl((u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

int os_axi_sys_write8(struct dvobj_priv *dvobj, u32 base, u32 addr, u8 val)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		writeb(val, (u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 1;
}

int os_axi_sys_write16(struct dvobj_priv *dvobj, u32 base, u32 addr, u16 val)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		writew(val, (u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 2;
}

int os_axi_sys_write32(struct dvobj_priv *dvobj, u32 base, u32 addr, u32 val)
{
	if (base == SYSTEM_CTRL_BASE_LP) {
		writel(val, (u8 *)dvobj_to_axi(dvobj)->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 4;
}