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
#include "efuse.h"
#include "mac_priv.h"

static struct mac_efuse_tbl efuse_tbl;
struct mac_bank_efuse_info bank_efuse_info;
u16 read_efuse_cnt = EFUSE_WAIT_CNT;
bool OTP_test;
enum rtw_dv_sel dv_sel = DDV;

static struct efuse_info_item offset_axi = {
	0x11A, /* mac_addr */
	0, /* pid */
	0, /* did */
	0, /* vid */
	0, /* svid */
	0, /* smid */
};

static struct efuse_info_item def_val_axi = {
	0x0, /* mac_addr */
	0, /* pid */
	0, /* did */
	0, /* vid */
	0, /* svid */
	0, /* smid */
};

static struct efuse_info_item len_axi = {
	6, /* mac_addr */
	0, /* pid */
	0, /* did */
	0, /* vid */
	0, /* svid */
	0, /* smid */
};

static struct efuse_info efuse_info_axi = {
	&offset_axi, /* offset */
	&def_val_axi, /* def_val */
	&len_axi, /* len */
};


static u32 efuse_map_init(struct mac_ax_adapter *adapter,
			  enum efuse_map_sel map_sel);
static u32 efuse_proc_ck(struct mac_ax_adapter *adapter);
static u32 cnv_efuse_state(struct mac_ax_adapter *adapter, u8 dest_state);
static u32 proc_dump_efuse(struct mac_ax_adapter *adapter);
static u32 read_hw_efuse(struct mac_ax_adapter *adapter, u32 offset, u32 size,
			 u8 *map);
static u32 write_hw_efuse(struct mac_ax_adapter *adapter, u32 offset, u8 value);
static u32 eeprom_parser(struct mac_ax_adapter *adapter, u8 *phy_map,
			 u8 *log_map);
static u32 dump_efuse_drv(struct mac_ax_adapter *adapter);
static u32 proc_write_log_efuse(struct mac_ax_adapter *adapter, u32 offset,
				u8 value);
static u32 read_efuse(struct mac_ax_adapter *adapter, u32 offset, u32 size,
		      u8 *map);
static u32 query_status_map(struct mac_ax_adapter *adapter,
			    enum mac_ax_efuse_feature_id feature_id, u8 *map);
static void switch_dv(struct mac_ax_adapter *adapter, enum rtw_dv_sel);
static u32 proc_dump_hidden(struct mac_ax_adapter *adapter);

