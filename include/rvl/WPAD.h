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
typedef void (*WPADConnectCallback_t)(int wiimote, WPADStatus_t status);
typedef void (*WPADExtensionCallback_t)(int wiimote, WPADExtension_t extension);
typedef void (*WPADSamplingCallback_t)(int wiimote);
typedef void (*WPADControlDpdCallback_t)(int wiimote, int status);

void WPADRead(int wiiremote, WPADData_t *data);
void WPADInit(void);
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

static inline size_t WPADDataFormatSize(WPADDataFormat_t format);

struct WPADData_t {
    uint16_t buttons;
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
            uint16_t buttons;
            int16_t stickX[2];
            int16_t stickY[2];
            uint8_t trigger[2];
        } classic;
        struct {
            uint16_t buttons;
            int16_t stick[2];
            int16_t unused;
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
            uint8_t unused2;
            uint8_t whammy;  // 0x78 -> 0xd0
        } guitar;
        struct {
            uint16_t buttons;
            int16_t stick[2];
            int16_t unused;
            int16_t which;
            uint8_t unused2;
            uint8_t whammy;
        } drum;
        struct {
            uint16_t buttons;
            int16_t stick[2];
            int16_t unused;
            int16_t which;
            uint8_t unused2;
            uint8_t whammy;
        } turntable;
        uint8_t unknown[0x30];
    } extension_data;
};

struct WPADAccGravityUnit_t {
    int16_t acceleration[3];  // x, y, z
};

enum WPADStatus_t {
    WPAD_STATUS_OK = 0,
    WPAD_STATUS_DISCONNECTED = -1,
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
    WPAD_FORMAT_TURNTABLE = 18,  // TODO: figure this one out

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
    WPAD_EXTENSION_TURNTABLE = 20,  // TODO: figure this one out
    WPAD_EXTENSION_PRO_CONTROLLER = 31,
    WPAD_EXTENSION_UNKNOWN = 255,
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
