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
#include "halbb_precomp.h"
#ifdef HALBB_DBCC_SUPPORT

enum bb_path halbb_get_cur_phy_valid_path(struct bb_info *bb)
{
	enum bb_path valid_path = BB_PATH_AB;

	switch (bb->ic_type) {

	case BB_RTL8852A:
	case BB_RTL8852C:
		if (bb->hal_com->dbcc_en) {
			if (bb->bb_phy_idx == HW_PHY_1) {
				valid_path = BB_PATH_B;
			} else {
				valid_path = BB_PATH_A;
			}
		} else {
			valid_path = BB_PATH_AB;
		}

		break;

	default:
		break;
	}
	return valid_path;
}

bool halbb_ctrl_dbcc(struct bb_info *bb, bool dbcc_enable)
{
	struct bb_dbcc_info	*dbcc = &bb->bb_dbcc_i;

	if (!dbcc->ic_dbcc_support) {
		return false;
	}

	switch (bb->ic_type) {

#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_ctrl_dbcc_8852a_2(bb, dbcc_enable);
		break;
#endif
#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_ctrl_dbcc_8852c(bb, dbcc_enable);
		break;
#endif
	default:
		break;
	}
	return true;
}

bool halbb_cfg_dbcc_cck_phy_map(struct bb_info *bb, enum phl_phy_idx cck_phy_map)
{
	struct bb_dbcc_info	*dbcc = &bb->bb_dbcc_i;

	if (!dbcc->ic_dbcc_support) {
		return false;
	}

	BB_DBG(bb, DBG_DBCC, "[%s] cck_phy_map=%d\n", __func__, cck_phy_map);

	switch (bb->ic_type) {

#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_cfg_dbcc_cck_phy_map_8852a_2(bb, bb->hal_com->dbcc_en, cck_phy_map);
		break;
#endif
#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_cfg_dbcc_cck_phy_map_8852c(bb, bb->hal_com->dbcc_en, cck_phy_map);
		break;
#endif

	default:
		break;
	}
	return true;
}

bool halbb_cfg_dbcc(struct bb_info *bb, struct bb_dbcc_cfg_info *cfg)
{
	struct bb_dbcc_info	*dbcc = &bb->bb_dbcc_i;
	u32 val = 0;

	if (!dbcc->ic_dbcc_support) {
		return false;
	}

	BB_DBG(bb, DBG_DBCC, "[%s] en=%d, cck_phy_map=%d\n", __func__,
	       cfg->dbcc_en, cfg->cck_phy_map);

	switch (bb->ic_type) {

#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_cfg_dbcc_8852a_2(bb, cfg);
		break;
#endif
#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_cfg_dbcc_8852c(bb, cfg);
		break;
#endif
	default:
		break;
	}

	if (cfg->dbcc_en) {
		halbb_dm_init_phy1(bb);
		halbb_pause_func(bb, F_CFO_TRK, HALBB_PAUSE_NO_SET, HALBB_PAUSE_LV_2, 1, &val, bb->bb_phy_idx);
	} else {
		halbb_pause_func(bb, F_CFO_TRK, HALBB_RESUME_NO_RECOVERY, HALBB_PAUSE_LV_2, 1, &val, bb->bb_phy_idx);
	}

	return true;
}

struct bb_info *halbb_get_curr_bb_pointer(struct bb_info *bb,
		enum phl_phy_idx phy_idx)
{
	if (bb->phl_com->dev_cap.dbcc_sup && phy_idx != bb->bb_phy_idx && bb->bb_phy_hooker) {
		BB_DBG(bb, DBG_DBCC, "[CVRT][en:%d] phy_idx (%d) -> (%d)\n",
		       bb->phl_com->dev_cap.dbcc_sup, bb->bb_phy_idx, phy_idx);
		return bb->bb_phy_hooker;
	} else {
		BB_DBG(bb, DBG_DBCC, "[STAY][en:%d] phy_idx (%d) -> (%d)\n",
		       bb->phl_com->dev_cap.dbcc_sup, bb->bb_phy_idx, phy_idx);
		return bb;
	}
}

