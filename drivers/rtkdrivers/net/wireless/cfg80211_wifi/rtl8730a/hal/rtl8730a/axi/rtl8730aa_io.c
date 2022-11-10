/******************************************************************************
 *
 * Copyright(c) 2015 - 2017 Realtek Corporation.
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
#define _RTL8730AA_IO_C_

#include <drv_types.h>		/* PADAPTER and etc. */

static u8 axi_read8(struct intf_hdl *phdl, u32 addr)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	return 0xff & readb((u8 *)pdvobjpriv->axi_mem_start + addr);
}

static u16 axi_read16(struct intf_hdl *phdl, u32 addr)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	return readw((u8 *)pdvobjpriv->axi_mem_start + addr);
}

static u32 axi_read32(struct intf_hdl *phdl, u32 addr)
{
	u32 value32 = 0;
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	value32 = readl((u8 *)pdvobjpriv->axi_mem_start + addr);
	return value32;
}

static int axi_write8(struct intf_hdl *phdl, u32 addr, u8 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	writeb(val, (u8 *)pdvobjpriv->axi_mem_start + addr);
	return 1;
}

static int axi_write16(struct intf_hdl *phdl, u32 addr, u16 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	writew(val, (u8 *)pdvobjpriv->axi_mem_start + addr);
	return 2;
}

static int axi_write32(struct intf_hdl *phdl, u32 addr, u32 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	writel(val, (u8 *)pdvobjpriv->axi_mem_start + addr);
	return 4;
}

static void axi_read_mem(struct intf_hdl *phdl, u32 addr, u32 cnt, u8 *rmem)
{
	RTW_INFO("%s(%d)fake function\n", __func__, __LINE__);
}

static void axi_write_mem(struct intf_hdl *phdl, u32 addr, u32 cnt, u8 *wmem)
{
	RTW_INFO("%s(%d)fake function\n", __func__, __LINE__);
}

static u32 axi_read_port(struct intf_hdl *phdl, u32 addr, u32 cnt, u8 *rmem)
{
	return 0;
}

static u32 axi_write_port(struct intf_hdl *phdl, u32 addr, u32 cnt, u8 *wmem)
{
	_adapter *padapter = (_adapter *)phdl->padapter;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	netif_trans_update(padapter->pnetdev);
#else
	padapter->pnetdev->trans_start = jiffies;
#endif

	return 0;
}

#ifdef CONFIG_AXI_HCI
static u8 axi_sys_read8(struct intf_hdl *phdl, u32 base, u32 addr)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		return 0xff & readb((u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

static u16 axi_sys_read16(struct intf_hdl *phdl, u32 base, u32 addr)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		return readw((u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

static u32 axi_sys_read32(struct intf_hdl *phdl, u32 base, u32 addr)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		return readl((u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
		return 0;
	}
}

static int axi_sys_write8(struct intf_hdl *phdl, u32 base, u32 addr, u8 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		writeb(val, (u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 1;
}

static int axi_sys_write16(struct intf_hdl *phdl, u32 base, u32 addr, u16 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		writew(val, (u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 2;
}

static int axi_sys_write32(struct intf_hdl *phdl, u32 base, u32 addr, u32 val)
{
	struct dvobj_priv *pdvobjpriv = (struct dvobj_priv *)phdl->pintf_dev;

	if (base == SYSTEM_CTRL_BASE_LP) {
		writel(val, (u8 *)pdvobjpriv->axi_sys_mem_start + addr);
	} else {
		RTW_ERR("%s: invalid base %08x.\n", __func__, base);
	}
	return 4;
}

#endif /* CONFIG_AXI_HCI */

void rtl8730aa_set_intf_ops(struct _io_ops *pops)
{

	_rtw_memset((u8 *)pops, 0, sizeof(struct _io_ops));


	pops->_read8 = &axi_read8;
	pops->_read16 = &axi_read16;
	pops->_read32 = &axi_read32;

	pops->_read_mem = &axi_read_mem;
	pops->_read_port = &axi_read_port;

	pops->_write8 = &axi_write8;
	pops->_write16 = &axi_write16;
	pops->_write32 = &axi_write32;

	pops->_write_mem = &axi_write_mem;
	pops->_write_port = &axi_write_port;

#ifdef CONFIG_AXI_HCI
	pops->_sys_read8 = &axi_sys_read8;
	pops->_sys_read16 = &axi_sys_read16;
	pops->_sys_read32 = &axi_sys_read32;

	pops->_sys_write8 = &axi_sys_write8;
	pops->_sys_write16 = &axi_sys_write16;
	pops->_sys_write32 = &axi_sys_write32;
#endif /* CONFIG_AXI_HCI */
}
