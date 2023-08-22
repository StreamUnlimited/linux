#ifndef __INIC_LINUX_BASE_TYPE__
#define __INIC_LINUX_BASE_TYPE__

#include "rtw_llhw_trx.h"

#define INIC_MAX_NET_PORT_NUM		(2)
#define INIC_STA_PORT			(0)
#define INIC_AP_PORT			(1)

#define IPC_MSG_QUEUE_DEPTH		(50)
#define IPC_MSG_QUEUE_WARNING_DEPTH	(4)

#define HOST_MSG_PARAM_NUM		(9)
#define DEV_MSG_PARAM_NUM		(6)

#define RTW_IP_ADDR_LEN 4

/* TODO: typeof */
/* Layer 2 structs. */

enum {
	/* Separated from IPC, can add/delete independently. */
	RTW_JOINSTATUS_UNKNOWN = 0,
	RTW_JOINSTATUS_STARTING,
	RTW_JOINSTATUS_SCANNING,
	RTW_JOINSTATUS_AUTHENTICATING,
	RTW_JOINSTATUS_AUTHENTICATED,
	RTW_JOINSTATUS_ASSOCIATING,
	RTW_JOINSTATUS_ASSOCIATED,
	RTW_JOINSTATUS_4WAY_HANDSHAKING,
	RTW_JOINSTATUS_4WAY_HANDSHAKE_DONE,
	RTW_JOINSTATUS_SUCCESS,
	RTW_JOINSTATUS_FAIL,
	RTW_JOINSTATUS_DISCONNECT,
	//TODO: RTW_JOINSTATUS_ABORTED,
};
typedef unsigned int fullmac_join_status;

struct inic_ipc_host_req_msg {
	u32				api_id;
	u32				param_buf[HOST_MSG_PARAM_NUM];
	int				ret;
	u8				dummy[20]; /* add for 64B size alignment */
};

struct inic_ipc_ex_msg {
	u32				event_num;
	u32				msg_addr;
	u32				msg_queue_status;
	u32				wlan_idx;
	u32				rsvd[12]; /* keep total size 64B aligned */
};

struct ipc_msg_node {
	struct list_head		list;
	struct inic_ipc_ex_msg		ipc_msg; /* to store ipc message */
	bool				is_used; /* sign whether to be used */
};

/* Layer 1 structs. */
struct event_priv_t {
	struct tasklet_struct		api_tasklet; /* event_priv task to haddle event_priv msg */
	ipc_msg_struct_t		api_ipc_msg; /* to store ipc msg for event_priv */
	struct mutex			iiha_send_mutex; /* mutex to protect send host event_priv message */
	spinlock_t			event_lock; /* lock to protect indicate event */
	struct inic_ipc_host_req_msg	*preq_msg;/* host event_priv message to send to device */
	dma_addr_t			req_msg_phy_addr;/* host event_priv message to send to device */
	uint32_t			*dev_req_network_info;
	dma_addr_t			dev_req_network_info_phy;
	ipc_msg_struct_t		recv_ipc_msg;
};

struct ipc_msg_q_priv {
	struct list_head		queue_head; /* msg queue */
	spinlock_t			lock; /* queue lock */
	struct work_struct		msg_work; /* message task in linux */
	struct mutex			msg_work_lock; /* tx lock lock */
	void	(*task_hdl)(struct inic_ipc_ex_msg *);    /* the haddle function of task */
	bool				b_queue_working; /* flag to notice the queue is working */
	struct ipc_msg_node		ipc_msg_pool[IPC_MSG_QUEUE_DEPTH]; /* static pool for queue node */
	int				queue_free; /* the free size of queue */
	struct inic_ipc_ex_msg		*p_inic_ipc_msg;/* host inic ipc message to send to device */
	dma_addr_t			ipc_msg_phy_addr;/* host inic message to send to device */
	spinlock_t			ipc_send_msg_lock;
};

struct xmit_priv_t {
	struct skb_info			*host_skb_info;
	dma_addr_t			host_skb_info_phy;
	struct skb_data			*host_skb_data;
	dma_addr_t			host_skb_data_phy;
	atomic_t				skb_free_num;
	u32				skb_idx;
	spinlock_t			skb_lock;
};

/* Scan and Join related parameters. */
struct mlme_priv_t {
	/* scan parameters. */
	struct cfg80211_scan_request	*pscan_req_global;

	/* join parameters. */
	internal_join_block_param_t	*join_block_param;
	fullmac_join_status		rtw_join_status;
	u8				assoc_req_ie[ASSOC_IE_MAX_LEN];
	u8				assoc_rsp_ie[ASSOC_IE_MAX_LEN];
	size_t				assoc_req_ie_len;
	size_t				assoc_rsp_ie_len;
	struct cfg80211_external_auth_params auth_ext_para;
};

struct inic_device {
	/* device register to upper layer. */
	struct device			*fullmac_dev;
	struct wiphy			*pwiphy_global;
	struct net_device		*pndev[INIC_MAX_NET_PORT_NUM];
	struct wireless_dev		*pwdev_global[INIC_MAX_NET_PORT_NUM];

	/* ops for upper layer. */
	struct cfg80211_ops		rtw_cfg80211_ops;
	struct ethtool_ops		rtw_ethtool_ops;

	/* IPC related parameters. */
	struct device			*ipc_dev;
	aipc_ch_t			*data_ch;
	aipc_ch_t			*event_ch;
	struct event_priv_t		event_priv;
	struct ipc_msg_q_priv		msg_priv;
	struct xmit_priv_t		xmit_priv;

	/* fullmac status management. */
	struct mlme_priv_t		mlme_priv;
	struct net_device_stats		stats[INIC_MAX_NET_PORT_NUM];
	u8				ip_addr[RTW_IP_ADDR_LEN];
	u8				wowlan_state; /* 0: resume, 1: suspend */
};
/* TODO: kzalloc to platform device private data instead of axi_data. */
extern struct inic_device global_idev;

#define MAX_NUM_WLAN_PORT		(2)
#define FLAG_WLAN_IF_NOT_RUNNING	((u32)0xFFFFFFFF)
#define IPC_USER_POINT			0

enum IPC_WIFI_CTRL_TYPE {
	IPC_WIFI_MSG_READ_DONE = 0,
	IPC_WIFI_MSG_MEMORY_NOT_ENOUGH,
	IPC_WIFI_MSG_RECV_DONE,
	IPC_WIFI_CMD_XIMT_PKTS,
	IPC_WIFI_EVT_RECV_PKTS,
	IPC_WIFI_EVT_TX_DONE
};

#endif /* __INIC_LINUX_BASE_TYPE__ */
