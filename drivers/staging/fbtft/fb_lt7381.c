/*
 * TFT driver for the LT7381 generic LCD graphics controller
 *
 * The driver supports only partial functionality of the TFT graphics controller
 * over a SPI interface. The driver makes it possible to configure a variety of
 * of TFT displays over a SPI-interface, by modifying only the DT.
 *
 * It support resolutions from 320*240(QVGA) to 1024*768(SXGA), 24bit RGB,
 * according to datasheet (in theory). The frame rate is not necessarily
 * guaranteed, neither is a particular display or resolution guaranteed. It uses
 * a 8-bit data channel, for which the SPI transfer speed can be a bottle neck
 * for a particular frame rate.
 *
 * Populate the display paramters in the DT when adding support for a new
 * display. It is possible to enable the test pattern with the test-pattern
 * entry to ease debugging. It will generate a test pattern independent of the
 * display ram and memory clock.
 *
 * NOTE1: Only register 0x00 and be directly modified which holds the address
 * pointer (AP). A register will be modified when its address is stored in the
 * AP while writing directly to register 0x80. The value of a register can also
 * be retrieved when reading directly from register 0xC0. Reading directly from
 * register 0x04 returns the value of the status register. The direct address
 * is set when TX the address value on the SPI directly after a CS. The exchange
 * of data happens directly after setting the direct address. The direct address
 * needs to be set again after the CS de-selects.
 *
 * Copyright (C) 2019 StreamUnlimited
 * Inspired from fb_ili9341.c by Christian Vogelgsang
 * and fb_watterott.c by Noralf Tronnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <video/mipi_display.h>
#include <linux/gpio.h>
#include <video/of_display_timing.h>
#include <video/display_timing.h>
#include <linux/spi/spi.h>

#include "fbtft.h"

#define DRVNAME				"fb_lt7381"
#define TXBUFLEN			(32 * PAGE_SIZE)
#define MIN_BRIGHTNESS			0   /* percentage */
#define DEFAULT_BRIGHTNESS		75  /* percentage */
#define MAX_BRIGHTNESS			100 /* percentage */

#define MHz				1000000     /* multiplier    */
#define FREQ_SCALING			100000      /* multiplier    */
#define MCLK_FREQ			(133 * MHz) /* Hz            */
#define CCLK_FREQ			(100 * MHz) /* Hz            */
#define PWM_RELOAD_VALUE		1800        /* counts        */
#define SDRAM_TIMEOUT			10          /* ms            */
#define SDRAM_TIMEOUT_RETRIES		10          /* no unit       */
#define STARTUP_TIMEOUT			10          /* ms            */
#define STARTUP_TIMEOUT_RETRIES		10          /* no unit       */
#define PLL_LOCK_TIMEOUT		10          /* ms            */
#define PLL_LOCK_TIMEOUT_RETRIES	10          /* no unit       */
/* SPI bus array: MISO | MOSI = [startbyte, VALUE_BYTE] */
#define BUS_REG_VAL_BYTE		1           /* byte position */
#define RESET_HOLD_TIME			100         /* ms            */
#define RESET_RELEASE_TIME		120         /* ms            */

/******************************************
 * LT7381 register specific defines start *
 ******************************************/
#define LT7381_PCLK_MAX			(80 * MHz)
#define LT7381_MCLK_MAX			(133 * MHz)
#define LT7381_CCLK_MAX			(100 * MHz)

#define LT7381_TREF			64   /* ms    */
#define LT7381_ROW_SIZE			4096 /* bytes */

/*
 * Host interface cycle
 *----+                                                               +----
 *SCS#|                                                               |
 *    +---------------------------------------------------------------+
 *
 *      1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
 *----+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^-+ ^------
 *SCLK| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 *    +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+
 *
 *    +---+---+                       +---+---+---+---+---+---+---+---+
 *  SD|A0 |RW#|                       |b7 |b6 |b5 |b4 |b3 |b2 |b1 |b0 |
 *----+---+---+-----------------------+---+---+---+---+---+---+---+---+----
 *
 * Table 14-1 (Short)
 * +----------------------+----+----+---------------------------+
 * | Host interface cycle | A0 | RW | Action Description        |
 * +----------------------+----+----+---------------------------+
 * | Command Write        | 0  | 0  | Write address of register |
 * | Status Read          | 0  | 1  | Read status register      |
 * | Data write           | 1  | 0  | Write data to reg or mem  |
 * | Data Read            | 1  | 1  | Read data from reg or mem |
 * +----------------------+----+----+---------------------------+
 */
#define LT7381_COMMAND_WRITE		0x00
#define LT7381_DATA_WRITE		0x80
#define LT7381_DATA_READ		0xc0
#define LT7381_STATUS_READ		0x40

