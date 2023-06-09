/******************************************************************************
 *
 * Copyright(c) 2007 - 2021 Realtek Corporation.
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
#ifndef __DRV_CONF_H__
#define __DRV_CONF_H__

#define TOTAL_IFACE_NUM 2

#define CONFIG_SINGLE_IMG

/*#define CONFIG_NO_FW*/
/*#define CONFIG_DISABLE_ODM*/

#define RTW_WKARD_CORE_RSSI_V1

#ifdef RTW_WKARD_CORE_RSSI_V1
#define CONFIG_RX_PSTS_PER_PKT
#define CONFIG_SIGNAL_STAT_PROCESS
#endif

/*#ifndef DBG_MEM_ALLOC
#define DBG_MEM_ALLOC

#define DBG_PHL_MEM_ALLOC
#define DBG_HAL_MAC_MEM_MOINTOR
#define DBG_HAL_MEM_MOINTOR
#endif*/
#define CONFIG_PHL_USE_KMEM_ALLOC
//#define CONFIG_HW_RTS

/*
 * Work around Config
 */
#define RTW_WKARD_DIS_PROBE_REQ_RPT_TO_HOSTAPD

#ifdef CONFIG_BTC
#define RTK_WKARD_CORE_BTC_STBC_CAP
#endif

/*halbb halrf config*/
#define DRV_BB_RUA_DISABLE
#define DRV_BB_LA_MODE_DISABLE
#define DRV_BB_TDMADIG_DISABLE
#define DRV_BB_DIG_MCC_DISABLE
#define DRV_BB_PWR_CTRL_DISABLE
//#define DRV_BB_ENV_MNTR_DISABLE
#define DRV_BB_DFS_DISABLE
//#define DRV_RF_DBG_TRACE_DISABLE

/***** temporarily flag *******/
/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define DRV_NAME "rtl8730ea"

#define CONFIG_AXI_HCI

#define CONFIG_PCIE_TRX_MIT
#ifdef CONFIG_PCIE_TRX_MIT
#define PCIE_RX_INT_MIT_TIMER 4096
#endif

#define CONFIG_RPQ_AGG_NUM 30

/*
 * Wi-Fi Functions Config
 */

#define CONFIG_RECV_REORDERING_CTRL

#define CONFIG_80211N_HT
#define CONFIG_80211AC_VHT
#define CONFIG_80211AX_HE
#ifdef CONFIG_80211AC_VHT
#ifndef CONFIG_80211N_HT
#define CONFIG_80211N_HT
#endif
#endif

#ifdef CONFIG_80211AX_HE
#ifndef CONFIG_80211N_HT
#define CONFIG_80211N_HT
#endif
#ifndef CONFIG_80211AC_VHT
#define CONFIG_80211AC_VHT
#endif
#endif

#define CONFIG_BEAMFORMING
#ifdef CONFIG_BEAMFORMING
/*#define RTW_WKARD_TX_DISABLE_BFEE*/
#endif

/*#define CONFIG_IOCTL_CFG80211*/
#ifdef CONFIG_IOCTL_CFG80211
/*#define RTW_USE_CFG80211_STA_EVENT*/ /* Indecate new sta asoc through cfg80211_new_sta */
#define CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER
/*#define CONFIG_DEBUG_CFG80211*/
#define CONFIG_SET_SCAN_DENY_TIMER
#endif
//#define CONFIG_TX_AMSDU
#ifdef CONFIG_TX_AMSDU
#ifdef CONFIG_PLATFORM_RTL8198D
#define CONFIG_TX_AMSDU_HW_MODE	1
#else
#define CONFIG_TX_AMSDU_SW_MODE	1
#endif
#endif

/*
 * Internal  General Config
 */
/*#define CONFIG_PWRCTRL*/
#define CONFIG_TRX_BD_ARCH	/* PCI only */
#define USING_RX_TAG

#define CONFIG_EMBEDDED_FWIMG

#ifdef CONFIG_EMBEDDED_FWIMG
#define	LOAD_FW_HEADER_FROM_DRIVER
#endif
/*#define CONFIG_FILE_FWIMG*/

//#define CONFIG_XMIT_ACK
#ifdef CONFIG_XMIT_ACK
#define CONFIG_ACTIVE_KEEP_ALIVE_CHECK
#endif

#define BUF_DESC_ARCH		/* if defined, hardware follows Rx buffer descriptor architecture */

#ifdef CONFIG_POWER_SAVING

#define CONFIG_IPS
#ifdef CONFIG_IPS
#endif

#define CONFIG_LPS

#if defined(CONFIG_LPS)
/*#define CONFIG_LPS_LCLK*/ /* 32K */
#endif

#ifdef CONFIG_LPS_LCLK
#define CONFIG_XMIT_THREAD_MODE
#define LPS_RPWM_WAIT_MS 300
#endif
#endif

#ifdef CONFIG_POWER_SAVE
#define CONFIG_RTW_IPS
#define CONFIG_RTW_LPS

#if defined(CONFIG_RTW_IPS) || defined(CONFIG_RTW_LPS)
/* #define CONFIG_PS_FW_DBG */
#endif
#endif

