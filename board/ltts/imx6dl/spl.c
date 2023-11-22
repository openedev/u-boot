// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <image.h>
#include <init.h>
#include <serial.h>
#include <spl.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/sizes.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>

#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/video.h>

#ifdef CONFIG_ENV_IS_IN_MMC
void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 boot_dev = BOOT_DEVICE_MMC1;

	switch ((bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC1 */
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC */
		boot_dev = BOOT_DEVICE_MMC2;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		printf("Wrong board boot order\n");
		break;
	}

	spl_boot_list[0] = boot_dev;
}
#endif

/*
 * Driving strength:
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */
#define IMX6SDL_DRIVE_STRENGTH		0x28

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	.dram_sdclk_0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_cas = IMX6SDL_DRIVE_STRENGTH,
	.dram_ras = IMX6SDL_DRIVE_STRENGTH,
	.dram_reset = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6SDL_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ctlds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b1ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b2ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b3ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b4ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b5ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b6ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b7ds = IMX6SDL_DRIVE_STRENGTH,
};

/* mt41j256 */
static struct mx6_ddr3_cfg mt41j256 = {
	.mem_speed = 1600,
	.density = 4,
	.width = 32,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1500,
	.trcmin = 5250,
	.trasmin = 3750,
	.SRT = 0,
};

static struct mx6_mmdc_calibration mx6dl_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001F0024,
	.p0_mpwldectrl1 = 0x00110018,
	.p1_mpwldectrl0 = 0x001F0024,
	.p1_mpwldectrl1 = 0x00110018,
	.p0_mpdgctrl0 = 0x4230022C,
	.p0_mpdgctrl1 = 0x02180220,
	.p1_mpdgctrl0 = 0x42440248,
	.p1_mpdgctrl1 = 0x02300238,
	.p0_mprddlctl = 0x44444A48,
	.p1_mprddlctl = 0x46484A42,
	.p0_mpwrdlctl = 0x38383234,
	.p1_mpwrdlctl = 0x3C34362E,
};

/* DDR 64bit 1GB */
static struct mx6_ddr_sysinfo mem_dl = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 1,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

/* DDR 32bit 512MB */
static struct mx6_ddr_sysinfo mem_s = {
	.dsize		= 1,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 1,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00003F3F, &ccm->CCGR0);
	writel(0x0030FC00, &ccm->CCGR1);
	writel(0x000FC000, &ccm->CCGR2);
	writel(0x3F300000, &ccm->CCGR3);
	writel(0xFF00F300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003CC, &ccm->CCGR6);
}

static void spl_dram_init(void)
{
	if (is_mx6solo()) {
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_s, &mx6dl_mmdc_calib, &mt41j256);
	} else if (is_mx6dl()) {
		mx6sdl_dram_iocfg(64, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_dl, &mx6dl_mmdc_calib, &mt41j256);
	}

	udelay(100);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	if (!(is_mx6ul()))
		gpr_init();

	/* setup GP timer */
	timer_init();

	/* Enable device tree and early DM support*/
	spl_early_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();
}
