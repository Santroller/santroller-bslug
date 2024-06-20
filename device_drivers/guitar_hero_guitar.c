#include "button_map.h"
#include "usb_hid.h"
#include "wiimote.h"
#include "rvl/WPAD.h"
#include <stdio.h>

#define GUITAR_ACC_RES_PER_G	113

struct guitar_input_report {
	uint8_t 			: 2;
	uint8_t pedal	: 1;
	uint8_t orange	: 1;
	uint8_t blue		: 1;
	uint8_t red		: 1;
	uint8_t green 	: 1;
	uint8_t yellow	: 1;

	uint8_t			: 3;
	uint8_t ps		: 1;
	uint8_t			: 2;
	uint8_t start	: 1;
	uint8_t select	: 1;

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

enum guitar_buttons_e {
	GUITAR_BUTTON_YELLOW,
	GUITAR_BUTTON_GREEN,
	GUITAR_BUTTON_RED,
	GUITAR_BUTTON_BLUE,
	GUITAR_BUTTON_ORANGE,
	GUITAR_BUTTON_UP,
	GUITAR_BUTTON_DOWN,
	GUITAR_BUTTON_LEFT,
	GUITAR_BUTTON_RIGHT,
	GUITAR_BUTTON_SP_PEDAL,
	GUITAR_BUTTON_SELECT,
	GUITAR_BUTTON_START,
	GUITAR_BUTTON_PS,
	GUITAR_BUTTON__NUM
};

enum guitar_analog_axis_e {
	GUITAR_ANALOG_AXIS_TAP_BAR,
	GUITAR_ANALOG_AXIS_WHAMMY_BAR,
	GUITAR_ANALOG_AXIS__NUM
};

#define MAX_ANALOG_AXIS GUITAR_ANALOG_AXIS__NUM

struct gh_guitar_private_data_t {
	struct {
		uint32_t buttons;
		uint8_t analog_axis[MAX_ANALOG_AXIS];
		int16_t acc_x, acc_y, acc_z;
	} input;
	uint8_t leds;
};

static const struct {
	uint16_t wiimote_button_map[GUITAR_BUTTON__NUM];
	uint16_t guitar_button_map[GUITAR_BUTTON__NUM];
	uint8_t guitar_analog_axis_map[GUITAR_ANALOG_AXIS__NUM];
} guitar_mapping =
	{
		.wiimote_button_map = {
			[GUITAR_BUTTON_PS] = WIIMOTE_BUTTON_HOME,
		},
		.guitar_analog_axis_map = {
			[GUITAR_ANALOG_AXIS_TAP_BAR] = BM_GUITAR_ANALOG_AXIS_TAP_BAR,
			[GUITAR_ANALOG_AXIS_WHAMMY_BAR] = BM_GUITAR_ANALOG_AXIS_WHAMMY_BAR,
		},
		.guitar_button_map = {
			[GUITAR_BUTTON_YELLOW] = GUITAR_CTRL_BUTTON_YELLOW,
			[GUITAR_BUTTON_RED] = GUITAR_CTRL_BUTTON_RED,
			[GUITAR_BUTTON_GREEN] = GUITAR_CTRL_BUTTON_GREEN,
			[GUITAR_BUTTON_BLUE] = GUITAR_CTRL_BUTTON_BLUE,
			[GUITAR_BUTTON_ORANGE] = GUITAR_CTRL_BUTTON_ORANGE,
			[GUITAR_BUTTON_UP] = GUITAR_CTRL_BUTTON_STRUM_UP,
			[GUITAR_BUTTON_DOWN] = GUITAR_CTRL_BUTTON_STRUM_DOWN,
			[GUITAR_BUTTON_START] = GUITAR_CTRL_BUTTON_PLUS,
			[GUITAR_BUTTON_SELECT] = GUITAR_CTRL_BUTTON_MINUS,
		}};

static inline void gh_guitar_get_buttons(const struct guitar_input_report *report, uint32_t *buttons) {
	uint32_t mask = 0;

#define MAP(field, button) \
	if (report->field)	 \
		mask |= BIT(button);

	MAP(green, GUITAR_BUTTON_GREEN)
	MAP(red, GUITAR_BUTTON_RED)
	MAP(yellow, GUITAR_BUTTON_YELLOW)
	MAP(blue, GUITAR_BUTTON_BLUE)
	MAP(orange, GUITAR_BUTTON_ORANGE)
	MAP(start, GUITAR_BUTTON_START)
	MAP(select, GUITAR_BUTTON_SELECT)
	MAP(pedal, GUITAR_BUTTON_SP_PEDAL)
	MAP(ps, GUITAR_BUTTON_PS)
#undef MAP
	if (report->hat == 0 || report->hat == 1 || report->hat == 7) {
		mask |= BIT(GUITAR_BUTTON_UP);
	}
	if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
		mask |= BIT(GUITAR_BUTTON_LEFT);
	}
	if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
		mask |= BIT(GUITAR_BUTTON_DOWN);
	}
	if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
		mask |= BIT(GUITAR_BUTTON_RIGHT);
	}

	*buttons = mask;
}


