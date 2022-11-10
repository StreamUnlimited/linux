/*
 * Realtek Semiconductor Corp.
 *
 * sheipa_hdmi.h:
 *
 * Copyright (C) 2006-2013 Jethro Hsu (jethro@realtek.com)
 */
#ifndef SHEIPAHDMI_H
#define SHEIPAHDMI_H

/* HDMI platform data */
struct sheipa_hdmi_pdata {
	u32		xres;
	u32		yres;
	u8		bpp;
	u8		mode;
};

#define SHIFT(field, reg)	(HDMI_##reg##_##field##_shift)
#define MASK(field, reg)	(HDMI_##reg##_##field##_mask)
#define SHIFTD(field, reg)	(VDMA_##reg##_##field##_shift)
#define MASKD(field, reg)	(VDMA_##reg##_##field##_mask)
#define VAL(field, reg, val) \
		(MASK(field, reg) & ((val) << SHIFT(field, reg)))
#define VALD(field, reg, val) \
		(MASKD(field, reg) & ((val) << SHIFTD(field, reg)))
/* HDMI registers map */

/* CPU command regitsters */
#define HDMI_INTEN				0x0
#define HDMI_INTEN_rxsenseint_shift		(3)
#define HDMI_INTEN_enriupdint_shift		(2)
#define HDMI_INTEN_enpjupdint_shift		(1)
#define HDMI_INTEN_write_data_shift		(0)
#define HDMI_INTEN_rxsenseint_mask		(0x00000008)
#define HDMI_INTEN_enriupdint_mask		(0x00000004)
#define HDMI_INTEN_enpjupdint_mask		(0x00000002)
#define HDMI_INTEN_write_data_mask		(0x00000001)

#define HDMI_INTENV				0x274
#define HDMI_INTENV_vendint_shift		(2)
#define HDMI_INTENV_vsyncint_shift		(1)
#define HDMI_INTENV_write_data_shift		(0)
#define HDMI_INTENV_vendint_mask		(0x00000004)
#define HDMI_INTENV_vsyncint_mask		(0x00000002)
#define HDMI_INTENV_write_data_mask		(0x00000001)

#define HDMI_AUDIO_ST				0x4
#define HDMI_AUDIO_ST_asfifo_empty_shift	(4)
#define HDMI_AUDIO_ST_asfifo_full_shift 	(3)
#define HDMI_AUDIO_ST_owctshw_shift     	(2)
#define HDMI_AUDIO_ST_asfifoow_shift    	(1)
#define HDMI_AUDIO_ST_write_data_shift  	(0)
#define HDMI_AUDIO_ST_asfifo_empty_mask 	(0x00000010)
#define HDMI_AUDIO_ST_asfifo_full_mask  	(0x00000008)
#define HDMI_AUDIO_ST_owctshw_mask      	(0x00000004)
#define HDMI_AUDIO_ST_asfifoow_mask     	(0x00000002)
#define HDMI_AUDIO_ST_write_data_mask   	(0x00000001)

#define HDMI_INTST				0x8
#define HDMI_INTST_rxupdated_shift		(2)
#define HDMI_INTST_riupdated_shift		(1)
#define HDMI_INTST_pjupdated_shift		(0)
#define HDMI_INTST_rxupdated_mask 		(0x00000004)
#define HDMI_INTST_riupdated_mask 		(0x00000002)
#define HDMI_INTST_pjupdated_mask 		(0x00000001)

#define HDMI_INTSTV				0x278
#define HDMI_INTSTV_vendupdated_shift		(1)
#define HDMI_INTSTV_vsyncupdated_shift		(0)
#define HDMI_INTSTV_vendupdated_mask		(0x00000002)
#define HDMI_INTSTV_vsyncupdated_mask		(0x00000001)

#define HDMI_PHY_STATUS				0x15C
#define HDMI_PHY_STATUS_wdout_shift		(1)
#define HDMI_PHY_STATUS_Rxstatus_shift		(0)
#define HDMI_PHY_STATUS_wdout_mask		(0x00000002)
#define HDMI_PHY_STATUS_Rxstatus_mask		(0x00000001)

#define HDMI_UPDATE_ST				0xC
#define HDMI_UPDATE_ST_write_en2_shift		(17)
#define HDMI_UPDATE_ST_rirate_shift		(9)
#define HDMI_UPDATE_ST_write_en1_shift		(8)
#define HDMI_UPDATE_ST_pjrate_shift		(0)
#define HDMI_UPDATE_ST_write_en2_mask		(0x00020000)
#define HDMI_UPDATE_ST_rirate_mask		(0x0001FE00)
#define HDMI_UPDATE_ST_write_en1_mask		(0x00000100)
#define HDMI_UPDATE_ST_pjrate_mask		(0x000000FF)

/* HDMI contorl registers */
#define HDMI_CHNL_SEL				0x20
#define HDMI_CHNL_SEL_sram_shut_down_en_shift	(11)
#define HDMI_CHNL_SEL_sram_shut_down_en_mask	(0x00000800)
#define HDMI_CHNL_SEL_sram_sleep_en_shift	(10)
#define HDMI_CHNL_SEL_sram_sleep_en_mask	(0x00000400)
#define HDMI_CHNL_SEL_video_sd_shift		(9)
#define HDMI_CHNL_SEL_video_sd_mask		(0x00000200)
#define HDMI_CHNL_SEL_Interlaced_vfmt_shift	(8)
#define HDMI_CHNL_SEL_Interlaced_vfmt_mask	(0x00000100)
#define HDMI_CHNL_SEL_3D_video_format_shift	(4)
#define HDMI_CHNL_SEL_3D_video_format_mask	(0x000000F0)
#define HDMI_CHNL_SEL_En_3D_shift		(3)
#define HDMI_CHNL_SEL_En_3D_mask		(0x00000008)
#define HDMI_CHNL_SEL_timing_gen_en_shift	(2)
#define HDMI_CHNL_SEL_timing_gen_en_mask	(0x00000004)
#define HDMI_CHNL_SEL_chl_sel_shift		(0)
#define HDMI_CHNL_SEL_chl_sel_mask		(0x00000003)

#define HDMI_CS_TRANS0				0x24
#define HDMI_CS_TRANS0_c1_shift			(0)
#define HDMI_CS_TRANS0_c1_mask			(0x0001FFFF)

#define HDMI_CS_TRANS1				0x28
#define HDMI_CS_TRANS1_c2_shift			(0)
#define HDMI_CS_TRANS1_c2_mask			(0x0001FFFF)

#define HDMI_CS_TRANS2				0x2C
#define HDMI_CS_TRANS2_c3_shift			(0)
#define HDMI_CS_TRANS2_c3_mask			(0x0001FFFF)

#define HDMI_CS_TRANS3                          0x200
#define HDMI_CS_TRANS3_c4_shift                 (0)
#define HDMI_CS_TRANS3_c4_mask                  (0x0001FFFF)

#define HDMI_CS_TRANS4				0x204
#define HDMI_CS_TRANS4_c5_shift			(0)
#define HDMI_CS_TRANS4_c5_mask 			(0x0001FFFF)

#define HDMI_CS_TRANS5				0x208
#define HDMI_CS_TRANS5_c6_shift			(0)
#define HDMI_CS_TRANS5_c6_mask			(0x0001FFFF)

#define HDMI_CS_TRANS6				0x20c
#define HDMI_CS_TRANS6_c7_shift			(0)
#define HDMI_CS_TRANS6_c7_mask			(0x0001FFFF)

#define HDMI_CS_TRANS7				0x210
#define HDMI_CS_TRANS7_c8_shift			(0)
#define HDMI_CS_TRANS7_c8_mask			(0x0001FFFF)

#define HDMI_CS_TRANS8				0x214
#define HDMI_CS_TRANS8_c9_shift			(0)
#define HDMI_CS_TRANS8_c9_mask			(0x0001FFFF)

#define HDMI_CS_TRANS9				0x218
#define HDMI_CS_TRANS9_k1_shift			(0)
#define HDMI_CS_TRANS9_k1_mask			(0x0000FFFF)

#define HDMI_CS_TRANS10				0x21C
#define HDMI_CS_TRANS10_k2_shift		(0)
#define HDMI_CS_TRANS10_k2_mask			(0x0000FFFF)

