/*
 *  Driver for SHEIPA_HDMI Controller
 *
 *  Copyright (C) 2019 Realtek Semiconductor Corp.
 */

#include <linux/kernel.h>
#include <linux/fb.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "sheipa_hdmi.h"


#define HDMI_R8(reg)		readb((hdmi_base) + (reg))
#define HDMI_R16(reg)		readw((hdmi_base) + (reg))
#define HDMI_R32(reg)		readl((hdmi_base) + (reg))
#define HDMI_W8(reg,val)  	writeb((val), (hdmi_base) + (reg))
#define HDMI_W16(reg,val) 	writew((val), (hdmi_base) + (reg))
#define HDMI_W32(reg,val) 	writel((val), (hdmi_base) + (reg))

#define VDMA_R8(reg)      	readb((vdma_base) + (reg))
#define VDMA_R16(reg)     	readw((vdma_base) + (reg))
#define VDMA_R32(reg)     	readl((vdma_base) + (reg))
#define VDMA_W8(reg,val)  	writeb((val), (vdma_base) + (reg))
#define VDMA_W16(reg,val) 	writew((val), (vdma_base) + (reg))
#define VDMA_W32(reg,val) 	writel((val), (vdma_base) + (reg))

struct sheipa_hdmi_info {
	struct device	*dev;
	struct fb_info	*info;
	unsigned long	hdmi_base;
	unsigned long	vdma_base;
	int				hdmi_size;
	int				vdma_size;
	int				irq_base;
};

static void __iomem *hdmi_base;
static void __iomem *vdma_base;

void HDMI_SetRamPacket(unsigned int pkt_no, unsigned char *pkt)
{
	unsigned char tpkt[36];
	unsigned long *reg = (unsigned long *)tpkt;
	unsigned int addr = pkt_no * 14;

	tpkt[3]  = 0;
	tpkt[2]  = pkt[2];
	tpkt[1]  = pkt[1];
	tpkt[0]  = pkt[0];

	tpkt[7]  = pkt[6];
	tpkt[6]  = pkt[5];
	tpkt[5]  = pkt[4];
	tpkt[4]  = pkt[3];

	tpkt[11] = 0;
	tpkt[10] = pkt[9];
	tpkt[9]  = pkt[8];
	tpkt[8]  = pkt[7];

	tpkt[15] =  pkt[13];
	tpkt[14] =  pkt[12];
	tpkt[13] =  pkt[11];
	tpkt[12] =  pkt[10];

	tpkt[19] =  0;
	tpkt[18] =  pkt[16];
	tpkt[17] =  pkt[15];
	tpkt[16] =  pkt[14];

	tpkt[23] =  pkt[20];
	tpkt[22] =  pkt[19];
	tpkt[21] =  pkt[18];
	tpkt[20] =  pkt[17];

	tpkt[27] =  0;
	tpkt[26] =  pkt[23];
	tpkt[25] =  pkt[22];
	tpkt[24] =  pkt[21];

	tpkt[31] =  pkt[27];
	tpkt[30] =  pkt[26];
	tpkt[29] =  pkt[25];
	tpkt[28] =  pkt[24];

	tpkt[35] =  0;
	tpkt[34] =  pkt[30];
	tpkt[33] =  pkt[29];
	tpkt[32] =  pkt[28];

	/* write mode */
	HDMI_W32(HDMI_ORP6PH,  VAL(once_pkt, ORP6PH,  reg[0]));
	HDMI_W32(HDMI_ORP6W_0, VAL(once_pkt, ORP6W_0, reg[1]));
	HDMI_W32(HDMI_ORP6W_1, VAL(once_pkt, ORP6W_1, reg[2]));
	HDMI_W32(HDMI_ORP6W_2, VAL(once_pkt, ORP6W_2, reg[3]));
	HDMI_W32(HDMI_ORP6W_3, VAL(once_pkt, ORP6W_3, reg[4]));
	HDMI_W32(HDMI_ORP6W_4, VAL(once_pkt, ORP6W_4, reg[5]));
	HDMI_W32(HDMI_ORP6W_5, VAL(once_pkt, ORP6W_5, reg[6]));
	HDMI_W32(HDMI_ORP6W_6, VAL(once_pkt, ORP6W_6, reg[7]));
	HDMI_W32(HDMI_ORP6W_7, VAL(once_pkt, ORP6W_7, reg[8]));

	HDMI_W32(HDMI_OPCR,
			VAL(write_en4, OPCR, 1) | VAL(enrbpktsram, OPCR, 1) |
			VAL(write_en3, OPCR, 1) | VAL(wrdes, OPCR, 0) |
			VAL(write_en2, OPCR, 1) | VAL(rprwcmd, OPCR, 1) |
			VAL(write_en1, OPCR, 1) | VAL(rpadd, OPCR, addr & 0x0ff));
}

void HDMI_gen_audio_infoframe(unsigned char ChannelNumber)
{
	unsigned char pkt[32];
	unsigned int checksum=0;
	unsigned int i;

	/* clear buffer */
	for (i = 0; i <= 31; i++)
	 	pkt[i] = 0;

	/* header */
	pkt[0] = AUDIO_INFOFRAME_TYPE;
	pkt[1] = AUDIO_INFOFRAME_VER;
	pkt[2] = AUDIO_INFOFRAME_LEN;
	pkt[3] = checksum;

	/* data */
	pkt[4] = 0x07 & ChannelNumber;
	pkt[5] = 0;
	pkt[6] = 0;
	pkt[7] = 0;
	pkt[8] = 0;

	/* count checksum */
	for (i = 0; i <= 31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1) & 0xff;

	/* set to RAM Packet */
	HDMI_SetRamPacket(1, pkt);

	HDMI_W32(HDMI_RPCR,
			VAL(write_en6, RPCR, 0) | VAL(prp5period, RPCR, 0) |
			VAL(write_en5, RPCR, 0) | VAL(prp4period, RPCR, 0) |
			VAL(write_en4, RPCR, 0) | VAL(prp3period, RPCR, 0) |
			VAL(write_en3, RPCR, 0) | VAL(prp2period, RPCR, 0) |
			VAL(write_en2, RPCR, 1) | VAL(prp1period, RPCR, 1) |
			VAL(write_en1, RPCR, 0) | VAL(prp0period, RPCR, 0));

	HDMI_W32(HDMI_RPEN,
			VAL(enprpkt5, RPEN, 0) |
			VAL(enprpkt4, RPEN, 0) |
			VAL(enprpkt3, RPEN, 0) |
			VAL(enprpkt2, RPEN, 0) |
			VAL(enprpkt1, RPEN, 1) |
			VAL(enprpkt0, RPEN, 0) |
			VAL(write_data, RPEN, 1));
}

