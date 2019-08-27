// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <power/regulator.h>
#include <usb.h>
#include <dwc3-uboot.h>

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xfe800000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.usb2_phyif_utmi_width = 16,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif

int board_init(void)
{
	int ret;

	ret = regulators_enable_boot_on(false);
	if (ret)
		debug("%s: Cannot enable boot on regulator\n", __func__);

	return 0;
}
