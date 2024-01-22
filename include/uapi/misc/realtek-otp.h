// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek OTP support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#ifndef _UAPI_REALTEK_OTP_H_
#define _UAPI_REALTEK_OTP_H_

#define LINUX_IPC_OTP_PHY_READ8		    	0
#define LINUX_IPC_OTP_PHY_WRITE8		1
#define LINUX_IPC_OTP_LOGI_READ_MAP		2
#define LINUX_IPC_OTP_LOGI_WRITE_MAP		3
#define LINUX_IPC_EFUSE_REMAIN_LEN		4

#define OTP_LMAP_LEN				0x400
#define OPT_REQ_MSG_PARAM_NUM			OTP_LMAP_LEN

/* for user space. */
struct otp_ipc_tx_msg {
	int otp_id;
	int offset;
	int len;
	u8 tx_value[OPT_REQ_MSG_PARAM_NUM];
	u8 tx_lock;
};

struct user_space_request {
	u8	*result;
	struct otp_ipc_tx_msg otp_order;
};

#define OTP_IPC_RET_LEN				2
typedef struct otp_ipc_rx_res {
	int ret;
	int complete_num;
} otp_ipc_rx_res_t;

enum realtek_otp_mode {
	RTK_OTP_PHYSIC,
	RTK_OTP_LOGIC,
};

enum realtek_otp_operation {
	RTK_OTP_READ,
	RTK_OTP_WRITE,
};

#endif