#ifdef CONFIG_WOWLAN
//#define CONFIG_GTK_OL
/* #define CONFIG_ARP_KEEP_ALIVE */
#endif /* CONFIG_WOWLAN */

/*#define CONFIG_ANTENNA_DIVERSITY*/


/*#define CONFIG_PCI_ASPM*/

#define CONFIG_AP_MODE
#ifdef CONFIG_AP_MODE
#define CONFIG_NATIVEAP_MLME
#ifndef CONFIG_NATIVEAP_MLME
#define CONFIG_HOSTAPD_MLME
#endif
/*#define CONFIG_FIND_BEST_CHANNEL*/
/*#define CONFIG_AUTO_AP_MODE*/
#endif

//#define CONFIG_P2P
#ifdef CONFIG_P2P
/* The CONFIG_WFD is for supporting the Wi-Fi display */
#define CONFIG_WFD

#define CONFIG_P2P_REMOVE_GROUP_INFO

/*#define CONFIG_DBG_P2P*/

#define CONFIG_P2P_PS
/*#define CONFIG_P2P_IPS*/
#define CONFIG_P2P_OP_CHK_SOCIAL_CH
#define CONFIG_CFG80211_ONECHANNEL_UNDER_CONCURRENT  /* replace CONFIG_P2P_CHK_INVITE_CH_LIST flag */
/*#define CONFIG_P2P_INVITE_IOT*/
#endif

/* Added by Kurt 20110511 */
#ifdef CONFIG_TDLS
#define CONFIG_TDLS_DRIVER_SETUP
#if 0
#ifndef CONFIG_WFD
#define CONFIG_WFD
#endif
#define CONFIG_TDLS_AUTOSETUP
#endif
#define CONFIG_TDLS_AUTOCHECKALIVE
/* #define CONFIG_TDLS_CH_SW */	/* Enable this flag only when we confirm that TDLS CH SW is supported in FW */
#endif

#define CONFIG_SKB_COPY	/* for amsdu */

/*#define CONFIG_RTW_LED*/
#ifdef CONFIG_RTW_LED
/*#define CONFIG_RTW_SW_LED*/
#ifdef CONFIG_RTW_SW_LED
/*#define CONFIG_RTW_LED_HANDLED_BY_CMD_THREAD*/
#endif
#endif /* CONFIG_RTW_LED */

#define CONFIG_GLOBAL_UI_PID

/*#define CONFIG_ADAPTOR_INFO_CACHING_FILE*/ /* now just applied on 8192cu only, should make it general...*/
/*#define CONFIG_RESUME_IN_WORKQUEUE*/
/*#define CONFIG_SET_SCAN_DENY_TIMER*/
#define CONFIG_LONG_DELAY_ISSUE
/*#define CONFIG_SIGNAL_DISPLAY_DBM*/ /* display RX signal with dbm */
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
/*#define CONFIG_BACKGROUND_NOISE_MONITOR*/
#endif


/*
 * Software feature Related Config
 */


/*
 * Interface  Related Config
 */

/*
 * HAL  Related Config
 */
#define CONFIG_RX_PACKET_APPEND_FCS


#define DISABLE_BB_RF	0

#ifdef CONFIG_MP_INCLUDED
#define MP_DRIVER 1
#else
#define MP_DRIVER 0
#endif

#ifndef EFUSE_MAP_PATH
#define EFUSE_MAP_PATH "/system/etc/wifi/wifi_efuse.map"
#endif
#ifndef WIFIMAC_PATH
#define WIFIMAC_PATH "/etc/wifi/wifimac.txt"
#endif

/* Use cmd frame to issue beacon. Use a fixed buffer for beacon. */
#define CONFIG_BCN_ICF

#ifdef CONFIG_HWSIM
/* Use pure sw beacon */
#undef CONFIG_BCN_ICF
#endif

/* #define RTL8814BE_AMPDU_PRE_TX_OFF */

/*
 * Platform  Related Config
 */


/* #define	CONFIG_TX_EARLY_MODE */


/*
 * Debug Related Config
 */
#define DBG	1


/*#define DBG_CONFIG_ERROR_DETECT*/
/* #define DBG_CONFIG_ERROR_DETECT_INT */
/* #define DBG_CONFIG_ERROR_RESET */

/* #define DBG_IO */
/* #define DBG_DELAY_OS */
/* #define DBG_MEM_ALLOC */
/* #define DBG_IOCTL */

/* #define DBG_TX */
/* #define DBG_XMIT_BUF */
/* #define DBG_XMIT_BUF_EXT */
/* #define DBG_TX_DROP_FRAME */

/* #define DBG_RX_DROP_FRAME */
/* #define DBG_RX_SEQ */
/* #define DBG_RX_SIGNAL_DISPLAY_PROCESSING */
/* #define DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED "jeff-ap" */

/* #define DBG_ROAMING_TEST */

/* #define DBG_HAL_INIT_PROFILING */

/*#define DBG_MEMORY_LEAK*/
/* #define CONFIG_FW_C2H_DEBUG */

