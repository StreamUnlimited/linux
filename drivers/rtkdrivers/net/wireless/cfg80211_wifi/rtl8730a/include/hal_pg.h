/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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

#ifndef __HAL_PG_H__
#define __HAL_PG_H__

#define PPG_BB_GAIN_2G_TX_OFFSET_MASK	0x0F
#define PPG_BB_GAIN_2G_TXB_OFFSET_MASK	0xF0

#define PPG_BB_GAIN_5G_TX_OFFSET_MASK	0x1F
#define PPG_THERMAL_OFFSET_MASK			0x1F
#define KFREE_BB_GAIN_2G_TX_OFFSET(_ppg_v) (((_ppg_v) == PPG_BB_GAIN_2G_TX_OFFSET_MASK) ? 0 : (((_ppg_v) & 0x01) ? ((_ppg_v) >> 1) : (-((_ppg_v) >> 1))))
#define KFREE_BB_GAIN_2G_TXB_OFFSET(_ppg_v) (((_ppg_v) == PPG_BB_GAIN_2G_TXB_OFFSET_MASK) ? 0 : (((_ppg_v) & 0x10) ? ((_ppg_v) >> 5) : (-((_ppg_v) >> 5))))
#define KFREE_BB_GAIN_5G_TX_OFFSET(_ppg_v) (((_ppg_v) == PPG_BB_GAIN_5G_TX_OFFSET_MASK) ? 0 : (((_ppg_v) & 0x01) ? ((_ppg_v) >> 1) : (-((_ppg_v) >> 1))))
#define KFREE_THERMAL_OFFSET(_ppg_v) (((_ppg_v) == PPG_THERMAL_OFFSET_MASK) ? 0 : (((_ppg_v) & 0x01) ? ((_ppg_v) >> 1) : (-((_ppg_v) >> 1))))

//====================================================
//			EEPROM/Efuse PG Offset for 8730A
//====================================================
#define PPG_8723D_S1 0
#define PPG_8723D_S0 1

// 0x10 ~ 0x63 = TX power area.
#define	EEPROM_TX_PWR_INX_8730A				0x20
#define	EEPROM_ChannelPlan_8730A			0xC8
#define	EEPROM_XTAL_8730A					0xC9
#define	EEPROM_THERMAL_METER_8730A			0xCA

#define	EEPROM_IQK_LCK_8730A				0xCB
#define	EEPROM_2G_5G_PA_TYPE_8730A			0xCC
#define	EEPROM_2G_LNA_TYPE_GAIN_SEL_8730A	0xCD
#define	EEPROM_5G_LNA_TYPE_GAIN_SEL_8730A	0xCF

#define	EEPROM_RF_BOARD_OPTION_8730A		0x131

#define	EEPROM_FEATURE_OPTION_8730A			0x132
#define	EEPROM_RF_BT_SETTING_8730A			0x133

#define	EEPROM_VERSION_8730A				0x134
#define	EEPROM_CustomID_8730A				0x135

#define	EEPROM_TX_BBSWING_2G_8730A			0x136
#define	EEPROM_TX_BBSWING_5G_8730A			0x137
#define	EEPROM_TX_PWR_CAL_RATE_8730A		0x138
#define	EEPROM_RF_ANTENNA_OPT_8730A			0x139
#define	EEPROM_RFE_OPTION_8730A				0x13A
#define	EEPROM_COUNTRY_CODE_8730A			0x13B

//RTL8723BU
#define EEPROM_MAC_ADDR_8723BU				0x107
#define EEPROM_VID_8723BU					0x100
#define EEPROM_PID_8723BU					0x102
#define EEPROM_PA_TYPE_8723BU				0xBC
#define EEPROM_LNA_TYPE_2G_8723BU			0xBD

//RTL8723BS
#define	EEPROM_MAC_ADDR_8730A				0x11A

#define EEPROM_Voltage_ADDR_8723B			0x8

#define EEPROM_TX_KFREE_8730A				0xEE

#define EEPROM_THERMAL_OFFSET_8730A	       	0xEF

#define EEPROM_PACKAGE_TYPE_8730A			0x1F8

