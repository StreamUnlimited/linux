/* ATTENTION */
/* Please keep the realtek-misc header for kernel space and user space in sync manually. */
/* Misc header for kernel space: <sdk>/openwrt/realtek/linux-5.4/include/uapi/misc/realtek-misc.h */
/* Misc header for user space: <sdk>/openwrt/realtek/application/prebuilts/kernel/include/uapi/misc/realtek-misc.h */

#define RTK_CMD_NOT_SET				0  // default: mode not set
#define RTK_CMD_SET_MODE			1  // call ioctl SET_CURRENT_MODE to change the affair modes.

/* Example of misc affairs. */
#define RTK_CMD_READ_FLASH_TYPE     2
#define RTK_CMD_SET_BT_POWER_ON     3
#define RTK_CMD_SET_BT_POWER_OFF    4

/* Add customize affairs here. */
/* RTK_CMD_AAAAAA_AAAAAA_AA */
/* RTK_CMD_AAAAAA_BBBBBB_AA */