#if 1
/*dump the whole physical otp*/
u32 mac_dump_efuse_map(struct mac_ax_adapter *adapter,
		       u8 *efuse_map)
{
	u32 ret, stat;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_PHY);
	if (ret != 0) {
		return ret;
	}

	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_PHY);
	if (ret != 0) {
		return ret;
	}

	ret = proc_dump_efuse(adapter);
	if (ret != 0) {
		PLTFM_MSG_ERR("[ERR]dump efuse!!\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return ret;
	}

	query_status_map(adapter, MAC_AX_DUMP_PHYSICAL_EFUSE, efuse_map);

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (ret != 0) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_write_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val)
{
	u32 ret, stat;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32 efuse_size = adapter->hw_info->efuse_size;

	if (addr > efuse_size) {
		PLTFM_MSG_ERR("[ERR]Offset is too large\n");
		return MACEFUSESIZE;
	}

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_PHY);
	if (ret != 0) {
		return ret;
	}

	ret = write_hw_efuse(adapter, addr, val);
	if (ret != 0) {
		PLTFM_MSG_ERR("[ERR]write physical efuse\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

u32 mac_read_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size,
		   u8 *val)
{
	u32 ret, stat, efuse_size = 0;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	efuse_size = adapter->hw_info->efuse_size;

	if (addr >= efuse_size || addr + size > efuse_size) {
		PLTFM_MSG_ERR("[ERR] Wrong efuse index\n");
		return MACEFUSESIZE;
	}

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_PHY);
	if (ret != 0) {
		return ret;
	}

	ret = read_hw_efuse(adapter, addr, size, val);
	if (ret != 0) {
		PLTFM_MSG_ERR("[ERR]read hw efuse\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (ret != 0) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_get_efuse_avl_size(struct mac_ax_adapter *adapter, u32 *size)
{
	u32 ret;
	u8 *map;
	u32 efuse_size = adapter->hw_info->log_efuse_size;
	struct mac_ax_efuse_param *efuse_param = &adapter->efuse_param;

	map = (u8 *)PLTFM_MALLOC(efuse_size);
	if (!map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		return MACBUFALLOC;
	}

	ret = mac_dump_log_efuse(adapter, map);

	PLTFM_FREE(map, efuse_size);

	if (ret != 0) {
		return ret;
	}

	*size = adapter->hw_info->efuse_size - efuse_param->efuse_end;

	return MACSUCCESS;
}

/*dump the whole logical otp*/
u32 mac_dump_log_efuse(struct mac_ax_adapter *adapter,
		       u8 *efuse_map)
{
	u8 *map = NULL;
	u32 ret, stat;
	u32 efuse_size;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_LOG);
	if (ret != 0) {
		return ret;
	}

	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_PHY);
	if (ret != 0) {
		return ret;
	}
	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_LOG);
	if (ret != 0) {
		return ret;
	}

	efuse_size = *bank_efuse_info.log_map_size;

	if (*bank_efuse_info.log_map_valid == 0) {
		ret = proc_dump_efuse(adapter);
		if (ret != 0) {
			PLTFM_MSG_ERR("[ERR]dump efuse\n");
			stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
			if (stat != 0) {
				return stat;
			}
			return ret;
		}

		if (*bank_efuse_info.phy_map_valid == 1) {
			map = (u8 *)PLTFM_MALLOC(efuse_size);
			if (!map) {
				PLTFM_MSG_ERR("[ERR]malloc map\n");
				stat = cnv_efuse_state(adapter,
						       MAC_AX_EFUSE_IDLE);
				if (stat != 0) {
					return stat;
				}
				return MACBUFALLOC;
			}

			ret = eeprom_parser(adapter, *bank_efuse_info.phy_map,
					    map);
			if (ret != 0) {
				PLTFM_FREE(map, efuse_size);
				stat = cnv_efuse_state(adapter,
						       MAC_AX_EFUSE_IDLE);
				if (stat != 0) {
					return stat;
				}
				return ret;
			}
			PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
			PLTFM_MEMCPY(*bank_efuse_info.log_map, map,
				     efuse_size);
			*bank_efuse_info.log_map_valid = 1;
			PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
			PLTFM_FREE(map, efuse_size);
		}
	}

	query_status_map(adapter, MAC_AX_DUMP_LOGICAL_EFUSE,
			 efuse_map);

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (ret != 0) {
		return ret;
	}

	return MACSUCCESS;
}

static u32 read_log_efuse_map(struct mac_ax_adapter *adapter, u8 *map,
			      u32 log_efuse_size)
{
	u8 *local_map = NULL;
	u32 efuse_size = *bank_efuse_info.phy_map_size;
	u32 ret;

	if (*bank_efuse_info.log_map_valid == 0) {
		if (*bank_efuse_info.phy_map_valid == 0) {
			local_map = (u8 *)PLTFM_MALLOC(efuse_size);
			if (!local_map) {
				PLTFM_MSG_ERR("[ERR]malloc map\n");
				return MACBUFALLOC;
			}

			ret = read_efuse(adapter, 0, efuse_size, local_map);
			if (ret != 0) {
				PLTFM_MSG_ERR("[ERR]read efuse\n");
				goto error;
			}

			PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
			PLTFM_MEMCPY(*bank_efuse_info.phy_map, local_map,
				     efuse_size);
			*bank_efuse_info.phy_map_valid = 1;
			PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);

			PLTFM_FREE(local_map, efuse_size);
		}

		ret = eeprom_parser(adapter, *bank_efuse_info.phy_map, map);
		if (ret != 0) {
			return ret;
		}

		PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
		PLTFM_MEMCPY(*bank_efuse_info.log_map, map, log_efuse_size);
		*bank_efuse_info.log_map_valid = 1;
		PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
	} else {
		PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
		PLTFM_MEMCPY(map, *bank_efuse_info.log_map, log_efuse_size);
		PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
	}

	return MACSUCCESS;
error:
	PLTFM_FREE(local_map, efuse_size);
	return ret;
}


