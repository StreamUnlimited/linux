/** @file */
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

#ifndef _MAC_AX_TRXCFG_H_
#define _MAC_AX_TRXCFG_H_

#include "../type.h"
#include "hw.h"
#include "init.h"
#include "role.h"
#include "cmac_tx.h"
#include "rx_filter.h"
#include "dle.h"
#include "hci_fc.h"
#include "mport.h"
#include "spatial_reuse.h"

/*--------------------Define -------------------------------------------*/
#define TRXCFG_WAIT_CNT		2000
#define TRXCFG_WAIT_US		1

/* MPDU Processor Control */
#define TRXCFG_MPDU_PROC_ACT_FRWD	0x02A95A95
#define TRXCFG_MPDU_PROC_TF_FRWD	0x0000AA55
#define TRXCFG_MPDU_PROC_CUT_CTRL	0x010E05F0

/* RMAC timeout control */
#define TRXCFG_RMAC_CCA_TO	32
#define TRXCFG_RMAC_DATA_TO	15

#define S_AX_TXSC_20M_0		0
#define S_AX_TXSC_20M_4		4
#define S_AX_TXSC_40M_0		0
#define S_AX_TXSC_40M_4		4
#define S_AX_TXSC_80M_0		0
#define S_AX_TXSC_80M_4		4

#define RRSR_OFDM_CCK_EN	3

/* TRXPTCL SIFS TIME*/
#define WMAC_SPEC_SIFS_OFDM_52A 0x15
#define WMAC_SPEC_SIFS_OFDM_52B 0x11
#define WMAC_SPEC_SIFS_OFDM_52C 0x11
#define WMAC_SPEC_SIFS_CCK	 0xA

/* RRSR disable 5.5M CCK*/
#define WMAC_CCK_EN_1M 0x1
#define WMAC_RRSR_RATE_LEGACY_EN 0x1

/* SRAM fifo address */
#define CMAC_TBL_BASE_ADDR	0x18840000

#define CMAC1_START_ADDR	0xE000
#define CMAC1_END_ADDR		0xFFFF

#if MAC_AX_ASIC_TEMP
#define R_AX_LTECOEX_CTRL 0x38
#define R_AX_LTECOEX_CTRL_2 0x3C
#endif

#define S_AX_CTS2S_TH_1K 4
#define S_AX_CTS2S_TH_SEC_256B 1

#define S_AX_PTCL_TO_2MS 0x3F

#define LBK_PLCP_DLY_DEF 0x28
#define LBK_PLCP_DLY_FPGA 0x46

#define PLD_RLS_MAX_PG 127
#define RX_MAX_LEN_UNIT 512
/* if spec max len is not align to rx max len unit, add 1 unit */
#define RX_SPEC_MAX_LEN (11454 + RX_MAX_LEN_UNIT)

#define SCH_PREBKF_24US 0x18
#define SCH_PREBKF_16US 0x10

#define BCN_IFS_25US 0x19
#define SIFS_MACTXEN_T1_V0 0x47
#define SIFS_MACTXEN_T1_V1 0x41

#define SDIO_DRV_INFO_SIZE 2

#define DMA_MOD_PCIE_1B 0x0
#define DMA_MOD_PCIE_4B 0x1
#define DMA_MOD_USB 0x2
#define DMA_MOD_SDIO 0x3

#define  NAV_12MS 0x5D // (12ms, unit: 128us)
#define  NAV_25MS 0xC4 // (25ms, unit: 128us)

#define FWD_TO_HOST 0
#define FWD_TO_WLCPU 1
#define FWD_TO_DATACPU 2

#define AMPDU_MAX_LEN_VHT_262K 0x3FF80
#define SS2F_PATH_WLCPU 0x0A

#define NAV_UPPER_DEFAULT 0

#define TCR_UDF_THSD          0x6
#define TXDFIFO_HIGH_MCS_THRE 0x7
#define TXDFIFO_LOW_MCS_THRE  0x7

#define B_AX_TX_TO  0x2
#define DRVINFO_PATCH_SIZE  0x5

/*--------------------Define MACRO--------------------------------------*/
#define RX_FULL_MODE (B_AX_RU0_PTR_FULL_MODE | B_AX_RU1_PTR_FULL_MODE | \
		      B_AX_RU2_PTR_FULL_MODE | B_AX_RU3_PTR_FULL_MODE | \
		      B_AX_CSI_PTR_FULL_MODE | B_AX_RXSTS_PTR_FULL_MODE)

/*--------------------Define Enum---------------------------------------*/
/*--------------------Define Struct-------------------------------------*/

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

u32 ser_imr_config(struct mac_ax_adapter *adapter, u8 band,
		   enum mac_ax_hwmod_sel sel);

/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief check_mac_en
 *
 * @param *adapter
 * @param band
 * @param sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 check_mac_en(struct mac_ax_adapter *adapter,
		 enum mac_ax_hwmod_sel sel);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */


/**
 * @brief cmac_init
 *
 * @param *adapter
 * @param *info
 * @param band
 * @return Please Place Description here.
 * @retval u32
 */
u32 cmac_init(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info,
	      enum mac_ax_band band);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief cfg_sec
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_cfg_sec(struct mac_ax_adapter *adapter,
		struct halmac_security_setting *set);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */


/**
 * @brief mac_trx_init
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_trx_init(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */


/**
 * @brief mac_two_nav_cfg
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_two_nav_cfg(struct mac_ax_adapter *adapter,
		    struct mac_ax_2nav_info *info);
/**
 * @}
 * @}
 *
 *//**
 * @brief mac_sifs_chk_edcca_en
 *
 * @param *adapter
 * @param *band
 * @param *en
 * @return check cca in sifs enable/disable
 * @retval u32
 */
u32 mac_sifs_chk_cca_en(struct mac_ax_adapter *adapter, u8 en);
/**
 * @}
 * @}
 */

/**
 * @}
 * @}
 *
 *//**
 * @brief mac_sifs_chk_edcca_en_ax
 *
 * @param *adapter
 * @param *band
 * @param *en
 * @return check cca in sifs enable/disable
 * @retval u32
 */
u32 mac_resp_chk_cca(struct mac_ax_adapter *adapter,
		     struct mac_ax_resp_chk_cca *cfg);
/**
 * @}
 * @}
 */

u32 mac_clr_idx_all(struct mac_ax_adapter *adapter);

#endif
