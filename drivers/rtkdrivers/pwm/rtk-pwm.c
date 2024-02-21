#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/swiotlb.h>
#include <linux/of.h>

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/time.h>

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

#include <linux/mfd/rtk-timer.h>


#define PWM_DISABLE		0
#define PWM_ENABLE		1


#define AMEBA_PWM_CHAN_NUM		6

struct rtk_pwm {
	struct pwm_chip chip;
	struct clk * pwm_clk;
	u32 clk;
	void __iomem *base;
	struct mutex lock; 			/*protect pwm for all channel */
	int chan_en[AMEBA_PWM_CHAN_NUM];  /*is channel enable*/
};


static struct rtk_pwm *to_rtk_pwm_dev(struct pwm_chip *chip)
{
	return container_of(chip, struct rtk_pwm, chip);
}


static int active_channels(struct pwm_chip *chip)
{
	int i;
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);

	for(i = 0; i < AMEBA_PWM_CHAN_NUM; i++) {
		if(pwm->chan_en[i] == 1)
			return -1;
	}

	return 0;
}



/*start or stop timer.(counter will not reset to 0)*/
static void rtk_pwm_timer_start_count(struct pwm_chip *chip, u32 NewState)
{
	u32 reg;
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);
	void __iomem *base = pwm->base;

	if (NewState != PWM_DISABLE) {
		/* Enable the TIM Counter, dont do this if timer is RUN */
		reg = readl(base + REG_TIM_EN);
		if ((readl(base + REG_TIM_EN) & TIM_BIT_CEN) == 0) {
			writel(TIM_BIT_CNT_START, base + REG_TIM_EN);
		}

		/* poll if cnt is running, 3*32k cycles */
		while (1) {
			mdelay(200);
			if (readl(base + REG_TIM_EN) & TIM_BIT_CEN)
				break;
		}
	} else {
		/* Disable the TIM Counter, dont do this if timer is not RUN */
		/* this action need sync to 32k domain for 100us */
		if (readl(base + REG_TIM_EN) & TIM_BIT_CEN) {
			writel(TIM_BIT_CNT_STOP, base + REG_TIM_EN);
		}

		/* poll if cnt is running, aout 100us */
		while (1) {
			if ((readl(base + REG_TIM_EN) & TIM_BIT_CEN) == 0)
				break;
		}
	}
}



static void rtk_pwm_polarity_config(struct pwm_chip *chip, u32 polarity, u32 ch)
{
	u32 reg;
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);
	void __iomem *base = pwm->base;

	reg = readl(base + REG_TIM_CCR0 + 4*ch);
	reg &= ~TIM_BIT_CCxP;
	if(polarity != 0)
		reg |= TIM_CCPolarity_High;
	else
		reg |= TIM_CCPolarity_Low;
	writel(reg, base + REG_TIM_CCR0 + 4*ch);
}

static void rtk_pwm_disable(struct pwm_chip *chip, int ch)
{
	u32 reg;
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);
	void __iomem *base = pwm->base;

	/*Set flag*/
	pwm->chan_en[ch] = 0;
	/* Disable channel */
	reg = readl(base + REG_TIM_CCR0 + 4*ch);
	reg &= ~TIM_BIT_CCxE;
	reg |= TIM_CCx_Disable;
	writel(reg, base + REG_TIM_CCR0 + 4*ch);

	/* When all channels are disabled, we can disable the controller */
	if (!active_channels(chip))
		rtk_pwm_timer_start_count(chip, PWM_DISABLE);
}



static int rtk_pwm_enable(struct pwm_chip *chip, int ch)
{
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);
	void __iomem *base = pwm->base;
	u32 reg;

	/*Set flag*/
	pwm->chan_en[ch] = 1;

	reg = readl(base + REG_TIM_CCR0 + 4*ch);
	reg &= ~TIM_BIT_CCxE;
	reg |= TIM_CCx_Enable;
	writel(reg, base + REG_TIM_CCR0 + 4*ch);

	return 0;
}