#define HDMI_CS_TRANS11				0x220
#define HDMI_CS_TRANS11_k3_shift		(0)
#define HDMI_CS_TRANS11_k3_mask			(0x0000FFFF)

#define HDMI_CS_TRANS12				0x224
#define HDMI_CS_TRANS12_write_en2_shift		(28)
#define HDMI_CS_TRANS12_write_en2_mask 		(0x10000000)
#define HDMI_CS_TRANS12_ygmax_shift		(16)
#define HDMI_CS_TRANS12_ygmax_mask		(0x0FFF0000)
#define HDMI_CS_TRANS12_write_en1_shift		(12)
#define HDMI_CS_TRANS12_write_en1_mask		(0x00001000)
#define HDMI_CS_TRANS12_ygmin_shift		(0)
#define HDMI_CS_TRANS12_ygmin_mask		(0x00000FFF)

#define HDMI_CS_TRANS13				0x228
#define HDMI_CS_TRANS13_write_en2_shift		(28)
#define HDMI_CS_TRANS13_write_en2_mask		(0x10000000)
#define HDMI_CS_TRANS13_cbbmax_shift		(16)
#define HDMI_CS_TRANS13_cbbmax_mask		(0x0FFF0000)
#define HDMI_CS_TRANS13_write_en1_shift		(12)
#define HDMI_CS_TRANS13_write_en1_mask		(0x00001000)
#define HDMI_CS_TRANS13_cbbmin_shift		(0)
#define HDMI_CS_TRANS13_cbbmin_mask		(0x00000FFF)

#define HDMI_CS_TRANS14				0x22C
#define HDMI_CS_TRANS14_write_en2_shift		(28)
#define HDMI_CS_TRANS14_write_en2_mask		(0x10000000)
#define HDMI_CS_TRANS14_crrmax_shift		(16)
#define HDMI_CS_TRANS14_crrmax_mask		(0x0FFF0000)
#define HDMI_CS_TRANS14_write_en1_shift		(12)
#define HDMI_CS_TRANS14_write_en1_mask		(0x00001000)
#define HDMI_CS_TRANS14_crrmin_shift		(0)
#define HDMI_CS_TRANS14_crrmin_mask		(0x00000FFF)

#define HDMI_SYNC_DLY				0x30
#define HDMI_SYNC_DLY_write_en4_shift		(11)
#define HDMI_SYNC_DLY_write_en4_mask		(0x00000800)
#define HDMI_SYNC_DLY_nor_v_shift		(9)
#define HDMI_SYNC_DLY_nor_v_mask		(0x00000600)
#define HDMI_SYNC_DLY_write_en3_shift		(8)
#define HDMI_SYNC_DLY_write_en3_mask		(0x00000100)
#define HDMI_SYNC_DLY_nor_h_shift		(6)
#define HDMI_SYNC_DLY_nor_h_mask		(0x000000C0)
#define HDMI_SYNC_DLY_write_en2_shift		(5)
#define HDMI_SYNC_DLY_write_en2_mask		(0x00000020)
#define HDMI_SYNC_DLY_disp_v_shift		(3)
#define HDMI_SYNC_DLY_disp_v_mask		(0x00000018)
#define HDMI_SYNC_DLY_write_en1_shift		(2)
#define HDMI_SYNC_DLY_write_en1_mask		(0x00000004)
#define HDMI_SYNC_DLY_disp_h_shift		(0)
#define HDMI_SYNC_DLY_disp_h_mask		(0x00000003)

#define HDMI_CR					0x34
#define HDMI_CR_write_en3_shift			(5)
#define HDMI_CR_write_en3_mask			(0x00000020)
#define HDMI_CR_tmds_encen_shift		(4)
#define HDMI_CR_tmds_encen_mask			(0x00000010)
#define HDMI_CR_write_en2_shift			(3)
#define HDMI_CR_write_en2_mask			(0x00000008)
#define HDMI_CR_enablehdcp_shift		(2)
#define HDMI_CR_enablehdcp_mask			(0x00000004)
#define HDMI_CR_write_en1_shift			(1)
#define HDMI_CR_write_en1_mask			(0x00000002)
#define HDMI_CR_enablehdmi_shift		(0)
#define HDMI_CR_enablehdmi_mask			(0x00000001)

#define HDMI_DPC				0x154
#define HDMI_DPC_write_en9_shift		(30)
#define HDMI_DPC_write_en9_mask			(0x40000000)
#define HDMI_DPC_dp_vfch_num_shift		(28)
#define HDMI_DPC_dp_vfch_num_mask		(0x30000000)
#define HDMI_DPC_write_en8_shift		(25)
#define HDMI_DPC_write_en8_mask			(0x02000000)
#define HDMI_DPC_fp_swen_shift			(24)
#define HDMI_DPC_fp_swen_mask			(0x01000000)
#define HDMI_DPC_write_en7_shift		(23)
#define HDMI_DPC_write_en7_mask			(0x00800000)
#define HDMI_DPC_fp_shift			(20)
#define HDMI_DPC_fp_mask			(0x00700000)
#define HDMI_DPC_write_en6_shift		(19)
#define HDMI_DPC_write_en6_mask			(0x00080000)
#define HDMI_DPC_pp_swen_shift			(18)
#define HDMI_DPC_pp_swen_mask			(0x00040000)
#define HDMI_DPC_write_en4_shift		(15)
#define HDMI_DPC_write_en4_mask			(0x00008000)
#define HDMI_DPC_dp_swen_shift			(14)
#define HDMI_DPC_dp_swen_mask			(0x00004000)
#define HDMI_DPC_write_en3_shift		(13)
#define HDMI_DPC_write_en3_mask			(0x00002000)
#define HDMI_DPC_default_phase_shift		(12)
#define HDMI_DPC_default_phase_mask		(0x00001000)
#define HDMI_DPC_write_en2_shift		(8)
#define HDMI_DPC_write_en2_mask			(0x00000100)
#define HDMI_DPC_color_depth_shift		(4)
#define HDMI_DPC_color_depth_mask		(0x000000F0)
#define HDMI_DPC_write_en1_shift		(1)
#define HDMI_DPC_write_en1_mask			(0x00000002)
#define HDMI_DPC_dpc_enable_shift		(0)
#define HDMI_DPC_dpc_enable_mask		(0x00000001)

#define HDMI_DPC1				0x158
#define HDMI_DPC1_write_en2_shift		(9)
#define HDMI_DPC1_write_en2_mask		(0x00000200)
#define HDMI_DPC1_dp_vfch_shift			(8)
#define HDMI_DPC1_dp_vfch_mask			(0x00000100)
#define HDMI_DPC1_write_en1_shift		(4)
#define HDMI_DPC1_write_en1_mask		(0x00000010)
#define HDMI_DPC1_pp_shift			(0)
#define HDMI_DPC1_pp_mask			(0x0000000F)

/* TMDS encoder registers */
#define HDMI_TMDS_EODR0				0x38
#define HDMI_TMDS_EODR0_tmds_ch2_out_shift	(20)
#define HDMI_TMDS_EODR0_tmds_ch2_out_mask	(0x3FF00000)
#define HDMI_TMDS_EODR0_tmds_ch1_out_shift	(10)
#define HDMI_TMDS_EODR0_tmds_ch1_out_mask	(0x000FFC00)
#define HDMI_TMDS_EODR0_tmds_ch0_out_shift	(0)
#define HDMI_TMDS_EODR0_tmds_ch0_out_mask	(0x000003FF)

#define HDMI_TMDS_EODR1				0x3C
#define HDMI_TMDS_EODR1_default_phase_st_shift	(12)
#define HDMI_TMDS_EODR1_default_phase_st_mask	(0x00001000)
#define HDMI_TMDS_EODR1_tmds_clk_out_shift	(0)
#define HDMI_TMDS_EODR1_tmds_clk_out_mask	(0x000003FF)

