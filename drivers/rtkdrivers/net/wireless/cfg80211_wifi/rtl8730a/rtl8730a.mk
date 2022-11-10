EXTRA_CFLAGS += -DCONFIG_RTL8730A

#ifeq ($(CONFIG_MP_INCLUDED), y)
### 8730A Default Enable VHT MP HW TX MODE ###
#EXTRA_CFLAGS += -DCONFIG_MP_VHT_HW_TX_MODE
#CONFIG_MP_VHT_HW_TX_MODE = y
#endif

_HAL_INTFS_FILES +=	hal/rtl8730a/rtl8730a_halinit.o \
			hal/rtl8730a/rtl8730a_mac.o \
			hal/rtl8730a/rtl8730a_cmd.o \
			hal/rtl8730a/rtl8730a_phy.o \
			hal/rtl8730a/rtl8730a_ops.o \
			hal/rtl8730a/hal8730a_fw.o

ifeq ($(CONFIG_AXI_HCI), y)
_HAL_INTFS_FILES +=	hal/rtl8730a/$(HCI_NAME)/rtl8730aa_halinit.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_halmac.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_io.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_xmit.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_recv.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_led.o \
			hal/rtl8730a/$(HCI_NAME)/rtl8730aa_ops.o

endif

include $(src)/halmac-rs.mk

_BTC_FILES +=		hal/btc/halbtc8730awifionly.o
ifeq ($(CONFIG_BT_COEXIST), y)
_BTC_FILES +=		hal/btc/halbtccommon.o \
			hal/btc/halbtc8730a.o
endif