#define LT7381_SRR			0x00
#define LT7381_CCR			0x01
#define LT7381_MACR			0x02
#define LT7381_ICR			0x03
#define LT7381_MRWDP			0x04
#define LT7381_PPLLC1			0x05
#define LT7381_PPLLC2			0x06
#define LT7381_MPLLC1			0x07
#define LT7381_MPLLC2			0x08
#define LT7381_CPLLC1			0x09
#define LT7381_CPLLC2			0x0a
#define LT7381_INTEN			0x0b
#define LT7381_INTF			0x0c
#define LT7381_MINTFR			0x0d
#define LT7381_PUENR			0x0e
#define LT7381_PSFSR			0x0f
#define LT7381_MPWCTR			0x10
#define LT7381_PIPCDEP			0x11
#define LT7381_DPCR			0x12
#define LT7381_PCSR			0x13
#define LT7381_HDWR			0x14
#define LT7381_HDWFTR			0x15
#define LT7381_HNDR			0x16
#define LT7381_HNDFTR			0x17
#define LT7381_HSTR			0x18
#define LT7381_HPWR			0x19
#define LT7381_VDHR_0			0x1a
#define LT7381_VDHR_1			0x1b
#define LT7381_VNDR_0			0x1c
#define LT7381_VNDR_1			0x1d
#define LT7381_VSTR			0x1e
#define LT7381_VPWR			0x1f

#define LT7381_MIW_0			0x24
#define LT7381_MIW_1			0x25

#define LT7381_AW_COLOR			0x5e
/*
 * Below is the meaning of CURH for the different modes.
 * DPRAM in Linear mode:
 * Memory Read/Write address[15:0], in bytes.
 * DPRAM in Block mode:
 * Graphic Read/Write X-Coordinate [12:0], in pixels
 */
#define LT7381_CURH_0			0x5f
#define LT7381_CURH_1			0x60
/*
 * Below is the meaning of CURV for the different modes.
 * DPRAM in Linear mode:
 * Memory Read/Write address[31:16], in bytes.
 * DPRAM in Block mode:
 * Graphic Read/Write Y-Coordinate [12:0], in pixels
 */
#define LT7381_CURV_0			0x61
#define LT7381_CURV_1			0x62

#define LT7381_PSCLR			0x84
#define LT7381_PMUXR			0x85
#define LT7381_PCFGR			0x86
#define LT7381_DZ_LENGTH		0x87
#define LT7381_TCMPB0_0			0x88
#define LT7381_TCMPB0_1			0x89
#define LT7381_TCNTB0_0			0x8a
#define LT7381_TCNTB0_1			0x8b
#define LT7381_TCMPB1_0			0x8c
#define LT7381_TCMPB1_1			0x8d
#define LT7381_TCNTB1_0			0x8e
#define LT7381_TCNTB1_1			0x8f

#define LT7381_SDRAR			0xe0
#define LT7381_SDRMD			0xe1
#define LT7381_SDR_REF_0		0xe2
#define LT7381_SDR_REF_1		0xe3
#define LT7381_SDRCR			0xe4

/* Operation mode status */
#define LT7381_STATUS_OM_INHIBIT_FLAG	BIT(1)
/* DRAM ready status */
#define LT7381_STATUS_DRAM_RDY_FLAG	BIT(2)

#define LT7381_SRR_RECONF_PLL		BIT(7)

#define LT7381_CCR_PLL_READY_FLAG	BIT(7)

/* Memory port destination */
#define LT7381_ICR_IMG_BUFFER		0x00
#define LT7381_ICR_GAMMA_TABLE		BIT(0)
#define LT7381_ICR_GRAPHIC_CURSOR	BIT(1)
#define LT7381_ICR_COLOR_PALETTE	(BIT(0) | BIT(1))
/* Text mode support */
#define LT7381_ICR_GRAPHIC_MODE		0
#define LT7381_ICR_TEXT_MODE		BIT(2)
/* The interrupt flags are not currently used */

#define LT7381_DPCR_DISPLAY_TSTBAR_MASK	BIT(5)
#define LT7381_DPCR_DISPLAY_TSTBAR_DE	0
#define LT7381_DPCR_DISPLAY_TSTBAR_EN	BIT(5)
#define LT7381_DPCR_DISPLAY_ONOFF_MASK	BIT(6)
#define LT7381_DPCR_DISPLAY_OFF		0x00
#define LT7381_DPCR_DISPLAY_ON		BIT(6)
/* PCLK inversion */
#define LT7381_DPCR_PCLK_RISING_EDGE	0
#define LT7381_DPCR_PCLK_FALLING_EDGE	BIT(7)

/* PDE polarity */
#define LT7381_PCSR_PDE_HIGH		0x00
#define LT7381_PCSR_PDE_LOW		BIT(5)

/* display modes */
#define LT7381_AW_COLOR_BLOCK_MODE	0x00
#define LT7381_AW_COLOR_LINEAR_MODE	BIT(2)
/* linear color mode is limited up to 16bpp */
#define LT7381_AW_COLOR_8BPP		0x00
#define LT7381_AW_COLOR_16BPP		BIT(0)
#define LT7381_AW_COLOR_24BPP		BIT(1)

#define LT7381_PSCLR_PRESCALE_VAL	0

