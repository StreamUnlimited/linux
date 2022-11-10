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
#ifndef _RTW_IO_H_
#define _RTW_IO_H_

#ifdef CONFIG_PCI_HCI
#define MAX_CONTINUAL_IO_ERR 4
#endif

#ifdef CONFIG_AXI_HCI
#define MAX_CONTINUAL_IO_ERR 4
#endif


int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj);
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj);

#endif /* _RTW_IO_H_ */