#define CONFIG_DBG_COUNTER
#define	DBG_RX_DFRAME_RAW_DATA
/*#define	DBG_TXBD_DESC_DUMP*/

#define CONFIG_BCN_POLLING
//#define RTW_PHL_TEST_FPGA //For 8852A PCIE FPGA TEST

/* disable dynamic rx buffer for efficiency
#ifndef CONFIG_DYNAMIC_RX_BUF
#define CONFIG_DYNAMIC_RX_BUF
#endif
*/

/*#define CONFIG_RTW_BTM_ROAM*/
/*#define CONFIG_RTW_80211R*/

#ifdef CONFIG_RTW_MBO
#ifndef CONFIG_RTW_WNM
#define CONFIG_RTW_WNM
#endif
#ifndef CONFIG_RTW_80211K
#define CONFIG_RTW_80211K
#endif
#endif /* CONFIG_RTW_MBO */

#ifdef RTW_PHL_TEST_FPGA

#ifndef RTW_PHL_TX
#define RTW_PHL_TX
#endif
#ifndef RTW_PHL_RX
#define RTW_PHL_RX
#endif
#ifndef DIRTY_FOR_WORK
#define DIRTY_FOR_WORK
#endif
#ifndef CONFIG_DYNAMIC_RX_BUF
#define CONFIG_DYNAMIC_RX_BUF
#endif
#ifndef RTW_PHL_DBG_CMD
#define RTW_PHL_DBG_CMD
#endif
#ifndef CONFIG_DRV_FAKE_AP
#error "Please enable CONFIG_DRV_FAKE_AP in Makefile before Beacon ready\n"
#endif
#ifndef RTW_PHL_FWDL
#define RTW_PHL_FWDL
#endif

#ifdef CONFIG_RTW_NAPI
#undef CONFIG_RTW_NAPI
#endif
#ifdef CONFIG_RTW_GRO
#undef CONFIG_RTW_GRO
#endif
#ifdef CONFIG_RTW_NETIF_SG
#undef CONFIG_RTW_NETIF_SG
#endif

#if 1
#define	DBGP(fmt, args...)	printk("dbg [%s][%d]"fmt, __FUNCTION__, __LINE__, ## args)
#else
#define DBGP(arg...) do {} while (0)
#endif

#else //RTW_PHL_TEST_FPGA

#define DBGP(arg...) do {} while (0)

#endif

#define CONFIG_RSSI_PRIORITY

/*
 * RTW_BUSY_DENY_SCAN control if scan would be denied by busy traffic.
 * When this defined, BUSY_TRAFFIC_SCAN_DENY_PERIOD would be used to judge if
 * scan request coming from scan UI. Scan request from scan UI would be
 * exception and never be denied by busy traffic.
 */
#define RTW_BUSY_DENY_SCAN

#if defined(CONFIG_MCC_MODE) && (!defined(CONFIG_CONCURRENT_MODE))

#error "Enable CONCURRENT_MODE before enable MCC MODE\n"

#endif

#if defined(CONFIG_MCC_MODE) && defined(CONFIG_BTC)

#error "Disable BT COEXIST before enable MCC MODE\n"

#endif

#if defined(CONFIG_MCC_MODE) && defined(CONFIG_TDLS)

#error "Disable TDLS before enable MCC MODE\n"

#endif

#if defined(CONFIG_RTW_80211R) && !defined(CONFIG_LAYER2_ROAMING)

#error "Enable CONFIG_LAYER2_ROAMING before enable CONFIG_RTW_80211R\n"

#endif

#ifdef CONFIG_RTW_ANDROID

#include <linux/version.h>

#ifndef CONFIG_IOCTL_CFG80211
#define CONFIG_IOCTL_CFG80211
#endif

#ifndef RTW_USE_CFG80211_STA_EVENT
#define RTW_USE_CFG80211_STA_EVENT
#endif

#if (CONFIG_RTW_ANDROID > 4)
#ifndef CONFIG_RADIO_WORK
#define CONFIG_RADIO_WORK
#endif
#endif

#if (CONFIG_RTW_ANDROID >= 8)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
#ifndef CONFIG_RTW_WIFI_HAL
#define CONFIG_RTW_WIFI_HAL
#endif
#else
#error "Linux kernel version is too old\n"
#endif
#endif

#ifdef CONFIG_RTW_WIFI_HAL
#ifndef CONFIG_RTW_WIFI_HAL_DEBUG
//#define CONFIG_RTW_WIFI_HAL_DEBUG
#endif
#if (CONFIG_RTW_ANDROID < 11)
#ifndef CONFIG_RTW_CFGVENDOR_LLSTATS
#define CONFIG_RTW_CFGVENDOR_LLSTATS
#endif
#endif
#ifndef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
#define CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
#endif
#ifndef CONFIG_RTW_CFGVENDOR_RSSIMONITOR
#define CONFIG_RTW_CFGVENDOR_RSSIMONITOR
#endif
#ifndef CONFIG_RTW_CFGVENDOR_WIFI_LOGGER
#define CONFIG_RTW_CFGVENDOR_WIFI_LOGGER
#endif
#if (CONFIG_RTW_ANDROID >= 10)
#ifndef CONFIG_RTW_CFGVENDOR_WIFI_OFFLOAD
//#define CONFIG_RTW_CFGVENDOR_WIFI_OFFLOAD
#endif
#ifndef CONFIG_KERNEL_PATCH_EXTERNAL_AUTH
#define CONFIG_KERNEL_PATCH_EXTERNAL_AUTH
#endif
#endif
#endif // CONFIG_RTW_WIFI_HAL


