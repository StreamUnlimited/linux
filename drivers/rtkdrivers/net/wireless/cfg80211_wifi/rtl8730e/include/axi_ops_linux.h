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
#ifndef __AXI_OPS_LINUX_H__
#define __AXI_OPS_LINUX_H__

u8 os_axi_read8(struct dvobj_priv *dvobj, u32 addr);
u16 os_axi_read16(struct dvobj_priv *dvobj, u32 addr);
u32 os_axi_read32(struct dvobj_priv *dvobj, u32 addr);
void os_axi_mem_r(struct dvobj_priv *dvobj, u32 addr, u8 *buf, u32 len);
int os_axi_write8(struct dvobj_priv *dvobj, u32 addr, u8 val);
int os_axi_write16(struct dvobj_priv *dvobj, u32 addr, u16 val);
int os_axi_write32(struct dvobj_priv *dvobj, u32 addr, u32 val);
void os_axi_mem_w(struct dvobj_priv *dvobj, u32 addr, u8 *buf, u32 len);

u8 os_axi_sys_read8(struct dvobj_priv *dvobj, u32 base, u32 addr);
u16 os_axi_sys_read16(struct dvobj_priv *dvobj, u32 base, u32 addr);
u32 os_axi_sys_read32(struct dvobj_priv *dvobj, u32 base, u32 addr);
int os_axi_sys_write8(struct dvobj_priv *dvobj, u32 base, u32 addr, u8 val);
int os_axi_sys_write16(struct dvobj_priv *dvobj, u32 base, u32 addr, u16 val);
int os_axi_sys_write32(struct dvobj_priv *dvobj, u32 base, u32 addr, u32 val);
#endif /*__AXI_OPS_LINUX_H__*/
