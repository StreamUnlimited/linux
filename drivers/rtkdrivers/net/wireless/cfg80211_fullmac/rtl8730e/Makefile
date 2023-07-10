# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I $(srctree)/drivers/rtkdrivers/net/wireless/cfg80211_fullmac/rtl8730e

CONFIG_NAN = n
ccflags-$(CONFIG_NAN) += -DCONFIG_NAN

ccflags-$(CONFIG_FULLMAC_RTK_RTL8730E) += -DCONFIG_FULLMAC
ccflags-$(CONFIG_FULLMAC_RTK_RTL8730E) += -I$(srctree)/../../firmware/component/wifi/driver/include
ccflags-$(CONFIG_FULLMAC_RTK_RTL8730E) += -I$(srctree)/../../firmware/component/wifi/api
ccflags-$(CONFIG_FULLMAC_RTK_RTL8730E) += -I$(srctree)/../../firmware/component/wifi/inic

obj-$(CONFIG_FULLMAC_RTK_RTL8730E)	+= fullmac-8730e.o
fullmac-8730e-objs			:= rtw_drv_probe.o \
					rtw_cfg80211_ops.o \
					rtw_wiphy.o \
					rtw_regd.o \
					rtw_ethtool_ops.o \
					rtw_cfg80211_ops_ap.o \
					rtw_cfg80211_ops_key.o \
					rtw_cfg80211_ops_nan.o \
					rtw_netdev_ops.o \
					rtw_llhw_ops.o \
					rtw_llhw_event.o \
					rtw_llhw_recv.o \
					rtw_llhw_xmit.o \
					rtw_llhw_hci_ipc_msg.o \
					rtw_llhw_hci_ipc.o \
					rtw_cfgvendor.o \
					rtw_proc.o \

