#include <stddef.h>
#include <stdio.h>

#include "rvl/WPAD.h"
#include "usb.h"
#include "usb_hid.h"
#include "wiimote.h"

#define DS4_TOUCHPAD_W 1920
#define DS4_TOUCHPAD_H 940
#define DS4_ACC_RES_PER_G 8192

struct ds4_input_report {
    uint8_t report_id;
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;

    uint8_t triangle : 1;
    uint8_t circle : 1;
    uint8_t cross : 1;
    uint8_t square : 1;
    uint8_t dpad : 4;

    uint8_t r3 : 1;
    uint8_t l3 : 1;
    uint8_t options : 1;
    uint8_t share : 1;
    uint8_t r2 : 1;
    uint8_t l2 : 1;
    uint8_t r1 : 1;
    uint8_t l1 : 1;

    uint8_t cnt1 : 6;
    uint8_t tpad : 1;
    uint8_t ps : 1;

    uint8_t l_trigger;
    uint8_t r_trigger;

    uint8_t cnt2;
    uint8_t cnt3;

    uint8_t battery;

    union {
        int16_t roll;
        int16_t gyro_z;
    };

    union {
        int16_t yaw;
        int16_t gyro_y;
    };

    union {
        int16_t pitch;
        int16_t gyro_x;
    };

    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    uint8_t unk1[5];

    uint8_t padding : 1;
    uint8_t microphone : 1;
    uint8_t headphones : 1;
    uint8_t usb_plugged : 1;
    uint8_t battery_level : 4;

    uint8_t unk2[2];
    uint8_t trackpadpackets;
    uint8_t packetcnt;

    uint8_t finger1_nactive : 1;
    uint8_t finger1_id : 7;
    uint8_t finger1_x_lo;
    uint8_t finger1_y_lo : 4;
    uint8_t finger1_x_hi : 4;
    uint8_t finger1_y_hi;

    uint8_t finger2_nactive : 1;
    uint8_t finger2_id : 7;
    uint8_t finger2_x_lo;
    uint8_t finger2_y_lo : 4;
    uint8_t finger2_x_hi : 4;
    uint8_t finger2_y_hi;
} ATTRIBUTE_PACKED;
enum bm_ir_emulation_mode_e {
    BM_IR_EMULATION_MODE_NONE,
    BM_IR_EMULATION_MODE_DIRECT,
    BM_IR_EMULATION_MODE_RELATIVE_ANALOG_AXIS,
    BM_IR_EMULATION_MODE_ABSOLUTE_ANALOG_AXIS,
};
enum bm_ir_axis_e {
    BM_IR_AXIS_NONE,
    BM_IR_AXIS_X,
    BM_IR_AXIS_Y,
    BM_IR_AXIS__NUM = BM_IR_AXIS_Y,
};
struct bm_ir_emulation_state_t {
    uint16_t position[BM_IR_AXIS__NUM];
};
struct ds4_private_data_t {
    enum bm_ir_emulation_mode_e ir_emu_mode;
    uint8_t mapping;
    uint8_t ir_emu_mode_idx;
    uint8_t leds;
    bool rumble_on;
    bool switch_mapping;
    bool switch_ir_emu_mode;
};

static inline int ds4_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       64);
}

bool ds4_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {SONY_VID, 0x05c4},
        {SONY_VID, 0x09cc}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}


static inline int ds4_set_leds_rumble(usb_input_device_t *device, uint8_t r, uint8_t g, uint8_t b,
				      uint8_t rumble_small, uint8_t rumble_large)
{
	uint8_t buf[] ATTRIBUTE_ALIGN(32) = {
		0x05, // Report ID
		0x03, 0x00, 0x00,
		rumble_small, // Fast motor
		rumble_large, // Slow motor
		r, g, b, // RGB
		0x00, // LED on duration
		0x00  // LED off duration
	};

	return usb_device_driver_issue_intr_transfer_async(device, 1, buf, sizeof(buf));
}

