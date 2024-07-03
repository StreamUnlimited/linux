// SPDX-License-Identifier: GPL-2.0-only

#ifndef _SUE_HSMP_COMMON_H_
#define _SUE_HSMP_COMMON_H_

struct __attribute__((__packed__)) sue_hsmp_packet {
	// Generic fields, compatible with e.g. HostLink
	uint8_t packet_start_lsb;
	uint8_t packet_start_msb;
	uint8_t packet_length_lsb;
	uint8_t packet_length_msb;

	// HSMP  specific fields
	uint8_t command_type;
	uint8_t register_addr;
	uint32_t value;
	uint32_t mask;
};

#define HSMP_PACKET_START_HEADER	0xFDFF

#define HSMP_COMMAND_READ		0x00
#define HSMP_COMMAND_WRITE		0x01
#define HSMP_COMMAND_UPDATE_BITS	0x02

#define HSMP_POWER_STATE_REG		0x00
#define HSMP_GPIO_VAL_REG		0x01
/*
 * Controls the color of the RGB LED in the `STATE_STREAMSDK` state
 * Bits: [24:16] red, [16:8] green, [8:0] blue
 */
#define HSMP_RGB_LED_REG		0x10

#define HSMP_MAX_REG			HSMP_RGB_LED_REG

enum hsmp_power_state {
	STATE_POWERON = 0x00,
	STATE_UBOOT = 0x01,
	STATE_KERNEL = 0x02,
	STATE_SWUPDATE = 0x03,
	STATE_STREAMSDK = 0x04,
	STATE_NETWORK_STANDBY = 0x05,
	STATE_SUSPEND_TO_RAM = 0x06,
	STATE_POWEROFF = 0x07,
	STATE_ERROR = 0xFFFFFFFF,
};

static const struct {
	enum hsmp_power_state value;
	const char *name;
} hsmp_power_state_names[] = {
	{ STATE_POWERON, "STATE_POWERON" },
	{ STATE_UBOOT, "STATE_UBOOT" },
	{ STATE_KERNEL, "STATE_KERNEL" },
	{ STATE_SWUPDATE, "STATE_SWUPDATE" },
	{ STATE_STREAMSDK, "STATE_STREAMSDK" },
	{ STATE_NETWORK_STANDBY, "STATE_NETWORK_STANDBY" },
	{ STATE_SUSPEND_TO_RAM, "STATE_SUSPEND_TO_RAM" },
	{ STATE_POWEROFF, "STATE_POWEROFF" },
	{ STATE_ERROR, "STATE_ERROR" },
};

#endif
