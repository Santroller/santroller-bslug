#include "button_map.h"
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

enum drum_buttons_e {
    DRUM_BUTTON_YELLOW,
    DRUM_BUTTON_GREEN,
    DRUM_BUTTON_RED,
    DRUM_BUTTON_BLUE,
    DRUM_BUTTON_ORANGE,
    DRUM_BUTTON_UP,
    DRUM_BUTTON_DOWN,
    DRUM_BUTTON_LEFT,
    DRUM_BUTTON_RIGHT,
    DRUM_BUTTON_KICK,
    DRUM_BUTTON_SELECT,
    DRUM_BUTTON_START,
    DRUM_BUTTON_PS,
    DRUM_BUTTON__NUM
};

enum drum_analog_axis_e {
    DRUM_ANALOG_AXIS_GREEN,
    DRUM_ANALOG_AXIS_RED,
    DRUM_ANALOG_AXIS_YELLOW,
    DRUM_ANALOG_AXIS_BLUE,
    DRUM_ANALOG_AXIS_ORANGE,
    DRUM_ANALOG_AXIS_KICK,
    DRUM_ANALOG_AXIS__NUM
};

#define MAX_ANALOG_AXIS DRUM_ANALOG_AXIS__NUM

struct gh_drum_private_data_t {
    struct {
        uint32_t buttons;
        uint8_t analog_axis[MAX_ANALOG_AXIS];
    } input;
    uint8_t leds;
};

static const struct {
    uint16_t wiimote_button_map[DRUM_BUTTON__NUM];
    uint16_t drum_button_map[DRUM_BUTTON__NUM];
} drum_mapping =
    {
        .wiimote_button_map = {
            [DRUM_BUTTON_PS] = WIIMOTE_BUTTON_HOME,
        },
        .drum_button_map = {
            [DRUM_BUTTON_YELLOW] = DRUM_CTRL_BUTTON_YELLOW,
            [DRUM_BUTTON_RED] = DRUM_CTRL_BUTTON_RED,
            [DRUM_BUTTON_GREEN] = DRUM_CTRL_BUTTON_GREEN,
            [DRUM_BUTTON_BLUE] = DRUM_CTRL_BUTTON_BLUE,
            [DRUM_BUTTON_ORANGE] = DRUM_CTRL_BUTTON_ORANGE,
            [DRUM_BUTTON_UP] = DRUM_CTRL_BUTTON_UP,
            [DRUM_BUTTON_DOWN] = DRUM_CTRL_BUTTON_DOWN,
            [DRUM_BUTTON_LEFT] = DRUM_CTRL_BUTTON_LEFT,
            [DRUM_BUTTON_RIGHT] = DRUM_CTRL_BUTTON_RIGHT,
            [DRUM_BUTTON_START] = DRUM_CTRL_BUTTON_PLUS,
            [DRUM_BUTTON_SELECT] = DRUM_CTRL_BUTTON_MINUS,
        }};

static inline void drum_get_buttons(const struct drum_input_report *report, uint32_t *buttons) {
    uint32_t mask = 0;

#define MAP(field, button) \
    if (report->field)     \
        mask |= BIT(button);

    MAP(green, DRUM_BUTTON_GREEN)
    MAP(red, DRUM_BUTTON_RED)
    MAP(yellow, DRUM_BUTTON_YELLOW)
    MAP(blue, DRUM_BUTTON_BLUE)
    MAP(orange, DRUM_BUTTON_ORANGE)
    MAP(start, DRUM_BUTTON_START)
    MAP(select, DRUM_BUTTON_SELECT)
    MAP(kick, DRUM_BUTTON_KICK)
    MAP(ps, DRUM_BUTTON_PS)
#undef MAP
    if (report->hat == 0 || report->hat == 1 || report->hat == 7) {
        mask |= BIT(DRUM_BUTTON_UP);
    }
    if (report->hat == 1 || report->hat == 2 || report->hat == 3) {
        mask |= BIT(DRUM_BUTTON_LEFT);
    }
    if (report->hat == 3 || report->hat == 4 || report->hat == 5) {
        mask |= BIT(DRUM_BUTTON_DOWN);
    }
    if (report->hat == 5 || report->hat == 6 || report->hat == 7) {
        mask |= BIT(DRUM_BUTTON_RIGHT);
    }

    *buttons = mask;
}