/* PWM[0] Function cpntrol */
#define LT7381_PMUXR_PWM0_OUT_GPIOC	0x00
#define LT7381_PMUXR_PWM0_OUT_TIMER0	BIT(1)
#define LT7381_PMUXR_PWM0_CCLK		(BIT(0) | BIT(1))
/* PWM[1] Function cpntrol */
#define LT7381_PMUXR_PWM1_OUT_SYS_ERR	0x00
#define LT7381_PMUXR_PWM1_OUT_TIMER1	BIT(3)
#define LT7381_PMUXR_PWM1_OUT_OSC_CLK	(BIT(2) | BIT(3))
/* PWM0 Timer-0 divisor */
#define LT7381_PMUXR_TIMER0_DIV_1	0x00
#define LT7381_PMUXR_TIMER0_DIV_2	BIT(4)
#define LT7381_PMUXR_TIMER0_DIV_4	BIT(5)
#define LT7381_PMUXR_TIMER0_DIV_8	(BIT(4) | BIT(5))
/* PWM1 Timer-1 divisor */
#define LT7381_PMUXR_TIMER1_DIV_1	0x00
#define LT7381_PMUXR_TIMER1_DIV_2	BIT(6)
#define LT7381_PMUXR_TIMER1_DIV_4	BIT(7)
#define LT7381_PMUXR_TIMER1_DIV_8	(BIT(6) | BIT(7))

#define LT7381_PCFGR_TIMER1_START	BIT(4)
/* PWM Timer-1 auto reload nn/off */
#define LT7381_PCFGR_TIMER1_ONE_SHOT	0x00
#define LT7381_PCFGR_TIMER1_AUTO_RELOAD	BIT(5)

#define LT7381_SDRAR_SDR_BANK		BIT(5)
#define LT7381_SDRMD_CASLAT		(BIT(0) | BIT(1))

#define LT7381_SDRCR_INITDONE		BIT(0)

/* LT7381 register specific defines end */

struct lt7381_ctrl {
	int cclk_freq;
	int pwm_cnt;
	u32 max_brightness_perc;
	u32 def_brightness_perc;
};

static u8 lt7381_read_val(struct fbtft_par *par)
{
	int ret;
	u8 buffer[2]; /* empty byte + value */

	/*
	 * Set the direct address to READ_REGISTER, it will return the value of
	 * the register pointed by the AP, see NOTE1. The device will return the
	 * value of the register, after we write READ_REGISTER as the startbyte
	 * value. Startbyte allows a value to be TX on the bus before we read
	 * from the bus.
	 */
	par->startbyte = LT7381_DATA_READ;
	ret = par->fbtftops.read(par, buffer, sizeof(buffer));
	if (ret < 0) {
		dev_err(par->info->device,
			"%s failed and returned %d\n", __func__, ret);
	}

	/* SPI bus array: MISO | MOSI = [startbyte, VALUE_BYTE] */
	return buffer[BUS_REG_VAL_BYTE];
}

static u8 lt7381_read_status(struct fbtft_par *par)
{
	int ret;
	u8 buffer[2]; /* empty byte + value */

	/*
	 * Set the direct address to STATUS_REGISTER, it will return the value
	 * of the status register pointed, see NOTE1. The device will return the
	 * value of the status register, after we write STATUS_REGISTER as the
	 * startbyte value. Startbyte allows a value to be TX on the bus before
	 * we read from the bus.
	 */
	par->startbyte = LT7381_STATUS_READ;
	ret = par->fbtftops.read(par, buffer, sizeof(buffer));
	if (ret < 0) {
		dev_err(par->info->device,
			"%s failed and returned %d\n", __func__, ret);
	}

	/* SPI bus array: MISO | MOSI = [startbyte, VALUE_BYTE] */
	return buffer[BUS_REG_VAL_BYTE];
}

static void lt7381_write_addr(struct fbtft_par *par, u8 addr)
{
	/* See NOTE1 */
	write_reg(par, LT7381_COMMAND_WRITE, addr);
}

static void lt7381_write_val(struct fbtft_par *par, u8 data)
{
	/* See NOTE1 */
	write_reg(par, LT7381_DATA_WRITE, data);
}

static void lt7381_write_reg(struct fbtft_par *par, u8 addr, u8 data)
{
	/* See NOTE1 */
	lt7381_write_addr(par, addr);
	lt7381_write_val(par, data);
}

static u8 lt7381_read_reg(struct fbtft_par *par, u8 addr)
{
	/* See NOTE1 */
	lt7381_write_addr(par, addr);
	return lt7381_read_val(par);
}

static void lt7381_update_reg(struct fbtft_par *par, u8 addr, u8 mask, u8 data)
{
	u8 val;

	lt7381_write_addr(par, addr);
	val = (lt7381_read_val(par) & ~mask) | (mask & data);
	lt7381_write_val(par, val);
}

static void lt7381_set_brightness(struct fbtft_par *par, u32 pwm_count,
		u8 percentage)
{
	lt7381_write_reg(par, LT7381_TCMPB1_0, ((pwm_count * percentage) / 100)
			& 0xff);
	lt7381_write_reg(par, LT7381_TCMPB1_1, ((pwm_count * percentage) / 100)
			>> 8);
}

