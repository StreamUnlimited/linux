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

#ifndef _MAC_AX_PCIE_H_
#define _MAC_AX_PCIE_H_

#include "../type.h"
//#include "../pcie_reg.h"
#include "../mac_ax.h"
#if defined(CONFIG_RTL8730E)
#include "rtl8730e/axi/hal_trx_8730ea.h"
#endif

/*--------------------Define -------------------------------------------*/
#define INTF_INTGRA_MINREF_V1	90
#define INTF_INTGRA_HOSTREF_V1	100

#define PCIE_POLL_IO_IDLE_CNT 100
#define PCIE_POLL_IO_IDLE_DLY_US 10
#define PCIE_POLL_DMACH_IDLE_CNT 100
#define PCIE_POLL_DMACH_IDLE_DLY_US 10

#define PCIE_POLL_BDRAM_RST_CNT 10000
#define PCIE_POLL_BDRAM_RST_DLY_US 50

#define PCIE_POLL_AUTOK_CNT 1000
#define PCIE_POLL_AUTOK_DLY_US 50

#define MIO_ADDR_PAGE_SH 8
#define MIO_WRITE_BYTE_ALL 0xF
#define MIO_4BYTE_ALIGN 4
#define MIO_SYNC_CNT 1000
#define MIO_SYNC_DLY 1

#define DBI_ADDR_MASK 0xFFC
#define DBI_ADDR_2LSB_MASK 0x3
#define DBI_WEN_DW 0xF
#define DBI_WEN_B 1
#define DBI_DLY_CNT 20
#define DBI_DLY_US 10

#define MDIO_ADDR_PG1 0x20
#define MDIO_DLY_CNT 20
#define MDIO_DLY_US 10

#define BDRAM_SIDX_MSK 0x1f
#define BDRAM_MAX_MSK 0x1f00
#define BDRAM_MIN_MSK 0x1f0000

#define MDIO_PG0_G1 0
#define MDIO_PG1_G1 1
#define MDIO_PG0_G2 2
#define MDIO_PG1_G2 3

#define BD_NORM_SIZE 12
#define BD_TRUNC_SIZE 8
#define RXBD_SEP_NORM_SIZE 20
#define RXBD_SEP_TRUNC_OLD_SIZE 12
#define RXBD_SEP_TRUNC_NEW_SIZE 16

#define BD_MAX_NUM 0x3FF
#define BD_IDX_INVALID 0xFFFF
#define TXBD_BYTE_ALIGN 8
#define RXBD_BYTE_ALIGN 4

#define CMAC_CLK_ALLEN 0xFFFFFFFF

#define PC_POWER_UP 1
#define PC_POWER_DOWN 0
#define BIT_WAKE_CTRL BIT5
#define AXI_DEFAULT_AGG_NUM 0x40
#define PCIE_8852A_AGG_NUM 0x80
#define PCIE_8852B_AGG_NUM 0x80
#define PCIE_8852C_AGG_NUM 0x100
#define PCIE_8192XB_AGG_NUM 0x100
#define PCIE_1115E_AGG_NUM 0x100

#define PCIE_AUTOK_DIV_2048 0x0
#define PCIE_AUTOK_MGN 0x8
#define PCIE_AUTOK_MGN_2048 64
#define PCIE_AUTOK_UD_CNT 30
#define PCIE_DPHY_DLY_0 0x0
#define PCIE_DPHY_DLY_25US 0x1
#define PCIE_AUTOK_4 0x3

#define PROC_ID_LIST_NUM 2
#define PROC_LONG_DLY 1
#define PROC_SHORT_DLY 0

#define PCIE_TP_THOLD 100

#define HAXIDMA_SYNC_TX_CH_NUM	6
#define HAXIDMA_SYNC_RX_CH_NUM	2

#define HW_BD_IDX_MSK	0xFFFF
#define HW_BD_IDX_SH	16

#define HOST_BD_IDX_MSK	0xFFFF
#define HOST_BD_IDX_SH	0

/*--------------------Define MACRO--------------------------------------*/
/*--------------------Define Enum---------------------------------------*/
enum axi_bd_ctrl_type {
	AXI_BD_CTRL_DESC_L = 0,
	AXI_BD_CTRL_DESC_H,
	AXI_BD_CTRL_NUM,
	AXI_BD_CTRL_IDX,
	AXI_BD_CTRL_BDRAM,

	AXI_BD_CTRL_LAST,
	AXI_BD_CTRL_MAX = AXI_BD_CTRL_LAST,
	AXI_BD_CTRL_INVALID = AXI_BD_CTRL_LAST,
};

/*--------------------Define Struct-------------------------------------*/
struct txbd_ram {
	u8 sidx;
	u8 max;
	u8 min;
};

/**
 * @brief reg_read8_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u8
 */
