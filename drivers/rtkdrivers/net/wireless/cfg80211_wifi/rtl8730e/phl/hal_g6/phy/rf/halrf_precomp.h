/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALRF_PRECOMP_H__
#define __HALRF_PRECOMP_H__

/*@--------------------------[Define] ---------------------------------------*/
#define	MASKBYTE0		0xff
#define	MASKBYTE1		0xff00
#define	MASKBYTE2		0xff0000
#define	MASKBYTE3		0xff000000
#define	MASKHWORD		0xffff0000
#define	MASKLWORD		0x0000ffff
#define	MASKDWORD		0xffffffff
#define	MASKRF		0xfffff
#define	MASKRFMODE		0xf0000
#define	MASKRFRXBB		0x003e0
#define	MASKTXPWR		0x0003f
#define	INVALID_RF_DATA 0xffffffff


/*---[Define Only] ----------------------------------------------------------*/
#include "../../hal_headers_le.h"

#ifdef CONFIG_RTL8730E
#define RF_8730E_SUPPORT
#define RF_6G_NOT_SUPPORT
#define RF_MULTIPATH_NOT_SUPPORT
#define RF_BW_20M_ONLY
#endif

#ifdef CONFIG_RTL8720E
#define RF_8720E_SUPPORT
#define RF_5G_NOT_SUPPORT
#define RF_6G_NOT_SUPPORT
#define RF_MULTIPATH_NOT_SUPPORT
#define RF_BW_20M_ONLY
#endif

#if defined (RF_8730E_SUPPORT) || defined (RF_8720E_SUPPORT)
#define IOT_SMALL_RAM
#define RF_NONBF
#endif

#include "halrf_ic_hw_info.h"
#include "halrf_ic_sw_info.h"
/*---[Include structure & prototype] ----------------------------------------*/

#include "halrf_hw_cfg.h"
#include "halrf_hw_cfg_ex.h"
#include "halrf_interface.h"
#include "halrf_dbg_cmd.h"
#include "halrf_dbg.h"
#include "halrf_txgapk.h"
#include "halrf_pwr_track.h"
#include "halrf_iqk.h"
#include "halrf_dpk.h"
#include "halrf_dack.h"
#include "halrf_pmac.h"
//#include "halrf_dbg_cmd.h"
#include "halrf_dbg_cmd_ex.h"
#include "halrf_init.h"
#include "halrf_init_ex.h"
#include "halrf_pwr_table.h"
#include "halrf_api.h"
#include "halrf_psd.h"
#include "halrf_kfree.h"
#include "halrf_hwimg.h"

#ifdef RF_8730E_SUPPORT
#include "halrf_8730e/halrf_dack_8730e.h"
#include "halrf_8730e/halrf_hwimg_8730e.h"
#include "halrf_8730e/halrf_kfree_8730e.h"
#endif

#ifdef RF_8720E_SUPPORT
#include "halrf_8720e/halrf_dack_8720e.h"
#include "halrf_8720e/halrf_hwimg_8720e.h"
#include "halrf_8720e/halrf_kfree_8720e.h"
#endif

#include "halrf.h"
#include "halrf_ex.h"

#ifdef RF_8730E_SUPPORT
#include "halrf_8730e/halrf_efuse_8730e.h"
#include "halrf_8730e/halrf_reg_cfg_8730e.h"
#include "halrf_8730e/halrf_8730e.h"
#include "halrf_8730e/halrf_8730e_api.h"
//	#include "halrf_8730e/halrf_8730e_api_ex.h"
#include "halrf_8730e/halrf_iqk_8730e.h"
#include "halrf_8730e/halrf_dpk_8730e.h"
#include "halrf_8730e/halrf_txgapk_8730e.h"
#include "halrf_8730e/halrf_version_rtl8730e.h"
#include "halrf_8730e/halrf_set_pwr_table_8730e.h"
#include "halrf_8730e/halrf_tssi_8730e.h"
#include "halrf_8730e/halrf_psd_8730e.h"
#endif

#ifdef RF_8720E_SUPPORT
#include "halrf_8720e/halrf_efuse_8720e.h"
#include "halrf_8720e/halrf_reg_cfg_8720e.h"
#include "halrf_8720e/halrf_8720e.h"
#include "halrf_8720e/halrf_8720e_api.h"
//	#include "halrf_8720e/halrf_8720e_api_ex.h"
#include "halrf_8720e/halrf_iqk_8720e.h"
#include "halrf_8720e/halrf_dpk_8720e.h"
#include "halrf_8720e/halrf_txgapk_8720e.h"
#include "halrf_8720e/halrf_version_rtl8720e.h"
#include "halrf_8720e/halrf_set_pwr_table_8720e.h"
#include "halrf_8720e/halrf_tssi_8720e.h"
#include "halrf_8720e/halrf_psd_8720e.h"
#endif

#include "hal_struct.h"

#endif
