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

#ifndef _MAC_AX_EFUSE_H_
#define _MAC_AX_EFUSE_H_

#include "../type.h"
#include "fwcmd.h"

#define phy_log_mapping_size                           0x200
#define OTP_LTYP0									0x00UL
#define OTP_LTYP1									0x01UL
#define OTP_LTYP2									0x02UL
#define OTP_LTYP3									0x03UL
#define OTP_LBASE_EFUSE							0x07UL
#define OTP_LBASE_PAD								0x06UL
#define OTP_LBASE_REG								0x0FUL
#define OTP_LBASE_ROM								0x0EUL
#define OTP_GET_LTYP(x)					((u32)(((x >> 28) & 0x0000000F)))
#define OTP_GET_LTYP1_DATA(x)			((u32)(((x >> 16) & 0x000000FF)))
#define OTP_GET_LTYP1_BASE(x)			((u32)(((x >> 12) & 0x0000000F)))
#define OTP_GET_LTYP1_OFFSET(x)			((u32)(((x >> 0) & 0x00000FFF)))

#define OTP_GET_LTYP2_LEN(x)			((u32)((((x >> 24) & 0x0000000F) + 1) << 2))
#define OTP_SET_LTYP2_LEN(x)			((u32)( ((x & 0x03F) >> 2) - 1))
#define OTP_GET_LTYP2_BASE(x)			((u32)(((x >> 12) & 0x0000000F)))
#define OTP_GET_LTYP2_OFFSET(x)			((u32)(((x >> 0) & 0x00000FFF)))

#define OTP_GET_LTYP3_OFFSET(x)			((u32)(((x >> 0) & 0x0FFFFFFF)))

#define REG_NON_SEC  0x42000008
#define REG_SEC   0x5200000c

#define RSVD_EFUSE_SIZE		16
#define RSVD_CS_EFUSE_SIZE	24
#define EFUSE_WAIT_CNT		10000
#define EFUSE_WAIT_CNT_PLUS	30000
#define EFUSE_C2HREG_WAIT_CNT	10000
#define EFUSE_C2HREG_RETRY_WAIT_US 1
#define EFUSE_FW_DUMP_WAIT_CNT	100
#define EFUSE_FW_DUMP_WAIT_CNT_V1 400
#define OTP_PHY_SIZE		0x800
#define CHK_OTP_ADDR		0x4
#define CHK_OTP_WAIT_CNT	50000
#define DUMMY_READ_DELAY	200

#define BT_1B_ENTRY_SIZE	0x80
#define UNLOCK_CODE		0x69

#define XTAL_SI_PWR_CUT		0x10
#define XTAL_SI_SMALL_PWR_CUT	BIT(0)
#define XTAL_SI_BIG_PWR_CUT	BIT(1)

#define XTAL_SI_LOW_ADDR	0x62
#define XTAL_SI_LOW_ADDR_SH	0
#define XTAL_SI_LOW_ADDR_MSK	0xFF

#define XTAL_SI_CTRL		0x63
#define XTAL_SI_MODE_SEL_SH	6
#define XTAL_SI_MODE_SEL_MSK	0x3
#define XTAL_SI_RDY		BIT(5)
#define XTAL_SI_HIGH_ADDR_SH	0
#define XTAL_SI_HIGH_ADDR_MSK	0x7

#define XTAL_SI_READ_VAL	0x7A
#define XTAL_SI_WRITE_VAL	0x60
#define XTAL_SI_WRITE_DATA_SH	0
#define XTAL_SI_WRITE_DATA_MSK	0xFF

#define BT_DIS_WAIT_CNT	100
#define BT_DIS_WAIT_US	50

extern struct mac_bank_efuse_info bank_efuse_info;
extern enum rtw_dv_sel dv_sel;

/**
 * @struct mac_efuse_tbl
 * @brief mac_efuse_tbl
 *
 * @var mac_efuse_tbl::lock
 * Please Place Description here.
 */
struct mac_efuse_tbl {
	mac_ax_mutex lock;
};

/**
 * @struct mac_efuse_hidden_h2creg
 * @brief mac_efuse_hidden_h2creg
 *
 * @var mac_efuse_hidden_h2creg::rsvd0
 * Please Place Description here.
 */
struct mac_efuse_hidden_h2creg {
	/* dword0 */
	u32 rsvd0: 16;
};

