/* WPAD.h
 *   by Alex Chadwick
 *
 * Copyright (C) 2014, Alex Chadwick
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* definitions of symbols inferred to exist in the WPAD.h header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_WPAD_H_
#define _RVL_WPAD_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct WPADData_t WPADData_t;
typedef struct WPADAccGravityUnit_t WPADAccGravityUnit_t;
typedef enum WPADStatus_t WPADStatus_t;
typedef enum WPADDataFormat_t WPADDataFormat_t;
typedef enum WPADExtension_t WPADExtension_t;
typedef enum WPADPeripheralSpace_t WPADPeripheralSpace_t;
typedef void (*WPADConnectCallback_t)(int wiimote, WPADStatus_t status);
typedef void (*WPADMemoryCallback_t)(int wiimote, WPADStatus_t status);
typedef void (*WPADExtensionCallback_t)(int wiimote, WPADExtension_t extension);
typedef void (*WPADSamplingCallback_t)(int wiimote);
typedef void (*WPADControlDpdCallback_t)(int wiimote, int status);
typedef void (*WPADInfoCallback_t)(int wiimote, int status);

typedef struct
{
    bool dpd;
    bool speaker;
    bool attach;
    bool lowBat;
    bool nearempty;
    uint8_t battery;
    uint8_t led;
    uint8_t protocol;
    uint8_t firmware;
} WPADInfo_t;

void WPADRead(int wiiremote, WPADData_t *data);
void WPADInit(void);
int WPADGetStatus(void);
WPADConnectCallback_t WPADSetConnectCallback(int wiimote, WPADConnectCallback_t newCallback);
WPADExtensionCallback_t WPADSetExtensionCallback(int wiimote, WPADExtensionCallback_t newCallback);
WPADSamplingCallback_t WPADSetSamplingCallback(int wiimote, WPADSamplingCallback_t newCallback);
void WPADSetAutoSamplingBuf(int wiimote, void *buffer, int count);
int WPADGetLatestIndexInBuf(int wiimote);
int WPADSetDataFormat(int wiimote, WPADDataFormat_t format);
WPADDataFormat_t WPADGetDataFormat(int wiimote);
void WPADGetAccGravityUnit(int wiimote, WPADExtension_t extension, WPADAccGravityUnit_t *result);
WPADStatus_t WPADProbe(int wiimote, WPADExtension_t *extension);
int WPADControlDpd(int wiimote, int command, WPADControlDpdCallback_t callback);
bool WPADIsDpdEnabled(int wiimote);
void WPADControlMotor(int wiimote, int cmd);
void WPADWriteExtReg(int wiimote, void *buffer, int size, WPADPeripheralSpace_t space, int address, WPADMemoryCallback_t callback);
int WPADGetInfoAsync(int wiimote, WPADInfo_t *info, WPADInfoCallback_t callback);
static inline size_t WPADDataFormatSize(WPADDataFormat_t format);

struct WPADData_t {
    union {
        uint16_t buttons;
        struct {
            uint16_t home : 1;
            uint16_t : 1;
            uint16_t : 1;
            uint16_t minus : 1;
            uint16_t a : 1;
            uint16_t b : 1;
            uint16_t one : 1;
            uint16_t two : 1;

            uint16_t : 1;
            uint16_t : 1;
            uint16_t : 1;
            uint16_t plus : 1;
            uint16_t dpadUp : 1;
            uint16_t dpadDown : 1;
            uint16_t dpadRight : 1;
            uint16_t dpadLeft : 1;
        };
    };
    int16_t acceleration[3];  // x, y, z
    struct {
        int16_t x, y;
        uint16_t size;
        uint8_t id;
        uint8_t padding;
    } ir[4];            // IR tracking points
    uint8_t extension;  // WPAD_EXTENSION_x
    uint8_t status;     // WPAD_STATUS_x
    union {
        struct {
            int16_t acceleration[3];
            uint8_t stick[2];
        } nunchuck;
        struct {
            union {
                uint16_t buttons;
                struct {
                    uint16_t dpadRight : 1;
                    uint16_t dpadDown : 1;
                    uint16_t lt : 1;
                    uint16_t minus : 1;
                    uint16_t home : 1;
                    uint16_t plus : 1;
                    uint16_t rt : 1;
                    uint16_t : 1;
                    uint16_t zl : 1;
                    uint16_t b : 1;
                    uint16_t y : 1;
                    uint16_t a : 1;
                    uint16_t x : 1;
                    uint16_t zr : 1;
                    uint16_t dpadLeft : 1;
                    uint16_t dpadUp : 1;
                };
            };
            int16_t leftStick[2];
            int16_t rightStick[2];
            uint8_t trigger[2];
        } classic;
        struct {
            union {
                uint16_t buttons;
                struct {
                    uint16_t : 1;
                    uint16_t dpadDown : 1;
                    uint16_t : 1;
                    uint16_t minus : 1;
                    uint16_t : 1;
                    uint16_t plus : 1;
                    uint16_t : 1;
                    uint16_t : 1;
                    uint16_t orange : 1;
                    uint16_t red : 1;
                    uint16_t blue : 1;
                    uint16_t green : 1;
                    uint16_t yellow : 1;
                    uint16_t pedal : 1;
                    uint16_t : 1;
                    uint16_t dpadUp : 1;
                };
            };
            int16_t stick[2];
            int16_t : 16;
            // 128 (80): green
            // 192 (C0): green+red
            // 320 (140): red
            // 416 (1A0): red+yellow
            // 480 (1E0): none
            // 576 (240): yellow
            // 640 (280): yellow+blue
            // 736 (2E0): blue
            // 832 (340): blue+orange
            // 992 (3E0): orange
            int16_t tapbar;
            uint8_t : 8;
            uint8_t whammy;  // 0x78 -> 0xd0
        } guitar;
        struct {
            union {
                uint16_t buttons;
                struct {
                    uint16_t : 1;
                    uint16_t : 1;
                    uint16_t : 1;
                    uint16_t minus : 1;
                    uint16_t : 1;
                    uint16_t plus : 1;
                    uint16_t : 1;
                    uint16_t : 1;
                    uint16_t orange : 1;
                    uint16_t red : 1;
                    uint16_t yellow : 1;
                    uint16_t green : 1;
                    uint16_t blue : 1;
                    uint16_t pedal : 1;
                    uint16_t : 1;
                    uint16_t : 1;
                };
            };
            int16_t stick[2];
            int16_t unused;
            int16_t which;
            uint8_t velocity;
            uint8_t whammy;
        } drum;
        struct {
            union {
                uint16_t buttons;
                struct {
                    uint16_t : 2;
                    uint16_t leftRed : 1;
                    uint16_t minus : 1;
                    uint16_t : 1;
                    uint16_t plus : 1;
                    uint16_t rightRed : 1;
                    uint16_t ltt_sign : 1;
                    uint16_t leftBlue : 1;
                    uint16_t : 1;
                    uint16_t rightGreen : 1;
                    uint16_t euphoria : 1;
                    uint16_t leftGreen : 1;
                    uint16_t rightBlue : 1;
                    uint16_t : 2;
                };
            };
            int16_t stick[2];  // -512 -> 512
            uint16_t : 6;
            uint16_t rtt : 5;
            uint16_t : 5;
            uint16_t : 6;
            uint16_t crossFader : 4;  // 0 - 15, center at 8
            uint16_t rtt_sign : 1;
            uint16_t : 5;
            uint8_t effectsDial : 5;  // 0 - 32, center at 16
            uint8_t : 3;
            uint8_t ltt : 5;
            uint8_t : 3;
        } turntable;
        struct {
            union {
                uint16_t buttons;
                struct {
                    uint16_t : 9;
                    uint16_t centerLeft : 1;
                    uint16_t rimLeft : 1;
                    uint16_t centerRight : 1;
                    uint16_t rimRight : 1;
                    uint16_t : 3;
                };
            };
        } taiko;
        uint8_t unknown[0x30];
    } extension_data;
} __attribute__((packed));

struct WPADAccGravityUnit_t {
    int16_t acceleration[3];  // x, y, z
};

enum WPADStatus_t {
    WPAD_STATUS_OK = 0,
    WPAD_STATUS_DISCONNECTED = -1,
};

enum WPADState_t {
    WPAD_STATE_DISABLED = 0,
    WPAD_STATE_ENABLING = 1,
    WPAD_STATE_ENABLED = 2,
    WPAD_STATE_SETUP = 3,
    WPAD_STATE_DISABLING = 4
};

enum WPADDataFormat_t {
    WPAD_FORMAT_NONE,
    WPAD_FORMAT_ACC,
    WPAD_FORMAT_ACC_IR,
    WPAD_FORMAT_NUNCHUCK,
    WPAD_FORMAT_NUNCHUCK_ACC,
    WPAD_FORMAT_NUNCHUCK_ACC_IR,
    WPAD_FORMAT_CLASSIC,
    WPAD_FORMAT_CLASSIC_ACC,
    WPAD_FORMAT_CLASSIC_ACC_IR,
    WPAD_FORMAT_TRAIN = 10,
    WPAD_FORMAT_GUITAR = 11,
    WPAD_FORMAT_DRUM = 15,
    WPAD_FORMAT_TAIKO = 17,
    WPAD_FORMAT_TURNTABLE = 18,

};

enum WPADExtension_t {
    WPAD_EXTENSION_NONE,
    WPAD_EXTENSION_NUNCHUCK,
    WPAD_EXTENSION_CLASSIC,
    WPAD_EXTENSION_MPLUS = 5,
    WPAD_EXTENSION_MPLUS_NUNCHUK = 6,
    WPAD_EXTENSION_MPLUS_CLASSIC = 7,
    WPAD_EXTENSION_TRAIN = 16,
    WPAD_EXTENSION_GUITAR = 17,
    WPAD_EXTENSION_DRUM = 18,
    WPAD_EXTENSION_TAIKO = 19,
    WPAD_EXTENSION_TURNTABLE = 20,
    WPAD_EXTENSION_PRO_CONTROLLER = 31,
    WPAD_EXTENSION_UNKNOWN = 255,
};

enum WPADPeripheralSpace_t {
    WPAD_PERIPHERAL_SPACE_SPEAKER,
    WPAD_PERIPHERAL_SPACE_EXTENSION,
    WPAD_PERIPHERAL_SPACE_MOTIONPLUS,
    WPAD_PERIPHERAL_SPACE_DPD
};

static inline size_t WPADDataFormatSize(WPADDataFormat_t format) {
    switch (format) {
        case WPAD_FORMAT_NONE:
        case WPAD_FORMAT_ACC:
        case WPAD_FORMAT_ACC_IR:
            return 0x2a;
        case WPAD_FORMAT_NUNCHUCK:
        case WPAD_FORMAT_NUNCHUCK_ACC:
        case WPAD_FORMAT_NUNCHUCK_ACC_IR:
            return 0x32;
        case WPAD_FORMAT_TURNTABLE:
        case WPAD_FORMAT_DRUM:
        case WPAD_FORMAT_GUITAR:
        case WPAD_FORMAT_CLASSIC:
        case WPAD_FORMAT_CLASSIC_ACC:
        case WPAD_FORMAT_CLASSIC_ACC_IR:
            return 0x36;
        default:
            return 0x5a;
    }
}

#endif /* _RVL_WPAD_H_ */