static void lt7381_set_pwm_backlight(struct fbtft_par *par)
{
	struct device_node *np = par->info->device->of_node;
	struct lt7381_ctrl *ltc = par->extra;
	u32 pwm_count;

	/* The backlight functionality is not registered if the PWM reload value
	 * is not specified in the DT. PWM is optional, so it is not necessarily
	 * an error. */
	if (of_property_read_u32(np, "pwm-count", &pwm_count) < 0) {
		dev_dbg(par->info->device,"%s: pwm backlight not used, no entry found\n",
				of_node_full_name(np));
		ltc->pwm_cnt = -EINVAL;
		return;
	}
	ltc->pwm_cnt = pwm_count;
	ltc->max_brightness_perc = MAX_BRIGHTNESS;
	ltc->def_brightness_perc = DEFAULT_BRIGHTNESS;
	(void)of_property_read_u32(np, "backlight-max-brightness",
			&ltc->max_brightness_perc);
	(void)of_property_read_u32(np, "backlight-default-brightness",
			&ltc->def_brightness_perc);

	/* Set prescaler: Core_Freq / (Prescaler + 1) */
	lt7381_write_reg(par, LT7381_PSCLR, LT7381_PSCLR_PRESCALE_VAL);

	/* Use PWM 1 and divide input to timer1 by 1/4 */
	lt7381_write_reg(par, LT7381_PMUXR, LT7381_PMUXR_PWM1_OUT_TIMER1 |
			LT7381_PMUXR_TIMER1_DIV_4);

	/* Set auto reload and start timer1 */
	lt7381_write_reg(par, LT7381_PCFGR, LT7381_PCFGR_TIMER1_START |
			LT7381_PCFGR_TIMER1_AUTO_RELOAD);

	/* Set reload value */
	lt7381_write_reg(par, LT7381_TCNTB1_0, pwm_count & 0xff);
	lt7381_write_reg(par, LT7381_TCNTB1_1, pwm_count >> 8);

	lt7381_set_brightness(par, pwm_count, DEFAULT_BRIGHTNESS);
}

static int lt7381_get_pll_parameters(u32 xi, u32 f_out, int *diff_perc,
		u32 *pOD, u32 *pN, u32 *pR)
{
	int best_diff = -EINVAL;
	int od, r, r_max;

	/*
	 *                 +-------+
	 *        +---+    |   +   |    +--------+       +----+
	 * xi --->| R |--->|       |--->| system |---+---| OD |---> f_out
	 *        +---+    |   -   |    +--------+   |   +----+
	 *                 +-------+                 |
	 *                     ^                     |
	 *                     |        +---+        |
	 *                     +--------| N |--------+
	 *                              +---+
	 * OD = [1, 2, 4]
	 * R  = [2, ..., 31]
	 * N  = [2, ..., 510]
	 *
	 * f_out = xi * (N/R) % OD
	 * f_input = xi / R >= 1MHz
	 */

	/* The scaling below prevent computation overflows by dividing the
	 * frequencies with a 100000. The best resolution accuracy will be
	 * 0.1 MHz, not necessarily guaranteed */
	xi /= FREQ_SCALING;
	f_out /= FREQ_SCALING;

	/* r <= 31 and xi / r >= 1MHz */
	r_max = min(31, (int)(xi / (MHz / FREQ_SCALING)));
	for (od = 1; od <= 4; od = od << 1) {
		for (r = 2; r <= r_max; r++) {
			int n = (f_out * r * od ) / xi;
			if (n >= 2 && n <= 510) {
				/* Local best_diff */
				int diff = abs(((xi * n) / (r * od)) - f_out);
				/* Initialise the values or set values to the
				 * smallest difference */
				if (best_diff == -EINVAL ||
						(diff < best_diff)) {
					best_diff = diff;
					*pOD = od;
					*pR = r;
					*pN = n;
					/* Stop seeking when difference is zero
					 */
					if (!diff) {
						*diff_perc = 0;
						return 0;
					}
				}
			}
		}
	}

	if (best_diff != -EINVAL) {
		/* Calculate the best diff in percentage */
		*diff_perc = (best_diff * 100) / (f_out + best_diff);
		return 0;
	}

	return -EINVAL;
}

