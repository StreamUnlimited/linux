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
#include "security_cam.h"

#define VALID			0x01
#define INVALID			0x00

u32 sec_info_tbl_init(struct mac_ax_adapter *adapter)
{
	u8 i = 0;
	struct sec_cam_table_t **sec_cam_table =
			&adapter->hw_info->sec_cam_table;

	/*First time access sec cam , initial sec cam table INVALID  */
	if ((*sec_cam_table) == NULL) {
		(*sec_cam_table) = (struct sec_cam_table_t *)PLTFM_MALLOC
				   (sizeof(struct sec_cam_table_t));

		if (!(*sec_cam_table)) {
			return MACNOBUF;
		}

		for (i = 0; i < SEC_CAM_ENTRY_NUM; i++) {
			(*sec_cam_table)->sec_cam_entry[i] =
				(struct sec_cam_entry_t *)PLTFM_MALLOC
				(sizeof(struct sec_cam_entry_t));

			if (!(*sec_cam_table)->sec_cam_entry[i]) {
				return MACNOBUF;
			}

			/* initial value*/
			(*sec_cam_table)->sec_cam_entry[i]->valid = INVALID;
			(*sec_cam_table)->sec_cam_entry[i]->macid = 0;
			(*sec_cam_table)->sec_cam_entry[i]->key_id = 0;
			(*sec_cam_table)->sec_cam_entry[i]->key_type = 0;
			PLTFM_MEMSET((*sec_cam_table)->sec_cam_entry[i]->mac_addr, 0x00, 6);
			(*sec_cam_table)->sec_cam_entry[i]->sec_type = 0;
			(*sec_cam_table)->sec_cam_entry[i]->mic = 0;
			(*sec_cam_table)->sec_cam_entry[i]->grp_bit = 0;
			(*sec_cam_table)->sec_cam_entry[i]->mgnt = 0;
			(*sec_cam_table)->sec_cam_entry[i]->ext_key = 0;
		}
	}

	return MACSUCCESS;
}

u32 free_sec_info_tbl(struct mac_ax_adapter *adapter)
{
	u8 i;
	struct sec_cam_table_t *sec_cam_table = adapter->hw_info->sec_cam_table;

	if (!sec_cam_table) {
		return MACSUCCESS;
	}

	for (i = 0; i < SEC_CAM_ENTRY_NUM; i++) {
		PLTFM_FREE(sec_cam_table->sec_cam_entry[i],
			   sizeof(struct sec_cam_entry_t));
	}

	PLTFM_FREE(sec_cam_table, sizeof(struct sec_cam_table_t));

	adapter->hw_info->sec_cam_table = NULL;
	return MACSUCCESS;
}

static u8 decide_sec_cam_index(struct mac_ax_adapter *adapter, u8 *sec_cam_idx,
			u8 mac_id, u8 key_id, u8 key_type, u8 *addr,
			u8 sec_type, u8 mic, u8 grp_bit, u8 mgnt,
			u8 ext_key)
{
	u8 sec_idx = 0, i = 0;
	u8 cam_entry_num = SEC_CAM_ENTRY_NUM - 4 - SEC_CAM_AP_BMC_NUM;
	u8 max_entry_num = SEC_CAM_ENTRY_NUM - SEC_CAM_AP_BMC_NUM;
	/* call by pointer */
	struct sec_cam_table_t **sec_cam_table =
			&adapter->hw_info->sec_cam_table;

	/*First time access sec cam , initial sec cam table INVALID  */
	sec_info_tbl_init(adapter);

	/*If table has been initialize, assgin the sec cam storge idx */
	sec_idx = 4;
	for (i = 0; i < cam_entry_num; i++) {
		if (((*sec_cam_table)->sec_cam_entry[sec_idx]->macid == mac_id) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->key_id == key_id) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->key_type == key_type) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->sec_type == sec_type) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->mic == mic) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->grp_bit == grp_bit) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->mgnt == mgnt) &&
		   ((*sec_cam_table)->sec_cam_entry[sec_idx]->ext_key == ext_key) &&
		   !PLTFM_MEMCMP((*sec_cam_table)->sec_cam_entry[sec_idx]->mac_addr, addr, 6)
		   ) {
			*sec_cam_idx = sec_idx;
			return MACSUCCESS;
		} else if ((*sec_cam_table)->sec_cam_entry[sec_idx]->valid ==
			   INVALID) {
			*sec_cam_idx = sec_idx;
			return MACSUCCESS;
		}
		sec_idx++;
		if (sec_idx > max_entry_num) {
			sec_idx = 4;
		}
	}

	return MACSECCAMFL;
}

