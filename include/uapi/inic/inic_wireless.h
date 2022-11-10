/*
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
 /******************************************************************************
  * history *
 ******************************************************************************/

#ifndef __INIC_WIRELESS_H__
#define __INIC_WIRELESS_H__

/* -------------------------------- Includes -------------------------------- */
/* external head files */
#include <linux/wireless.h>

/* internal head files */
#include "wifi_type/wifi_structures.h"

/* -------------------------------- Defines --------------------------------- */
/* ioctl cmd, from userspace to kernal space */
#define RINICIO_CMD (SIOCDEVPRIVATE + 1) /* inic cmd for wifi_conf.h */
#define RINICIO_EXT_CMD (SIOCDEVPRIVATE + 2) /* inic cmd for wifi_conf_ext.h */
#define RINICIO_PRMC_CMD (SIOCDEVPRIVATE + 3) /* inic cmd for wifi promisc */

/* define some private IOCTL options which are not in inic_io_cmd_t */
#define RINICIO_S_START (0x0000) /* ioctl start to set */
#define RINICIO_S_SACN (0x0001) /* ioctl to scan*/
#define RINICIO_S_CONNECT (0x0002)  /* ioctl to connet without block */
#define RINICIO_S_DISCONNECT (0x0003) /* ioctl to disconnect */
#define RINICIO_S_WIFION (0x0004) /* ioctl to set wifi on */
#define RINICIO_S_WIFIOFF (0x0005) /* ioctl to set wifi off */
#define RINICIO_S_STARTAP (0x0006) /* ioctl to set start softap */
#define RINICIO_S_SCANABORT (0x0007) /* ioctl to set scan abort */
#define RINICIO_S_CHANNEL (0x0008) /* ioctl to set channel */
#define RINICIO_S_MODE (0x0009) /* ioctl to set mode */
#define RINICIO_S_REGEVENT (0x000A) /* ioctl to register event handdler */
#define RINICIO_S_UNREGEVENT (0x000B) /* ioctl to unregister event handdler */
#define RINICIO_S_INDEVENT (0x000C) /* ioctl to set indicate event */
#define RINICIO_S_END (0x000D) /* ioctl end to set */

#define RINICIO_G_START (0x1000) /* ioctl start to get */
#define RINICIO_G_JOINSTAUTS (0x1001) /* ioctl to get joint status */
#define RINICIO_G_SCANNNEDAPNUM (0x1002) /* ioctl to get joint status */
#define RINICIO_G_SCANRESULT (0x1003) /* ioctl to get joint status */
#define RINICIO_G_ISCONNECTTOAP (0x1004) /* ioctl to get whether wifi is connected */
#define RINICIO_G_MODE (0x1005) /* ioctl to get wifi mode */
#define RINICIO_G_WIFIISRUNNING (0x1006) /* ioctl to get if wifi is running */
#define RINICIO_G_CHANNEL (0x1007) /* ioctl to get channel */
#define RINICIO_G_DISREASONCODE (0x1008) /* ioctl to get disconnecced reason code */
#define RINICIO_G_END (0x1009) /* ioctl end to get */