static int lt7381_update_clock_registers(struct fbtft_par *par,
	u32 xtal_freq, u8 reg, u32 f_out)
{
	struct lt7381_ctrl *ltc = par->extra;
	int freq_dec_denom = (MHz / FREQ_SCALING);
	int diff_perc, f_true;
	u32 od, r, n;
	int ret;

	if ((ret = lt7381_get_pll_parameters(xtal_freq, f_out, &diff_perc,
			&od, &n, &r)) < 0) {
		dev_err(par->info->device,
				"unable to compute the pll parameters for xi: %d and f_out: %d\n",
				xtal_freq, f_out);
		return ret;
	}

	/* Frequency value at one tenth of a MHz */
	f_true = ((xtal_freq / FREQ_SCALING) * n) / (r * od);

	/* Issue a warning when the error is more than 10%. Just notify for
	 * smaller errors. */
	if (diff_perc >= 10)
		dev_warn(par->info->device,
				"f_out inaccurate: %d.%d MHz, OD: %d, N: %d, R: %d \n",
				f_true / freq_dec_denom,
				f_true % freq_dec_denom,
				od, n, r);
	else if (diff_perc)
		dev_notice(par->info->device,
				"f_out imprecise: %d.%d MHz, OD: %d, N: %d, R: %d \n",
				f_true / freq_dec_denom,
				f_true % freq_dec_denom,
				od, n, r);
	else
		dev_dbg(par->info->device,
				"f_out: %d.%d Mhz, OD: %d, N: %d, R: %d \n",
				f_true / freq_dec_denom,
				f_true % freq_dec_denom,
				od, n, r);

	/*
	 * The register definition is the same for all the PLL Control Registers
	 * +--------+-------------------------------+---------+
	 * | Bit(s) | Description                   | Range   |
	 * +--------+-------------------------------+---------+
	 * | 7:6    | Output Divider Ratio, OD[1:0] | 1, 2, 4 |
	 * +--------+-------------------------------+---------+
	 * | 5:1    | Input Divider Ratio, R[4:0]   | 2~31    |
	 * +--------+-------------------------------+---------+
	 * | 0      | Feedback Divider Ratio, N[8]  | 2~511   |
	 * +--------+-------------------------------+---------+
	 */
	lt7381_write_reg(par, reg, ((od - 1) << 6) | (r << 1) |
			((n >> 8) & 0x01));
	if (reg == LT7381_CPLLC1)
		ltc->cclk_freq = f_true;
	/*
	 * This is part2 of the control registers
	 * +--------+--------------------------------+-------+
	 * | Bit(s) | Description                    | Range |
	 * +--------+--------------------------------+-------+
	 * | 7:0    | Feedback Divider Ratio, N[7:0] | 2~511 |
	 * +--------+--------------------------------+-------+
	 */
	lt7381_write_reg(par, reg + 1, n & 0xff);

	return 0;
}

static int lt7381_setup_clocks(struct fbtft_par *par, struct display_timing *dt)
{
	struct device_node *np = par->info->device->of_node;
	int retry_count = 0;
	u32 xtal_freq;
	int ret;

	if ((ret = of_property_read_u32(np, "xtal-frequency", &xtal_freq))
			< 0) {
		dev_err(par->info->device,"%s: could not find property xtal-frequency\n",
			of_node_full_name(np));
		return ret;
	}

	if (dt->pixelclock.typ > LT7381_PCLK_MAX)
		dev_warn(par->info->device,"PPLL should not exceeds 80MHz\n");
	if (MCLK_FREQ > LT7381_MCLK_MAX)
		dev_warn(par->info->device,"MPLL should not exceeds 133MHz\n");
	if (CCLK_FREQ > LT7381_CCLK_MAX)
		dev_warn(par->info->device,"CPLL should not exceeds 100MHz\n");

	/* Setup pixel clock */
	if ((ret = lt7381_update_clock_registers(par, xtal_freq, LT7381_PPLLC1,
					dt->pixelclock.typ)) < 0)
		return ret;
	/* Setup memory clock*/
	if ((ret = lt7381_update_clock_registers(par, xtal_freq, LT7381_MPLLC1,
					MCLK_FREQ)) < 0)
		return ret;
	/* Setup cclk for Host interface*/
	if ((ret = lt7381_update_clock_registers(par, xtal_freq, LT7381_CPLLC1,
					CCLK_FREQ)) < 0)
		return ret;

	/* Reconfigure PLL frequency for all the clocks */
	lt7381_write_reg(par, LT7381_SRR, LT7381_SRR_RECONF_PLL);
	/* Wait till PLL lock or timeout */
	while (!(lt7381_read_reg(par, LT7381_CCR) &
				LT7381_CCR_PLL_READY_FLAG)) {
		if (++retry_count >= PLL_LOCK_TIMEOUT_RETRIES)
			return -EIO;
		msleep(PLL_LOCK_TIMEOUT);
	}

	return 0;
}

static void lt7381_setup_timing(struct fbtft_par *par,
		struct display_timing *dt)
{
	/*
	 * All the timing values are 16-bit values. Every register has its own
	 * description on how to set the value of its 8-bit portion if the
	 * relevant timing parameter.
	 */

	/* Set display width */
	lt7381_write_reg(par, LT7381_HDWR, (dt->hactive.typ / 8) - 1);
	lt7381_write_reg(par, LT7381_HDWFTR, dt->hactive.typ % 8);
	/* Set display height */
	lt7381_write_reg(par, LT7381_VDHR_0, (dt->vactive.typ - 1) & 0xff);
	lt7381_write_reg(par, LT7381_VDHR_1, (dt->vactive.typ - 1) >> 8);

	/* Horizontal non display (horizontal back portch) */
	lt7381_write_reg(par, LT7381_HNDR, (dt->hback_porch.typ / 8) - 1);
	lt7381_write_reg(par, LT7381_HNDFTR, dt->hback_porch.typ % 8);
	/* Verical non display (vertical back portch) */
	lt7381_write_reg(par, LT7381_VNDR_0, (dt->vback_porch.typ - 1) & 0xff);
	lt7381_write_reg(par, LT7381_VNDR_1, (dt->vback_porch.typ - 1) >> 8);

	/* HSYNC start position (horizontal front portch) */
	lt7381_write_reg(par, LT7381_HSTR, (dt->hfront_porch.typ / 8) - 1);
	/* VSYNC start position (vertical front portch) */
	lt7381_write_reg(par, LT7381_VSTR, (dt->vfront_porch.typ - 1) & 0xff);

