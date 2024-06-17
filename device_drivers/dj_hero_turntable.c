#include "button_map.h"
#include "defines.h"
#include "usb_hid.h"
#include "wiimote.h"
#include "rvl/WPAD.h"

struct turntable_input_report {
	uint8_t			: 4;
	uint8_t triangle_euphoria : 1;
	uint8_t circle	: 1;
	uint8_t cross	: 1;
	uint8_t square	: 1;

	uint8_t			: 3;
	uint8_t ps		: 1;
	uint8_t			: 2;
	uint8_t start	: 1;
	uint8_t select	: 1;

	uint8_t hat;
	uint8_t unused;
	uint8_t unused2;
	uint8_t left_turn_table_velocity;
	uint8_t right_turn_table_velocity;
	uint8_t pressure_yellow;
	uint8_t pressure_red;
	uint8_t pressure_green;
	uint8_t pressure_blue;
	uint8_t pressure_kick;
	uint8_t pressure_orange;
	uint8_t unused3[2];
	uint16_t effects_knob;
	uint16_t cross_fader;
	uint16_t right_green : 1;
	uint16_t right_red : 1;
	uint16_t right_blue : 1;
	uint16_t left_green : 1;
	uint16_t left_red : 1;
	uint16_t left_blue : 1;
	uint16_t : 3;
	uint16_t table_neutral : 1;
	uint16_t : 6;
	uint16_t : 16;

} __attribute__((packed));

enum turntable_buttons_e {
	TURNTABLE_BUTTON_SQUARE,
	TURNTABLE_BUTTON_CROSS,
	TURNTABLE_BUTTON_CIRCLE,
	TURNTABLE_BUTTON_TRIANGLE_EUPHORIA,
	TURNTABLE_BUTTON_SELECT,
	TURNTABLE_BUTTON_START,
	TURNTABLE_BUTTON_LEFT_GREEN,
	TURNTABLE_BUTTON_LEFT_RED,
	TURNTABLE_BUTTON_LEFT_BLUE,
	TURNTABLE_BUTTON_RIGHT_GREEN,
	TURNTABLE_BUTTON_RIGHT_RED,
	TURNTABLE_BUTTON_RIGHT_BLUE,
	TURNTABLE_BUTTON_UP,
	TURNTABLE_BUTTON_DOWN,
	TURNTABLE_BUTTON_LEFT,
	TURNTABLE_BUTTON_RIGHT,
	TURNTABLE_BUTTON_PS,
	TURNTABLE_BUTTON__NUM
};

enum turntable_analog_axis_e {
	TURNTABLE_ANALOG_AXIS_LEFT_VELOCITY,
	TURNTABLE_ANALOG_AXIS_RIGHT_VELOCITY,
	TURNTABLE_ANALOG_AXIS_CROSS_FADER,
	TURNTABLE_ANALOG_AXIS_EFFECTS_KNOB,
	TURNTABLE_ANALOG_AXIS__NUM
};

#define MAX_ANALOG_AXIS TURNTABLE_ANALOG_AXIS__NUM

struct turntable_private_data_t {
	struct {
		uint32_t buttons;
		uint8_t analog_axis[MAX_ANALOG_AXIS];
	} input;
	uint8_t leds;
};

static const struct {
	uint16_t wiimote_button_map[TURNTABLE_BUTTON__NUM];
	uint16_t turntable_button_map[TURNTABLE_BUTTON__NUM];
	uint8_t turntable_analog_axis_map[TURNTABLE_ANALOG_AXIS__NUM];
} turntable_mapping =
	{
		.wiimote_button_map = {
			[TURNTABLE_BUTTON_PS] = WIIMOTE_BUTTON_HOME,
		},
		.turntable_analog_axis_map = {
			[TURNTABLE_ANALOG_AXIS_LEFT_VELOCITY] = BM_TURNTABLE_ANALOG_LEFT_TURNTABLE_VELOCITY,
			[TURNTABLE_ANALOG_AXIS_RIGHT_VELOCITY] = BM_TURNTABLE_ANALOG_RIGHT_TURNTABLE_VELOCITY,
			[TURNTABLE_ANALOG_AXIS_CROSS_FADER] = BM_TURNTABLE_ANALOG_CROSS_FADER,
			[TURNTABLE_ANALOG_AXIS_EFFECTS_KNOB] = BM_TURNTABLE_ANALOG_EFFECTS_DIAL,
		},
		.turntable_button_map = {
			[TURNTABLE_BUTTON_LEFT_GREEN] = TURNTABLE_CTRL_BUTTON_LEFT_GREEN,
			[TURNTABLE_BUTTON_LEFT_RED] = TURNTABLE_CTRL_BUTTON_LEFT_RED,
			[TURNTABLE_BUTTON_LEFT_BLUE] = TURNTABLE_CTRL_BUTTON_LEFT_BLUE,
			[TURNTABLE_BUTTON_RIGHT_GREEN] = TURNTABLE_CTRL_BUTTON_RIGHT_GREEN,
			[TURNTABLE_BUTTON_RIGHT_RED] = TURNTABLE_CTRL_BUTTON_RIGHT_RED,
			[TURNTABLE_BUTTON_RIGHT_BLUE] = TURNTABLE_CTRL_BUTTON_RIGHT_BLUE,
			[TURNTABLE_BUTTON_CROSS] = TURNTABLE_CTRL_BUTTON_LEFT_GREEN | TURNTABLE_CTRL_BUTTON_RIGHT_GREEN,
			[TURNTABLE_BUTTON_CIRCLE] = TURNTABLE_CTRL_BUTTON_LEFT_RED | TURNTABLE_CTRL_BUTTON_RIGHT_RED,
			[TURNTABLE_BUTTON_SQUARE] = TURNTABLE_CTRL_BUTTON_LEFT_BLUE | TURNTABLE_CTRL_BUTTON_RIGHT_BLUE,
			[TURNTABLE_BUTTON_START] = TURNTABLE_CTRL_BUTTON_PLUS,
			[TURNTABLE_BUTTON_SELECT] = TURNTABLE_CTRL_BUTTON_MINUS,
		}};