void HDMI_gen_avi_infoframe(unsigned char Video_Code, unsigned char RGB_YCbCr,
			    unsigned char xv_YCC)
{
	unsigned char pkt[32];
	unsigned char checksum = 0;
	unsigned char i;
	unsigned char Y1Y0 = RGB_YCbCr; /* 0=>RGB, 1=>YCbCr422, 2=>YCbCr444 */

	if (Y1Y0 == 3)
		Y1Y0 = 1;

	/* clear buffer */
	for (i = 0; i <= 31; i++)
		pkt[i] = 0;

	/* header */
	pkt[0] = VIDEO_INFORAME_TYPE;
	pkt[1] = VIDEO_INFOFRAME_VER;
	pkt[2] = VIDEO_INFOFRAME_LEN;
	pkt[3] = checksum;

	/* data */
	pkt[4] = 0x7f & ((F7 << 7) | (Y1Y0 << 5) | (A0 << 4) | (B1B0 << 2) | (S1S0));
	pkt[5] = (C1C0 << 6) | (M1M0 << 4) | (R3R2R1R0);
	pkt[6] = (ITC << 7) | (EC2EC1EC0 << 4) | (Q1Q0 << 2) | (SC1SC0);
	pkt[7] = 0x7f & Video_Code;
	pkt[8] = 0x0f & PR3PR2PR1PR0; //480i,576i=1,others:0
	pkt[9] = 0;
	pkt[10] = 0;
	pkt[11] = 0;
	pkt[12] = 0;
	pkt[13] = 0;
	pkt[14] = 0;
	pkt[15] = 0;
	pkt[16] = 0;

	/* count checksum */
	for (i = 0; i <= 31; i++)
		checksum += pkt[i];
	pkt[3] = (~checksum + 1) & 0xff;

	/* set to RAM Packet */
	HDMI_SetRamPacket(0,pkt);

	HDMI_W32(HDMI_RPCR,
			VAL(write_en6, RPCR, 0) | VAL(prp5period, RPCR, 0) |
			VAL(write_en5, RPCR, 0) | VAL(prp4period, RPCR, 0) |
			VAL(write_en4, RPCR, 0) | VAL(prp3period, RPCR, 0) |
			VAL(write_en3, RPCR, 0) | VAL(prp2period, RPCR, 0) |
			VAL(write_en2, RPCR, 0) | VAL(prp1period, RPCR, 0) |
			VAL(write_en1, RPCR, 1) | VAL(prp0period, RPCR, 1));

	HDMI_W32(HDMI_RPEN,
			VAL(enprpkt5, RPEN, 0) |
			VAL(enprpkt4, RPEN, 0) |
			VAL(enprpkt3, RPEN, 0) |
			VAL(enprpkt2, RPEN, 0) |
			VAL(enprpkt1, RPEN, 0) |
			VAL(enprpkt0, RPEN, 1) |
			VAL(write_data, RPEN, 1));
}

static void sheipa_hdmi_hdcp_on(struct fb_info *info)
{
	unsigned int data = 0;

	HDMI_W32(HDMI_HDCP_ANILR, VAL(aninfluencelsw,HDCP_ANILR, 0));
	HDMI_W32(HDMI_HDCP_ANIMR, VAL(aninfluencemsw, HDCP_ANIMR, 0));
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en5, HDCP_AUTH, 1) | VAL(seedload, HDCP_AUTH, 1));

	/* get dummy An */
	HDMI_W32(HDMI_HDCP_ANLR, VAL(anlsw, HDCP_ANLR, 0));
	HDMI_W32(HDMI_HDCP_ANMR, VAL(anmsw, HDCP_ANMR, 0));
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en6, HDCP_AUTH, 1) | VAL(aninfreq, HDCP_AUTH, 1));

	/* set An Influence Mode, influence will be load from AnIR0, AnIR1 */
	HDMI_W32(HDMI_HDCP_CR,
			VAL(write_en4, HDCP_CR, 1) | VAL(en1_1_feature, HDCP_CR, 0) |
			VAL(write_en3, HDCP_CR, 1) | VAL(downstrisrepeater, HDCP_CR, 0) |
			VAL(write_en2, HDCP_CR, 1) | VAL(aninfluencemode, HDCP_CR, 1));

	/* trigger to get An */
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en1, HDCP_AUTH, 1) | VAL(authrequest, HDCP_AUTH, 1));
	do{
		data = HDMI_R32(HDMI_HDCP_SR);
	}while(!(data & MASK(anready, HDCP_SR)));

	HDMI_W32(HDMI_HDCP_ANLR, VAL(anlsw, HDCP_ANLR, 0x0c070403));
	HDMI_W32(HDMI_HDCP_ANMR, VAL(anmsw, HDCP_ANMR, 0x34271c13));

	/* turn off, 0xb800d0fc */
	HDMI_W32(HDMI_HDCP_CR,
			VAL(write_en4, HDCP_CR, 1) | VAL(en1_1_feature, HDCP_CR, 0) |
			VAL(write_en1, HDCP_CR, 1) | VAL(hdcp_encryptenable, HDCP_CR, 0));

	/* == call hdcpBlkCipher(), wait Ri interrupt */
	HDMI_W32(HDMI_INTEN,
			VAL(enriupdint, INTEN, 1) | VAL(enpjupdint, INTEN, 0) |
			VAL(write_data, INTEN, 0));

	/* clear Ri updated pending bit */
	HDMI_W32(HDMI_INTST,
			VAL(riupdated, INTST, 0) | VAL(pjupdated, INTST, 0));

	/* trigger hdcpBlockCipher at authentication */
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en2, HDCP_AUTH, 1) | VAL(authcompute, HDCP_AUTH, 1));

	/* wait 48+56 pixel clock to get R0 */
	do{
		data = HDMI_R32(HDMI_INTST);
	}while(!(data & MASK(riupdated, INTST)));

	/* enable hdcp encryption */
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en4, HDCP_AUTH, 1) | VAL(deviceauthenticated, HDCP_AUTH, 1));
	HDMI_W32(HDMI_HDCP_CR,
			VAL(write_en4, HDCP_CR, 1) | VAL(en1_1_feature, HDCP_CR, 1) |
			VAL(write_en1, HDCP_CR, 1) | VAL(hdcp_encryptenable, HDCP_CR, 1));
}

