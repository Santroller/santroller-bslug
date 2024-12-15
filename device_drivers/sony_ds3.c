#include <stddef.h>
#include <stdio.h>
#include "rvl/WPAD.h"
#include "usb_hid.h"
#include "usb.h"
#include "wiimote.h"

#define DS3_ACC_RES_PER_G	113

struct ds3_input_report {
	uint8_t report_id;
	uint8_t unk0;

	uint8_t left     : 1;
	uint8_t down     : 1;
	uint8_t right    : 1;
	uint8_t up       : 1;
	uint8_t start    : 1;
	uint8_t r3       : 1;
	uint8_t l3       : 1;
	uint8_t select   : 1;

	uint8_t square   : 1;
	uint8_t cross    : 1;
	uint8_t circle   : 1;
	uint8_t triangle : 1;
	uint8_t r1       : 1;
	uint8_t l1       : 1;
	uint8_t r2       : 1;
	uint8_t l2       : 1;

	uint8_t not_used : 7;
	uint8_t ps       : 1;

	uint8_t unk1;

	uint8_t left_x;
	uint8_t left_y;
	uint8_t right_x;
	uint8_t right_y;

	uint32_t unk2;

	uint8_t dpad_sens_up;
	uint8_t dpad_sens_right;
	uint8_t dpad_sens_down;
	uint8_t dpad_sens_left;

	uint8_t shoulder_sens_l2;
	uint8_t shoulder_sens_r2;
	uint8_t shoulder_sens_l1;
	uint8_t shoulder_sens_r1;

	uint8_t button_sens_triangle;
	uint8_t button_sens_circle;
	uint8_t button_sens_cross;
	uint8_t button_sens_square;

	uint16_t unk3;
	uint8_t  unk4;

	uint8_t status;
	uint8_t power_rating;
	uint8_t comm_status;

	uint32_t unk5;
	uint32_t unk6;
	uint8_t  unk7;

	uint16_t acc_x;
	uint16_t acc_y;
	uint16_t acc_z;
	uint16_t z_gyro;
} ATTRIBUTE_PACKED;

struct ds3_rumble {
	uint8_t duration_right;
	uint8_t power_right;
	uint8_t duration_left;
	uint8_t power_left;
};

struct ds3_private_data_t {
    uint8_t leds;
};

static inline int ds3_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       sizeof(struct ds3_input_report));
}

bool ds3_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {SONY_VID, 0x0268}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int ds3_set_operational(usb_input_device_t *device)
{
	static uint8_t buf[17] ATTRIBUTE_ALIGN(32);
	return usb_device_driver_issue_ctrl_transfer_async(device,
						     USB_REQTYPE_INTERFACE_GET,
						     USB_REQ_GETREPORT,
						     (USB_REPTYPE_FEATURE << 8) | 0xf2, 0,
						     buf, sizeof(buf));
}

int ds3_driver_ops_init(usb_input_device_t *device) {
    int ret;
    // DS3 to gh3 wiimote mappings
    // device->extension = WPAD_EXTENSION_NONE;
    // device->wpadData.extension = WPAD_EXTENSION_NONE;
    // device->format = WPAD_FORMAT_NONE;
    device->extension = WPAD_EXTENSION_CLASSIC;
    device->wpadData.extension = WPAD_EXTENSION_CLASSIC;
    device->format = WPAD_FORMAT_CLASSIC;
    
    device->gravityUnit[0].acceleration[0] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[1] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[2] = ACCEL_ONE_G;
    ret = ds3_set_operational(device);
    if (ret < 0)
        return ret;

    return 0;
}

static int ds3_driver_update_leds(usb_input_device_t *device) {
    // TODO: this
    return 0;
}

int ds3_driver_ops_disconnect(usb_input_device_t *device) {
    struct ds3_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return ds3_driver_update_leds(device);
}

int ds3_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot) {
    struct ds3_private_data_t *priv = (void *)device->private_data;

    priv->leds = slot;

    return ds3_driver_update_leds(device);
}

bool ds3_report_input(const struct ds3_input_report *report, usb_input_device_t *device) {
    // DS3 to GH3 wiimote mappings
    // device->wpadData.buttons = 0;
    // device->wpadData.home = report->ps;
    // device->wpadData.b = report->l2;
    // device->wpadData.dpadRight = report->l1;
    // device->wpadData.a = report->r1;
    // device->wpadData.one = report->r2;
    // device->wpadData.two = report->cross;
    // device->wpadData.dpadDown = report->down;
    // device->wpadData.dpadUp = report->up;
    // device->wpadData.plus = report->start;
    // device->wpadData.minus = report->select;

    device->wpadData.buttons = 0;
    device->wpadData.extension_data.classic.a = report->cross;
    device->wpadData.extension_data.classic.b = report->circle;
    device->wpadData.extension_data.classic.x = report->triangle;
    device->wpadData.extension_data.classic.y = report->square;
    device->wpadData.extension_data.classic.home = report->ps;
    device->wpadData.extension_data.classic.minus = report->select;
    device->wpadData.extension_data.classic.plus = report->start;
    device->wpadData.extension_data.classic.dpadUp = report->up;
    device->wpadData.extension_data.classic.dpadDown = report->down;
    device->wpadData.extension_data.classic.dpadLeft = report->left;
    device->wpadData.extension_data.classic.dpadRight = report->right;
    device->wpadData.extension_data.classic.zl = report->l1;
    device->wpadData.extension_data.classic.zr = report->r1;
    device->wpadData.extension_data.classic.lt = report->l2;
    device->wpadData.extension_data.classic.rt = report->r2;
    device->wpadData.extension_data.classic.trigger[0] = report->shoulder_sens_l2;
    device->wpadData.extension_data.classic.trigger[1] = report->shoulder_sens_r2;
    device->wpadData.extension_data.classic.leftStick[0] = (report->left_x << 2) - 512;
    device->wpadData.extension_data.classic.leftStick[1] = (report->left_y << 2) - 512;
    device->wpadData.extension_data.classic.rightStick[0] = (report->right_x << 2) - 512;
    device->wpadData.extension_data.classic.rightStick[1] = (report->right_y << 2) - 512;



    return true;
}
int ds3_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct ds3_input_report *report = (void *)device->usb_async_resp;
    ds3_report_input(report, device);
    return ds3_request_data(device);
}

const usb_device_driver_t ds3_usb_device_driver = {
    .probe = ds3_driver_ops_probe,
    .hid = true,
    .init = ds3_driver_ops_init,
    .disconnect = ds3_driver_ops_disconnect,
    .slot_changed = ds3_driver_ops_slot_changed,
    .usb_async_resp = ds3_driver_ops_usb_async_resp,
};
