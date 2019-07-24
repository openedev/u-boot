// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/utils.h>

#define DW_WDT_CR	0x00
#define DW_WDT_TORR	0x04
#define DW_WDT_CRR	0x0C

#define DW_WDT_CR_EN_OFFSET	0x00
#define DW_WDT_CR_RMOD_OFFSET	0x01
#define DW_WDT_CRR_RESTART_VAL	0x76

#define DW_WDT_MIN_TOP		0
#define DW_WDT_MAX_TOP		15
#define DW_WDT_TOPINIT_SHIFT	4

#ifdef CONFIG_WDT

#include <dm.h>
#include <wdt.h>
#include <clk.h>

struct dw_wdt {
	void __iomem *regs;
	unsigned long clk_rate;
};

static inline int dw_wdt_is_enabled(struct dw_wdt *dw)
{
	return readl(dw->regs + DW_WDT_CR) & DW_WDT_CR_RMOD_OFFSET;
}

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
static int dw_wdt_set_timeout(struct dw_wdt *dw, unsigned int timeout)
{
	int i, top_val;

	/* calculate the timeout range value */
	i = log_2_n_round_up(timeout * dw->clk_rate) - 16;
	top_val = clamp_t(int, i, DW_WDT_MIN_TOP, DW_WDT_MAX_TOP);

	writel((top_val | (top_val << DW_WDT_TOPINIT_SHIFT)),
	       dw->regs + DW_WDT_TORR);

	return 0;
}

static void dw_wdt_enable(struct dw_wdt *dw)
{
	u32 val = readl(dw->regs + DW_WDT_CR);

	/* Enable watchdog */
	val |= DW_WDT_CR_RMOD_OFFSET;
	writel(val, dw->regs + DW_WDT_CR);
}

static int dw_wdt_reset(struct udevice *dev)
{
	struct dw_wdt *dw = dev_get_priv(dev);

	if (dw_wdt_is_enabled(dw))
		writel(DW_WDT_CRR_RESTART_VAL, dw->regs + DW_WDT_CRR);
	else
		dw_wdt_enable(dw);

	return 0;
}

static int dw_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct dw_wdt *dw = dev_get_priv(dev);

	dw_wdt_set_timeout(dw, timeout);
	dw_wdt_enable(dw);

	return 0;
}

static int dw_wdt_probe(struct udevice *dev)
{
	struct dw_wdt *dw = dev_get_priv(dev);
	struct clk clk;
	int ret;

	dw->regs = dev_remap_addr(dev);
	if (!dw->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	dw->clk_rate = clk_get_rate(&clk);
	if (!dw->clk_rate)
		return -EINVAL;

	dw_wdt_reset(dev);

	return 0;
}

static const struct wdt_ops dw_wdt_ops = {
	.reset = dw_wdt_reset,
	.start = dw_wdt_start,
};

static const struct udevice_id dw_wdt_ids[] = {
	{ .compatible = "snps,dw-wdt" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(dw_wdt) = {
	.name = "dw_wdt",
	.id = UCLASS_WDT,
	.of_match = dw_wdt_ids,
	.ops = &dw_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct dw_wdt),
	.probe = dw_wdt_probe,
};

#else

#include <watchdog.h>

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
static int designware_wdt_settimeout(unsigned int timeout)
{
	signed int i;

	/* calculate the timeout range value */
	i = (log_2_n_round_up(timeout * CONFIG_DW_WDT_CLOCK_KHZ)) - 16;
	if (i > 15)
		i = 15;
	if (i < 0)
		i = 0;

	writel((i | (i << 4)), (CONFIG_DW_WDT_BASE + DW_WDT_TORR));
	return 0;
}

static void designware_wdt_enable(void)
{
	u32 val = readl(CONFIG_DW_WDT_BASE + DW_WDT_CR);

	/* Enable watchdog */
	val |= DW_WDT_CR_RMOD_OFFSET;
	writel(val, CONFIG_DW_WDT_BASE + DW_WDT_CR);
}

static unsigned int designware_wdt_is_enabled(void)
{
	return readl(CONFIG_DW_WDT_BASE + DW_WDT_CR) & DW_WDT_CR_RMOD_OFFSET;
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	if (designware_wdt_is_enabled())
		/* restart the watchdog counter */
		writel(DW_WDT_CRR_RESTART_VAL,
		       (CONFIG_DW_WDT_BASE + DW_WDT_CRR));
}

void hw_watchdog_init(void)
{
	/* reset to disable the watchdog */
	hw_watchdog_reset();
	/* set timer in miliseconds */
	designware_wdt_settimeout(CONFIG_WATCHDOG_TIMEOUT_MSECS);
	/* enable the watchdog */
	designware_wdt_enable();
	/* reset the watchdog */
	hw_watchdog_reset();
}
#endif

#endif /* CONFIG_WDT */
