# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_OTP_RTK_AMEBA_CORE)	+= otp-realtek.o
otp-realtek-objs			:= realtek-otp-core.o
ifeq ($(CONFIG_OTP_RTK_AMEBA_CDEV),y)
	otp-realtek-objs		+= realtek-otp-cdev.o
endif
