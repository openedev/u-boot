// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Copyright (C) 2020 Engicam S.r.l.
 * Copyright (C) 2020 Amarula Solutions(India)
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <misc.h>
#include <net.h>
#include <phy.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <power/regulator.h>

/* SYSCFG registers */
#define SYSCFG_BOOTR		0x00
#define SYSCFG_PMCSETR		0x04
#define SYSCFG_IOCTRLSETR	0x18
#define SYSCFG_ICNR		0x1C
#define SYSCFG_CMPCR		0x20
#define SYSCFG_CMPENSETR	0x24
#define SYSCFG_PMCCLRR		0x44

#define SYSCFG_PMCSETR_ETH_CLK_SEL	BIT(16)
#define SYSCFG_PMCSETR_ETH_REF_CLK_SEL	BIT(17)

#define SYSCFG_PMCSETR_ETH_SELMII	BIT(20)

#define SYSCFG_PMCSETR_ETH_SEL_MASK	GENMASK(23, 21)
#define SYSCFG_PMCSETR_ETH_SEL_GMII_MII	0
#define SYSCFG_PMCSETR_ETH_SEL_RGMII	BIT(21)
#define SYSCFG_PMCSETR_ETH_SEL_RMII	BIT(23)

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* nothing to do, only used in SPL */
	return 0;
}

int checkboard(void)
{
	char *mode;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_TFABOOT))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	return 0;
}

/* board dependent setup after realloc */
int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	if (IS_ENABLED(CONFIG_DM_REGULATOR))
		regulators_enable_boot_on(_DEBUG);

	return 0;
}

int board_late_init(void)
{
	const void *fdt_compat;
	int fdt_compat_len;
	int ret;
	u32 otp;
	struct udevice *dev;
	char buf[10];
	char dtb_name[256];
	int buf_len;

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
					 &fdt_compat_len);
		if (fdt_compat && fdt_compat_len) {
			if (strncmp(fdt_compat, "st,", 3) != 0) {
				env_set("board_name", fdt_compat);
			} else {
				env_set("board_name", fdt_compat + 3);

				buf_len = sizeof(dtb_name);
				strncpy(dtb_name, fdt_compat + 3, buf_len);
				buf_len -= strlen(fdt_compat + 3);
				strncat(dtb_name, ".dtb", buf_len);
				env_set("fdtfile", dtb_name);
			}
		}
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_GET_DRIVER(stm32mp_bsec),
						  &dev);

		if (!ret)
			ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
					&otp, sizeof(otp));
		if (ret > 0 && otp) {
			snprintf(buf, sizeof(buf), "0x%04x", otp >> 16);
			env_set("board_id", buf);

			snprintf(buf, sizeof(buf), "0x%04x",
				 ((otp >> 8) & 0xF) - 1 + 0xA);
			env_set("board_rev", buf);
		}
	}

	return 0;
}

/* eth init function : weak called in eqos driver */
int board_interface_eth_init(struct udevice *dev,
			     phy_interface_t interface_type)
{
	u8 *syscfg;
	u32 value;
	bool eth_clk_sel_reg = false;
	bool eth_ref_clk_sel_reg = false;

	/* Gigabit Ethernet 125MHz clock selection. */
	eth_clk_sel_reg = dev_read_bool(dev, "st,eth_clk_sel");

	/* Ethernet 50Mhz RMII clock selection */
	eth_ref_clk_sel_reg =
		dev_read_bool(dev, "st,eth_ref_clk_sel");

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	if (!syscfg)
		return -ENODEV;

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
			SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		debug("%s: PHY_INTERFACE_MODE_MII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_GMII:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII;
		debug("%s: PHY_INTERFACE_MODE_GMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (eth_ref_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RMII |
				SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RMII;
		debug("%s: PHY_INTERFACE_MODE_RMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII;
		debug("%s: PHY_INTERFACE_MODE_RGMII\n", __func__);
		break;
	default:
		debug("%s: Do not manage %d interface\n",
		      __func__, interface_type);
		/* Do not manage others interfaces */
		return -EINVAL;
	}

	/* clear and set ETH configuration bits */
	writel(SYSCFG_PMCSETR_ETH_SEL_MASK | SYSCFG_PMCSETR_ETH_SELMII |
	       SYSCFG_PMCSETR_ETH_REF_CLK_SEL | SYSCFG_PMCSETR_ETH_CLK_SEL,
	       syscfg + SYSCFG_PMCCLRR);
	writel(value, syscfg + SYSCFG_PMCSETR);

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
		else if (CONFIG_IS_ENABLED(ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NAND:
	case BOOT_FLASH_SPINAND:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_UBI))
			return ENVL_UBI;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NOR:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		else
			return ENVL_NOWHERE;

	default:
		return ENVL_NOWHERE;
	}
}

const char *env_ext4_get_intf(void)
{
	u32 bootmode = get_bootmode();

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		return "mmc";
	default:
		return "";
	}
}

const char *env_ext4_get_dev_part(void)
{
	static char *const dev_part[] = {"0:auto", "1:auto", "2:auto"};
	u32 bootmode = get_bootmode();

	return dev_part[(bootmode & TAMP_BOOT_INSTANCE_MASK) - 1];
}

int mmc_get_env_dev(void)
{
	u32 bootmode = get_bootmode();

	return (bootmode & TAMP_BOOT_INSTANCE_MASK) - 1;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif
