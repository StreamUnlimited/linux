// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek OTP support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#ifndef _REALTEK_OTP_CORE_H_
#define _REALTEK_OTP_CORE_H_

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/if_link.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysrq.h>
#include <linux/of.h>
#include <linux/kern_levels.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <asm-generic/io.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#include <ameba_ipc/ameba_ipc.h>
#include <uapi/misc/realtek-otp.h>

enum {
	IPC_USER_POINT = 0,
	IPC_USER_DATA = 1
};

typedef struct otp_ipc_host_req_msg {
	u32 otp_id;
	u32 addr;
	u32 len;
	u32 write_lock;
	u8 param_buf[OPT_REQ_MSG_PARAM_NUM];
} otp_ipc_host_req_t;

struct rtk_otp {
    struct device		    *dev;
    struct aipc_ch          *potp_ipc_ch;
	otp_ipc_host_req_t      *preq_msg;              /* host api message to send to device */
	dma_addr_t              req_msg_phy_addr;       /* host api message to send to device */
	struct tasklet_struct   otp_tasklet;            /* api task to haddle api msg */
	ipc_msg_struct_t        otp_ipc_msg;            /* to store ipc msg for api */
    struct completion       otp_complete;           /* only one otp process can send ipc instruction */
};

extern int rtk_otp_process(void* data, u8 *result);

#endif
