/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#ifndef __AXI_OSINTF_H
#define __AXI_OSINTF_H

#ifdef CONFIG_PLATFORM_RTK129X
#define PCIE_SLOT1_MEM_START	0x9804F000
#define PCIE_SLOT1_MEM_LEN	0x1000
#define PCIE_SLOT1_CTRL_START	0x9804EC00

#define PCIE_SLOT2_MEM_START	0x9803C000
#define PCIE_SLOT2_MEM_LEN	0x1000
#define PCIE_SLOT2_CTRL_START	0x9803BC00

#define PCIE_MASK_OFFSET	0x100 /* mask offset from CTRL_START */
#define PCIE_TRANSLATE_OFFSET	0x104 /* translate offset from CTRL_START */
#endif

#define PCI_BC_CLK_REQ		BIT0
#define PCI_BC_ASPM_L0s		BIT1
#define PCI_BC_ASPM_L1		BIT2
#define PCI_BC_ASPM_L1Off	BIT3
//#define PCI_BC_ASPM_LTR	BIT4
//#define PCI_BC_ASPM_OBFF	BIT5

void	PlatformClearPciPMEStatus(PADAPTER Adapter);

#endif
