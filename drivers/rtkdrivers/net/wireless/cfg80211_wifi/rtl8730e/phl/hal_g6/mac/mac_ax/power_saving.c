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

#include "power_saving.h"
#include "coex.h"
#include "mac_priv.h"
#include "fwcmd.h"

#define RPWM_SEQ_NUM_MAX                3
#define CPWM_SEQ_NUM_MAX                3

//RPWM bit definition
#define PS_RPWM_TOGGLE          BIT(15)
#define PS_RPWM_ACK             BIT(6)
#define PS_RPWM_PARTIAL_OFF_EN  BIT(4)
#define PS_RPWM_32K_EN          BIT(0)
#define PS_RPWM_32K_EN_SH       (0)

//CPWM bit definition
#define PS_CPWM_TOGGLE			BIT(15)
#define PS_CPWM_ACK             BIT(14)
#define PS_CPWM_SEQ_NUM_SH      12
#define PS_CPWM_SEQ_NUM_MSK     0x3
#define PS_CPWM_RSP_SEQ_NUM_SH  8
#define PS_CPWM_RSP_SEQ_NUM_MSK 0x3
#define PS_CPWM_STATE_SH        0
#define PS_CPWM_STATE_MSK       0x7

//(workaround) CPWM register is in OFF area
//LPS debug message bit definition
#define B_PS_LDM_32K_EN         BIT(31)
#define B_PS_LDM_32K_EN_SH      31

struct t_h2c_20 {
	/* data 0 */
	u8 mode: 7;
	u8 clk_request: 1;
	/* data 1 */
	u8 rlbm: 4;
	u8 smartps: 4;
	/* data 2 */
	u8 awake_intvl;
	/* data 3 */
	u8 b_all_q_uapsd: 1;
	u8 rsvd1: 1;
	u8 bcn_early_prt: 1;
	u8 xtal_sel: 1;
	u8 rsvd2: 1;
	u8 port_id: 3;
	/* data 4 */
	u8 pwr_state;
	/* data 5 */
	u8 rsvd3;
	/* data 6 */
	u8 bcn_recv_time: 5;
	u8 bcn_listen_interval: 2;
	u8 adopt_bcn_recv_time: 1;
};

static u32 lps_status[4] = {0};
static u32 ips_status[4] = {0};
static u8 rpwm_seq_num = RPWM_SEQ_NUM_MAX;
static u8 cpwm_seq_num = CPWM_SEQ_NUM_MAX;

static u32 send_h2c_lps_parm(struct mac_ax_adapter *adapter,
			     struct lps_parm *parm)
{
	struct rtw_hal_com_t *hal_com = adapter->drv_adapter;
	struct t_h2c_20 h2c_data = {0};
	u8 *p_h2c_data = (u8 *)&h2c_data;
	u8 pwr_state = 0;
	u32 ret = 0;

	h2c_data.awake_intvl = parm->awakeinterval;
	h2c_data.rlbm = parm->rlbm;

	/* pwr_state: AllON(0x0C), RFON(0x04), RFOFF(0x00) */
	if (parm->psmode > 0) {
#ifdef CONFIG_BTCOEX
		/* todo if is bt control lps */
#endif
		{
			pwr_state = 0x00;
		}
	} else {
		pwr_state = 0x0c;
	}
	h2c_data.mode = parm->psmode;
	h2c_data.smartps = parm->smartps;
	h2c_data.b_all_q_uapsd = 0;
	h2c_data.pwr_state = pwr_state;
	h2c_data.port_id = 0;

	ret = rtw_hal_mac_send_h2c_ameba(hal_com, H2C_SET_PWR_MODE,
					 sizeof(struct t_h2c_20), p_h2c_data);

	return ret;
}

static void send_rpwm(struct mac_ax_adapter *adapter,
		      struct ps_rpwm_parm *parm)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (parm->notify_wake) {
		/* do nothing*/
	} else {
		if (parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_ACTIVE ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_BAND0_RFON ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_BAND1_RFON ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_BAND0_RFOFF ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_BAND1_RFOFF) {
		}

		if (parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_CLK_GATED ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_PWR_GATED ||
		    parm->req_pwr_state == MAC_AX_RPWM_REQ_PWR_STATE_HIOE_PWR_GATED) {
			adapter->mac_pwr_info.pwr_in_lps = 1;
		}
	}
}