//====================================================
//			EEPROM/Efuse Value Type
//====================================================
#define EETYPE_TX_PWR							0x0
//====================================================
//			EEPROM/Efuse Default Value
//====================================================
#define EEPROM_CID_DEFAULT					0x0
#define EEPROM_CID_DEFAULT_EXT				0xFF // Reserved for Realtek
#define EEPROM_CID_TOSHIBA						0x4
#define EEPROM_CID_CCX							0x10
#define EEPROM_CID_QMI							0x0D
#define EEPROM_CID_WHQL 						0xFE

#define EEPROM_CHANNEL_PLAN_FCC				0x0
#define EEPROM_CHANNEL_PLAN_IC				0x1
#define EEPROM_CHANNEL_PLAN_ETSI				0x2
#define EEPROM_CHANNEL_PLAN_SPAIN			0x3
#define EEPROM_CHANNEL_PLAN_FRANCE			0x4
#define EEPROM_CHANNEL_PLAN_MKK				0x5
#define EEPROM_CHANNEL_PLAN_MKK1				0x6
#define EEPROM_CHANNEL_PLAN_ISRAEL			0x7
#define EEPROM_CHANNEL_PLAN_TELEC			0x8
#define EEPROM_CHANNEL_PLAN_GLOBAL_DOMAIN	0x9
#define EEPROM_CHANNEL_PLAN_WORLD_WIDE_13	0xA
#define EEPROM_CHANNEL_PLAN_NCC_TAIWAN		0xB
#define EEPROM_CHANNEL_PLAN_CHIAN			0XC
#define EEPROM_CHANNEL_PLAN_SINGAPORE_INDIA_MEXICO  0XD
#define EEPROM_CHANNEL_PLAN_KOREA			0xE
#define EEPROM_CHANNEL_PLAN_TURKEY              	0xF
#define EEPROM_CHANNEL_PLAN_JAPAN                 	0x10
#define EEPROM_CHANNEL_PLAN_FCC_NO_DFS		0x11
#define EEPROM_CHANNEL_PLAN_JAPAN_NO_DFS	0x12
#define EEPROM_CHANNEL_PLAN_WORLD_WIDE_5G	0x13
#define EEPROM_CHANNEL_PLAN_TAIWAN_NO_DFS 	0x14

#define EEPROM_USB_OPTIONAL1					0xE
#define EEPROM_CHANNEL_PLAN_BY_HW_MASK		0x80

#define RTL_EEPROM_ID							0x8730
#define EEPROM_Default_TSSI						0x0
#define EEPROM_Default_BoardType				0x02
#define EEPROM_Default_ThermalMeter			0x12
#define EEPROM_Default_ThermalMeter_92SU		0x7
#define EEPROM_Default_ThermalMeter_88E		0x18
#define EEPROM_Default_ThermalMeter_8812		0x18
#define	EEPROM_Default_ThermalMeter_8192E			0x1A
#define	EEPROM_Default_ThermalMeter_8723B		0x18
#define	EEPROM_Default_ThermalMeter_8730A		0x1A


#define EEPROM_Default_CrystalCap				0x0
#define EEPROM_Default_CrystalCap_8723B			0x20
#define EEPROM_Default_CrystalCap_8730A			0x38
#define EEPROM_Default_CrystalFreq				0x0
#define EEPROM_Default_TxPowerLevel_92C		0x22
#define EEPROM_Default_TxPowerLevel_2G			0x2C
#define EEPROM_Default_TxPowerLevel_5G			0x22
#define EEPROM_Default_TxPowerLevel			0x22
#define EEPROM_Default_HT40_2SDiff				0x0
#define EEPROM_Default_HT20_Diff				2
#define EEPROM_Default_LegacyHTTxPowerDiff		0x3
#define EEPROM_Default_LegacyHTTxPowerDiff_92C	0x3
#define EEPROM_Default_LegacyHTTxPowerDiff_92D	0x4
#define EEPROM_Default_HT40_PwrMaxOffset		0
#define EEPROM_Default_HT20_PwrMaxOffset		0

#define EEPROM_Default_PID						0x1234
#define EEPROM_Default_VID						0x5678
#define EEPROM_Default_CustomerID				0xAB
#define EEPROM_Default_CustomerID_8188E		0x00
#define EEPROM_Default_SubCustomerID			0xCD
#define EEPROM_Default_Version					0