static inline void gh_guitar_get_analog_axis(const struct guitar_input_report *report,
										  uint8_t analog_axis[static MAX_ANALOG_AXIS]) {
	analog_axis[GUITAR_ANALOG_AXIS_TAP_BAR] = report->tap_bar;
	analog_axis[GUITAR_ANALOG_AXIS_WHAMMY_BAR] = report->whammy_bar;
}

static inline int gh_guitar_request_data(usb_input_device_t *device)
{
	return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
							   sizeof(struct guitar_input_report));
}


bool gh_guitar_driver_ops_probe(uint16_t vid, uint16_t pid)
{
	static const struct device_id_t compatible[] = {
		{SONY_INST_VID, GH_GUITAR_PID},
	};

	return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int gh_guitar_driver_ops_init(usb_input_device_t *device)
{
	int ret;
	if (device->extensionCallback) {
		device->extensionCallback(device->wiimote, WPAD_EXTENSION_GUITAR);
	}
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

static int gh_guitar_driver_update_leds(usb_input_device_t *device)
{
	// TODO: this
	return 0;
}

int gh_guitar_driver_ops_disconnect(usb_input_device_t *device)
{
	struct gh_guitar_private_data_t *priv = (void *)device->private_data;

	priv->leds = 0;

	return gh_guitar_driver_update_leds(device);
}

int gh_guitar_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot)
{
	struct gh_guitar_private_data_t *priv = (void *)device->private_data;

	priv->leds = slot;

	return gh_guitar_driver_update_leds(device);
}

bool gh_guitar_report_input(usb_input_device_t *device)
{
	struct gh_guitar_private_data_t *priv = (void *)device->private_data;

	bm_map_wiimote(GUITAR_BUTTON__NUM, priv->input.buttons,
				   guitar_mapping.wiimote_button_map,
				   &device->wpadData.buttons);
	device->wpadData.acceleration[0] = priv->input.acc_z;
	device->wpadData.acceleration[1] = priv->input.acc_x;
	device->wpadData.acceleration[2] = priv->input.acc_y;
	uint32_t buttons = priv->input.buttons;
	uint16_t guitar_buttons = 0;
	for (int i = 0; i < GUITAR_BUTTON__NUM; i++) {
		if (buttons & 1)
			guitar_buttons |= guitar_mapping.guitar_button_map[i];
		buttons >>= 1;
	}
	device->wpadData.extension_data.guitar.buttons = guitar_buttons;
	device->wpadData.extension_data.guitar.whammy = priv->input.analog_axis[GUITAR_ANALOG_AXIS_WHAMMY_BAR] - 0x80;
	device->wpadData.status = WPAD_STATUS_OK;
	// TODO: tap bar

	return true;
}
static uint32_t cpu_isr_disable(void) {
    uint32_t isr, tmp;
    asm volatile("mfmsr %0; rlwinm %1, %0, 0, 0xFFFF7FFF; mtmsr %1" : "=r"(isr), "=r"(tmp));
    return isr;
}
static void cpu_isr_restore(uint32_t isr) {
    uint32_t tmp;
    asm volatile("mfmsr %0; rlwimi %0, %1, 0, 0x8000; mtmsr %0" : "=&r"(tmp) : "r"(isr));
}
int gh_guitar_driver_ops_usb_async_resp(usb_input_device_t *device)
{
	uint32_t isr = cpu_isr_disable();
	struct gh_guitar_private_data_t *priv = (void *)device->private_data;
	struct guitar_input_report *report = (void *)device->usb_async_resp;
	gh_guitar_get_buttons(report, &priv->input.buttons);
	gh_guitar_get_analog_axis(report, priv->input.analog_axis);

	priv->input.acc_x = (int16_t)le16toh(report->acc_x) - 511;
	priv->input.acc_y = 511 - (int16_t)le16toh(report->acc_y);
	priv->input.acc_z = 511 - (int16_t)le16toh(report->acc_z);
	gh_guitar_report_input(device);
	cpu_isr_restore(isr);
	return gh_guitar_request_data(device);
}

const usb_device_driver_t gh_guitar_usb_device_driver = {
	.probe		= gh_guitar_driver_ops_probe,
	.init		= gh_guitar_driver_ops_init,
	.disconnect	= gh_guitar_driver_ops_disconnect,
	.slot_changed	= gh_guitar_driver_ops_slot_changed,
	.report_input	= gh_guitar_report_input,
	.usb_async_resp	= gh_guitar_driver_ops_usb_async_resp,
};
