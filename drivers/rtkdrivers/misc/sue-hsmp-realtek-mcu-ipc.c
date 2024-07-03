// SPDX-License-Identifier: GPL-2.0-only
/*
 * SUE HSMP driver using Realtek IPC
 *
 * Copyright (C) 2024, StreamUnlimited GmbH. All rights reserved.
 */

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#include <ameba_ipc/ameba_ipc.h>
#include <misc/sue-hsmp-common.h>

// This is the max packet size for our IPC layer, for HSMP we technically
// would only need `sizeof(struct sue_hsmp_packet)` but we **could** also
// multiplex HostLink (`packet_start` = 0xFEFF) over this protocol in the
// future, which would transfer more data.
#define SUE_MCU_IPC_MAX_PACKET_SIZE (1 << 16)


struct sue_hsmp_rtk_mcu_ipc {
	struct device		*dev;

	struct aipc_ch		*ipc_ch;
	ipc_msg_struct_t	ipc_msg;
	struct completion	ipc_done;

	uint8_t			*data_buffer;
	dma_addr_t		data_buffer_paddr;

	struct regmap		*regmap;
};


static u32 sue_hsmp_rtk_mcu_ipc_channel_recv(struct aipc_ch *ch, ipc_msg_struct_t *pmsg)
{
	struct sue_hsmp_rtk_mcu_ipc *ipc = ch->priv_data;

	memcpy(&ipc->ipc_msg, pmsg, sizeof(ipc_msg_struct_t));

	return 0;
}

static void sue_hsmp_rtk_mcu_ipc_channel_empty(struct aipc_ch *ch)
{
	struct sue_hsmp_rtk_mcu_ipc *ipc = ch->priv_data;
	complete_all(&ipc->ipc_done);
}

static struct aipc_ch_ops sue_hsmp_rtk_mcu_ipc_ch_ops = {
	.channel_recv = sue_hsmp_rtk_mcu_ipc_channel_recv,
	.channel_empty_indicate = sue_hsmp_rtk_mcu_ipc_channel_empty,
};

static int sue_rtk_ipc_send(struct sue_hsmp_rtk_mcu_ipc *ipc, char *data, size_t len)
{
	if (len > SUE_MCU_IPC_MAX_PACKET_SIZE)
		return -EINVAL;

	if (!completion_done(&ipc->ipc_done))
		return -EBUSY;

	reinit_completion(&ipc->ipc_done);

	memcpy(ipc->data_buffer, data, len);
	memset(&ipc->ipc_msg, 0, sizeof(ipc_msg_struct_t));
	ipc->ipc_msg.msg = (u32)ipc->data_buffer_paddr;
	ipc->ipc_msg.msg_type = 0; // 0 = IPC_USER_POINT
	ipc->ipc_msg.msg_len = len;

	return ameba_ipc_channel_send(ipc->ipc_ch, &ipc->ipc_msg);
}

static int sue_rtk_ipc_hsmp_send_and_recv(struct sue_hsmp_rtk_mcu_ipc *ipc, uint8_t command_type, uint8_t reg_addr, uint32_t *value, uint32_t mask)
{
	int ret;
	unsigned long timeleft;
	struct sue_hsmp_packet p;

	memset(&p, 0, sizeof(p));
	p.packet_start_lsb = HSMP_PACKET_START_HEADER & 0xFF;
	p.packet_start_msb = (HSMP_PACKET_START_HEADER >> 8) & 0xFF;
	p.packet_length_lsb = sizeof(struct sue_hsmp_packet) & 0xFF;
	p.packet_length_msb = (sizeof(struct sue_hsmp_packet) >> 8) & 0xFF;

	p.command_type = command_type;
	p.register_addr = reg_addr;
	p.value = *value;
	p.mask = mask;

	ret = sue_rtk_ipc_send(ipc, (uint8_t *)&p, sizeof(p));
	if (ret != 0) {
		dev_err(ipc->dev, "IPS send failed: %d\n", ret);
		return -EINVAL;
	}

	timeleft = wait_for_completion_timeout(&ipc->ipc_done, msecs_to_jiffies(10));
	if (!timeleft) {
		dev_err(ipc->dev, "IPC response timed out\n");
		return -ETIMEDOUT;
	}

	if (command_type == HSMP_COMMAND_READ)
		*value = ((struct sue_hsmp_packet *)ipc->data_buffer)->value;

	dev_dbg(ipc->dev, "command_type: %d, reg_addr: %d, value: 0x%x, mask: 0x%x\n", command_type, reg_addr, *value, mask);
	return 0;
}

static bool sue_rtk_ipc_hsmp_regmap_readable(struct device *dev, unsigned int reg)
{
	return true;
}

static bool sue_rtk_ipc_hsmp_regmap_writeable(struct device *dev, unsigned int reg)
{
	return true;
}

static struct regmap_config sue_rtk_ipc_hsmp_regmap_config = {
	.name = "sue_rtk_ipc_hsmp",
	.reg_bits = 8,
	.val_bits = 32,
	.max_register = HSMP_MAX_REG,
	.cache_type = REGCACHE_RBTREE,
	.readable_reg = sue_rtk_ipc_hsmp_regmap_readable,
	.writeable_reg = sue_rtk_ipc_hsmp_regmap_writeable,
};

