#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/sched_clock.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/mfd/rtk-timer.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <uapi/sound/rtk_tim9.h>
#include "ameba_audio_clock.h"
#include "ameba_audio_clock_driver.h"


struct sdm_param {
	u32 DIVN_SDM;
	u32 F0F_SDM;
	u32 F0N_SDM;
};


struct capture_timer {
	struct device *tim_dev;
	void __iomem *base;
	void __iomem *pll_base;
	u32 irq;
	struct clk *tim9_mux;
	struct clk *tim9_cke;
	struct clk *tim9_i2s_pll2;
	struct mutex ops_lock;
} ;

struct capture_timer cap_tim;
static int major = 20;
static int minor = 0;
static dev_t devno;
static struct class *tim9_class;
static struct device *tim9_device;
u64 tim9_ovf_cnt = 0;

static irqreturn_t rtk_capture_timer_irq(int irq, void *dev_id)
{
	u32 CounterIndex = 0;
	u32 reg;
	void __iomem *base = cap_tim.base;

	/* Clear the all IT pending Bits */
	reg = readl_relaxed(base + REG_TIM_SR);
	writel_relaxed(reg, base + REG_TIM_SR);

	/* make sure write ok, because bus delay */
	while (1) {
		CounterIndex++;
		if (CounterIndex >= 300) {
			break;
		}

		if (((readl_relaxed(base + REG_TIM_SR)) & 0xFFFFFF) == 0) {
			break;
		}
	}

	tim9_ovf_cnt++;

	return IRQ_HANDLED;
}


static void tim9_start(void)
{
	u32 reg;
	void __iomem *base = cap_tim.base;

	/*disable interrupt*/
	writel_relaxed(0, base + REG_TIM_DIER);

	/*clear all pending bits*/
	reg = readl_relaxed(base + REG_TIM_SR);
	writel_relaxed(reg, base + REG_TIM_SR);
	reg = readl_relaxed(base + REG_TIM_SR);

	/*set ARR*/
	writel_relaxed(0xffff, base + REG_TIM_ARR);

	writel_relaxed(0, base + REG_TIM_PSC);

	/*set CR*/
	reg = readl_relaxed(base + REG_TIM_CR);
	reg |= TIM_BIT_ARPE | TIM_BIT_URS;
	reg &= ~TIM_BIT_UDIS;
	writel_relaxed(reg, base + REG_TIM_CR);

	/*Generate an update event*/
	writel_relaxed(TIM_PSCReloadMode_Immediate, base + REG_TIM_EGR);

	while (1) {
		if (readl_relaxed(base + REG_TIM_SR) & TIM_BIT_UG_DONE) {
			break;
		}
	}

	/* Clear all flags*/
	reg = readl_relaxed(base + REG_TIM_SR);
	writel_relaxed(reg, base + REG_TIM_SR);

	/* enable interrupt */
	reg = readl_relaxed(base + REG_TIM_DIER);
	reg |= TIM_IT_Update;
	writel_relaxed(reg, base + REG_TIM_DIER);

	/* Enable the TIM Counter, dont do this if timer is RUN */
	reg = readl_relaxed(base + REG_TIM_EN);
	if ((readl_relaxed(base + REG_TIM_EN) & TIM_BIT_CEN) == 0) {
		writel_relaxed(TIM_BIT_CNT_START, base + REG_TIM_EN);
	}

	/* poll if cnt is running, 3*32k cycles */
	while (1) {
		if (readl_relaxed(base + REG_TIM_EN) & TIM_BIT_CEN) {
			break;
		}
	}

}

static int ameba_i2s_pll2_clk_tune(u32 ppm, u32 action, struct sdm_param *sdm)
{
	u32 F0F_new;
	u32 tmp;

	if (ppm > 1000) {
		pr_info("invalid ppm:%d", ppm);
		return -EINVAL;
	}

	tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);
	tmp &= (~PLL_MASK_IPLL2_DIVN_SDM);
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);
	tmp |= (PLL_IPLL2_DIVN_SDM(sdm->DIVN_SDM));
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);

	if (action == I2SPLL2_FASTER) {
		F0F_new = sdm->F0F_SDM + ppm * 59 / 100;
	} else if (action == I2SPLL2_SLOWER) {
		F0F_new = sdm->F0F_SDM - ppm * 59 / 100;
	} else {
		F0F_new = sdm->F0F_SDM;
	}

	tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);
	tmp &= (~PLL_MASK_IPLL2_F0F_SDM);
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);
	tmp |= (PLL_IPLL2_F0F_SDM(F0F_new));
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);

	tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);
	tmp &= (~PLL_MASK_IPLL2_F0N_SDM);
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);
	tmp |= (PLL_IPLL2_F0N_SDM(sdm->F0N_SDM));
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL3);

	tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);
	tmp |= (PLL_BIT_IPLL2_TRIG_FREQ);
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);
	tmp &= (~PLL_BIT_IPLL2_TRIG_FREQ);
	writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);

	return 0;
}