	/* HSYNC pulse width */
	lt7381_write_reg(par, LT7381_HPWR, (dt->hsync_len.typ / 8) - 1);
	/* VSYNC pulse width */
	lt7381_write_reg(par, LT7381_VPWR, (dt->vsync_len.typ - 1) & 0xff);
}

static void lt7381_setup_line_polarity(struct fbtft_par *par,
		struct display_timing *dt)
{
	lt7381_write_reg(par, LT7381_PCSR, dt->flags & DISPLAY_FLAGS_DE_HIGH ?
			LT7381_PCSR_PDE_LOW : LT7381_PCSR_PDE_HIGH);
}

static int lt7381_setup_sdram(struct fbtft_par *par)
{
	struct lt7381_ctrl *ltc = par->extra;
	int retry_count = 0;
	int sdr_ref_val = 0;

	/*
	 * Table 14-5: The initialize of REG[E0h] (SDRAR)
	 * +---------------------------+----------+-----------------------+
	 * | Embedded Display RAM Type | REG[E0h] | Description           |
	 * +---------------------------+----------+-----------------------+
	 * | 32Mb(4MB, 2M*16)          | 0x20     | Bank no: 4, Row Size: |
	 * |                           |          | 2048, ColumnSize: 256 |
	 * +---------------------------+----------+-----------------------+
	 * Note from datasheet: The value of register REG[E0h] must be set
	 * according to above table. Otherwise, the display of TFT panel will
	 * behave undefined and the image can be garbled.
	 */
	lt7381_write_reg(par, LT7381_SDRAR, LT7381_SDRAR_SDR_BANK);
	/* It is unclear from the datasheet what the different case latency
	 * values are, but it is suggested to turn on both.*/
	lt7381_write_reg(par, LT7381_SDRMD, LT7381_SDRMD_CASLAT);
	/* This value is the SDRAM auto refresh interval. The value dependents
	 * on the RAM PLL (CPLL) and the row size.
	 * sdr_ref_val <= (Tref * CCLK_FREQ) / row_size
	 * According to the datasheet:
	 * Tref = 64ms
	 * row_size = 4096, I don't know why bank 4 is specified as 2048 in
	 * table 14-5
	 * We set the refresh rate to the max allowed value, because that is the
	 * example the datasheet uses and the minimum requirement is not
	 * specified, except that 0 turns off the auto refresh.
	 */
	sdr_ref_val = ((LT7381_TREF * ltc->cclk_freq) /
		LT7381_ROW_SIZE) * FREQ_SCALING;
	lt7381_write_reg(par, LT7381_SDR_REF_0, sdr_ref_val & 0xff);
	lt7381_write_reg(par, LT7381_SDR_REF_1, sdr_ref_val >> 8);
	/* Execute  display  RAM  initialization */
	lt7381_write_reg(par, LT7381_SDRCR, LT7381_SDRCR_INITDONE);
	/* Wait till display RAM initializes or timeout */
	while(!(lt7381_read_status(par) & LT7381_STATUS_DRAM_RDY_FLAG)) {
		if (++retry_count >= SDRAM_TIMEOUT_RETRIES)
			return -EIO;
		msleep(SDRAM_TIMEOUT);
	}
	/* Linear mode with 16bit memory read and write access. Linear mode is
	 * limited up to 16bpp */
	lt7381_write_reg(par, LT7381_AW_COLOR, LT7381_AW_COLOR_LINEAR_MODE |
			LT7381_AW_COLOR_16BPP);
	lt7381_write_reg(par, LT7381_ICR, LT7381_ICR_IMG_BUFFER |
			LT7381_ICR_GRAPHIC_MODE);

	return 0;
}

static void lt7381_setup_pixel(struct fbtft_par *par, struct display_timing *dt)
{
	/* Main image width (MIW) */
	lt7381_write_reg(par, LT7381_MIW_0, dt->hactive.typ & 0xff);
	lt7381_write_reg(par, LT7381_MIW_1, dt->hactive.typ >> 8);
	/* Set pixel clock inversion */
	lt7381_write_reg(par, LT7381_DPCR, dt->flags &
			DISPLAY_FLAGS_PIXDATA_NEGEDGE ?
			LT7381_DPCR_PCLK_FALLING_EDGE :
			LT7381_DPCR_PCLK_RISING_EDGE);
}

