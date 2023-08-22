#ifndef __RTW_LLHW_EVENT_H__
#define __RTW_LLHW_EVENT_H__

#define WEP_ENABLED			0x0001
#define TKIP_ENABLED			0x0002
#define AES_ENABLED			0x0004
#define AES_CMAC_ENABLED		0x0010
#define ENTERPRISE_ENABLED		0x0020
#define SHARED_ENABLED			0x00008000
#define WPA_SECURITY			0x00200000
#define WPA2_SECURITY			0x00400000
#define WPA3_SECURITY			0x00800000
#define WPS_ENABLED			0x10000000

/**
  * @brief  The enumeration lists the possible security types to set when connection.\n
  *			Station mode supports OPEN, WEP, and WPA2.\n
  *			AP mode support OPEN and WPA2.
  */
enum {
	RTW_SECURITY_OPEN		= 0,							/**< Open security                           */
	RTW_SECURITY_WEP_PSK		= WEP_ENABLED,						/**< WEP Security with open authentication   */
	RTW_SECURITY_WEP_SHARED		= (WEP_ENABLED | SHARED_ENABLED),			/**< WEP Security with shared authentication */
	RTW_SECURITY_WPA_TKIP_PSK	= (WPA_SECURITY  | TKIP_ENABLED),			/**< WPA Security with TKIP                  */
	RTW_SECURITY_WPA_AES_PSK	= (WPA_SECURITY  | AES_ENABLED),			/**< WPA Security with AES                   */
	RTW_SECURITY_WPA2_AES_PSK	= (WPA2_SECURITY | AES_ENABLED),			/**< WPA2 Security with AES                  */
	RTW_SECURITY_WPA2_TKIP_PSK	= (WPA2_SECURITY | TKIP_ENABLED),			/**< WPA2 Security with TKIP                 */
	RTW_SECURITY_WPA2_MIXED_PSK	= (WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED),		/**< WPA2 Security with AES & TKIP           */
	RTW_SECURITY_WPA_WPA2_MIXED	= (WPA_SECURITY  | WPA2_SECURITY),			/**< WPA/WPA2 Security                       */
	RTW_SECURITY_WPA2_AES_CMAC	= (WPA2_SECURITY | AES_CMAC_ENABLED),			/**< WPA2 Security with AES and Management Frame Protection*/
	RTW_SECURITY_WPA2_ENTERPRISE	= (WPA2_SECURITY | ENTERPRISE_ENABLED),			/**< WPA2 Security with 802.1X authentication>*/
	RTW_SECURITY_WPA_WPA2_ENTERPRISE = (WPA_SECURITY | WPA2_SECURITY | ENTERPRISE_ENABLED),	/** <WPA/WPA2 Security with 802.1X authentication>*/

	RTW_SECURITY_WPS_OPEN		= WPS_ENABLED,						/**< WPS with open security                  */
	RTW_SECURITY_WPS_SECURE		= (WPS_ENABLED | AES_ENABLED),				/**< WPS with AES security                   */

	RTW_SECURITY_WPA3_AES_PSK	= (WPA3_SECURITY | AES_ENABLED),			/**< WPA3-SAE with AES security              */
	RTW_SECURITY_WPA2_WPA3_MIXED	= (WPA2_SECURITY | WPA3_SECURITY | AES_ENABLED),	/**< WPA3-SAE/WPA2 with AES security         */

	RTW_SECURITY_UNKNOWN		= (unsigned long) -1,					/**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

	RTW_SECURITY_FORCE_32_BIT	= 0x7fffffff						/**< Exists only to force rtw_security_t type to 32 bits */
};
typedef unsigned long rtw_security_t;

/**
  * @brief  The enumeration lists the scan options.
  */
enum {
	RTW_SCAN_ACTIVE             	 = 0x01,
	RTW_SCAN_PASSIVE            	 = 0x02,
	RTW_SCAN_NO_HIDDEN_SSID     	 = 0x04,						/**< Filter hidden ssid APs*/
	RTW_SCAN_REPORT_EACH        	 = 0x08,
};
typedef unsigned int rtw_scan_option_t;

/**
  * @brief  The enumeration is event type indicated from wlan driver.
  */
