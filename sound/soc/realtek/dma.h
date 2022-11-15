/*
 * Copyright (c) 2021 Realtek, LLC.
 * All rights reserved.
 *
 * Licensed under the Realtek License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License from Realtek
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SND_SOC_REALTEK_GDMA_H_
#define __SND_SOC_REALTEK_GDMA_H_

#define MAX_IDMA_PERIOD (128 * 1024)
#define MAX_IDMA_BUFFER (160 * 1024)
#define DMA_BURST_AGGREGATION 16

//dma fifo infos.
struct ameba_pcm_dma_params {
	dma_addr_t dev_phys_0;
	dma_addr_t dev_phys_1;
	u32 datawidth;
	u32 src_maxburst;
	u32 handshaking_0;
	u32 handshaking_1;
	const char *chan_name;
	//means the total counter calculated by irq interrupt.
	//however it's not the real current time, need to add the delta
	//counter value from last interrupt->now.
	u64 total_sport_counter;
	//means the current substream's sport's base addr.
	void __iomem * sport_base_addr;
};

struct dma_callback_params {
	struct snd_pcm_substream *substream;
	unsigned int dma_id;
};

void ameba_alsa_callback_handle(struct dma_callback_params *params);

#endif
