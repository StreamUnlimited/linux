// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/sizes.h>
#include "../../internals.h"
#include "aml_mtd_nand.h"

struct nand_flash_dev aml_nand_flash_ids[] = {
	{"TC58NVG2S0H 4G 3.3V 8-bit",
		{ .id = {0x98, 0xdc, 0x90, 0x26, 0x76, 0x16, 0x08, 0x00} },
		  SZ_4K, SZ_512, SZ_256K, 0, 8, 256, NAND_ECC_INFO(8, SZ_512) },

	{NULL,}
};