#define HDMI_TMDS_SCR1				0x40
#define HDMI_TMDS_SCR1_ip_shift			(28)
#define HDMI_TMDS_SCR1_ip_mask			(0xF0000000)
#define HDMI_TMDS_SCR1_rs_shift			(25)
#define HDMI_TMDS_SCR1_rs_mask			(0x0E000000)
#define HDMI_TMDS_SCR1_cs_shift			(23)
#define HDMI_TMDS_SCR1_cs_mask			(0x01800000)
#define HDMI_TMDS_SCR1_cp_shift			(22)
#define HDMI_TMDS_SCR1_cp_mask			(0x00400000)
#define HDMI_TMDS_SCR1_m_shift			(20)
#define HDMI_TMDS_SCR1_m_mask			(0x00300000)
#define HDMI_TMDS_SCR1_wdrst_shift		(19)
#define HDMI_TMDS_SCR1_wdrst_mask		(0x00080000)
#define HDMI_TMDS_SCR1_wdset_shift		(18)
#define HDMI_TMDS_SCR1_wdset_mask		(0x00040000)
#define HDMI_TMDS_SCR1_rstb_shift		(17)
#define HDMI_TMDS_SCR1_rstb_mask		(0x00020000)
#define HDMI_TMDS_SCR1_test_en_shift		(16)
#define HDMI_TMDS_SCR1_test_en_mask		(0x00010000)
#define HDMI_TMDS_SCR1_powck_shift		(15)
#define HDMI_TMDS_SCR1_powck_mask		(0x00008000)
#define HDMI_TMDS_SCR1_powa_shift		(14)
#define HDMI_TMDS_SCR1_powa_mask		(0x00004000)
#define HDMI_TMDS_SCR1_powb_shift		(13)
#define HDMI_TMDS_SCR1_powb_mask		(0x00002000)
#define HDMI_TMDS_SCR1_powc_shift		(12)
#define HDMI_TMDS_SCR1_powc_mask		(0x00001000)
#define HDMI_TMDS_SCR1_emck_shift		(11)
#define HDMI_TMDS_SCR1_emck_mask		(0x00000800)
#define HDMI_TMDS_SCR1_ema_shift		(10)
#define HDMI_TMDS_SCR1_ema_mask			(0x00000400)
#define HDMI_TMDS_SCR1_emb_shift		(9)
#define HDMI_TMDS_SCR1_emb_mask			(0x00000200)
#define HDMI_TMDS_SCR1_emc_shift		(8)
#define HDMI_TMDS_SCR1_emc_mask			(0x00000100)
#define HDMI_TMDS_SCR1_ibrta_shift		(7)
#define HDMI_TMDS_SCR1_ibrta_mask		(0x00000080)
#define HDMI_TMDS_SCR1_ibrtb_shift		(6)
#define HDMI_TMDS_SCR1_ibrtb_mask		(0x00000040)
#define HDMI_TMDS_SCR1_ibrtc_shift		(5)
#define HDMI_TMDS_SCR1_ibrtc_mask		(0x00000020)
#define HDMI_TMDS_SCR1_Ibrtck_shift		(4)
#define HDMI_TMDS_SCR1_Ibrtck_mask		(0x00000010)
#define HDMI_TMDS_SCR1_reglpf_shift		(3)
#define HDMI_TMDS_SCR1_reglpf_mask		(0x00000008)
#define HDMI_TMDS_SCR1_powpll_shift		(2)
#define HDMI_TMDS_SCR1_powpll_mask		(0x00000004)
#define HDMI_TMDS_SCR1_cksel_shift		(0)
#define HDMI_TMDS_SCR1_cksel_mask		(0x00000003)

#define HDMI_TMDS_SCR2				0x44
#define HDMI_TMDS_SCR2_sel_shift		(30)
#define HDMI_TMDS_SCR2_sel_mask			(0xC0000000)
#define HDMI_TMDS_SCR2_bias_shift		(29)
#define HDMI_TMDS_SCR2_bias_mask		(0x20000000)
#define HDMI_TMDS_SCR2_vsel_shift		(28)
#define HDMI_TMDS_SCR2_vsel_mask		(0x10000000)
#define HDMI_TMDS_SCR2_powlv_shift		(27)
#define HDMI_TMDS_SCR2_powlv_mask		(0x08000000)
#define HDMI_TMDS_SCR2_lv_shift			(24)
#define HDMI_TMDS_SCR2_lv_mask			(0x07000000)
#define HDMI_TMDS_SCR2_iemck_shift		(20)
#define HDMI_TMDS_SCR2_iemck_mask		(0x00F00000)
#define HDMI_TMDS_SCR2_iema_shift		(16)
#define HDMI_TMDS_SCR2_iema_mask		(0x000F0000)
#define HDMI_TMDS_SCR2_iemb_shift		(12)
#define HDMI_TMDS_SCR2_iemb_mask		(0x0000F000)
#define HDMI_TMDS_SCR2_iemc_shift		(8)
#define HDMI_TMDS_SCR2_iemc_mask		(0x00000F00)
#define HDMI_TMDS_SCR2_rtonck_shift		(6)
#define HDMI_TMDS_SCR2_rtonck_mask		(0x000000C0)
#define HDMI_TMDS_SCR2_rtona_shift		(4)
#define HDMI_TMDS_SCR2_rtona_mask		(0x00000030)
#define HDMI_TMDS_SCR2_rtonb_shift		(2)
#define HDMI_TMDS_SCR2_rtonb_mask		(0x0000000C)
#define HDMI_TMDS_SCR2_rtonc_shift		(0)
#define HDMI_TMDS_SCR2_rtonc_mask		(0x00000003)

#define HDMI_TMDS_SCR3				0x230
#define HDMI_TMDS_SCR3_pdsrca_shift		(30)
#define HDMI_TMDS_SCR3_pdsrca_mask		(0xC0000000)
#define HDMI_TMDS_SCR3_pdsrcb_shift		(28)
#define HDMI_TMDS_SCR3_pdsrcb_mask		(0x30000000)
#define HDMI_TMDS_SCR3_pdsrcc_shift		(26)
#define HDMI_TMDS_SCR3_pdsrcc_mask		(0x0C000000)
#define HDMI_TMDS_SCR3_pdsrcck_shift		(24)
#define HDMI_TMDS_SCR3_pdsrcck_mask		(0x03000000)
#define HDMI_TMDS_SCR3_srca_shift		(22)
#define HDMI_TMDS_SCR3_srca_mask		(0x00C00000)
#define HDMI_TMDS_SCR3_srcb_shift		(20)
#define HDMI_TMDS_SCR3_srcb_mask		(0x00300000)
#define HDMI_TMDS_SCR3_srcc_shift		(18)
#define HDMI_TMDS_SCR3_srcc_mask		(0x000C0000)
#define HDMI_TMDS_SCR3_srcck_shift		(16)
#define HDMI_TMDS_SCR3_srcck_mask		(0x00030000)
#define HDMI_TMDS_SCR3_pdida_shift		(15)
#define HDMI_TMDS_SCR3_pdida_mask		(0x00008000)
#define HDMI_TMDS_SCR3_idrvck_shift		(12)
#define HDMI_TMDS_SCR3_idrvck_mask		(0x00007000)
#define HDMI_TMDS_SCR3_pdidb_shift		(11)
#define HDMI_TMDS_SCR3_pdidb_mask		(0x00000800)
#define HDMI_TMDS_SCR3_idrva_shift		(8)
#define HDMI_TMDS_SCR3_idrva_mask		(0x00000700)
#define HDMI_TMDS_SCR3_pdidc_shift		(7)
#define HDMI_TMDS_SCR3_pdidc_mask		(0x00000080)
#define HDMI_TMDS_SCR3_idrvb_shift		(4)
#define HDMI_TMDS_SCR3_idrvb_mask		(0x00000070)
#define HDMI_TMDS_SCR3_pdidck_shift		(3)
#define HDMI_TMDS_SCR3_pdidck_mask		(0x00000008)
#define HDMI_TMDS_SCR3_idrvc_shift		(0)
#define HDMI_TMDS_SCR3_idrvc_mask		(0x00000007)

#define HDMI_TMDS_STR0				0x48
#define HDMI_TMDS_STR0_tmds1ten_shift		(31)
#define HDMI_TMDS_STR0_tmds1ten_mask		(0x80000000)
#define HDMI_TMDS_STR0_td1src_shift		(30)
#define HDMI_TMDS_STR0_td1src_mask		(0x40000000)
#define HDMI_TMDS_STR0_tmds1td_shift		(16)
#define HDMI_TMDS_STR0_tmds1td_mask		(0x03FF0000)
#define HDMI_TMDS_STR0_tmds0ten_shift		(15)
#define HDMI_TMDS_STR0_tmds0ten_mask		(0x00008000)
#define HDMI_TMDS_STR0_td0src_shift		(14)
#define HDMI_TMDS_STR0_td0src_mask		(0x00004000)
#define HDMI_TMDS_STR0_tmds0td_shift		(0)
#define HDMI_TMDS_STR0_tmds0td_mask		(0x000003FF)

