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
#ifndef __OSDEP_LINUX_SERVICE_H_
#define __OSDEP_LINUX_SERVICE_H_

#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/kref.h>

/* #include <linux/smp_lock.h> */
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/sem.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <net/addrconf.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/interrupt.h>	/* for struct tasklet_struct */
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

#ifdef CONFIG_RECV_THREAD_MODE
#include <uapi/linux/sched/types.h>	/* struct sched_param */
#endif

#include <uapi/linux/limits.h>

#ifdef RTK_DMP_PLATFORM
#include <linux/pageremap.h>
#include <asm/io.h>
#endif

#ifdef CONFIG_NET_RADIO
#define CONFIG_WIRELESS_EXT
#endif

/* Monitor mode */
#include <net/ieee80211_radiotap.h>

#include <linux/ieee80211.h>


/*	#include <linux/ieee80211.h> */
#include <net/cfg80211.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef CONFIG_EFUSE_CONFIG_FILE
#include <linux/fs.h>
#endif


#define ATOMIC_T atomic_t

#ifdef DBG_MEMORY_LEAK
extern ATOMIC_T _malloc_cnt;
extern ATOMIC_T _malloc_size;
#endif

static inline void *_rtw_vmalloc(u32 sz)
{
	void *pbuf;

	pbuf = vmalloc(sz);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

static inline void *_rtw_zvmalloc(u32 sz)
{
	void *pbuf;

	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL) {
		memset(pbuf, 0, sz);
	}

	return pbuf;
}

static inline void _rtw_vmfree(void *pbuf, u32 sz)
{
	vfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
}

