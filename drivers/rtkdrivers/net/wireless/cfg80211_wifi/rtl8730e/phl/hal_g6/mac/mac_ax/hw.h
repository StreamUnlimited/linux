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

#ifndef _MAC_AX_HW_H_
#define _MAC_AX_HW_H_

#define BT_2_DW(B3, B2, B1, B0)	\
	(((B3) << 24) | ((B2) << 16) | ((B1) << 8) | (B0))

#define NIB_2_DW(B7, B6, B5, B4, B3, B2, B1, B0)	\
	((((B7) & 0xf) << 28) | (((B6) & 0xf) << 24) | \
	(((B5) & 0xf) << 20) | (((B4) & 0xf) << 16) | \
	(((B3) & 0xf) << 12) | (((B2) & 0xf) << 8) | \
	(((B1) & 0xf) << 4) | ((B0) & 0xf))

#include "../type.h"
#include "status.h"
#include "wowlan.h"
#include "tblupd.h"
#include "ser.h"

/*--------------------Define -------------------------------------------*/
/*==========REG_TXPWR Register Address Definition==========*/
#define REG_AX_PWR_RATE_CTRL                         0xD200
#define REG_AX_PWR_RATE_OFST_CTRL                    0xD204
#define REG_AX_PWR_LMT_CTRL                          0xD208
#define REG_AX_PWR_MACID_CTRL                        0xD20C
#define REG_AX_PWR_BF_CTRL                           0xD210
#define REG_AX_PWR_MACID_REG                         0xD214
#define REG_AX_PWR_MACID_REG2                        0xD218
#define REG_AX_PWR_RU_LMT_CTRL                       0xD21C
#define REG_AX_PWR_COEXT_CTRL                        0xD220
#define REG_AX_PWR_SWING_LEG_CTRL                    0xD224
#define REG_AX_PWR_SWING_VHT_CTRL                    0xD228
#define REG_AX_PWR_SWING_HE_CTRL                     0xD22C
#define REG_AX_PWR_SWING_OTHER_CTRL0                 0xD230
#define REG_AX_PWR_SWING_OTHER_CTRL1                 0xD234
#define REG_AX_PWR_SR_CTRL0                          0xD238
#define REG_AX_PWR_SR_CTRL1                          0xD23C
#define REG_AX_PWR_UL_CTRL0                          0xD240
#define REG_AX_PWR_UL_CTRL1                          0xD244
#define REG_AX_PWR_UL_CTRL2                          0xD248
#define REG_AX_PWR_UL_CTRL3                          0xD24C
#define REG_AX_PWR_UL_CTRL4                          0xD250
#define REG_AX_PWR_UL_CTRL5                          0xD254
#define REG_AX_PWR_UL_CTRL6                          0xD258
#define REG_AX_PWR_NORM_FORCE0                       0xD25C
#define REG_AX_PWR_NORM_FORCE1                       0xD260
#define REG_AX_PWR_SR_FORCE0                         0xD264
#define REG_AX_PWR_SR_FORCE1                         0xD268
#define REG_AX_PWR_SR_FORCE2                         0xD26C
#define REG_AX_PWR_UL_FORCE0                         0xD270
#define REG_AX_PWR_NORM_FORCE2                       0xD274
#define REG_AX_PWR_UL_FORCE1                         0xD278
#define REG_AX_PWR_NORM_FORCE3                       0xD27C
#define REG_AX_PWR_NORM_FORCE4                       0xD280
#define REG_AX_PWR_RSVD0                             0xD284
#define REG_AX_PWR_RSVD1                             0xD288
#define REG_AX_PWR_RSVD2                             0xD28C
#define REG_AX_PWR_RSVD3                             0xD290
#define REG_AX_PWR_RSVD4                             0xD294
#define REG_AX_PWR_RSVD5                             0xD298
#define REG_AX_PWR_RSVD6                             0xD29C
#define REG_AX_PWR_BY_RATE_TABLE0                    0xD2C0
#define REG_AX_PWR_BY_RATE_TABLE1                    0xD2C4
#define REG_AX_PWR_BY_RATE_TABLE2                    0xD2C8
#define REG_AX_PWR_BY_RATE_TABLE3                    0xD2CC
#define REG_AX_PWR_BY_RATE_TABLE4                    0xD2D0
#define REG_AX_PWR_BY_RATE_TABLE5                    0xD2D4
#define REG_AX_PWR_BY_RATE_TABLE6                    0xD2D8
#define REG_AX_PWR_LMT_TABLE0                        0xD2EC
#define REG_AX_PWR_LMT_TABLE1                        0xD2F0
#define REG_AX_PWR_RU_LMT_TABLE0                     0xD33C
#define REG_AX_PWR_RU_LMT_TABLE1                     0xD340
#define REG_AX_PWR_RU_LMT_TABLE2                     0xD344
#define REG_AX_PWR_RU_LMT_TABLE3                     0xD348
#define REG_AX_PWR_RU_LMT_TABLE4                     0xD34C
#define REG_AX_PWR_RU_LMT_TABLE5                     0xD350
#define REG_AX_PWR_MACID_LMT_TABLE0                  0xD36C
#define REG_AX_PWR_MACID_LMT_TABLE1                  0xD370
#define REG_AX_PWR_MACID_LMT_TABLE2                  0xD374
#define REG_AX_PWR_MACID_LMT_TABLE3                  0xD378
#define REG_AX_PWR_MACID_LMT_TABLE4                  0xD37C
#define REG_AX_PWR_MACID_LMT_TABLE5                  0xD380
#define REG_AX_PWR_MACID_LMT_TABLE6                  0xD384
#define REG_AX_PWR_MACID_LMT_TABLE7                  0xD388
#define REG_AX_PWR_MACID_LMT_TABLE8                  0xD38C
#define REG_AX_PWR_MACID_LMT_TABLE9                  0xD390
#define REG_AX_PWR_MACID_LMT_TABLE10                 0xD394
#define REG_AX_PWR_MACID_LMT_TABLE11                 0xD398
#define REG_AX_PWR_MACID_LMT_TABLE12                 0xD39C
#define REG_AX_PWR_MACID_LMT_TABLE13                 0xD3A0
#define REG_AX_PWR_MACID_LMT_TABLE14                 0xD3A4
#define REG_AX_PWR_MACID_LMT_TABLE15                 0xD3A8
#define REG_AX_PWR_SR_MCS0_TXDIFF_TABLE0             0xD56C
#define REG_AX_PWR_SR_MCS0_TXDIFF_TABLE1             0xD570
#define REG_AX_PWR_SR_MCS0_TXDIFF_TABLE2             0xD574
#define REG_AX_PWR_SR_MCS0_TXDIFF_TABLE3             0xD578
#define REG_AX_PWR_SR_MCS1_TXDIFF_TABLE0             0xD57C
#define REG_AX_PWR_SR_MCS1_TXDIFF_TABLE1             0xD580
#define REG_AX_PWR_SR_MCS1_TXDIFF_TABLE2             0xD584
#define REG_AX_PWR_SR_MCS1_TXDIFF_TABLE3             0xD588
#define REG_AX_PWR_SR_MCS2_TXDIFF_TABLE0             0xD58C
#define REG_AX_PWR_SR_MCS2_TXDIFF_TABLE1             0xD590
#define REG_AX_PWR_SR_MCS2_TXDIFF_TABLE2             0xD594
#define REG_AX_PWR_SR_MCS2_TXDIFF_TABLE3             0xD598
#define REG_AX_PWR_SR_MCS3_TXDIFF_TABLE0             0xD59C
#define REG_AX_PWR_SR_MCS3_TXDIFF_TABLE1             0xD5A0
#define REG_AX_PWR_SR_MCS3_TXDIFF_TABLE2             0xD5A4
#define REG_AX_PWR_SR_MCS3_TXDIFF_TABLE3             0xD5A8
#define REG_AX_PWR_SR_MCS4_TXDIFF_TABLE0             0xD5AC
#define REG_AX_PWR_SR_MCS4_TXDIFF_TABLE1             0xD5B0
#define REG_AX_PWR_SR_MCS4_TXDIFF_TABLE2             0xD5B4
#define REG_AX_PWR_SR_MCS4_TXDIFF_TABLE3             0xD5B8
#define REG_AX_PWR_SR_MCS5_TXDIFF_TABLE0             0xD5BC
#define REG_AX_PWR_SR_MCS5_TXDIFF_TABLE1             0xD5C0
#define REG_AX_PWR_SR_MCS5_TXDIFF_TABLE2             0xD5C4
#define REG_AX_PWR_SR_MCS5_TXDIFF_TABLE3             0xD5C8
#define REG_AX_PWR_SR_MCS6_TXDIFF_TABLE0             0xD5CC
#define REG_AX_PWR_SR_MCS6_TXDIFF_TABLE1             0xD5D0
#define REG_AX_PWR_SR_MCS6_TXDIFF_TABLE2             0xD5D4
#define REG_AX_PWR_SR_MCS6_TXDIFF_TABLE3             0xD5D8
#define REG_AX_PWR_SR_MCS7_TXDIFF_TABLE0             0xD5DC
#define REG_AX_PWR_SR_MCS7_TXDIFF_TABLE1             0xD5E0
#define REG_AX_PWR_SR_MCS7_TXDIFF_TABLE2             0xD5E4
#define REG_AX_PWR_SR_MCS7_TXDIFF_TABLE3             0xD5E8
#define REG_AX_PWR_SR_MCS8_TXDIFF_TABLE0             0xD5EC
#define REG_AX_PWR_SR_MCS8_TXDIFF_TABLE1             0xD5F0
#define REG_AX_PWR_SR_MCS8_TXDIFF_TABLE2             0xD5F4
#define REG_AX_PWR_SR_MCS8_TXDIFF_TABLE3             0xD5F8
#define REG_AX_PWR_SR_MCS9_TXDIFF_TABLE0             0xD5FC
#define REG_AX_PWR_SR_MCS9_TXDIFF_TABLE1             0xD600
#define REG_AX_PWR_SR_MCS9_TXDIFF_TABLE2             0xD604
#define REG_AX_PWR_SR_MCS9_TXDIFF_TABLE3             0xD608
#define REG_AX_PWR_SR_MCS10_TXDIFF_TABLE0            0xD60C
#define REG_AX_PWR_SR_MCS10_TXDIFF_TABLE1            0xD610
#define REG_AX_PWR_SR_MCS10_TXDIFF_TABLE2            0xD614
#define REG_AX_PWR_SR_MCS10_TXDIFF_TABLE3            0xD618
#define REG_AX_PWR_SR_MCS11_TXDIFF_TABLE0            0xD61C
#define REG_AX_PWR_SR_MCS11_TXDIFF_TABLE1            0xD620
#define REG_AX_PWR_SR_MCS11_TXDIFF_TABLE2            0xD624
#define REG_AX_PWR_SR_MCS11_TXDIFF_TABLE3            0xD628
#define REG_AX_PWR_CCK1M_TXDIFF_TABLE0               0xD62C
#define REG_AX_PWR_CCK1M_TXDIFF_TABLE1               0xD630
#define REG_AX_PWR_CCK1M_TXDIFF_TABLE2               0xD634
#define REG_AX_PWR_CCK1M_TXDIFF_TABLE3               0xD638
#define REG_AX_PWR_CCK2M_TXDIFF_TABLE0               0xD63C
#define REG_AX_PWR_CCK2M_TXDIFF_TABLE1               0xD640
#define REG_AX_PWR_CCK2M_TXDIFF_TABLE2               0xD644
#define REG_AX_PWR_CCK2M_TXDIFF_TABLE3               0xD648
#define REG_AX_PWR_CCK5P5M_TXDIFF_TABLE0             0xD64C
#define REG_AX_PWR_CCK5P5M_TXDIFF_TABLE1             0xD650
#define REG_AX_PWR_CCK5P5M_TXDIFF_TABLE2             0xD654
#define REG_AX_PWR_CCK5P5M_TXDIFF_TABLE3             0xD658
#define REG_AX_PWR_CCK11M_TXDIFF_TABLE0              0xD65C
#define REG_AX_PWR_CCK11M_TXDIFF_TABLE1              0xD660
#define REG_AX_PWR_CCK11M_TXDIFF_TABLE2              0xD664
#define REG_AX_PWR_CCK11M_TXDIFF_TABLE3              0xD668
#define REG_AX_PWR_LEGACY6M_TXDIFF_TABLE0            0xD66C
#define REG_AX_PWR_LEGACY6M_TXDIFF_TABLE1            0xD670
#define REG_AX_PWR_LEGACY6M_TXDIFF_TABLE2            0xD674
#define REG_AX_PWR_LEGACY6M_TXDIFF_TABLE3            0xD678
#define REG_AX_PWR_LEGACY9M_TXDIFF_TABLE0            0xD67C
#define REG_AX_PWR_LEGACY9M_TXDIFF_TABLE1            0xD680
#define REG_AX_PWR_LEGACY9M_TXDIFF_TABLE2            0xD684
#define REG_AX_PWR_LEGACY9M_TXDIFF_TABLE3            0xD688
#define REG_AX_PWR_LEGACY12M_TXDIFF_TABLE0           0xD68C
#define REG_AX_PWR_LEGACY12M_TXDIFF_TABLE1           0xD690
#define REG_AX_PWR_LEGACY12M_TXDIFF_TABLE2           0xD694
#define REG_AX_PWR_LEGACY12M_TXDIFF_TABLE3           0xD698
#define REG_AX_PWR_LEGACY18M_TXDIFF_TABLE0           0xD69C
#define REG_AX_PWR_LEGACY18M_TXDIFF_TABLE1           0xD6A0
#define REG_AX_PWR_LEGACY18M_TXDIFF_TABLE2           0xD6A4
#define REG_AX_PWR_LEGACY18M_TXDIFF_TABLE3           0xD6A8
#define REG_AX_PWR_LEGACY24M_TXDIFF_TABLE0           0xD6AC
#define REG_AX_PWR_LEGACY24M_TXDIFF_TABLE1           0xD6B0
#define REG_AX_PWR_LEGACY24M_TXDIFF_TABLE2           0xD6B4
#define REG_AX_PWR_LEGACY24M_TXDIFF_TABLE3           0xD6B8
#define REG_AX_PWR_LEGACY36M_TXDIFF_TABLE0           0xD6BC
#define REG_AX_PWR_LEGACY36M_TXDIFF_TABLE1           0xD6C0
#define REG_AX_PWR_LEGACY36M_TXDIFF_TABLE2           0xD6C4
#define REG_AX_PWR_LEGACY36M_TXDIFF_TABLE3           0xD6C8
#define REG_AX_PWR_LEGACY48M_TXDIFF_TABLE0           0xD6CC
#define REG_AX_PWR_LEGACY48M_TXDIFF_TABLE1           0xD6D0
#define REG_AX_PWR_LEGACY48M_TXDIFF_TABLE2           0xD6D4
#define REG_AX_PWR_LEGACY48M_TXDIFF_TABLE3           0xD6D8
#define REG_AX_PWR_LEGACY54M_TXDIFF_TABLE0           0xD6DC
#define REG_AX_PWR_LEGACY54M_TXDIFF_TABLE1           0xD6E0
#define REG_AX_PWR_LEGACY54M_TXDIFF_TABLE2           0xD6E4
#define REG_AX_PWR_LEGACY54M_TXDIFF_TABLE3           0xD6E8
#define REG_AX_PWR_TCM_WRAPPER_CR0                   0xD800
#define REG_AX_PWR_TCM_WRAPPER_CR1                   0xD804
#define REG_AX_PWR_TCM_WRAPPER_CR2                   0xD808
#define REG_AX_PWR_TCM_WRAPPER_CR3                   0xD80C
#define REG_AX_PWR_TCM_WRAPPER_CR4                   0xD810
#define REG_AX_PWR_TCM_WRAPPER_CR5                   0xD814
#define REG_AX_PWR_TCM_WRAPPER_CR6                   0xD818
#define REG_AX_PWR_TCM_WRAPPER_CR7                   0xD81C
#define REG_AX_PWR_TCM_WRAPPER_CR8                   0xD820
#define REG_AX_PWR_TCM_WRAPPER_CR9                   0xD824
#define REG_AX_PWR_TCM_WRAPPER_CR10                  0xD828
#define REG_AX_PWR_TCM_WRAPPER_CR11                  0xD82C
#define REG_AX_PWR_TCM_WRAPPER_CR12                  0xD830
#define REG_AX_PWR_TCM_WRAPPER_CR13                  0xD834
#define REG_AX_PWR_TCM_WRAPPER_CR14                  0xD838
#define REG_AX_PWR_TCM_WRAPPER_CR15                  0xD83C
#define REG_AX_PWR_TCM_WRAPPER_CR16                  0xD840
#define REG_AX_PWR_TCM_WRAPPER_CR17                  0xD844
#define REG_AX_PWR_TCM_WRAPPER_CR18                  0xD848
#define REG_AX_PWR_TSSI_TABLE0                       0xD908
#define REG_AX_PWR_TSSI_TABLE1                       0xD90C
#define REG_AX_PWR_TSSI_TABLE2                       0xD910
#define REG_AX_PWR_TSSI_TABLE3                       0xD914
#define REG_AX_PWR_TSSI_TABLE4                       0xD918
#define REG_AX_PWR_TSSI_TABLE5                       0xD91C
#define REG_AX_PWR_TSSI_TABLE6                       0xD920
#define REG_AX_PWR_TSSI_TABLE7                       0xD924
#define REG_AX_PWR_TSSI_TABLE8                       0xD928
#define REG_AX_PWR_TSSI_TABLE9                       0xD92C
#define REG_AX_PWR_TSSI_TABLE10                      0xD930
#define REG_AX_PWR_TSSI_TABLE11                      0xD934
#define REG_AX_PWR_TSSI_TABLE12                      0xD938
#define REG_AX_PWR_TSSI_TABLE13                      0xD93C
#define REG_AX_PWR_TSSI_TABLE14                      0xD940
#define REG_AX_PWR_TSSI_TABLE15                      0xD944
#define REG_AX_PWR_TSSI_TABLE16                      0xD948
#define REG_AX_PWR_TSSI_TABLE17                      0xD94C
#define REG_AX_PWR_TSSI_TABLE18                      0xD950
#define REG_AX_PWR_TSSI_TABLE19                      0xD954
#define REG_AX_PWR_TSSI_TABLE20                      0xD958
#define REG_USR_LIST_SRAM                            0xDB00
#define REG_AX_PWR_END                               0xFFFF

