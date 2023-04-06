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

#include "mac_ax.h"
#include "mac_ax/mac_priv.h"
#include "misc/realtek-misc.h"

#define CHIP_ID_HW_DEF_8730E	0x6678

#define PID_HW_DEF_8852AS	0xA852
#define PID_HW_DEF_8852BS	0xB852
#define PID_HW_DEF_8852BSA	0xB85B
#define PID_HW_DEF_8851ASA	0x8851
#define PID_HW_DEF_8852BPS	0xA85C
#define PID_HW_DEF_8852CS	0xC852
#define PID_HW_DEF_8192XBS	0x0192
#define PID_HW_DEF_8851BS	0xB851
#define PID_HW_DEF_8851BSM	0xB51A
#define PID_HW_DEF_1115ES	0x892A

#define SDIO_FN0_CIS_PID	0x1004

#define SDIO_WAIT_CNT		50
#define MAC_AX_INDIR_CNT	23

#if MAC_AX_SDIO_SUPPORT
static u8 r8_indir_cmd52_sdio(void *drv_adapter,
			      struct mac_ax_pltfm_cb *pltfm_cb, u32 adr);
#endif

#ifndef CONFIG_NEW_HALMAC_INTERFACE
static u8 chk_pltfm_cb(void *drv_adapter, enum mac_ax_intf intf,
		       struct mac_ax_pltfm_cb *pltfm_cb)
{
	if (!pltfm_cb) {
		return MACSUCCESS;
	}

	if (!pltfm_cb->msg_print) {
		return MACSUCCESS;
	}

#if MAC_AX_SDIO_SUPPORT
	if (!pltfm_cb->sdio_cmd52_r8 || !pltfm_cb->sdio_cmd53_r8 ||
	    !pltfm_cb->sdio_cmd53_r16 || !pltfm_cb->sdio_cmd53_r32 ||
	    !pltfm_cb->sdio_cmd53_rn || !pltfm_cb->sdio_cmd52_w8 ||
	    !pltfm_cb->sdio_cmd53_w8 || !pltfm_cb->sdio_cmd53_w16 ||
	    !pltfm_cb->sdio_cmd53_w32 || !pltfm_cb->sdio_cmd53_wn ||
	    !pltfm_cb->sdio_cmd52_cia_r8) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "[ERR]CB-SDIO\n");
		return MACSUCCESS;
	}
#endif

#if (MAC_AX_USB_SUPPORT || MAC_AX_PCIE_SUPPORT)
	if (!pltfm_cb->reg_r8 || !pltfm_cb->reg_r16 ||
	    !pltfm_cb->reg_r32 || !pltfm_cb->reg_w8 ||
	    !pltfm_cb->reg_w16 || !pltfm_cb->reg_w32) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "[ERR]CB-USB or PCIE\n");
		return MACSUCCESS;
	}
#endif

#if MAC_AX_AXI_SUPPORT
	if (!pltfm_cb->reg_r8 || !pltfm_cb->reg_r16 ||
	    !pltfm_cb->reg_r32 || !pltfm_cb->reg_w8 ||
	    !pltfm_cb->reg_w16 || !pltfm_cb->reg_w32 ||
	    !pltfm_cb->mem_write || !pltfm_cb->mem_read ||
	    !pltfm_cb->sys_reg_r8 || !pltfm_cb->sys_reg_r16 ||
	    !pltfm_cb->sys_reg_r32 || !pltfm_cb->sys_reg_w8 ||
	    !pltfm_cb->sys_reg_w16 || !pltfm_cb->sys_reg_w32) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "[ERR]CB-AXI\n");
		return MACSUCCESS;
	}
#endif

	if (!pltfm_cb->rtl_free || !pltfm_cb->rtl_malloc ||
	    !pltfm_cb->rtl_memcpy || !pltfm_cb->rtl_memset ||
	    !pltfm_cb->rtl_delay_us || !pltfm_cb->rtl_delay_ms ||
	    !pltfm_cb->rtl_mutex_init || !pltfm_cb->rtl_mutex_deinit ||
	    !pltfm_cb->rtl_mutex_lock || !pltfm_cb->rtl_mutex_unlock ||
	    !pltfm_cb->event_notify) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "[ERR]CB-OS\n");
		return MACSUCCESS;
	}

	return MACPFCB;
}
#endif /*CONFIG_NEW_HALMAC_INTERFACE*/

static u8 chk_pltfm_endian(void)
{
	u32 num = 1;
	u8 *num_ptr = (u8 *)&num;

	if (*num_ptr != PLATFOM_IS_LITTLE_ENDIAN) {
		return MACSUCCESS;
	}

	return MACPFCB;
}