u32
halbb_buffer_init_phy1(struct bb_info *bb_0)
{
	struct bb_info *bb_1 = NULL;

	BB_DBG(bb_0, DBG_INIT, "[%s]\n", __func__);

	if (!bb_0) {
		BB_WARNING("[%s]*bb_phy_0 = NULL\n", __func__);
		return RTW_HAL_STATUS_BB_INIT_FAILURE;
	}

	bb_1 = halbb_mem_alloc(bb_0, sizeof(struct bb_info));

	if (!bb_1) {
		BB_WARNING("[%s]*bb_phy_1 = NULL\n", __func__);
		return RTW_HAL_STATUS_BB_INIT_FAILURE;
	}
	/*Hook Link*/
	bb_0->bb_phy_hooker = bb_1;
	bb_1->bb_phy_hooker = bb_0;

	bb_1->bb_phy_idx = HW_PHY_1;
	bb_1->bb_cmn_hooker = bb_0->bb_cmn_hooker;

	bb_1->phl_com = bb_0->phl_com;/*shared memory for all components*/
	bb_1->hal_com = bb_0->hal_com;/*shared memory for phl and hal*/

	halbb_hw_init(bb_1);

#if 0
	BB_DBG(bb_0, DBG_DBCC, "phy_idx[0,1]={%d, %d}\n", bb_0->bb_phy_idx, bb_1->bb_phy_idx);
	BB_DBG(bb_1, DBG_DBCC, "phy_idx[0,1]={%d, %d}\n", bb_0->bb_phy_idx, bb_1->bb_phy_idx);

	BB_DBG(bb_0, DBG_DBCC, "phy_idx[0,1]={%d, %d}\n", bb_1->bb_phy_hooker->bb_phy_idx, bb_0->bb_phy_hooker->bb_phy_idx);
	BB_DBG(bb_1, DBG_DBCC, "phy_idx[0,1]={%d, %d}\n", bb_1->bb_phy_hooker->bb_phy_idx, bb_0->bb_phy_hooker->bb_phy_idx);
#endif

	return RTW_HAL_STATUS_SUCCESS;
}

void halbb_dbcc_early_init(struct bb_info *bb)
{
	struct bb_dbcc_info	*dbcc = &bb->bb_dbcc_i;

	if (bb->ic_type & BB_IC_DBCC) {
		dbcc->ic_dbcc_support = true;
	} else {
		dbcc->ic_dbcc_support = false;
	}

	BB_DBG(bb, DBG_DBCC, "IC_dbcc_support=%d\n", dbcc->ic_dbcc_support);
}


void halbb_dbcc_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		    char *output, u32 *_out_len)
{
	struct bb_dbcc_cfg_info cfg;
	struct bb_info *bb_out;
	u32 val[10] = {0};
	u16 i = 0;

	if (_os_strcmp(input[1], "-h") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "phy {0/1}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "init\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "cck_map {dbcc_en} {cck_phy_map}\n");
		return;
	}

	if (_os_strcmp(input[1], "phy") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		if (val[0] == 1) {
			bb->bb_cmn_hooker->bb_echo_cmd_i.echo_phy_idx = HW_PHY_1;
		} else {
			bb->bb_cmn_hooker->bb_echo_cmd_i.echo_phy_idx = HW_PHY_0;
		}

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "echo cmd convert to phy-%d mode\n",
			    bb->bb_cmn_hooker->bb_echo_cmd_i.echo_phy_idx);

	} else if (_os_strcmp(input[1], "cck_map") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);

		cfg.dbcc_en = (bool)val[0];
		cfg.cck_phy_map = (enum phl_phy_idx)val[1];

		halbb_cfg_dbcc(bb, &cfg);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "dbcc_en=%d, cck_map=%d\n", cfg.dbcc_en, cfg.cck_phy_map);