#define BITS_WLRF_CTRL 0x82
#define BITS_WLRF1_CTRL 0x8200
#define PHYREG_SET_ALL_CYCLE 0xC
#define PHYREG_SET_XYN_CYCLE 0xE
#define PHYREG_SET_X_CYCLE 0x4
#define PHYREG_SET_N_CYCLE 0x2
#define PHYREG_SET_Y_CYCLE 0x1

#define TXSC_80M 0x91
#define TXSC_40M 0x1
#define TXSC_20M 0x0

#define TBL_READ_OP 0x0
#define TBL_WRITE_OP 0x1
#define TXCNT_LMT_MSK 0x1

#define CHANNEL_5G 34

#define CR_TXCNT_MSK 0x7FFFFFFF

/*For XTAL_SI */
#define XTAL_SI_POLLING_CNT 1000
#define XTAL_SI_POLLING_DLY_US 50
#define XTAL_SI_NORMAL_WRITE 0x00
#define XTAL_SI_NORMAL_READ 0x01
#define XTAL_SI_XTAL_SC_XI 0x04
#define XTAL_SI_XTAL_SC_XO 0x05
#define XTAL_SI_XTAL_XMD_2 0x24
#define XTAL_SI_XTAL_XMD_4 0x26
#define XTAL_SI_CV 0x41
#define XTAL0 0x0
#define XTAL3 0x3
#define XTAL_SI_WL_RFC_S0 0x80
#define XTAL_SI_WL_RFC_S1 0x81
#define XTAL_SI_ANAPAR_WL 0x90
#define XTAL_SI_GNT_CTRL 0x91
#define XTAL_SI_SRAM_CTRL 0xA1
#define FULL_BIT_MASK 0xFF