enum _WIFI_EVENT_INDICATE {
	/* common event */
	WIFI_EVENT_STA_ASSOC = 0,		///<used in p2p, simple config, 11s, customer
	WIFI_EVENT_STA_DISASSOC,		///<used in p2p, customer
	WIFI_EVENT_RX_MGNT,				///<used in p2p, customer
	WIFI_EVENT_RX_MGNT_AP,
	WIFI_EVENT_CONNECT,				///<used in ipv6 example, p2p, wifi_manager example
	WIFI_EVENT_DISCONNECT,			///<used in wifi_manager example, p2p, eap
	WIFI_EVENT_EXTERNAL_AUTH_REQ,

	WIFI_EVENT_GROUP_KEY_CHANGED,
	WIFI_EVENT_RECONNECTION_FAIL,
	WIFI_EVENT_ICV_ERROR,
	WIFI_EVENT_CHALLENGE_FAIL,

	WIFI_EVENT_JOIN_STATUS,			///<internally use for joinstatus indicate

	/* wpa event */
	WIFI_EVENT_WPA_STA_WPS_START,
	WIFI_EVENT_WPA_WPS_FINISH,
	WIFI_EVENT_WPA_EAPOL_START,
	WIFI_EVENT_WPA_EAPOL_RECVD,
	WIFI_EVENT_WPA_STA_4WAY_START,
	WIFI_EVENT_WPA_AP_4WAY_START,
	WIFI_EVENT_WPA_STA_4WAY_RECV,
	WIFI_EVENT_WPA_AP_4WAY_RECV,

	/* csi rx done event */
	WIFI_EVENT_CSI_DONE,

	WIFI_EVENT_MAX,
};
typedef unsigned long rtw_event_indicate_t;

#define INIC_MAX_SSID_LENGTH (33)

#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/* SET pack mode 1-alignment for the following area. */
#pragma pack(1)
#endif

/**
  * @brief  The structure is used to describe the SSID.
  */
typedef struct rtw_ssid {
	unsigned char		len;     /**< SSID length */
	unsigned char		val[INIC_MAX_SSID_LENGTH]; /**< SSID name (AP name)  */
} rtw_ssid_t;

/**
  * @brief  The structure is used to describe the unique 6-byte MAC address.
  */
typedef struct rtw_mac {
	unsigned char		octet[6]; /**< Unique 6-byte MAC address */
} rtw_mac_t;

/**
  * @brief  The structure is used to describe the scan result of the AP.
  */
typedef struct rtw_scan_result {
	rtw_ssid_t		SSID;             /**< Service Set Identification (i.e. Name of Access Point)                    */
	rtw_mac_t		BSSID;            /**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
	signed short		signal_strength;  /**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
	unsigned long		bss_type;         /**< Network type: linux not used, delete struct definition and keep size      */
	rtw_security_t		security;         /**< Security type                                                             */
	unsigned long		wps_type;         /**< WPS type: linux not used, delete struct definition and keep size          */
	unsigned int		channel;          /**< Radio channel that the AP beacon was received on                          */
	unsigned long		band;             /**< Radio band: : linux not used, delete struct definition and keep size      */
} rtw_scan_result_t;

#if defined(__IAR_SYSTEMS_ICC__) || defined (__GNUC__) || defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/* REMOVE pack mode 1-alignment for the next definitions, use DEFAULT. */
#pragma pack()
#endif

/**
 * @brief  The enumeration is join block param.
 */
typedef struct {
	struct completion	join_sema;
	unsigned int		join_timeout;
	unsigned char		block;
} internal_join_block_param_t;

/**
  * @brief  The structure is used for fullmac to get wpa_supplicant's info for STA connect,
  */
typedef struct rtw_wpa_supp_connect {
	u8 rsnxe_ie[RSNXE_MAX_LEN];
} rtw_wpa_supp_connect_t;

/**
  * @brief  The structure is used to describe the setting about SSID,
  *			security type, password and default channel, used to start AP mode.
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
typedef struct rtw_softap_info {
	rtw_ssid_t		ssid;
	unsigned char		hidden_ssid;
	rtw_security_t		security_type;
	unsigned char 		*password;
	unsigned char 		password_len;
	unsigned char		channel;
} rtw_softap_info_t;

/* rtos: typedef void (*rtw_joinstatus_callback_t)(rtw_join_status_t join_status, void *user_data, unsigned int user_data_len); */
typedef void (*rtw_joinstatus_callback_t)(unsigned int join_status, void *user_data, unsigned int user_data_len);

