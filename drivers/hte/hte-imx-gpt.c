#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/hte.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define GPT_TPRER_PRE24M		12

/* Register Offsets for i.MX GPT v2 */
#define GPT_CR		0x00
#define GPT_PR		0x04
#define GPT_SR		0x08
#define GPT_IR		0x0C
#define GPT_ICR1	0x1C
#define GPT_ICR2	0x20
#define GPT_CNT		0x24

/* Control Register Bits */
#define GPT_CR_SWR 			(1 << 15)
#define GPT_CR_24MEN		(1 << 10)
#define GPT_CR_FRR			(1 << 9)
#define GPT_CR_CLK_PER		(2 << 6)
#define GPT_CR_STOPEN		(1 << 5)
#define GPT_CR_WAITEN		(1 << 3)
#define GPT_CR_TEN			(1 << 0)
/* Capture Mode Bits */
#define GPT_CR_IM1_MASK		GENMASK(17, 16)
#define GPT_CR_IM2_MASK		GENMASK(19, 18)
#define GPT_CR_RISING		0x1
#define GPT_CR_FALLING		0x2
#define GPT_CR_BOTH			0x3

/* Status/Interrupt Bits */
#define GPT_SR_IF1		(1 << 3)
#define GPT_SR_IF2		(1 << 4)
#define GPT_IR_IF1IE	(1 << 3)
#define GPT_IR_IF2IE	(1 << 4)

struct gpt_capture_dev {
	void __iomem *base;
	struct clk *clk_ipg;
	struct clk *clk_per;
	int irq;
	u32 rate;
	spinlock_t lock;
	struct hte_chip chip;
};

static inline void gpt_push_capture_event(struct gpt_capture_dev *dev, int line,
					  u32 reg, u32 gpt_now, u64 ns_now)
{
	struct hte_ts_data ts = { 0 };
	u32 gpt_event, delta_cycles;
	u64 delta_ns;

	/* Input Capture Register holds the counter value at the event */
	gpt_event = readl(dev->base + reg);

	/* Calculate elapsed cycles (Handles 32-bit wrap-around safely) */
	delta_cycles = gpt_now - gpt_event;
	delta_ns = div_u64((u64)delta_cycles * NSEC_PER_SEC, dev->rate);

	ts.tsc = ns_now - delta_ns;
	hte_push_ts_ns(&dev->chip, line, &ts);
}

static irqreturn_t gpt_capture_isr(int irq, void *dev_id)
{
	struct gpt_capture_dev *dev = dev_id;
	u32 status, gpt_now;
	u64 ns_now;

	status = readl(dev->base + GPT_SR);

	if (!(status & (GPT_SR_IF1 | GPT_SR_IF2)))
		return IRQ_NONE;

	if (!dev->rate) {
		dev_err_ratelimited(dev->chip.dev, "clock rate is zero\n");
		writel(status, dev->base + GPT_SR);
		return IRQ_HANDLED;
	}

	ns_now = ktime_get_raw_fast_ns();
	gpt_now = readl(dev->base + GPT_CNT);

	if (status & GPT_SR_IF1)
		gpt_push_capture_event(dev, 0, GPT_ICR1, gpt_now, ns_now);

	if (status & GPT_SR_IF2)
		gpt_push_capture_event(dev, 1, GPT_ICR2, gpt_now, ns_now);

	writel(status, dev->base + GPT_SR);
	return IRQ_HANDLED;
}

static int gpt_hte_enable(struct hte_chip *chip, u32 xlated_id)
{
	struct gpt_capture_dev *dev = dev_get_drvdata(chip->dev);
	u32 ir;
	unsigned long flags;

	if (xlated_id > 1)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);

	/* Clear any pending status before enabling */
	writel(xlated_id == 0 ? GPT_SR_IF1 : GPT_SR_IF2, dev->base + GPT_SR);

	ir = readl(dev->base + GPT_IR);
	ir |= (xlated_id == 0 ? GPT_IR_IF1IE : GPT_IR_IF2IE);
	writel(ir, dev->base + GPT_IR);

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

static int gpt_hte_disable(struct hte_chip *chip, u32 xlated_id)
{
	struct gpt_capture_dev *dev = dev_get_drvdata(chip->dev);
	u32 ir;
	unsigned long flags;

	if (xlated_id > 1)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);

	ir = readl(dev->base + GPT_IR);
	ir &= ~(xlated_id == 0 ? GPT_IR_IF1IE : GPT_IR_IF2IE);
	writel(ir, dev->base + GPT_IR);

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

static int gpt_hte_request(struct hte_chip *chip, struct hte_ts_desc *desc, u32 xlated_id)
{
	struct gpt_capture_dev *dev = dev_get_drvdata(chip->dev);
	unsigned long flags;
	u32 cr, edge_val = 0;

	if (xlated_id > 1)
		return -EINVAL;

	if (desc->attr.edge_flags & HTE_FALLING_EDGE_TS)
		edge_val |= GPT_CR_FALLING;
	if (desc->attr.edge_flags & HTE_RISING_EDGE_TS)
		edge_val |= GPT_CR_RISING;

	/* Default to falling if no specific edge requested */
	if (!edge_val)
		edge_val = GPT_CR_FALLING;

	spin_lock_irqsave(&dev->lock, flags);

	cr = readl(dev->base + GPT_CR);
	if (xlated_id == 0) {
		cr &= ~GPT_CR_IM1_MASK;
		cr |= FIELD_PREP(GPT_CR_IM1_MASK, edge_val);
	} else {
		cr &= ~GPT_CR_IM2_MASK;
		cr |= FIELD_PREP(GPT_CR_IM2_MASK, edge_val);
	}
	writel(cr, dev->base + GPT_CR);

	spin_unlock_irqrestore(&dev->lock, flags);

	return gpt_hte_enable(chip, xlated_id);
}