u32 mac_read_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size,
		       u8 *val)
{
	u8 *map = NULL;
	u32 ret = 0, stat;
	u32 efuse_size;

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_LOG);
	if (ret != 0) {
		return ret;
	}

	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_PHY);
	if (ret) {
		return ret;
	}
	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_LOG);
	if (ret) {
		return ret;
	}

	efuse_size = *bank_efuse_info.log_map_size;

	if (addr >= efuse_size || addr + size > efuse_size) {
		PLTFM_MSG_ERR("[ERR] Wrong efuse index\n");
		return MACEFUSESIZE;
	}

	map = (u8 *)PLTFM_MALLOC(efuse_size);
	if (!map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return MACBUFALLOC;
	}

	ret = read_log_efuse_map(adapter, map, efuse_size);
	if (ret != 0) {
		PLTFM_FREE(map, efuse_size);
		PLTFM_MSG_ERR("[ERR]read logical efuse\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return ret;
	}

	PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
	PLTFM_MEMCPY(val, map + addr, size);
	PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
	PLTFM_FREE(map, efuse_size);

	stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (stat != 0) {
		return stat;
	}

	return MACSUCCESS;
}

u32 mac_write_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val)
{
	u32 ret, stat;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	ret = efuse_proc_ck(adapter);
	if (ret != 0) {
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_LOG);
	if (ret != 0) {
		return ret;
	}

	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_PHY);
	if (ret) {
		return ret;
	}

	ret = efuse_map_init(adapter, EFUSE_MAP_SEL_LOG);
	if (ret) {
		return ret;
	}

	if (addr >= *bank_efuse_info.log_map_size) {
		PLTFM_MSG_ERR("[ERR]addr is too large\n");
		return MACEFUSESIZE;
	}

	ret = proc_write_log_efuse(adapter, addr, val);
	if (ret != 0) {
		PLTFM_MSG_ERR("[ERR]write logical efuse\n");
		stat = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
		if (stat != 0) {
			return stat;
		}
		return ret;
	}

	ret = cnv_efuse_state(adapter, MAC_AX_EFUSE_IDLE);
	if (ret != 0) {
		return ret;
	}

	PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
	*bank_efuse_info.phy_map_valid = 0;
	*bank_efuse_info.log_map_valid = 0;
	PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);

	return MACSUCCESS;
}
#endif

u32 mac_check_efuse_autoload(struct mac_ax_adapter *adapter,
			     u8 *autoload_status)
{
#define AUTOLOAD_SUS 1
#define AUTOLOAD_FAIL 0

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (SYS_REG_R16(SYSTEM_CTRL_BASE_LP, REG_SYSTEM_CFG0) & PTRP_OTPBYP) {
		*autoload_status = AUTOLOAD_FAIL;
	} else {
		*autoload_status = AUTOLOAD_SUS;
	}

	return MACSUCCESS;

#undef AUTOLOAD_SUS
#undef AUTOLOAD_FAIL
}

#if 1
u32 mac_set_efuse_ctrl(struct mac_ax_adapter *adapter, bool is_secure)
{

	return MACSUCCESS;
}


u32 efuse_tbl_init(struct mac_ax_adapter *adapter)
{
	PLTFM_MUTEX_INIT(&efuse_tbl.lock);

	return MACSUCCESS;
}

u32 efuse_tbl_exit(struct mac_ax_adapter *adapter)
{
	PLTFM_MUTEX_DEINIT(&efuse_tbl.lock);

	return MACSUCCESS;
}

static u32 efuse_map_init(struct mac_ax_adapter *adapter,
			  enum efuse_map_sel map_sel)
{
	u32 size;
	struct mac_ax_efuse_param *efuse_param = &adapter->efuse_param;
	struct mac_ax_hw_info *hw_info = adapter->hw_info;

	switch (map_sel) {
	case EFUSE_MAP_SEL_PHY:
		size = adapter->hw_info->efuse_size;
		if (!efuse_param->efuse_map) {
			efuse_param->efuse_map = (u8 *)PLTFM_MALLOC(size);
			if (!efuse_param->efuse_map) {
				PLTFM_MSG_ERR("[ERR]malloc map\n");
				return MACBUFALLOC;
			}
		}
		break;
	case EFUSE_MAP_SEL_LOG:
		size = adapter->hw_info->log_efuse_size;
		if (!efuse_param->log_efuse_map) {
			efuse_param->log_efuse_map = (u8 *)PLTFM_MALLOC(size);
			if (!efuse_param->log_efuse_map) {
				PLTFM_MSG_ERR("[ERR]malloc map\n");
				return MACBUFALLOC;
			}
		}
		break;
	default:
		break;
	}