/**
  * @brief  The structure is used to describe the connection setting about SSID,
  *			security type and password,etc., used when connecting to an AP.
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  * @note  If this struct is used for wifi connect, the channel is used to config
  *        whether it is a full channel scan(when channel is set to 0), or it will
  *        only scan one channel(do active scan on the configured channel).
  * @note  pscan_option set to PSCAN_FAST_SURVEY means do fast survey on the specified channel
  *        set to 0 means do normal scan on the specified channel or full channel.
  */
typedef struct rtw_network_info {
	rtw_ssid_t 		ssid;
	rtw_mac_t		bssid;
	rtw_security_t		security_type;
	unsigned char 		*password;
	int 			password_len;
	int			key_id;
	unsigned char		channel;        /**< set to 0 means full channel scan, set to other value means only scan on the specified channel */
	unsigned char		pscan_option;   /**< used when the specified channel is set, set to 0 for normal partial scan, set to PSCAN_FAST_SURVEY for fast survey*/
	rtw_joinstatus_callback_t joinstatus_user_callback;   /**< user callback for processing joinstatus, please set to NULL if not use it */
	rtw_wpa_supp_connect_t	wpa_supp;
} rtw_network_info_t;

typedef struct {
	unsigned short		active_scan_time;      /**< active scan time per channel, units: millisecond, default is 100ms */
	unsigned short		passive_scan_time;     /**< passive scan time per channel, units: millisecond, default is 110ms */
} rtw_channel_scan_time_t;

/* DO NOT define or use any rtw_result_t in linux. Use asm-generic/errno.h instead. */
/* rtos definition: typedef rtw_result_t (*scan_user_callback_t)(unsigned int ap_num, void *user_data); */
typedef long int (*scan_user_callback_t)(unsigned int ap_num, void *user_data);
/* rtos definition: typedef rtw_result_t (*scan_report_each_mode_user_callback_t)(rtw_scan_result_t *scanned_ap_info, void *user_data); */
typedef long int (*scan_report_each_mode_user_callback_t)(rtw_scan_result_t *scanned_ap_info, void *user_data);

/**
  * @brief  The structure is used to describe the scan parameters used for scan,
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
typedef struct rtw_scan_param {
	rtw_scan_option_t	options;
	char			*ssid;
	unsigned char		*channel_list;
	unsigned char		channel_list_num;
	rtw_channel_scan_time_t	chan_scan_time;
	unsigned short		max_ap_record_num;     /**< config the max number of recorded AP, when set to 0, use default value 64 */
	void			*scan_user_data;
	scan_user_callback_t	scan_user_callback;   /**< used for normal asynchronized mode */
	scan_report_each_mode_user_callback_t scan_report_each_mode_user_callback; /*used for RTW_SCAN_REPORT_EACH mode */
} rtw_scan_param_t;

typedef struct raw_data_desc {
	unsigned char		wlan_idx;      /**< index of wlan interface which will transmit */
	unsigned char		*buf;          /**< poninter of buf where raw data is stored*/
	unsigned short		buf_len;      /**< the length of raw data*/
	unsigned short		flags;        /**< send options*/
	unsigned char		tx_rate;        /**< specific tx rate, please refer to enum MGN_RATE in wifi_constants.h*/
	unsigned char		retry_limit;   /**< retry limit configure, when set to 0, will use default retry limit 12*/
	unsigned int		tx_power;
} raw_data_desc_t;

struct rtw_crypt_info {
	u8			pairwise;
	u8			mac_addr[6];
	u8			wlan_idx;
	u16			key_len;
	u8			key[32];
	u8			key_idx;
	u32			cipher;
	u8			transition_disable_exist;
	u8			transition_disable_bitmap;
};

/**
  * @brief  The structure is used to describe the phy statistics
  */
typedef struct {
	signed char		rssi;          /*!<average rssi in 1 sec (for STA mode) */
	signed char		snr;          /*!< average snr in 1 sec (not include cck rate, for STA mode)*/
	unsigned int		false_alarm_cck;
	unsigned int		false_alarm_ofdm;
	unsigned int		cca_cck;
	unsigned int		cca_ofdm;
	unsigned int		tx_retry;
	unsigned short		tx_drop;
	unsigned int		rx_drop;
	unsigned int		supported_max_rate;
} rtw_phy_statistics_t;

#endif //__RTW_LLHW_EVENT_H__