/*For ADC LDO setting*/
#define LDO2PW_LDO_VSEL 0x3600000

/* For TXPWR Usage*/
#define PWR_BY_RATE_LGCY_OFFSET 0XC0
#define PWR_BY_RATE_OFFSET 0XCC

#define PWR_LMT_CCK_OFFSET 0XEC
#define PWR_LMT_LGCY_OFFSET 0XF0
#define PWR_LMT_TBL2_OFFSET 0XF4
#define PWR_LMT_TBL5_OFFSET 0X100
#define PWR_LMT_TBL6_OFFSET 0X104
#define PWR_LMT_TBL7_OFFSET 0X108
#define PWR_LMT_TBL8_OFFSET 0X10C
#define PWR_LMT_TBL9_OFFSET 0X110

#define PWR_LMT_TBL_UNIT 0x08

#define POLL_SWR_VOLT_CNT 2
#define POLL_SWR_VOLT_US 50
#define CORE_SWR_VOLT_MAX 0xE
#define CORE_SWR_VOLT_MIN 0x8

/* For SS SRAM access*/
#define SS_R_QUOTA_SETTING  0
#define SS_W_QUOTA_SETTING  1
#define SS_R_QUOTA          2
#define SS_W_QUOTA          3
#define SS_R_TX_LEN         5
#define SS_R_DL_MURU_DIS    8
#define SS_W_DL_MURU_DIS    9
#define SS_R_UL_TBL         10
#define SS_W_UL_TBL         11
#define SS_R_BSR_LEN        12
#define SS_W_BSR_LEN        13