static void sheipa_hdmi_hdcp_off(struct fb_info *info)
{
	/* 0xb800d0fc */
	HDMI_W32(HDMI_HDCP_CR,
			VAL(write_en4, HDCP_CR, 1) | VAL(en1_1_feature, HDCP_CR, 0) |
			VAL(write_en1, HDCP_CR, 1) | VAL(hdcp_encryptenable, HDCP_CR, 0));

	/* force HDCP module to unauthenticated state */
	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en3, HDCP_AUTH, 1) | VAL(forcetounauthenticated, HDCP_AUTH, 1));
}

static void sheipa_hdmi_timming_setup(struct fb_info *info)
{
	/* Write all parameters to HDMI registers */
	HDMI_W32(HDMI_H_PARA1, VAL(hblank, H_PARA1, 137) | VAL(hactive, H_PARA1, 719));
	HDMI_W32(HDMI_H_PARA2, VAL(hsync, H_PARA2, 61) | VAL(hfront, H_PARA2, 15));
	HDMI_W32(HDMI_H_PARA3, VAL(hback, H_PARA3, 59));
	HDMI_W32(HDMI_V_PARA1, VAL(Vact_video, V_PARA1, 479) | VAL(vactive, V_PARA1, 479));
	HDMI_W32(HDMI_V_PARA2, VAL(Vact_space1, V_PARA2, 0) | VAL(Vact_space, V_PARA2, 0));
	HDMI_W32(HDMI_V_PARA3, VAL(Vblank3, V_PARA3, 0) | VAL(Vact_space2, V_PARA3, 0));
	HDMI_W32(HDMI_V_PARA4, VAL(vsync, V_PARA4, (6 << 1)) | VAL(vblank, V_PARA4, 44));
	HDMI_W32(HDMI_V_PARA5, VAL(vback, V_PARA5, 29) | VAL(vfront, V_PARA5, (9 << 1)));
	HDMI_W32(HDMI_V_PARA6, VAL(Vsync1, V_PARA6, (0 << 1)) | VAL(Vblank1, V_PARA6, 0));
	HDMI_W32(HDMI_V_PARA7, VAL(Vback1, V_PARA7, 0) | VAL(Vfront1, V_PARA7, (0 << 1)));
	HDMI_W32(HDMI_V_PARA8, VAL(Vsync2, V_PARA8, (0 << 1)) | VAL(Vblank2, V_PARA8, 0));
	HDMI_W32(HDMI_V_PARA9, VAL(Vback2, V_PARA9, 0) | VAL(Vfront2, V_PARA9, (0 << 1)));
	HDMI_W32(HDMI_V_PARA12, VAL(vsynci, V_PARA12, (0 << 1)) | VAL(vblanki, V_PARA12, 0));
	HDMI_W32(HDMI_V_PARA13, VAL(vbacki, V_PARA13, (0)) | VAL(vfronti, V_PARA13, (0 << 1)));
	HDMI_W32(HDMI_V_PARA10, VAL(G, V_PARA10, 0x00beef));
	HDMI_W32(HDMI_V_PARA11, VAL(B, V_PARA11, 0x00beef) | VAL(R, V_PARA11, 0x00beef));
}

static void sheipa_hdmi_vdma_setup(struct fb_info *info)
{
	/* Set up VDMA registers */
	VDMA_W32(VDMA_FBUF_ADDR, VALD(addr, FBUF_ADDR, (info->fix.smem_start)));

	VDMA_W32(VDMA_FBUF_SIZE, VALD(ver, FBUF_SIZE, 480) |
			VALD(hor, FBUF_SIZE, 720) | VALD(rgb, FBUF_SIZE, 0x8));

	/* Auto-enable */
//	VDMA_W32(VDMA_CTRL, VALD(encondition, CTRL, 0x1) | VALD(errormode_en, CTRL, 0x1));

	/* Interrupt-based */
	VDMA_W32(VDMA_CTRL, VALD(encondition, CTRL, 0x0) |
			VALD(vdma_en, CTRL, 0x0) | VALD(errormode_en, CTRL, 0x1));
}