static inline void turntable_get_buttons(const struct turntable_input_report *report, uint32_t *buttons) {
	uint32_t mask = 0;

#define MAP(field, button) \
	if (report->field)	 \
		mask |= BIT(button);

	MAP(left_green, TURNTABLE_BUTTON_LEFT_GREEN)
	MAP(left_red, TURNTABLE_BUTTON_LEFT_RED)
	MAP(left_blue, TURNTABLE_BUTTON_LEFT_BLUE)
	MAP(right_green, TURNTABLE_BUTTON_RIGHT_GREEN)
	MAP(right_red, TURNTABLE_BUTTON_RIGHT_RED)
	MAP(right_blue, TURNTABLE_BUTTON_RIGHT_BLUE)
	MAP(cross, TURNTABLE_BUTTON_CROSS)
	MAP(circle, TURNTABLE_BUTTON_CIRCLE)
	MAP(square, TURNTABLE_BUTTON_SQUARE)
	MAP(triangle_euphoria, TURNTABLE_BUTTON_TRIANGLE_EUPHORIA)
	MAP(select, TURNTABLE_BUTTON_SELECT)
	MAP(ps, TURNTABLE_BUTTON_PS)
#undef MAP
	if (report->hat == 0 || report->hat == 1 || report->hat == 7) {
		mask |= BIT(TURNTABLE_BUTTON_UP);
	}
	if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
		mask |= BIT(TURNTABLE_BUTTON_LEFT);
	}
	if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
		mask |= BIT(TURNTABLE_BUTTON_DOWN);
	}
	if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
		mask |= BIT(TURNTABLE_BUTTON_RIGHT);
	}

	*buttons = mask;
}


static inline void turntable_get_analog_axis(const struct turntable_input_report *report,
										uint8_t analog_axis[static MAX_ANALOG_AXIS]) {
	analog_axis[TURNTABLE_ANALOG_AXIS_CROSS_FADER] = report->cross_fader;
	analog_axis[TURNTABLE_ANALOG_AXIS_EFFECTS_KNOB] = report->effects_knob;
	analog_axis[TURNTABLE_ANALOG_AXIS_LEFT_VELOCITY] = report->left_turn_table_velocity;
	analog_axis[TURNTABLE_ANALOG_AXIS_RIGHT_VELOCITY] = report->right_turn_table_velocity;
}


static inline int turntable_request_data(usb_input_device_t *device)
{
	return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
							   device->max_packet_len_in);
}

static int turntable_driver_update_leds(usb_input_device_t *device)
{
	// TODO: this
	return 0;
}


bool turntable_driver_ops_probe(uint16_t vid, uint16_t pid)
{
	static const struct device_id_t compatible[] = {
		{SONY_INST_VID, DJ_TURNTABLE_PID}
	};

	return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int turntable_driver_ops_init(usb_input_device_t *device)
{
	int ret;
	struct turntable_private_data_t *priv = (void *)device->private_data;
	
	priv->leds = 0;

	if (device->extensionCallback) {
		device->extensionCallback(device->wiimote, WPAD_EXTENSION_TURNTABLE);
	}

	device->extension = WPAD_EXTENSION_TURNTABLE;
	device->format = WPAD_FORMAT_TURNTABLE;

	ret = turntable_request_data(device);
	if (ret < 0)
		return ret;

	return 0;
}

int turntable_driver_ops_disconnect(usb_input_device_t *device)
{
	struct turntable_private_data_t *priv = (void *)device->private_data;

	priv->leds = 0;

	return turntable_driver_update_leds(device);
}

int turntable_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot)
{
	struct turntable_private_data_t *priv = (void *)device->private_data;

	priv->leds = slot;

	return turntable_driver_update_leds(device);
}

bool turntable_report_input(usb_input_device_t *device)
{
	struct turntable_private_data_t *priv = (void *)device->private_data;
	uint16_t wiimote_buttons = 0;
	union wiimote_extension_data_t extension_data;

	bm_map_wiimote(TURNTABLE_BUTTON__NUM, priv->input.buttons,
				   turntable_mapping.wiimote_button_map,
				   &wiimote_buttons);

	bm_map_turntable(TURNTABLE_BUTTON__NUM, priv->input.buttons,
				  TURNTABLE_ANALOG_AXIS__NUM, priv->input.analog_axis,
				  turntable_mapping.turntable_button_map,
				  turntable_mapping.turntable_analog_axis_map,
				  &extension_data.turntable);

	return true;
}

int turntable_driver_ops_usb_async_resp(usb_input_device_t *device)
{
	struct turntable_private_data_t *priv = (void *)device->private_data;
	struct turntable_input_report *report = (void *)device->usb_async_resp;
	turntable_get_buttons(report, &priv->input.buttons);
	turntable_get_analog_axis(report, priv->input.analog_axis);

	return turntable_request_data(device);
}

const usb_device_driver_t turntable_usb_device_driver = {
	.probe		= turntable_driver_ops_probe,
	.init		= turntable_driver_ops_init,
	.disconnect	= turntable_driver_ops_disconnect,
	.slot_changed	= turntable_driver_ops_slot_changed,
	.report_input	= turntable_report_input,
	.usb_async_resp	= turntable_driver_ops_usb_async_resp,
};
