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
#define _OSDEP_SERVICE_LINUX_C_
#include <rtw_drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>

void _rtw_memcpy(void *dst, const void *src, u32 sz)
{
	memcpy(dst, src, sz);
}

inline void _rtw_memmove(void *dst, const void *src, u32 sz)
{
	memmove(dst, src, sz);
}

int _rtw_memcmp(const void *dst, const void *src, u32 sz)
{
	/* under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0 */
	if (!(memcmp(dst, src, sz))) {
		return _TRUE;
	} else {
		return _FALSE;
	}
}

void _rtw_memset(void *pbuf, int c, u32 sz)
{
	memset(pbuf, c, sz);
}