#define RINICIO_ES_START (0x2000) /* ioctl start to set ext wifi */
#define RINICIO_ES_PSKINFO (0x2001) /* ioctl to set psk information */
#define RINICIO_ES_COEXBLESCANDUTY (0x2002) /* ioctl to set ble scan duty in bt-coexsitance */
#define RINICIO_ES_POWERSAVEMODE (0x2003) /* ioctl to set powersaving mode */
#define RINICIO_ES_MFPSUPPORT (0x2004) /* ioctl to set mfp supporting */
#define RINICIO_ES_GROUPID (0x2005) /* ioctl to set group id */
#define RINICIO_ES_PMKCACHEENABLE (0x2006) /* ioctl to set to enable pmk caching */
#define RINICIO_ES_NETWORKMODE (0x2007) /* ioctl to set network mode */
#define RINICIO_ES_WPSPHASE (0x2008) /* ioctl to set wps phase */
#define RINICIO_ES_GENIE (0x2009) /* ioctl to set gen ie */
#define RINICIO_ES_EAPPHASE (0x200A) /* ioctl to set eap phase */
#define RINICIO_ES_EAPMETHOD (0x200B) /* ioctl to set eap method */
#define RINICIO_ES_SENDEAPOL (0x200C) /* ioctl to send eapol */
#define RINICIO_ES_CUSTOMIE (0x200D) /* ioctl to add custom ie */
#define RINICIO_ES_INDICATEMGNT (0x200E) /* ioctl to set indicate mgnt */
#define RINICIO_ES_SENDRAWFRAME (0x200F) /* ioctl to send raw frame */
#define RINICIO_ES_TXRATEBYTOS (0x2010) /* ioctl to set tx rate by Tos */
#define RINICIO_ES_EDCAPARAM (0x2011) /* ioctl to set EDCA param */
#define RINICIO_ES_TXCCA (0x2012) /* ioctl to set TX CCA */
#define RINICIO_ES_CTS2SELF (0x2013) /* ioctl to set CTS 2 self duration and send it */
#define RINICIO_ES_INITMACFILTER (0x2014) /* ioctl to set initialize the mac filter */
#define RINICIO_ES_ADDMACFILTER (0x2015) /* ioctl to add mac filter */
#define RINICIO_ES_DELMACFILTER (0x2016) /* ioctl to delete mac filter */
#define RINICIO_ES_DELSTA (0x2017) /* ioctl to delete sation for softap */
#define RINICIO_ES_APSWITCHCHLANDINFORM (0x2018) /* ioctl to switch channel and inform stations for softap */
#define RINICIO_ES_AUTORECONNECT (0x2019) /* ioctl to configure autoreconnect */
#define RINICIO_ES_NOBEACONTIMEOUT (0x201A) /* ioctl to set no beacon timeout */
#define RINICIO_ES_END (0x201B) /* ioctl end to set ext wifi */

#define RINICIO_EG_START (0x3000) /* ioctl to get ext wifi */
#define RINICIO_EG_PSKINFO (0x3001) /* ioctl to get psk information */
#define RINICIO_EG_MACADDR (0x3002) /* ioctl to get MAC address */
#define RINICIO_EG_ISMP (0x3003) /* ioctl to get whether the firm is MP */
#define RINICIO_EG_ASSOCCLIENTLIST (0x3004) /* ioctl to get the list of associated client */
#define RINICIO_EG_WIFISETTING (0x3005) /* ioctl to get wifi setting */
#define RINICIO_EG_SWSTATISTIC (0x3006) /* ioctl to get software statistic in KM4 driver */
#define RINICIO_EG_PHYTATISTIC (0x3007) /* ioctl to get physic statistic from BBRF */
#define RINICIO_EG_EAPPHASE (0x3008) /* ioctl to get eap phase */
#define RINICIO_EG_ANTANNAINFO (0x3009) /* ioctl to get antenna information */
#define RINICIO_EG_BANDTYPE (0x300A) /* ioctl to get band type */
#define RINICIO_EG_AUTOCHANNEL (0x300B) /* ioctl to get auto channel for channel selection */
#define RINICIO_EG_AUTORECONNECT (0x300C) /* ioctl to get the configuration of autoreconnectting */
#define RINICIO_EG_END (0x300D) /* ioctl end to get ext wifi */

