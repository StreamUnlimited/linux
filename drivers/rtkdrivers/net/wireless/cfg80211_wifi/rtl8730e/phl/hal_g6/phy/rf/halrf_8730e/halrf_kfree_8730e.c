/******************************************************************************
 *
 * Copyright(c) 2021 Realtek Corporation.
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
 *****************************************************************************/
#include "../halrf_precomp.h"

#ifdef RF_8730E_SUPPORT

void _halrf_get_total_efuse_8730e(struct rf_info *rf,
				  enum phl_phy_idx phy)
{
	struct halrf_kfree_info *kfree = &rf->kfree_info;
	u8 i;

	halrf_phy_efuse_get_info(rf, HIDE_EFUSE_START_ADDR_8730E,
				 HIDE_EFUSE_SIZE_8730E, kfree->efuse_content);

	for (i = 0; i < HIDE_EFUSE_SIZE_8730E; i++)
		RF_DBG(rf, DBG_RF_TSSI_TRIM, "======> %s   efuse_con[0x%x] = 0x%x\n",
		       __func__, i + HIDE_EFUSE_START_ADDR_8730E, kfree->efuse_content[i]);
}

void _halrf_get_1byte_efuse_8730e(struct rf_info *rf, u32 addr, u8 *value)
{
	struct halrf_kfree_info *kfree = &rf->kfree_info;

	if (addr < HIDE_EFUSE_START_ADDR_8730E || addr > HIDE_EFUSE_END_ADDR_8730E) {
		RF_DBG(rf, DBG_RF_TSSI_TRIM, "===> %s addr(0x%x) < 0x%x(Min), > 0x%x(Max) Over Range Return !!!\n",
		       __func__, addr, HIDE_EFUSE_START_ADDR_8730E, HIDE_EFUSE_END_ADDR_8730E);
		return;
	}

	*value = kfree->efuse_content[addr - HIDE_EFUSE_START_ADDR_8730E];
}

s8 _halrf_efuse_exchange_8730e(struct rf_info *rf, u8 value, u8 mask)
{
	s8 tmp = 0;

	if (mask == LOW_MASK) {
		tmp = value & 0xf;

		if (tmp & BIT(3)) {
			tmp = tmp | 0xf0;
		}
	} else {
		tmp = (value & 0xf0) >> 4;

		if (tmp & BIT(3)) {
			tmp = tmp | 0xf0;
		}
	}

	return tmp;
}

void _halrf_set_thermal_trim_8730e(struct rf_info *rf,
				   enum phl_phy_idx phy)
{
	u8 thermal_a, offset_lsb;

	RF_DBG(rf, DBG_RF_THER_TRIM, "======> %s   phy=%d\n", __func__, phy);

	if (!(rf->support_ability & HAL_RF_THER_TRIM)) {
		RF_DBG(rf, DBG_RF_THER_TRIM, "<== %s phy=%d support_ability=%d Ther Trim Off!!!\n",
		       __func__, phy, rf->support_ability);
		return;
	}

	_halrf_get_1byte_efuse_8730e(rf, THERMAL_TRIM_HIDE_EFUSE_A_8730E, &thermal_a);

	RF_DBG(rf, DBG_RF_THER_TRIM, "efuse Ther_A=0x%x\n", thermal_a);

	if (thermal_a == 0xff) {
		RF_DBG(rf, DBG_RF_THER_TRIM, "Ther_A=0xff no PG Return!!!\n");
		return;
	}

	offset_lsb = (thermal_a & 0x3) | (thermal_a & BIT(7)); //[1:0]
	thermal_a = (thermal_a & 0xfc) >> 2;

	RF_DBG(rf, DBG_RF_THER_TRIM, "Ther_A=0x%x, ther_ofst_lsb=0x%x\n", thermal_a, offset_lsb);
	halrf_wrf(rf, RF_PATH_A, 0x42, 0x3f000, thermal_a);
	rf->ther_ofst_lsb = offset_lsb;
}

void _halrf_set_pa_bias_trim_8730e(struct rf_info *rf,
				   enum phl_phy_idx phy)
{
	u8 value_tmp, pa_bias_a;
	s8 pa_bias_a_2g, pa_bias_a_5g;

	RF_DBG(rf, DBG_RF_PABIAS_TRIM, "======> %s   phy=%d\n", __func__, phy);

