realtek-fwlib-sport-objs := ameba_sport.o

snd-soc-realtek-i2s-objs := ameba_audio_clock.o ameba_audio_clock_driver.o sport.o
snd-soc-realtek-dma-objs := dma.o
snd-soc-realtek-machine-objs := machine.o

obj-$(CONFIG_SND_SOC_SPORT_RTK_AMEBA) += realtek-fwlib-sport.o
obj-$(CONFIG_SND_SOC_SPORT_RTK_AMEBA) += snd-soc-realtek-i2s.o
obj-$(CONFIG_SND_SOC_DMA_RTK_AMEBA) += snd-soc-realtek-dma.o
obj-$(CONFIG_SND_SOC_RTK_AMEBA) += snd-soc-realtek-machine.o
obj-$(CONFIG_SND_SOC_RTK_CAPTURE_TIMER) += ameba-captimer.o

