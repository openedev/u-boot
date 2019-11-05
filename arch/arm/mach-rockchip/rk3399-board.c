// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <bootcount.h>
#include <environment.h>
#include <asm/arch-rockchip/boot_mode.h>

enum upgrade_type {
	UPGRADE_NO_NEED	= 0,
	UPGRADE_NEED	= 1,
	UPGRADE_DONE	= 2,
};

static void upgrade_bootcount_variable(void)
{
	int val = env_get_ulong("upgrade_available", 10, 0);

	switch (val) {
	case UPGRADE_NEED:
		env_set("bootcmd", "run updbootcmd");
		break;
	case UPGRADE_DONE:
		bootcount_store(0);
		env_set_ulong("upgrade_available", UPGRADE_NO_NEED);
		env_save();
		break;
	case UPGRADE_NO_NEED:
		debug("%s: No upgrade action needed!\n", __func__);
	}
}

int board_late_init(void)
{
	setup_boot_mode();
	upgrade_bootcount_variable();

	return 0;
}
