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
#include "cmac_tx_8730e.h"
#include "../cmac_tx.h"
#include "../../mac_reg.h"
#if MAC_AX_8730E_SUPPORT

#define PTCL_TXQ_TB		16

#define TX_EN_CTRL_ALL		0xFFFFFFFF


u32 stop_sch_tx_8730e(struct mac_ax_adapter *adapter, enum sch_tx_sel sel,
		      struct mac_ax_sch_tx_en_cfg *bak)
{
	struct mac_ax_sch_tx_en_cfg cfg;
	u32 ret;

	ret = get_hw_sch_tx_en(adapter, bak);
	if (ret != MACSUCCESS) {
		return ret;
	}

	cfg.band = bak->band;
	u32_2_sch(adapter, &cfg.tx_en_mask, 0);

	switch (sel) {
	case SCH_TX_SEL_ALL:
		u32_2_sch(adapter, &cfg.tx_en, TX_EN_CTRL_ALL);
		u32_2_sch(adapter, &cfg.tx_en_mask, TX_EN_CTRL_ALL);
		ret = set_hw_sch_tx_en(adapter, &cfg);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case SCH_TX_SEL_HIQ:
		cfg.tx_en.hiq = 1;
		cfg.tx_en_mask.hiq = 1;
		ret = set_hw_sch_tx_en(adapter, &cfg);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	case SCH_TX_SEL_MG0:
		cfg.tx_en.mgnt = 1;
		cfg.tx_en_mask.mgnt = 1;
		ret = set_hw_sch_tx_en(adapter, &cfg);
		if (ret != MACSUCCESS) {
			return ret;
		}
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

#endif /* #if MAC_AX_8730E_SUPPORT */
