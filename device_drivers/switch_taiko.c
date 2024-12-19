#include "rvl/WPAD.h"
#include "usb_hid.h"
#include "wiimote.h"

#define GUITAR_ACC_RES_PER_G 113
struct taiko_input_report {
    uint8_t rimRight : 1;
    uint8_t rimLeft : 1;
    uint8_t r : 1;
    uint8_t l : 1;
    uint8_t x : 1;
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t y : 1;

    uint8_t : 2;
    uint8_t capture : 1;
    uint8_t home : 1;
    uint8_t centerRight : 1;
    uint8_t centerLeft : 1;
    uint8_t plus : 1;
    uint8_t minus : 1;

    uint8_t hat;
    uint8_t sticks[4]; // always 0x80
    uint8_t : 8;

} __attribute__((packed));

static inline int switch_taiko_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       sizeof(struct taiko_input_report));
}

bool switch_taiko_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {HORI_VID, HORI_SWITCH_TAIKO_PID}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int switch_taiko_driver_ops_init(usb_input_device_t *device) {
    int ret;
    device->extension = WPAD_EXTENSION_TAIKO;
    device->wpadData.extension = WPAD_EXTENSION_TAIKO;
    device->format = WPAD_FORMAT_TAIKO;

    device->gravityUnit[0].acceleration[0] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[1] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[2] = ACCEL_ONE_G;
    ret = ps3_set_leds(device);
    if (ret < 0)
        return ret;

    return 0;
}

int switch_taiko_driver_ops_disconnect(usb_input_device_t *device) {
    return 0;
}

bool switch_taiko_report_input(const struct taiko_input_report *report, usb_input_device_t *device) {
    device->wpadData.buttons = 0;
    device->wpadData.home = report->home;
    device->wpadData.plus = report->plus;
    device->wpadData.minus = report->minus;
    device->wpadData.a = report->a;
    device->wpadData.b = report->b;
    device->wpadData.one = report->x;
    device->wpadData.two = report->y;
    device->wpadData.dpadUp = report->hat == 0 || report->hat == 1 || report->hat == 7;
    device->wpadData.dpadDown = report->hat == 3 || report->hat == 4 || report->hat == 5;
    device->wpadData.dpadLeft = report->hat == 5 || report->hat == 6 || report->hat == 7;
    device->wpadData.dpadRight = report->hat == 1 || report->hat == 2 || report->hat == 3;
    device->wpadData.extension_data.taiko.buttons = 0;
    device->wpadData.extension_data.taiko.centerLeft = report->centerLeft;
    device->wpadData.extension_data.taiko.centerRight = report->centerRight;
    device->wpadData.extension_data.taiko.rimLeft = report->rimLeft;
    device->wpadData.extension_data.taiko.rimRight = report->rimRight;

    device->wpadData.status = WPAD_STATUS_OK;

    return true;
}
int switch_taiko_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct taiko_input_report *report = (void *)device->usb_async_resp;
    switch_taiko_report_input(report, device);
    return switch_taiko_request_data(device);
}

const usb_device_driver_t switch_taiko_usb_device_driver = {
    .probe = switch_taiko_driver_ops_probe,
    .hid = true,
    .init = switch_taiko_driver_ops_init,
    .disconnect = switch_taiko_driver_ops_disconnect,
    .usb_async_resp = switch_taiko_driver_ops_usb_async_resp,
};
