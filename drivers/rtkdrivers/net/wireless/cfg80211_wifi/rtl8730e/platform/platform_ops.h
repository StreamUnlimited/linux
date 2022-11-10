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
#ifndef __PLATFORM_OPS_H__
#define __PLATFORM_OPS_H__

/*
 * Return:
 *0:	power on successfully
 *others: power on failed
 */

#ifdef CONFIG_PLATFORM_OPS
int platform_wifi_power_on(void);
void platform_wifi_power_off(void);
#if defined(CONFIG_PCI_HCI)
void pci_cache_wback(struct pci_dev *hwdev,
		     dma_addr_t *bus_addr, size_t size, int direction);
void pci_cache_inv(struct pci_dev *hwdev,
		   dma_addr_t *bus_addr, size_t size, int direction);
void pci_get_bus_addr(struct pci_dev *hwdev,
		      void *vir_addr, dma_addr_t *bus_addr, size_t size, int direction);
void pci_unmap_bus_addr(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction);
void *pci_alloc_cache_mem(struct pci_dev *pdev,
			  dma_addr_t *bus_addr, size_t size, int direction);
void *pci_alloc_noncache_mem(struct pci_dev *pdev,
			     dma_addr_t *bus_addr, size_t size);
void pci_free_cache_mem(struct pci_dev *pdev, void *vir_addr,
			dma_addr_t *bus_addr, size_t size, int direction);
void pci_free_noncache_mem(struct pci_dev *pdev, void *vir_addr,
			   dma_addr_t *bus_addr, size_t size);
#elif defined(CONFIG_AXI_HCI)
void axi_cache_wback(struct device *dev,
		     dma_addr_t *bus_addr, size_t size, int direction);
void axi_cache_inv(struct device *dev,
		   dma_addr_t *bus_addr, size_t size, int direction);
void axi_get_bus_addr(struct device *dev,
		      void *vir_addr, dma_addr_t *bus_addr, size_t size, int direction);
void axi_unmap_bus_addr(struct device *dev,
			dma_addr_t *bus_addr, size_t size, int direction);
void *axi_alloc_cache_mem(struct device *dev,
			  dma_addr_t *bus_addr, size_t size, int direction);
void *axi_alloc_noncache_mem(struct device *dev,
			     dma_addr_t *bus_addr, size_t size);
void axi_free_cache_mem(struct device *dev, void *vir_addr,
			dma_addr_t *bus_addr, size_t size, int direction);
void axi_free_noncache_mem(struct device *dev, void *vir_addr,
			   dma_addr_t *bus_addr, size_t size);
#endif /* CONFIG_AXI_HCI */
#else
#define platform_wifi_power_on(void) 0
#define platform_wifi_power_off(void)
#endif /* CONFIG_PLATFORM_OPS */
#endif /* __PLATFORM_OPS_H__ */