static int lt7381_init_display(struct fbtft_par *par)
{
	int retry_count = 0;
	int ret = 0;
	struct device_node *np = par->info->device->of_node;
	struct display_timings *disp_timings;
	struct display_timing *dt;
	struct lt7381_ctrl *ltc;

	ltc = devm_kzalloc(par->info->device, sizeof(*ltc), GFP_KERNEL);
	if (!ltc)
		return -ENOMEM;
	/* extra entry can be used as long as the SPI is not 9-bits */
	par->extra = ltc;

	disp_timings = of_get_display_timings(np);
	if (!disp_timings) {
		dev_err(par->info->device, "failed to find display phandle\n");
		return -ENOENT;
	}

	par->fbtftops.reset(par);

	while (lt7381_read_status(par) & LT7381_STATUS_OM_INHIBIT_FLAG) {
		if (++retry_count >= STARTUP_TIMEOUT_RETRIES) {
			dev_err(par->info->device, "Could not read OK status\n");
			return -EIO;
		}
		msleep(STARTUP_TIMEOUT);
	}

	/* Use index 0, only one display defined at a time */
	dt = display_timings_get(disp_timings, 0);

	if ((ret = lt7381_setup_clocks(par, dt)) < 0)
	{
		dev_err(par->info->device, "Could not setup clocks\n");
		return ret;
	}
	lt7381_setup_timing(par, dt);
	lt7381_setup_line_polarity(par, dt);

	if ((ret = lt7381_setup_sdram(par)) < 0) {
		dev_err(par->info->device, "Could not setup SDRAM\n");
		return ret;
	}

	lt7381_setup_pixel(par, dt);
	lt7381_set_pwm_backlight(par);

	/* Turn display on */
	lt7381_update_reg(par, LT7381_DPCR, LT7381_DPCR_DISPLAY_ONOFF_MASK,
			LT7381_DPCR_DISPLAY_ON);

	/*
	 * The test-pattern property are used to turn on the test pattern. It is
	 * normally used when adding a new display to make it easier to debug
	 * the timing properties. The pattern remains despite what is written in
	 * the memory. This make it possible to only modify the DT without
	 * changing any thing in the kernel when adding a new display.
	 */
	if (of_find_property(np, "test-pattern", NULL))
		lt7381_update_reg(par, LT7381_DPCR,
				LT7381_DPCR_DISPLAY_TSTBAR_MASK,
				LT7381_DPCR_DISPLAY_TSTBAR_EN);

	dev_dbg(par->info->device, "init ok\n");

	return 0;
}

static void lt7381_write_reg8_bus8(struct fbtft_par *par, int len, ...)
{
	va_list args;
	int i, ret;
	u8 *buf = par->txbuf.buf;

	/* Prevent OOM */
	len = min(len, (int)par->txbuf.len);

	va_start(args, len);
	for (i = 0; i < len; i++)
		*buf++ = (u8)va_arg(args, unsigned int);
	va_end(args);

	fbtft_par_dbg_hex(DEBUG_WRITE_REGISTER, par,
		par->info->device, u8, par->txbuf.buf, len, "%s: ", __func__);

	ret = par->fbtftops.write(par, par->txbuf.buf, len);
	if (ret < 0) {
		dev_err(par->info->device,
				"%s failed and returned %d\n", __func__, ret);
	}
}

static int lt7381_write_vmem(struct fbtft_par *par, size_t offset, size_t len)
{
	u8 *vmem = (u8 *)(par->info->screen_buffer + offset);
	u8 *buf8 = par->txbuf.buf;
	int tx_len = 0;
	int tx_size;

	/* Set the display ram offset. It sets the mmap address of the display
	 * ram on the device. It auto increments with every data write cycle */
	lt7381_write_reg(par, LT7381_CURH_0, offset & 0xff);
	lt7381_write_reg(par, LT7381_CURH_1, offset >> 8);
	lt7381_write_reg(par, LT7381_CURV_0, offset >> 16);
	lt7381_write_reg(par, LT7381_CURV_1, offset >> 24);

	/*
	 * Set address register to Memory Port. Writing to this register maps
	 * the data to display RAM starting from the RAM offset, which was
	 * configured above. The offset auto increments with every write cycle.
	 * See NOTE1 in the top of this file for more info on the addressing
	 * modes.
	 */
	lt7381_write_addr(par, LT7381_MRWDP);

	/*
	 * This driver was tested with the MESON SPICC hw, which performs poorly
	 * in terms of throughput, because it can only output 16 words per burst
	 * followed by an interrupted delay as at the time of writing this
	 * comment. We can transfer more bytes per word if we increase the word
	 * size, thus higher throughput. It was found that the transfer fail if
	 * the word length exceeds 32, so we TX multiple bytes as 32 bit words,
	 * by swapping the byte order during the copy process and also changing
	 * the bit_per_word setting.
	 */
	while (tx_len < len) {
		bool change_byte_order = false;

		/*
		 * Set the direct address to DATA_WRITE, it will write the data
		 * to the register pointed by the AP, see NOTE1. The data will
		 * in turn then be mapped to the display ram as we write to the
		 * MRWDP register.
		 */
		buf8[0] = LT7381_DATA_WRITE;

		/* calculate the size of the transfer, it will also prevent OOM
		 */
		tx_size = min(len - tx_len, par->txbuf.len - 1);

/* The drive dependency was only added to improve performance, it is not
 * required for this driver to function properly*/
#ifdef CONFIG_SPI_MESON_SPICC
		/* We need 32 bytes or more to make it worth the effort */
		if (tx_size >= 32) {
			/* The size needs to be multiples of 32, re-adjust it.
			 * The minus one is for the DATA_WRITE byte. */
			tx_size = (tx_size / 32) * 32 - 1;
			/* We need to swap the byte order to be compatible with
			 * the SPI interface. We are sending multiple bytes as
			 * single words and SPI interface uses MSB first, we
			 * need swap it*/
			change_byte_order = true;
		}
#endif
		/* copy to offset 1, because buf8[0] = DATA_WRITE */
		memcpy(&buf8[1], &vmem[tx_len], tx_size);

#ifdef CONFIG_SPI_MESON_SPICC
		if (change_byte_order) {
			u32 *pSrc = (u32 *)buf8;
			__be32 *pDst = (__be32 *)buf8;
			int cpy_count;

			/* Swap the byte order and store it in the same buffer.
			 * The DATA_WRITE byte is included. */
			for (cpy_count = 0; cpy_count < (tx_size + 1) / sizeof(u32);
					cpy_count++)
				*pDst++ = cpu_to_be32p(pSrc++);

			/* Increase the bit per word to get a larger burst, 64
			 * bit tansfers do not work for some reason, but 32 bit
			 * does work */
			par->spi->bits_per_word = 32;
		}
#endif
		/* Write data with tx_size + DATA_WRITE byte */
		par->fbtftops.write(par, buf8, tx_size + 1);

		if (change_byte_order)
			/* change the bit_per_word to default. It was changed to
			 * boost the transfer. */
			par->spi->bits_per_word = 8;

		tx_len += tx_size;
	}

	return 0;
}