static int rtk_pwm_config(struct pwm_chip *chip, int ch, u32 duty_ns, u32 period_ns)
{
	unsigned long long prd, div, dty;
	unsigned int prescaler = 0;
	u32 reg;
	struct rtk_pwm *pwm = to_rtk_pwm_dev(chip);
	void __iomem *base = pwm->base;

	/* Period and prescaler values depends on clock rate */
	div = (unsigned long long)pwm->clk * period_ns;

	/*start to compute arr and prescaler*/
	do_div(div, NSEC_PER_SEC);
	prd = div;

	/*max count is UINT32_MAX because REG_TIM_CNT is 32bit*/
	while (div > U16_MAX) {
		prescaler++;
		div = prd;
		do_div(div, prescaler + 1);
	}

	prd = div;

	/*prescaler is 16bit*/
	if (prescaler > U16_MAX || prd > U16_MAX)
		return -EINVAL;

	/*
	 * All channels share the same prescaler and counter so when two
	 * channels are active at the same time we can't change them
	 */
	if (active_channels(chip) ) {
		u32 psc, arr;

		psc = readl(base + REG_TIM_PSC);
		arr = readl(base + REG_TIM_ARR);

		if ((psc != prescaler) || (arr != prd - 1))
			return -EBUSY;
	}

	/*update arr and perscaler*/
	writel(prescaler, base + REG_TIM_PSC);
	writel(prd - 1, base + REG_TIM_ARR);

	/*update APRE to reload ARR*/
	reg = readl(base + REG_TIM_CR);
	reg &= ~TIM_BIT_ARPE;
	writel(reg, base + REG_TIM_CR);

	/* Calculate the duty cycles */
	dty = prd * duty_ns;
	do_div(dty, period_ns);

	/* write duty and set pwm mode */
	reg = readl(base + REG_TIM_CCR0 + 4*ch);
	reg &= ~TIM_CCMode_CCR;
	reg |= dty;
	reg |= TIM_CCMode_PWM;
	writel(reg, base + REG_TIM_CCR0 + 4*ch);

	return 0;
}


static int rtk_pwm_apply(struct pwm_chip *chip, struct pwm_device *pwm,
						 const struct pwm_state *state)
{
	struct rtk_pwm *priv = to_rtk_pwm_dev(chip);
	int ret;
	bool enabled;

	/* protect common prescaler for all active channels */
	mutex_lock(&priv->lock);

	enabled = pwm->state.enabled;

	if (enabled && !state->enabled) {
		rtk_pwm_disable(chip, pwm->hwpwm);
		ret = 0;
		goto exit;
	}

	/* set polarity */
	if (state->polarity != pwm->state.polarity)
		rtk_pwm_polarity_config(chip, state->polarity, pwm->hwpwm);

	/* config duty_cycle and period */
	ret = rtk_pwm_config(chip, pwm->hwpwm, state->duty_cycle, state->period);
	if (ret)
		goto exit;

	if (!enabled && state->enabled) {
		ret = rtk_pwm_enable(chip, pwm->hwpwm);
		if (ret)
			goto exit;
	}

	/* start timer counter if it is not start */
	rtk_pwm_timer_start_count(chip, PWM_ENABLE);

exit:
	mutex_unlock(&priv->lock);
	return ret;
}


static const struct pwm_ops rtk_pwm_ops = {
	.owner = THIS_MODULE,
	.apply = rtk_pwm_apply,
	// .capture = rtk_pwm_capture,  //todo_eric: note that pwm timer can only capture period
	//and capture timer can only capture pulse or width. So
	//tim8 and tim9 are not fit to linux pwm capture function.
};





static int rtk_pwm_probe(struct platform_device *pdev)
{
	struct rtk_pwm *pwm;
	struct rtk_tim * tim = dev_get_drvdata(pdev->dev.parent);
	int ret;

	pwm = devm_kzalloc(&pdev->dev, sizeof(struct rtk_pwm), GFP_KERNEL);  //todo_eric
	if (!pwm)
		return -ENOMEM;

	mutex_init(&pwm->lock);

	pwm->base = tim->base;
	pwm->pwm_clk = tim->tim_clk;
	pwm->clk = tim->clk_rate;      //IP clk is XTAL40M
	pwm->chip.dev = &pdev->dev;
	pwm->chip.ops = &rtk_pwm_ops;
	pwm->chip.npwm = AMEBA_PWM_CHAN_NUM;
	pwm->chip.base = -1;

	ret = pwmchip_add(&pwm->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add PWM chip, error %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, pwm);

	return 0;
}


static const struct of_device_id rtk_pwm_of_match[] = {
	{ .compatible = "realtek,amebad2-pwm",	},
	{ /* end node */ },
};

static struct platform_driver rtk_pwm_driver = {
	.probe	= rtk_pwm_probe,
	.driver	= {
		.name = "rtk-pwm",
		.of_match_table = rtk_pwm_of_match,
	},
};

builtin_platform_driver(rtk_pwm_driver);

MODULE_AUTHOR("<eric_gao@realsil.com.cn>");
MODULE_DESCRIPTION("realtek PWM driver");
MODULE_LICENSE("GPL v2");