#define RINICIO_P_START (0x4000) /* ioctl start for promisc */
#define RINICIO_P_SETPROMISC (0x4001) /* ioctl to set promisc */
#define RINICIO_P_ENTERPROMISCMODE (0x4002) /* ioctl to enter promisc mode */
#define RINICIO_P_INITPACKETFILTER (0x4003) /* ioctl to initilaze packet filter */
#define RINICIO_P_ADDPACKETFILTER (0x4004) /* ioctl to add packet filter */
#define RINICIO_P_ENPACKETFILTER (0x4005) /* ioctl to enable packet filter */
#define RINICIO_P_DISPACKETFILTER (0x4006) /* ioctl to disable packet filter */
#define RINICIO_P_RMPACKETFILTER (0x4007) /* ioctl to remove packet filter */
#define RINICIO_P_RETXPACKETFILTER (0x4008) /* ioctl to retransmit packet filter */
#define RINICIO_P_FILTERBYAPANDMAC (0x4009) /* ioctl to filter by AP and phone MAC */
#define RINICIO_P_FILTERWITHLEN (0x400A) /* ioctl to filter with length */
#define RINICIO_P_ISPROMISCENABLE (0x400B) /* ioctl to get whether promisc mode is enable */
#define RINICIO_P_GETFIXEDCHANNEL (0x400C) /* ioctl to get promisc fixed channel */
#define RINICIO_P_SETMGNTFRAME (0x400D) /* ioctl to set mgnt frame */
#define RINICIO_P_GETCHNLBYBSSID (0x400E) /* ioctl to get channel by bssid */
#define RINICIO_P_UPDATECANDIAPRSSIAVG (0x400F) /* ioctl to get avrage RSSI of cadidate AP */
#define RINICIO_P_STOPTXBEACON (0x4010) /* ioctl to stop TX beacon */
#define RINICIO_P_RESUMETXBEACON (0x4011) /* ioctl to resume TX beacon */
#define RINICIO_P_ISSUEPROBERSP (0x4012) /* ioctl to issue probe response */
#define RINICIO_P_END (0x4013) /* ioctl end for promisc */

#define RINICIO_NL_MAX_PAYLOAD 2048  /* maximum payload size*/

#define RINICIO_NL_NOTIFY_EVENT 19
#define RINICIO_NL_NOTIFY_GPID 0x21

/* inic signal number, not ioctl!!! */
#define RINICIO_SIG_WIFIEVENT (45) /* signal to notify scan done */

/* the flags of inic ioctl cmd */
#define RINICIO_FLAGS_BLOCK (0x01u) /* declare this block cmd, it is busy block for upper layer. */

#ifndef ETH_ALEN
#define ETH_ALEN (6)
#endif

/* event callback number for same event command */
#define WIFI_EVENT_MAX_ROW (3)

/* ------------------------------- Data Types ------------------------------- */
typedef struct inic_io_cmd {
	int cmd;
	void *pointer;
	uint16_t length;
	uint16_t flags;
	int index;
} inic_io_cmd_t;

typedef struct inic_io_scan_arg {
	rtw_scan_param_t *param_ptr;
	int ssid_len;
} inic_io_scan_arg_t;

typedef enum inic_io_event {
	RINICIO_EVT_SCANDONE = 0,
	RINICIO_EVT_SACNEACHREPORT,
	RINICIO_EVT_WIFISTATUS,
	RINICIO_EVT_APCHLSWITCH,
	RINICIO_EVT_AUTORECONNECT,
	RINICIO_EVT_PROMISC,
	RINICIO_EVT_WIFIIND,
	RINICIO_EVT_MAX
} inic_io_event_t;

typedef struct inic_io_reg_event_param {
	rtw_event_indicate_t event_cmd;
	int row_id;
} inic_io_reg_event_param_t;

typedef struct inic_io_ind_event_param {
	rtw_event_indicate_t event_cmd;
	char *buf;
	int buf_len;
	int flags;
} inic_io_ind_event_param_t;

typedef enum inic_wifi_state {
	RINIC_WIFISTS_CLOSE = 0,
	RINIC_WIFISTS_UP,
	RINIC_WIFISTS_JOINSTARTING,
	RINIC_WIFISTS_SCANNING,
	RINIC_WIFISTS_AUTHING,
	RINIC_WIFISTS_AUTHED,
	RINIC_WIFISTS_ASSOCING,
	RINIC_WIFISTS_ASSOCED,
	RINIC_WIFISTS_4WAYING,
	RINIC_WIFISTS_4WAYDONE,
	RINIC_WIFISTS_CONNECTED,
	RINIC_WIFISTS_FAIL,
	RINIC_WIFISTS_DISCONNECT,
	RINIC_WIFISTS_MAX
} inic_wifi_state_t;

typedef struct inic_io_evt_msg {
	inic_io_event_t id;
	char *data;
} inic_io_evt_msg_t;

typedef struct inic_ext_buf_type {
	void *buf;
	uint16_t buf_len;
	uint16_t flags;
} inic_ext_buf_type_t;

typedef struct inic_prmc_setting {
	rtw_rcr_level_t enabled;
	unsigned char cb_en;
	unsigned char len_used;
} inic_prmc_setting_t;