	bank_efuse_info.phy_map = &efuse_param->efuse_map;
	bank_efuse_info.log_map = &efuse_param->log_efuse_map;
	bank_efuse_info.phy_map_valid = &efuse_param->efuse_map_valid;
	bank_efuse_info.log_map_valid =
		&efuse_param->log_efuse_map_valid;
	bank_efuse_info.efuse_end = &efuse_param->efuse_end;
	bank_efuse_info.phy_map_size = &hw_info->efuse_size;
	bank_efuse_info.log_map_size = &hw_info->log_efuse_size;


	return MACSUCCESS;
}


static u32 efuse_proc_ck(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.efuse != MAC_AX_EFUSE_IDLE) {
		PLTFM_MSG_WARN("[WARN]Proc not idle(efuse)\n");
		return MACPROCBUSY;
	}

	if (adapter->sm.pwr != MAC_AX_PWR_ON) {
		PLTFM_MSG_ERR("[ERR]Access efuse in suspend\n");
	}

	return MACSUCCESS;
}
//remain to modify
static u32 cnv_efuse_state(struct mac_ax_adapter *adapter, u8 dest_state)
{
#define BT_DISN_EN 1
#define BT_DISN_DIS 0
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (adapter->sm.efuse >= MAC_AX_EFUSE_MAX) {
		return MACPROCERR;
	}

	if (adapter->sm.efuse == dest_state) {
		return MACPROCERR;
	}

	if (dest_state != MAC_AX_EFUSE_IDLE) {
		if (adapter->sm.efuse != MAC_AX_EFUSE_IDLE) {
			return MACPROCERR;
		}
	}

	adapter->sm.efuse = dest_state;

	return MACSUCCESS;
}

static u32 proc_dump_efuse(struct mac_ax_adapter *adapter)
{
	u32 ret;

	ret = dump_efuse_drv(adapter);
	if (ret != 0) {
		PLTFM_MSG_ERR("[ERR]dump efsue drv/fw\n");
		return ret;
	}

	return MACSUCCESS;
}

static u32 read_hw_efuse(struct mac_ax_adapter *adapter, u32 offset,
			 u32 size, u8 *map)
{
	u32 tmp32 = 0, i;
	u32 ret = MACSUCCESS;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	for (i = 0; i < size; i++) {
		ret = p_ops->efuse_read8(adapter, offset + i, map + i);
		if (ret != MACSUCCESS) {
			return MACEFUSEREAD;
		}
	}

	return ret;
}

static u32 write_hw_efuse(struct mac_ax_adapter *adapter, u32 offset, u8 value)
{
	//const u8 unlock_code = 0x69;
	u8 value_read = 0;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_efuse_param *efuse_param = &adapter->efuse_param;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
	*bank_efuse_info.phy_map_valid = 0;
	*bank_efuse_info.log_map_valid = 0;
	PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);

	ret = p_ops->efuse_write8(adapter, offset, value);
	if (ret != MACSUCCESS) {
		return MACEFUSEWRITE;
	}

	if (efuse_param->auto_ck_en == 1) {
		ret = read_hw_efuse(adapter, offset, 1, &value_read);
		if (ret != 0) {
			return ret;
		}

		if (value_read != value) {
			PLTFM_MSG_ERR("[ERR]efuse compare\n");
			return MACEFUSEWRITE;
		}
	}

	return MACSUCCESS;
}

