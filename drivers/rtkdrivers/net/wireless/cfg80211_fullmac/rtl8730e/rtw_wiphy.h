#ifndef __RTW_WIPHY_H__
#define __RTW_WIPHY_H__

#define RTW_SSID_SCAN_AMOUNT		1 /* for WEXT_CSCAN_AMOUNT 9 */
#define RTW_SCAN_IE_LEN_MAX      	2304
#define RTW_MAX_NUM_PMKIDS		4
#define	MAX_CHANNEL_NUM_2G		14
#define	MAX_CHANNEL_NUM_5G		28
#define RTW_G_RATES_NUM			12
#define RTW_A_RATES_NUM			8

/*cipher suite from 802.11-2016 p884*/
#define WIFI_CIPHER_SUITE_WEP_40	0x000FAC01
#define WIFI_CIPHER_SUITE_TKIP		0x000FAC02
#define WIFI_CIPHER_SUITE_CCMP_128	0x000FAC04
#define WIFI_CIPHER_SUITE_WEP_104	0x000FAC05
#define WIFI_CIPHER_SUITE_BIP_CMAC_128	0x000FAC06
#define WIFI_CIPHER_SUITE_GCMP		0x000FAC08
#define WIFI_CIPHER_SUITE_GCMP_256	0x000FAC09
#define WIFI_CIPHER_SUITE_CCMP_256	0x000FAC0A
#define WIFI_CIPHER_SUITE_BIP_GMAC_128	0x000FAC0B
#define WIFI_CIPHER_SUITE_BIP_GMAC_256	0x000FAC0C
#define WIFI_CIPHER_SUITE_BIP_CMAC_256	0x000FAC0D

#define WIFI_AKM_SUITE_PSK		0x000FAC02
#define WIFI_AKM_SUITE_SAE		0x000FAC08

/*self-defined cipher suite used in driver*/
#define _NO_PRIVACY_		0x0
#define _WEP40_				0x1
#define _TKIP_				0x2
#define _TKIP_WTMIC_		0x3
#define _AES_				0x4
#define _WEP104_			0x5
#define _WEP_WPA_MIXED_		0x07  // WEP + WPA
#define _SMS4_				0x06
#define _BIP_				0x8
#define _GCMP_ 0x07
#define _GCMP_256_ (_GCMP_ | BIT(3))
#define _CCMP_256_ (_AES_ | BIT(3))

#define RTW_SSID_SCAN_AMOUNT		1 //LINUX_TODO
#define RTW_CHANNEL_SCAN_AMOUNT		(14+37)

#define ASSOC_IE_MAX_LEN		500
#define WLAN_HDR_A3_LEN			24
#define RSNXE_MAX_LEN (18)
#define WLAN_EID_RSNX 244

#define rtw_a_rates			(rtw_rates + 4)
#define rtw_g_rates			(rtw_rates + 0)

#define RATETAB_ENT(_rate, _rateid, _flags) \
	{								\
		.bitrate		= (_rate),			\
		.hw_value		= (_rateid),			\
		.flags			= (_flags),			\
	}

#define CHAN2G(_channel, _freq, _flags) {				\
		.band			= NL80211_BAND_2GHZ,		\
		.center_freq		= (_freq),			\
		.hw_value		= (_channel),			\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}

#define CHAN5G(_channel, _flags) {					\
		.band			= NL80211_BAND_5GHZ,		\
		.center_freq		= 5000 + (5 * (_channel)),	\
		.hw_value		= (_channel),			\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}

#endif //__RTW_WIPHY_H__