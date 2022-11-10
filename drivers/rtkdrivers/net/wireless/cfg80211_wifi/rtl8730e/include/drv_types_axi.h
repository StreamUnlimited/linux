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
#ifndef __DRV_TYPES_AXI_H__
#define __DRV_TYPES_AXI_H__

#ifdef PLATFORM_LINUX
#include <linux/dma-mapping.h>
#endif

#define	INTEL_VENDOR_ID				0x8086
#define	SIS_VENDOR_ID					0x1039
#define	ATI_VENDOR_ID					0x1002
#define	ATI_DEVICE_ID					0x7914
#define	AMD_VENDOR_ID					0x1022

typedef struct _RT_ISR_CONTENT {
	union {
		u32			IntArray[2];
		u32			IntReg4Byte;
		u16			IntReg2Byte;
	};
} RT_ISR_CONTENT, *PRT_ISR_CONTENT;

typedef struct axi_data {
#ifdef PLATFORM_LINUX
	struct platform_device *pdev;

	/* AXI MEM map */
	unsigned long axi_mem_end; /* shared mem end */
	unsigned long axi_mem_start; /* shared mem start */

	/* AXI MEM map */
	unsigned long axi_sys_mem_end; /* shared mem end */
	unsigned long axi_sys_mem_start; /* shared mem start */

	u8 irq_alloc;
	unsigned int irq_protocol; /* wifi protocol interrupt */
	unsigned int irq_dma; /* wifi packet dma interrupt */
	u16 irqline;
	u8 irq_enabled;
	RT_ISR_CONTENT isr_content;
	_lock irq_th_lock;

	u8 bdma64;

	aipc_ch_t *fw_ipc;
#endif/* PLATFORM_LINUX */
} AXI_DATA, *PAXI_DATA;

#endif