static u32 add_sec_cam_index(struct mac_ax_adapter *adapter, u8 sec_cam_idx,
			    u8 mac_id, u8 key_id, u8 key_type, u8 *addr,
			    u8 sec_type, u8 mic, u8 grp_bit, u8 mgnt,
			    u8 ext_key)
{
	u8 sec_idx = 0, i = 0;
	/* call by pointer */
	struct sec_cam_table_t **sec_cam_table =
			&adapter->hw_info->sec_cam_table;

	/*First time access sec cam , initial sec cam table INVALID  */
	sec_info_tbl_init(adapter);

	/*If table has been initialize, assgin the sec cam storge idx */
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->key_id = key_id;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->macid = mac_id;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->key_type = key_type;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->valid = VALID;
	PLTFM_MEMCPY((*sec_cam_table)->sec_cam_entry[sec_cam_idx]->mac_addr, addr, 6);
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->sec_type = sec_type;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->mic = mic;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->grp_bit = grp_bit;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->mgnt = mgnt;
	(*sec_cam_table)->sec_cam_entry[sec_cam_idx]->ext_key = ext_key;

	PLTFM_MSG_ALWAYS("(macid %d) add key: idx %d, type %d.",
			 mac_id, sec_cam_idx, key_type);

	return MACSUCCESS;
}

u32 mac_write_security_cam(struct mac_ax_adapter *adapter, u32 idx,
			   struct halmac_cam_entry_info *info)
{
	u32 i = 0;
	u32 val32 = 0, *pval32 = NULL;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct halmac_cam_entry_format *fmt = NULL;

	PLTFM_MSG_TRACE("[TRACE]%s ===>\n", __func__);

	if (idx >= SEC_CAM_ENTRY_NUM) {
		PLTFM_MSG_ERR("write keycam failed, keycam indx (%d) > %d.\n",
			      idx, SEC_CAM_ENTRY_NUM);
		return MACOUTINDEX;
	}
	if (info->key_id > 3) {
		PLTFM_MSG_ERR("write keycam failed, key (%d) > 3.\n",
			      info->key_id);
		return MACNOITEM;
	}

	fmt = (struct halmac_cam_entry_format *)PLTFM_MALLOC(sizeof(*fmt));
	if (!fmt) {
		return MACNOBUF;
	}
	PLTFM_MEMSET(fmt, 0x00, sizeof(*fmt));

	fmt->key_id = info->key_id;
	fmt->valid = info->valid;
	fmt->ext_sectype = info->ext_key;
	fmt->mic = info->mic;
	fmt->mgnt = info->mgnt;
	fmt->spp_mode = info->spp_mode;
	fmt->mfb = info->mfb;
	PLTFM_MEMCPY(fmt->mac_address, info->mac_address, 6);
	PLTFM_MEMCPY(fmt->key, info->key, 16);
	switch (info->security_type) {
	case RTW_ENC_NONE:
		fmt->type = HAL_SEC_TYPE_NONE;
		break;
	case RTW_ENC_WEP40:
		fmt->type = HAL_SEC_TYPE_WEP40;
		break;
	case RTW_ENC_WEP104:
		fmt->type = HAL_SEC_TYPE_WEP104;
		break;
	case RTW_ENC_TKIP:
		if (info->mic == 1) {
			fmt->type = HAL_SEC_TYPE_TKIP_MIC;
		} else {
			fmt->type = HAL_SEC_TYPE_TKIP_NO_MIC;
		}
		break;
	case RTW_ENC_CCMP:
		fmt->type = HAL_SEC_TYPE_AES;
		break;
	case RTW_ENC_WAPI:
		fmt->type = HAL_SEC_TYPE_WAPI_SMS4;
		break;
	case RTW_ENC_CCMP256:
		fmt->type = HAL_SEC_TYPE_AES;
		fmt->ext_sectype = 1;
		break;
	case RTW_ENC_GCMP:
		fmt->type = HAL_SEC_TYPE_GCMP;
		break;
	case RTW_ENC_GCMP256:
		fmt->type = HAL_SEC_TYPE_GCMP;
		fmt->ext_sectype = 1;
		break;
	case RTW_ENC_GCMSMS4:
		fmt->type = HAL_SEC_TYPE_WAPI_SMS4;
		fmt->ext_sectype = 1;
		break;
	case RTW_ENC_BIP_CCMP128:
		fmt->type = (info->unicast == 1) ? 4 : 0;
		fmt->mgnt = 1;
		fmt->grp = (info->unicast == 1) ? 0 : 1;
		fmt->mic = 1;
		//jerry_zhou
		break;
	default:
		PLTFM_FREE(fmt, sizeof(*fmt));
		return MACNOITEM;
	}