static int ameba_update_i2s_pll_input_clock_status(bool enabled, u32 div)
{
	int res = 0;
	u32 tmp;

	if (enabled == true) {
		//Check BandGap power on
		tmp = readl(cap_tim.pll_base + REG_PLL_AUX_BG);
		if ((tmp & PLL_BG_POW_MASK) == 0) {
			tmp |= (PLL_BIT_POW_BG | PLL_BIT_POW_I | PLL_BIT_POW_MBIAS);
			writel(tmp, cap_tim.pll_base + REG_PLL_AUX_BG);
			//DelayUs(160);
			usleep_range(160, 170);
		}

		// erc enable
		tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);
		tmp |= PLL_BIT_IPLL2_POW_ERC;
		writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);

		//DelayUs(1);
		usleep_range(1, 2);

		// PLL power on
		tmp = readl(cap_tim.pll_base + REG_PLL_PS);
		tmp |= PLL_BIT_POW_CKGEN;
		writel(tmp, cap_tim.pll_base + REG_PLL_PS);

		tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);
		tmp |= PLL_BIT_IPLL2_POW_PLL | PLL_BIT_IPLL2_CK_EN | PLL_BIT_IPLL2_CK_EN_D4;
		writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);

		// Select I2S1
		tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);
		tmp &= ~PLL_MASK_IPLL2_CK_OUT_SEL;
		tmp |= PLL_IPLL2_CK_OUT_SEL(div);
		writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL1);

		// Wait ready
		//to be checked, if no sleep here, pll will stuck here
		msleep(10);
		while (!(readl(cap_tim.pll_base + REG_PLL_STATE) & PLL_BIT_CKRDY_I2S2));
	} else {
		tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);
		tmp &= ~ PLL_BIT_IPLL2_POW_PLL;
		writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);

		tmp = readl(cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);
		tmp &= ~ PLL_BIT_IPLL2_POW_ERC;
		writel(tmp, cap_tim.pll_base + REG_PLL_I2SPLL2_CTRL0);

		tmp = readl(cap_tim.pll_base + REG_PLL_AUX_BG);
		tmp &= ~(PLL_BIT_POW_BG | PLL_BIT_POW_I | PLL_BIT_POW_MBIAS);
		writel(tmp, cap_tim.pll_base + REG_PLL_AUX_BG);

		msleep(10);
		while (readl(cap_tim.pll_base + REG_PLL_STATE) & PLL_BIT_CKRDY_I2S1);
	}
	return res;
}




static ssize_t tim9_read_count(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	u64 cnt;
	int ret;

	cnt = readl_relaxed(cap_tim.base + REG_TIM_CNT) & 0xffff;
	cnt += 65536 * tim9_ovf_cnt;

	ret = copy_to_user((void *)buf, (void *) &cnt, 8);
	if (ret) {
		pr_err("tim9: fail to copy time to user buf: %d\n", ret);
	}

	return 0;
}