#define SS_QUOTA_SETTING_MSK    0xF
#define SS_MACID_SH		8

/* WDRLS filter map */
#define B_WDRLS_FLTR_TXOK BIT(24)
#define B_WDRLS_FLTR_RTYLMT BIT(25)
#define B_WDRLS_FLTR_LIFTIM BIT(26)
#define B_WDRLS_FLTR_MACID BIT(27)

#define MAC_AX_DRV_INFO_SIZE_0 0
#define MAC_AX_DRV_INFO_SIZE_16 1
#define MAC_AX_DRV_INFO_SIZE_32 2
#define MAC_AX_DRV_INFO_SIZE_48 3
#define MAC_AX_DRV_INFO_SIZE_64 4
#define MAC_AX_DRV_INFO_SIZE_80 5

#define CHK_BBRF_IO 0

/* Narrow BW*/
#define US_TIME_10M     0x50
#define US_TIME_5M      0x40
#define SLOTTIME_10M    0xD
#define SLOTTIME_5M     0x15
#define ACK_TO_10M      0x80
#define ACK_TO_5M       0xFF

/*--------------------Define Enum---------------------------------------*/

/**
 * @enum tx_tf_info
 *
 * @brief tx_tf_info
 *
 * @var tx_tf_info::USER_INFO0_SEL
 * Please Place Description here.
 * @var tx_tf_info::USER_INFO1_SEL
 * Please Place Description here.
 * @var tx_tf_info::USER_INFO2_SEL
 * Please Place Description here.
 * @var tx_tf_info::USER_INFO3_SEL
 * Please Place Description here.
 * @var tx_tf_info::COMMON_INFO_SEL
 * Please Place Description here.
 */