	val32 = MAC_REG_R32(REG_KEYCAMCMD);
	val32 |= BIT_KEYCAM_DA_EN;
	MAC_REG_W32(REG_KEYCAMCMD, val32);

	pval32 = (u32 *)fmt;
	for (i = 0; i < 8; i++) {
		MAC_REG_W32(SEC_CAM_BASE + ((idx << 3) + i) * 4, pval32[i]);
	}

	val32 = MAC_REG_R32(REG_KEYCAMCMD);
	val32 &= ~ BIT_KEYCAM_DA_EN;
	MAC_REG_W32(REG_KEYCAMCMD, val32);

	val32 = MAC_REG_R32(REG_SECCFG);
	val32 |= (BIT_TXBCUSEDK | BIT_RXBCUSEDK);
	if ((info->security_type == RTW_ENC_WEP40)
	    || (info->security_type == RTW_ENC_WEP104)) {
		val32 |= (BIT_TXUHUSEDK | BIT_RXUHUSEDK);
	} else {
		val32 &= ~(BIT_TXUHUSEDK | BIT_RXUHUSEDK);
	}
	MAC_REG_W32(REG_SECCFG, val32);

	PLTFM_FREE(fmt, sizeof(*fmt));
	PLTFM_MSG_TRACE("[TRACE]%s <===\n", __func__);
	return MACSUCCESS;
}

u32 mac_sta_hw_security_support(struct mac_ax_adapter *adapter,
				u8 hw_security_support_type, u8 enable)
{
	u32 val32 = 0, val = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	switch (enable) {
	case 1:
		val32 = MAC_REG_R32(REG_SECCFG);
		val = MAC_REG_R32(REG_WSEC_OPTION);
		switch (hw_security_support_type) {
		case SEC_TX_UHUSEDK:
			val32 |= BIT_TXUHUSEDK;
			break;
		case SEC_RX_UHUSEDK:
			val32 |= BIT_RXUHUSEDK;
			break;
		case SEC_TX_ENC:
			val32 |= BIT_TXENC;
			break;
		case SEC_RX_DEC:
			val32 |= BIT_RXDEC;
			break;
		case SEC_UC_MGNT_DEC:
			val |= BIT_RXDEC_UNI_MGNT;
			break;
		case SEC_BMC_MGNT_DEC:
			val |= BIT_RXDEC_BM_MGNT;
			break;
		case SEC_UC_MGNT_ENC:
			val |= BIT_TXENC_UNI_MGNT;
			break;
		case SEC_BMC_MGNT_ENC:
			val |= BIT_TXENC_BM_MGNT;
			break;
		case SEC_TX_BCUSEDK:
			val32 |= BIT_TXBCUSEDK;
			break;
		case SEC_RX_BCUSEDK:
			val32 |= BIT_RXBCUSEDK;
			break;
		default:
			break;
		}
		MAC_REG_W32(REG_SECCFG, val32);
		MAC_REG_W32(REG_WSEC_OPTION, val);
		break;
	case 0:
		val32 = MAC_REG_R32(REG_SECCFG);
		switch (hw_security_support_type) {
		case SEC_TX_UHUSEDK:
			val32 &= ~BIT_TXUHUSEDK;
			break;
		case SEC_RX_UHUSEDK:
			val32 &= ~BIT_RXUHUSEDK;
			break;
		case SEC_TX_ENC:
			val32 &= ~BIT_TXENC;
			break;
		case SEC_RX_DEC:
			val32 &= ~BIT_RXDEC;
			break;
		case SEC_UC_MGNT_DEC:
			val &= ~BIT_RXDEC_UNI_MGNT;
			break;
		case SEC_BMC_MGNT_DEC:
			val &= ~BIT_RXDEC_BM_MGNT;
			break;
		case SEC_UC_MGNT_ENC:
			val &= ~BIT_TXENC_UNI_MGNT;
			break;
		case SEC_BMC_MGNT_ENC:
			val &= ~BIT_TXENC_BM_MGNT;
			break;
		case SEC_TX_BCUSEDK:
			val32 &= ~BIT_TXBCUSEDK;
			break;
		case SEC_RX_BCUSEDK:
			val32 &= ~BIT_RXBCUSEDK;
			break;
		default:
			break;
		}

		MAC_REG_W32(REG_SECCFG, val32);
		MAC_REG_W32(REG_WSEC_OPTION, val);
		break;
	default:
		break;
	}

	return MACSUCCESS;
}