static void sheipa_hdmi_video_setup(struct fb_info *info)
{
	unsigned char Video_Code = 4, IP = 0, SHD = 0;
	unsigned char Deep_Color = 1, Deep_Depth = 4;
	unsigned char Timing_Gen = 1, En_3D = 0, Format_3D = 0;
	unsigned char Pixel_Colocr_Format = 0;

	HDMI_W32(HDMI_CR,
			VAL(write_en3, CR, 1) | VAL(tmds_encen, CR, 0) |
			VAL(write_en2, CR, 0) | VAL(enablehdcp, CR, 0) |
			VAL(write_en1, CR, 1) | VAL(enablehdmi, CR, 0));

	HDMI_W32(HDMI_ICR,
			VAL(write_en3, ICR, 1) | VAL(enaudio, ICR, 0) |
			VAL(write_en2, ICR, 1) | VAL(envitd, ICR, 0) |
			VAL(write_en1, ICR, 0) | VAL(vitd, ICR, 0));

	HDMI_W32(HDMI_SYNC_DLY,
			VAL(write_en4, SYNC_DLY, 1) | VAL(nor_v, SYNC_DLY, 0) |
			VAL(write_en3, SYNC_DLY, 1) | VAL(nor_h, SYNC_DLY, 0) |
			VAL(write_en2, SYNC_DLY, 1) | VAL(disp_v, SYNC_DLY, 0) |
			VAL(write_en1, SYNC_DLY, 1) | VAL(disp_h, SYNC_DLY, 0));

	HDMI_W32(HDMI_DPC,
			VAL(write_en9, DPC, 1) | VAL(dp_vfch_num, DPC, 1) |
			VAL(write_en8, DPC, 1) | VAL(fp_swen, DPC, 0) |
			VAL(write_en7, DPC, 1) | VAL(fp, DPC, 0) |
			VAL(write_en4, DPC, 1) | VAL(dp_swen, DPC, 1) |
			VAL(write_en3, DPC, 1) | VAL(default_phase, DPC, 1) |
			VAL(write_en2, DPC, 1) | VAL(color_depth, DPC, Deep_Depth) |
			VAL(write_en1, DPC, 1) | VAL(dpc_enable, DPC, Deep_Color));

	if(Video_Code == 6 || Video_Code == 21 || \
			Video_Code == 20 || Video_Code == 5)
		IP = 1;
	else
		IP = 0;

	/* HDMI Control */
	/* chl_sel: 0: p channel 1: i channel 2: H channel(p channel)*/
	HDMI_W32(HDMI_CHNL_SEL,
			VAL(video_sd, CHNL_SEL, SHD) | VAL(Interlaced_vfmt, CHNL_SEL, IP) |
			VAL(3D_video_format, CHNL_SEL, Format_3D) | VAL(En_3D, CHNL_SEL, En_3D) |
			VAL(timing_gen_en, CHNL_SEL, Timing_Gen) | VAL(chl_sel, CHNL_SEL, 2));

	HDMI_W32(HDMI_RPEN,
			VAL(enprpkt5, RPEN, 1) | VAL(enprpkt4, RPEN, 1) |
			VAL(enprpkt3, RPEN, 1) | VAL(enprpkt2, RPEN, 1) |
			VAL(enprpkt1, RPEN, 1) | VAL(enprpkt0, RPEN, 1) |
			VAL(write_data, RPEN, 0));

	HDMI_W32(HDMI_SCHCR,
			VAL(write_en10, SCHCR, 1) | VAL(422_pixel_repeat, SCHCR, 0));

	HDMI_W32(HDMI_TMDS_SCR1,
			VAL(cksel, TMDS_SCR1, 0) | VAL(ip, TMDS_SCR1, 7) |
			VAL(rs, TMDS_SCR1, 0) | VAL(cs, TMDS_SCR1, 3) |
			VAL(cp, TMDS_SCR1, 0) | VAL(m, TMDS_SCR1, 1) |
			VAL(wdrst, TMDS_SCR1, 0) | VAL(wdset, TMDS_SCR1, 0) |
			VAL(powb, TMDS_SCR1, 0) | VAL(powc, TMDS_SCR1, 0) |
			VAL(emck, TMDS_SCR1, 0) | VAL(ema, TMDS_SCR1, 1) |
			VAL(emb, TMDS_SCR1, 0) | VAL(emc, TMDS_SCR1, 1));

	HDMI_W32(HDMI_TMDS_SCR3,
			VAL(idrvck, TMDS_SCR3, 2) | VAL(idrva, TMDS_SCR3, 0) |
			VAL(idrvb, TMDS_SCR3, 0) | VAL(idrvc, TMDS_SCR3, 0));

	HDMI_W32(HDMI_TMDS_SCR2,
			VAL(iemck, TMDS_SCR2, 0) | VAL(iema, TMDS_SCR2, 4) |
			VAL(iemb, TMDS_SCR2, 4) | VAL(iemc, TMDS_SCR2, 0xe) |
			VAL(rtonck, TMDS_SCR2, 1) | VAL(rtona, TMDS_SCR2, 0) |
			VAL(rtonb, TMDS_SCR2, 0) | VAL(rtonc, TMDS_SCR2, 0) |
			VAL(vsel, TMDS_SCR2, 1));

	HDMI_W32(HDMI_GCPCR,
			VAL(enablegcp, GCPCR, 1) | VAL(gcp_clearavmute, GCPCR, 0) |
			VAL(gcp_setavmute, GCPCR, 0) | VAL(write_data, GCPCR, 0));

	HDMI_W32(HDMI_GCPCR,
			VAL(enablegcp, GCPCR, 1) | VAL(gcp_clearavmute, GCPCR, 0) |
			VAL(gcp_setavmute, GCPCR, 0) | VAL(write_data, GCPCR, 1));

	HDMI_W32(HDMI_RPCR,
			VAL(write_en6, RPCR, 1) | VAL(prp5period, RPCR, 0) |
			VAL(write_en5, RPCR, 1) | VAL(prp4period, RPCR, 0) |
			VAL(write_en4, RPCR, 1) | VAL(prp3period, RPCR, 0) |
			VAL(write_en3, RPCR, 1) | VAL(prp2period, RPCR, 0) |
			VAL(write_en2, RPCR, 0) | VAL(prp1period, RPCR, 0) |
			VAL(write_en1, RPCR, 1) | VAL(prp0period, RPCR, 0));

	HDMI_W32(HDMI_DIPCCR,
			VAL(write_en2, DIPCCR, 1) | VAL(vbidipcnt, DIPCCR, 17) |
			VAL(write_en1, DIPCCR, 1) | VAL(hbidipcnt, DIPCCR, 8));
	/* Scheduler */

	HDMI_W32(HDMI_SCHCR,
			VAL(write_en9, SCHCR, 0) | VAL(vsyncpolin, SCHCR, 0) |
			VAL(write_en8, SCHCR, 0) | VAL(hsyncpolin, SCHCR, 0) |
			VAL(write_en7, SCHCR, 1) | VAL(vsyncpolinv, SCHCR, 0) |
			VAL(write_en6, SCHCR, 1) | VAL(hsyncpolinv, SCHCR, 0) |
			VAL(write_en5, SCHCR, 1) | VAL(pixelencycbcr422, SCHCR, 0) |
			VAL(write_en4, SCHCR, 1) | VAL(hdmi_videoxs, SCHCR, 0) |
			VAL(write_en3, SCHCR, 1) | VAL(pixelencfmt, SCHCR, Pixel_Colocr_Format) |
			VAL(write_en2, SCHCR, 1) | VAL(pixelrepeat, SCHCR, 0) |
			VAL(write_en1, SCHCR, 1) | VAL(hdmi_modesel, SCHCR, 1)); /*  HDMI mode */
//			VAL(write_en1, SCHCR, 1) | VAL(hdmi_modesel, SCHCR, 0)); /* DVI mode */

	/* HDCP Engine */
	HDMI_W32(HDMI_HDCP_KOWR, VAL(hdcprekeykeepoutwin, HDCP_KOWR, 0x2a));

	HDMI_W32(HDMI_HDCP_KOWRSE,
			VAL(write_en2, HDCP_KOWRSE, 1) | VAL(hdcpverkeepoutwinend, HDCP_KOWRSE, 0x289) |
			VAL(write_en1, HDCP_KOWRSE, 1) | VAL(hdcpvertkeepoutwinstart, HDCP_KOWRSE, 0x1fb));

	HDMI_W32(HDMI_HDCP_OWR,
			VAL(write_en2, HDCP_OWR, 1) | VAL(hdcpoppwinend, HDCP_OWR, 0x20f) |
			VAL(write_en1, HDCP_OWR, 1) | VAL(hdcpoppwinstart, HDCP_OWR, 0x1ff));

	HDMI_W32(HDMI_HDCP_CR,
			VAL(write_en3, HDCP_CR, 1) | VAL(downstrisrepeater, HDCP_CR, 0) |
			VAL(write_en2, HDCP_CR, 1) | VAL(aninfluencemode, HDCP_CR, 0) |
			VAL(write_en1, HDCP_CR, 0) | VAL(hdcp_encryptenable, HDCP_CR, 0));

	HDMI_W32(HDMI_HDCP_AUTH,
			VAL(write_en9, HDCP_AUTH, 0) | VAL(ddpken, HDCP_AUTH, 0) |
			VAL(write_en8, HDCP_AUTH, 0) | VAL(resetkmacc, HDCP_AUTH, 0) |
			VAL(write_en7, HDCP_AUTH, 0) | VAL(update_an, HDCP_AUTH, 0) |
			VAL(write_en6, HDCP_AUTH, 0) | VAL(aninfreq, HDCP_AUTH, 0) |
			VAL(write_en5, HDCP_AUTH, 0) | VAL(seedload, HDCP_AUTH, 0) |
			VAL(write_en4, HDCP_AUTH, 0) | VAL(deviceauthenticated, HDCP_AUTH, 0) |
			VAL(write_en3, HDCP_AUTH, 0) | VAL(forcetounauthenticated, HDCP_AUTH, 0) |
			VAL(write_en2, HDCP_AUTH, 0) | VAL(authcompute, HDCP_AUTH, 0) |
			VAL(write_en1, HDCP_AUTH, 0) | VAL(authrequest, HDCP_AUTH, 0));

	HDMI_W32(HDMI_HDCP_ANLR, VAL(anlsw, HDCP_ANLR, 0));
	HDMI_W32(HDMI_HDCP_ANMR, VAL(anmsw, HDCP_ANMR, 0));
	HDMI_W32(HDMI_HDCP_ANILR, VAL(aninfluencelsw, HDCP_ANILR, 0));
	HDMI_W32(HDMI_HDCP_ANIMR, VAL(aninfluencemsw, HDCP_ANIMR, 0));
	HDMI_W32(HDMI_HDCP_DPKLR, VAL(ddpklsb24, HDCP_DPKLR, 0) | VAL(dpkencpnt, HDCP_DPKLR, 0));
	HDMI_W32(HDMI_HDCP_DPKMR, VAL(ddpkmsw, HDCP_DPKMR, 0));

	HDMI_W32(HDMI_HDCP_SHACR,
			VAL(shastart, HDCP_SHACR, 0) | VAL(shafirst, HDCP_SHACR, 0) |
			VAL(rstshaptr, HDCP_SHACR, 0) | VAL(write_data, HDCP_SHACR, 0));

	HDMI_W32(HDMI_HDCP_SHADR, VAL(sha_data, HDCP_SHADR, 0));
	HDMI_W32(HDMI_HDCP_MILSW, VAL(milsw, HDCP_MILSW, 0));
	HDMI_W32(HDMI_HDCP_MIMSW, VAL(mimsw, HDCP_MIMSW, 0));
	HDMI_W32(HDMI_HDCP_KMLSW, VAL(kmlsw, HDCP_KMLSW, 0));
	HDMI_W32(HDMI_HDCP_KMMSW, VAL(kmmsw, HDCP_KMMSW, 0));

	/* CPU command */
	HDMI_W32(HDMI_INTEN,
			VAL(enriupdint, INTEN, 1) | VAL(enpjupdint, INTEN, 1) |
			VAL(write_data, INTEN, 0));

	HDMI_W32(HDMI_INTENV,
			VAL(vendint, INTENV, 1) | VAL(vsyncint, INTENV, 0) |
			VAL(write_data, INTENV, 1));

	HDMI_gen_avi_infoframe(Video_Code, Pixel_Colocr_Format, 0);

	HDMI_W32(HDMI_CR,
			VAL(write_en3, CR, 1) | VAL(tmds_encen, CR, 1) |
			VAL(write_en2, CR, 1) | VAL(enablehdcp, CR, 1) |
			VAL(write_en1, CR, 1) | VAL(enablehdmi, CR, 1));
}