static u32 leave_lps(struct mac_ax_adapter *adapter, u8 macid)
{
	struct lps_parm h2c_lps_parm;
	u32 ret;

	PLTFM_MEMSET(&h2c_lps_parm, 0, sizeof(struct lps_parm));

	h2c_lps_parm.macid = macid;
	h2c_lps_parm.psmode = MAC_AX_PS_MODE_ACTIVE;
	h2c_lps_parm.lastrpwm = LAST_RPWM_ACTIVE;

	ret = send_h2c_lps_parm(adapter, &h2c_lps_parm);

	if (ret) {
		return ret;
	}

	lps_status[(macid >> 5)] &= ~BIT(macid & 0x1F);

	return MACSUCCESS;
}

static u32 enter_lps(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_lps_info *lps_info)
{
	struct lps_parm h2c_lps_parm;
	u32 ret = 0;

	if (!lps_info) {
		PLTFM_MSG_ERR("[ERR]:LPS info is null\n");
		return MACNOITEM;
	}

	if (lps_status[(macid >> 5)] & BIT(macid & 0x1F)) {
		ret = leave_lps(adapter, macid);
	}

	if (ret) {
		return ret;
	}

	PLTFM_MEMSET(&h2c_lps_parm, 0, sizeof(struct lps_parm));

	h2c_lps_parm.macid = macid;
	h2c_lps_parm.psmode = MAC_AX_PS_MODE_LEGACY;

	if (lps_info->listen_bcn_mode > MAC_AX_RLBM_USERDEFINE) {
		lps_info->listen_bcn_mode = MAC_AX_RLBM_MIN;
	}

	if (lps_info->listen_bcn_mode == MAC_AX_RLBM_USERDEFINE) {
		h2c_lps_parm.rlbm = MAC_AX_RLBM_USERDEFINE;
		h2c_lps_parm.awakeinterval = lps_info->awake_interval;
		if (h2c_lps_parm.awakeinterval == 0) {
			h2c_lps_parm.awakeinterval = 1;
		}
	} else if (lps_info->listen_bcn_mode == MAC_AX_RLBM_MAX) {
		h2c_lps_parm.rlbm = MAC_AX_RLBM_MAX;
		h2c_lps_parm.awakeinterval = 1;
	} else {
		h2c_lps_parm.rlbm = MAC_AX_RLBM_MIN;
		h2c_lps_parm.awakeinterval = 1;
	}

	h2c_lps_parm.smartps = lps_info->smart_ps_mode;
	h2c_lps_parm.lastrpwm = LAST_RPWM_PS;
	h2c_lps_parm.bcnnohit_en = lps_info->bcnnohit_en;

	ret = send_h2c_lps_parm(adapter, &h2c_lps_parm);

	if (ret) {
		return ret;
	}

	lps_status[(macid >> 5)] |= BIT(macid & 0x1F);

	return MACSUCCESS;
}

static u32 set_req_pwr_state(struct mac_ax_adapter *adapter,
			     enum mac_ax_rpwm_req_pwr_state req_pwr_state)
{
	struct ps_rpwm_parm parm;

	PLTFM_MEMSET(&parm, 0, sizeof(struct ps_rpwm_parm));

	if (req_pwr_state >= MAC_AX_RPWM_REQ_PWR_STATE_MAX) {
		PLTFM_MSG_ERR("%s: invalid pwr state:%d\n",
			      __func__, req_pwr_state);
		return MACNOITEM;
	}

	parm.req_pwr_state = req_pwr_state;
	parm.notify_wake = 0;
	send_rpwm(adapter, &parm);

	return MACSUCCESS;
}

static u32 _chk_cpwm_seq_num(u8 seq_num)
{
	u32 ret;

	if (cpwm_seq_num == CPWM_SEQ_NUM_MAX) {
		if (seq_num == 0) {
			cpwm_seq_num = seq_num;
			ret = MACSUCCESS;
		} else {
			ret = MACCPWMSEQERR;
		}
	} else {
		if (seq_num == (cpwm_seq_num + 1)) {
			cpwm_seq_num = seq_num;
			ret = MACSUCCESS;
		} else {
			ret = MACCPWMSEQERR;
		}
	}

	return ret;
}

static u32 chk_cur_pwr_state(struct mac_ax_adapter *adapter,
			     enum mac_ax_rpwm_req_pwr_state req_pwr_state)
{
	u32 in_lps;
#if MAC_AX_AXI_SUPPORT
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	u32 ret = MACSUCCESS;
#endif

	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (req_pwr_state >= MAC_AX_RPWM_REQ_PWR_STATE_CLK_GATED) {
		in_lps = 1;
	} else {
		in_lps = 0;
	}

