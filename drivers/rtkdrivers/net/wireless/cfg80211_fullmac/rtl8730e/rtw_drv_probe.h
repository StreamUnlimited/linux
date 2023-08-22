#ifndef __RTW_DRV_PROBE_H__
#define __RTW_DRV_PROBE_H__

typedef struct axi_data {
	struct platform_device	*pdev;

	/* AXI MEM map */
	unsigned long		axi_mem_end; /* shared mem end */
	unsigned long		axi_mem_start; /* shared mem start */

	/* AXI MEM map */
	unsigned long		axi_sys_mem_end; /* shared mem end */
	unsigned long		axi_sys_mem_start; /* shared mem start */

	u8			bdma64;
} AXI_DATA, *PAXI_DATA;

struct axi_drv_priv {
	struct platform_driver	rtw_axi_drv;
	int			drv_registered;
};

extern PAXI_DATA paxi_data_global;
#endif // __RTW_DRV_PROBE_H__