	if (!(rf->support_ability & HAL_RF_PABIAS_TRIM)) {
		RF_DBG(rf, DBG_RF_PABIAS_TRIM, "<== %s phy=%d support_ability=%d PA Bias K Off!!!\n",
		       __func__, phy, rf->support_ability);
		return;
	}

	_halrf_get_1byte_efuse_8730e(rf, PABIAS_TRIM_HIDE_EFUSE_A_8730E, &value_tmp);

	RF_DBG(rf, DBG_RF_PABIAS_TRIM, "efuse PA_Bias_A=0x%x\n", value_tmp);

	if (value_tmp == 0xff) {
		RF_DBG(rf, DBG_RF_PABIAS_TRIM, "PA_Bias_A=0xff no PG Return!!!\n");
		return;
	}

	pa_bias_a = value_tmp & 0xf;

	if (pa_bias_a & BIT(3)) {
		pa_bias_a_2g = (-1 * (pa_bias_a & 0x7));
	} else {
		pa_bias_a_2g = pa_bias_a & 0x7;
	}


	pa_bias_a = (value_tmp & 0xf0) >> 4;

	if (pa_bias_a & BIT(3)) {
		pa_bias_a_5g = (-1 * (pa_bias_a & 0x7));
	} else {
		pa_bias_a_5g = pa_bias_a & 0x7;
	}

	RF_DBG(rf, DBG_RF_PABIAS_TRIM, "After Calculate PA_Bias_A_2G=0x%x PA_Bias_A_5G=0x%x\n",
	       pa_bias_a_2g, pa_bias_a_5g);

	halrf_wrf(rf, RF_PATH_A, 0x60, 0x0000000f, pa_bias_a_2g); //0x60[3:0] TXG_OFS_FT_PA_IB
	halrf_wrf(rf, RF_PATH_A, 0x60, 0x00000f00, pa_bias_a_5g); //0x60[11:8] TXA_OFS_FT_PA_IB[3:0]
}

void _halrf_get_tssi_trim_8730e(struct rf_info *rf,
				enum phl_phy_idx phy)
{
	struct halrf_tssi_info *tssi = &rf->tssi;
	u8 i, j, check_tmp = 0;

	RF_DBG(rf, DBG_RF_TSSI_TRIM, "======> %s   phy=%d\n", __func__, phy);

	if (!(rf->support_ability & HAL_RF_TSSI_TRIM)) {
		RF_DBG(rf, DBG_RF_TSSI_TRIM, "<== %s phy=%d support_ability=%d TSSI Trim Off!!!\n",
		       __func__, phy, rf->support_ability);
		return;
	}

	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_2GL_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][0]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_2GH_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][1]);

	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GL1_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][2]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GL2_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][3]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GM1_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][4]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GM2_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][5]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GH1_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][6]);
	_halrf_get_1byte_efuse_8730e(rf, TSSI_TRIM_HIDE_EFUSE_5GH2_A_8730E,
				     (u8 *)&tssi->tssi_trim[RF_PATH_A][7]);

	for (i = 0; i < 1; i++) {
		for (j = 0; j < TSSI_HIDE_EFUSE_NUM; j++) {
			RF_DBG(rf, DBG_RF_TSSI_TRIM, "tssi->tssi_trim[%d][%d]=0x%x\n", i, j, tssi->tssi_trim[i][j]);
			if ((tssi->tssi_trim[i][j] & 0xff) == 0xff) {
				check_tmp++;
			}
		}
	}

	RF_DBG(rf, DBG_RF_TSSI_TRIM, "check_tmp=%d\n", check_tmp);

	if (check_tmp == 2 * TSSI_HIDE_EFUSE_NUM) {
		for (i = 0; i < 1; i++) {
			for (j = 0; j < TSSI_HIDE_EFUSE_NUM; j++) {
				tssi->tssi_trim[i][j] = 0;
			}
		}

		RF_DBG(rf, DBG_RF_TSSI_TRIM, "TSSI Trim no PG tssi->tssi_trim=0x0\n");
	}

}

void halrf_get_efuse_trim_8730e(struct rf_info *rf,
				enum phl_phy_idx phy)
{
	_halrf_get_total_efuse_8730e(rf, phy);
	_halrf_set_thermal_trim_8730e(rf, phy);
	_halrf_set_pa_bias_trim_8730e(rf, phy);
	_halrf_get_tssi_trim_8730e(rf, phy);
}

#endif	/*RF_8730E_SUPPORT*/