#define EEPROM_Default_externalPA_C9		0x00
#define EEPROM_Default_externalPA_CC		0xFF
#define EEPROM_Default_internalPA_SP3T_C9	0xAA
#define EEPROM_Default_internalPA_SP3T_CC	0xAF
#define EEPROM_Default_internalPA_SPDT_C9	0xAA
#define EEPROM_Default_internalPA_SPDT_CC	0xFA
#define EEPROM_Default_PAType						0
#define EEPROM_Default_LNAType						0

//New EFUSE deafult value
#define EEPROM_DEFAULT_24G_CCK_INDEX		0x40
#define EEPROM_DEFAULT_24G_40M_INDEX		0x40
#define EEPROM_DEFAULT_24G_HT20_DIFF		0X00
#define EEPROM_DEFAULT_24G_OFDM_DIFF		0X02

#define EEPROM_DEFAULT_5G_INDEX				0X44
#define EEPROM_DEFAULT_5G_HT20_DIFF			0X00
#define EEPROM_DEFAULT_5G_OFDM_DIFF			0X04

#define EEPROM_DEFAULT_DIFF					0XFE
#define EEPROM_DEFAULT_CHANNEL_PLAN			0x7F
#define EEPROM_DEFAULT_BOARD_OPTION			0x01	// Enable power by rate and power limit
#define EEPROM_DEFAULT_RFE_OPTION			0x04
#define EEPROM_DEFAULT_FEATURE_OPTION		0x00
#define EEPROM_DEFAULT_BT_OPTION			0x10
#define EEPROM_DEFAULT_TX_CALIBRATE_RATE	0x00


/*
 * For VHT series TX power by rate table.
 * VHT TX power by rate off setArray =
 * Band:-2G&5G = 0 / 1
 * RF: at most 4*4 = ABCD=0/1/2/3
 * CCK=0 OFDM=1/2 HT-MCS 0-15=3/4/56 VHT=7/8/9/10/11
 *   */
#define TX_PWR_BY_RATE_NUM_BAND			2
#define TX_PWR_BY_RATE_NUM_RF			4
#define TX_PWR_BY_RATE_NUM_RATE			84

#define TXPWR_LMT_MAX_RF			4

/* ----------------------------------------------------------------------------
 * EEPROM/EFUSE data structure definition.
 * ---------------------------------------------------------------------------- */

/* For 88E new structure */

/*
2.4G:
{
{1,2},
{3,4,5},
{6,7,8},
{9,10,11},
{12,13},
{14}
}

5G:
{
{36,38,40},
{44,46,48},
{52,54,56},
{60,62,64},
{100,102,104},
{108,110,112},
{116,118,120},
{124,126,128},
{132,134,136},
{140,142,144},
{149,151,153},
{157,159,161},
{173,175,177},
}
*/
#define	MAX_RF_PATH				4
#define RF_PATH_MAX				MAX_RF_PATH
#define	MAX_CHNL_GROUP_24G		6
#define	MAX_CHNL_GROUP_5G		14

/* It must always set to 4, otherwise read efuse table sequence will be wrong. */
#define	MAX_TX_COUNT				4

typedef	enum _BT_Ant_NUM {
	Ant_x2	= 0,
	Ant_x1	= 1
} BT_Ant_NUM, *PBT_Ant_NUM;

typedef	enum _BT_CoType {
	BT_2WIRE		= 0,
	BT_ISSC_3WIRE	= 1,
	BT_ACCEL		= 2,
	BT_CSR_BC4		= 3,
	BT_CSR_BC8		= 4,
	BT_RTL8756		= 5,
	BT_RTL8723A		= 6,
	BT_RTL8821		= 7,
	BT_RTL8723B		= 8,
	BT_RTL8192E		= 9,
	BT_RTL8814A		= 10,
	BT_RTL8812A		= 11,
	BT_RTL8703B		= 12,
	BT_RTL8822B		= 13,
	BT_RTL8723D		= 14,
	BT_RTL8821C		= 15,
	BT_RTL8192F		= 16,
	BT_RTL8822C		= 17,
	BT_RTL8814B		= 18,
	BT_RTL8730A		= 19,
} BT_CoType, *PBT_CoType;

typedef	enum _BT_RadioShared {
	BT_Radio_Shared	= 0,
	BT_Radio_Individual	= 1,
} BT_RadioShared, *PBT_RadioShared;


#endif