#define HDMI_TMDS_STR1				0x4C
#define HDMI_TMDS_STR1_tmdscten_shift		(31)
#define HDMI_TMDS_STR1_tmdscten_mask		(0x80000000)
#define HDMI_TMDS_STR1_tdcsrc_shift		(30)
#define HDMI_TMDS_STR1_tdcsrc_mask		(0x40000000)
#define HDMI_TMDS_STR1_tmdsctd_shift		(16)
#define HDMI_TMDS_STR1_tmdsctd_mask		(0x03FF0000)
#define HDMI_TMDS_STR1_tmds2ten_shift		(15)
#define HDMI_TMDS_STR1_tmds2ten_mask		(0x00008000)
#define HDMI_TMDS_STR1_td2src_shift		(14)
#define HDMI_TMDS_STR1_td2src_mask		(0x00004000)
#define HDMI_TMDS_STR1_tmds2td_shift		(0)
#define HDMI_TMDS_STR1_tmds2td_mask		(0x000003FF)

#define HDMI_ASPCR1				0x54
#define HDMI_ASPCR1_write_en8_shift		(28)
#define HDMI_ASPCR1_write_en8_mask		(0x10000000)
#define HDMI_ASPCR1_ycbcr422_8bits_shift	(27)
#define HDMI_ASPCR1_ycbcr422_8bits_mask		(0x08000000
#define HDMI_ASPCR1_write_en7_shift		(26)
#define HDMI_ASPCR1_write_en7_mask		(0x04000000)
#define HDMI_ASPCR1_cts_pkt_hp_shift		(25)
#define HDMI_ASPCR1_cts_pkt_hp_mask		(0x02000000)
#define HDMI_ASPCR1_write_en6_shift		(24)
#define HDMI_ASPCR1_write_en6_mask		(0x01000000)
#define HDMI_ASPCR1_Audioheader0_shift		(16)
#define HDMI_ASPCR1_Audioheader0_mask		(0x00FF0000)
#define HDMI_ASPCR1_write_en5_shift		(15)
#define HDMI_ASPCR1_write_en5_mask		(0x00008000)
#define HDMI_ASPCR1_samplepresent_shift		(11)
#define HDMI_ASPCR1_samplepresent_mask		(0x00007800)
#define HDMI_ASPCR1_write_en4_shift		(10)
#define HDMI_ASPCR1_write_en4_mask		(0x00000400)
#define HDMI_ASPCR1_forcesamplepresent_shift	(9)
#define HDMI_ASPCR1_forcesamplepresent_mask	(0x00000200)
#define HDMI_ASPCR1_write_en3_shift		(8)
#define HDMI_ASPCR1_write_en3_mask		(0x00000100)
#define HDMI_ASPCR1_b_frame_shift		(4)
#define HDMI_ASPCR1_b_frame_mask		(0x000000F0)
#define HDMI_ASPCR1_write_en2_shift		(3)
#define HDMI_ASPCR1_write_en2_mask		(0x00000008)
#define HDMI_ASPCR1_forceb_frame_shift		(2)
#define HDMI_ASPCR1_forceb_frame_mask		(0x00000004)
#define HDMI_ASPCR1_write_en1_shift		(1)
#define HDMI_ASPCR1_write_en1_mask		(0x00000002)
#define HDMI_ASPCR1_audiolayout_shift		(0)
#define HDMI_ASPCR1_audiolayout_mask		(0x00000001)

#define HDMI_ACACR				0x58
#define HDMI_ACACR_write_en4_shift		(15)
#define HDMI_ACACR_write_en4_mask		(0x00008000)
#define HDMI_ACACR_hdmi_pk3map_shift		(12)
#define HDMI_ACACR_hdmi_pk3map_mask		(0x00007000)
#define HDMI_ACACR_write_en3_shift		(11)
#define HDMI_ACACR_write_en3_mask		(0x00000800)
#define HDMI_ACACR_hdmi_pk2map_shift		(8)
#define HDMI_ACACR_hdmi_pk2map_mask		(0x00000700)
#define HDMI_ACACR_write_en2_shift		(7)
#define HDMI_ACACR_write_en2_mask		(0x00000080)
#define HDMI_ACACR_hdmi_pk1map_shift		(4)
#define HDMI_ACACR_hdmi_pk1map_mask		(0x00000070)
#define HDMI_ACACR_write_en1_shift		(3)
#define HDMI_ACACR_write_en1_mask		(0x00000008)
#define HDMI_ACACR_hdmi_pk0map_shift		(0)
#define HDMI_ACACR_hdmi_pk0map_mask		(0x00000007)

#define HDMI_ACRPCR				0x6c
#define HDMI_ACRPCR_write_en6_shift		(30)
#define HDMI_ACRPCR_write_en6_mask		(0x40000000)
#define HDMI_ACRPCR_Audio_info_hp_shift		(29)
#define HDMI_ACRPCR_Audio_info_hp_mask		(0x20000000)
#define HDMI_ACRPCR_write_en5_shift		(28)
#define HDMI_ACRPCR_write_en5_mask		(0x10000000)
#define HDMI_ACRPCR_ac_hw_ctrl_en_shift		(27)
#define HDMI_ACRPCR_ac_hw_ctrl_en_mask		(0x08000000)
#define HDMI_ACRPCR_write_en4_shift		(26)
#define HDMI_ACRPCR_write_en4_mask		(0x04000000)
#define HDMI_ACRPCR_fs_div4_shift		(25)
#define HDMI_ACRPCR_fs_div4_mask		(0x02000000)
#define HDMI_ACRPCR_write_en3_shift		(24)
#define HDMI_ACRPCR_write_en3_mask		(0x01000000)
#define HDMI_ACRPCR_disablecrp_shift		(23)
#define HDMI_ACRPCR_disablecrp_mask		(0x00800000)
#define HDMI_ACRPCR_write_en2_shift		(22)
#define HDMI_ACRPCR_write_en2_mask		(0x00400000)
#define HDMI_ACRPCR_cts_source_shift		(21)
#define HDMI_ACRPCR_cts_source_mask		(0x00200000)
#define HDMI_ACRPCR_write_en1_shift		(20)
#define HDMI_ACRPCR_write_en1_mask		(0x00100000)
#define HDMI_ACRPCR_n_value_shift		(0)
#define HDMI_ACRPCR_n_value_mask		(0x000FFFFF)

#define HDMI_ACRPCTSR				0x70
#define HDMI_ACRPCTSR_ctssw_shift		(0)
#define HDMI_ACRPCTSR_ctssw_mask		(0x000FFFFF)

#define HDMI_ACRPPR				0x74
#define HDMI_ACRPPR_ctshw_shift			(0)
#define HDMI_ACRPPR_ctshw_mask			(0x000FFFFF)

#define HDMI_GCPCR				0x78
#define HDMI_GCPCR_enablegcp_shift		(3)
#define HDMI_GCPCR_enablegcp_mask		(0x00000008)
#define HDMI_GCPCR_gcp_clearavmute_shift	(2)
#define HDMI_GCPCR_gcp_clearavmute_mask		(0x00000004)
#define HDMI_GCPCR_gcp_setavmute_shift		(1)
#define HDMI_GCPCR_gcp_setavmute_mask		(0x00000002)
#define HDMI_GCPCR_write_data_shift		(0)
#define HDMI_GCPCR_write_data_mask		(0x00000001)

#define HDMI_ORP6PH				0x7C
#define HDMI_ORP6PH_once_pkt_shift		(0)
#define HDMI_ORP6PH_once_pkt_mask		(0xFFFFFFFF)

