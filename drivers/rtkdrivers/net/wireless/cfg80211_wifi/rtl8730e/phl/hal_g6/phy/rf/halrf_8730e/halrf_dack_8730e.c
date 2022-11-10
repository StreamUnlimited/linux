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
#include "mac/mac_def.h"
#ifdef RF_8730E_SUPPORT

#define t_avg 100


void halrf_addck_8730e(struct rf_info *rf)
{
	struct halrf_dack_info *dack = &rf->dack;
	u8	i = 0;
	u32	offset_val;
	s32	val_avg_I = 0, val_avg_Q = 0, offset_val_I[16], offset_val_Q[16];
	bool	do_pass = true;
	u32	bb_20F0;


	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK start!!!\n");

	bb_20F0 = halrf_rreg(rf, 0x120F0, MASKDWORD);
	halrf_wreg(rf, 0x112b8, BIT(30), 0x1);
	halrf_wreg(rf, 0x1030c, 0xFF000000, 0x07);
	halrf_wreg(rf, 0x1032c, BIT(16), 0x0);
	halrf_wreg(rf, 0x1180c, BIT(30), 0x1);
	halrf_wreg(rf, 0x112dc, BIT(15), 0x1);
	halrf_wreg(rf, 0x120F0, 0x00FFFFFF, 0x010289);

	for (i = 0; i < 16; i++) {
		offset_val = halrf_rreg(rf, 0x11730, MASKDWORD);
		offset_val_Q[i] = offset_val & 0x000003FF;
		offset_val_I[i] = (offset_val & 0x000FFC00) >> 10;

		//	RF_DBG(rf, DBG_RF_RXDCK, "[AFE DCK][before] read 1730 = %x, [19:10] = %x, [9:0] = %x\n",
		//		offset_val, offset_val_I[i], offset_val_Q[i]);
		if (offset_val_I[i] & BIT(9)) {
			offset_val_I[i] = offset_val_I[i] - 1024;
}
		if (offset_val_Q[i] & BIT(9)) {
			offset_val_Q[i] = offset_val_Q[i] - 1024;
		}
		//	RF_DBG(rf, DBG_RF_RXDCK, "[AFE DCK]offset_val_I[%d] = %d, offset_val_Q[%d] = %d\n",
		//		i, offset_val_I[i], i, offset_val_Q[i]);
		val_avg_I += offset_val_I[i];
		val_avg_Q += offset_val_Q[i];
}

	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK sum val_avg_I = %d, val_avg_Q = %d\n",
	       val_avg_I, val_avg_Q);

	val_avg_I = (-val_avg_I / 16) & 0x3FF;
	val_avg_Q = (-val_avg_Q / 16) & 0x3FF;

	if (val_avg_I & BIT(9)) {
		val_avg_I = val_avg_I - 1024;
	}
	if (val_avg_Q & BIT(9)) {
		val_avg_Q = val_avg_Q - 1024;
}

	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK divid val_avg_I = %d, val_avg_Q = %d\n",
	       val_avg_I, val_avg_Q);

	halrf_wreg(rf, 0x145fc, 0x00FFC000, val_avg_I);
	halrf_wreg(rf, 0x145fc, 0x000003FF, val_avg_Q);

	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK BB 0x145fc = 0x%x\n",
	       halrf_rreg(rf, 0x145fc, MASKDWORD));

	halrf_wreg(rf, 0x145f4, BIT(28), 0x1);
	halrf_wreg(rf, 0x120F0, 0x00FFFFFF, 0x010283);
	val_avg_I = 0;
	val_avg_Q = 0;
	for (i = 0; i < 16; i++) {
		offset_val = halrf_rreg(rf, 0x11730, MASKDWORD);
		offset_val_Q[i] = offset_val & 0x000003FF;
		offset_val_I[i] = (offset_val & 0x000FFC00) >> 10;

		//RF_DBG(rf, DBG_RF_DACK, "[AFE DCK][check] read 1730[19:10] = %x, [9:0] = %x\n",
		//	offset_val_I[i], offset_val_Q[i]);
		if (offset_val_I[i] & BIT(9)) {
			offset_val_I[i] = offset_val_I[i] - 1024;
}
		if (offset_val_Q[i] & BIT(9)) {
			offset_val_Q[i] = offset_val_Q[i] - 1024;
	}
		//RF_DBG(rf, DBG_RF_DACK, "[AFE DCK] offset_val_I[%d] = %d, offset_val_Q[%d] = %d\n",
		//	i, offset_val_I[i], i, offset_val_Q[i]);
		val_avg_I += offset_val_I[i];
		val_avg_Q += offset_val_Q[i];
	}
	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK sum val_avg_I = %d, val_avg_Q = %d\n",
	       val_avg_I, val_avg_Q);
	val_avg_I = (val_avg_I / 16) & 0x3FF;
	val_avg_Q = (val_avg_Q / 16) & 0x3FF;
	if (val_avg_I & BIT(9)) {
		val_avg_I = val_avg_I - 1024;
	}
	if (val_avg_Q & BIT(9)) {
		val_avg_Q = val_avg_Q - 1024;
}

	RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK divid val_avg_I = %d, val_avg_Q = %d\n",
	       val_avg_I, val_avg_Q);

	if ((val_avg_I > 14) || val_avg_I < -14) {
		do_pass = false;
}
	if ((val_avg_Q > 14) || val_avg_Q < -14) {
		do_pass = false;
	}
	if (do_pass) {
		RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK pass!\n");
		} else {
		RF_DBG(rf, DBG_RF_DACK, "[DACK]ADC DCK fail!\n");
		}

	//halrf_wreg(rf, 0x120F0, 0x00FFFFFF, 0x000000);
	halrf_wreg(rf, 0x112dc, BIT(15), 0x0);
	halrf_wreg(rf, 0x1180c, BIT(30), 0x0);
	halrf_wreg(rf, 0x1030c, 0xFF000000, 0x07);
	halrf_wreg(rf, 0x1032c, BIT(16), 0x1);
	halrf_wreg(rf, 0x112b8, BIT(30), 0x0);
	halrf_wreg(rf, 0x120F0, MASKDWORD, bb_20F0);
	halrf_wreg(rf, 0x14600, 0x000003FF, 0x0);
	}

