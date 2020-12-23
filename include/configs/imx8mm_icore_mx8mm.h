/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020 Engicam srl
 * Copyright (c) 2020 Amarula Solutions(India)
 */

#ifndef __IMX8MM_ICORE_MX8MM_H
#define __IMX8MM_ICORE_MX8MM_H

#define CONFIG_SYS_INIT_RAM_SIZE	0x200000
#define PHYS_SDRAM_SIZE			0x40000000 /* 2GB DDR */

#include "imx8mm-common.h"

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END	(CONFIG_SYS_MEMTEST_START + (PHYS_SDRAM_SIZE >> 1))

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0)
#include <config_distro_bootcmd.h>
#undef CONFIG_ISO_PARTITION
#else
#define BOOTENV
#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdt_addr=0x43000000\0"	\
	"fdt_high=0xffffffffffffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"console=ttymxc1,115200\0" \
	BOOTENV

#define CONFIG_MXC_UART_BASE		UART2_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* USDHC */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* I2C */
#define CONFIG_SYS_I2C_SPEED		100000

#endif /* __IMX8MM_ICORE_MX8MM_H */