static u32 eeprom_parser(struct mac_ax_adapter *adapter, u8 *phy_map,
			 u8 *log_map)
{
	u32 phy_data, offset, data;
	u8 plen;
	u8 hdr = 0;
	u8 hdr2 = 0;
	u32 otp_addr = 0;
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	u32 efuse_idx = 0;
	u32 efuse_size = phy_log_mapping_size;
	bool is_bt = 0;
	u32 log_efuse_size = *bank_efuse_info.log_map_size;

	PLTFM_MEMSET(log_map, 0xFF, log_efuse_size);

	while (otp_addr < efuse_size - 3) {
		phy_data = (*(phy_map + otp_addr)) | (*(phy_map + otp_addr) << 8) |
			   (*(phy_map + otp_addr) << 16) | (*(phy_map + otp_addr) << 24);

		if (phy_data == 0xFFFFFFFF) { /*not write*/
			break;
		}

		otp_addr += 4;

		switch (OTP_GET_LTYP(phy_data)) {
		case OTP_LTYP0:
			/*empty entry shift to next entry*/
			break;
		case OTP_LTYP1:
			if (OTP_GET_LTYP1_BASE(phy_data) == OTP_LBASE_EFUSE) {
				offset = OTP_GET_LTYP1_OFFSET(phy_data);
				data = OTP_GET_LTYP1_DATA(phy_data);
				log_map[offset] = data;
			}

			break;

		case OTP_LTYP2:
			plen = OTP_GET_LTYP2_LEN(phy_data);
			offset = OTP_GET_LTYP2_OFFSET(phy_data);

			if (OTP_GET_LTYP2_BASE(phy_data) == OTP_LBASE_EFUSE) {
				while (plen-- > 0) {
					log_map[offset] = *(phy_map + otp_addr);
					otp_addr++;
				}
			} else {
				otp_addr += plen;
			}

			break;

		default :
			break;

		}
		if ((otp_addr & 0x03) != 0) {
			PLTFM_MSG_ERR("[ERR]alignment error %x \n", otp_addr);
		}
	}

	*bank_efuse_info.efuse_end = otp_addr;

	return MACSUCCESS;
}

static u32 dump_efuse_drv(struct mac_ax_adapter *adapter)
{
	u8 *map = NULL;
	u32 efuse_size;
	u32 ret;

	efuse_size = *bank_efuse_info.phy_map_size;

	if (*bank_efuse_info.phy_map_valid == 0) {
		map = (u8 *)PLTFM_MALLOC(efuse_size);
		if (!map) {
			PLTFM_MSG_ERR("[ERR]malloc map\n");
			return MACBUFALLOC;
		}

		ret = read_hw_efuse(adapter, 0, efuse_size, map);
		if (ret != 0) {
			PLTFM_FREE(map, efuse_size);
			return ret;
		}

		PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
		PLTFM_MEMCPY(*bank_efuse_info.phy_map, map, efuse_size);
		*bank_efuse_info.phy_map_valid = 1;
		PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
		PLTFM_FREE(map, efuse_size);
	}

	return MACSUCCESS;
}

static u32 proc_write_log_efuse(struct mac_ax_adapter *adapter, u32 offset,
				u8 value)
{
	u8 hdr2 = 0;
	u8 *map = NULL;
	u32 log_efuse_size = *bank_efuse_info.log_map_size, ret;
	u32 end;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	if (offset > log_efuse_size) {
		return MACEFUSESIZE;
	}

	map = (u8 *)PLTFM_MALLOC(log_efuse_size);
	if (!map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		return MACBUFALLOC;
	}

	if (*bank_efuse_info.log_map_valid == 0) {
		ret = read_log_efuse_map(adapter, map, log_efuse_size);
		if (ret != 0) {
			PLTFM_MSG_ERR("[ERR]read logical efuse\n");
			PLTFM_FREE(map, log_efuse_size);
			return ret;
		}
	} else {
		PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
		PLTFM_MEMCPY(map, *bank_efuse_info.log_map, log_efuse_size);
		PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);
	}


	if (*(map + offset) != value) {
		end = *bank_efuse_info.efuse_end;

		if (phy_log_mapping_size <= 4 + end) { // write 4 bytes
			PLTFM_FREE(map, log_efuse_size);
			return MACEFUSESIZE;
		}

		ret = p_ops->efuse_write8(adapter, end, 0xFF);//header[7:0]
		if (ret != 0) {
			goto error;
		}

		ret = p_ops->efuse_write8(adapter, end + 1, (OTP_LBASE_EFUSE << 4) |
					  ((offset >> 8) & 0x0F));//header[15:8]
		if (ret != 0) {
			goto error;
		}

		ret = p_ops->efuse_write8(adapter, end + 2, value);//header[31:24]
		if (ret != 0) {
			goto error;
		}

		ret = p_ops->efuse_write8(adapter, end + 3, ((OTP_LTYP1 << 4) | 0x0F));
		if (ret != 0) {
			goto error;
		}

	}

	PLTFM_FREE(map, log_efuse_size);
	return MACSUCCESS;
error:
	PLTFM_FREE(map, log_efuse_size);
	return ret;
}