void _halrf_dack_backup_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)
{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		reg_backup[i] = halrf_rreg(rf, reg[i], MASKDWORD);

//		RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup BB 0x%08x = 0x%08x\n",
//			reg[i], reg_backup[i]);

		}
	}

void _halrf_dack_reload_bb_registers_8730e(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u32 reg_num)

{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

//		RF_DBG(rf, DBG_RF_DACK, "[DACK] Reload BB 0x%08x = 0x%08x\n",
//			reg[i], reg_backup[i]);

	}
}

void halrf_dack_8730e_s0(struct rf_info *rf)
{
	struct halrf_dack_info *dack = &rf->dack;
	u32	bb_reg[7] = {0x112E0, 0x112E4, 0x112dc, 0x112b8,
			     0x1030c, 0x112a0, 0x1129c
			};
	u32	reg_backup[7];
	u32	sys_2c, sys_30, sys_34, sys_38, sys_3c, sys_result, val32;
	u32	reg_1E58, dck_dos_I, dck_dos_Q, dck_kosen_I, dck_kosen_Q;
	u8	i = 0;
	_halrf_dack_backup_bb_registers_8730e(rf, bb_reg, reg_backup, 7);
	/*reset to 0*/
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 & 0xFFFFFFF3);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, val32 & 0x1FFFFFFF);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3, val32 & 0xFFFFFFE0);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1, val32 & 0xF03FFFFF);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, val32 & 0xF000FFC0);
	/*backup sys reg*/
	sys_2c = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	sys_30 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1);
	sys_34 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2);
	sys_38 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3);
	//sys_3c = WLAFE_BASE->WLAFE_ANAPAR_DCK_BT;
	RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup Sys_reg 002c = 0x%08x\n", sys_2c);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup Sys_reg 0030 = 0x%08x\n", sys_30);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup Sys_reg 0034 = 0x%08x\n", sys_34);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup Sys_reg 0038 = 0x%08x\n", sys_38);
	//RF_DBG(rf, DBG_RF_DACK, "[DACK] Backup Sys_reg 003c = 0x%08x\n", sys_3c);
	//	halrf_dack_backup_s0_8720e(rf);
	/*step1 load 01 BB reg*/
	halrf_wreg(rf, 0x112B8, BIT(30), 0x1);
	halrf_wreg(rf, 0x1030C, 0x0F000000, 0xF);
	halrf_wreg(rf, 0x112A0, BIT(19), 0x1);
	halrf_wreg(rf, 0x1129C, BIT(9), 0x0);
	halrf_wreg(rf, 0x112A0, 0x70000, 0x3);
	halrf_wreg(rf, 0x112A0, BIT(15), 0x1);
	halrf_wreg(rf, 0x112A0, 0x7000, 0x3);
	halrf_wreg(rf, 0x112DC, BIT(20), 0x0);
	halrf_wreg(rf, 0x112E4, BIT(20), 0x1);
	halrf_wreg(rf, 0x112E4, BIT(22), 0x0);
	halrf_wreg(rf, 0x112E4, BIT(19), 0x0);
	halrf_wreg(rf, 0x112DC, 0x1800, 0x0);
	halrf_wreg(rf, 0x112DC, BIT(0), 0x1);
	halrf_wreg(rf, 0x112E4, BIT(25), 0x0);
	halrf_wreg(rf, 0x112E4, BIT(26), 0x0);
	halrf_wreg(rf, 0x112E0, BIT(8), 0x1);
	halrf_wreg(rf, 0x112E0, BIT(9), 0x1);
	halrf_wreg(rf, 0x112DC, BIT(27), 0x1);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x112B8 = 0x%x\n", halrf_rreg(rf, 0x112B8, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x1030C = 0x%x\n", halrf_rreg(rf, 0x1030C, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x112A0 = 0x%x\n", halrf_rreg(rf, 0x112A0, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x1129C = 0x%x\n", halrf_rreg(rf, 0x1129C, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x112DC = 0x%x\n", halrf_rreg(rf, 0x112DC, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x112E0 = 0x%x\n", halrf_rreg(rf, 0x112E0, MASKDWORD));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] 0x112E4 = 0x%x\n", halrf_rreg(rf, 0x112E4, MASKDWORD));
	/*step2 load 02 SYS reg, base 0x4100C480*/
	//2C[30]=0
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, sys_2c & 0xBFFFFFFF);
	//2C[31]=1
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 | BIT(31));
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1,(sys_30 | 0x63E) & (~ BIT(0)));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 | 0x00C60000);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3, val32 | 0x00C60000);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, (val32 & 0xFFFF1FFF) | 0x0000C000);
	halrf_delay_ms(rf, 1);
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 & (~ BIT(2)));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 | BIT(2));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 & (~ BIT(3)));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 | BIT(3));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 & (~ BIT(4)));
	val32 = hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, val32 | BIT(4));

	RF_DBG(rf, DBG_RF_DACK, "[DACK] set Sys_reg 002c = 0x%08x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] set Sys_reg 0030 = 0x%08x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] set Sys_reg 0034 = 0x%08x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] set Sys_reg 0038 = 0x%08x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3));

	/*step3 check bb 1E58[1:0]= 0x3*/
	while ((halrf_rreg(rf, 0x11E58, 0x3) != 0x3) && (i < 30)) {
		halrf_delay_ms(rf, 1);
		i++;
	}
	//halrf_delay_ms(rf, 20);
	reg_1E58 = halrf_rreg(rf, 0x11E58, MASKDWORD);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] DACK result, 0x11E58 = 0x%x, delay %d ms\n",
	       reg_1E58, i);
	if (reg_1E58 & BIT(0)) {
		RF_DBG(rf, DBG_RF_DACK, "[DACK] Q DACK done!\n");
	} else {
		RF_DBG(rf, DBG_RF_DACK, "[DACK] Q DACK undone!\n");
}

	if (reg_1E58 & BIT(1)) {
		RF_DBG(rf, DBG_RF_DACK, "[DACK] I DACK done!\n");
	} else {
		RF_DBG(rf, DBG_RF_DACK, "[DACK] I DACK undone!\n");
	}

	dck_dos_I = (reg_1E58 & 0xFC) >> 2;
	dck_dos_Q = (reg_1E58 & 0x3F00) >> 8;
	dck_kosen_I = (reg_1E58 & 0xFC000) >> 14;
	dck_kosen_Q = (reg_1E58 & 0x03F00000) >> 20;

	RF_DBG(rf, DBG_RF_DACK, "[DACK] dos_I=%x,dos_Q=%x,kosen_I=%x,kosen_Q=%x\n",
	       dck_dos_I, dck_dos_Q, dck_kosen_I, dck_kosen_Q);

	_halrf_dack_reload_bb_registers_8730e(rf, bb_reg, reg_backup, 7);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, sys_2c);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1, sys_30);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, sys_34);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3, sys_38);
	//WLAFE_BASE->WLAFE_ANAPAR_DCK_BT = sys_3c;

	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_0, sys_2c | 0xC);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, sys_34 | 0xE0000000);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_3, sys_38 | 0x1F);

	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1, sys_30 | (dck_dos_I << 22));
	sys_result = sys_34 | 0xE0000000 | dck_dos_Q | (dck_kosen_I << 16) | (dck_kosen_Q << 22);
	hal_sys_write32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2, sys_result);
	RF_DBG(rf, DBG_RF_DACK, "[DACK] sys_30 = 0x%x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_1));
	RF_DBG(rf, DBG_RF_DACK, "[DACK] sys_34 = 0x%x\n",
		hal_sys_read32(rf->hal_com, SYSTEM_CTRL_BASE_LP, WLAFE_ANAPAR_DCK_2));
}