static inline void *_rtw_malloc(u32 sz)
{
	void *pbuf = NULL;

#ifdef RTK_DMP_PLATFORM
	if (sz > 0x4000) {
		pbuf = dvr_malloc(sz);
	} else
#endif
		pbuf = kmalloc(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;

}

static inline void *_rtw_zmalloc(u32 sz)
{
	/*kzalloc in KERNEL_VERSION(2, 6, 14)*/
	void *pbuf = kzalloc(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

	return pbuf;
}

static inline void _rtw_mfree(void *pbuf, u32 sz)
{
#ifdef RTK_DMP_PLATFORM
	if (sz > 0x4000) {
		dvr_free(pbuf);
	} else
#endif
		kfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */

}

/*lock - spinlock*/
typedef	spinlock_t _lock;
static inline void _rtw_spinlock_init(_lock *plock)
{
	spin_lock_init(plock);
}
static inline void _rtw_spinlock_free(_lock *plock)
{
}
static inline void _rtw_spinlock(_lock *plock)
{
	spin_lock(plock);
}
static inline void _rtw_spinunlock(_lock *plock)
{
	spin_unlock(plock);
}

__inline static void _rtw_spinlock_irq(_lock *plock, unsigned long *flags)
{
	spin_lock_irqsave(plock, *flags);
}
__inline static void _rtw_spinunlock_irq(_lock *plock, unsigned long *flags)
{
	spin_unlock_irqrestore(plock, *flags);
}
__inline static void _rtw_spinlock_bh(_lock *plock)
{
	spin_lock_bh(plock);
}
__inline static void _rtw_spinunlock_bh(_lock *plock)
{
	spin_unlock_bh(plock);
}


/*lock - semaphore*/
typedef struct	semaphore _sema;
static inline void _rtw_init_sema(_sema *sema, int init_val)
{
	sema_init(sema, init_val);
}
static inline void _rtw_free_sema(_sema *sema)
{
}
static inline void _rtw_up_sema(_sema *sema)
{
	up(sema);
}
static inline u32 _rtw_down_sema(_sema *sema)
{
	if (down_interruptible(sema)) {
		return _FAIL;
	} else {
		return _SUCCESS;
	}
}

/*lock - mutex*/
typedef struct mutex		_mutex;

static inline void _rtw_mutex_init(_mutex *pmutex)
{
	mutex_init(pmutex);
}

static inline void _rtw_mutex_free(_mutex *pmutex)
{
	mutex_destroy(pmutex);
}
__inline static int _rtw_mutex_lock_interruptible(_mutex *pmutex)
{
	int ret = 0;
	/* mutex_lock(pmutex); */
	ret = mutex_lock_interruptible(pmutex);

	return ret;
}

__inline static int _rtw_mutex_lock(_mutex *pmutex)
{
	int ret = 0;
	mutex_lock(pmutex);

	return ret;
}

__inline static void _rtw_mutex_unlock(_mutex *pmutex)
{
	mutex_unlock(pmutex);
}


/*completion*/
typedef struct completion _completion;
static inline void _rtw_init_completion(_completion *comp)
{
	init_completion(comp);
}
static inline unsigned long _rtw_wait_for_comp_timeout(_completion *comp, unsigned long timeout)
{
	return wait_for_completion_timeout(comp, timeout);
}
static inline void _rtw_wait_for_comp(_completion *comp)
{
	return wait_for_completion(comp);
}

struct	__queue	{
	struct	list_head	queue;
	_lock	lock;
};

typedef unsigned char	_buffer;

typedef struct	__queue	_queue;


/*list*/
#define LIST_CONTAINOR(ptr, type, member) \
	((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


typedef struct	list_head	_list;
/* Caller must check if the list is empty before calling rtw_list_delete*/
__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

__inline static _list *get_next(_list	*list)
{
	return list->next;
}
__inline static _list	*get_list_head(_queue *queue)
{
	return &(queue->queue);
}
#define rtw_list_first_entry(ptr, type, member) list_first_entry(ptr, type, member)

/* hlist */
typedef struct	hlist_head	rtw_hlist_head;
typedef struct	hlist_node	rtw_hlist_node;
#define rtw_hlist_for_each_entry(pos, head, member) hlist_for_each_entry(pos, head, member)
#define rtw_hlist_for_each_safe(pos, n, head) hlist_for_each_safe(pos, n, head)
#define rtw_hlist_entry(ptr, type, member) hlist_entry(ptr, type, member)
#define rtw_hlist_for_each_entry_safe(pos, np, n, head, member) hlist_for_each_entry_safe(pos, n, head, member)
#define rtw_hlist_for_each_entry_rcu(pos, node, head, member) hlist_for_each_entry_rcu(pos, head, member)


/* rhashtable */
//#include "../os_dep/linux/rtw_rhashtable.h"


/*thread*/
typedef void *_thread_hdl_;
typedef int thread_return;
typedef void *thread_context;
struct thread_hdl {
	_thread_hdl_ thread_handler;
	u8 thread_status;
};
#define THREAD_STATUS_STARTED BIT(0)
#define THREAD_STATUS_STOPPED BIT(1)
#define RST_THREAD_STATUS(t) (t->thread_status = 0)
#define SET_THREAD_STATUS(t, s) 	(t->thread_status |= s)
#define CLR_THREAD_STATUS(t, cl)	(t->thread_status &= ~(cl))
#define CHK_THREAD_STATUS(t, ck) (t->thread_status & ck)

typedef void timer_hdl_return;
typedef void *timer_hdl_context;

static inline void rtw_thread_enter(char *name)
{
	allow_signal(SIGTERM);
}

static inline void rtw_thread_exit(_completion *comp)
{
	complete_and_exit(comp, 0);
}

static inline _thread_hdl_ rtw_thread_start(int (*threadfn)(void *data),
		void *data, const char namefmt[])
{
	_thread_hdl_ _rtw_thread = NULL;

	_rtw_thread = kthread_run(threadfn, data, namefmt);
	if (IS_ERR(_rtw_thread)) {
		WARN_ON(!_rtw_thread);
		_rtw_thread = NULL;
	}
	return _rtw_thread;
}
static inline bool rtw_thread_stop(_thread_hdl_ th)
{

	return kthread_stop(th);
}
static inline void rtw_thread_wait_stop(void)
{
	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		schedule();
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
}

static inline void flush_signals_thread(void)
{
	if (signal_pending(current)) {
		flush_signals(current);
	}
}

typedef unsigned long systime;

/*tasklet*/
typedef struct tasklet_struct _tasklet;
typedef void (*tasklet_fn_t)(unsigned long);

static inline void rtw_tasklet_init(_tasklet *t, tasklet_fn_t func,
									unsigned long data)
{
	tasklet_init(t, func, data);
}
static inline void rtw_tasklet_kill(_tasklet *t)
{
	tasklet_kill(t);
}

static inline void rtw_tasklet_schedule(_tasklet *t)
{
	tasklet_schedule(t);
}
static inline void rtw_tasklet_hi_schedule(_tasklet *t)
{
	tasklet_hi_schedule(t);
}

/*skb_buffer*/
static inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

static inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline int _rtw_skb_linearize(struct sk_buff *skb)
{
	return skb_linearize(skb);
}

static inline struct sk_buff *_rtw_pskb_copy(struct sk_buff *skb)
{
	return pskb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline u8 *rtw_skb_data(struct sk_buff *pkt)
{
	return pkt->data;
}

static inline u32 rtw_skb_len(struct sk_buff *pkt)
{
	return pkt->len;
}

static inline void *rtw_skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	return skb_put_zero(skb, len);
}

/*timer*/
typedef struct rtw_timer_list _timer;
struct rtw_timer_list {
	struct timer_list timer;
	void (*function)(void *);
	void *arg;
};

static inline void timer_hdl(struct timer_list *in_timer)
{
	_timer *ptimer = from_timer(ptimer, in_timer, timer);

	ptimer->function(ptimer->arg);
}

__inline static void _init_timer(_timer *ptimer, void *pfunc, void *cntx)
{
	ptimer->function = pfunc;
	ptimer->arg = cntx;

	timer_setup(&ptimer->timer, timer_hdl, 0);
}

__inline static void _set_timer(_timer *ptimer, u32 delay_time)
{
	mod_timer(&ptimer->timer, (jiffies + (delay_time * HZ / 1000)));
}

__inline static void _cancel_timer(_timer *ptimer, u8 *bcancelled)
{
	*bcancelled = del_timer_sync(&ptimer->timer) == 1 ? 1 : 0;
}

__inline static void _cancel_timer_async(_timer *ptimer)
{
	del_timer(&ptimer->timer);
}


/*
* Global Mutex: can only be used at PASSIVE level.
*   */
#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1) { \
			atomic_dec((atomic_t *)&(_MutexCounter));        \
			msleep(10);                          \
		}                                                           \
	}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
	}


typedef	struct	net_device *_nic_hdl;
static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
	return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
			netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
			netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
			netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)));
}

