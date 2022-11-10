/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef __PHL_API_TMP_H_
#define __PHL_API_TMP_H_

static inline u32 rtw_hal_get_txdesc_len(void *h,
		struct pkt_attrib *attrib)
{
	return 0;
};



#ifdef CONFIG_PCI_HCI
static inline u8 rtw_hal_pci_check_enough_txdesc(void *h, u8 queue_id)
{
	return 0;
}
#endif

#ifdef CONFIG_AXI_HCI
static inline u8 rtw_hal_axi_check_enough_txdesc(void *h, u8 queue_id)
{
	return 0;
}
#endif

#endif /*__PHL_API_TMP_H_*/
