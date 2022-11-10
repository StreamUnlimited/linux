/*
 * Realtek Semiconductor Corp.
 *
 * bsp/include/mach/io.h:
 *
 * Copyright 2012  Tony Wu (tonywu@realtek.com)
 */

#ifndef _IO_H_
#define _IO_H_

#include <mach/hardware.h>

#define IO_SPACE_LIMIT 0xffffffff

/*
 * We don't actually have real ISA nor PCI buses, but there is so many
 * drivers out there that might just work if we fake them...
 */
#define __io(a)		__typesafe_io(a)
#define __mem_pci(a)	(a)

/*
 * ----------------------------------------------------------------------------
 * I/O mapping
 * ----------------------------------------------------------------------------
 */

#ifdef __ASSEMBLER__
#define IOMEM(x)		(x)
#else
#define IOMEM(x)		((void __force __iomem *)(x))
#endif

#endif /* _IO_H_ */
