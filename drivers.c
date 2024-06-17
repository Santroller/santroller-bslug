#include "usb_hid.h"
static const usb_device_driver_t *usb_device_drivers[] = {
	&gh_guitar_usb_device_driver,
	&gh_drum_usb_device_driver,
	&turntable_usb_device_driver
};