	if (in_lps) {
		return MACSUCCESS;
	}

	if (adapter->mac_pwr_info.pwr_in_lps) {
		adapter->mac_pwr_info.pwr_in_lps = 0;
	}

	return MACSUCCESS;
}

u32 mac_cfg_lps(struct mac_ax_adapter *adapter, u8 macid,
		enum mac_ax_ps_mode ps_mode, struct mac_ax_lps_info *lps_info)
{
	u32 ret = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	switch (ps_mode) {
	case MAC_AX_PS_MODE_ACTIVE:
		ret = leave_lps(adapter, macid);
		break;
	case MAC_AX_PS_MODE_LEGACY:
		ret = enter_lps(adapter, macid, lps_info);
		break;
	case MAC_AX_PS_MODE_WMMPS:
		// TODO:
		break;
	default:
		return MACNOITEM;
	}

	if (ret) {
		return ret;
	}

	return MACSUCCESS;
}

u32 mac_ps_pwr_state(struct mac_ax_adapter *adapter,
		     enum mac_ax_pwr_state_action action,
		     enum mac_ax_rpwm_req_pwr_state req_pwr_state)
{
	u32 ret = MACSUCCESS;

	switch (action) {
	case MAC_AX_PWR_STATE_ACT_REQ:
		ret = set_req_pwr_state(adapter, req_pwr_state);
		break;
	case MAC_AX_PWR_STATE_ACT_CHK:
		ret = chk_cur_pwr_state(adapter, req_pwr_state);
		break;
	default:
		ret = MACNOITEM;
	}

	return ret;
}

u32 mac_chk_leave_lps(struct mac_ax_adapter *adapter, u8 macid)
{
	u8 port = 0;
	u32 chk_msk = 0;
	struct mac_role_tbl *role;
	u16 pwrbit_set_reg = REG_PWRBIT_SETTING;
	u32 pwr_mgt_en_bit = 0x2;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg_sleep = 0;
	u8 macid_grp = macid >> MACID_GRP_SH;
	u8 macid_sh = macid & MACID_GRP_MASK;

	role = mac_role_srch(adapter, macid);

	if (!role) {
		PLTFM_MSG_ERR("[ERR]cannot find macid: %d\n", macid);
		return MACNOITEM;
	}

	port = role->info.a_info.port_int;

	chk_msk = pwr_mgt_en_bit << (PORT_SH * port);
	switch (macid_grp) {
	case MACID_GRP_0:
		reg_sleep = REG_MACID_SLEEP;
		break;
	case MACID_GRP_1:
		reg_sleep = REG_MACID_SLEEP1;
		break;
	case MACID_GRP_2:
		reg_sleep = REG_MACID_SLEEP2;
		break;
	case MACID_GRP_3:
		reg_sleep = REG_MACID_SLEEP3;
		break;
	default:
		return MACPSSTATFAIL;
	}


	if ((MAC_REG_R32(reg_sleep) & BIT(macid_sh))) {
		return MACPSSTATFAIL;
	}

	if ((MAC_REG_R32(pwrbit_set_reg) & chk_msk)) {
		return MACPSSTATPWRBITFAIL;
	}

	return MACSUCCESS;
}

u8 _is_in_lps(struct mac_ax_adapter *adapter)
{
	u8 i;

	for (i = 0; i < 4; i++) {
		if (lps_status[i] != 0) {
			return 1;
		}
	}

	return 0;
}

void reset_lps_seq_num(struct mac_ax_adapter *adapter)
{
	rpwm_seq_num = RPWM_SEQ_NUM_MAX;
	cpwm_seq_num = CPWM_SEQ_NUM_MAX;
}

static u32 send_h2c_ips_cfg(struct mac_ax_adapter *adapter,
			    struct ips_cfg *cfg)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 leave_ips(struct mac_ax_adapter *adapter, u8 macid)
{
	struct ips_cfg h2c_ips_cfg;
	u32 ret;

	PLTFM_MEMSET(&h2c_ips_cfg, 0, sizeof(struct ips_cfg));

	h2c_ips_cfg.macid = macid;
	h2c_ips_cfg.enable = 0;

	ret = send_h2c_ips_cfg(adapter, &h2c_ips_cfg);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ips_status[(macid >> 5)] &= ~BIT(macid & 0x1F);

	return MACSUCCESS;
}

