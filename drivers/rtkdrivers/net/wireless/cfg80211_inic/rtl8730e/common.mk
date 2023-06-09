########### OS_DEP PATH  #################################
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
EXTRA_CFLAGS += -DCONFIG_TRAFFIC_PROTECT
EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT
EXTRA_CFLAGS += -DCONFIG_RADIO_WORK
EXTRA_CFLAGS += -DCONFIG_CONCURRENT_MODE

EXTRA_CFLAGS += -DCONFIG_PLATFORM_IOT
EXTRA_CFLAGS += -DRTW_PHL_BCN_IOT
EXTRA_CFLAGS += -DPHL_PLATFORM_LINUX
#EXTRA_CFLAGS += -Wno-error=date-time

EXTRA_CFLAGS += -DCONFIG_PLATFORM_OPS
EXTRA_CFLAGS += -I$(srctree)/$(src)/inic_ipc

_OS_INTFS_FILES :=	osdep_service.o \
			rtw_cfg80211_ops.o \
			rtw_netdev_ops.o \
			rtw_xmit.o \
			rtw_ioctl.o \
			#mlme_linux.o \
			#rtw_recv.o \
			#rtw_cfg.o \
			#rtw_cfgvendor.o \
			#wifi_regd.o \
			#rtw_android.o \
			#rtw_proc.o \
			#nlrtw.o \
			#rtw_rhashtable.o


_OS_INTFS_FILES += rtw_drv_probe.o

_OS_INTFS_FILES += 	inic_ipc/inic_ipc_msg_queue.o \
		inic_ipc/inic_ipc_host_api.o \
		inic_ipc/inic_net_device.o \
		inic_ipc/inic_ipc_host_trx.o \
		inic_ipc/inic_ipc_host.o \
		#inic/inic_cmd.o \
		#inic/inic_ext_cmd.o \
		#inic/inic_prmc_cmd.o \

ifeq ($(CONFIG_MP_INCLUDED), y)
_OS_INTFS_FILES += ioctl_mp.o \
		ioctl_efuse.o
endif
########### CORE PATH  #################################

OBJS += $(_OS_INTFS_FILES) $(_CORE_FILES)
