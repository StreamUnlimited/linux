/******************************************************************************
 *
 * Copyright(c) 2019 -  Realtek Corporation.
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
#include "drv_types.h"

#ifdef CONFIG_AXI_HCI
void axi_cache_wback(struct device *dev,
		     dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != dev && NULL != bus_addr)
		dma_sync_single_range_for_device(dev, *bus_addr, 0, size,
						 direction);
	else {
		RTW_ERR("dev handle or bus addr is NULL!\n");
	}
}
void axi_cache_inv(struct device *dev,
		   dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != dev && NULL != bus_addr) {
		dma_sync_single_range_for_cpu(dev, *bus_addr, 0, size, direction);
	} else {
		RTW_ERR("dev handle or bus addr is NULL!\n");
	}
}
void axi_get_bus_addr(struct device *dev,
		      void *vir_addr, dma_addr_t *bus_addr,
		      size_t size, int direction)
{
	if (NULL != dev) {
		*bus_addr = dma_map_single(dev, vir_addr, size, direction);
	} else {
		RTW_ERR("dev handle is NULL!\n");
		*bus_addr = (dma_addr_t)virt_to_phys(vir_addr);
		/*RTW_ERR("Get bus_addr: %x by virt_to_phys()\n", bus_addr);*/
	}
}
void axi_unmap_bus_addr(struct device *dev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != dev && NULL != bus_addr) {
		dma_unmap_single(dev, *bus_addr, size, direction);
	} else {
		RTW_ERR("dev handle or bus addr is NULL!\n");
	}
}
void *axi_alloc_cache_mem(struct device *dev,
			  dma_addr_t *bus_addr, size_t size, int direction)
{
	void *vir_addr = NULL;

	vir_addr = rtw_zmalloc(size);

	if (!vir_addr) {
		bus_addr = NULL;
	} else {
		axi_get_bus_addr(dev, vir_addr, bus_addr, size, direction);
	}

	return vir_addr;
}
void *axi_alloc_noncache_mem(struct device *dev,
			     dma_addr_t *bus_addr, size_t size)
{
	void *vir_addr = NULL;

	if (NULL != dev)
		vir_addr = dma_alloc_coherent(dev,
					      size, bus_addr,
					      (in_atomic() ? GFP_ATOMIC : GFP_KERNEL));
	if (!vir_addr) {
		bus_addr = NULL;
	}

	return vir_addr;
}
void axi_free_cache_mem(struct device *dev,
			void *vir_addr, dma_addr_t *bus_addr,
			size_t size, int direction)
{
	axi_unmap_bus_addr(dev, bus_addr, size, direction);
	rtw_mfree(vir_addr, size);

	vir_addr = NULL;
}
void axi_free_noncache_mem(struct device *dev,
			   void *vir_addr, dma_addr_t *bus_addr, size_t size)
{
	if (NULL != dev) {
		dma_free_coherent(dev, size, vir_addr, *bus_addr);
	}
	vir_addr = NULL;
}
#endif /* CONFIG_AXI_HCI */
