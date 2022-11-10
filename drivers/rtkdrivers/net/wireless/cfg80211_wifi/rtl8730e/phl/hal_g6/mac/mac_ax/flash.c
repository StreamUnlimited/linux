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

#include "flash.h"
#include "../mac_ax/fwcmd.h"

#if MAC_AX_FW_REG_OFLD
u32 mac_flash_erase(struct mac_ax_adapter *adapter,
		    u32 addr,
		    u32 length,
		    u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_flash_read(struct mac_ax_adapter *adapter,
		   u32 addr,
		   u32 length,
		   u8 *buffer,
		   u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_flash_write(struct mac_ax_adapter *adapter,
		    u32 addr,
		    u32 length,
		    u8 *buffer,
		    u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

#else
u32 mac_flash_erase(struct mac_ax_adapter *adapter,
		    u32 addr,
		    u32 length,
		    u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_flash_read(struct mac_ax_adapter *adapter,
		   u32 addr,
		   u32 length,
		   u8 *buffer,
		   u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_flash_write(struct mac_ax_adapter *adapter,
		    u32 addr,
		    u32 length,
		    u8 *buffer,
		    u32 timeout)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}
#endif

u32 c2h_sys_flash_pkt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		      struct rtw_c2h_info *info)
{
	u32 hdr0, i;
	u32 func, length, addr;
	u32 *pbuf;
	u32 *psource_data;

	PLTFM_MSG_TRACE("%s\n", __func__);
	hdr0 = ((struct fwcmd_hdr *)buf)->hdr0;
	hdr0 = le32_to_cpu(hdr0);

	//set info
	info->c2h_cat = GET_FIELD(hdr0, C2H_HDR_CAT);
	info->c2h_class = GET_FIELD(hdr0, C2H_HDR_CLASS);
	info->c2h_func = GET_FIELD(hdr0, C2H_HDR_FUNC);
	//info->done_ack = 0;
	//info->rec_ack = 0;
	info->content = buf  + FWCMD_HDR_LEN;
	//info->h2c_return = info->c2h_data[1];

	func = GET_FIELD(hdr0, C2H_HDR_FUNC);

	if (func == FWCMD_H2C_FUNC_PLAT_FLASH_READ) {
		addr = *((u32 *)info->content);
		addr = le32_to_cpu(addr);
		length = *((u32 *)(info->content + LEN_FLASH_C2H_HDR_ADDR));
		length = le32_to_cpu(length);
		PLTFM_MSG_TRACE("addr = 0x%x, length = 0x%x\n", addr, length);
		PLTFM_MSG_TRACE("adapter->flash_info.read_addr = 0x%x\n",
				adapter->flash_info.read_addr);
		//PLTFM_MSG_TRACE("adapter->flash_info.buf_addr = 0x%llx\n",
		//(u64)adapter->flash_info.buf_addr);

		PLTFM_MUTEX_LOCK(&adapter->flash_info.lock);
		if (addr == adapter->flash_info.read_addr && adapter->flash_info.reading == 1) {
			// lost data if timeout happened
			//PLTFM_MSG_TRACE("ready to read\n");
			if (adapter->flash_info.buf_addr && length <= MAX_READ_SIZE) {
				PLTFM_MSG_TRACE("memcpy to buf\n");
				//PLTFM_MEMCPY(adapter->flash_info.buf_addr,
				//	     info->content + LEN_FLASH_C2H_HDR, length);
				pbuf = (u32 *)adapter->flash_info.buf_addr;
				psource_data = (u32 *)(info->content + LEN_FLASH_C2H_HDR);
				for (i = 0; i < length / sizeof(u32); i++) {
					*pbuf = le32_to_cpu(*psource_data);
					pbuf++;
					psource_data++;
				}
				//PLTFM_MSG_TRACE("memcpy to buf end\n");
			}
		}
		//PLTFM_MSG_TRACE("set reading = 0\n");
		adapter->flash_info.reading = 0;
		adapter->flash_info.read_done = 1;
		//PLTFM_MSG_TRACE("set reading = 0 end\n");
		PLTFM_MUTEX_UNLOCK(&adapter->flash_info.lock);
		//PLTFM_MSG_TRACE("exit critical\n");
	} else if (func == FWCMD_H2C_FUNC_PLAT_FLASH_WRITE) {
		addr = *((u32 *)info->content);
		addr = le32_to_cpu(addr);
		length = *((u32 *)(info->content + LEN_FLASH_C2H_HDR_ADDR));
		length = le32_to_cpu(length);
		PLTFM_MSG_TRACE("write addr = 0x%x\n", *((u32 *)info->content));
		PLTFM_MSG_TRACE("adapter->flash_info.write_addr = 0x%x\n",
				adapter->flash_info.write_addr);
		PLTFM_MSG_TRACE("write length = 0x%x\n", length);

		PLTFM_MUTEX_LOCK(&adapter->flash_info.lock);
		if (addr == adapter->flash_info.write_addr && adapter->flash_info.writing == 1) {
			adapter->flash_info.writing = 0;
			adapter->flash_info.write_done = 1;
		}
		PLTFM_MUTEX_UNLOCK(&adapter->flash_info.lock);
	} else if (func == FWCMD_H2C_FUNC_PLAT_FLASH_ERASE) {
		addr = *((u32 *)info->content);
		addr = le32_to_cpu(addr);
		length = *((u32 *)(info->content + LEN_FLASH_C2H_HDR_ADDR));
		length = le32_to_cpu(length);
		PLTFM_MSG_TRACE("erase addr = 0x%x\n", *((u32 *)info->content));
		PLTFM_MSG_TRACE("adapter->flash_info.erase_addr = 0x%x\n",
				adapter->flash_info.erase_addr);
		PLTFM_MSG_TRACE("erase length = 0x%x\n", length);

		PLTFM_MUTEX_LOCK(&adapter->flash_info.lock);
		if (addr == adapter->flash_info.erase_addr && adapter->flash_info.erasing == 1) {
			adapter->flash_info.erasing = 0;
			adapter->flash_info.erase_done = 1;
		}
		PLTFM_MUTEX_UNLOCK(&adapter->flash_info.lock);
	}

	return MACSUCCESS;
}