static void sheipa_hdmi_audio_setup(struct fb_info *info)
{
	unsigned char AudioChannelCount = 1;
	unsigned char CTS_Source=0;

	HDMI_W32(HDMI_ICR,
			VAL(write_en3, ICR, 1) | VAL(enaudio, ICR, 0) |
			VAL(write_en2, ICR, 1) | VAL(envitd, ICR, 0) |
			VAL(write_en1, ICR, 0) | VAL(vitd, ICR, 0));

	HDMI_W32(HDMI_ASPCR1,
			VAL(write_en6, ASPCR1, 1) | VAL(Audioheader0, ASPCR1, 2) |
			VAL(write_en5, ASPCR1, 1) | VAL(samplepresent, ASPCR1, 0) |
			VAL(write_en4, ASPCR1, 1) | VAL(forcesamplepresent, ASPCR1, 0) |
			VAL(write_en3, ASPCR1, 1) | VAL(b_frame, ASPCR1, 0x1) |
			VAL(write_en2, ASPCR1, 1) | VAL(forceb_frame, ASPCR1, 0) |
			VAL(write_en1, ASPCR1, 1) | VAL(audiolayout, ASPCR1, 0));

	HDMI_W32(HDMI_ACACR,
			VAL(write_en4, ACACR, 1) | VAL(hdmi_pk3map, ACACR, 3) |
			VAL(write_en3, ACACR, 1) | VAL(hdmi_pk2map, ACACR, 2) |
			VAL(write_en2, ACACR, 1) | VAL(hdmi_pk1map, ACACR, 1) |
			VAL(write_en1, ACACR, 1) | VAL(hdmi_pk0map, ACACR, 0));

	HDMI_W32(HDMI_ACRPCR,
			VAL(write_en4, ACRPCR, 0) | VAL(fs_div4, ACRPCR, 0) |
			VAL(write_en3, ACRPCR, 1) | VAL(disablecrp, ACRPCR, 0) |
			VAL(write_en2, ACRPCR, 0) | VAL(cts_source, ACRPCR, 0) |
			VAL(write_en1, ACRPCR, 0) | VAL(n_value, ACRPCR, 0));

	HDMI_W32(HDMI_ACRPCTSR, VAL(ctssw, ACRPCTSR, 74250));

	HDMI_W32(HDMI_ACRPCR,
			VAL(write_en4, ACRPCR, 1) | VAL(fs_div4, ACRPCR, 0) |
			VAL(write_en3, ACRPCR, 0) |
			VAL(disablecrp, ACRPCR, 0) | /*1: no CRP will be sent */
			VAL(write_en2, ACRPCR, 1) | VAL(cts_source, ACRPCR, CTS_Source) |
			VAL(write_en1, ACRPCR, 1) |
			VAL(n_value, ACRPCR, 4096)); /* 32k */

	HDMI_W32(HDMI_GCPCR,
		VAL(enablegcp, GCPCR, 0) | VAL(gcp_clearavmute, GCPCR, 1) |
		VAL(gcp_setavmute, GCPCR, 1) | VAL(write_data, GCPCR, 0));

	HDMI_W32(HDMI_ORP6PH, VAL(once_pkt, ORP6PH, 0));
	HDMI_W32(HDMI_ORP6W_0, VAL(once_pkt, ORP6W_0, 0));
	HDMI_W32(HDMI_ORP6W_1, VAL(once_pkt, ORP6W_1, 0));
	HDMI_W32(HDMI_ORP6W_2, VAL(once_pkt, ORP6W_2, 0));
	HDMI_W32(HDMI_ORP6W_3, VAL(once_pkt, ORP6W_3, 0));
	HDMI_W32(HDMI_ORP6W_4, VAL(once_pkt, ORP6W_4, 0));
	HDMI_W32(HDMI_ORP6W_5, VAL(once_pkt, ORP6W_5, 0));
	HDMI_W32(HDMI_ORP6W_6, VAL(once_pkt, ORP6W_6, 0));
	HDMI_W32(HDMI_ORP6W_7, VAL(once_pkt, ORP6W_7, 0));

	HDMI_W32(HDMI_RPCR,
			VAL(write_en6, RPCR, 1) | VAL(prp5period, RPCR, 0) |
			VAL(write_en5, RPCR, 1) | VAL(prp4period, RPCR, 0) |
			VAL(write_en4, RPCR, 1) | VAL(prp3period, RPCR, 0) |
			VAL(write_en3, RPCR, 0) | VAL(prp2period, RPCR, 0) |
			VAL(write_en2, RPCR, 1) | VAL(prp1period, RPCR, 0) |
			VAL(write_en1, RPCR, 0) | VAL(prp0period, RPCR, 0));

	HDMI_W32(HDMI_RPEN,
		VAL(enprpkt5, RPEN, 1) |
		VAL(enprpkt4, RPEN, 1) |
		VAL(enprpkt3, RPEN, 1) |
		VAL(enprpkt2, RPEN, 0) | /* Metadata package clear by HDMI_video_set */
		VAL(enprpkt1, RPEN, 1) | /* Audio InfoFrame package clear */
		VAL(enprpkt0, RPEN, 0) | /* AVI InfoFrame package clear by HDMI_video_set */
		VAL(write_data, RPEN, 0));

	HDMI_gen_audio_infoframe(AudioChannelCount);
}

