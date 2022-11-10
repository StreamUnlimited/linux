/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation. All rights reserved.
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

#include "halmac_fw_87xx.h"
#include "halmac_87xx_cfg.h"
#include "halmac_common_87xx.h"
#include "halmac_init_87xx.h"

#if HALMAC_87XX_SUPPORT

#define DLFW_RESTORE_REG_NUM		6
#define ILLEGAL_KEY_GROUP		0xFAAAAA00

/* Max dlfw size can not over 31K, due to SDIO HW limitation */
#define DLFW_PKT_SIZE_LIMIT		31744

#define ID_INFORM_DLEMEM_RDY		0x80
#define ID_INFORM_ENETR_CPU_SLEEP	0x20
#define ID_CHECK_DLEMEM_RDY		0x80
#define ID_CHECK_ENETR_CPU_SLEEP	0x05

#define FW_STATUS_CHK_FATAL	(BIT(1) | BIT(20))
#define FW_STATUS_CHK_ERR	(BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8) | \
				 BIT(9) | BIT(12) | BIT(14) | BIT(15) | \
				 BIT(16) | BIT(17) | BIT(18) | BIT(19) | \
				 BIT(21) | BIT(22) | BIT(25))
#define FW_STATUS_CHK_WARN	~(FW_STATUS_CHK_FATAL | FW_STATUS_CHK_ERR)
#define REG_DBG_29	0x11FC
#define REG_DL_HANDSHK_DRV2FW	(REG_DBG_29)
#define REG_DL_HANDSHK_FW2DRV	(REG_DBG_29 + 1)
//#define REG_SECURE_CTRL	0x1C0		// for test
#define REG_SECURE_CTRL	0x14
#define FW_SECURE_DATA_LEN	736

struct halmac_backup_info {
	u32 mac_register;
	u32 value;
	u8 length;
};

static void
fw_fatal_status_debug_87xx(struct halmac_adapter *adapter);

static void
wlan_cpu_en_87xx(struct halmac_adapter *adapter, u8 enable);

/*
static enum halmac_ret_status
proc_send_general_info_87xx(struct halmac_adapter *adapter,
			    struct halmac_general_info *info);

static enum halmac_ret_status
proc_send_phydm_info_87xx(struct halmac_adapter *adapter,
			  struct halmac_general_info *info);
*/

static void
wlan_cpu_en_87xx(struct halmac_adapter *adapter, u8 enable)
{
	u32 value32;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;

	if (enable == 1) {
		/* cpu enable or disable */
		value32 = HALMAC_REG_R32(REG_WL_FUNC_EN);
		value32 |= BIT_FEN_WLMCU;
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);

	} else {
		/* cpu enable or disable */
		value32 = HALMAC_REG_R8(REG_WL_FUNC_EN);
		value32 &= ~BIT_FEN_WLMCU;
		HALMAC_REG_W32(REG_WL_FUNC_EN, value32);
	}
}

