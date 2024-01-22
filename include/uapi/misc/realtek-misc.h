// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek Misc support
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#ifndef _UAPI_REALTEK_MISC_H_
#define _UAPI_REALTEK_MISC_H_

/* ATTENTION */
/* Please keep the realtek-misc header for kernel space and user space in sync manually. */
/* Misc header for kernel space: <sdk>/kernel/linux-5.4/include/uapi/misc/realtek-misc.h */
/* Misc header for user space: <sdk>/prebuilts/kernel/include/uapi/misc/realtek-misc.h */

#define RTK_CMD_NOT_SET				0  // default: mode not set
#define RTK_CMD_SET_MODE			1  // call ioctl SET_CURRENT_MODE to change the affair modes.

/* Example of misc affairs. */
#define RTK_CMD_GET_RL_INFO         2
#define RTK_CMD_SET_BT_POWER_ON     3
#define RTK_CMD_SET_BT_POWER_OFF    4

/* RTK_CMD_GET_RL_INFO */
#define RTK_CUT_VERSION_A           0
#define RTK_CUT_VERSION_B           1
#define RTK_CUT_VERSION_C           2

/* Add customize affairs here. */
/* RTK_CMD_AAAAAA_AAAAAA_AA */
/* RTK_CMD_AAAAAA_BBBBBB_AA */

#endif
