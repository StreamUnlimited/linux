#ifndef _RTW_TOP_HEADER_
#define _RTW_TOP_HEADER_

/* Allow print debug info at runtime by control log level. */
/* Runtime order: echo 8 4 1 7 > /proc/sys/kernel/printk   */
#define DEBUG 1

/******************************************************************/
/******************** Linux platform related. *********************/
/******************************************************************/

#include <linux/version.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/kref.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/dma-mapping.h>
#include <net/cfg80211.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm-generic/io.h>
#include <linux/semaphore.h>
#include <linux/sem.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <net/addrconf.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/sysrq.h>
#include <linux/ioport.h>
#include <asm/string.h>
#include <linux/workqueue.h>
#include <uapi/linux/limits.h>
#include <net/ieee80211_radiotap.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>

/* ipc driver. */
#include <ameba_ipc/ameba_ipc.h>

/* fullmac headers. */
#include "rtw_wiphy.h"
#include "rtw_llhw_event.h"
#include "rtw_drv_probe.h"
#include "rtw_regd.h"
#include "rtw_netdev_ops.h"
#include "rtw_ethtool_ops.h"
#include "rtw_llhw_hci_ipc.h"
#include "rtw_llhw_ops.h"
#include "rtw_llhw_trx.h"
#include "rtw_llhw_hci_ipc_msg.h"
#include "rtw_functions.h"

/******************************************************************/
/********** Definitions between Linux and FULLMAC. **************/
/******************************************************************/
#define TOTAL_IFACE_NUM			2
#define ETH_ALEN			6
#define FUNC_NDEV_FMT			"%s(%s)"
#define FUNC_NDEV_ARG(ndev)		__func__, ndev->name

/******************************************************************/
/***************** Definitions for cfg80211_ops. ******************/
/******************************************************************/

struct rtw_ieee80211_hdr_3addr {
	u16				frame_ctl;
	u16				duration_id;
	u8				addr1[ETH_ALEN];
	u8				addr2[ETH_ALEN];
	u8				addr3[ETH_ALEN];
	u16				seq_ctl;
} __attribute__((packed));

#define set_frame_sub_type(pbuf, type) \
	do {    \
		*(unsigned short *)(pbuf) &= cpu_to_le16(~(BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2))); \
		*(unsigned short *)(pbuf) |= cpu_to_le16(type); \
	} while (0)

#define get_addr2_ptr(pbuf)    ((unsigned char *)((unsigned int)(pbuf) + 10))

/******************************************************************/
/***************** inline functions for fullmac. *****************/
/******************************************************************/
static inline int rtw_ch2freq(int chan)
{
	if (chan >= 1 && chan <= 14) {
		if (chan == 14) {
			return 2484;
		} else if (chan < 14) {
			return 2407 + chan * 5;
		}
	} else if (chan >= 36 && chan <= 177) {
		return 5000 + chan * 5;
	}

	return 0; /* not supported */
}

static inline int rtw_freq2ch(int freq)
{
	/* see 802.11 17.3.8.3.2 and Annex J */
	if (freq == 2484) {
		return 14;
	} else if (freq < 2484) {
		return (freq - 2407) / 5;
	} else if (freq >= 4910 && freq <= 4980) {
		return (freq - 4000) / 5;
	} else if (freq >= 5000 && freq < 5950) {
		return (freq - 5000) / 5;
	} else if (freq >= 5950 && freq <= 7215) {
		return (freq - 5950) / 5;
	} else if (freq >= 58320 && freq <= 64800) {
		return (freq - 56160) / 2160;
	} else {
		return 0;
	}
}

static inline u8 *rtw_get_ie(const u8 *pbuf, int index, int *len, int limit)
{
	int tmp, i;
	const u8 *p;
	if (limit < 1) {
		return NULL;
	}

	p = pbuf;
	i = 0;
	*len = 0;
	while (1) {
		if (*p == index) {
			*len = *(p + 1);
			return (u8 *)p;
		} else {
			tmp = *(p + 1);
			p += (tmp + 2);
			i += (tmp + 2);
		}
		if (i >= limit) {
			break;
		}
	}
	return NULL;
}

#endif // _RTW_TOP_HEADER_
