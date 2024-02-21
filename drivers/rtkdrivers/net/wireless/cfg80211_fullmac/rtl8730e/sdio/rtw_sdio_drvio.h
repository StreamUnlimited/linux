#ifndef __RTW_SDIO_DRVIO_H__
#define __RTW_SDIO_DRVIO_H__

s32 sd_cmd52_read(inic_sdio_t *priv, u32 addr, u32 cnt, u8 *pdata);
s32 sd_cmd52_write(inic_sdio_t *priv, u32 addr, u32 cnt, u8 *pdata);
u8 sd_read8(inic_sdio_t *priv, u32 addr, s32 *err);
u8 sd_f0_read8(inic_sdio_t *priv, u32 addr, s32 *err);
void sd_f0_write8(inic_sdio_t *priv, u32 addr, u8 v, s32 *err);
u16 sd_read16(inic_sdio_t *priv, u32 addr, s32 *err);
u32 sd_read32(inic_sdio_t *priv, u32 addr, s32 *err);
void sd_write8(inic_sdio_t *priv, u32 addr, u8 v, s32 *err);
void sd_write16(inic_sdio_t *priv, u32 addr, u16 v, s32 *err);
void sd_write32(inic_sdio_t *priv, u32 addr, u32 v, s32 *err);
s32 sd_read(inic_sdio_t *priv, u32 addr, u32 cnt, void *pdata);
s32 sd_write(inic_sdio_t *priv, u32 addr, u32 cnt, void *pdata);
#endif