/* Some Android build will restart the UI while non-printable ascii is passed
* between java and c/c++ layer (JNI). We force CONFIG_VALIDATE_SSID
* for Android here. If you are sure there is no risk on your system about this,
* mask this macro define to support non-printable ascii ssid.
* #define CONFIG_VALIDATE_SSID */

/* Android expect dbm as the rx signal strength unit */
#define CONFIG_SIGNAL_DISPLAY_DBM
#endif // CONFIG_RTW_ANDROID

/*
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_RESUME_IN_WORKQUEUE)
	#warning "You have CONFIG_HAS_EARLYSUSPEND enabled in your system, we disable CONFIG_RESUME_IN_WORKQUEUE automatically"
	#undef CONFIG_RESUME_IN_WORKQUEUE
#endif

#if defined(CONFIG_ANDROID_POWER) && defined(CONFIG_RESUME_IN_WORKQUEUE)
	#warning "You have CONFIG_ANDROID_POWER enabled in your system, we disable CONFIG_RESUME_IN_WORKQUEUE automatically"
	#undef CONFIG_RESUME_IN_WORKQUEUE
#endif
*/

#ifdef CONFIG_RESUME_IN_WORKQUEUE /* this can be removed, because there is no case for this... */
#if !defined(CONFIG_WAKELOCK) && !defined(CONFIG_ANDROID_POWER)
#error "enable CONFIG_RESUME_IN_WORKQUEUE without CONFIG_WAKELOCK or CONFIG_ANDROID_POWER will suffer from the danger of wifi's unfunctionality..."
#error "If you still want to enable CONFIG_RESUME_IN_WORKQUEUE in this case, mask this preprossor checking and GOOD LUCK..."
#endif
#endif

/* About USB VENDOR REQ */
#if defined(CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC) && !defined(CONFIG_USB_VENDOR_REQ_MUTEX)
#warning "define CONFIG_USB_VENDOR_REQ_MUTEX for CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC automatically"
#define CONFIG_USB_VENDOR_REQ_MUTEX
#endif
#if defined(CONFIG_VENDOR_REQ_RETRY) &&  !defined(CONFIG_USB_VENDOR_REQ_MUTEX)
#warning "define CONFIG_USB_VENDOR_REQ_MUTEX for CONFIG_VENDOR_REQ_RETRY automatically"
#define CONFIG_USB_VENDOR_REQ_MUTEX
#endif

#ifdef CONFIG_WIFI_MONITOR
/*	#define CONFIG_MONITOR_MODE_XMIT	*/
#endif

#ifdef CONFIG_CUSTOMER_ALIBABA_GENERAL
#ifndef CONFIG_WIFI_MONITOR
#define CONFIG_WIFI_MONITOR
#endif
#ifndef CONFIG_MONITOR_MODE_XMIT
#define CONFIG_MONITOR_MODE_XMIT
#endif
#ifdef CONFIG_POWER_SAVING
#undef CONFIG_POWER_SAVING
#endif
#endif

#ifndef CONFIG_RTW_DATA_BMC_TO_UC
#define CONFIG_RTW_DATA_BMC_TO_UC 0
#endif

#ifdef CONFIG_AP_MODE
#define CONFIG_LIMITED_AP_NUM 1
#define BMC_ON_HIQ

#ifndef CONFIG_RTW_AP_DATA_BMC_TO_UC
#define CONFIG_RTW_AP_DATA_BMC_TO_UC 1
#endif
#if CONFIG_RTW_AP_DATA_BMC_TO_UC
#undef CONFIG_RTW_DATA_BMC_TO_UC
#define CONFIG_RTW_DATA_BMC_TO_UC 1
#endif
#ifndef CONFIG_RTW_AP_SRC_B2U_FLAGS
#define CONFIG_RTW_AP_SRC_B2U_FLAGS 0x8 /* see RTW_AP_B2U_XXX */
#endif
#ifndef CONFIG_RTW_AP_FWD_B2U_FLAGS
#define CONFIG_RTW_AP_FWD_B2U_FLAGS 0x8 /* see RTW_AP_B2U_XXX */
#endif
#endif

#ifdef CONFIG_RTW_MULTI_AP
#ifndef CONFIG_AP_MODE
#error "enable CONFIG_RTW_MULTI_AP without CONFIG_AP_MODE"
#endif
#ifndef CONFIG_RTW_WDS
#define CONFIG_RTW_WDS
#endif
#ifndef CONFIG_RTW_UNASOC_STA_MODE_OF_STYPE
#define CONFIG_RTW_UNASOC_STA_MODE_OF_STYPE {2, 1} /* BMC:2 for all, NMY_UC:1 for interested target */
#endif
#ifndef CONFIG_RTW_NLRTW
#define CONFIG_RTW_NLRTW
#endif
#ifndef CONFIG_RTW_WNM
#define CONFIG_RTW_WNM
#endif
#ifndef CONFIG_RTW_80211K
#define CONFIG_RTW_80211K
#endif
#endif