enum tx_tf_info {
	USER_INFO0_SEL		= 0,
	USER_INFO1_SEL		= 1,
	USER_INFO2_SEL		= 2,
	USER_INFO3_SEL		= 3,
	COMMON_INFO_SEL		= 4,
};

enum addr_rang_idx {
	ADDR_AON = 0,
	ADDR_HCI,
	ADDR_PON,
	ADDR_DMAC,
	ADDR_CMAC0,
	ADDR_CMAC1,
	ADDR_BB0,
	ADDR_BB1,
	ADDR_RF,
	ADDR_IND_ACES,
	ADDR_RSVD,

	/* keep last */
	ADDR_LAST,
	ADDR_MAX = ADDR_LAST,
	ADDR_INVALID = ADDR_LAST
};

/*--------------------Define MACRO--------------------------------------*/
#define ADDR_IS_AON_8852A(addr) ((addr) <= 0x4FF ? 1 : 0)
#define ADDR_IS_AON_8852B(addr) ((addr) <= 0x4FF ? 1 : 0)
#define ADDR_IS_AON_8852C(addr) ((addr) <= 0x4FF ? 1 : 0)
#define ADDR_IS_AON_8192XB(addr) ((addr) <= 0x4FF ? 1 : 0)

#define ADDR_IS_HCI_8852A(addr) \
	((addr) >= 0x1000 && (addr) <= 0x1FFF ? 1 : 0)