static void sheipa_fb_enable_controller(struct fb_info *info)
{
	/* Enable HDMI */
	HDMI_W32(HDMI_CR, HDMI_R32(HDMI_CR) | MASK(enablehdmi, CR));
	/* Enable VDMA */
	VDMA_W32(VDMA_CTRL, VDMA_R32(VDMA_CTRL) | MASKD(encondition, CTRL));
	return;
}

static void sheipa_fb_disable_controller(struct fb_info *info)
{
	/* Disable HDMI */
	HDMI_W32(HDMI_CR, HDMI_R32(HDMI_CR) & ~MASK(enablehdmi, CR));
	/* Disable VDMA */
	VDMA_W32(VDMA_CTRL, VDMA_R32(VDMA_CTRL) & ~MASKD(encondition, CTRL));
	return;
}

static int sheipa_hdmi_setup(struct fb_info *info)
{
	sheipa_hdmi_vdma_setup(info);
	sheipa_hdmi_timming_setup(info);
	sheipa_hdmi_video_setup(info);

	HDMI_W32(HDMI_TMDS_STR0,
			VAL(tmds1ten, TMDS_STR0, 0) | VAL(td1src, TMDS_STR0, 0) |
			VAL(tmds1td, TMDS_STR0, 0x2aa) | VAL(tmds0ten, TMDS_STR0, 0) |
			VAL(td0src, TMDS_STR0, 0) | VAL(tmds0td, TMDS_STR0, 0x2aa));

	HDMI_W32(HDMI_TMDS_STR1,
			VAL(tmdscten, TMDS_STR1, 0) | VAL(tdcsrc, TMDS_STR1, 0) |
			VAL(tmdsctd, TMDS_STR1, 0x3e0) | VAL(tmds2ten, TMDS_STR1, 0) |
			VAL(td2src, TMDS_STR1, 0) | VAL(tmds2td, TMDS_STR1, 0x2aa));

	sheipa_hdmi_hdcp_off(info);
	sheipa_hdmi_audio_setup(info);

	HDMI_W32(HDMI_CRCCR,
			VAL(write_en2, CRCCR, 0) | VAL(crcmode, CRCCR, 0) |
			VAL(write_en1, CRCCR, 0) | VAL(crcchsel, CRCCR, 0));

	HDMI_W32(HDMI_CHNL_SEL, HDMI_R32(HDMI_CHNL_SEL) | MASK(timing_gen_en, CHNL_SEL));
	HDMI_W32(HDMI_DPC1,
			VAL(write_en2, DPC1, 1) | VAL(dp_vfch, DPC1, 1) |
			VAL(write_en1, DPC1, 0) | VAL(pp, DPC1, 0));

	HDMI_W32(HDMI_ICR,
			VAL(write_en3, ICR, 1) | VAL(enaudio, ICR, 1) |
			VAL(write_en2, ICR, 1) | VAL(envitd, ICR, 0) |
			VAL(write_en1, ICR, 0) | VAL(vitd, ICR, 0));

	return 0;
}

static int sheipa_fb_check_var(struct fb_var_screeninfo *var,
			    struct fb_info *info)
{
	return 0;
}

static int sheipa_fb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;
	struct fb_fix_screeninfo *fix = &info->fix;

	fix->accel = FB_ACCEL_NONE;
	fix->visual = FB_VISUAL_TRUECOLOR;
	fix->line_length = var->xres * var->bits_per_pixel / 8;

	return 0;
}

static int sheipa_fb_setcolreg(u32 color_index, u32 red, u32 green,
					u32 blue, u32 transp, struct fb_info *info)
{
	unsigned value;

	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		red >>= (16 - info->var.red.length);
		green >>= (16 - info->var.green.length);
		blue >>= (16 - info->var.blue.length);
		value = (red << info->var.red.offset);
		value |= (green << info->var.green.offset);
		value |= (blue << info->var.blue.offset);
		((u32 *) (info->pseudo_palette))[color_index] = value;
	}

	return 0;
}

static int sheipa_fb_fb_blank(int blank_mode, struct fb_info *info)
{
	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
	case FB_BLANK_NORMAL:
		sheipa_fb_enable_controller(info);
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
		break;
	case FB_BLANK_POWERDOWN:
		sheipa_fb_disable_controller(info);
		break;
	default:
		return -EINVAL;
	}

	return ((blank_mode == FB_BLANK_NORMAL) ? 1 : 0);
}