#ifdef CONFIG_HWSIM
int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb);
#else
static inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
#if defined(CONFIG_RTW_FC_FASTFWD)
	extern int fwdEngine_wifi_rx(struct sk_buff * skb);
	enum {
		RE8670_RX_STOP = 0,
		RE8670_RX_CONTINUE,
		RE8670_RX_STOP_SKBNOFREE,
		RE8670_RX_END
	};
	int ret = 0;

	skb->dev = ndev;
	skb->data -= 14;
	skb->len += 14;

	ret = fwdEngine_wifi_rx(skb);

	if (ret == RE8670_RX_CONTINUE) {
		skb->data += 14;
		skb->len -= 14;
		return netif_rx(skb);
	} else if (ret == RE8670_RX_STOP) {
		kfree_skb(skb);
	}

	return 0;
#else
	skb->dev = ndev;
	return netif_rx(skb);
#endif
}
#endif

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_wake_all_queues(pnetdev);
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}

static inline int rtw_merge_string(char *dst, int dst_len, const char *src1, const char *src2)
{
	int	len = 0;
	len += snprintf(dst + len, dst_len - len, "%s", src1);
	len += snprintf(dst + len, dst_len - len, "%s", src2);

	return len;
}


#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)), (sig), 1)

/* limitation of path length */
#define PATH_LENGTH_MAX PATH_MAX

/* Atomic integer operations */
static inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	atomic_set(v, i);
}

static inline int ATOMIC_READ(ATOMIC_T *v)
{
	return atomic_read(v);
}

static inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	atomic_add(i, v);
}
static inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	atomic_sub(i, v);
}

static inline void ATOMIC_INC(ATOMIC_T *v)
{
	atomic_inc(v);
}

static inline void ATOMIC_DEC(ATOMIC_T *v)
{
	atomic_dec(v);
}

static inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	return atomic_add_return(i, v);
}

static inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	return atomic_sub_return(i, v);
}

static inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	return atomic_inc_return(v);
}

static inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	return atomic_dec_return(v);
}

static inline bool ATOMIC_INC_UNLESS(ATOMIC_T *v, int u)
{
	return atomic_add_unless(v, 1, u);
}

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) (adapter->pnetdev ? adapter->pnetdev->name : NULL)
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, (adapter->pnetdev ? adapter->pnetdev->name : NULL)

#define rtw_netdev_idx(netdev) (((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->wlan_idx)
struct rtw_netdev_priv_indicator {
	void *priv;
	u32 wlan_idx;
};


#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(dev_net(ndev), name)

static inline void rtw_dump_stack(void)
{
	dump_stack();
}
#define rtw_bug_on(condition) BUG_ON(condition)
#define rtw_warn_on(condition) WARN_ON(condition)
#define RTW_DIV_ROUND_UP(n, d)	DIV_ROUND_UP(n, d)
#define rtw_sprintf(buf, size, format, arg...) snprintf(buf, size, format, ##arg)

#define STRUCT_PACKED __attribute__ ((packed))

#ifndef fallthrough
#if __GNUC__ >= 5 || defined(__clang__)
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#if __has_attribute(__fallthrough__)
#define fallthrough __attribute__((__fallthrough__))
#endif
#endif
#ifndef fallthrough
#define fallthrough do {} while (0) /* fallthrough */
#endif
#endif

#endif /* __OSDEP_LINUX_SERVICE_H_ */