static int tim9_i2spll2_adjust(struct i2spll2_micro_adjust_params *args)
{
	int res = 0;
	struct sdm_param sdm;
	struct i2spll2_micro_adjust_params tmp;
	struct i2spll2_micro_adjust_params *p_tmp = &tmp;

	if (copy_from_user((unsigned char *)p_tmp, (unsigned char *)args, sizeof(struct i2spll2_micro_adjust_params))) {
		pr_info("error copy from user\n");
		return -EFAULT;
	}

	if (p_tmp->clock == I2SPLL2_24P576M_DIV2) {
		sdm.DIVN_SDM = 17;
		sdm.F0F_SDM = 2346;
		sdm.F0N_SDM = 5;
		res = ameba_i2s_pll2_clk_tune(p_tmp->ppm, p_tmp->action, &sdm);
		ameba_update_i2s_pll_input_clock_status(true, 15);
	} else if (p_tmp->clock == I2SPLL2_45P158M_DIV2) {
		sdm.DIVN_SDM = 7;
		sdm.F0F_SDM = 2071;
		sdm.F0N_SDM = 0;
		res = ameba_i2s_pll2_clk_tune(p_tmp->ppm, p_tmp->action, &sdm);
		ameba_update_i2s_pll_input_clock_status(true, 12);
	} else if (p_tmp->clock == I2SPLL2_98P304M_DIV2) {
		sdm.DIVN_SDM = 17;
		sdm.F0F_SDM = 2346;
		sdm.F0N_SDM = 5;
		res = ameba_i2s_pll2_clk_tune(p_tmp->ppm, p_tmp->action, &sdm);
		ameba_update_i2s_pll_input_clock_status(true, 12);
	}

	tim9_start();
	tim9_ovf_cnt = 0;

	return res;
}

static long int tim9_i2spll2_clk_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	int res = 0;
	switch (cmd) {
	case TIM9_I2SPLL2_ADJUST:
		res = tim9_i2spll2_adjust((struct i2spll2_micro_adjust_params *)args);
		break;
	default:
		return -EFAULT;
	}
	return res;
}

static const struct file_operations tim9_ops = {
	.owner    = THIS_MODULE,
	.unlocked_ioctl    = tim9_i2spll2_clk_ioctl,
	.read     = tim9_read_count,
};


static int rtk_capture_timer_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct resource *res_pll;
	struct sdm_param sdm;

	cap_tim.tim_dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	cap_tim.base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(cap_tim.base)) {
		return PTR_ERR(cap_tim.base);
	}

	res_pll = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	cap_tim.pll_base = devm_ioremap(&pdev->dev, res_pll->start, resource_size(res_pll));
	if (IS_ERR(cap_tim.pll_base)) {
		return PTR_ERR(cap_tim.pll_base);
	}

	cap_tim.irq = platform_get_irq(pdev, 0);
	if (cap_tim.irq < 0) {
		dev_err(&pdev->dev, "Fail to get irq\n");
		return cap_tim.irq;
	}

	cap_tim.tim9_cke = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(cap_tim.tim9_cke)) {
		ret =  PTR_ERR(cap_tim.tim9_cke);
		dev_err(&pdev->dev, "Fail to get rcc tim9_cke: %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(cap_tim.tim9_cke);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to enable clock %d\n", ret);
		return ret;
	}

	ret = request_irq(cap_tim.irq, (irq_handler_t) rtk_capture_timer_irq, 0, pdev->name, NULL);
	if (ret) {
		dev_err(&pdev->dev, "Fail to request irq\n");
		return ret;
	}

	sdm.DIVN_SDM = 7;
	sdm.F0F_SDM = 2071;
	sdm.F0N_SDM = 0;
	ameba_i2s_pll2_clk_tune(0, 0, &sdm);
	ameba_update_i2s_pll_input_clock_status(true, 12);

	/* enable tim9 */
	tim9_start();

	devno = MKDEV(major, minor);
	ret = register_chrdev(major, "tim9", &tim9_ops);

	tim9_class = class_create(THIS_MODULE, "tim9_class");
	if (IS_ERR(tim9_class)) {
		unregister_chrdev(major, "tim9");
		return -EBUSY;
	}

	/* mknod /dev/tim9 */
	tim9_device = device_create(tim9_class, NULL, devno, NULL, "tim9");
	if (IS_ERR(tim9_device)) {
		class_destroy(tim9_class);
		unregister_chrdev(major, "tim9");
		return -EBUSY;
	}

	return 0;
}


static const struct of_device_id rtk_capture_timer_of_match[] = {
	{ .compatible = "realtek,amebad2-captimer",	},
	{ /* end node */ },
};

static struct platform_driver rtk_capture_timer_driver = {
	.probe	= rtk_capture_timer_probe,
	.driver	= {
		.name = "rtk-capture-timer",
		.of_match_table = rtk_capture_timer_of_match,
	},
};



builtin_platform_driver(rtk_capture_timer_driver);

MODULE_AUTHOR("<eric_gao@realsil.com.cn>");
MODULE_DESCRIPTION("realtek capture timer driver");
MODULE_LICENSE("GPL v2");