#define HDMI_ORP6W_0					0x80
#define HDMI_ORP6W_1					0x84
#define HDMI_ORP6W_2					0x88
#define HDMI_ORP6W_3					0x8C
#define HDMI_ORP6W_4					0x90
#define HDMI_ORP6W_5					0x94
#define HDMI_ORP6W_6					0x98
#define HDMI_ORP6W_7					0x9C
#define HDMI_ORP6W_0_once_pkt_shift			(0)
#define HDMI_ORP6W_1_once_pkt_shift			(0)
#define HDMI_ORP6W_2_once_pkt_shift			(0)
#define HDMI_ORP6W_3_once_pkt_shift			(0)
#define HDMI_ORP6W_4_once_pkt_shift			(0)
#define HDMI_ORP6W_5_once_pkt_shift			(0)
#define HDMI_ORP6W_6_once_pkt_shift			(0)
#define HDMI_ORP6W_7_once_pkt_shift			(0)
#define HDMI_ORP6W_0_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_1_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_2_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_3_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_4_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_5_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_6_once_pkt_mask			(0xFFFFFFFF)
#define HDMI_ORP6W_7_once_pkt_mask			(0xFFFFFFFF)

#define HDMI_RPCR				0xA0
#define HDMI_RPCR_write_en6_shift		(29)
#define HDMI_RPCR_write_en6_mask		(0x20000000)
#define HDMI_RPCR_prp5period_shift		(25)
#define HDMI_RPCR_prp5period_mask		(0x1E000000)
#define HDMI_RPCR_write_en5_shift		(24)
#define HDMI_RPCR_write_en5_mask		(0x01000000)
#define HDMI_RPCR_prp4period_shift		(20)
#define HDMI_RPCR_prp4period_mask		(0x00F00000)
#define HDMI_RPCR_write_en4_shift		(19)
#define HDMI_RPCR_write_en4_mask		(0x00080000)
#define HDMI_RPCR_prp3period_shift		(15)
#define HDMI_RPCR_prp3period_mask		(0x00078000)
#define HDMI_RPCR_write_en3_shift		(14)
#define HDMI_RPCR_write_en3_mask		(0x00004000)
#define HDMI_RPCR_prp2period_shift		(10)
#define HDMI_RPCR_prp2period_mask		(0x00003C00)
#define HDMI_RPCR_write_en2_shift		(9)
#define HDMI_RPCR_write_en2_mask		(0x00000200)
#define HDMI_RPCR_prp1period_shift		(5)
#define HDMI_RPCR_prp1period_mask		(0x000001E0)
#define HDMI_RPCR_write_en1_shift		(4)
#define HDMI_RPCR_write_en1_mask		(0x00000010)
#define HDMI_RPCR_prp0period_shift		(0)
#define HDMI_RPCR_prp0period_mask		(0x0000000F)

#define HDMI_RPEN				0xA4
#define HDMI_RPEN_enprpkt5_shift		(6)
#define HDMI_RPEN_enprpkt5_mask			(0x00000040)
#define HDMI_RPEN_enprpkt4_shift		(5)
#define HDMI_RPEN_enprpkt4_mask			(0x00000020)
#define HDMI_RPEN_enprpkt3_shift		(4)
#define HDMI_RPEN_enprpkt3_mask			(0x00000010)
#define HDMI_RPEN_enprpkt2_shift		(3)
#define HDMI_RPEN_enprpkt2_mask			(0x00000008)
#define HDMI_RPEN_enprpkt1_shift		(2)
#define HDMI_RPEN_enprpkt1_mask			(0x00000004)
#define HDMI_RPEN_enprpkt0_shift		(1)
#define HDMI_RPEN_enprpkt0_mask			(0x00000002)
#define HDMI_RPEN_write_data_shift		(0)
#define HDMI_RPEN_write_data_mask		(0x00000001)

#define HDMI_RPRBDR				0xA8
#define HDMI_RPRBDR_isramrbd_shift		(0)
#define HDMI_RPRBDR_isramrbd_mask		(0x0003FFFF)

#define HDMI_OPCR				0xB0
#define HDMI_OPCR_write_en4_shift		(14)
#define HDMI_OPCR_write_en4_mask		(0x00004000)
#define HDMI_OPCR_enrbpktsram_shift		(13)
#define HDMI_OPCR_enrbpktsram_mask		(0x00002000)
#define HDMI_OPCR_write_en3_shift		(12)
#define HDMI_OPCR_write_en3_mask		(0x00001000)
#define HDMI_OPCR_wrdes_shift			(11)
#define HDMI_OPCR_wrdes_mask			(0x00000800)
#define HDMI_OPCR_write_en2_shift		(10)
#define HDMI_OPCR_write_en2_mask		(0x00000400)
#define HDMI_OPCR_rprwcmd_shift			(9)
#define HDMI_OPCR_rprwcmd_mask			(0x00000200)
#define HDMI_OPCR_write_en1_shift		(8)
#define HDMI_OPCR_write_en1_mask		(0x00000100)
#define HDMI_OPCR_rpadd_shift			(0)
#define HDMI_OPCR_rpadd_mask			(0x000000FF)

#define HDMI_DIPCCR				0xB4
#define HDMI_DIPCCR_write_en2_shift		(11)
#define HDMI_DIPCCR_write_en2_mask		(0x00000800)
#define HDMI_DIPCCR_vbidipcnt_shift		(6)
#define HDMI_DIPCCR_vbidipcnt_mask		(0x000007C0)
#define HDMI_DIPCCR_write_en1_shift		(5)
#define HDMI_DIPCCR_write_en1_mask		(0x00000020)
#define HDMI_DIPCCR_hbidipcnt_shift		(0)
#define HDMI_DIPCCR_hbidipcnt_mask		(0x0000001F)

/* Scheduler register */
#define HDMI_SCHCR				0xB8
#define HDMI_SCHCR_write_en12_shift		(28)
#define HDMI_SCHCR_write_en12_mask		(0x10000000)
#define HDMI_SCHCR_color_transform_en_shift	(27)
#define HDMI_SCHCR_color_transform_en_mask	(0x08000000)
#define HDMI_SCHCR_write_en11_shift		(26)
#define HDMI_SCHCR_write_en11_mask		(0x04000000)
#define HDMI_SCHCR_ycbcr422_algo_shift		(25)
#define HDMI_SCHCR_ycbcr422_algo_mask		(0x02000000)
#define HDMI_SCHCR_write_en10_shift		(24)
#define HDMI_SCHCR_write_en10_mask		(0x01000000)
#define HDMI_SCHCR_422_pixel_repeat_shift	(23)
#define HDMI_SCHCR_422_pixel_repeat_mask	(0x00800000)
#define HDMI_SCHCR_write_en9_shift		(22)
#define HDMI_SCHCR_write_en9_mask		(0x00400000)
#define HDMI_SCHCR_vsyncpolin_shift		(21)
#define HDMI_SCHCR_vsyncpolin_mask		(0x00200000)
#define HDMI_SCHCR_write_en8_shift		(20)
#define HDMI_SCHCR_write_en8_mask		(0x00100000)
#define HDMI_SCHCR_hsyncpolin_shift		(19)
#define HDMI_SCHCR_hsyncpolin_mask		(0x00080000)
#define HDMI_SCHCR_write_en7_shift		(18)
#define HDMI_SCHCR_write_en7_mask		(0x00040000)
#define HDMI_SCHCR_vsyncpolinv_shift		(17)
#define HDMI_SCHCR_vsyncpolinv_mask		(0x00020000)
#define HDMI_SCHCR_write_en6_shift		(16)
#define HDMI_SCHCR_write_en6_mask		(0x00010000)
#define HDMI_SCHCR_hsyncpolinv_shift		(15)
#define HDMI_SCHCR_hsyncpolinv_mask		(0x00008000)
#define HDMI_SCHCR_write_en5_shift		(14)
#define HDMI_SCHCR_write_en5_mask		(0x00004000)
#define HDMI_SCHCR_pixelencycbcr422_shift	(13)
#define HDMI_SCHCR_pixelencycbcr422_mask	(0x00002000)
#define HDMI_SCHCR_write_en4_shift		(12)
#define HDMI_SCHCR_write_en4_mask		(0x00001000)
#define HDMI_SCHCR_hdmi_videoxs_shift		(7)
#define HDMI_SCHCR_hdmi_videoxs_mask		(0x00000F80)
#define HDMI_SCHCR_write_en3_shift		(6)
#define HDMI_SCHCR_write_en3_mask		(0x00000040)
#define HDMI_SCHCR_pixelencfmt_shift		(4)
#define HDMI_SCHCR_pixelencfmt_mask		(0x00000030)
#define HDMI_SCHCR_write_en2_shift		(3)
#define HDMI_SCHCR_write_en2_mask		(0x00000008)
#define HDMI_SCHCR_pixelrepeat_shift		(2)
#define HDMI_SCHCR_pixelrepeat_mask		(0x00000004)
#define HDMI_SCHCR_write_en1_shift		(1)
#define HDMI_SCHCR_write_en1_mask		(0x00000002)
#define HDMI_SCHCR_hdmi_modesel_shift		(0)
#define HDMI_SCHCR_hdmi_modesel_mask		(0x00000001)