#ifdef CONFIG_RTW_MESH
#ifndef CONFIG_RTW_MESH_ACNODE_PREVENT
#define CONFIG_RTW_MESH_ACNODE_PREVENT 1
#endif

#ifndef CONFIG_RTW_MESH_OFFCH_CAND
#define CONFIG_RTW_MESH_OFFCH_CAND 1
#endif

#ifndef CONFIG_RTW_MESH_PEER_BLACKLIST
#define CONFIG_RTW_MESH_PEER_BLACKLIST 1
#endif

#ifndef CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST
#define CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST 1
#endif
#ifndef CONFIG_RTW_MESH_CTO_MGATE_CARRIER
#define CONFIG_RTW_MESH_CTO_MGATE_CARRIER CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST
#endif

#ifndef CONFIG_RTW_MPM_TX_IES_SYNC_BSS
#define CONFIG_RTW_MPM_TX_IES_SYNC_BSS 1
#endif
#if CONFIG_RTW_MPM_TX_IES_SYNC_BSS
#ifndef CONFIG_RTW_MESH_AEK
#define CONFIG_RTW_MESH_AEK
#endif
#endif

#ifndef CONFIG_RTW_MESH_DATA_BMC_TO_UC
#define CONFIG_RTW_MESH_DATA_BMC_TO_UC 1
#endif
#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
#undef CONFIG_RTW_DATA_BMC_TO_UC
#define CONFIG_RTW_DATA_BMC_TO_UC 1
#endif
#ifndef CONFIG_RTW_MSRC_B2U_FLAGS
#define CONFIG_RTW_MSRC_B2U_FLAGS 0x0 /* see RTW_MESH_B2U_XXX */
#endif
#ifndef CONFIG_RTW_MFWD_B2U_FLAGS
#define CONFIG_RTW_MFWD_B2U_FLAGS 0x2 /* see RTW_MESH_B2U_XXX */
#endif
#endif

#if !defined(CONFIG_SCAN_BACKOP) && defined(CONFIG_AP_MODE)
#define CONFIG_SCAN_BACKOP
#endif

#define RTW_SCAN_SPARSE_MIRACAST 1
#define RTW_SCAN_SPARSE_BG 0

#ifndef CONFIG_RTW_HIQ_FILTER
#define CONFIG_RTW_HIQ_FILTER 1
#endif

#ifndef CONFIG_RTW_ADAPTIVITY_EN
#define CONFIG_RTW_ADAPTIVITY_EN 0
#endif

#ifndef CONFIG_RTW_ADAPTIVITY_MODE
#define CONFIG_RTW_ADAPTIVITY_MODE 0
#endif

#ifndef CONFIG_RTW_ADAPTIVITY_TH_L2H_INI
#define CONFIG_RTW_ADAPTIVITY_TH_L2H_INI 0
#endif

#ifndef CONFIG_RTW_ADAPTIVITY_TH_EDCCA_HL_DIFF
#define CONFIG_RTW_ADAPTIVITY_TH_EDCCA_HL_DIFF 0
#endif

#ifndef CONFIG_RTW_EXCL_CHS
#define CONFIG_RTW_EXCL_CHS {0}
#endif

#ifndef CONFIG_RTW_EXCL_CHS_6G
#define CONFIG_RTW_EXCL_CHS_6G {0}
#endif

#ifndef CONFIG_RTW_COUNTRY_IE_SLAVE_EN_ROLE
#define CONFIG_RTW_COUNTRY_IE_SLAVE_EN_ROLE 0x03 /* BIT0 for pure STA mode, BIT1 for P2P group client */
#endif

#ifndef CONFIG_RTW_COUNTRY_IE_SLAVE_EN_IFBMP
#define CONFIG_RTW_COUNTRY_IE_SLAVE_EN_IFBMP 0xFF /* all iface */
#endif

#define CONFIG_IEEE80211_BAND_5GHZ 1

#ifndef CONFIG_IEEE80211_BAND_6GHZ
#define CONFIG_IEEE80211_BAND_6GHZ 0
#endif

#ifndef CONFIG_DFS
#define CONFIG_DFS 1
#endif

#if CONFIG_DFS
#define CONFIG_ECSA_PHL /* Process ECSA by PHL cmd dispatcher */
#ifdef CONFIG_ECSA_PHL
#define CONFIG_ECSA
#endif
#endif

