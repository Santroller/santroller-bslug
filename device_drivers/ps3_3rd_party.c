#include <stddef.h>
#include <stdio.h>
#include "defines.h"
#include "rvl/WPAD.h"
#include "usb.h"
#include "usb_hid.h"
#include "wiimote.h"
int ps3_set_leds(usb_input_device_t *device) {
    printf("Set leds! %d\r\n", device->wiimote);
    static uint8_t buf[] ATTRIBUTE_ALIGN(32) = {
        0x01, /* outputType */
        0x08, /* data len? */
        0x00, /* LED_1 = 0x01, LED_2 = 0x10, ... */
        0x00, 0x00, 0x00, 0x00, 0x00};

    buf[2] = 1 << device->wiimote;
    return usb_device_driver_issue_ctrl_transfer_async(device,
                                                       USB_REQTYPE_INTERFACE_SET,
                                                       USB_REQ_SETREPORT,
                                                       (USB_REPTYPE_OUTPUT << 8) | 0x01, 0,
                                                       buf, sizeof(buf));
}