#define HDMI_ICR				0xBC
#define HDMI_ICR_write_en3_shift		(28)
#define HDMI_ICR_write_en3_mask			(0x10000000)
#define HDMI_ICR_enaudio_shift			(27)
#define HDMI_ICR_enaudio_mask			(0x08000000)
#define HDMI_ICR_write_en2_shift		(26)
#define HDMI_ICR_write_en2_mask			(0x04000000)
#define HDMI_ICR_envitd_shift			(25)
#define HDMI_ICR_envitd_mask			(0x02000000)
#define HDMI_ICR_write_en1_shift		(24)
#define HDMI_ICR_write_en1_mask			(0x01000000)
#define HDMI_ICR_vitd_shift			(0)
#define HDMI_ICR_vitd_mask			(0x00FFFFFF)

#define HDMI_ICR1				0x150
#define HDMI_ICR1_write_en1_shift		(12)
#define HDMI_ICR1_write_en1_mask		(0x00001000)
#define HDMI_ICR1_vitd_shift			(0)
#define HDMI_ICR1_vitd_mask			(0x00000FFF)

#define HDMI_CRCCR				0xC0
#define HDMI_CRCCR_write_en2_shift		(4)
#define HDMI_CRCCR_write_en2_mask		(0x00000010)
#define HDMI_CRCCR_crcmode_shift		(3)
#define HDMI_CRCCR_crcmode_mask			(0x00000008)
#define HDMI_CRCCR_write_en1_shift		(2)
#define HDMI_CRCCR_write_en1_mask		(0x00000004)
#define HDMI_CRCCR_crcchsel_shift		(0)
#define HDMI_CRCCR_crcchsel_mask		(0x00000003)

#define HDMI_CRCDOR				0xC4
#define HDMI_CRCDOR_crcvalue_shift		(0)
#define HDMI_CRCDOR_crcvalue_mask		(0xFFFFFFFF)

/* HDCP Engine registers */

#define HDMI_HDCP_KOWR					0xF0
#define HDMI_HDCP_KOWR_hdcprekeykeepoutwin_shift	(0)
#define HDMI_HDCP_KOWR_hdcprekeykeepoutwin_mask 	(0x000000FF)

#define HDMI_HDCP_KOWRSE				0xF4
#define HDMI_HDCP_KOWRSE_write_en2_shift		(25)
#define HDMI_HDCP_KOWRSE_write_en2_mask			(0x02000000)
#define HDMI_HDCP_KOWRSE_hdcpverkeepoutwinend_shift	(13)
#define HDMI_HDCP_KOWRSE_hdcpverkeepoutwinend_mask	(0x01FFE000)
#define HDMI_HDCP_KOWRSE_write_en1_shift		(12)
#define HDMI_HDCP_KOWRSE_write_en1_mask			(0x00001000)
#define HDMI_HDCP_KOWRSE_hdcpvertkeepoutwinstart_shift	(0)
#define HDMI_HDCP_KOWRSE_hdcpvertkeepoutwinstart_mask	(0x00000FFF)

#define HDMI_HDCP_OWR					0xF8
#define HDMI_HDCP_OWR_write_en2_shift			(25)
#define HDMI_HDCP_OWR_write_en2_mask			(0x02000000)
#define HDMI_HDCP_OWR_hdcpoppwinend_shift		(13)
#define HDMI_HDCP_OWR_hdcpoppwinend_mask		(0x01FFE000)
#define HDMI_HDCP_OWR_write_en1_shift			(12)
#define HDMI_HDCP_OWR_write_en1_mask			(0x00001000)
#define HDMI_HDCP_OWR_hdcpoppwinstart_shift		(0)
#define HDMI_HDCP_OWR_hdcpoppwinstart_mask		(0x00000FFF)

#define HDMI_HDCP_CR					0xFC
#define HDMI_HDCP_CR_write_en4_shift			(7)
#define HDMI_HDCP_CR_write_en4_mask			(0x00000080)
#define HDMI_HDCP_CR_en1_1_feature_shift		(6)
#define HDMI_HDCP_CR_en1_1_feature_mask			(0x00000040)
#define HDMI_HDCP_CR_write_en3_shift			(5)
#define HDMI_HDCP_CR_write_en3_mask			(0x00000020)
#define HDMI_HDCP_CR_downstrisrepeater_shift		(4)
#define HDMI_HDCP_CR_downstrisrepeater_mask		(0x00000010)
#define HDMI_HDCP_CR_write_en2_shift			(3)
#define HDMI_HDCP_CR_write_en2_mask			(0x00000008)
#define HDMI_HDCP_CR_aninfluencemode_shift		(2)
#define HDMI_HDCP_CR_aninfluencemode_mask		(0x00000004)
#define HDMI_HDCP_CR_write_en1_shift			(1)
#define HDMI_HDCP_CR_write_en1_mask			(0x00000002)
#define HDMI_HDCP_CR_hdcp_encryptenable_shift		(0)
#define HDMI_HDCP_CR_hdcp_encryptenable_mask		(0x00000001)

#define HDMI_HDCP_AUTH					0x100
#define HDMI_HDCP_AUTH_write_en9_shift			(17)
#define HDMI_HDCP_AUTH_write_en9_mask			(0x00020000)
#define HDMI_HDCP_AUTH_ddpken_shift			(16)
#define HDMI_HDCP_AUTH_ddpken_mask			(0x00010000)
#define HDMI_HDCP_AUTH_write_en8_shift			(15)
#define HDMI_HDCP_AUTH_write_en8_mask			(0x00008000)
#define HDMI_HDCP_AUTH_resetkmacc_shift			(14)
#define HDMI_HDCP_AUTH_resetkmacc_mask			(0x00004000)
#define HDMI_HDCP_AUTH_write_en7_shift			(13)
#define HDMI_HDCP_AUTH_write_en7_mask			(0x00002000)
#define HDMI_HDCP_AUTH_update_an_shift			(12)
#define HDMI_HDCP_AUTH_update_an_mask			(0x00001000)
#define HDMI_HDCP_AUTH_write_en6_shift			(11)
#define HDMI_HDCP_AUTH_write_en6_mask			(0x00000800)
#define HDMI_HDCP_AUTH_aninfreq_shift			(10)
#define HDMI_HDCP_AUTH_aninfreq_mask			(0x00000400)
#define HDMI_HDCP_AUTH_write_en5_shift			(9)
#define HDMI_HDCP_AUTH_write_en5_mask			(0x00000200)
#define HDMI_HDCP_AUTH_seedload_shift			(8)
#define HDMI_HDCP_AUTH_seedload_mask			(0x00000100)
#define HDMI_HDCP_AUTH_write_en4_shift			(7)
#define HDMI_HDCP_AUTH_write_en4_mask			(0x00000080)
#define HDMI_HDCP_AUTH_deviceauthenticated_shift	(6)
#define HDMI_HDCP_AUTH_deviceauthenticated_mask		(0x00000040)
#define HDMI_HDCP_AUTH_write_en3_shift			(5)
#define HDMI_HDCP_AUTH_write_en3_mask			(0x00000020)
#define HDMI_HDCP_AUTH_forcetounauthenticated_shift	(4)
#define HDMI_HDCP_AUTH_forcetounauthenticated_mask	(0x00000010)
#define HDMI_HDCP_AUTH_write_en2_shift			(3)
#define HDMI_HDCP_AUTH_write_en2_mask			(0x00000008)
#define HDMI_HDCP_AUTH_authcompute_shift		(2)
#define HDMI_HDCP_AUTH_authcompute_mask			(0x00000004)
#define HDMI_HDCP_AUTH_write_en1_shift			(1)
#define HDMI_HDCP_AUTH_write_en1_mask			(0x00000002)
#define HDMI_HDCP_AUTH_authrequest_shift		(0)
#define HDMI_HDCP_AUTH_authrequest_mask			(0x00000001)