#if CONFIG_IEEE80211_BAND_5GHZ && CONFIG_DFS && defined(CONFIG_AP_MODE)
#if !defined(CONFIG_DFS_SLAVE_WITH_RADAR_DETECT)
#define CONFIG_DFS_SLAVE_WITH_RADAR_DETECT 0
#endif
//#if !defined(CONFIG_DFS_MASTER) || CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
//#define CONFIG_DFS_MASTER
//#endif
#if defined(CONFIG_DFS_MASTER) && !defined(CONFIG_RTW_DFS_REGION_DOMAIN)
#define CONFIG_RTW_DFS_REGION_DOMAIN 0
#endif
#else
#undef CONFIG_DFS_MASTER
#undef CONFIG_RTW_DFS_REGION_DOMAIN
#define CONFIG_RTW_DFS_REGION_DOMAIN 0
#undef CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
#define CONFIG_DFS_SLAVE_WITH_RADAR_DETECT 0
#endif

#ifndef CONFIG_TXPWR_BY_RATE_EN
#define CONFIG_TXPWR_BY_RATE_EN 2 /* by efuse */
#endif
#ifndef CONFIG_TXPWR_LIMIT_EN
#define CONFIG_TXPWR_LIMIT_EN 1 /* by efuse */
#endif

#ifndef CONFIG_RTW_CHPLAN
#define CONFIG_RTW_CHPLAN 0xFFFF /* RTW_CHPLAN_IOCTL_UNSPECIFIED */
#endif

#ifndef CONFIG_RTW_CHPLAN_6G
#define CONFIG_RTW_CHPLAN_6G 0xFFFF /* RTW_CHPLAN_IOCTL_UNSPECIFIED */
#endif

/* compatible with old fashion configuration */
#if defined(CONFIG_CALIBRATE_TX_POWER_BY_REGULATORY)
#undef CONFIG_TXPWR_BY_RATE_EN
#undef CONFIG_TXPWR_LIMIT_EN
#define CONFIG_TXPWR_BY_RATE_EN 1
#define CONFIG_TXPWR_LIMIT_EN 1
#elif defined(CONFIG_CALIBRATE_TX_POWER_TO_MAX)
#undef CONFIG_TXPWR_BY_RATE_EN
#undef CONFIG_TXPWR_LIMIT_EN
#define CONFIG_TXPWR_BY_RATE_EN 1
#define CONFIG_TXPWR_LIMIT_EN 0
#endif

#ifndef RTW_DEF_MODULE_REGULATORY_CERT
#define RTW_DEF_MODULE_REGULATORY_CERT 0
#endif

#if RTW_DEF_MODULE_REGULATORY_CERT
#ifdef CONFIG_REGD_SRC_FROM_OS
#error "CONFIG_REGD_SRC_FROM_OS is not supported when enable RTW_DEF_MODULE_REGULATORY_CERT"
#endif
/* force enable TX power by rate and TX power limit */
#undef CONFIG_TXPWR_BY_RATE_EN
#undef CONFIG_TXPWR_LIMIT_EN
#define CONFIG_TXPWR_BY_RATE_EN 1
#define CONFIG_TXPWR_LIMIT_EN 1
#endif

#if !CONFIG_TXPWR_LIMIT && CONFIG_TXPWR_LIMIT_EN
#undef CONFIG_TXPWR_LIMIT
#define CONFIG_TXPWR_LIMIT 1
#endif

#ifndef CONFIG_RTW_REGD_SRC
#define CONFIG_RTW_REGD_SRC 1 /* 0:RTK_PRIV, 1:OS */
#endif

#ifdef CONFIG_RTW_IPCAM_APPLICATION
#undef CONFIG_TXPWR_BY_RATE_EN
#define CONFIG_TXPWR_BY_RATE_EN 1
#define CONFIG_RTW_CUSTOMIZE_BEEDCA		0x0000431C
#define CONFIG_RTW_CUSTOMIZE_BWMODE		0x00
#define CONFIG_RTW_CUSTOMIZE_RLSTA		0x7
#if defined(CONFIG_RTL8822B)
#define CONFIG_RTW_TX_NPATH_EN		/*mutually incompatible with STBC_TX & Beamformer	*/
#endif
#endif
/* #define CONFIG_RTW_TOKEN_BASED_XMIT */
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
#define NR_TBTX_SLOT			4
#define NR_MAXSTA_INSLOT		5
#define TBTX_TX_DURATION		30

#define MAX_TXPAUSE_DURATION	(TBTX_TX_DURATION*NR_TBTX_SLOT)
#endif

/*#define CONFIG_EXTEND_LOWRATE_TXOP			*/

#ifndef CONFIG_RTW_RX_AMPDU_SZ_LIMIT_1SS
#define CONFIG_RTW_RX_AMPDU_SZ_LIMIT_1SS {0xFF, 0xFF, 0xFF, 0xFF}
#endif
#ifndef CONFIG_RTW_RX_AMPDU_SZ_LIMIT_2SS
#define CONFIG_RTW_RX_AMPDU_SZ_LIMIT_2SS {0xFF, 0xFF, 0xFF, 0xFF}
#endif
#ifndef CONFIG_RTW_RX_AMPDU_SZ_LIMIT_3SS
#define CONFIG_RTW_RX_AMPDU_SZ_LIMIT_3SS {0xFF, 0xFF, 0xFF, 0xFF}
#endif
#ifndef CONFIG_RTW_RX_AMPDU_SZ_LIMIT_4SS
#define CONFIG_RTW_RX_AMPDU_SZ_LIMIT_4SS {0xFF, 0xFF, 0xFF, 0xFF}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_2G_A
#define CONFIG_RTW_TARGET_TX_PWR_2G_A {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_2G_B
#define CONFIG_RTW_TARGET_TX_PWR_2G_B {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_2G_C
#define CONFIG_RTW_TARGET_TX_PWR_2G_C {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_2G_D
#define CONFIG_RTW_TARGET_TX_PWR_2G_D {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_5G_A
#define CONFIG_RTW_TARGET_TX_PWR_5G_A {-1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_5G_B
#define CONFIG_RTW_TARGET_TX_PWR_5G_B {-1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_5G_C
#define CONFIG_RTW_TARGET_TX_PWR_5G_C {-1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_TARGET_TX_PWR_5G_D
#define CONFIG_RTW_TARGET_TX_PWR_5G_D {-1, -1, -1, -1, -1, -1, -1, -1, -1}
#endif