static u32 read_efuse(struct mac_ax_adapter *adapter, u32 offset, u32 size,
		      u8 *map)
{
	u32 ret;

	if (!map) {
		PLTFM_MSG_ERR("[ERR]malloc map\n");
		return MACBUFALLOC;
	}

	if (*bank_efuse_info.phy_map_valid == 1) {
		PLTFM_MEMCPY(map, *bank_efuse_info.phy_map + offset, size);
	} else {
		ret = read_hw_efuse(adapter, offset, size, map);
		if (ret != 0) {
			return ret;
		}
	}

	return MACSUCCESS;
}

static u32 query_status_map(struct mac_ax_adapter *adapter,
			    enum mac_ax_efuse_feature_id feature_id,
			    u8 *map)
{
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	enum mac_ax_intf intf = adapter->hw_info->intf;
	u32 map_size = 0;

	PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
	switch (feature_id) {
	case MAC_AX_DUMP_PHYSICAL_EFUSE:
		map_size = *bank_efuse_info.phy_map_size;
		PLTFM_MEMCPY(map, *bank_efuse_info.phy_map, map_size);
		break;
	case MAC_AX_DUMP_LOGICAL_EFUSE:
		map_size = *bank_efuse_info.log_map_size;
		PLTFM_MEMCPY(map, *bank_efuse_info.log_map, map_size);
		break;
	default:
		return MACFUNCINPUT;
	}
	PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);

	return MACSUCCESS;
}
#endif

static u32 compare_info_length(struct efuse_info *info,
			       enum rtw_efuse_info id, u32 length)
{
	u32 idle_len;

	switch (id) {
	case EFUSE_INFO_MAC_ADDR:
		idle_len = info->len->mac_addr;
		break;
	case EFUSE_INFO_MAC_PID:
		idle_len = info->len->pid;
		break;
	case EFUSE_INFO_MAC_DID:
		idle_len = info->len->did;
		break;
	case EFUSE_INFO_MAC_VID:
		idle_len = info->len->vid;
		break;
	case EFUSE_INFO_MAC_SVID:
		idle_len = info->len->svid;
		break;
	case EFUSE_INFO_MAC_SMID:
		idle_len = info->len->smid;
		break;
	default:
		return MACNOITEM;
	}

	if (length != idle_len || idle_len == 0)
		return MACLENCMP;

	return MACSUCCESS;
}

u32 mac_get_efuse_info(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length,
		       u8 *autoload_status)
{
	u32 offset, def_val;
	u32 ret;
	enum mac_ax_intf intf = adapter->hw_info->intf;
	struct efuse_info info;

	switch (intf) {
	case MAC_AX_INTF_AXI:
		info = efuse_info_axi;
		break;
	default:
		return MACINTF;
	}

	ret = compare_info_length(&info, id, length);
	if (ret != MACSUCCESS)
		return ret;

	switch (id) {
	case EFUSE_INFO_MAC_ADDR:
		offset = info.offset->mac_addr;
		def_val = info.def_val->mac_addr;
		break;
	case EFUSE_INFO_MAC_PID:
		offset = info.offset->pid;
		def_val = info.def_val->pid;
		break;
	case EFUSE_INFO_MAC_DID:
		offset = info.offset->did;
		def_val = info.def_val->did;
		break;
	case EFUSE_INFO_MAC_VID:
		offset = info.offset->vid;
		def_val = info.def_val->vid;
		break;
	case EFUSE_INFO_MAC_SVID:
		offset = info.offset->svid;
		def_val = info.def_val->svid;
		break;
	case EFUSE_INFO_MAC_SMID:
		offset = info.offset->smid;
		def_val = info.def_val->smid;
		break;
	default:
		return MACNOITEM;
	}

	if (*autoload_status == 0)
		PLTFM_MEMCPY(value, &def_val, 1);
	else
		PLTFM_MEMCPY(value, efuse_map + offset, length);

	return MACSUCCESS;
}