u32 mac_sta_add_key(struct mac_ax_adapter *adapter,
		    struct mac_ax_sec_cam_info *sec_cam_info,
		    u8 mac_id,
		    u8 key_id,
		    u8 key_type)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct halmac_cam_entry_info info = {0};
	struct mac_role_tbl *role = NULL;
	struct sec_cam_table_t *sec_cam_table =
		adapter->hw_info->sec_cam_table;
	u8 idx = 0;
	u32 ret = 0;
	bool fixed_camidx = true;

	role = mac_role_srch(adapter, mac_id);
	if (!role) {
		PLTFM_MSG_TRACE("to get role failed!\n");
		return MACNOROLE;
	}

	if ((sec_cam_info->type == RTW_ENC_WEP40)
	    || (sec_cam_info->type == RTW_ENC_WEP104)
	    || (sec_cam_info->type == RTW_ENC_NONE)
	    || (key_type == KEY_TYPE_GROUP)){
		PLTFM_MEMSET(sec_cam_info->addr, 0, 6);
	} else if ((key_type == KEY_TYPE_UNI)
		   && (role->info.wifi_role == MAC_AX_WIFI_ROLE_STATION)) {
		PLTFM_MEMCPY(sec_cam_info->addr, role->info.bssid, 6);
	} else if ((key_type == KEY_TYPE_UNI)
		   && (role->info.wifi_role == MAC_AX_WIFI_ROLE_AP)) {
		PLTFM_MEMCPY(sec_cam_info->addr, role->info.target_mac, 6);
	}

	/* softAP mode: write GTK to SEC_CAM entry30 and entry31 (if extended key);
	 * STA mode: write GTK to  SEC_CAM entry0~3 according keyid */
	if ((sec_cam_info->type == RTW_ENC_WEP40)
	    || (sec_cam_info->type == RTW_ENC_WEP104)){
		sec_cam_info->sec_cam_idx = key_id;
	} else if (role->info.wifi_role == MAC_AX_WIFI_ROLE_STATION) {
		if (key_type == KEY_TYPE_GROUP) {
			sec_cam_info->sec_cam_idx = key_id;
		} else {
			fixed_camidx = false;
		}
	} else if (role->info.wifi_role == MAC_AX_WIFI_ROLE_AP) {
		if (key_type == KEY_TYPE_GROUP) {
#ifdef CONFIG_CONCURRENT_MODE
			if (sec_cam_info->ext_key && sec_cam_info->mic) {
				sec_cam_info->sec_cam_idx = BMC_SEC_MAC_CAM_ID + 1;
			} else {
				sec_cam_info->sec_cam_idx = BMC_SEC_MAC_CAM_ID;
			}
#else
			sec_cam_info->sec_cam_idx = key_id;
#endif
		} else {
			/* cam id = sta macid + 4 */
			fixed_camidx = false;
		}
	}

	sec_cam_info->grp_bit = (key_type == KEY_TYPE_GROUP) ? 1 : 0;
	if (!fixed_camidx) {
		ret = decide_sec_cam_index(adapter, &idx, role->info.macid,
					   key_id, key_type, sec_cam_info->addr,
					   sec_cam_info->type, sec_cam_info->mic,
					   sec_cam_info->grp_bit, sec_cam_info->mgnt,
					   sec_cam_info->ext_key);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("find keycam index failed, ret (%d).\n",
			      ret);
			return ret;
		}
		sec_cam_info->sec_cam_idx = idx;
	}

	PLTFM_MSG_ALWAYS("(macid %d, mac: "MAC_FMT")add key: idx %d, type %d, key_id %d, "
			 "grp_bit %d, mgnt %d, mic %d.",
			 role->info.macid, MAC_ARG(sec_cam_info->addr),
			 sec_cam_info->sec_cam_idx, sec_cam_info->type, key_id,
			 sec_cam_info->grp_bit, sec_cam_info->mgnt,
			 sec_cam_info->mic);
	PLTFM_MSG_ALWAYS("ext_key %d, spp_mode %d, mfb %d.",
			 sec_cam_info->ext_key, sec_cam_info->spp_mode,
			 sec_cam_info->mfb);
	/* set halmac_cam_entry_info for mac_write_security_cam. */
	info.key_id = key_id;
	info.grp_bit = sec_cam_info->grp_bit;
	info.unicast = (key_type == KEY_TYPE_UNI) ? 1 : 0;
	info.mgnt = sec_cam_info->mgnt;
	info.mic = sec_cam_info->mic;
	info.ext_key = sec_cam_info->ext_key;
	info.spp_mode = sec_cam_info->spp_mode;
	info.mfb = sec_cam_info->mfb;
	info.valid = 1;
	PLTFM_MEMCPY(info.mac_address, sec_cam_info->addr, 6);
	info.security_type = sec_cam_info->type;
	PLTFM_MEMCPY(info.key, sec_cam_info->key, 16);

	ret = mac_write_security_cam(adapter, sec_cam_info->sec_cam_idx, &info);
	if (ret == MACSUCCESS) {
		PLTFM_MEMCPY(&role->info.s_info, sec_cam_info, sizeof(struct mac_ax_sec_cam_info));
		ret = add_sec_cam_index(adapter, sec_cam_info->sec_cam_idx,
					mac_id, key_id, key_type,
					sec_cam_info->addr, sec_cam_info->type,
					sec_cam_info->mic, sec_cam_info->grp_bit,
					sec_cam_info->mgnt, sec_cam_info->ext_key);
	}

	return ret;
}