#if MAC_AX_SDIO_SUPPORT
static u8 r8_indir_cmd52_sdio(void *drv_adapter,
			      struct mac_ax_pltfm_cb *pltfm_cb, u32 adr)
{
	u8 tmp;
	u32 cnt;

	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR,
				(u8)adr);
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 1,
				(u8)(adr >> 8));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 2,
				(u8)(adr >> 16));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 3,
				(u8)((adr | B_AX_INDIRECT_RDY) >> 24));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_CTRL,
				(u8)B_AX_INDIRECT_REG_R);

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = pltfm_cb->sdio_cmd52_r8(drv_adapter,
					      R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_,
				    "[ERR]sdio indirect CMD52 read\n");

	return pltfm_cb->sdio_cmd52_r8(drv_adapter, R_AX_SDIO_INDIRECT_DATA);
}

static u8 r8_dir_cmd52_sdio(void *drv_adapter,
			    struct mac_ax_pltfm_cb *pltfm_cb, u32 adr)
{
	/* Original: Use indirect access WLAN reg due to SDIO power off*/
	/* SDIO reg changes after 8852C (0x1000 -> 0x4000) */
	/* -> SDIO indirect access reg is uncertain before get_chip_info */
	/* Fix: CMD52 direct read WLAN reg and repeat several times */
	/* to sync SDIO reg with MAC reg */
	/* MAC reg is in MAC clk domain & SDIO reg is in SDIO clk domain */
	u32 cnt;

	cnt = MAC_AX_INDIR_CNT;
	do {
		pltfm_cb->sdio_cmd52_r8(drv_adapter, adr);
		cnt--;
	} while (cnt);

	return pltfm_cb->sdio_cmd52_r8(drv_adapter, adr);
}

static u8 get_chip_id_hw_def_sdio(void *drv_adapter,
				  struct mac_ax_pltfm_cb *pltfm_cb, u32 adr)
{
	u16 pid;

	pid = pltfm_cb->sdio_cmd52_cia_r8(drv_adapter, SDIO_FN0_CIS_PID) |
	      (pltfm_cb->sdio_cmd52_cia_r8(drv_adapter, SDIO_FN0_CIS_PID + 1) << 8);

	switch (pid) {
	case PID_HW_DEF_8852AS:
		return CHIP_ID_HW_DEF_8852A;
	case PID_HW_DEF_8852BS:
	case PID_HW_DEF_8852BSA:
	case PID_HW_DEF_8851ASA:
	case PID_HW_DEF_8852BPS:
		return CHIP_ID_HW_DEF_8852B;
	case PID_HW_DEF_8852CS:
		return CHIP_ID_HW_DEF_8852C;
	case PID_HW_DEF_8192XBS:
		return CHIP_ID_HW_DEF_8192XB;
	case PID_HW_DEF_8851BS:
	case PID_HW_DEF_8851BSM:
		return CHIP_ID_HW_DEF_8851B;
	case PID_HW_DEF_1115ES:
		return CHIP_ID_HW_DEF_1115E;
	default:
		break;
	}

	return r8_dir_cmd52_sdio(drv_adapter, pltfm_cb, adr);
}
#endif
static u32 get_chip_info(void *drv_adapter, struct mac_ax_pltfm_cb *pltfm_cb,
			 enum mac_ax_intf intf, u16 *id, u8 *cv)
{
	u16 cur_id;

	if (!cv || !id) {
		return MACNPTR;
	}

	switch (intf) {
#if MAC_AX_AXI_SUPPORT
	case MAC_AX_INTF_AXI:
		cur_id = rtk_misc_get_rl_number();
		break;
#endif
	default:
		return MACINTF;
	}

	switch (cur_id) {
	case CHIP_ID_HW_DEF_8730E:
		*id = MAC_AX_CHIP_ID_8730E;
		*cv = rtk_misc_get_rl_version();
		break;
	default:
		return MACCHIPID;
	}

	return MACSUCCESS;
}

u32 mac_ax_ops_init(void *drv_adapter, struct mac_ax_pltfm_cb *pltfm_cb,
		    enum mac_ax_intf intf,
		    struct mac_ax_adapter **mac_adapter,
		    struct mac_ax_ops **mac_ops)
{
	u32 ret;
	u16 chip_id = 0;
	u8 cv = 0;
	struct mac_ax_adapter *adapter;
	struct mac_ax_priv_ops *p_ops;

	if (!chk_pltfm_cb(drv_adapter, intf, pltfm_cb)) {
		return MACPFCB;
	}

	if (!chk_pltfm_endian()) {
		return MACPFED;
	}

	pltfm_cb->msg_print(drv_adapter, _PHL_ERR_,
			    "MAC_AX_MAJOR_VER = %d\n"
			    "MAC_AX_PROTOTYPE_VER = %d\n"
			    "MAC_AX_SUB_VER = %d\n"
			    "MAC_AX_SUB_INDEX = %d\n",
			    MAC_AX_MAJOR_VER, MAC_AX_PROTOTYPE_VER,
			    MAC_AX_SUB_VER, MAC_AX_SUB_INDEX);

	ret = get_chip_info(drv_adapter, pltfm_cb, intf, &chip_id, &cv);
	if (ret) {
		return ret;
	}

