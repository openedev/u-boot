/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_RK3399_COMMON_H
#define __CONFIG_RK3399_COMMON_H

#include "rockchip-common.h"

#define CONFIG_SYS_MALLOC_LEN		(32 << 20)
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SKIP_LOWLEVEL_INIT

#define COUNTER_FREQUENCY               24000000

#define CONFIG_SYS_NS16550_MEM32

#define CONFIG_SYS_INIT_SP_ADDR		0x00300000
#define CONFIG_SYS_LOAD_ADDR		0x00800800

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_TPL_BOOTROM_SUPPORT)
#define CONFIG_SPL_STACK		0x00400000
#define CONFIG_SPL_MAX_SIZE             0x100000
#define CONFIG_SPL_BSS_START_ADDR	0x00400000
#define CONFIG_SPL_BSS_MAX_SIZE         0x2000
#else
#define CONFIG_SPL_STACK		0xff8effff
#define CONFIG_SPL_MAX_SIZE		0x30000 - 0x2000
/*  BSS setup */
#define CONFIG_SPL_BSS_START_ADDR       0xff8e0000
#define CONFIG_SPL_BSS_MAX_SIZE         0x10000
#endif

#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* 64M */

/* MMC/SD IP block */
#define CONFIG_ROCKCHIP_SDHCI_MAX_FREQ	200000000

/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(128 << 10)

/* FAT sd card locations. */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xf8000000

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"upgrade_available=0\0" \
	"bootpart=4\0" \
	"upgrade_version=0.0-g0123456789ab\0" \
	"scriptaddr=0x00500000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02080000\0" \
	"ramdisk_addr_r=0x04000000\0"

#ifndef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS
#endif

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"uuid_gpt_root=" ROOT_UUID "\0" \
	"uuid_gpt_recovery=" RESV_UUID "\0" \
	"uuid_gpt_swap=" SWAP_UUID "\0" \
	"partitions=" PARTS_DEFAULT \
	ROCKCHIP_DEVICE_SETTINGS \
	BOOTENV \
	"updbootcmd=" \
		"setenv boot_syslinux_conf extlinux-updater/extlinux-updater.conf;" \
		"run distro_bootcmd\0" \
	"scan_dev_for_boot_part=" \
		"part list ${devtype} ${devnum} -bootable devplist;" \
		"env exists devplist || setenv devplist 1;" \
		"setenv devplist ${bootpart};" \
		"for distro_bootpart in ${devplist};" \
			"do if fstype ${devtype} ${devnum}:${distro_bootpart} bootfstype;" \
			"then run scan_dev_for_boot;" \
			"fi;" \
			"done\0" \
	"altbootcmd=" \
		"setenv boot_syslinux_conf extlinux-rollback/extlinux-rollback.conf;" \
		"run distro_bootcmd\0"

#endif

/* enable usb config for usb ether */

#endif
