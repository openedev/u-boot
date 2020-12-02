// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}