static int sheipa_fb_pan_display(struct fb_var_screeninfo *var,
			    struct fb_info *info)
{
	unsigned int yres = info->var.yres;

	if (info->var.yoffset == 0)
		info->var.yoffset = yres;
	else
		info->var.yoffset = 0;

	return 0;
}

static int sheipa_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct fb_ops sheipa_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = sheipa_fb_check_var,      /* Sanity check */
	.fb_set_par     = sheipa_fb_set_par,        /* Program controller registers */
	.fb_setcolreg   = sheipa_fb_setcolreg,      /* Set color map */
	.fb_blank       = sheipa_fb_fb_blank,       /* Blank/unblank display */
	.fb_pan_display = sheipa_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,             /* Generic function to fill rectangle */
	.fb_copyarea    = cfb_copyarea,             /* Generic function to copy area */
	.fb_imageblit   = cfb_imageblit,            /* Generic function to draw */
	.fb_ioctl       = sheipa_fb_ioctl,          /* Device-specific ioctl */
};

static struct sg_table *sheipa_map_dma_buf(struct dma_buf_attachment *attach,
                enum dma_data_direction direction)
{
	struct fb_info *pdata = attach->dmabuf->priv;
	unsigned long pfn = PFN_DOWN(pdata->fix.smem_start);
	struct page *page = pfn_to_page(pfn);
	struct sg_table *table;
	int nents, ret;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret < 0)
		goto err_alloc;

	sg_set_page(table->sgl, page, pdata->fix.smem_len, 0);

	nents = dma_map_sg(attach->dev, table->sgl, 1, direction);
	if (!nents) {
		ret = -EINVAL;
		goto err_map;
	}

	return table;

err_map:
	sg_free_table(table);
err_alloc:
	kfree(table);
	return ERR_PTR(ret);

}

static void sheipa_unmap_dma_buf(struct dma_buf_attachment *attach,
                struct sg_table *table, enum dma_data_direction direction)
{
	dma_unmap_sg(attach->dev, table->sgl, 1, direction);
	sg_free_table(table);
}

static void sheipa_release_dma_buf(struct dma_buf *buf)
{
	return;
}

static struct dma_buf_ops sheipa_dmabuf_ops = {
	.map_dma_buf    = sheipa_map_dma_buf,
	.unmap_dma_buf  = sheipa_unmap_dma_buf,
	.release        = sheipa_release_dma_buf,
	.kmap_atomic    = dma_buf_kmap_atomic,
	.kunmap_atomic  = dma_buf_kunmap_atomic,
	.kmap           = dma_buf_kmap,
	.kunmap         = dma_buf_kunmap,
	.mmap           = dma_buf_mmap,
};

static int sheipa_hdmi_parse_of_params(struct sheipa_hdmi_info *sinfo)
{
	struct fb_info *info = sinfo->info;
	struct fb_var_screeninfo *var = &info->var;
	struct device *dev = sinfo->dev;
	struct device_node *np = dev->of_node;
	int ret;

	if (!np) {
		dev_err(dev, "missing DT data");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "sheipa,hdmi-res-x", &var->xres);
	if (ret < 0) {
		dev_err(dev, "failed to get property xres\n");
		return ret;
	}

	ret = of_property_read_u32(np, "sheipa,hdmi-res-y", &var->yres);
	if (ret < 0) {
		dev_err(dev, "failed to get property bits-yres\n");
		return ret;
	}

	ret = of_property_read_u32(np, "sheipa,hdmi-bpp", &var->bits_per_pixel);
	if (ret < 0) {
		dev_err(dev, "failed to get property bits-per-pixel\n");
		return ret;
	}

	return 0;
}

static int sheipa_hdmi_init_fbinfo(struct device *dev,
			    struct sheipa_hdmi_info *sinfo)
{
	struct sheipa_hdmi_pdata *pdata = dev_get_platdata(dev);
	struct fb_info *info = sinfo->info;
	int ret;

	if (pdata) {
		info->var.xres = pdata->xres;
		info->var.yres = pdata->yres;
		info->var.bits_per_pixel = pdata->bpp;
	} else {
		ret = sheipa_hdmi_parse_of_params(sinfo);
		if (ret)
			return ret;
	}

	info->var.xres_virtual = info->var.xres;
	info->var.yres_virtual = info->var.yres * 2;
	info->var.xoffset = 0;
	info->var.yoffset = 0;
	info->fix.ypanstep = 1;
	info->var.hsync_len = 61;
	info->var.left_margin = 15;
	info->var.right_margin = 59;
	info->var.vsync_len = 0;
	info->var.upper_margin = 0;
	info->var.lower_margin = 0;
	info->var.red.offset = 24;
	info->var.red.length = 8;
	info->var.green.offset = 16;
	info->var.green.length = 8;
	info->var.blue.offset = 8;
	info->var.blue.length = 8;
	info->var.transp.offset = 0;
	info->var.transp.length = 8;

	info->var.vmode = 0;

	info->pseudo_palette = devm_kzalloc(dev, (sizeof(unsigned int) * 16),
				    GFP_KERNEL);
	info->fix.smem_len = info->var.xres_virtual * info->var.yres_virtual * 4;

	/* Allocate colormap */
	ret = fb_alloc_cmap(&info->cmap, 24, 0);
	if (ret < 0)
		dev_err(dev, "Alloc color map failed\n");

	return ret;
}

static inline void sheipa_hdmi_free_video_memory(struct sheipa_hdmi_info *sinfo)
{
	struct fb_info *info = sinfo->info;

	dma_free_coherent(info->device, info->fix.smem_len,
				info->screen_base, info->fix.smem_start);
}

static int sheipa_hdmi_alloc_video_memory(struct sheipa_hdmi_info *sinfo)
{
	struct fb_info *info = sinfo->info;

	info->screen_base = dma_alloc_coherent(info->device, info->fix.smem_len,
				    (dma_addr_t *)&info->fix.smem_start,
				    GFP_KERNEL | GFP_DMA);

	if (!info->screen_base) {
		return -ENOMEM;
	}

	memset(info->screen_base, 0x0, info->fix.smem_len);

	return 0;
}