#define ADDR_IS_HCI_8852B(addr) \
	((addr) >= 0x1000 && (addr) <= 0x1FFF ? 1 : 0)
#define ADDR_IS_HCI_8852C(addr) \
	(((addr) >= 0x2000 && (addr) <= 0x63FF) || \
	 ((addr) >= 0x7C00 && (addr) <= 0x7FFF) ? 1 : 0)
#define ADDR_IS_HCI_8192XB(addr) \
		(((addr) >= 0x2000 && (addr) <= 0x63FF) || \
		 ((addr) >= 0x7C00 && (addr) <= 0x7FFF) ? 1 : 0)

#define ADDR_IS_PON_8852A(addr) \
	(((addr) >= 0x400 && (addr) <= 0xFFF) || \
	 ((addr) >= 0x8000 && (addr) <= 0x8407) || \
	 ((addr) >= 0xC000 && (addr) <= 0xC007) || \
	 ((addr) >= 0xE000 && (addr) <= 0xE007) ? 1 : 0)
#define ADDR_IS_PON_8852B(addr) \
	(((addr) >= 0x400 && (addr) <= 0xFFF) || \
	 ((addr) >= 0x8000 && (addr) <= 0x8407) || \
	 ((addr) >= 0xC000 && (addr) <= 0xC007) ? 1 : 0)
#define ADDR_IS_PON_8852C(addr) \
	(((addr) >= 0x400 && (addr) <= 0xFFF) || \
	 ((addr) >= 0x7000 && (addr) <= 0x7BFF) || \
	 ((addr) >= 0x8000 && (addr) <= 0x8407) || \
	 ((addr) >= 0xC000 && (addr) <= 0xC007) || \
	 ((addr) >= 0xE000 && (addr) <= 0xE007) ? 1 : 0)
#define ADDR_IS_PON_8192XB(addr) \
		(((addr) >= 0x400 && (addr) <= 0xFFF) || \
		 ((addr) >= 0x7000 && (addr) <= 0x7BFF) || \
		 ((addr) >= 0x8000 && (addr) <= 0x8407) || \
		 ((addr) >= 0xC000 && (addr) <= 0xC007) || \
		 ((addr) >= 0xE000 && (addr) <= 0xE007) ? 1 : 0)

#define ADDR_IS_DMAC_8852A(addr) \
	((addr) >= 0x8408 && (addr) <= 0xBFFF ? 1 : 0)
#define ADDR_IS_DMAC_8852B(addr) \
	((addr) >= 0x8408 && (addr) <= 0xBFFF ? 1 : 0)
#define ADDR_IS_DMAC_8852C(addr) \
	(((addr) >= 0x1000 && (addr) <= 0x1FFF) || \
	 ((addr) >= 0x8408 && (addr) <= 0xBFFF) ? 1 : 0)
#define ADDR_IS_DMAC_8192XB(addr) \
		(((addr) >= 0x1000 && (addr) <= 0x1FFF) || \
		 ((addr) >= 0x8408 && (addr) <= 0xBFFF) ? 1 : 0)

#define ADDR_IS_CMAC0_8852A(addr) \
	((addr) >= 0xC008 && (addr) <= 0xDFFF ? 1 : 0)
#define ADDR_IS_CMAC0_8852B(addr) \
	((addr) >= 0xC008 && (addr) <= 0xDFFF ? 1 : 0)
#define ADDR_IS_CMAC0_8852C(addr) \
	((addr) >= 0xC008 && (addr) <= 0xDFFF ? 1 : 0)
#define ADDR_IS_CMAC0_8192XB(addr) \
		((addr) >= 0xC008 && (addr) <= 0xDFFF ? 1 : 0)

#define ADDR_IS_CMAC1_8852A(addr) \
	((addr) >= 0xE008 && (addr) <= 0xFFFF ? 1 : 0)
#define ADDR_IS_CMAC1_8852B(addr) \
	((addr) >= 0xE008 && (addr) <= 0xFFFF ? 0 : 0)
#define ADDR_IS_CMAC1_8852C(addr) \
	((addr) >= 0xE008 && (addr) <= 0xFFFF ? 1 : 0)
#define ADDR_IS_CMAC1_8192XB(addr) \
		((addr) >= 0xE008 && (addr) <= 0xFFFF ? 1 : 0)

#define ADDR_IS_BB0_8852A(addr) \
	(((addr) >= 0x10000 && (addr) <= 0x125FF) || \
	 ((addr) >= 0x12E00 && (addr) <= 0x138FF) || \
	 ((addr) >= 0x13C00 && (addr) <= 0x15FFF) || \
	 ((addr) >= 0x17000 && (addr) <= 0x17FFF) ? 1 : 0)
#define ADDR_IS_BB0_8852B(addr) \
	(((addr) >= 0x10000 && (addr) <= 0x125FF) || \
	 ((addr) >= 0x12E00 && (addr) <= 0x138FF) || \
	 ((addr) >= 0x13C00 && (addr) <= 0x15FFF) || \
	 ((addr) >= 0x17000 && (addr) <= 0x17FFF) ? 1 : 0)