static u32 enter_ips(struct mac_ax_adapter *adapter,
		     u8 macid)
{
	struct ips_cfg h2c_ips_cfg;
	u32 ret;

	if (ips_status[(macid >> 5)] & BIT(macid & 0x1F)) {
		PLTFM_MSG_ERR("[ERR]:IPS info is null\n");
		ret = leave_ips(adapter, macid);
		if (ret != MACSUCCESS) {
			return ret;
		}
	}

	PLTFM_MEMSET(&h2c_ips_cfg, 0, sizeof(struct ips_cfg));

	h2c_ips_cfg.macid = macid;
	h2c_ips_cfg.enable = 1;

	ret = send_h2c_ips_cfg(adapter, &h2c_ips_cfg);
	if (ret != MACSUCCESS) {
		return ret;
	}

	ips_status[(macid >> 5)] |= BIT(macid & 0x1F);

	return MACSUCCESS;
}

u32 mac_cfg_ips(struct mac_ax_adapter *adapter, u8 macid,
		u8 enable)
{
	u32 ret;

	if (enable == 1) {
		ret = enter_ips(adapter, macid);
	} else {
		ret = leave_ips(adapter, macid);
	}

	return ret;
}

u32 mac_chk_leave_ips(struct mac_ax_adapter *adapter, u8 macid)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg_sleep = 0;
	u8 macid_grp = macid >> MACID_GRP_SH;
	u8 macid_sh = macid & MACID_GRP_MASK;

	switch (macid_grp) {
	case MACID_GRP_0:
		reg_sleep = REG_MACID_SLEEP;
		break;
	case MACID_GRP_1:
		reg_sleep = REG_MACID_SLEEP1;
		break;
	case MACID_GRP_2:
		reg_sleep = REG_MACID_SLEEP2;
		break;
	case MACID_GRP_3:
		reg_sleep = REG_MACID_SLEEP3;
		break;
	default:
		return MACPSSTATFAIL;
	}

	// Bypass Tx pause check during STOP SER period
	if (MAC_REG_R32(reg_sleep) & BIT(macid_sh)) {
		return MACPSSTATFAIL;
	}

	return MACSUCCESS;
}

u8 _is_in_ips(struct mac_ax_adapter *adapter)
{
	u8 i;

	for (i = 0; i < 4; i++) {
		if (ips_status[i] != 0) {
			return 1;
		}
	}

	return 0;
}

u32 mac_ps_notify_wake(struct mac_ax_adapter *adapter)
{
	struct ps_rpwm_parm parm;

	PLTFM_MEMSET(&parm, 0, sizeof(struct ps_rpwm_parm));

	if (adapter->mac_pwr_info.pwr_in_lps == 0) {
		PLTFM_MSG_ERR("%s: Not in power saving!\n", __func__);
		return MACPWRSTAT;
	}

	parm.notify_wake = 1;
	send_rpwm(adapter, &parm);

	return MACSUCCESS;
}

u32 mac_ps_set_32k(struct mac_ax_adapter *adapter, bool en_32k, bool en_ack)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u16 rpwm_value = 0;
	u8 toggle = 0;
	int trycnt = 10000;

	if (adapter->mac_pwr_info.pwr_in_lps == 0) {
		PHL_TRACE(COMP_PHL_PS, _PHL_ERR_, "[HALPS], %s(): Not in power saving!\n", __func__);
		return MACPWRSTAT;
	}
	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[HALPS], %s(): en_32k %d, "
		  "en_ack %d!\n", __func__, en_32k, en_ack);

	/* 8730e mp chip: BIT0(clock bit) ,BIT4(PG bit), BIT6(Ack bit), and BIT15(Toggle bit) in REG_RPWM.
	   BIT0 value - 1: 32k, 0:40MHz.
	   BIT4 value - partial off.
	   BIT6 value - 1: report cpwm value after success set, 0:do not report.
	   BIT15 value - Toggle bit change.
	*/
	rpwm_value = MAC_REG_R16(REG_RPWM);
	rpwm_value &= (BIT_RPWM_TOGGLING | PS_RPWM_ACK \
		      | PS_RPWM_32K_EN | PS_RPWM_PARTIAL_OFF_EN);

	/* Toggle bit15 */
	rpwm_value += BIT_RPWM_TOGGLING;

	/* set 32k bit */
	if (en_32k == true) {
		rpwm_value |= (PS_RPWM_32K_EN | PS_RPWM_PARTIAL_OFF_EN);
	} else {
		rpwm_value &= ~(PS_RPWM_32K_EN | PS_RPWM_PARTIAL_OFF_EN);
	}

	/* set ACK bit */
	if (en_ack == true) {
		rpwm_value |= PS_RPWM_ACK;
	} else {
		rpwm_value &= ~PS_RPWM_ACK;
	}

	/*clear cpwm intterupt */
	MAC_REG_W32(REG_HISR0, BIT_HCPWM_INT);
	MAC_REG_W16(REG_RPWM, rpwm_value);

	/*wait ack*/
	do {
		if (MAC_REG_R32(REG_HISR0) & BIT_HCPWM_INT) {
			break;
		}
		PLTFM_DELAY_US(1);
	} while (en_ack && (--trycnt > 1));

	if (trycnt <= 1) {
		PHL_TRACE(COMP_PHL_PS, _PHL_ERR_, "[HALPS], %s(): trycnt: %d\n", __func__, trycnt);
		return MACPOLLTO;
	}

	return MACSUCCESS;
}

