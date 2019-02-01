#ifndef __DTS__IMX8MM_PAD_H__
#define __DTS__IMX8MM_PAD_H__

#define PAD_CTL_HYS		(1 << 7)		/* Enable Hysteresis */
#define PAD_CTL_PE		(1 << 8)		/* Enable Pull-Up/Down resistors */
#define PAD_CTL_PE_UP		(1 << 6 | PAD_CTL_PE)	/* Enable Pull-Up resistor */
#define PAD_CTL_PE_DOWN		(0 << 6 | PAD_CTL_PE)	/* Enable Pull-Down resistor */
#define PAD_CTL_ODE		(1 << 5)		/* Enable Open-Drain config */
#define PAD_CTL_FAST		(1 << 4)		/* Set Slew Rate ctrl to Fast */
#define PAD_CTL_SLOW		(0 << 4)		/* Set Slew Rate ctrl to Slow */
#define PAD_CTL_DSE1x		(0 << 1)		/* Set Drive Strength to 1x */
#define PAD_CTL_DSE2x		(2 << 1)		/* Set Drive Strength to 2x; This is correct, not a TYPO! */
#define PAD_CTL_DSE4x		(1 << 1)		/* Set Drive Strength to 4x; This is correct, not a TYPO! */
#define PAD_CTL_DSE6x		(3 << 1)		/* Set Drive Strength to 6x */
#define IOMUX_CONFIG_SION	(1 << 30)		/* Enable software input path of PAD */

#endif /* __DTS__IMX8MM_PAD_H__ */