static int gpt_hte_release(struct hte_chip *chip, struct hte_ts_desc *desc, u32 xlated_id)
{
	if (xlated_id > 1)
		return -EINVAL;
	return gpt_hte_disable(chip, xlated_id);
}

static int gpt_hte_get_clk_info(struct hte_chip *chip, struct hte_clk_info *ci)
{
	struct gpt_capture_dev *dev = dev_get_drvdata(chip->dev);

	ci->hz = dev->rate;
	ci->type = CLOCK_MONOTONIC;
	return 0;
}

static const struct hte_ops gpt_hte_ops = {
	.request = gpt_hte_request,
	.release = gpt_hte_release,
	.enable = gpt_hte_enable,
	.disable = gpt_hte_disable,
	.get_clk_src_info = gpt_hte_get_clk_info,
};

static int gpt_hte_xlate_of(struct hte_chip *gc, const struct of_phandle_args *args, struct hte_ts_desc *desc, u32 *xlated_id)
{
	if (args->args_count != gc->of_hte_n_cells)
		return -EINVAL;

	*xlated_id = args->args[0];

	return 0;
}

static void gpt_clk_disable(void *data)
{
	struct gpt_capture_dev *gpt_dev = data;

	clk_disable_unprepare(gpt_dev->clk_per);
	clk_disable_unprepare(gpt_dev->clk_ipg);
}

static int gpt_capture_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct gpt_capture_dev *gpt_dev;
	u32 cr, prescaler;
	int ret;

	gpt_dev = devm_kzalloc(dev, sizeof(*gpt_dev), GFP_KERNEL);
	if (!gpt_dev)
		return -ENOMEM;

	spin_lock_init(&gpt_dev->lock);

	gpt_dev->base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(gpt_dev->base))
		return PTR_ERR(gpt_dev->base);

	gpt_dev->irq = platform_get_irq(pdev, 0);
	if (gpt_dev->irq < 0)
		return gpt_dev->irq;

	gpt_dev->clk_ipg = devm_clk_get(dev, "ipg");
	if (IS_ERR(gpt_dev->clk_ipg))
		return PTR_ERR(gpt_dev->clk_ipg);

	gpt_dev->clk_per = devm_clk_get(dev, "osc_per");
	if (IS_ERR(gpt_dev->clk_per))
		gpt_dev->clk_per = devm_clk_get(dev, "per");
	if (IS_ERR(gpt_dev->clk_per))
		return PTR_ERR(gpt_dev->clk_per);

	ret = clk_prepare_enable(gpt_dev->clk_ipg);
	if (ret)
		return ret;

	ret = clk_prepare_enable(gpt_dev->clk_per);
	if (ret) {
		clk_disable_unprepare(gpt_dev->clk_ipg);
		return ret;
	}

	ret = devm_add_action_or_reset(dev, gpt_clk_disable, gpt_dev);
	if (ret)
		return ret;

	/* Reset */
	writel(GPT_CR_SWR, gpt_dev->base + GPT_CR);
	while (readl(gpt_dev->base + GPT_CR) & GPT_CR_SWR)
		cpu_relax();

	prescaler = 0;
	of_property_read_u32(dev->of_node, "fsl,prescaler", &prescaler);
	writel(prescaler, gpt_dev->base + GPT_PR);

	if (of_property_read_u32(dev->of_node, "clock-frequency", &gpt_dev->rate))
		gpt_dev->rate = clk_get_rate(gpt_dev->clk_per) / (prescaler + 1);

	gpt_dev->chip.nlines = 2; /* Support ICR1 and ICR2 */
	gpt_dev->chip.dev = dev;
	gpt_dev->chip.ops = &gpt_hte_ops;
	gpt_dev->chip.xlate_of = gpt_hte_xlate_of;
	gpt_dev->chip.of_hte_n_cells = 1;

	dev_set_drvdata(dev, gpt_dev);

	/* Clear status and disable interrupts before registering ISR */
	writel(0x3F, gpt_dev->base + GPT_SR);
	writel(0, gpt_dev->base + GPT_IR);

	ret = devm_request_irq(dev, gpt_dev->irq, gpt_capture_isr, 
			       0, "gpt_capture", gpt_dev);
	if (ret)
		return ret;

	/* Configure timer */
	cr = GPT_CR_FRR | GPT_CR_WAITEN | GPT_CR_STOPEN | GPT_CR_CLK_PER;

	/* Start timer */
	cr |= GPT_CR_TEN;
	writel(cr, gpt_dev->base + GPT_CR);

	return devm_hte_register_chip(&gpt_dev->chip);
}

static void gpt_capture_remove(struct platform_device *pdev)
{
	struct gpt_capture_dev *gpt_dev = dev_get_drvdata(&pdev->dev);

	/* Disable timer and interrupts */
	writel(0, gpt_dev->base + GPT_IR);
	writel(0, gpt_dev->base + GPT_CR);

	/* Clocks are automatically disabled safely via devm_add_action_or_reset */
}

static const struct of_device_id gpt_dt_ids[] = {
	{ .compatible = "fsl,hte-imx-gpt" },
	{}
};
MODULE_DEVICE_TABLE(of, gpt_dt_ids);

static struct platform_driver gpt_capture_driver = {
	.probe = gpt_capture_probe,
	.remove = gpt_capture_remove,
	.driver = {
		.name = "hte-imx-gpt",
		.of_match_table = gpt_dt_ids,
	},
};

module_platform_driver(gpt_capture_driver);
MODULE_LICENSE("GPL");