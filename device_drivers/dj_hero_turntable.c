#include "defines.h"
#include "usb_hid.h"
#include "wiimote.h"
#include "rvl/WPAD.h"
#include <stdio.h>
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
	uint8_t unused3[6];
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

struct turntable_private_data_t {
	uint8_t leds;
};



static inline int turntable_request_data(usb_input_device_t *device)
{
	return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
							   sizeof(struct turntable_input_report));
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
	device->extension = WPAD_EXTENSION_TURNTABLE;
	device->wpadData.extension = WPAD_EXTENSION_TURNTABLE;
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

bool turntable_report_input(const struct turntable_input_report *report, usb_input_device_t *device)
{
	device->wpadData.buttons = 0;
	device->wpadData.home = report->ps;
	device->wpadData.extension_data.turntable.buttons = 0;
	device->wpadData.extension_data.turntable.leftBlue = report->left_blue;
	device->wpadData.extension_data.turntable.leftGreen = report->left_green;
	device->wpadData.extension_data.turntable.leftRed = report->left_red;
	device->wpadData.extension_data.turntable.rightBlue = report->right_blue;
	device->wpadData.extension_data.turntable.rightGreen = report->right_green;
	device->wpadData.extension_data.turntable.rightRed = report->right_red;
	device->wpadData.extension_data.turntable.euphoria = report->triangle_euphoria;
	device->wpadData.extension_data.turntable.plus = report->start;
	device->wpadData.extension_data.turntable.minus = report->select;

    device->wpadData.extension_data.turntable.stick[0] = 0;
    device->wpadData.extension_data.turntable.stick[1] = 0;
	// UP
	if (report->hat == 0 || report->hat == 1 || report->hat == 7) {
		device->wpadData.extension_data.turntable.stick[1] = 10;
	}
	// DOWN
	if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
		device->wpadData.extension_data.turntable.stick[1] = -10;
	}

	// LEFT
	if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
		device->wpadData.extension_data.turntable.stick[0] = -10;
	}

	// RIGHT
	if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
		device->wpadData.extension_data.turntable.stick[0] = 10;
	}
    // TODO: check turntable again
	uint8_t ltt = report->left_turn_table_velocity >> 3;
	device->wpadData.extension_data.turntable.ltt4 = ltt & (1 << 4);
	device->wpadData.extension_data.turntable.ltt30 = ltt;
	uint8_t rtt = report->right_turn_table_velocity >> 4;
	device->wpadData.extension_data.turntable.rtt5 = !(rtt & (1 << 5));
	device->wpadData.extension_data.turntable.rtt40 = rtt;
	device->wpadData.extension_data.turntable.crossFader = report->cross_fader >> 6;
	device->wpadData.extension_data.turntable.effectsDial = report->effects_knob >> 5;

	device->wpadData.status = WPAD_STATUS_OK;

	return true;
}

int turntable_driver_ops_usb_async_resp(usb_input_device_t *device)
{
	struct turntable_input_report *report = (void *)device->usb_async_resp;
	turntable_report_input(report, device);

	return turntable_request_data(device);
}

const usb_device_driver_t turntable_usb_device_driver = {
	.probe		= turntable_driver_ops_probe,
    .hid = true,
	.init		= turntable_driver_ops_init,
	.disconnect	= turntable_driver_ops_disconnect,
	.slot_changed	= turntable_driver_ops_slot_changed,
	.usb_async_resp	= turntable_driver_ops_usb_async_resp,
};
