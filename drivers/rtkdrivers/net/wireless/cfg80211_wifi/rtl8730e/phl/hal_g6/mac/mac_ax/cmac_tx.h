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

#ifndef _MAC_AX_CMAC_TX_H_
#define _MAC_AX_CMAC_TX_H_

#include "../type.h"
#include "trxcfg.h"
#include "role.h"
#include "hw.h"
#include "dbgpkg.h"

/*--------------------Define ----------------------------------------*/
#define PTCL_TXQ_WMM0_BE	0
#define PTCL_TXQ_WMM0_BK	1
#define PTCL_TXQ_WMM0_VI	2
#define PTCL_TXQ_WMM0_VO	3
#define PTCL_TXQ_WMM1_BE	4
#define PTCL_TXQ_WMM1_BK	5
#define PTCL_TXQ_WMM1_VI	6
#define PTCL_TXQ_WMM1_VO	7
#define PTCL_TXQ_MG0		8
#define PTCL_TXQ_MG1		9
#define PTCL_TXQ_MG2		10
#define PTCL_TXQ_HIQ		11
#define PTCL_TXQ_BCNQ		12
#define PTCL_TXQ_UL		13
#define PTCL_TXQ_TWT0		14
#define PTCL_TXQ_TWT1		15

#define TX_PAUSE_WAIT_CNT	5000

#define PTCL_IDLE_POLL_CNT	2200
#define SW_CVR_DUR_US	30
#define SW_CVR_CNT	8
#define TX_DLY_MAX	9

#define MACTXEN_MAX	81
#define MACTXEN_MIN	56

/*--------------------Define Enum------------------------------------*/

/**
 * @enum tb_stop_sel
 *
 * @brief tb_stop_sel
 *
 * @var tb_stop_sel::TB_STOP_SEL_BE
 * Please Place Description here.
 * @var tb_stop_sel::TB_STOP_SEL_BK
 * Please Place Description here.
 * @var tb_stop_sel::TB_STOP_SEL_VI
 * Please Place Description here.
 * @var tb_stop_sel::TB_STOP_SEL_VO
 * Please Place Description here.
 * @var tb_stop_sel::TB_STOP_SEL_ALL
 * Please Place Description here.
 */
enum tb_stop_sel {
	TB_STOP_SEL_BE,
	TB_STOP_SEL_BK,
	TB_STOP_SEL_VI,
	TB_STOP_SEL_VO,
	TB_STOP_SEL_ALL,
};

/**
 * @enum sch_tx_sel
 *
 * @brief sch_tx_sel
 *
 * @var sch_tx_sel::SCH_TX_SEL_ALL
 * Please Place Description here.
 * @var sch_tx_sel::SCH_TX_SEL_HIQ
 * Please Place Description here.
 * @var sch_tx_sel::SCH_TX_SEL_MG0
 * Please Place Description here.
 * @var sch_tx_sel::SCH_TX_SEL_MACID
 * Please Place Description here.
 */
enum sch_tx_sel {
	SCH_TX_SEL_ALL,
	SCH_TX_SEL_HIQ,
	SCH_TX_SEL_MG0,
	SCH_TX_SEL_MACID,
};

/**
 * @enum ptcl_tx_sel
 *
 * @brief ptcl_tx_sel
 *
 * @var ptcl_tx_sel::PTCL_TX_SEL_HIQ
 * Please Place Description here.
 * @var ptcl_tx_sel::PTCL_TX_SEL_MG0
 * Please Place Description here.
 */
enum ptcl_tx_sel {
	PTCL_TX_SEL_HIQ,
	PTCL_TX_SEL_MG0,
};

/*--------------------Define MACRO----------------------------------*/
/*--------------------Define Struct----------------------------------*/

