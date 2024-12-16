#include "rvl/WPAD.h"
#include "usb_hid.h"
#include "wiimote.h"

struct drum_input_report {
    uint8_t : 2;
    uint8_t kick : 1;
    uint8_t orange : 1;
    uint8_t yellow : 1;
    uint8_t red : 1;
    uint8_t green : 1;
    uint8_t blue : 1;

    uint8_t : 3;
    uint8_t ps : 1;
    uint8_t : 2;
    uint8_t start : 1;
    uint8_t select : 1;

    uint8_t hat;
    uint8_t unused;
    uint8_t unused2;
    uint8_t whammy_bar;
    uint8_t tap_bar;
    uint8_t pressure_yellow;
    uint8_t pressure_red;
    uint8_t pressure_green;
    uint8_t pressure_blue;
    uint8_t pressure_kick;
    uint8_t pressure_orange;
    uint8_t unused3[2];
    uint16_t unused4[4];

} __attribute__((packed));

struct gh_drum_private_data_t {
    uint8_t leds;
};

static inline int gh_drum_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp,
                                                       device->max_packet_len_in);
}

static int gh_drum_driver_update_leds(usb_input_device_t *device) {
    // TODO: this
    return 0;
}

bool gh_drum_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {SONY_INST_VID, GH_DRUM_PID}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int gh_drum_driver_ops_init(usb_input_device_t *device) {
    int ret;
    device->extension = WPAD_EXTENSION_DRUM;
    device->wpadData.extension = WPAD_EXTENSION_DRUM;
    device->format = WPAD_FORMAT_DRUM;

    ret = ps3_set_leds(device);
    if (ret < 0)
        return ret;

    return 0;
}

int gh_drum_driver_ops_disconnect(usb_input_device_t *device) {
    struct gh_drum_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return gh_drum_driver_update_leds(device);
}

bool gh_drum_report_input(const struct drum_input_report *report, usb_input_device_t *device) {

    device->wpadData.buttons = 0;
	device->wpadData.home = report->ps;
	device->wpadData.extension_data.drum.buttons = 0;
	device->wpadData.extension_data.drum.green = report->green;
	device->wpadData.extension_data.drum.red = report->red;
	device->wpadData.extension_data.drum.yellow = report->yellow;
	device->wpadData.extension_data.drum.blue = report->blue;
	device->wpadData.extension_data.drum.orange = report->orange;
	device->wpadData.extension_data.drum.pedal = report->kick;
	device->wpadData.extension_data.drum.plus = report->start;
	device->wpadData.extension_data.drum.minus = report->select;
	
    device->wpadData.extension_data.drum.stick[0] = 0;
    device->wpadData.extension_data.drum.stick[1] = 0;

    // TODO: Velcocity

	// UP
	if (report->hat == 0 || report->hat == 1 || report->hat == 7) {
		device->wpadData.extension_data.guitar.stick[1] = 10;
	}
	// DOWN
	if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
		device->wpadData.extension_data.guitar.stick[1] = -10;
	}

	// LEFT
	if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
		device->wpadData.extension_data.guitar.stick[0] = -10;
	}

	// RIGHT
	if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
		device->wpadData.extension_data.guitar.stick[0] = 10;
	}

    device->wpadData.status = WPAD_STATUS_OK;
    return true;
}

int gh_drum_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct drum_input_report *report = (void *)device->usb_async_resp;
    gh_drum_report_input(report, device);
    return gh_drum_request_data(device);
}

const usb_device_driver_t gh_drum_usb_device_driver = {
    .probe = gh_drum_driver_ops_probe,
    .hid = true,
    .init = gh_drum_driver_ops_init,
    .disconnect = gh_drum_driver_ops_disconnect,
    .usb_async_resp = gh_drum_driver_ops_usb_async_resp,
};