#define HDMI_HDCP_SR					0x104
#define HDMI_HDCP_SR_hdcpcipherstate_shift		(24)
#define HDMI_HDCP_SR_hdcpcipherstate_mask		(0xFF000000)
#define HDMI_HDCP_SR_curdpkaccdone_shift		(3)
#define HDMI_HDCP_SR_curdpkaccdone_mask			(0x00000008)
#define HDMI_HDCP_SR_hdcp_encrypt_status_shift		(2)
#define HDMI_HDCP_SR_hdcp_encrypt_status_mask		(0x00000004)
#define HDMI_HDCP_SR_authenticatedok_shift		(1)
#define HDMI_HDCP_SR_authenticatedok_mask		(0x00000002)
#define HDMI_HDCP_SR_anready_shift			(0)
#define HDMI_HDCP_SR_anready_mask			(0x00000001)

#define HDMI_HDCP_ANLR					0x108
#define HDMI_HDCP_ANLR_anlsw_shift			(0)
#define HDMI_HDCP_ANLR_anlsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_ANMR					0x10C
#define HDMI_HDCP_ANMR_anmsw_shift			(0)
#define HDMI_HDCP_ANMR_anmsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_ANILR					0x110
#define HDMI_HDCP_ANILR_aninfluencelsw_shift		(0)
#define HDMI_HDCP_ANILR_aninfluencelsw_mask		(0xFFFFFFFF)

#define HDMI_HDCP_ANIMR					0x114
#define HDMI_HDCP_ANIMR_aninfluencemsw_shift		(0)
#define HDMI_HDCP_ANIMR_aninfluencemsw_mask		(0xFFFFFFFF)

#define HDMI_HDCP_DPKLR					0x118
#define HDMI_HDCP_DPKLR_ddpklsb24_shift			(8)
#define HDMI_HDCP_DPKLR_ddpklsb24_mask			(0xFFFFFF00)
#define HDMI_HDCP_DPKLR_dpkencpnt_shift			(0)
#define HDMI_HDCP_DPKLR_dpkencpnt_mask			(0x000000FF)

#define HDMI_HDCP_DPKMR					0x11C
#define HDMI_HDCP_DPKMR_ddpkmsw_shift			(0)
#define HDMI_HDCP_DPKMR_ddpkmsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_LIR					0x120
#define HDMI_HDCP_LIR_ri_shift				(8)
#define HDMI_HDCP_LIR_ri_mask				(0x00FFFF00)
#define HDMI_HDCP_LIR_pj_shift				(0)
#define HDMI_HDCP_LIR_pj_mask				(0x000000FF)

#define HDMI_HDCP_SHACR					0x124
#define HDMI_HDCP_SHACR_shastart_shift			(3)
#define HDMI_HDCP_SHACR_shastart_mask			(0x00000008)
#define HDMI_HDCP_SHACR_shafirst_shift			(2)
#define HDMI_HDCP_SHACR_shafirst_mask			(0x00000004)
#define HDMI_HDCP_SHACR_rstshaptr_shift			(1)
#define HDMI_HDCP_SHACR_rstshaptr_mask			(0x00000002)
#define HDMI_HDCP_SHACR_write_data_shift		(0)
#define HDMI_HDCP_SHACR_write_data_mask			(0x00000001)

#define HDMI_HDCP_SHARR					0x128
#define HDMI_HDCP_SHARR_vmatch_shift			(1)
#define HDMI_HDCP_SHARR_vmatch_mask			(0x00000002)
#define HDMI_HDCP_SHARR_shaready_shift			(0)
#define HDMI_HDCP_SHARR_shaready_mask			(0x00000001)

#define HDMI_HDCP_SHADR					0x12C
#define HDMI_HDCP_SHADR_sha_data_shift			(0)
#define HDMI_HDCP_SHADR_sha_data_mask			(0xFFFFFFFF)

#define HDMI_HDCP_MILSW					0x130
#define HDMI_HDCP_MILSW_milsw_shift			(0)
#define HDMI_HDCP_MILSW_milsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_MIMSW					0x134
#define HDMI_HDCP_MIMSW_mimsw_shift			(0)
#define HDMI_HDCP_MIMSW_mimsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_KMLSW					0x138
#define HDMI_HDCP_KMLSW_kmlsw_shift			(0)
#define HDMI_HDCP_KMLSW_kmlsw_mask			(0xFFFFFFFF)

#define HDMI_HDCP_KMMSW					0x13C
#define HDMI_HDCP_KMMSW_kmmsw_shift			(0)
#define HDMI_HDCP_KMMSW_kmmsw_mask			(0xFFFFFFFF)

/* HDMI debug register */
#define HDMI_DBG					0x140
#define HDMI_DBG_chsel0_shift				(4)
#define HDMI_DBG_chsel0_mask				(0x000000F0)
#define HDMI_DBG_chsel1_shift				(0)
#define HDMI_DBG_chsel1_mask				(0x0000000F)

/* HDMI dummy registers */
#define HDMI_DUMMY_0					0x144
#define HDMI_DUMMY_1					0x148
#define HDMI_DUMMY_0_dummy_shift			(0)
#define HDMI_DUMMY_1_dummy_mask				(0xFFFFFFFF)
#define HDMI_DUMMY_0_dummy_shift			(0)
#define HDMI_DUMMY_1_dummy_mask				(0xFFFFFFFF)

/* HDMI PHY registers */
#define HDMI_PHY					0x14C
#define HDMI_PHY_test_mode_shift			(1)
#define HDMI_PHY_test_mode_mask				(0x00000002)
#define HDMI_PHY_data_realign_shift			(0)
#define HDMI_PHY_data_realign_mask			(0x00000001)

/* HDMI timing generator contorl registers */
#define HDMI_H_PARA1				0x234
#define HDMI_H_PARA1_hblank_shift		(16)
#define HDMI_H_PARA1_hactive_shift		(0)
#define HDMI_H_PARA1_hblank_mask		(0x3FFF0000)
#define HDMI_H_PARA1_hactive_mask		(0x00003FFF)

#define HDMI_H_PARA2				0x238
#define HDMI_H_PARA2_hsync_shift		(16)
#define HDMI_H_PARA2_hfront_shift		(0)
#define HDMI_H_PARA2_hsync_mask			(0x3FFF0000)
#define HDMI_H_PARA2_hfront_mask		(0x00003FFF)

#define HDMI_H_PARA3				0x23C
#define HDMI_H_PARA3_hback_shift		(0)
#define HDMI_H_PARA3_hback_mask			(0x00003FFF)

#define HDMI_V_PARA1				0x240
#define HDMI_V_PARA1_Vact_video_shift		(16)
#define HDMI_V_PARA1_vactive_shift		(0)
#define HDMI_V_PARA1_Vact_video_mask		(0x3FFF0000)
#define HDMI_V_PARA1_vactive_mask		(0x00003FFF)

#define HDMI_V_PARA2				0x244
#define HDMI_V_PARA2_Vact_space1_shift		(16)
#define HDMI_V_PARA2_Vact_space_shift		(0)
#define HDMI_V_PARA2_Vact_space1_mask		(0x3FFF0000)
#define HDMI_V_PARA2_Vact_space_mask		(0x00003FFF)

#define HDMI_V_PARA3				0x248
#define HDMI_V_PARA3_Vblank3_shift		(16)
#define HDMI_V_PARA3_Vact_space2_shift		(0)
#define HDMI_V_PARA3_Vblank3_mask		(0x3FFF0000)
#define HDMI_V_PARA3_Vact_space2_mask		(0x00003FFF)

#define HDMI_V_PARA4				0x24C
#define HDMI_V_PARA4_vsync_shift		(16)
#define HDMI_V_PARA4_vblank_shift		(0)
#define HDMI_V_PARA4_vsync_mask			(0x3FFF0000)
#define HDMI_V_PARA4_vblank_mask		(0x00003FFF)

#define HDMI_V_PARA5				0x250
#define HDMI_V_PARA5_vback_shift		(16)
#define HDMI_V_PARA5_vfront_shift		(0)
#define HDMI_V_PARA5_vback_mask			(0x3FFF0000)
#define HDMI_V_PARA5_vfront_mask		(0x00003FFF)