#if 0
static u32 set_check_sum_val(struct mac_ax_adapter *adapter,
			     u8 *map, u16 value)
{
	u8 byte1;
	u8 byte2;
	u8 blk;
	u8 blk_idx;
	u8 hdr;
	u8 hdr2;
	u32 end = *bank_efuse_info.efuse_end, ret;
	u32 offset = chksum_offset_1;
	u8 i = 0;
	u8 value8 = (u8)(value & 0xFF);
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	p_ops->enable_efuse_sw_pwr_cut(adapter, 1);

	for (i = 0; i < 2; i++) {
		blk = (u8)(offset >> 3);
		blk_idx = (u8)((offset & (8 - 1)) >> 1);

		hdr = ((blk & 0xF0) >> 4) | 0x30;
		hdr2 = (u8)(((blk & 0x0F) << 4) + ((0x1 << blk_idx) ^ 0x0F));

		if ((offset & 1) == 0) {
			byte1 = value8;
			byte2 = *(map + offset + 1);
		} else {
			byte1 = (u8)(value & 0xFF);
			byte2 = value8;
		}

		if (*bank_efuse_info.phy_map_size <= 4 + end) {
			return MACEFUSESIZE;
		}

		ret = write_hw_efuse(adapter, end, hdr);
		if (ret != 0) {
			return ret;
		}

		ret = write_hw_efuse(adapter, end + 1, hdr2);
		if (ret != 0) {
			return ret;
		}

		ret = write_hw_efuse(adapter, end + 2, byte1);
		if (ret != 0) {
			return ret;
		}

		ret = write_hw_efuse(adapter, end + 3, byte2);
		if (ret != 0) {
			return ret;
		}

		offset = chksum_offset_2;
		value8 = (u8)((value & 0xFF00) >> 8);
		end += 4;
	}

	p_ops->disable_efuse_sw_pwr_cut(adapter, 1);

	return MACSUCCESS;
}

static void cal_check_sum(struct mac_ax_adapter *adapter, u16 *chksum)
{
	u32 i = 0;
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	struct mac_ax_efuse_param *efuse_param = &adapter->efuse_param;
	u8 *map = efuse_param->log_efuse_map;
	u16 *data;

	data = (u16 *)map;

	*chksum = 0x0000;
	for (i = 0; i < hw_info->log_efuse_size >> 2; i++) {
		if (i == chksum_offset_1 >> 2) {
			*chksum ^= 0x0000 ^ *(data + (2 * i + 1));
		} else {
			*chksum ^= *(data + 2 * i) ^ *(data + (2 * i + 1));
		}
	}
}

static u32 compare_version(struct mac_ax_adapter *adapter,
			   struct mac_ax_pg_efuse_info *info, u32 ver_len)
{
	u8 *map = info->efuse_map;
	u8 *mask = info->efuse_mask;
	u32 map_size = info->efuse_map_size;
	u32 i = 0;

	for (i = 0; i < ver_len; i++) {
		if (*(map + map_size + i) != *(mask + (map_size >> 4) + i)) {
			return MACVERERR;
		}
	}
	return MACSUCCESS;
}

u32 enable_efuse_pwr_cut_dav(struct mac_ax_adapter *adapter,
			     bool is_write)
{
	u32 ret;
	u8 chip_id = adapter->hw_info->chip_id;

	if (!(is_write)) {
		return MACSUCCESS;
	}

	if (chip_id == MAC_AX_CHIP_ID_8852A) {
		return MACSUCCESS;
	} else {
		ret = mac_write_xtal_si(adapter, XTAL_SI_PWR_CUT,
					XTAL_SI_SMALL_PWR_CUT,
					XTAL_SI_SMALL_PWR_CUT);
		if (ret) {
			return ret;
		}

		ret = mac_write_xtal_si(adapter, XTAL_SI_PWR_CUT,
					XTAL_SI_BIG_PWR_CUT,
					XTAL_SI_BIG_PWR_CUT);
		if (ret) {
			return ret;
		}
	}

	return MACSUCCESS;
}

u32 disable_efuse_pwr_cut_dav(struct mac_ax_adapter *adapter,
			      bool is_write)
{
	u32 ret;
	u8 chip_id = adapter->hw_info->chip_id;

	if (!(is_write)) {
		return MACSUCCESS;
	}

	if (chip_id == MAC_AX_CHIP_ID_8852A) {
		return MACSUCCESS;
	} else {
		ret = mac_write_xtal_si(adapter, XTAL_SI_PWR_CUT, 0,
					XTAL_SI_BIG_PWR_CUT);
		if (ret) {
			return ret;
		}

		ret = mac_write_xtal_si(adapter, XTAL_SI_PWR_CUT, 0,
					XTAL_SI_SMALL_PWR_CUT);
		if (ret) {
			return ret;
		}
	}

	return MACSUCCESS;
}

