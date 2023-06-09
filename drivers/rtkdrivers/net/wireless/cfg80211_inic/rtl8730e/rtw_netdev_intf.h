/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#ifndef __OSDEP_INTF_H_
#define __OSDEP_INTF_H_

#ifdef PLATFORM_LINUX

int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);

int rtw_os_ndevs_init(void);
void rtw_os_ndevs_deinit(void);

#define wdev_to_ndev(w) ((w)->netdev)
#define wdev_to_wiphy(w) ((w)->wiphy)
#define ndev_to_wdev(n) ((n)->ieee80211_ptr)

//#include "../os_dep/linux/rtw_proc.h"
//#include "../os_dep/linux/nlrtw.h"

#endif /* PLATFORM_LINUX */
#endif /* _OSDEP_INTF_H_ */