u32 mac_sta_del_key(struct mac_ax_adapter *adapter,
		    u8 mac_id,
		    u8 key_id,
		    u8 key_type)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct halmac_cam_entry_info info = {0};
	struct sec_cam_entry_t *s_entry = NULL;
	struct sec_cam_table_t *sec_cam_table =
		adapter->hw_info->sec_cam_table;
	u8 sec_cam_idx = 0;
	u32 ret = 0;

	/*Search SEC CAM Table */
	for (sec_cam_idx = 0; sec_cam_idx < SEC_CAM_ENTRY_NUM; sec_cam_idx++) {
		/* Only search valid entry */
		if (sec_cam_table->sec_cam_entry[sec_cam_idx]->valid == VALID) {
			s_entry = sec_cam_table->sec_cam_entry[sec_cam_idx];

			if (s_entry->macid != mac_id)
				continue;
			if (s_entry->key_id != key_id)
				continue;
			if (s_entry->key_type != key_type)
				continue;

			break;
		}
	}

	if (sec_cam_idx >= SEC_CAM_ENTRY_NUM) {
		return MACNOKEYINDEX;
	}

	info.key_id = 0;
	info.grp_bit = 0;
	info.unicast = 0;
	info.mgnt = 0;
	info.mic = 0;
	info.valid = 0;
	PLTFM_MEMSET(info.mac_address,0, 6);
	PLTFM_MEMSET(info.key, 0, 16);
	info.security_type = 0;


	ret = mac_write_security_cam(adapter, sec_cam_idx, &info);
	if (ret == MACSUCCESS) {
		sec_cam_table->sec_cam_entry[sec_cam_idx]->valid = INVALID;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->macid = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->key_id = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->key_type = 0;
		PLTFM_MEMSET(sec_cam_table->sec_cam_entry[sec_cam_idx]->mac_addr, 0x00, 6);
		sec_cam_table->sec_cam_entry[sec_cam_idx]->sec_type = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->mic = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->grp_bit = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->mgnt = 0;
		sec_cam_table->sec_cam_entry[sec_cam_idx]->ext_key = 0;
		PLTFM_MSG_ALWAYS("(macid %d) delete key: idx %d, type %d.",
			 mac_id, sec_cam_idx, key_type);
	} else {
		PLTFM_MSG_ERR("Delete key failed: macid %d, idx %d, type %d.",
			      mac_id, sec_cam_idx, key_type);
	}

	return ret;
}

u32 mac_sta_search_key_idx(struct mac_ax_adapter *adapter,
			   u8 mac_id, u8 key_id, u8 key_type)
{
	u8 sec_cam_idx = 0;
	struct sec_cam_entry_t *s_entry = NULL;
	struct sec_cam_table_t *sec_cam_table =
		adapter->hw_info->sec_cam_table;

	if (!sec_cam_table)
		return MACNOKEYINDEX;

	/*Search SEC CAM Table */
	for (sec_cam_idx = 0; sec_cam_idx < SEC_CAM_ENTRY_NUM; sec_cam_idx++) {
		/* Only search valid entry */
		if (sec_cam_table->sec_cam_entry[sec_cam_idx]->valid == VALID) {
			s_entry = sec_cam_table->sec_cam_entry[sec_cam_idx];

			if (s_entry->macid != mac_id)
				continue;
			if (s_entry->key_id != key_id)
				continue;
			if (s_entry->key_type != key_type)
				continue;

			return sec_cam_idx;
		}
	}
	return MACNOKEYINDEX;
}