static irqreturn_t sheipa_hdmi_vsync_isr(int irq, void *dev_instance)
{
	unsigned int hdmi_intstv, fbuf_addr;
	struct fb_info *info = dev_instance;

	hdmi_intstv = HDMI_R32(HDMI_INTSTV);
	if (!(hdmi_intstv & MASK(vendupdated, INTSTV)))
		return IRQ_RETVAL(0);

	fbuf_addr = VDMA_R32(VDMA_FBUF_ADDR);
	if (info->var.yoffset == 0)
		VDMA_W32(VDMA_FBUF_ADDR, VALD(addr, FBUF_ADDR, (info->fix.smem_start)));
	else
		VDMA_W32(VDMA_FBUF_ADDR, VALD(addr, FBUF_ADDR, info->fix.smem_start + info->var.xres * info->var.yres * 4));

	VDMA_W32(VDMA_CTRL, VALD(vdma_en, CTRL, 0x1));
	HDMI_W32(HDMI_INTSTV, VAL(vendupdated, INTSTV, 0));

	return IRQ_RETVAL(1);
}

static int sheipa_hdmi_probe(struct platform_device *pdev)
{
	struct fb_info *info;
	struct sheipa_hdmi_info *sinfo;
	struct resource *res;
	int ret;

	info = framebuffer_alloc(sizeof(struct sheipa_hdmi_info), &pdev->dev);
	if (!info) {
		dev_err(&pdev->dev, "cannot allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	sinfo = info->par;
	sinfo->info = info;
	sinfo->dev = &pdev->dev;

	INIT_LIST_HEAD(&info->modelist);
	info->fbops = &sheipa_fb_ops;

	/* irq resource */
	sinfo->irq_base = platform_get_irq(pdev, 0);
	if (sinfo->irq_base < 0) {
		dev_err(&pdev->dev, "failed to get hdmi IRQ\n");
		ret = -ENXIO;
		goto free_info;
	}

	/* hdmi regs resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "resources 0 unusable\n");
		ret = -ENXIO;
		goto free_info;
	}

	info->fix.mmio_start = res->start;
	info->fix.mmio_len = resource_size(res);
	if (!!!request_mem_region(info->fix.mmio_start,
				info->fix.mmio_len, pdev->name)) {
		ret = -EBUSY;
		goto free_info;
	}

	hdmi_base = ioremap_nocache(info->fix.mmio_start, info->fix.mmio_len);
	if (!hdmi_base) {
		dev_err(&pdev->dev, "cannot map HDMI registers\n");
		ret = -ENOMEM;
		goto release_mem_hdmi;
	}

	sinfo->hdmi_base = (unsigned long)hdmi_base;
	sinfo->hdmi_size = resource_size(res);

	/* vdma memory resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(&pdev->dev, "resources 1 unusable\n");
		ret = -ENXIO;
		goto unmap_hdmi;
	}

	if (!!!request_mem_region(res->start,
				resource_size(res), pdev->name)) {
		ret = -EBUSY;
		goto unmap_hdmi;
	}

	vdma_base = ioremap_nocache(res->start, resource_size(res));
	if (!vdma_base) {
		ret = -ENOMEM;
		goto release_mem_vdma;
	}

	sinfo->vdma_base = (unsigned long)vdma_base;
	sinfo->vdma_size = resource_size(res);

	ret = devm_request_irq(&pdev->dev, sinfo->irq_base, sheipa_hdmi_vsync_isr,
			    IRQF_TRIGGER_RISING, pdev->name, info);
	if (ret) {
		dev_err(&pdev->dev, "failed to request keyboard IRQ\n");
		goto unmap_vdma;
	}

	ret = sheipa_hdmi_init_fbinfo(&pdev->dev, sinfo);
	if (ret) {
		dev_err(&pdev->dev, "failed to init fb_info\n");
		goto unmap_vdma;
	}

	/* allocate dma buffer */
	ret = sheipa_hdmi_alloc_video_memory(sinfo);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot allocate framebuffer: %d\n", ret);
		goto unmap_vdma;
	}

	sheipa_hdmi_setup(info);

	ret = sheipa_fb_set_par(info);
	if (ret < 0) {
		dev_err(&pdev->dev, "set par failed: %d\n", ret);
		goto release_dma_buf;
	}

	dev_set_drvdata(&pdev->dev, sinfo);

	/* Tell the world that we're ready to go */
	ret = register_framebuffer(info);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register framebuffer device: %d\n", ret);
		goto reset_drvdata;
	}

	dev_info(&pdev->dev, "fb%d: Sheipa HDMI at 0x%08lx (mapped at %p), irq %d\n",
		       info->node, info->fix.mmio_start, hdmi_base, sinfo->irq_base);

	return 0;

reset_drvdata:
	dev_set_drvdata(&pdev->dev, NULL);
	fb_dealloc_cmap(&info->cmap);
release_dma_buf:
	sheipa_hdmi_free_video_memory(sinfo);
unmap_vdma:
	iounmap(vdma_base);
release_mem_vdma:
	release_mem_region(sinfo->vdma_base, sinfo->vdma_size);
unmap_hdmi:
	iounmap(hdmi_base);
release_mem_hdmi:
	release_mem_region(sinfo->hdmi_base, sinfo->hdmi_size);
free_info:
	framebuffer_release(info);
out:
	return ret;
}

static int sheipa_hdmi_remove(struct platform_device *pdev)
{
	struct sheipa_hdmi_info *sinfo = platform_get_drvdata(pdev);
	struct fb_info *info = sinfo->info;

	/* Disable screen refresh, turn off DMA */
	sheipa_fb_disable_controller(info);
	/* Unregister frame buffer driver */
	unregister_framebuffer(info);
	/* Deallocate color map */
	fb_dealloc_cmap(&info->cmap);
	/* Release memory region */
	iounmap(hdmi_base);
	release_mem_region(sinfo->hdmi_base, sinfo->hdmi_size);
	iounmap(vdma_base);
	release_mem_region(sinfo->vdma_base, sinfo->vdma_size);
	sheipa_hdmi_free_video_memory(sinfo);
	/* Reset driver data */
	platform_set_drvdata(pdev, NULL);
	/* Reverse of framebuffer_alloc() */
	framebuffer_release(info);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id sheipa_hdmi_dt_match_table[] = {
	{ .compatible = "realtek,sheipa-hdmi" },
	{}
};
MODULE_DEVICE_TABLE(of, sheipa_hdmi_dt_match_table);
#endif

static struct platform_driver sheipa_hdmi_driver = {
	.probe		= sheipa_hdmi_probe,
	.remove		= sheipa_hdmi_remove,
	.driver		= {
		.name	= "sheipa-hdmi",
		.of_match_table	= of_match_ptr(sheipa_hdmi_dt_match_table),
	},
};
module_platform_driver(sheipa_hdmi_driver);

MODULE_AUTHOR("Realtek PSP software");
MODULE_LICENSE("GPL");