static u32 read_hw_efuse_dav(struct mac_ax_adapter *adapter, u32 offset, u32 size,
			     u8 *map)
{
	u32 addr;
	u8 tmp8;
	u32 cnt;
	u32 ret;

	ret = enable_efuse_pwr_cut_dav(adapter, 0);
	if (ret) {
		return ret;
	}

	for (addr = offset; addr < offset + size; addr++) {
		/* clear ready bit*/
		ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, 0x40, FULL_BIT_MASK);
		if (ret) {
			return ret;
		}
		/* set addr */
		ret = mac_write_xtal_si(adapter, XTAL_SI_LOW_ADDR,
					(addr & 0xff) << XTAL_SI_LOW_ADDR_SH,
					XTAL_SI_LOW_ADDR_MSK);
		if (ret) {
			return ret;
		}
		ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, (addr >> 8)
					<< XTAL_SI_HIGH_ADDR_SH,
					XTAL_SI_HIGH_ADDR_MSK
					<< XTAL_SI_HIGH_ADDR_SH);
		if (ret) {
			return ret;
		}
		/* set ctrl mode sel */
		ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, 0,
					XTAL_SI_MODE_SEL_MSK
					<< XTAL_SI_MODE_SEL_SH);
		if (ret) {
			return ret;
		}

		/* polling */
		cnt = EFUSE_WAIT_CNT;
		while (--cnt) {
			ret = mac_read_xtal_si(adapter, XTAL_SI_CTRL, &tmp8);
			if (ret) {
				return ret;
			}
			if (tmp8 & XTAL_SI_RDY) {
				break;
			}
			PLTFM_DELAY_US(1);
		}
		if (!cnt) {
			PLTFM_MSG_ERR("[ERR]read efuse\n");
			return MACEFUSEREAD;
		}

		ret = mac_read_xtal_si(adapter, XTAL_SI_READ_VAL, &tmp8);
		if (ret) {
			return ret;
		}
		*(map + addr - offset) = tmp8;
	}

	ret = disable_efuse_pwr_cut_dav(adapter, 0);
	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

static u32 write_hw_efuse_dav(struct mac_ax_adapter *adapter, u32 offset,
			      u8 value)
{
	u32 cnt;
	u32 ret;
	u8 tmp8;

	PLTFM_MUTEX_LOCK(&efuse_tbl.lock);
	*bank_efuse_info.phy_map_valid = 0;
	*bank_efuse_info.log_map_valid = 0;
	PLTFM_MUTEX_UNLOCK(&efuse_tbl.lock);

	/* clear ready bit*/
	ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, 0x40, FULL_BIT_MASK);
	if (ret) {
		return ret;
	}
	/* set val */
	ret = mac_write_xtal_si(adapter, XTAL_SI_WRITE_VAL, value <<
				XTAL_SI_WRITE_DATA_SH, XTAL_SI_WRITE_DATA_MSK <<
				XTAL_SI_WRITE_DATA_SH);
	if (ret) {
		return ret;
	}
	/* set addr */
	ret = mac_write_xtal_si(adapter, XTAL_SI_LOW_ADDR, (offset & 0xff)
				<< XTAL_SI_LOW_ADDR_SH,
				XTAL_SI_LOW_ADDR_MSK << XTAL_SI_LOW_ADDR_SH);
	if (ret) {
		return ret;
	}
	ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, (offset >> 8)
				<< XTAL_SI_HIGH_ADDR_SH,
				XTAL_SI_HIGH_ADDR_MSK << XTAL_SI_HIGH_ADDR_SH);
	if (ret) {
		return ret;
	}
	/* set ctrl mode sel */
	ret = mac_write_xtal_si(adapter, XTAL_SI_CTRL, 0x2
				<< XTAL_SI_MODE_SEL_SH,
				XTAL_SI_MODE_SEL_MSK <<	XTAL_SI_MODE_SEL_SH);
	if (ret) {
		return ret;
	}

	/* polling */
	cnt = EFUSE_WAIT_CNT;
	while (--cnt) {
		ret = mac_read_xtal_si(adapter, XTAL_SI_CTRL, &tmp8);
		if (ret) {
			return ret;
		}
		if (tmp8 & XTAL_SI_RDY) {
			break;
		}
		PLTFM_DELAY_US(1);
	}
	if (!cnt) {
		PLTFM_MSG_ERR("[ERR]write efuse\n");
		return MACEFUSEREAD;
	}

	return MACSUCCESS;
}

static void switch_dv(struct mac_ax_adapter *adapter, enum rtw_dv_sel sel)
{
	dv_sel = sel;
}
#endif