/**
 * reset_wifi_fw_87xx() - reset wifi fw
 * @adapter : the adapter of halmac
 * Author : LIN YONG-CHING
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
reset_wifi_fw_87xx(struct halmac_adapter *adapter) {
	enum halmac_ret_status status;
	u32 lte_coex_backup = 0;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	status = ltecoex_reg_read_87xx(adapter, 0x38, &lte_coex_backup);
	if (status != HALMAC_RET_SUCCESS)
	{
		return status;
	}

	wlan_cpu_en_87xx(adapter, 0);
	init_ofld_feature_state_machine_87xx(adapter);
	wlan_cpu_en_87xx(adapter, 1);

	status = ltecoex_reg_write_87xx(adapter, 0x38, lte_coex_backup);
	if (status != HALMAC_RET_SUCCESS)
	{
		return status;
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return HALMAC_RET_SUCCESS;
}

/**
 * get_fw_version_87xx() - get FW version
 * @adapter : the adapter of halmac
 * @ver : fw version info
 * Author : Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
get_fw_version_87xx(struct halmac_adapter *adapter,
		    struct halmac_fw_version *ver) {
	struct halmac_fw_version *info = &adapter->fw_ver;

	if (!ver)
	{
		return HALMAC_RET_NULL_POINTER;
	}

	if (adapter->halmac_state.dlfw_state == HALMAC_DLFW_NONE)
	{
		return HALMAC_RET_NO_DLFW;
	}

	ver->version = info->version;
	ver->sub_version = info->sub_version;
	ver->sub_index = info->sub_index;
	ver->h2c_version = info->h2c_version;
	ver->build_time.month = info->build_time.month;
	ver->build_time.date = info->build_time.date;
	ver->build_time.hour = info->build_time.hour;
	ver->build_time.min = info->build_time.min;
	ver->build_time.year = info->build_time.year;

	return HALMAC_RET_SUCCESS;
}

/**
 * check_fw_status_87xx() -check fw status
 * @adapter : the adapter of halmac
 * @status : fw status
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
check_fw_status_87xx(struct halmac_adapter *adapter, u8 *fw_status) {
	u32 cnt;
	u32 fw_dbg6;
	u32 fw_pc;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	*fw_status = 1;

	fw_dbg6 = HALMAC_REG_R32(REG_FW_DBG6_V1);

	if (fw_dbg6 != 0)
	{
		PLTFM_MSG_ERR("[ERR]REG_FW_DBG6_V1 !=0\n");
		if ((fw_dbg6 & FW_STATUS_CHK_WARN) != 0) {
			PLTFM_MSG_WARN("[WARN]fw status(warn):%X\n", fw_dbg6);
		}

		if ((fw_dbg6 & FW_STATUS_CHK_ERR) != 0) {
			PLTFM_MSG_ERR("[ERR]fw status(err):%X\n", fw_dbg6);
		}

		if ((fw_dbg6 & FW_STATUS_CHK_FATAL) != 0) {
			PLTFM_MSG_ERR("[ERR]fw status(fatal):%X\n", fw_dbg6);
			fw_fatal_status_debug_87xx(adapter);
			*fw_status = 0;
			return status;
		}
	}

	fw_pc = HALMAC_REG_R32(REG_FW_DBG7_V1);
	cnt = 10;
	while (HALMAC_REG_R32(REG_FW_DBG7_V1) == fw_pc)
	{
		cnt--;
		if (cnt == 0) {
			break;
		}
	}

	if (cnt == 0)
	{
		cnt = 200;
		while (HALMAC_REG_R32(REG_FW_DBG7_V1) == fw_pc) {
			cnt--;
			if (cnt == 0) {
				PLTFM_MSG_ERR("[ERR]fw pc\n");
				*fw_status = 0;
				return status;
			}
			PLTFM_DELAY_US(50);
		}
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return status;
}

static void
fw_fatal_status_debug_87xx(struct halmac_adapter *adapter)
{
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;

	PLTFM_MSG_ERR("[ERR]0x%X = %X\n",
		      REG_FW_DBG6_V1, HALMAC_REG_R32(REG_FW_DBG6_V1));

	PLTFM_MSG_ERR("[ERR]0x%X = %X\n",
		      REG_ARFR_L5, HALMAC_REG_R32(REG_ARFR_L5));

	PLTFM_MSG_ERR("[ERR]0x%X = %X\n",
		      REG_ARFR_H5, HALMAC_REG_R32(REG_ARFR_H5));

	PLTFM_MSG_ERR("[ERR]0x%X = %X\n",
		      REG_MCUTST_I, HALMAC_REG_R32(REG_MCUTST_I));
}

enum halmac_ret_status
dump_fw_dmem_87xx(struct halmac_adapter *adapter, u8 *dmem, u32 *size) {
	return HALMAC_RET_SUCCESS;
}

/**
 * cfg_max_dl_size_87xx() - config max download FW size
 * @adapter : the adapter of halmac
 * @size : max download fw size
 *
 * Halmac uses this setting to set max packet size for
 * download FW.
 * If user has not called this API, halmac use default
 * setting for download FW
 * Note1 : size need multiple of 2
 * Note2 : max size is 31K
 *
 * Author : Ivan Lin/KaiYuan Chang
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
cfg_max_dl_size_87xx(struct halmac_adapter *adapter, u32 size) {
	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	if (size > DLFW_PKT_SIZE_LIMIT)
	{
		PLTFM_MSG_ERR("[ERR]size > max dl size!\n");
		return HALMAC_RET_CFG_DLFW_SIZE_FAIL;
	}

	if ((size & (2 - 1)) != 0)
	{
		PLTFM_MSG_ERR("[ERR]not multiple of 2!\n");
		return HALMAC_RET_CFG_DLFW_SIZE_FAIL;
	}

	adapter->dlfw_pkt_size = size;

	PLTFM_MSG_TRACE("[TRACE]Cfg max size:%X\n", size);
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return HALMAC_RET_SUCCESS;
}

/**
 * enter_cpu_sleep_mode_87xx() -wlan cpu enter sleep mode
 * @adapter : the adapter of halmac
 * Author : Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
enter_cpu_sleep_mode_87xx(struct halmac_adapter *adapter) {
	u32 cnt;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	enum halmac_wlcpu_mode *cur_mode = &adapter->halmac_state.wlcpu_mode;

	if (halmac_fw_validate(adapter) != HALMAC_RET_SUCCESS)
	{
		return HALMAC_RET_NO_DLFW;
	}

	if (*cur_mode != HALMAC_WLCPU_ACTIVE)
	{
		return HALMAC_RET_ERROR_STATE;
	}

	cnt = 100;
	while (HALMAC_REG_R8(REG_HMETFR + 3) != 0)
	{
		cnt--;
		if (cnt == 0) {
			PLTFM_MSG_ERR("[ERR]0x1CF != 0\n");
			return HALMAC_RET_STATE_INCORRECT;
		}
		PLTFM_DELAY_US(50);
	}

	HALMAC_REG_W8(REG_HMETFR + 3, ID_INFORM_ENETR_CPU_SLEEP);

	*cur_mode = HALMAC_WLCPU_ENTER_SLEEP;

	return HALMAC_RET_SUCCESS;
}

/**
 * get_cpu_mode_87xx() -get wlcpu mode
 * @adapter : the adapter of halmac
 * @mode : cpu mode
 * Author : Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
get_cpu_mode_87xx(struct halmac_adapter *adapter,
		  enum halmac_wlcpu_mode *mode) {
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	enum halmac_wlcpu_mode *cur_mode = &adapter->halmac_state.wlcpu_mode;

	if (halmac_fw_validate(adapter) != HALMAC_RET_SUCCESS)
	{
		return HALMAC_RET_NO_DLFW;
	}

	if (*cur_mode == HALMAC_WLCPU_ACTIVE)
	{
		*mode = HALMAC_WLCPU_ACTIVE;
		return HALMAC_RET_SUCCESS;
	}

	if (*cur_mode == HALMAC_WLCPU_SLEEP)
	{
		*mode = HALMAC_WLCPU_SLEEP;
		return HALMAC_RET_SUCCESS;
	}

	if (HALMAC_REG_R8(REG_BLK_TST) == ID_CHECK_ENETR_CPU_SLEEP)
	{
		*mode = HALMAC_WLCPU_SLEEP;
		*cur_mode = HALMAC_WLCPU_SLEEP;
		HALMAC_REG_W8(REG_BLK_TST, 0);
	} else
	{
		*mode = HALMAC_WLCPU_ENTER_SLEEP;
	}

	return HALMAC_RET_SUCCESS;
}

/**
 * send_general_info_87xx() -send general information to FW
 * @adapter : the adapter of halmac
 * @info : general information
 * Author : KaiYuan Chang/Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
send_general_info_87xx(struct halmac_adapter *adapter,
		       struct halmac_general_info *info) {
	u8 h2cq_ele[4] = {0};
	u32 h2cq_addr;
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;
	u8 cnt;
#if 0
	if (halmac_fw_validate(adapter) != HALMAC_RET_SUCCESS)
	{
		return HALMAC_RET_NO_DLFW;
	}

	if (adapter->fw_ver.h2c_version < 4)
	{
		return HALMAC_RET_FW_NO_SUPPORT;
	}

	if (adapter->fw_ver.h2c_version < 14)
	{
		PLTFM_MSG_WARN("[WARN]the H2C ver. does not match halmac\n");
	}

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	if (adapter->halmac_state.dlfw_state == HALMAC_DLFW_NONE)
	{
		PLTFM_MSG_ERR("[ERR]no dl fw!!\n");
		return HALMAC_RET_NO_DLFW;
	}

	status = proc_send_general_info_87xx(adapter, info);
	if (status != HALMAC_RET_SUCCESS)
	{
		PLTFM_MSG_ERR("[ERR]send gen info!!\n");
		return status;
	}

	status = proc_send_phydm_info_87xx(adapter, info);
	if (status != HALMAC_RET_SUCCESS)
	{
		PLTFM_MSG_ERR("[ERR]send phydm info\n");
		return status;
	}

	h2cq_addr = adapter->txff_alloc.rsvd_h2cq_addr;
	h2cq_addr <<= TX_PAGE_SIZE_SHIFT_87XX;

	cnt = 100;
	do
	{
		status = dump_fifo_87xx(adapter, HAL_FIFO_SEL_TX,
					h2cq_addr, 4, h2cq_ele);
		if (status != HALMAC_RET_SUCCESS) {
			PLTFM_MSG_ERR("[ERR]dump h2cq!!\n");
			return status;
		}

		if ((h2cq_ele[0] & 0x7F) == 0x01 && h2cq_ele[1] == 0xFF) {
			break;
		}

		cnt--;
		if (cnt == 0) {
			PLTFM_MSG_ERR("[ERR]h2cq compare!!\n");
			return HALMAC_RET_SEND_H2C_FAIL;
		}
		PLTFM_DELAY_US(5);
	} while (1);

	if (adapter->halmac_state.dlfw_state == HALMAC_DLFW_DONE)
	{
		adapter->halmac_state.dlfw_state = HALMAC_GEN_INFO_SENT;
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

#endif
	return HALMAC_RET_SUCCESS;
}

//fw require cancel
/*
static enum halmac_ret_status
proc_send_general_info_87xx(struct halmac_adapter *adapter,
			    struct halmac_general_info *info)
{
	u8 h2c_buf[H2C_PKT_SIZE_87XX] = { 0 };
	u16 seq_num = 0;
	struct halmac_h2c_header_info hdr_info;
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;

	PLTFM_MSG_TRACE("[TRACE]%s\n", __func__);

	GENERAL_INFO_SET_FW_TX_BOUNDARY(h2c_buf,
					adapter->txff_alloc.rsvd_fw_txbuf_addr -
					adapter->txff_alloc.rsvd_boundary);

	hdr_info.sub_cmd_id = SUB_CMD_ID_GENERAL_INFO;
	hdr_info.content_size = 4;
	hdr_info.ack = 0;
	set_h2c_pkt_hdr_87xx(adapter, h2c_buf, &hdr_info, &seq_num);

	status = send_h2c_pkt_87xx(adapter, h2c_buf);

	if (status != HALMAC_RET_SUCCESS)
		PLTFM_MSG_ERR("[ERR]send h2c!!\n");

	return status;
}

static enum halmac_ret_status
proc_send_phydm_info_87xx(struct halmac_adapter *adapter,
			  struct halmac_general_info *info)
{
	u8 h2c_buf[H2C_PKT_SIZE_87XX] = { 0 };
	u16 seq_num = 0;
	struct halmac_h2c_header_info hdr_info;
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;

	PLTFM_MSG_TRACE("[TRACE]%s\n", __func__);

	PHYDM_INFO_SET_REF_TYPE(h2c_buf, info->rfe_type);
	PHYDM_INFO_SET_RF_TYPE(h2c_buf, info->rf_type);
	PHYDM_INFO_SET_CUT_VER(h2c_buf, adapter->chip_ver);
	PHYDM_INFO_SET_RX_ANT_STATUS(h2c_buf, info->rx_ant_status);
	PHYDM_INFO_SET_TX_ANT_STATUS(h2c_buf, info->tx_ant_status);
	PHYDM_INFO_SET_EXT_PA(h2c_buf, info->ext_pa);
	PHYDM_INFO_SET_PACKAGE_TYPE(h2c_buf, info->package_type);
	PHYDM_INFO_SET_MP_MODE(h2c_buf, info->mp_mode);

	hdr_info.sub_cmd_id = SUB_CMD_ID_PHYDM_INFO;
	hdr_info.content_size = 8;
	hdr_info.ack = 0;
	set_h2c_pkt_hdr_87xx(adapter, h2c_buf, &hdr_info, &seq_num);

	status = send_h2c_pkt_87xx(adapter, h2c_buf);

	if (status != HALMAC_RET_SUCCESS)
		PLTFM_MSG_ERR("[ERR]send h2c!!\n");

	return status;
}
*/
/**
 * drv_fwctrl_87xx() - send drv-defined h2c pkt
 * @adapter : the adapter of halmac
 * @payload : no include offload pkt h2c header
 * @size : no include offload pkt h2c header
 * Author : Ivan Lin
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
drv_fwctrl_87xx(struct halmac_adapter *adapter, u8 *payload, u32 size, u8 ack) {
	u8 h2c_buf[H2C_PKT_SIZE_87XX] = { 0 };
	u16 seq_num = 0;
	struct halmac_h2c_header_info hdr_info;
	enum halmac_ret_status status = HALMAC_RET_SUCCESS;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	if (!payload)
	{
		return HALMAC_RET_DATA_BUF_NULL;
	}

	if (size > H2C_PKT_SIZE_87XX - H2C_PKT_HDR_SIZE_87XX)
	{
		return HALMAC_RET_DATA_SIZE_INCORRECT;
	}

	PLTFM_MEMCPY(h2c_buf + H2C_PKT_HDR_SIZE_87XX, payload, size);

	hdr_info.sub_cmd_id = SUB_CMD_ID_FW_FWCTRL;
	hdr_info.content_size = (u16)size;
	hdr_info.ack = ack;
	set_h2c_pkt_hdr_87xx(adapter, h2c_buf, &hdr_info, &seq_num);

	status = send_h2c_pkt_87xx(adapter, h2c_buf);

	if (status != HALMAC_RET_SUCCESS)
	{
		PLTFM_MSG_ERR("[ERR]send h2c!!\n");
	}

	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);

	return status;
}

#endif /* HALMAC_87XX_SUPPORT */