/**
 * @enum efuse_map_sel
 *
 * @brief efuse_map_sel
 *
 * @var efuse_map_sel::EFUSE_MAP_SEL_PHY_WL
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_PHY_BT
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_LOG
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_LOG_BT
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_PHY_OTP
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_LAST
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_MAX
 * Please Place Description here.
 * @var efuse_map_sel::EFUSE_MAP_SEL_INVALID
 * Please Place Description here.
 */
enum efuse_map_sel {
	/*EFUSE_MAP_SEL_PHY_WL,
	EFUSE_MAP_SEL_PHY_BT,
	EFUSE_MAP_SEL_LOG,
	EFUSE_MAP_SEL_LOG_BT,
	EFUSE_MAP_SEL_PHY_OTP,
	EFUSE_MAP_SEL_PHY_DAV,
	EFUSE_MAP_SEL_LOG_DAV,
	EFUSE_MAP_SEL_HIDDEN_RF,*/
	EFUSE_MAP_SEL_PHY,
	EFUSE_MAP_SEL_LOG,

	/* keep last */
	EFUSE_MAP_SEL_LAST,
	EFUSE_MAP_SEL_MAX = EFUSE_MAP_SEL_LAST,
	EFUSE_MAP_SEL_INVALID = EFUSE_MAP_SEL_LAST,
};

/**
 * @struct efuse_info_item
 * @brief efuse_info_item
 *
 * @var efuse_info_item::mac_addr
 * MAC Address
 * @var efuse_info_item::pid
 * Product ID
 * @var efuse_info_item::did
 * Device ID
 * @var efuse_info_item::vid
 * Vendor ID
 * @var efuse_info_item::svid
 * Sybsystem Vendor ID
 * @var efuse_info_offset::smid
 * Sybsystem Device ID
 */
struct efuse_info_item {
	u32 mac_addr;
	u32 pid;
	u32 did;
	u32 vid;
	u32 svid;
	u32 smid;
};

/**
 * @struct efuse_info
 * @brief efuse_info
 *
 * @var efuse_info::offset
 * Efuse information offset
 * @var efuse_info::def_val
 * Efuse information default value
 * @var efuse_info::len
 * Efuse information length
 */
struct efuse_info {
	struct efuse_info_item *offset;
	struct efuse_info_item *def_val;
	struct efuse_info_item *len;
};

/**
 * @enum mac_checksum_offset
 *
 * @brief mac_checksum_offset
 *
 * @var mac_checksum_offset::chksum_offset_1
 * Please Place Description here.
 * @var mac_checksum_offset::chksum_offset_2
 * Please Place Description here.
 */
enum mac_checksum_offset {
	chksum_offset_1 = 0x1AC,
	chksum_offset_2 = 0x1AD,
};

/**
 * @struct mac_bank_efuse_info
 * @brief mac_bank_efuse_info
 *
 * @var mac_bank_efuse_info::phy_map
 * Please Place Description here.
 * @var mac_bank_efuse_info::log_map
 * Please Place Description here.
 * @var mac_bank_efuse_info::phy_map_valid
 * Please Place Description here.
 * @var mac_bank_efuse_info::log_map_valid
 * Please Place Description here.
 * @var mac_bank_efuse_info::efuse_end
 * Please Place Description here.
 * @var mac_bank_efuse_info::phy_map_size
 * Please Place Description here.
 * @var mac_bank_efuse_info::log_map_size
 * Please Place Description here.
 */
struct mac_bank_efuse_info {
	/* efuse_param */
	u8 **phy_map;
	u8 **log_map;
	u8 *phy_map_valid;
	u8 *log_map_valid;
	u32 *efuse_end;
	/* hw_info */
	u32 *phy_map_size;//whole physical size
	u32 *log_map_size;
	u32 *efuse_start;
};

