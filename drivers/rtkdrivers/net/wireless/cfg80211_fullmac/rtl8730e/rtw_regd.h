// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek wireless local area network IC driver.
*   This is an interface between cfg80211 and firmware in other core. The
*   commnunication between driver and firmware is IPC（Inter Process
*   Communication）bus.
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#ifndef __RTW_REGD_H__
#define __RTW_REGD_H__

/* for kernel 3.14 , both value are changed to IEEE80211_CHAN_NO_IR*/
#define IEEE80211_CHAN_NO_IBSS  IEEE80211_CHAN_NO_IR
#define IEEE80211_CHAN_PASSIVE_SCAN IEEE80211_CHAN_NO_IR

/*
 * Only these channels all allow active
 * scan on all world regulatory domains
 */
#define RTL819x_2GHZ_CH01_11    REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)

/*
 * We enable active scan on these a case
 * by case basis by regulatory domain
 */
#define RTL819x_2GHZ_CH12_13    REG_RULE(2467-10, 2472+10, 40, 0, 20, NL80211_RRF_PASSIVE_SCAN)

#define RTL819x_2GHZ_CH14       REG_RULE(2484-10, 2484+10, 40, 0, 20, NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_OFDM)

/* 5G chan 36 - chan 64*/
#define RTL819x_5GHZ_5150_5350  REG_RULE(5150-10, 5350+10, 80, 0, 30, 0)
/* 5G chan 100 - chan 165*/
#define RTL819x_5GHZ_5470_5850  REG_RULE(5470-10, 5830+10, 80, 0, 30, 0)
/* 5G chan 149 - chan 165*/
#define RTL819x_5GHZ_5725_5850  REG_RULE(5725-10, 5830+10, 80, 0, 30, 0)

#define RTL819x_5GHZ_ALL        (RTL819x_5GHZ_5150_5350, RTL819x_5GHZ_5470_5850)

#define COUNTRY_CHPLAN_ENT(_alpha2, _chplan, _pwr_lmt) \
	{.alpha2 = (_alpha2), \
	 .chplan = (_chplan), \
	 .pwr_lmt = (_pwr_lmt) \
	}

enum country_code_type_t {
	COUNTRY_CODE_FCC = 0,
	COUNTRY_CODE_IC = 1,
	COUNTRY_CODE_ETSI = 2,
	COUNTRY_CODE_SPAIN = 3,
	COUNTRY_CODE_FRANCE = 4,
	COUNTRY_CODE_MKK = 5,
	COUNTRY_CODE_MKK1 = 6,
	COUNTRY_CODE_ISRAEL = 7,
	COUNTRY_CODE_TELEC = 8,
	COUNTRY_CODE_MIC = 9,
	COUNTRY_CODE_GLOBAL_DOMAIN = 10,
	COUNTRY_CODE_WORLD_WIDE_13 = 11,
	COUNTRY_CODE_TELEC_NETGEAR = 12,
	COUNTRY_CODE_WORLD_WIDE_13_5G_ALL = 13,

	/*add new channel plan above this line */
	COUNTRY_CODE_MAX
};

struct country_chplan {
	char alpha2[2];
	u8 chplan;
	u8 pwr_lmt;
};

#endif //__RTW_REGD_H__