u32 mac_ps_store_axi_regs(struct mac_ax_adapter *adapter, bool store)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	static u32 *pdata;
	static u16 reg300;
	const u16 store_sniff_ranges[][2] = {{0x308, 0x344},
					     {0x380, 0x39c},
					     {0x3a0, 0x3d8}};
	u16 addr;
	u8 mem_size = 0;
	u8 i, j;
	u8 size;

	size = sizeof(store_sniff_ranges) / sizeof(u16) / 2;
	for (i = 0; i < size; i++) {
		mem_size += store_sniff_ranges[i][1] - store_sniff_ranges[i][0];
	}

	if (store) {
		//store
		pdata = (u32 *) PLTFM_MALLOC(mem_size);
		if (pdata == NULL) {
			return MACNOBUF;
		}

		addr = 0x300;
		reg300 = MAC_REG_R16(REG_AXI_CTRL);
		for (i = 0; i < (mem_size / 4);) {
			for (j = 0; j < size; j++) {
				if (addr  >= store_sniff_ranges[j][0] && addr < store_sniff_ranges[j][1]) {
					*(pdata + i) = MAC_REG_R32(addr);
					i++;
					break;
				}
			}
			addr += 4;
		}
	} else {
		//restore
		if (pdata == NULL) {
			return MACNOBUF;
		}

		//for sleep pg whe should restore page 3,for sleep CG,just return
		if (MAC_REG_R16(REG_BCNQ_TXBD_DESA) != 0x00) { //compare with default value
			PLTFM_FREE(pdata, mem_size);
			pdata = NULL;
			return MACSUCCESS;
		}

		//pause dma
		MAC_REG_W16(REG_AXI_CTRL, 0xffff);
		addr = 0x300;

		for (i = 0; i < (mem_size / 4);) {
			for (j = 0; j < size; j++) {
				if (addr  >= store_sniff_ranges[j][0] && addr < store_sniff_ranges[j][1]) {
					MAC_REG_W32(addr, *(pdata + i));
					i++;
					break;
				}
			}
			addr += 4;
		}

		MAC_REG_W16(REG_AXI_CTRL, reg300);
		PLTFM_FREE(pdata, mem_size);
		pdata = NULL;
	}

	return MACSUCCESS;
}

u32 mac_cfg_ps_advance_parm(struct mac_ax_adapter *adapter,
			    struct mac_ax_ps_adv_parm *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

static u32 send_h2c_pw_cfg(struct mac_ax_adapter *adapter,
			   struct periodic_wake_cfg *parm)
{
	printk("%s: H2C Packet not supported.", __FUNCTION__);
	return 0;
}

u32 mac_periodic_wake_cfg(struct mac_ax_adapter *adapter,
			  struct mac_ax_periodic_wake_info pw_info)
{
	u32 ret = MACSUCCESS;
	struct periodic_wake_cfg parm;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		return MACNOFW;
	}

	PLTFM_MEMSET(&parm, 0, sizeof(struct periodic_wake_cfg));
	parm.macid = pw_info.macid;
	parm.enable = pw_info.enable;
	parm.band = pw_info.band;
	parm.port = pw_info.port;
	parm.wake_period = pw_info.wake_period;
	parm.wake_duration = pw_info.wake_duration;

	PLTFM_MSG_ALWAYS("%s: macid(%d), enable(%d), band(%d), port(%d)\n",
			 __func__, parm.macid, parm.enable, parm.band, parm.port);
	PLTFM_MSG_ALWAYS("%s: wake_period(%d), wake_duration(%d)\n",
			 __func__, parm.wake_period, parm.wake_duration);

	ret = send_h2c_pw_cfg(adapter, &parm);
	if (ret) {
		return ret;
	}

	return ret;
}
