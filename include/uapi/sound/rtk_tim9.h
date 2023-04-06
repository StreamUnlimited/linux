#ifndef __SND_SOC_REALTEK_TIM9_H_
#define __SND_SOC_REALTEK_TIM9_H_

#include <linux/types.h>

#define I2SPLL2_24P576M_DIV2			0
#define I2SPLL2_45P158M_DIV2			1
#define I2SPLL2_98P304M_DIV2			2

#define I2SPLL2_AUTO			0
#define I2SPLL2_FASTER			1
#define I2SPLL2_SLOWER			2


#define TIM9_I2SPLL2    't'
#define TIM9_I2SPLL2_ADJUST           _IOW(TIM9_I2SPLL2, 0, struct i2spll2_micro_adjust_params)
#define TIM9_I2SPLL2_READ_COUNT       _IOR(TIM9_I2SPLL2, 1, unsigned long long)


struct i2spll2_micro_adjust_params {
	/*can be I2SPLL2_24P576M_DIV2, I2SPLL2_45P158M_DIV2, or I2SPLL2_98P304M_DIV2*/
	unsigned int clock;
	/*can be #define I2SPLL2_AUTO, I2SPLL2_FASTER, or I2SPLL2_SLOWER*/
	unsigned int action;
	/*can be any value between 0-1000*/
	unsigned int ppm;
};


#endif
