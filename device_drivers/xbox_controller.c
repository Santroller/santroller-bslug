#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "rvl/WPAD.h"
#include "usb.h"
#include "usb_hid.h"
#include "wiimote.h"

#define GUITAR_ACC_RES_PER_G 113
#define XINPUT_GAMEPAD 1
#define XINPUT_WHEEL 2
#define XINPUT_ARCADE_STICK 3
#define XINPUT_FLIGHT_STICK 4
#define XINPUT_DANCE_PAD 5
#define XINPUT_GUITAR 6
#define XINPUT_GUITAR_ALTERNATE 7
#define XINPUT_DRUMS 8
#define XINPUT_STAGE_KIT 9
#define XINPUT_GUITAR_BASS 11
#define XINPUT_PRO_KEYS 15
#define XINPUT_ARCADE_PAD 19
#define XINPUT_TURNTABLE 23
#define XINPUT_PRO_GUITAR 25
#define XINPUT_GUITAR_WT 26

typedef struct {
    uint8_t rid;
    uint8_t rsize;

    uint8_t rightThumbClick : 1;
    uint8_t leftThumbClick : 1;
    uint8_t back : 1;
    uint8_t start : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;

    uint8_t y : 1;
    uint8_t x : 1;
    uint8_t b : 1;
    uint8_t a : 1;

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t rightShoulder : 1;
    uint8_t leftShoulder : 1;

    uint8_t leftTrigger;
    uint8_t rightTrigger;
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputGamepad_Data_t;

typedef struct
{
    uint8_t rid;
    uint8_t rsize;

    uint8_t padFlag : 1;  // right thumb click
    uint8_t pedal2 : 1;   // pedal2
    uint8_t back : 1;
    uint8_t start : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;

    uint8_t yellow : 1;  // yellow
    uint8_t blue : 1;    // blue
    uint8_t red : 1;     // red
    uint8_t green : 1;   // green

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t cymbalFlag : 1;  // right shoulder click
    uint8_t pedal1 : 1;      // pedal1

    uint8_t unused[2];
    int16_t redVelocity;
    int16_t yellowVelocity;
    int16_t blueVelocity;
    int16_t greenVelocity;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputRockBandDrums_Data_t;

typedef struct
{
    uint8_t rid;
    uint8_t rsize;
    uint8_t : 1;
    uint8_t leftThumbClick : 1;  // isGuitarHero
    uint8_t start : 1;
    uint8_t back : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;

    uint8_t yellow : 1;  // yellow
    uint8_t blue : 1;    // blue
    uint8_t red : 1;     // red
    uint8_t green : 1;   // green

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t orange : 1;  // orange
    uint8_t kick : 1;    // kick

    // TODO: The hi-hat pedal data is probably here somewhere
    uint8_t unused1[2];
    int16_t unused2;
    uint8_t greenVelocity;
    uint8_t redVelocity;
    uint8_t yellowVelocity;
    uint8_t blueVelocity;
    uint8_t orangeVelocity;
    uint8_t kickVelocity;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputGuitarHeroDrums_Data_t;

typedef struct
{
    uint8_t rid;
    uint8_t rsize;

    uint8_t : 1;
    uint8_t : 1;
    uint8_t back : 1;
    uint8_t start : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;  // dpadStrumDown
    uint8_t dpadUp : 1;    // dpadStrumUp

    uint8_t yellow : 1;  // yellow
    uint8_t blue : 1;    // blue
    uint8_t red : 1;     // red
    uint8_t green : 1;   // green

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t pedal : 1;   // pedal
    uint8_t orange : 1;  // orange

    uint8_t accelZ;
    uint8_t accelX;
    int16_t slider;
    int16_t unused;
    int16_t whammy;
    int16_t tilt;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputGuitarHeroGuitar_Data_t;

typedef struct
{
    uint8_t rid;
    uint8_t rsize;
    uint8_t : 1;
    uint8_t solo : 1;  // leftThumbClick
    uint8_t back : 1;
    uint8_t start : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;  // dpadStrumDown
    uint8_t dpadUp : 1;    // dpadStrumUp

    uint8_t yellow : 1;  // yellow
    uint8_t blue : 1;    // blue
    uint8_t red : 1;     // red
    uint8_t green : 1;   // green

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t : 1;
    uint8_t orange : 1;  // orange

    uint8_t pickup;
    uint8_t unused1;
    int16_t calibrationSensor;
    int16_t unused2;
    int16_t whammy;
    int16_t tilt;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputRockBandGuitar_Data_t;

typedef struct
{
    uint8_t rid;
    uint8_t rsize;

    uint8_t : 1;
    uint8_t : 1;
    uint8_t back : 1;
    uint8_t start : 1;

    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;

    uint8_t y : 1;  // euphoria
    uint8_t x : 1;
    uint8_t b : 1;
    uint8_t a : 1;

    uint8_t : 1;
    uint8_t guide : 1;
    uint8_t : 1;
    uint8_t : 1;

    uint8_t : 5;
    uint8_t leftBlue : 1;
    uint8_t leftRed : 1;
    uint8_t leftGreen : 1;

    uint8_t : 5;
    uint8_t rightBlue : 1;
    uint8_t rightRed : 1;
    uint8_t rightGreen : 1;

    int16_t leftTableVelocity;
    int16_t rightTableVelocity;

    int16_t effectsKnob;  // Whether or not this is signed doesn't really matter, as either way it's gonna loop over when it reaches min/max
    int16_t crossfader;
    uint8_t reserved_1[6];
} __attribute__((packed)) XInputTurntable_Data_t;

static inline int xbox_controller_request_data(usb_input_device_t *device) {
    return usb_device_driver_issue_intr_transfer_async(device, false, device->usb_async_resp, device->max_packet_len_in);
}

bool xbox_controller_driver_ops_probe(uint16_t vid, uint16_t pid) {
    static const struct device_id_t compatible[] = {
        {0x045e, 0x02a9}};

    return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

static int xbox_controller_driver_set_rumble(usb_input_device_t *device, uint8_t left, uint8_t right) {
    if (device->type == XINPUT_TYPE_WIRED) {
        static uint8_t buf[] ATTRIBUTE_ALIGN(32) = {
            0x00, /* Type */
            0x08, /* Size */
            0x00, /* Padding */
            0x00, /* Left */
            0x00, /* Right */
            0x00, 0x00, 0x00 /* Padding */};

        buf[3] = left;
        buf[4] = right;
        return usb_device_driver_issue_intr_transfer_async(device, true, buf, sizeof(buf));
    } else {
        static uint8_t buf[] ATTRIBUTE_ALIGN(32) = {
            0x00,
            0x01,
            0x0f,
            0xc0,
            0x00,
            0x00, /* Left */
            0x00 /* Right */};

        buf[5] = left;
        buf[6] = right;
        return usb_device_driver_issue_intr_transfer_async(device, true, buf, sizeof(buf));
    }
}

static uint8_t disconnect[12] IOS_ALIGN = {0x00, 0x00, 0x08, 0xC0};
static uint8_t led[12] IOS_ALIGN = {0x00, 0x00, 0x08, 0x41};
static uint8_t led_wired[3] IOS_ALIGN = {0x01, 0x03, 0x00};
static uint8_t capabilities[12] IOS_ALIGN = {0x00, 0x00, 0x02, 0x80};

int xbox_controller_driver_ops_init(usb_input_device_t *device) {
    int ret;

    if (device->type == XINPUT_TYPE_WIRELESS) {
        // We don't receive a link packet for devices that are already connected, so disconnect all of them
        device->state = 0;

        ret = usb_device_driver_issue_intr_transfer_async(device, true, led, sizeof(led));
        ;
        if (ret < 0)
            return ret;

        return 0;
    }

    uint8_t ext = WPAD_EXTENSION_CLASSIC;
    uint8_t df = WPAD_FORMAT_CLASSIC;
    if (device->sub_type == XINPUT_GUITAR || device->sub_type == XINPUT_GUITAR_ALTERNATE || device->sub_type == XINPUT_GUITAR_BASS) {
        ext = WPAD_EXTENSION_GUITAR;
        df = WPAD_FORMAT_GUITAR;
    }
    if (device->sub_type == XINPUT_TURNTABLE) {
        ext = WPAD_EXTENSION_TURNTABLE;
        df = WPAD_FORMAT_TURNTABLE;
    }

    device->extension = ext;
    device->wpadData.extension = ext;
    device->format = df;
    device->gravityUnit[0].acceleration[0] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[1] = ACCEL_ONE_G;
    device->gravityUnit[0].acceleration[2] = ACCEL_ONE_G;
    led_wired[2] = device->wiimote + 2;
    ret = usb_device_driver_issue_intr_transfer_async(device, true, led_wired, sizeof(led_wired));
    if (ret < 0)
        return ret;

    return 0;
}
int xbox_controller_driver_ops_disconnect(usb_input_device_t *device) {
    return 0;
}

bool xbox_controller_report_turntable_input(const XInputTurntable_Data_t *report, usb_input_device_t *device) {
    device->wpadData.buttons = 0;
    device->wpadData.home = report->guide;
    device->wpadData.extension_data.turntable.buttons = 0;
    device->wpadData.extension_data.turntable.leftBlue = report->leftBlue;
    device->wpadData.extension_data.turntable.leftGreen = report->leftGreen;
    device->wpadData.extension_data.turntable.leftRed = report->leftRed;
    device->wpadData.extension_data.turntable.rightBlue = report->rightBlue;
    device->wpadData.extension_data.turntable.rightGreen = report->rightGreen;
    device->wpadData.extension_data.turntable.euphoria = report->y;
    device->wpadData.extension_data.turntable.plus = report->start;
    device->wpadData.extension_data.turntable.minus = report->back;

    device->wpadData.extension_data.turntable.stick[0] = 0;
    device->wpadData.extension_data.turntable.stick[1] = 0;
    if (report->dpadUp) {
        device->wpadData.extension_data.guitar.stick[1] = -10;
    }
    if (report->dpadDown) {
        device->wpadData.extension_data.guitar.stick[1] = 10;
    }
    if (report->dpadLeft) {
        device->wpadData.extension_data.guitar.stick[0] = -10;
    }

    if (report->dpadRight) {
        device->wpadData.extension_data.guitar.stick[0] = 10;
    }
    device->wpadData.status = WPAD_STATUS_OK;
    int8_t ltt = ((int16_t)__builtin_bswap16(report->leftTableVelocity)) >> 1;
    device->wpadData.extension_data.turntable.ltt_sign = ltt >= 0;
    device->wpadData.extension_data.turntable.ltt = ltt;
    int8_t rtt = ((int16_t)__builtin_bswap16(report->rightTableVelocity)) >> 1;
    device->wpadData.extension_data.turntable.rtt_sign = rtt < 0;
    device->wpadData.extension_data.turntable.rtt = rtt;
    device->wpadData.extension_data.turntable.crossFader = (__builtin_bswap16(report->crossfader) + INT16_MAX) >> 12;
    device->wpadData.extension_data.turntable.effectsDial = (__builtin_bswap16(report->effectsKnob) + INT16_MAX) >> 11;

    return true;
}

bool xbox_controller_report_gh_guitar_input(const XInputGuitarHeroGuitar_Data_t *report, usb_input_device_t *device) {
    // Guitar is sideways!
    device->wpadData.acceleration[1] = ((int16_t)le16toh(report->tilt)) >> 8;

    device->wpadData.buttons = 0;
    device->wpadData.home = report->guide;
    device->wpadData.extension_data.guitar.buttons = 0;
    device->wpadData.extension_data.guitar.green = report->green;
    device->wpadData.extension_data.guitar.red = report->red;
    device->wpadData.extension_data.guitar.yellow = report->yellow;
    device->wpadData.extension_data.guitar.blue = report->blue;
    device->wpadData.extension_data.guitar.orange = report->orange;
    device->wpadData.extension_data.guitar.pedal = report->pedal;
    device->wpadData.extension_data.guitar.plus = report->start;
    device->wpadData.extension_data.guitar.minus = report->back;

    device->wpadData.extension_data.guitar.stick[0] = 0;
    device->wpadData.extension_data.guitar.stick[1] = 0;
    device->wpadData.extension_data.guitar.dpadUp = report->dpadUp;
    device->wpadData.extension_data.guitar.dpadDown = report->dpadDown;
    if (report->dpadLeft) {
        device->wpadData.extension_data.guitar.stick[0] = -10;
    }

    if (report->dpadRight) {
        device->wpadData.extension_data.guitar.stick[0] = 10;
    }
    int16_t whammy = __builtin_bswap16(report->whammy);

    device->wpadData.extension_data.guitar.whammy = ((whammy >> 8) + 0x80) >> 1;
    if (!device->old_wpad) {
        device->wpadData.extension_data.guitar.whammy += 0x80;
    }
    // TODO: tap bar
    device->wpadData.extension_data.guitar.tapbar = 0x1E0;
    device->wpadData.status = WPAD_STATUS_OK;

    return true;
}
bool xbox_controller_report_rb_guitar_input(const XInputRockBandGuitar_Data_t *report, usb_input_device_t *device) {
    device->wpadData.acceleration[0] = (int16_t)le16toh(report->tilt) - 511;

    device->wpadData.buttons = 0;
    device->wpadData.home = report->guide;
    device->wpadData.extension_data.guitar.buttons = 0;
    device->wpadData.extension_data.guitar.green = report->green;
    device->wpadData.extension_data.guitar.red = report->red;
    device->wpadData.extension_data.guitar.yellow = report->yellow;
    device->wpadData.extension_data.guitar.blue = report->blue;
    device->wpadData.extension_data.guitar.orange = report->orange;
    device->wpadData.extension_data.guitar.plus = report->start;
    device->wpadData.extension_data.guitar.minus = report->back;

    device->wpadData.extension_data.guitar.stick[0] = 0;
    device->wpadData.extension_data.guitar.stick[1] = 0;
    device->wpadData.extension_data.guitar.dpadUp = report->dpadUp;
    device->wpadData.extension_data.guitar.dpadDown = report->dpadDown;

    if (report->dpadLeft) {
        device->wpadData.extension_data.guitar.stick[0] = -10;
    }

    if (report->dpadRight) {
        device->wpadData.extension_data.guitar.stick[0] = 10;
    }

    int16_t whammy = __builtin_bswap16(report->whammy);

    device->wpadData.extension_data.guitar.whammy = ((whammy >> 8) + 0x80) >> 1;
    if (!device->old_wpad) {
        device->wpadData.extension_data.guitar.whammy += 0x80;
    }
    device->wpadData.status = WPAD_STATUS_OK;

    return true;
}
bool xbox_controller_report_drums_input(const XInputGuitarHeroDrums_Data_t *report, usb_input_device_t *device) {
    device->wpadData.buttons = 0;
    device->wpadData.extension_data.drum.buttons = 0;
    device->wpadData.extension_data.drum.green = report->green;
    device->wpadData.extension_data.drum.red = report->red;
    device->wpadData.extension_data.drum.yellow = report->yellow;
    device->wpadData.extension_data.drum.blue = report->blue;
    device->wpadData.extension_data.drum.orange = report->orange;
    device->wpadData.extension_data.drum.plus = report->start;
    device->wpadData.extension_data.drum.minus = report->back;
    device->wpadData.extension_data.drum.connected = WPAD_DRUM_HAS_VELOCITY;
    uint8_t velocity = 0x7F;
    uint8_t note = 0x7F;
    if (report->greenVelocity) {
        note = GREEN;
        velocity = report->greenVelocity;
    } else if (report->redVelocity) {
        note = RED;
        velocity = report->redVelocity;
    } else if (report->yellowVelocity) {
        note = YELLOW;
        velocity = report->yellowVelocity;
    } else if (report->blueVelocity) {
        note = BLUE;
        velocity = report->blueVelocity;
    } else if (report->orangeVelocity) {
        note = ORANGE;
        velocity = report->orangeVelocity;
    } else if (report->kickVelocity) {
        note = KICK_PEDAL;
        velocity = report->kickVelocity;
    } else {
        device->wpadData.extension_data.drum.connected = WPAD_DRUM_NO_VELOCITY;
    }
    velocity = 0x7F - velocity;
    note = 0x7F - note;
    device->wpadData.extension_data.drum.velocity0 = ~velocity;
    device->wpadData.extension_data.drum.velocity1 = ~velocity >> 1;
    device->wpadData.extension_data.drum.velocity2 = velocity >> 2;
    device->wpadData.extension_data.drum.velocity3 = velocity >> 3;
    device->wpadData.extension_data.drum.velocity4 = velocity >> 4;
    device->wpadData.extension_data.drum.velocity5 = velocity >> 5;
    device->wpadData.extension_data.drum.velocity6 = velocity >> 6;
    device->wpadData.extension_data.drum.note0 = note;
    device->wpadData.extension_data.drum.note1 = note >> 1;
    device->wpadData.extension_data.drum.note2 = note >> 2;
    device->wpadData.extension_data.drum.note3 = note >> 3;
    device->wpadData.extension_data.drum.note4 = note >> 4;
    device->wpadData.extension_data.drum.note5 = note >> 5;
    device->wpadData.extension_data.drum.note6 = note >> 6;

    device->wpadData.status = WPAD_STATUS_OK;

    return true;
}
bool xbox_controller_report_gamepad_input(const XInputGamepad_Data_t *report, usb_input_device_t *device) {
    device->wpadData.buttons = 0;
    device->wpadData.extension_data.classic.buttons = 0;
    device->wpadData.extension_data.classic.a = report->a;
    device->wpadData.extension_data.classic.b = report->b;
    device->wpadData.extension_data.classic.x = report->x;
    device->wpadData.extension_data.classic.y = report->y;
    device->wpadData.extension_data.classic.zl = report->leftShoulder;
    device->wpadData.extension_data.classic.zr = report->rightShoulder;
    device->wpadData.extension_data.classic.lt = report->leftTrigger > 0x80;
    device->wpadData.extension_data.classic.rt = report->rightTrigger > 0x80;
    device->wpadData.extension_data.classic.plus = report->start;
    device->wpadData.extension_data.classic.minus = report->back;
    device->wpadData.extension_data.classic.home = report->guide;
    device->wpadData.extension_data.classic.dpadUp = report->dpadUp;
    device->wpadData.extension_data.classic.dpadDown = report->dpadDown;
    device->wpadData.extension_data.classic.dpadLeft = report->dpadLeft;
    device->wpadData.extension_data.classic.dpadRight = report->dpadRight;
    device->wpadData.extension_data.classic.leftStick[0] = __builtin_bswap16(report->leftStickX) >> 6;
    device->wpadData.extension_data.classic.leftStick[1] = __builtin_bswap16(report->leftStickY) >> 6;
    device->wpadData.extension_data.classic.rightStick[0] = __builtin_bswap16(report->rightStickX) >> 6;
    device->wpadData.extension_data.classic.rightStick[1] = __builtin_bswap16(report->rightStickY) >> 6;
    device->wpadData.extension_data.classic.trigger[0] = report->leftTrigger;
    device->wpadData.extension_data.classic.trigger[1] = report->rightTrigger;

    device->wpadData.status = WPAD_STATUS_OK;

    return true;
}
int xbox_controller_driver_ops_usb_async_resp(usb_input_device_t *device) {
    bool hasPacket = device->type == XINPUT_TYPE_WIRED;
    if (device->type == XINPUT_TYPE_WIRELESS) {
        xboxwirelessheader *header = (xboxwirelessheader *)device->usb_async_resp;
        // Gamepad inputs
        if (header->id == 0x00 && (header->type == 0x01 || header->type == 0x03)) {
            // USB Reports have an extra RID, so we want to start our "report" one byte before it actually starts
            memcpy(device->usb_async_resp, device->usb_async_resp + sizeof(xboxwirelessheader) - 1, sizeof(XInputGamepad_Data_t));
            hasPacket = true;
        } else {
            if (header->id == 0x08) {
                // Disconnected
                if (header->type == 0x00) {
                    printf_v("Xbox 360 wireless device disconnected!\r\n");
                    device->sub_type = 0;
                }
            } else if (header->id == 0x00) {
                // Link report
                if (header->type == 0x0f) {
                    xboxwirelesslinkreport *linkReport = (xboxwirelesslinkreport *)device->usb_async_resp;
                    if (linkReport->always_0xCC == 0xCC) {
                        uint8_t sub_type = linkReport->subtype & ~0x80;
                        printf_v("Found wireless subtype: %02x\r\n", sub_type);
                        // Request capabilities so we can figure out WT guitars
                        if (sub_type == XINPUT_GUITAR_ALTERNATE) {
                            // this should work?
                            return usb_device_driver_issue_intr_transfer_async(device, true, capabilities, sizeof(capabilities));
                        }

                        uint8_t ext = WPAD_EXTENSION_CLASSIC;
                        uint8_t df = WPAD_FORMAT_CLASSIC;
                        if (sub_type == XINPUT_GUITAR || sub_type == XINPUT_GUITAR_ALTERNATE || sub_type == XINPUT_GUITAR_BASS) {
                            ext = WPAD_EXTENSION_GUITAR;
                            df = WPAD_FORMAT_GUITAR;
                        }
                        if (sub_type == XINPUT_TURNTABLE) {
                            ext = WPAD_EXTENSION_TURNTABLE;
                            df = WPAD_FORMAT_TURNTABLE;
                        }
                        device->extension = ext;
                        device->wpadData.extension = ext;
                        device->format = df;
                        if (device->extensionCallback) {
                            printf_v("ext callback! %02x\r\n", device->wiimote);
                            device->extensionCallback(device->wiimote, device->extension);
                        }
                        device->gravityUnit[0].acceleration[0] = ACCEL_ONE_G;
                        device->gravityUnit[0].acceleration[1] = ACCEL_ONE_G;
                        device->gravityUnit[0].acceleration[2] = ACCEL_ONE_G;
                        if (device->sub_type == 0) {
                            device->sub_type = sub_type;
                            led[3] = (device->wiimote + 2) | 0x40;
                            return usb_device_driver_issue_intr_transfer_async(device, true, led, sizeof(led));
                        }
                        device->sub_type = sub_type;
                    }
                }
                if (header->type == 0x05) {
                    xboxwirelesscapabilities *caps = (xboxwirelesscapabilities *)device->usb_async_resp;
                    if (caps->always_0x12 == 0x12) {
                        printf_v("Found wireless capabilities: %04x\r\n", caps->leftStickX);
                        if (caps->leftStickX == 0xFFC0 && caps->rightStickX == 0xFFC0) {
                            device->sub_type = XINPUT_GUITAR_WT;
                            printf_v("Found wt\r\n");
                        }
                    }
                }
            }
        }
    }
    if (device->sub_type == XINPUT_GAMEPAD) {
        if (device->rumble_on != device->last_rumble_on) {
            return xbox_controller_driver_set_rumble(device, device->rumble_on * 255, device->rumble_on * 255);
        }
    }
    if (device->type == XINPUT_TURNTABLE) {
        if (device->euphoria_led != device->last_euphoria_led) {
            return xbox_controller_driver_set_rumble(device, device->euphoria_led * 255, device->rumble_on * 255);
        }
    }
    if (device->state == 0) {
        device->state = 1;
        return usb_device_driver_issue_intr_transfer_async(device, true, disconnect, sizeof(disconnect));
    }
    if (hasPacket) {
        if (device->sub_type == XINPUT_GUITAR_ALTERNATE || device->sub_type == XINPUT_GUITAR_WT) {
            xbox_controller_report_gh_guitar_input((XInputGuitarHeroGuitar_Data_t *)device->usb_async_resp, device);
        } else if (device->sub_type == XINPUT_GUITAR || device->sub_type == XINPUT_GUITAR_BASS) {
            xbox_controller_report_rb_guitar_input((XInputRockBandGuitar_Data_t *)device->usb_async_resp, device);
        } else if (device->sub_type == XINPUT_TURNTABLE) {
            xbox_controller_report_turntable_input((XInputTurntable_Data_t *)device->usb_async_resp, device);
        } else if (device->sub_type == XINPUT_DRUMS) {
            xbox_controller_report_drums_input((XInputGuitarHeroDrums_Data_t *)device->usb_async_resp, device);
        } else {
            xbox_controller_report_gamepad_input((XInputGamepad_Data_t *)device->usb_async_resp, device);
        }
    }
    return xbox_controller_request_data(device);
}

const usb_device_driver_t xbox_controller_usb_device_driver = {
    .probe = xbox_controller_driver_ops_probe,
    .hid = false,
    .init = xbox_controller_driver_ops_init,
    .disconnect = xbox_controller_driver_ops_disconnect,
    .usb_async_resp = xbox_controller_driver_ops_usb_async_resp,
};