/**
 * @struct sch_tx_en_h2creg
 * @brief sch_tx_en_h2creg
 *
 * @var sch_tx_en_h2creg::tx_en
 * Please Place Description here.
 * @var sch_tx_en_h2creg::mask
 * Please Place Description here.
 * @var sch_tx_en_h2creg::band
 * Please Place Description here.
 * @var sch_tx_en_h2creg::rsvd
 * Please Place Description here.
 */
struct sch_tx_en_h2creg {
	/* dword0 */
	u32 tx_en: 16;
	/* dword1 */
	u32 mask: 16;
	u32 band: 1;
	u32 rsvd0: 15;
};

/*--------------------Export global variable----------------------------*/
/*--------------------Function declaration-----------------------------*/

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief set_hw_ampdu_cfg
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg);
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
 * @brief set_hw_fast_edca_param
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_fast_edca_param(struct mac_ax_adapter *adapter,
			   struct mac_ax_usr_edca_param *param);
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
 * @brief set_hw_edca_param
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param);
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
 * @brief get_hw_edca_param
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 get_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param);
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
 * @brief set_hw_muedca_param
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param);
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
 * @brief set_hw_muedca_ctrl
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg);
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
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */


/**
 * @brief set_hw_sch_tx_en
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg);
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
 * @brief hw_sch_tx_en
 *
 * @param *adapter
 * @param band
 * @param tx_en_u16
 * @param mask_u16
 * @return Please Place Description here.
 * @retval u32
 */
u32 hw_sch_tx_en(struct mac_ax_adapter *adapter, u8 band,
		 u16 tx_en_u16, u16 mask_u16);
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
 * @brief get_hw_sch_tx_en
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 get_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg);
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
 * @brief set_hw_lifetime_cfg
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg);
/**
 * @}
 * @}
 */

/**
 * @brief get_hw_lifetime_cfg
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 get_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg);
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
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief resume_sch_tx
 *
 * @param *adapter
 * @param *bak
 * @return Please Place Description here.
 * @retval u32
 */
u32 resume_sch_tx(struct mac_ax_adapter *adapter,
		  struct mac_ax_sch_tx_en_cfg *bak);
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
 * @brief tx_idle_poll_sel
 *
 * @param *adapter
 * @param sel
 * @param band
 * @return Please Place Description here.
 * @retval u32
 */
u32 tx_idle_poll_sel(struct mac_ax_adapter *adapter, enum ptcl_tx_sel sel,
		     u8 band);
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

u32 get_edca_addr(struct mac_ax_adapter *adapter,
		  struct mac_ax_edca_param *param, u32 *reg_edca);

u32 get_muedca_param_addr(struct mac_ax_adapter *adapter,
			  struct mac_ax_muedca_param *param,
			  u32 *reg_edca);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief mac_set_hw_rts_th
 *
 * @param *adapter
 * @param *
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_set_hw_rts_th(struct mac_ax_adapter *adapter,
		      struct mac_ax_hw_rts_th *th);
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
 * @brief mac_get_hw_rts_th
 *
 * @param *adapter
 * @param *
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_get_hw_rts_th(struct mac_ax_adapter *adapter,
		      struct mac_ax_hw_rts_th *th);
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
 * @brief mac_tx_duty
 *
 * @param *adapter
 * @param pause_intvl
 * @param tx_intvl
 * @return Do tx_duty for Thermal Prodection
 * @retval u32
 */
u32 mac_tx_duty(struct mac_ax_adapter *adapter,
		u16 pause_intvl, u16 tx_intvl);
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
 * @brief mac_tx_duty_stop
 *
 * @param *adapter
 * @param pause_intvl
 * @param tx_intvl
 * @return Stop tx_duty for Thermal Prodection
 * @retval u32
 */
u32 mac_tx_duty_stop(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

void u16_2_sch(struct mac_ax_adapter *adapter,
	       struct mac_ax_sch_tx_en *tx_en, u16 val16);
void u32_2_sch(struct mac_ax_adapter *adapter,
	       struct mac_ax_sch_tx_en *tx_en, u32 val32);

#endif