void halrf_dack_8730e(struct rf_info *rf)
{
	halrf_dack_8730e_s0(rf);
	//halrf_dack_8730e_s1(rf);
}

void halrf_dack_dump_8730e(struct rf_info *rf)
{
	struct halrf_dack_info *dack = &rf->dack;
	u8 i;
	u8 t;

	RF_DBG(rf, DBG_RF_DACK, "[DACK]S0 ADC_DCK ic = 0x%x, qc = 0x%x\n",
	       dack->addck_d[0][0], dack->addck_d[0][1]);
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S1 ADC_DCK ic = 0x%x, qc = 0x%x\n",
	       dack->addck_d[1][0], dack->addck_d[1][1]);
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S0 DAC_DCK ic = 0x%x, qc = 0x%x\n",
	       dack->dadck_d[0][0], dack->dadck_d[0][1]);
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S1 DAC_DCK ic = 0x%x, qc = 0x%x\n",
	       dack->dadck_d[1][0], dack->dadck_d[1][1]);

	RF_DBG(rf, DBG_RF_DACK, "[DACK]S0 biask ic = 0x%x, qc = 0x%x\n",
	       dack->biask_d[0][0], dack->biask_d[0][1]);
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S1 biask ic = 0x%x, qc = 0x%x\n",
	       dack->biask_d[1][0], dack->biask_d[1][1]);

	RF_DBG(rf, DBG_RF_DACK, "[DACK]S0 MSBK ic:\n");
	for (i = 0; i < 0x10; i++) {
		t = dack->msbk_d[0][0][i];
		RF_DBG(rf, DBG_RF_DACK, "[DACK]0x%x\n", t);
	}
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S0 MSBK qc:\n");
	for (i = 0; i < 0x10; i++) {
		t = dack->msbk_d[0][1][i];
		RF_DBG(rf, DBG_RF_DACK, "[DACK]0x%x\n", t);
	}
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S1 MSBK ic:\n");
	for (i = 0; i < 0x10; i++) {
		t = dack->msbk_d[1][0][i];
		RF_DBG(rf, DBG_RF_DACK, "[DACK]0x%x\n", t);
	}
	RF_DBG(rf, DBG_RF_DACK, "[DACK]S1 MSBK qc:\n");
	for (i = 0; i < 0x10; i++) {
		t = dack->msbk_d[1][1][i];
		RF_DBG(rf, DBG_RF_DACK, "[DACK]0x%x\n", t);
	}
}

void halrf_dac_cal_8730e(struct rf_info *rf, bool force)
{
	struct halrf_dack_info *dack = &rf->dack;

	dack->dack_done = false;
	RF_DBG(rf, DBG_RF_DACK, "[DACK]DACK 0x1\n");
	//RF_DBG(rf, DBG_RF_DACK, "[DACK]DACK start!!!\n");
#if 1
	halrf_addck_8730e(rf);
	halrf_dack_8730e(rf);
	dack->dack_done = true;
#endif
	dack->dack_cnt++;
	RF_DBG(rf, DBG_RF_DACK, "[DACK]DACK finish!!!\n");
}
#endif

