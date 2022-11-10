/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef __RTL8730E_SPEC_H__
#define __RTL8730E_SPEC_H__

#define	IMR_BCNDERR7_8730E BIT31 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR6_8730E BIT30 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR5_8730E BIT29 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR4_8730E BIT28 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR3_8730E BIT27 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR2_8730E BIT26 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR1_8730E BIT25 /* Beacon Queue DMA Error */
#define	IMR_BCNDERR0_8730E BIT24 /* Beacon Queue DMA Error */
#define	IMR_BCNDMAOK7_8730E BIT23 /* Beacon DMA OK Interrupt 7 */
#define	IMR_BCNDMAOK6_8730E BIT22 /* Beacon DMA OK Interrupt 6 */
#define	IMR_BCNDMAOK5_8730E BIT21 /* Beacon DMA OK Interrupt 5 */
#define	IMR_BCNDMAOK4_8730E BIT20 /* Beacon DMA OK Interrupt 4 */
#define	IMR_BCNDMAOK3_8730E BIT19 /* Beacon DMA OK Interrupt 3 */
#define	IMR_BCNDMAOK2_8730E BIT18 /* Beacon DMA OK Interrupt 2 */
#define	IMR_BCNDMAOK1_8730E BIT17 /* Beacon DMA OK Interrupt 1 */
#define	IMR_BCNDMAOK0_8730E BIT16 /* Beacon DMA OK Interrupt 0 */
#define	IMR_H7DOK_8730E BIT15 /* High Queue DMA OK Interrup 7 */
#define	IMR_H6DOK_8730E BIT14 /* High Queue DMA OK Interrup 6 */
#define	IMR_H5DOK_8730E BIT13 /* High Queue DMA OK Interrup 5 */
#define	IMR_H4DOK_8730E BIT12 /* High Queue DMA OK Interrup 4 */
#define	IMR_H3DOK_8730E BIT11 /* High Queue DMA OK Interrup 3 */
#define	IMR_H2DOK_8730E BIT10 /* High Queue DMA OK Interrup 2 */
#define	IMR_H1DOK_8730E BIT9 /* High Queue DMA OK Interrup 1 */
#define	IMR_H0DOK_8730E BIT8 /* High Queue DMA OK Interrup 1 */
#define	IMR_H2COK_8730E BIT7 /* H2C DMA OK Interrup */
#define	IMR_MGNTDOK_8730E BIT6 /* Management Queue DMA OK */
#define	IMR_BKDOK_8730E BIT5 /* AC_BK DMA OK */
#define	IMR_BEDOK_8730E BIT4 /* AC_BE DMA OK */
#define	IMR_VIDOK_8730E BIT3 /* AC_VI DMA OK */
#define	IMR_VODOK_8730E BIT2 /* AC_VO DMA OK */
#define	IMR_RDU_8730E BIT1 /* Rx Descriptor Unavailable */
#define	IMR_ROK_8730E BIT0 /* Receive DMA OK */

#ifdef CONFIG_AXI_HCI
#define IMR_RX_MASK (IMR_ROK_8730E | IMR_RDU_8730E | IMR_RXFOVW_8730E)
#define IMR_TX_MASK (IMR_VODOK_8730E | IMR_VIDOK_8730E | IMR_BEDOK_8730E | IMR_BKDOK_8730E | IMR_MGNTDOK_8730E | IMR_H0DOK_8730E)
#define RT_BCN_INT_MASKS (IMR_BCNDMAOK0_8730E | BIT_TXBCN1_OK_INT_8730E | BIT_TXBCN1_ERR_INT_8730E | IMR_BCNDERR0_8730E)
#define RT_AC_INT_MASKS (IMR_VIDOK_8730E | IMR_VODOK_8730E | IMR_BEDOK_8730E | IMR_BKDOK_8730E)
#endif /*CONFIG_AXI_HCI*/

#endif /* __RTL8730E_SPEC_H__ */