#define HDMI_V_PARA6				0x254
#define HDMI_V_PARA6_Vsync1_shift		(16)
#define HDMI_V_PARA6_Vblank1_shift		(0)
#define HDMI_V_PARA6_Vsync1_mask		(0x3FFF0000)
#define HDMI_V_PARA6_Vblank1_mask		(0x00003FFF)

#define HDMI_V_PARA7				0x258
#define HDMI_V_PARA7_Vback1_shift		(16)
#define HDMI_V_PARA7_Vfront1_shift		(0)
#define HDMI_V_PARA7_Vback1_mask		(0x3FFF0000)
#define HDMI_V_PARA7_Vfront1_mask		(0x00003FFF)

#define HDMI_V_PARA8				0x25C
#define HDMI_V_PARA8_Vsync2_shift		(16)
#define HDMI_V_PARA8_Vblank2_shift		(0)
#define HDMI_V_PARA8_Vsync2_mask		(0x3FFF0000)
#define HDMI_V_PARA8_Vblank2_mask		(0x00003FFF)

#define HDMI_V_PARA9				0x260
#define HDMI_V_PARA9_Vback2_shift		(16)
#define HDMI_V_PARA9_Vfront2_shift		(0)
#define HDMI_V_PARA9_Vback2_mask		(0x3FFF0000)
#define HDMI_V_PARA9_Vfront2_mask		(0x00003FFF)

#define HDMI_V_PARA12				0x264
#define HDMI_V_PARA12_vsynci_shift		(16)
#define HDMI_V_PARA12_vblanki_shift		(0)
#define HDMI_V_PARA12_vsynci_mask		(0x3FFF0000)
#define HDMI_V_PARA12_vblanki_mask		(0x00003FFF)

#define HDMI_V_PARA13				0x268
#define HDMI_V_PARA13_vbacki_shift		(16)
#define HDMI_V_PARA13_vfronti_shift		(0)
#define HDMI_V_PARA13_vbacki_mask		(0x3FFF0000)
#define HDMI_V_PARA13_vfronti_mask		(0x00003FFF)

#define HDMI_V_PARA10				0x26C
#define HDMI_V_PARA10_G_shift			(0)
#define HDMI_V_PARA10_G_mask			(0x00000FFF)

#define HDMI_V_PARA11				0x270
#define HDMI_V_PARA11_B_shift			(16)
#define HDMI_V_PARA11_R_shift			(0)
#define HDMI_V_PARA11_B_mask			(0x0FFF0000)
#define HDMI_V_PARA11_R_mask			(0x00000FFF)

/* VDMA registers map */
#define VDMA_CTRL				0x0
#define VDMA_CTRL_vdma_en_shift			(0)
#define VDMA_CTRL_encondition_shift		(1)
#define VDMA_CTRL_errormode_en_shift		(4)
#define VDMA_CTRL_vdma_en_mask			(0x00000001)
#define VDMA_CTRL_encondition_mask		(0x00000002)
#define VDMA_CTRL_errormode_en_mask		(0x00000010)

#define VDMA_FBUF_ADDR				0x4
#define VDMA_FBUF_ADDR_addr_shift		(0)
#define VDMA_FBUF_ADDR_addr_mask		(0xFFFFFFFF)

#define VDMA_FBUF_SIZE				0x8
#define VDMA_FBUF_SIZE_ver_shift		(0)
#define VDMA_FBUF_SIZE_hor_shift		(12)
#define VDMA_FBUF_SIZE_rgb_shift		(24)
#define VDMA_FBUF_SIZE_ver_mask			(0x00000FFF)
#define VDMA_FBUF_SIZE_hor_mask			(0x00FFF000)
#define VDMA_FBUF_SIZE_rgb_mask			(0x0F000000)

#define VDMA_PHY_TEST				0xC
#define VDMA_PHY_TEST_en_shift			(0)
#define VDMA_PHY_TEST_en_mask			(0x00000001)

#define VDMA_STS				0x10
#define VDMA_STS_err_clr_shift			(1)
#define VDMA_STS_truncate_clr_shift		(2)
#define VDMA_STS_err_clr_mask			(0x00000002)
#define VDMA_STS_truncate_clr_mask		(0x00000001)

#define VDMA_CBAR				0x14
#define VDMA_CBAR_en_shift			(0)
#define VDMA_CBAR_en_mask			(0x00000001)

#define VDMA_CBAR_DATA0				0x18
#define VDMA_CBAR_DATA0_data_shift		(0)
#define VDMA_CBAR_DATA0_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA1				0x1C
#define VDMA_CBAR_DATA1_data_shift		(0)
#define VDMA_CBAR_DATA1_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA2				0x20
#define VDMA_CBAR_DATA2_data_shift		(0)
#define VDMA_CBAR_DATA2_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA3				0x24
#define VDMA_CBAR_DATA3_data_shift		(0)
#define VDMA_CBAR_DATA3_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA4				0x28
#define VDMA_CBAR_DATA4_data_shift		(0)
#define VDMA_CBAR_DATA4_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA5				0x2C
#define VDMA_CBAR_DATA5_data_shift		(0)
#define VDMA_CBAR_DATA5_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA6				0x30
#define VDMA_CBAR_DATA6_data_shift		(0)
#define VDMA_CBAR_DATA6_data_mask		(0x00FFFFFF)

#define VDMA_CBAR_DATA7				0x34
#define VDMA_CBAR_DATA7_data_shift		(0)
#define VDMA_CBAR_DATA7_data_mask		(0x00FFFFFF)

#define VDMA_DUMMY				0x50
#define VDMA_DUMMY_data_shift			(0)
#define VDMA_DUMMY_data_mask			(0xFFFFFFFF)

#define VDMA_DEBUG				0x4C
#define VDMA_DEBGU_dbgsel_shift			(0)
#define VDMA_DEBGU_chnsel_shift			(8)
#define VDMA_DEBGU_phyrst_shift			(31)
#define VDMA_DEBGU_dbgsel_mask			(0x0000001f)
#define VDMA_DEBGU_chnsel_mask			(0x00000700)
#define VDMA_DEBGU_phyrst_mask			(0x80000000)

#define VDMA_CMD_MASK				0x58
#define VDMA_CMD_MASK_data_shift		(0)
#define VDMA_CMD_MASK_data_mask			(0xFFFFFFFF)

#define VDMA_RESP_MASK				0x5C
#define VDMA_RESP_MASK_data_shift		(0)
#define VDMA_RESP_MASK_data_mask		(0xFFFFFFFF)

#define AUDIO_INFOFRAME_TYPE	0x84
#define AUDIO_INFOFRAME_VER	0x01
#define AUDIO_INFOFRAME_LEN	10

#define VIDEO_INFORAME_TYPE	0x82
#define VIDEO_INFOFRAME_VER	0x02
#define VIDEO_INFOFRAME_LEN	13
#define S1S0			0	/* 0=>No data, 1=>overscanned, 2=>underscanned */
#define B1B0			0	/* 0=>No data, 1=>V Bar Info valid, 2=>H Bar Info
								 valid, 3=>V&H Bar Info valid */
#define A0			1	/* 0=>No data, 1=>Active Format Infomation present */
#define F7			0
#define R3R2R1R0		8	/* 8=>Same as picture aspect ratio, 9=>4:3, 10=>16:9, 11=>14:9 */
#define M1M0			2	/* 0=>No Data, 1=>4:3, 2=>16:9 */
#define C1C0			0	/* 0=>No Data, 1=>ITU601, 2=>ITU709,
							3=>Extended Colorimetry Valid */
#define SC1SC0			0	/* 0=>unknown, 1=>Scaled H, 2=>Scaled V, 3=>Scaled V&H */
#define Q1Q0			0	/* 0=>default, 1=>Limited Range, 2=>Full Range */
#define EC2EC1EC0		2	/* 0=>xvYCC601, 1=>xvYCC709, 2=>reserved */
#define ITC			0	/* 0=>No data, 1=>IT content */
#define PR3PR2PR1PR0		0	/* 0:Pixel No Repetition, 1:Pixel sent 2 times */

#endif/* SHEIPAHDMI_H */