#ifndef CONFIG_RTW_AMPLIFIER_TYPE_2G
#define CONFIG_RTW_AMPLIFIER_TYPE_2G 0
#endif

#ifndef CONFIG_RTW_AMPLIFIER_TYPE_5G
#define CONFIG_RTW_AMPLIFIER_TYPE_5G 0
#endif

#ifndef CONFIG_RTW_RFE_TYPE
#define CONFIG_RTW_RFE_TYPE 0xFF
#endif

#ifndef CONFIG_RTW_GLNA_TYPE
#define CONFIG_RTW_GLNA_TYPE 0
#endif

#ifndef CONFIG_RTW_PLL_REF_CLK_SEL
#define CONFIG_RTW_PLL_REF_CLK_SEL 0x0F
#endif

#ifndef CONFIG_IFACE_NUMBER
#ifdef CONFIG_CONCURRENT_MODE
#define CONFIG_IFACE_NUMBER	2
#else
#define CONFIG_IFACE_NUMBER	1
#endif
#endif

#ifdef CONFIG_CONCURRENT_MODE
#if (CONFIG_IFACE_NUMBER < 2)
#error "CONFIG_IFACE_NUMBER less 2,but CONFIG_CONCURRENT_MODE defined"
#endif
#else
#if (CONFIG_IFACE_NUMBER > 1)
#error "CONFIG_IFACE_NUMBER over 1,but CONFIG_CONCURRENT_MODE not defined"
#endif
#endif

#if (CONFIG_IFACE_NUMBER == 0)
#error "CONFIG_IFACE_NUMBER cound not be 0 !!"
#endif


#if (CONFIG_IFACE_NUMBER > 4)
#error "Not support over 4 interfaces yet !!"
#endif

#if (CONFIG_IFACE_NUMBER > 8)	/*IFACE_ID_MAX*/
#error "HW count not support over 8 interfaces !!"
#endif

#if (CONFIG_IFACE_NUMBER > 2)
#ifdef CONFIG_AP_MODE
#undef CONFIG_LIMITED_AP_NUM
#define CONFIG_LIMITED_AP_NUM	2
#endif /*CONFIG_AP_MODE*/

#endif/*(CONFIG_IFACE_NUMBER > 2)*/

#define MACID_NUM_SW_LIMIT 16
#define SEC_CAM_ENT_NUM_SW_LIMIT 16


/*
Mark CONFIG_DEAUTH_BEFORE_CONNECT by Arvin 2015/07/20
If the failure of Wi-Fi connection is due to some irregular disconnection behavior (like unplug dongle,
power down etc.) in last time, we can unmark this flag to avoid some unpredictable response from AP.
*/
/*#define CONFIG_DEAUTH_BEFORE_CONNECT */

/*#define CONFIG_WEXT_DONT_JOIN_BYSSID	*/
/* #include <rtw_byteorder.h> */


/*#define CONFIG_DOSCAN_IN_BUSYTRAFFIC	*/

/*Don't release SDIO irq in suspend/resume procedure*/
#define CONFIG_RTW_SDIO_KEEP_IRQ	0

/*
 * Add by Lucas@2016/02/15
 * For RX Aggregation
 */
#if defined(CONFIG_USB_RX_AGGREGATION)
#define RTW_RX_AGGREGATION
#endif /* CONFIG_USB_RX_AGGREGATION */

#ifdef CONFIG_RTW_HOSTAPD_ACS
#ifdef CONFIG_FIND_BEST_CHANNEL
#undef CONFIG_FIND_BEST_CHANNEL
#endif
#ifndef CONFIG_RTW_ACS
#define CONFIG_RTW_ACS
#endif
#endif


#ifdef DBG_CONFIG_ERROR_RESET
#ifndef CONFIG_IPS
#define CONFIG_IPS
#endif
#endif

/* IPS */
#ifndef RTW_IPS_MODE
#if defined(CONFIG_IPS)
#define RTW_IPS_MODE 1
#else
#define RTW_IPS_MODE 0
#endif
#endif /* !RTW_IPS_MODE */

#if (RTW_IPS_MODE > 1 || RTW_IPS_MODE < 0)
#error "The CONFIG_IPS_MODE value is wrong. Please follow HowTo_enable_the_power_saving_functionality.pdf.\n"
#endif