static inline void drum_get_analog_axis(const struct drum_input_report *report,
                                        uint8_t analog_axis[static MAX_ANALOG_AXIS]) {
    analog_axis[DRUM_ANALOG_AXIS_GREEN] = report->pressure_green;
    analog_axis[DRUM_ANALOG_AXIS_RED] = report->pressure_red;
    analog_axis[DRUM_ANALOG_AXIS_YELLOW] = report->pressure_yellow;
    analog_axis[DRUM_ANALOG_AXIS_BLUE] = report->pressure_blue;
    analog_axis[DRUM_ANALOG_AXIS_ORANGE] = report->pressure_orange;
    analog_axis[DRUM_ANALOG_AXIS_KICK] = report->pressure_kick;
}

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

    if (device->extensionCallback) {
        device->extensionCallback(device->wiimote, WPAD_EXTENSION_DRUM);
    }
    device->extension = WPAD_EXTENSION_DRUM;
    device->format = WPAD_FORMAT_DRUM;

    ret = gh_drum_request_data(device);
    if (ret < 0)
        return ret;

    return 0;
}

int gh_drum_driver_ops_disconnect(usb_input_device_t *device) {
    struct gh_drum_private_data_t *priv = (void *)device->private_data;

    priv->leds = 0;

    return gh_drum_driver_update_leds(device);
}

int gh_drum_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot) {
    struct gh_drum_private_data_t *priv = (void *)device->private_data;

    priv->leds = slot;

    return gh_drum_driver_update_leds(device);
}

bool gh_drum_report_input(usb_input_device_t *device) {
    struct gh_drum_private_data_t *priv = (void *)device->private_data;
    union wiimote_extension_data_t extension_data;

    bm_map_wiimote(DRUM_BUTTON__NUM, priv->input.buttons,
                   drum_mapping.wiimote_button_map,
                   &device->wpadData.buttons);

    uint8_t drum_analog_axis[BM_DRUM_ANALOG_AXIS__NUM] = {0};
    // TODO: this
    drum_analog_axis[BM_DRUM_ANALOG_AXIS_STICK_X - 1] = 128;
    drum_analog_axis[BM_DRUM_ANALOG_AXIS_STICK_Y - 1] = 128;
    // Manually handle velocity here, as its pretty different between the wii and ps3 formats
    if (priv->input.analog_axis[DRUM_ANALOG_AXIS_GREEN]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b10010;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_GREEN];
    } else if (priv->input.analog_axis[DRUM_ANALOG_AXIS_RED]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b11001;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_RED];
    } else if (priv->input.analog_axis[DRUM_ANALOG_AXIS_YELLOW]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b10001;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_YELLOW];
    } else if (priv->input.analog_axis[DRUM_ANALOG_AXIS_BLUE]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b01111;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_BLUE];
    } else if (priv->input.analog_axis[DRUM_ANALOG_AXIS_ORANGE]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b01110;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_ORANGE];
    } else if (priv->input.analog_axis[DRUM_ANALOG_AXIS_KICK]) {
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1] = 0b11011;
        drum_analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] = priv->input.analog_axis[DRUM_ANALOG_AXIS_KICK];
    }

    bm_map_drum(DRUM_BUTTON__NUM, priv->input.buttons,
                BM_GUITAR_ANALOG_AXIS__NUM, drum_analog_axis,
                drum_mapping.drum_button_map,
                &extension_data.drum);
    device->wpadData.extension_data.drum.buttons = extension_data.drum.bt.hex;

    device->wpadData.status = WPAD_STATUS_OK;
    return true;
}

int gh_drum_driver_ops_usb_async_resp(usb_input_device_t *device) {
    struct gh_drum_private_data_t *priv = (void *)device->private_data;
    struct drum_input_report *report = (void *)device->usb_async_resp;
    drum_get_buttons(report, &priv->input.buttons);
    drum_get_analog_axis(report, priv->input.analog_axis);
    gh_drum_report_input(device);
    return gh_drum_request_data(device);
}

const usb_device_driver_t gh_drum_usb_device_driver = {
    .probe = gh_drum_driver_ops_probe,
    .init = gh_drum_driver_ops_init,
    .disconnect = gh_drum_driver_ops_disconnect,
    .slot_changed = gh_drum_driver_ops_slot_changed,
    .report_input = gh_drum_report_input,
    .usb_async_resp = gh_drum_driver_ops_usb_async_resp,
};
