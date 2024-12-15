#include "rvl/WPAD.h"
#include "usb_hid.h"
#include "wiimote.h"

#define GUITAR_ACC_RES_PER_G 113

struct guitar_input_report {
    uint8_t : 2;
    uint8_t pedal : 1;
    uint8_t orange : 1;
    uint8_t blue : 1;
    uint8_t red : 1;
    uint8_t green : 1;
    uint8_t yellow : 1;

    uint8_t : 3;
    uint8_t ps : 1;
    uint8_t : 2;
    uint8_t start : 1;
    uint8_t select : 1;

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

struct gh_guitar_private_data_t {
    uint8_t leds;
};

static inline int gh_guitar_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       sizeof(struct guitar_input_report));
}

bool gh_guitar_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {SONY_INST_VID, GH_GUITAR_PID}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int gh_guitar_driver_ops_init(usb_input_device_t *device) {
    int ret;
    device->extension = WPAD_EXTENSION_GUITAR;
    device->wpadData.extension = WPAD_EXTENSION_GUITAR;
    device->format = WPAD_FORMAT_GUITAR;
    
    device->gravityUnit[0].acceleration[0] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[1] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[2] = ACCEL_ONE_G;
    ret = gh_guitar_request_data(device);
    if (ret < 0)
        return ret;

    return 0;
}

static int gh_guitar_driver_update_leds(usb_input_device_t *device) {
    // TODO: this
    return 0;
}

int gh_guitar_driver_ops_disconnect(usb_input_device_t *device) {
    struct gh_guitar_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return gh_guitar_driver_update_leds(device);
}

int gh_guitar_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot) {
    struct gh_guitar_private_data_t *priv = (void *)device->private_data;

    priv->leds = slot;

    return gh_guitar_driver_update_leds(device);
}

bool gh_guitar_report_input(const struct guitar_input_report *report, usb_input_device_t *device) {
    device->wpadData.acceleration[0] = (int16_t)le16toh(report->acc_x) - 511;
    device->wpadData.acceleration[1] = 511 - (int16_t)le16toh(report->acc_y);
    device->wpadData.acceleration[2] = 511 - (int16_t)le16toh(report->acc_z);

    device->wpadData.buttons = 0;
    device->wpadData.home = report->ps;
    device->wpadData.extension_data.guitar.buttons = 0;
    device->wpadData.extension_data.guitar.green = report->green;
    device->wpadData.extension_data.guitar.red = report->red;
    device->wpadData.extension_data.guitar.yellow = report->yellow;
    device->wpadData.extension_data.guitar.blue = report->blue;
    device->wpadData.extension_data.guitar.orange = report->orange;
    device->wpadData.extension_data.guitar.pedal = report->pedal;
    device->wpadData.extension_data.guitar.plus = report->start;
    device->wpadData.extension_data.guitar.minus = report->select;

    device->wpadData.extension_data.guitar.stick[0] = 0;
    device->wpadData.extension_data.guitar.stick[1] = 0;
    device->wpadData.extension_data.guitar.dpadUp = report->hat == 0 || report->hat == 1 || report->hat == 7;
    device->wpadData.extension_data.guitar.dpadDown = report->hat == 3 || report->hat == 4 || report->hat == 5;
    // UP
    // if (report->hat == 0 || report->hat == 1 || report->hat == 7) {

    // device->wpadData.extension_data.guitar.stick[1] = 10;
    // }
    // DOWN
    // if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
    // device->wpadData.extension_data.guitar.stick[1] = -10;
    // }

    // LEFT
    if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
        device->wpadData.extension_data.guitar.stick[0] = -10;
    }

    // RIGHT
    if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
        device->wpadData.extension_data.guitar.stick[0] = 10;
    }

    device->wpadData.extension_data.guitar.whammy = report->whammy_bar;
    if (device->old_wpad) {
        device->wpadData.extension_data.guitar.whammy = report->whammy_bar - 0x80;
    }
    device->wpadData.status = WPAD_STATUS_OK;

    // TODO: tap bar
    device->wpadData.extension_data.guitar.tapbar = 0x1E0;

    return true;
}
int gh_guitar_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct guitar_input_report *report = (void *)device->usb_async_resp;
    gh_guitar_report_input(report, device);
    return gh_guitar_request_data(device);
}

const usb_device_driver_t gh_guitar_usb_device_driver = {
    .probe = gh_guitar_driver_ops_probe,
    .hid = true,
    .init = gh_guitar_driver_ops_init,
    .disconnect = gh_guitar_driver_ops_disconnect,
    .slot_changed = gh_guitar_driver_ops_slot_changed,
    .usb_async_resp = gh_guitar_driver_ops_usb_async_resp,
};