static int regmap_sue_rtk_ipc_hsmp_write(void *context, unsigned int reg, unsigned int val)
{
	return sue_rtk_ipc_hsmp_send_and_recv((struct sue_hsmp_rtk_mcu_ipc *)context, HSMP_COMMAND_WRITE, reg, &val, 0xFFFFFFFF);
}

static int regmap_sue_rtk_ipc_hsmp_read(void *context, unsigned int reg, unsigned int *val)
{
	return sue_rtk_ipc_hsmp_send_and_recv((struct sue_hsmp_rtk_mcu_ipc *)context, HSMP_COMMAND_READ, reg, val, 0xFFFFFFFF);
}

static int regmap_sue_rtk_ipc_hsmp_reg_update_bits(void *context, unsigned int reg, unsigned int mask, unsigned int val)
{
	return sue_rtk_ipc_hsmp_send_and_recv((struct sue_hsmp_rtk_mcu_ipc *)context, HSMP_COMMAND_UPDATE_BITS, reg, &val, mask);
}

static struct regmap_bus sue_rtk_ipc_hsmp_regmap = {
	.reg_write = regmap_sue_rtk_ipc_hsmp_write,
	.reg_read = regmap_sue_rtk_ipc_hsmp_read,
	.reg_update_bits = regmap_sue_rtk_ipc_hsmp_reg_update_bits,
};

static int sue_hsmp_rtk_mcu_ipc_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct sue_hsmp_rtk_mcu_ipc *ipc;

	dev_info(&pdev->dev, "loading StreamUnlimited HSMP over Realtek IPC\n");

	ipc = devm_kzalloc(&pdev->dev, sizeof(*ipc), GFP_KERNEL);
	if (!ipc) {
		ret = -ENOMEM;
		goto out;
	}

	ipc->dev = &pdev->dev;
	platform_set_drvdata(pdev, ipc);

	ipc->ipc_ch = ameba_ipc_alloc_ch(sizeof(struct sue_hsmp_rtk_mcu_ipc *));
	if (!ipc->ipc_ch) {
		ret = -ENOMEM;
		dev_err(ipc->dev, "Failed to allocate IPC channel\n");
		goto out;
	}

	ipc->ipc_ch->port_id = AIPC_PORT_LP;
	ipc->ipc_ch->ch_id = 3;	// 3 = IPC_A2L_SUE_MCU_IPC
	ipc->ipc_ch->ch_config = AIPC_CONFIG_HANDSAKKE;
	ipc->ipc_ch->ops = &sue_hsmp_rtk_mcu_ipc_ch_ops;
	ipc->ipc_ch->priv_data = ipc;

	ret = ameba_ipc_channel_register(ipc->ipc_ch);
	if (ret < 0) {
		dev_err(ipc->dev, "Failed to register IPC channel\n");
		goto free_ipc_ch;
	}

	if (!ipc->ipc_ch->pdev) {
		dev_err(ipc->dev, "No device in registered IPC channel\n");
		goto unregist_ch;
	}

	ipc->data_buffer = dmam_alloc_coherent(ipc->dev, SUE_MCU_IPC_MAX_PACKET_SIZE, &ipc->data_buffer_paddr, GFP_KERNEL);
	if (!ipc->data_buffer) {
		dev_err(ipc->dev, "Failed to allocated DMA buffer for IPC\n");
		goto unregist_ch;
	}

	init_completion(&ipc->ipc_done);
	complete_all(&ipc->ipc_done);

	ipc->regmap = devm_regmap_init(ipc->dev, &sue_rtk_ipc_hsmp_regmap, ipc, &sue_rtk_ipc_hsmp_regmap_config);
	if (IS_ERR(ipc->regmap)) {
		dev_err(ipc->dev, "Failed to initialize regmap\n");
		ret = PTR_ERR(ipc->regmap);
		goto unregist_ch;
	}

	ret  = devm_of_platform_populate(ipc->dev);
	if (ret != 0) {
		dev_err(ipc->dev, "Failed to populate devices\n");
		goto unregist_ch;
	}

	goto out;

unregist_ch:
	ameba_ipc_channel_unregister(ipc->ipc_ch);

free_ipc_ch:
	kfree(ipc->ipc_ch);

out:
	return ret;
}

static const struct of_device_id sue_hsmp_rtk_mcu_ipc_of_match[] = {
	{ .compatible = "sue,hsmp-rtk-mcu-ipc",	},
	{ },
};

static struct platform_driver sue_hsmp_rtk_mcu_ipc_driver = {
	.probe	= sue_hsmp_rtk_mcu_ipc_probe,
	.driver	= {
		.name = "sue-hsmp-rtk-mcu-ipc",
		.of_match_table = sue_hsmp_rtk_mcu_ipc_of_match,
	},
};

builtin_platform_driver(sue_hsmp_rtk_mcu_ipc_driver);

MODULE_DESCRIPTION("SUE HSMP driver using Realtek IPC");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Peter Suti <peter.suti@streamunlimited.com>");