#if 0
	} else if (_os_strcmp(input[1], "test1") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "[TEST_1]\n");

		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);

		BB_DBG(bb, DBG_DBCC, "[IN]bb->bb_phy_idx=%d, echo_phy_idx=%d\n",
		       bb->bb_phy_idx,
		       bb->bb_cmn_hooker->bb_echo_cmd_i.echo_phy_idx);

		HALBB_GET_PHY_PTR(bb, bb_out, (enum phl_phy_idx)val[0]);

		if (bb_out) {
			BB_DBG(bb, DBG_DBCC, "[OUT]bb_out->bb_phy_idx=%d, echo_phy_idx=%d\n",
			       bb_out->bb_phy_idx,
			       bb_out->bb_cmn_hooker->bb_echo_cmd_i.echo_phy_idx);
		} else {
			BB_DBG(bb, DBG_DBCC, "bb_out=NULL\n");
		}
#endif
	} else if (_os_strcmp(input[1], "test2") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "[TEST_2][DBG]phy_idx={%d}\n", bb->bb_phy_idx);

		BB_DBG(bb, DBG_DBCC, "[IN]phy_idx={%d}\n", bb->bb_phy_idx);
		BB_DBG(bb, DBG_DBCC, "phy0->phy1\n");
		HALBB_GET_PHY_PTR(bb, bb_out, HW_PHY_1);
		BB_DBG(bb, DBG_DBCC, "[OUT]phy_idx={%d}\n", bb_out->bb_phy_idx);

		bb = bb_out;
		BB_DBG(bb, DBG_DBCC, "[IN]phy_idx={%d}\n", bb->bb_phy_idx);
		BB_DBG(bb, DBG_DBCC, "phy1->phy1\n");
		HALBB_GET_PHY_PTR(bb, bb_out, HW_PHY_1); /*phy1->phy1*/
		BB_DBG(bb, DBG_DBCC, "[OUT]phy_idx={%d}\n", bb_out->bb_phy_idx);

		bb = bb_out;
		BB_DBG(bb, DBG_DBCC, "[IN]phy_idx={%d}\n", bb->bb_phy_idx);
		BB_DBG(bb, DBG_DBCC, "phy1->phy0\n");
		HALBB_GET_PHY_PTR(bb, bb_out, HW_PHY_0); /*phy1->phy0*/
		BB_DBG(bb, DBG_DBCC, "[OUT]phy_idx={%d}\n", bb_out->bb_phy_idx);

		bb = bb_out;
		BB_DBG(bb, DBG_DBCC, "[IN]phy_idx={%d}\n", bb->bb_phy_idx);
		BB_DBG(bb, DBG_DBCC, "phy0->phy0\n");
		HALBB_GET_PHY_PTR(bb, bb_out, HW_PHY_0); /*phy0->phy0*/
		BB_DBG(bb, DBG_DBCC, "[OUT]phy_idx={%d}\n", bb_out->bb_phy_idx);

		bb = bb_out;
		BB_DBG(bb, DBG_DBCC, "[IN]phy_idx={%d}\n", bb->bb_phy_idx);
		BB_DBG(bb, DBG_DBCC, "phy0->phy1\n");
		HALBB_GET_PHY_PTR(bb, bb_out, HW_PHY_1); /*phy0->phy1*/
		BB_DBG(bb, DBG_DBCC, "[OUT]phy_idx={%d}\n", bb_out->bb_phy_idx);
	} else if (_os_strcmp(input[1], "test3") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "[TEST_3][DBG]phy_idx={%d}\n", bb->bb_phy_idx);

		BB_DBG(bb, DBG_DBCC, "phy0->phy1\n");
		bb = halbb_get_curr_bb_pointer(bb, HW_PHY_1); /*phy0->phy1*/
		BB_DBG(bb, DBG_DBCC, "phy1->phy1\n");
		bb = halbb_get_curr_bb_pointer(bb, HW_PHY_1); /*phy1->phy1*/
		BB_DBG(bb, DBG_DBCC, "phy1->phy0\n");
		bb = halbb_get_curr_bb_pointer(bb, HW_PHY_0); /*phy1->phy0*/
		BB_DBG(bb, DBG_DBCC, "phy0->phy0\n");
		bb = halbb_get_curr_bb_pointer(bb, HW_PHY_0); /*phy0->phy0*/
		BB_DBG(bb, DBG_DBCC, "phy0->phy1\n");
		bb = halbb_get_curr_bb_pointer(bb, HW_PHY_1); /*phy0->phy1*/
	}
}

#endif



