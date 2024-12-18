#include "rvl/WPAD.h"
#include "usb_hid.h"
#include "wiimote.h"


    // y =          01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // b =          00, 02, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // a =          01, 00, 00, 08, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // x =          01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // l =          00, 00, 00, 00, 10, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // r =          00, 00, 00, 00, 00, 20, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // leftrim =    00, 00, 00, 00, 00, 00, 40, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    // rightrim =   00, 00, 00, 00, 00, 00, 00, 80, 00, 00, 00, 00, 00, 00, 00, 00,
    // - =          00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00,
    // + =          00, 00, 00, 00, 00, 00, 00, 00, 00, 02, 00, 00, 00, 00, 00, 00,
    // centerl =    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 04, 00, 00, 00, 00, 00,
    // centerr =    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 08, 00, 00, 00, 00,
    // home =       00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 10, 00, 00, 00,
    // capture =    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 20, 00, 00,
#define GUITAR_ACC_RES_PER_G 113
struct taiko_input_report {
    uint8_t rimRight: 1;
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
    uint8_t centerLeft: 1;
    uint8_t plus : 1;
    uint8_t minus: 1;

    uint8_t hat;

    uint8_t unused0;
    uint8_t unused1;
    uint8_t whammy_bar;
    uint8_t tap_bar;

    uint8_t pressure_dpadRight_yellow;
    uint8_t pressure_dpadLeft;
    uint8_t pressure_dpadUp_green;
    uint8_t pressure_dpadDown_orange;
    uint8_t pressure_blue;
    uint8_t pressure_red;
    uint8_t unused2[6];

    // Reminder that these values are 10-bit in range
    uint16_t acc_x;
    uint16_t acc_z;
    uint16_t acc_y;
    uint16_t z_gyro;

} __attribute__((packed));

struct switch_taiko_private_data_t {
    uint8_t leds;
};

static inline int switch_taiko_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       sizeof(struct taiko_input_report));
}

bool switch_taiko_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {0x0f0d, 0x00f0}};

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

static int switch_taiko_driver_update_leds(usb_input_device_t *device) {
    // TODO: this
    return 0;
}

int switch_taiko_driver_ops_disconnect(usb_input_device_t *device) {
    struct switch_taiko_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return switch_taiko_driver_update_leds(device);
}

bool switch_taiko_report_input(const struct taiko_input_report *report, usb_input_device_t *device) {
    device->wpadData.acceleration[0] = (int16_t)le16toh(report->acc_x) - 511;
    device->wpadData.acceleration[1] = 511 - (int16_t)le16toh(report->acc_y);
    device->wpadData.acceleration[2] = 511 - (int16_t)le16toh(report->acc_z);

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