#define ADDR_IS_BB0_8852C(addr) \
	(((addr) >= 0x10000 && (addr) <= 0x125FF) || \
	 ((addr) >= 0x12E00 && (addr) <= 0x138FF) || \
	 ((addr) >= 0x13C00 && (addr) <= 0x15FFF) || \
	 ((addr) >= 0x17000 && (addr) <= 0x17FFF) ? 1 : 0)
#define ADDR_IS_BB0_8192XB(addr) \
		(((addr) >= 0x10000 && (addr) <= 0x125FF) || \
		 ((addr) >= 0x12E00 && (addr) <= 0x138FF) || \
		 ((addr) >= 0x13C00 && (addr) <= 0x15FFF) || \
		 ((addr) >= 0x17000 && (addr) <= 0x17FFF) ? 1 : 0)

#define ADDR_IS_BB1_8852A(addr) \
	(((addr) >= 0x12600 && (addr) <= 0x12DFF) || \
	 ((addr) >= 0x13900 && (addr) <= 0x13BFF) || \
	 ((addr) >= 0x16000 && (addr) <= 0x16FFF) ? 1 : 0)
#define ADDR_IS_BB1_8852B(addr) \
	(((addr) >= 0x12600 && (addr) <= 0x12DFF) || \
	 ((addr) >= 0x13900 && (addr) <= 0x13BFF) || \
	 ((addr) >= 0x16000 && (addr) <= 0x16FFF) ? 1 : 0)
#define ADDR_IS_BB1_8852C(addr) \
	(((addr) >= 0x12600 && (addr) <= 0x12DFF) || \
	 ((addr) >= 0x13900 && (addr) <= 0x13BFF) || \
	 ((addr) >= 0x16000 && (addr) <= 0x16FFF) ? 1 : 0)
#define ADDR_IS_BB1_8192XB(addr) \
		(((addr) >= 0x12600 && (addr) <= 0x12DFF) || \
		 ((addr) >= 0x13900 && (addr) <= 0x13BFF) || \
		 ((addr) >= 0x16000 && (addr) <= 0x16FFF) ? 1 : 0)

#define ADDR_IS_RF_8852A(addr) \
	((addr) >= 0x18000 && (addr) <= 0x1DFFF ? 1 : 0)
#define ADDR_IS_RF_8852B(addr) \
	((addr) >= 0x18000 && (addr) <= 0x1FFFF ? 1 : 0)
#define ADDR_IS_RF_8852C(addr) \
	((addr) >= 0x18000 && (addr) <= 0x1FFFF ? 1 : 0)
#define ADDR_IS_RF_8192XB(addr) \
		((addr) >= 0x18000 && (addr) <= 0x1FFFF ? 1 : 0)

#define ADDR_IS_IND_ACES_8852A(addr) \
	((addr) >= 0x40000 && (addr) <= 0x7FFFF ? 1 : 0)
#define ADDR_IS_IND_ACES_8852B(addr) \
	((addr) >= 0x40000 && (addr) <= 0x7FFFF ? 1 : 0)
#define ADDR_IS_IND_ACES_8852C(addr) \
	((addr) >= 0x40000 && (addr) <= 0x7FFFF ? 1 : 0)
#define ADDR_IS_IND_ACES_8192XB(addr) \
		((addr) >= 0x40000 && (addr) <= 0x7FFFF ? 1 : 0)

#define ADDR_IS_RSVD_8852A(addr) \
	(((addr) >= 0x2000 && (addr) <= 0x7FFF) || \
	 ((addr) >= 0x1E000 && (addr) <= 0x3FFFF) || \
	 (addr) >= 0x80000 ? 1 : 0)
#define ADDR_IS_RSVD_8852B(addr) \
	(((addr) >= 0x2000 && (addr) <= 0x7FFF) || \
	 ((addr) >= 0x20000 && (addr) <= 0x3FFFF) || \
	 (addr) >= 0x80000 ? 1 : 0)
#define ADDR_IS_RSVD_8852C(addr) \
	(((addr) >= 0x6400 && (addr) <= 0x6FFF) || \
	 ((addr) >= 0x20000 && (addr) <= 0x3FFFF) || \
	 (addr) >= 0x80000 ? 1 : 0)
#define ADDR_IS_RSVD_8192XB(addr) \
		(((addr) >= 0x6400 && (addr) <= 0x6FFF) || \
		 ((addr) >= 0x20000 && (addr) <= 0x3FFFF) || \
		 (addr) >= 0x80000 ? 1 : 0)

#if (MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT)
#define ADDR_NOT_ALLOW_SERL1(addr) \
	((addr) != R_AX_SER_DBG_INFO && (addr) != R_AX_HCI_FUNC_EN && \
	 (addr) != R_AX_HD0IMR && (addr) != R_AX_HD0ISR ? 1 : 0)
