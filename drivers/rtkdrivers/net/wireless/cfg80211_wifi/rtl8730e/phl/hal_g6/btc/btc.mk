# All needed files would be added to _HAL_INTFS_FILES, and it would include
# hal_g6/btc and all related files in directory hal_g6/btc/.
# Before include this makefile, be sure interface (CONFIG_*_HCI) and IC
# (CONFIG_RTL*) setting are all ready!

HAL = hal_g6

ifeq ($(CONFIG_PHL_ARCH), y)
phl_path := phl/hal_g6
phl_path_d1 := $(srctree)/$(src)/phl/$(HAL)
else
phl_path := hal_g6
phl_path_d1 := $(srctree)/$(src)/$(HAL)
endif

# Base directory
path_halbtc_d1 := $(phl_path)/btc

halbtc-y += $(path_halbtc_d1)/hal_btc.o \
			$(path_halbtc_d1)/halbtc_def.o \
			$(path_halbtc_d1)/halbtc_action.o \
			$(path_halbtc_d1)/halbtc_fw.o \
			$(path_halbtc_d1)/halbtc_dbg_cmd.o

ifeq ($(CONFIG_RTL8730E), y)
ic := 8730e
# Level 2 directory
path_halbtc_8730e := $(path_halbtc_d1)/btc_$(ic)

halbtc-y += $(path_halbtc_8730e)/btc_8730e.o
endif

_BTC_FILES +=	$(halbtc-y)