static int lt7381_backlight_chip_update_status(struct backlight_device *bd)
{
	struct fbtft_par *par = bl_get_data(bd);
	struct lt7381_ctrl *ltc = par->extra;
	int brightness = bd->props.brightness;

	fbtft_par_dbg(DEBUG_BACKLIGHT, par,
		"%s: brightness=%d, power=%d, fb_blank=%d\n",
		__func__, bd->props.brightness, bd->props.power,
		bd->props.fb_blank);

	if (bd->props.power != FB_BLANK_UNBLANK)
		brightness = MIN_BRIGHTNESS;

	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		brightness = MIN_BRIGHTNESS;

	lt7381_set_brightness(par, (u32)ltc->pwm_cnt, brightness);

	return 0;
}

static const struct backlight_ops lt7381_bl_ops = {
	.update_status = lt7381_backlight_chip_update_status,
};

static void lt7381_register_chip_backlight(struct fbtft_par *par)
{
	struct backlight_device *bd;
	struct backlight_properties bl_props = { 0, };
	struct lt7381_ctrl *ltc = par->extra;

	/* The backlight functionality is not registered if the PWM reload value
	 * is not specified in the DT. PWM is optional, so it is not necessarily
	 * an error. We already print a debug message in the init_display
	 * function */
	if (ltc->pwm_cnt == -EINVAL)
		return;

	bl_props.type = BACKLIGHT_RAW;
	bl_props.power = FB_BLANK_POWERDOWN;
	bl_props.max_brightness = ltc->max_brightness_perc;
	bl_props.brightness = ltc->def_brightness_perc;

	bd = backlight_device_register(dev_driver_string(par->info->device),
				par->info->device, par, &lt7381_bl_ops,
				&bl_props);
	if (IS_ERR(bd)) {
		dev_err(par->info->device,
			"cannot register backlight device (%ld)\n",
			PTR_ERR(bd));
		return;
	}
	par->info->bl_dev = bd;
}

/*
 * TODO: We don't use set_addr_win callback, because we are using a
 * linear buffer solution. However, chip does support block mode and it
 * would make sense to implement this in the future, but this is not
 * implemented in the meantime. The core driver use a default function
 * when we don't provide one. The default function tends to modify
 * register directly and we really don't want that.
 */
static void lt7381_fbtft_set_addr_win(struct fbtft_par *par, int xs, int ys, int xe,
			       int ye)
{

}

/* This function was copied from the core driver. Only the values and comments
 * are altered */
static void lt7381_reset(struct fbtft_par *par)
{
	/* Minus is explicitly set by the core driver if the GPIO is not found.
	 */
	if (desc_to_gpio(par->gpio.reset) == -1)
		return;
	fbtft_par_dbg(DEBUG_RESET, par, "%s()\n", __func__);
	gpio_set_value(desc_to_gpio(par->gpio.reset), 0);
	msleep(RESET_HOLD_TIME);
	gpio_set_value(desc_to_gpio(par->gpio.reset), 1);
	msleep(RESET_RELEASE_TIME);
}

static int verify_gpios(struct fbtft_par *par)
{
	if (par->gpio.reset < 0) {
		dev_err(par->info->device, "Missing 'reset' gpio. Aborting.\n");
		return -EINVAL;
	}
	return 0;
}

static struct fbtft_display display = {
	.txbuflen = TXBUFLEN,
	.fbtftops = {
		.init_display = lt7381_init_display,
		.write_register = lt7381_write_reg8_bus8,
		.write_vmem = lt7381_write_vmem,
		.verify_gpios = verify_gpios,
		.register_backlight = lt7381_register_chip_backlight,
		.unregister_backlight = fbtft_unregister_backlight,
		.reset = lt7381_reset,
		.set_addr_win = lt7381_fbtft_set_addr_win,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "levetop,lt7381", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:lt7381");
MODULE_ALIAS("platform:lt7381");

MODULE_DESCRIPTION("FB driver for the LT7381 LCD display bridge");
MODULE_AUTHOR("StreamUnlimited");
MODULE_LICENSE("GPL");