static int ds4_driver_update_leds_rumble(usb_input_device_t *device)
{
	uint8_t index;

	static const uint8_t colors[5][3] = {
		{  0,   0,   0},
		{  0,   0,  32},
		{ 32,   0,   0},
		{  0,  32,   0},
		{ 32,   0,  32},
	};

	index = (device->wiimote+1) % ARRAY_SIZE(colors);

	uint8_t r = colors[index][0],
	   g = colors[index][1],
	   b = colors[index][2];

	return ds4_set_leds_rumble(device, r, g, b, device->rumble_on * 192, 0);
}

int ds4_driver_ops_init(usb_input_device_t *device) {
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

    return ds4_driver_update_leds_rumble(device);
}

static int ds4_driver_update_leds(usb_input_device_t *device) {
    // TODO: this
    return 0;
}

int ds4_driver_ops_disconnect(usb_input_device_t *device) {
    struct ds4_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return ds4_driver_update_leds(device);
}

bool ds4_report_input(const struct ds4_input_report *report, usb_input_device_t *device) {
    // DS3 to GH3 wiimote mappings
    // device->wpadData.buttons = 0;
    // device->wpadData.home = report->ps;
    // device->wpadData.b = report->l2;
    // device->wpadData.dpadRight = report->l1;
    // device->wpadData.a = report->r1;
    // device->wpadData.one = report->r2;
    // device->wpadData.two = report->cross;
    // device->wpadData.dpadDown = report->dpad == 3 || report->dpad == 4 || report->dpad == 5;
    // device->wpadData.dpadUp = report->dpad == 0 || report->dpad == 1 || report->dpad == 7;
    // device->wpadData.plus = report->options;
    // device->wpadData.minus = report->share;
    device->wpadData.buttons = 0;
    device->wpadData.extension_data.classic.a = report->cross;
    device->wpadData.extension_data.classic.b = report->circle;
    device->wpadData.extension_data.classic.x = report->triangle;
    device->wpadData.extension_data.classic.y = report->square;
    device->wpadData.extension_data.classic.home = report->ps;
    device->wpadData.extension_data.classic.minus = report->options;
    device->wpadData.extension_data.classic.plus = report->share;
    device->wpadData.extension_data.classic.dpadUp = report->dpad == 0 || report->dpad == 1 || report->dpad == 7;
    device->wpadData.extension_data.classic.dpadDown = report->dpad == 3 || report->dpad == 4 || report->dpad == 5;
    device->wpadData.extension_data.classic.dpadLeft = report->dpad == 5 || report->dpad == 6 || report->dpad == 7;
    device->wpadData.extension_data.classic.dpadRight = report->dpad == 1 || report->dpad == 2 || report->dpad == 3;
    device->wpadData.extension_data.classic.zl = report->l1;
    device->wpadData.extension_data.classic.zr = report->r1;
    device->wpadData.extension_data.classic.lt = report->l2;
    device->wpadData.extension_data.classic.rt = report->r2;
    device->wpadData.extension_data.classic.trigger[0] = report->l_trigger;
    device->wpadData.extension_data.classic.trigger[1] = report->r_trigger;
    device->wpadData.extension_data.classic.leftStick[0] = (report->left_x << 2) - 512;
    device->wpadData.extension_data.classic.leftStick[1] = (report->left_y << 2) - 512;
    device->wpadData.extension_data.classic.rightStick[0] = (report->right_x << 2) - 512;
    device->wpadData.extension_data.classic.rightStick[1] = (report->right_y << 2) - 512;

    return true;
}
int ds4_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct ds4_input_report *report = (void *)device->usb_async_resp;
    ds4_report_input(report, device);

    if (device->last_rumble_on != device->rumble_on) {
        device->last_rumble_on = device->rumble_on;
        return ds4_driver_update_leds_rumble(device);
    }
    return ds4_request_data(device);
}

const usb_device_driver_t ds4_usb_device_driver = {
    .probe = ds4_driver_ops_probe,
    .hid = true,
    .init = ds4_driver_ops_init,
    .disconnect = ds4_driver_ops_disconnect,
    .usb_async_resp = ds4_driver_ops_usb_async_resp,
};