typedef struct inic_prmc_pkt_filter {
	uint8_t filter_id;
	rtw_packet_filter_pattern_t *patt;
	rtw_packet_filter_rule_t rule;
} inic_prmc_pkt_filter_t;

typedef struct inic_prmc_mac_filter {
	uint8_t enable;
	uint8_t ap_mac[ETH_ALEN];
	uint8_t sta_mac[ETH_ALEN];
} inic_prmc_mac_filter_t;

typedef struct inic_prmc_fixed_ch_param {
	uint8_t fixed_bssid[ETH_ALEN];
	uint8_t ssid[INIC_MAX_SSID_LENGTH];
	int ssid_len;
} inic_prmc_fixed_ch_param_t;

/* -------------------------------- Macros ---------------------------------- */
#define INIC_WIFI_STATUS(x) ((x == RINIC_WIFISTS_CLOSE) ? "down" :\
			    ((x == RINIC_WIFISTS_UP) ? "up" :\
			    ((x == RINIC_WIFISTS_JOINSTARTING) ? "join starting" :\
			    ((x == RINIC_WIFISTS_SCANNING) ? "scanning" :\
			    ((x == RINIC_WIFISTS_AUTHING) ? "authorizing" :\
			    ((x == RINIC_WIFISTS_AUTHED) ? "authorized" :\
			    ((x == RINIC_WIFISTS_ASSOCING) ? "associating" :\
			    ((x == RINIC_WIFISTS_ASSOCED) ? "associated" :\
			    ((x == RINIC_WIFISTS_4WAYING) ? "4 way handshaking" :\
			    ((x == RINIC_WIFISTS_4WAYDONE) ? "4 way handshake done" :\
			    ((x == RINIC_WIFISTS_CONNECTED) ? "connected" :\
			    ((x == RINIC_WIFISTS_FAIL) ? "failed" :\
			    ((x == RINIC_WIFISTS_DISCONNECT) ? "disconnet" :\
			    "unknown")))))))))))))

/* -------------------------- Function declaration -------------------------- */
/*
 * convert rtw_join_status_t to inic_wifi_state_t.
 * @join_status: input join status, refer to rtw_join_status_t
 */
static inline inic_wifi_state_t join_status_to_wifi_status(rtw_join_status_t join_status)
{
	inic_wifi_state_t ret = RINIC_WIFISTS_CLOSE;
	switch (join_status) {
	case RTW_JOINSTATUS_UNKNOWN:
		ret = RINIC_WIFISTS_UP;
		break;
	case RTW_JOINSTATUS_STARTING:
		ret = RINIC_WIFISTS_JOINSTARTING;
		break;
	case RTW_JOINSTATUS_SCANNING:
		ret = RINIC_WIFISTS_SCANNING;
		break;
	case RTW_JOINSTATUS_AUTHENTICATING:
		ret = RINIC_WIFISTS_AUTHING;
		break;
	case RTW_JOINSTATUS_AUTHENTICATED:
		ret = RINIC_WIFISTS_AUTHED;
		break;
	case RTW_JOINSTATUS_ASSOCIATING:
		ret = RINIC_WIFISTS_ASSOCING;
		break;
	case RTW_JOINSTATUS_ASSOCIATED:
		ret = RINIC_WIFISTS_ASSOCED;
		break;
	case RTW_JOINSTATUS_4WAY_HANDSHAKING:
		ret = RINIC_WIFISTS_4WAYING;
		break;
	case RTW_JOINSTATUS_4WAY_HANDSHAKE_DONE:
		ret = RINIC_WIFISTS_4WAYDONE;
		break;
	case RTW_JOINSTATUS_SUCCESS:
		ret = RINIC_WIFISTS_CONNECTED;
		break;
	case RTW_JOINSTATUS_FAIL:
		ret = RINIC_WIFISTS_FAIL;
		break;
	case RTW_JOINSTATUS_DISCONNECT:
		ret = RINIC_WIFISTS_DISCONNECT;
		break;
	default:
		ret = RINIC_WIFISTS_UP;
		break;
	}

	return ret;
}

#endif /* __INIC_WIRELESS_H__ */