	adapter = get_mac_ax_adapter(intf, chip_id, cv, drv_adapter,
				     pltfm_cb);
	if (!adapter) {
		pltfm_cb->msg_print(drv_adapter, _PHL_ERR_, "[ERR]Get MAC adapter\n");
		return MACADAPTER;
	}

	*mac_adapter = adapter;
	*mac_ops = adapter->ops;

#if MAC_AX_FEATURE_HV
	adapter->hv_ops = get_hv_ax_ops(adapter);
#endif

#if MAC_AX_PHL_H2C

#else
	ret = h2cb_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]h2cb init %d\n", ret);
		return ret;
	}
#endif

	ret = role_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]role tbl init %d\n", ret);
		return ret;
	}

	p_ops = adapter_to_priv_ops(adapter);
	ret = p_ops->sec_info_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sec info tbl init %d\n", ret);
		return ret;
	}

	ret = efuse_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]efuse tbl init %d\n", ret);
		return ret;
	}

	ret = p2p_info_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]p2p info init %d\n", ret);
		return ret;
	}

	ret = mport_info_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]mpinfo info init %d\n", ret);
		return ret;
	}

	ret = mix_info_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]mix info init %d\n", ret);
		return ret;
	}

#if MAC_AX_SDIO_SUPPORT
	ret = sdio_tbl_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sdio tbl init %d\n", ret);
		return ret;
	}
#endif

	ret = hfc_info_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]hfc info init %d\n", ret);
		return ret;
	}

	ret = twt_info_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]twt info init %d\n", ret);
		return ret;
	}

	return MACSUCCESS;
}

#if MAC_AX_PHL_H2C
u32 mac_ax_phl_init(void *phl_adapter, struct mac_ax_adapter *mac_adapter)
{
	struct mac_ax_adapter *adapter = mac_adapter;

	adapter->phl_adapter = phl_adapter;

	return MACSUCCESS;
}
#endif
u32 mac_ax_ops_exit(struct mac_ax_adapter *adapter)
{
	u32 ret = MACSUCCESS;
	struct mac_ax_efuse_param *efuse_param = &adapter->efuse_param;
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct scan_chinfo_list *scan_list = NULL;
	u32 priv_size = 0;

	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]h2c buffer exit %d\n", ret);
		return ret;
	}

	if (ofld_info->buf) {
		PLTFM_FREE(ofld_info->buf, CMD_OFLD_MAX_LEN);
		ofld_info->buf = NULL;
	}

	ret = p_ops->free_sec_info_tbl(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sec table exit %d\n", ret);
		return ret;
	}

	ret = role_tbl_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]role table exit %d\n", ret);
		return ret;
	}

	ret = efuse_tbl_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]efuse table exit %d\n", ret);
		return ret;
	}

	ret = p2p_info_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]p2p info exit %d\n", ret);
		return ret;
	}

	ret = mport_info_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]mpinfo info exit %d\n", ret);
		return ret;
	}

	ret = mix_info_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]mix info exit %d\n", ret);
		return ret;
	}

#if MAC_AX_SDIO_SUPPORT
	ret = sdio_tbl_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]efuse table exit %d\n", ret);
		return ret;
	}
#endif

	ret = hfc_info_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]hfc info exit %d\n", ret);
		return ret;
	}

	ret = twt_info_exit(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]twt info exit %d\n", ret);
		return ret;
	}

	if (efuse_param->efuse_map) {
		PLTFM_FREE(efuse_param->efuse_map,
			   adapter->hw_info->efuse_size);
		efuse_param->efuse_map = (u8 *)NULL;
	}

	if (efuse_param->log_efuse_map) {
		PLTFM_FREE(efuse_param->log_efuse_map,
			   adapter->hw_info->log_efuse_size);
		efuse_param->log_efuse_map = (u8 *)NULL;
	}

	scan_list = adapter->scanofld_info.list;
	if (scan_list) {
		mac_scanofld_ch_list_clear(adapter, scan_list);
		PLTFM_FREE((u8 *)scan_list, sizeof(struct scan_chinfo_list));
		adapter->scanofld_info.list = NULL;
	}

	PLTFM_FREE(adapter->hw_info, sizeof(struct mac_ax_hw_info));

	priv_size = get_mac_ax_priv_size();
	PLTFM_FREE(adapter, sizeof(struct mac_ax_adapter) + priv_size);
	return MACSUCCESS;
}

u32 is_chip_id(struct mac_ax_adapter *adapter, enum mac_ax_chip_id id)
{
	return (id == adapter->hw_info->chip_id ? 1 : 0);
}

u32 is_cv(struct mac_ax_adapter *adapter, enum rtw_cv cv)
{
	return (cv == adapter->hw_info->cv ? 1 : 0);
}

u32 is_intf(struct mac_ax_adapter *adapter, enum mac_ax_intf intf)
{
	return (intf == adapter->hw_info->intf ? 1 : 0);
}