/* LPS */
#ifndef RTW_LPS_MODE
#if defined(CONFIG_LPS_PG)
#define RTW_LPS_MODE 3
#elif defined(CONFIG_LPS_LCLK)
#define RTW_LPS_MODE 2
#elif defined(CONFIG_LPS)
#define RTW_LPS_MODE 1
#else
#define RTW_LPS_MODE 0
#endif
#endif /* !RTW_LPS_MODE */

#if (RTW_LPS_MODE > 3 || RTW_LPS_MODE < 0)
#error "The CONFIG_LPS_MODE value is wrong. Please follow HowTo_enable_the_power_saving_functionality.pdf.\n"
#endif

#ifndef RTW_LPS_1T1R
#define RTW_LPS_1T1R 0
#endif

#ifndef RTW_WOW_LPS_1T1R
#define RTW_WOW_LPS_1T1R 0
#endif

/* WOW LPS */
#ifndef RTW_WOW_LPS_MODE
#if defined(CONFIG_LPS_PG)
#define RTW_WOW_LPS_MODE 3
#elif defined(CONFIG_LPS_LCLK)
#define RTW_WOW_LPS_MODE 2
#elif defined(CONFIG_LPS)
#define RTW_WOW_LPS_MODE 1
#else
#define RTW_WOW_LPS_MODE 0
#endif
#endif /* !RTW_WOW_LPS_MODE */

#if (RTW_WOW_LPS_MODE > 3 || RTW_WOW_LPS_MODE < 0)
#error "The RTW_WOW_LPS_MODE value is wrong. Please follow HowTo_enable_the_power_saving_functionality.pdf.\n"
#endif

#ifdef RTW_REDUCE_SCAN_SWITCH_CH_TIME
#ifndef CONFIG_RTL8822B
#error "Only 8822B support RTW_REDUCE_SCAN_SWITCH_CH_TIME"
#endif
#ifndef RTW_CHANNEL_SWITCH_OFFLOAD
#define RTW_CHANNEL_SWITCH_OFFLOAD
#endif
#endif

#ifdef CONFIG_BCN_POLLING
#define CONFIG_BCN_ICF
#endif

#ifndef CONFIG_PCI_MSI
#define CONFIG_RTW_PCI_MSI_DISABLE
#endif

#if 0
/* Debug related compiler flags */
#define DBG_THREAD_PID	/* Add thread pid to debug message prefix */
#define DBG_CPU_INFO	/* Add CPU info to debug message prefix */
#endif

#ifdef CONFIG_TX_AMSDU_HW_MODE
#define CONFIG_HW_HDR_CONVERSION
#elif defined(CONFIG_TX_AMSDU_SW_MODE)
#else
#define CONFIG_HW_HDR_CONVERSION	/* TODO: should be 'unchange' */
#endif

/* for phl illegal mac io access check*/
#define CONFIG_MAC_REG_RW_CHK

/* To enable the CONFIG_PHL_P2PPS definition in phl_config.h */
#ifdef CONFIG_P2P_PS
#define CONFIG_P2PPS
#endif

#ifdef CONFIG_CMD_DISP
/*#define DBG_CONFIG_CMD_DISP*/

#define CONFIG_CMD_SCAN
#ifdef CONFIG_CMD_SCAN
#ifdef CONFIG_IOCTL_CFG80211
#define CONFIG_PHL_CMD_SCAN_BKOP_TIME
#endif
/* Scan hidden AP in passive channel */
#define RTW_WKARD_CMD_SCAN_EXTEND_ACTIVE_SCAN
#define RTW_EXTEND_ACTIVE_SCAN_PERIOD	30	/* unit: ms */
#endif /* CONFIG_CMD_SCAN */

#define CONFIG_CMD_GENERAL
#define CONFIG_CMD_SER
#define CONFIG_STA_CMD_DISPR
#define CONFIG_AP_CMD_DISPR
/*#define CONFIG_IBSS_CMD_DISPR*/
#define CONFIG_CMD_TSF_SYNC
#endif

#ifdef ROKU_PRIVATE
#define CONFIG_RELEASE_RPT
#define CONFIG_RA_TXSTS_DBG
#endif

#ifdef CONFIG_80211AX_HE
#define CONFIG_STA_MULTIPLE_BSSID
#endif

#ifdef CONFIG_NARROWBAND_SUPPORTING
#define CONFIG_NB_VALUE		RTW_NB_CONFIG_NONE/*RTW_NB_CONFIG_WIDTH_10 or RTW_NB_CONFIG_WIDTH_5*/
#endif
/*
 * Work around Config
 */
/*
 * RTW_WKARD_UPDATE_PHL_ROLE_CAP
 * Update adapter->phl_role.cap & proto_role_cap by driver parameters(registry).
 */
#define RTW_WKARD_UPDATE_PHL_ROLE_CAP

/*
*RTW_WKARD_TRIGGER_FRAME_PARSER-OFDMA UL TB control
*/
#define RTW_WKARD_TRIGGER_FRAME_PARSER
#endif /* __DRV_CONF_H__ */