#define ADDR_NOT_ALLOW_LPS(addr) ((addr) != R_AX_CPWM ? 1 : 0)
#else // (MAC_AX_8852C_SUPPORT || MAC_AX_8192XB_SUPPORT)
#define ADDR_NOT_ALLOW_SERL1(addr) \
	((addr) != R_AX_SER_DBG_INFO && (addr) != R_AX_HCI_FUNC_EN_V1 && \
	 (addr) != R_AX_HD0IMR_V1 && (addr) != R_AX_HD0ISR_V1 ? 1 : 0)
#define ADDR_NOT_ALLOW_LPS(addr) ((addr) != R_AX_CPWM_V1 ? 1 : 0)
#endif

/*--------------------Define Struct-------------------------------------*/

/*--------------------Function declaration------------------------------*/

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */
/**
 * @brief mac_get_hw_info
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval  mac_ax_hw_info
 */
struct mac_ax_hw_info *mac_get_hw_info(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_set_hw_value
 *
 * @param *adapter
 * @param hw_id
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_set_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_get_hw_value
 *
 * @param *adapter
 * @param hw_id
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_get_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_xtal_si
 *
 * @param *adapter
 * @param offset
 * @param val
 * @param bitmask
 * @return Please Place Description here.
 * @retval u32
 */
//u32 mac_write_xtal_si(struct mac_ax_adapter *adapter,
//		      u8 offset, u8 val, u8 bitmask);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_read_xtal_si
 *
 * @param *adapter
 * @param offset
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
//u32 mac_read_xtal_si(struct mac_ax_adapter *adapter,
//		     u8 offset, u8 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_read_pwr_reg
 *
 * @param *adapter
 * @param band
 * @param offset
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_l2_status(struct mac_ax_adapter *adapter);
u32 mac_read_pwr_reg(struct mac_ax_adapter *adapter,
		     const u32 offset, u32 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_reg
 *
 * @param *adapter
 * @param band
 * @param offset
 * @param val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_reg(struct mac_ax_adapter *adapter,
		      const u32 offset, u32 val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_msk_pwr_reg
 *
 * @param *adapter
 * @param band
 * @param offset
 * @param mask
 * @param val
 * @return write tx power reg with mask value into fw
 * @retval u32
 */
u32 mac_write_msk_pwr_reg(struct mac_ax_adapter *adapter,
			  const u32 offset, u32 mask, u32 val);

/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_ofst_mode
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_ofst_mode(struct mac_ax_adapter *adapter,
			    struct rtw_tpu_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_ofst_bw
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_ofst_bw(struct mac_ax_adapter *adapter,
			  struct rtw_tpu_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_ref_reg
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_ref_reg(struct mac_ax_adapter *adapter,
			  struct rtw_tpu_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_limit_en
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_limit_en(struct mac_ax_adapter *adapter,
			   struct rtw_tpu_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_limit_rua_reg
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_limit_rua_reg(struct mac_ax_adapter *adapter,
				struct rtw_tpu_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_limit_reg
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_limit_reg(struct mac_ax_adapter *adapter,
			    struct rtw_tpu_pwr_imt_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_pwr_by_rate_reg
 *
 * @param *adapter
 * @param band
 * @param *tpu
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_pwr_by_rate_reg(struct mac_ax_adapter *adapter,
			      struct rtw_tpu_pwr_by_rate_info *tpu);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/** * @brief mac_read_xcap_reg
 *
 * @param *adapter
 * @param sc_xo
 * @param *val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_read_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 *val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief mac_write_xcap_reg
 *
 * @param *adapter
 * @param sc_xo
 * @param val
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_write_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 val);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */


/**
 * @brief mac_write_bbrst_reg
 *
 * @param *adapter
 * @param val
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_write_bbrst_reg(struct mac_ax_adapter *adapter, u8 val);
/**
 * @}
 * @}
 */

/**
 * @brief set_enable_bb_rf
 *
 * @param *adapter
 * @param enable
 * @return Please Place Description here.
 * @retval u32
 */
u32 set_enable_bb_rf(struct mac_ax_adapter *adapter, u8 enable);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief cfg_mac_bw
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 cfg_mac_bw(struct mac_ax_adapter *adapter,
	       struct mac_ax_cfg_bw *cfg);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief get_ss_quota_mode
 *
 * @param *adapter
 * @param *ctrl
 * @return Please Place Description here.
 * @retval u32
 */
u32 get_ss_quota_mode(struct mac_ax_adapter *adapter,
		      struct mac_ax_ss_quota_mode_ctrl *ctrl);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

/**
 * @brief ss_get_quotasetting
 *
 * @param *adapter
 * @param *para
 * @return Please Place Description here.
 * @retval u32
 */
u32 ss_get_quotasetting(struct mac_ax_adapter *adapter,
			struct mac_ax_ss_quota_setting *para);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Common
 * @{
 * @addtogroup HW_Related
 * @{
 */

u32 set_macid_pause(struct mac_ax_adapter *adapter,
		    struct mac_ax_macid_pause_cfg *cfg);


//u32 mac_get_bt_dis(struct mac_ax_adapter *adapter);

//u32 mac_set_bt_dis(struct mac_ax_adapter *adapter, u8 en);


#endif