/**
 * @enum mac_defeature_offset
 *
 * @brief mac_defeature_offset
 *
 * @var mac_defeature_offset::rx_spatial_stream
 * Please Place Description here.
 * @var mac_defeature_offset::rx_spatial_stream_sh
 * Please Place Description here.
 * @var mac_defeature_offset::rx_spatial_stream_msk
 * Please Place Description here.
 * @var mac_defeature_offset::bandwidth
 * Please Place Description here.
 * @var mac_defeature_offset::bandwidth_sh
 * Please Place Description here.
 * @var mac_defeature_offset::bandwidth_msk
 * Please Place Description here.
 * @var mac_defeature_offset::tx_spatial_stream
 * Please Place Description here.
 * @var mac_defeature_offset::tx_spatial_stream_sh
 * Please Place Description here.
 * @var mac_defeature_offset::tx_spatial_stream_msk
 * Please Place Description here.
 * @var mac_defeature_offset::protocol_80211
 * Please Place Description here.
 * @var mac_defeature_offset::protocol_80211_sh
 * Please Place Description here.
 * @var mac_defeature_offset::protocol_80211_msk
 * Please Place Description here.
 * @var mac_defeature_offset::NIC_router
 * Please Place Description here.
 * @var mac_defeature_offset::NIC_router_sh
 * Please Place Description here.
 * @var mac_defeature_offset::NIC_router_msk
 * Please Place Description here.
 */
enum mac_defeature_offset {
	rx_spatial_stream = 0xB,
	rx_spatial_stream_sh = 0x4,
	rx_spatial_stream_msk = 0x7,
	bandwidth = 0xD,
	bandwidth_sh = 0x0,
	bandwidth_msk = 0x7,
	tx_spatial_stream = 0xD,
	tx_spatial_stream_sh = 0x4,
	tx_spatial_stream_msk = 0x7,
	protocol_80211 = 0x11,
	protocol_80211_sh = 0x2,
	protocol_80211_msk = 0x3,
	NIC_router = 0x11,
	NIC_router_sh = 0x6,
	NIC_router_msk = 0x3,
};

/**
 * @enum mac_cntlr_mode_sel
 *
 * @brief mac_cntlr_mode_sel
 *
 * @var mac_cntlr_mode_sel::MODE_READ
 * Please Place Description here.
 * @var mac_cntlr_mode_sel::MODE_AUTOLOAD_EN
 * Please Place Description here.
 * @var mac_cntlr_mode_sel::MODE_WRITE
 * Please Place Description here.
 * @var mac_cntlr_mode_sel::MODE_CMP
 * Please Place Description here.
 */
enum mac_cntlr_mode_sel {
	MODE_READ,
	MODE_AUTOLOAD_EN,
	MODE_WRITE,
	MODE_CMP,
};

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_dump_efuse_map
 *
 * @param *adapter
 * @param cfg
 * @param *efuse_map
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_dump_efuse_map(struct mac_ax_adapter *adapter,
		       u8 *efuse_map);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_write_efuse
 *
 * @param *adapter
 * @param addr
 * @param val
 * @param bank
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */

/**
 * @brief mac_read_efuse
 *
 * @param *adapter
 * @param addr
 * @param size
 * @param *val
 * @param bank
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_read_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size,
		   u8 *val);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_get_efuse_avl_size
 *
 * @param *adapter
 * @param *size
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_get_efuse_avl_size(struct mac_ax_adapter *adapter, u32 *size);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_dump_log_efuse
 *
 * @param *adapter
 * @param parser_cfg
 * @param cfg
 * @param *efuse_map
 * @param is_limit
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_dump_log_efuse(struct mac_ax_adapter *adapter,
		       u8 *efuse_map);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */

/**
 * @brief mac_read_log_efuse
 *
 * @param *adapter
 * @param addr
 * @param size
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_read_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size,
		       u8 *val);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_write_log_efuse
 *
 * @param *adapter
 * @param addr
 * @param val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief mac_check_efuse_autoload
 *
 * @param *adapter
 * @param *autoload_status
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_check_efuse_autoload(struct mac_ax_adapter *adapter,
			     u8 *autoload_status);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief _patch_otp_power_issue
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 _patch_otp_power_issue(struct mac_ax_adapter *adapter);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */

/**
 * @brief mac_set_efuse_ctrl
 *
 * @param *adapter
 * @param is_secure
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_set_efuse_ctrl(struct mac_ax_adapter *adapter, bool is_secure);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */


/**
 * @brief efuse_tbl_init
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_tbl_init(struct mac_ax_adapter *adapter);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */

/**
 * @brief efuse_tbl_exit
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 efuse_tbl_exit(struct mac_ax_adapter *adapter);
/**
 * @}
 */

/**
 * @addtogroup Efuse
 * @{
 */

u32 mac_get_efuse_info(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length,
		       u8 *autoload_status);

#endif