u8 reg_read8_axi(struct mac_ax_adapter *adapter, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_read8_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u8
 */
u8 sys_reg_read8_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup BasicIO
 * @{
 */

/**
 * @brief reg_write8_axi
 *
 * @param *adapter
 * @param addr
 * @param val
 * @return Please Place Description here.
 * @retval void
 */
void reg_write8_axi(struct mac_ax_adapter *adapter, u32 addr, u8 val);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_write8_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval void
 */
void sys_reg_write8_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u8 val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup BasicIO
 * @{
 */

/**
 * @brief reg_read16_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u16
 */
u16 reg_read16_axi(struct mac_ax_adapter *adapter, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_read16_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u16
 */
u16 sys_reg_read16_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup BasicIO
 * @{
 */
/**
 * @brief reg_write16_axi
 *
 * @param *adapter
 * @param addr
 * @param val
 * @return Please Place Description here.
 * @retval void
 */
void reg_write16_axi(struct mac_ax_adapter *adapter, u32 addr, u16 val);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_write16_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval void
 */
void sys_reg_write16_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u16 val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup BasicIO
 * @{
 */

/**
 * @brief reg_read32_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u32
 */
u32 reg_read32_axi(struct mac_ax_adapter *adapter, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_read32_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval u32
 */
u32 sys_reg_read32_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup BasicIO
 * @{
 */

/**
 * @brief reg_write32_axi
 *
 * @param *adapter
 * @param addr
 * @param val
 * @return Please Place Description here.
 * @retval void
 */
void reg_write32_axi(struct mac_ax_adapter *adapter, u32 addr, u32 val);
/**
 * @}
 * @}
 */

/**
 * @brief sys_reg_write32_axi
 *
 * @param *adapter
 * @param addr
 * @return Please Place Description here.
 * @retval void
 */
void sys_reg_write32_axi(struct mac_ax_adapter *adapter, u32 base, u32 addr, u32 val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief dbi_r8_pcie
 *
 * @param *adapter
 * @param addr
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbi_r8_pcie(struct mac_ax_adapter *adapter, u16 addr, u8 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief dbi_w8_pcie
 *
 * @param *adapter
 * @param addr
 * @param data
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbi_w8_pcie(struct mac_ax_adapter *adapter, u16 addr, u8 data);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief dbi_r32_pcie
 *
 * @param *adapter
 * @param addr
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbi_r32_pcie(struct mac_ax_adapter *adapter, u16 addr, u32 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief dbi_w32_pcie
 *
 * @param *adapter
 * @param addr
 * @param data
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbi_w32_pcie(struct mac_ax_adapter *adapter, u16 addr, u32 data);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief mdio_r16_pcie
 *
 * @param *adapter
 * @param addr
 * @param speed
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mdio_r16_pcie(struct mac_ax_adapter *adapter, u8 addr, u8 speed, u16 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief mdio_w16_pcie
 *
 * @param *adapter
 * @param addr
 * @param data
 * @param speed
 * @return Please Place Description here.
 * @retval u32
 */
u32 mdio_w16_pcie(struct mac_ax_adapter *adapter, u8 addr, u16 data, u8 speed);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief update_pcie_func_u8
 *
 * @param *val
 * @param bitmask
 * @param ctrl
 * @param def_ctrl
 * @return Please Place Description here.
 * @retval u32
 */
void update_pcie_func_u8(u8 *val, u8 bitmask,
			 enum mac_ax_pcie_func_ctrl ctrl,
			 enum mac_ax_pcie_func_ctrl def_ctrl);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief calc_avail_wptr
 *
 * @param rptr
 * @param wptr
 * @param bndy
 * @return Please Place Description here.
 * @retval u32
 */
u16 calc_avail_wptr(u16 rptr, u16 wptr, u16 bndy);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief calc_avail_rptr
 *
 * @param rptr
 * @param wptr
 * @param bndy
 * @return Please Place Description here.
 * @retval u32
 */
u16 calc_avail_rptr(u16 rptr, u16 wptr, u16 bndy);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief cfgspc_set_pcie
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
//u32 cfgspc_set_pcie(struct mac_ax_adapter *adapter,
//		    struct mac_ax_pcie_cfgspc_param *param);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief ltr_set_pcie
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 ltr_set_pcie(struct mac_ax_adapter *adapter,
		 struct mac_ax_pcie_ltr_param *param);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief clr_idx_all_pcie
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
//u32 clr_idx_all_pcie(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief pcie_pre_init
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 pcie_pre_init(struct mac_ax_adapter *adapter, void *param);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief pcie_init
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 pcie_init(struct mac_ax_adapter *adapter, void *param);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

/**
 * @brief pcie_deinit
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 pcie_deinit(struct mac_ax_adapter *adapter, void *param);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */


/**
 * @brief axi_pwr_switch
 *
 * @param *vadapter
 * @param pre_switch
 * @param on
 * @return Please Place Description here.
 * @retval u32
 */
u32 axi_pwr_switch(void *vadapter,
		   u8 pre_switch, u8 on);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */


/**
 * @brief set_pcie_l2_leave
 *
 * @param *adapter
 * @param set
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_pcie_l2_leave(struct mac_ax_adapter *adapter, u8 set);
/**
 * @}
 * @}
 */


/**
 * @brief pcie_autok_counter_avg
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 pcie_autok_counter_avg(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @addtogroup HCI
 * @{
 * @addtogroup PCIE
 * @{
 */

u32 axi_get_txagg_num(struct mac_ax_adapter *adapter, u8 band);


#endif
