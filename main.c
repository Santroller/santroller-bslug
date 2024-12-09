/* main.c
 *   by Alex Chadwick
 *
 * Copyright (C) 2017, Alex Chadwick
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

/* This module adds support for the USB GCN Adapter (WUP-028). It achieves this
 * by simply replacing the Wii's PADRead and PADControlMotor calls. When PADRead
 * is called for the first time, the module initialises. Thereafter, the reading
 * of the actual USB proceeds asynchronously and continuously and the PADRead
 * method just returns the result of the most recent message from the USB. The
 * outbound rumble control messages get inserted between inbound controller data
 * messages, as it seems like having both inbound and outbound messages
 * travelling independently can cause lock ups. */

/* Interfacing with the USB is done by the IOS on behalf of the game. The
 * interface that the IOS exposes differs by IOS version. This module uses the
 * /dev/usb/hid interface. I've not tested exhaustively but IOS36 doesn't
 * support /dev/usb/hid at all, so this module won't work there. IOS37
 * introduces /dev/usb/hid at version 4. Later, IOS58 changes /dev/usb/hid
 * completely to version 5. This module supports both version 4 and version 5 of
 * /dev/usb/hid which it detects dynamically. I don't know if any later IOSs
 * change /dev/usb/hid in a non-suported way, or if earlier IOSs may be
 * compatible. */

#include <bslug.h>
#include <rvl/OSTime.h>
#include <rvl/Pad.h>
#include <rvl/WPAD.h>
#include <rvl/cache.h>
#include <rvl/ipc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "rvl/OSInterrupts.h"
#include "usb.h"
#include "usb_hid.h"

BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("Guitar Hero USB Instrument Support");
BSLUG_MODULE_VERSION("v1.1");
BSLUG_MODULE_AUTHOR("sanjay900");
BSLUG_MODULE_LICENSE("BSD");

/*============================================================================*/
/* Macros */
/*============================================================================*/

/* If you don't want to support both interfaces, remove one of these two
 * defines. */
#define SUPPORT_DEV_USB_HID4
#define SUPPORT_DEV_USB_HID5

#define IOS_ALIGN __attribute__((aligned(32)))
#define MAX_FAKE_WIIMOTES 8
/* Path to the USB device interface. */
#define DEV_USB_HID_PATH "/dev/usb/hid"
#define DEV_USB_VEN_PATH "/dev/usb/ven"
#define DEV_USB_OH0_PATH "/dev/usb/oh0"

#define BUFFER_SIZE 5
#define USB_MAX_DEVICES 32
#define USBV0_IOCTL_CTRLMSG 0
#define USBV0_IOCTL_BLKMSG 1
#define USBV0_IOCTL_INTRMSG 2
#define USBV0_IOCTL_SUSPENDDEV 5
#define USBV0_IOCTL_RESUMEDEV 6
#define USBV0_IOCTL_ISOMSG 9
#define USBV0_IOCTL_GETDEVLIST 12
#define USBV0_IOCTL_DEVREMOVALHOOK 26
#define USBV0_IOCTL_DEVINSERTHOOK 27
#define USBV0_IOCTL_DEVICECLASSCHANGE 28
#define USBV0_IOCTL_RESETDEVICE 29

#define USBV4_IOCTL_GETVERSION 6  // returns 0x40001
#define USBV4_IOCTL_CANCELINTERRUPT 8

#define USBV5_IOCTL_GETVERSION 0  // should return 0x50001
#define USBV5_IOCTL_GETDEVICECHANGE 1
#define USBV5_IOCTL_SHUTDOWN 2
#define USBV5_IOCTL_GETDEVPARAMS 3
#define USBV5_IOCTL_ATTACHFINISH 6
#define USBV5_IOCTL_SETALTERNATE 7
#define USBV5_IOCTL_SUSPEND_RESUME 16
#define USBV5_IOCTL_CANCELENDPOINT 17
#define USBV5_IOCTL_CTRLMSG 18
#define USBV5_IOCTL_INTRMSG 19
#define USBV5_IOCTL_ISOMSG 20
#define USBV5_IOCTL_BULKMSG 21
#define USBV5_IOCTL_MSC_READWRITE_ASYNC 32 /* unimplemented */
#define USBV5_IOCTL_MSC_READ_ASYNC 33      /* unimplemented */
#define USBV5_IOCTL_MSC_WRITE_ASYNC 34     /* unimplemented */
#define USBV5_IOCTL_MSC_READWRITE 35       /* unimplemented */
#define USBV5_IOCTL_MSC_RESET 36           /* unimplemented */
#define DEV_USB_HID4_DEVICE_CHANGE_SIZE 0x180

#define HID_INTF 0x03

#define USB_ENDPOINT_IN 0x80
#define USB_ENDPOINT_OUT 0x00
// TODO: we can just drop HIDv4 much like we dropped HIDv5, and just use the raw usb apis everywhere. The wii is already doing that as it is it appears.
/*============================================================================*/
/* Globals */
/*============================================================================*/

static ios_fd_t dev_usb_hid_fd = -1;
static int8_t started = 0;
#if defined(SUPPORT_DEV_USB_HID5) && defined(SUPPORT_DEV_USB_HID4)
#define HAVE_VERSION
static int8_t version;
#endif
int8_t error;
int8_t errorMethod;
static bool initCalled = false;
static const usb_device_driver_t *usb_device_drivers[] = {
    &gh_guitar_usb_device_driver,
    &gh_drum_usb_device_driver,
    &turntable_usb_device_driver,
    &santroller_usb_device_driver,
    &xbox_controller_usb_device_driver};
static usb_input_device_t fake_devices[MAX_FAKE_WIIMOTES];

static uint32_t dev_oh0_devices[DEV_USB_HID4_DEVICE_CHANGE_SIZE] IOS_ALIGN;
/*============================================================================*/
/* Top level interface to game */
/*============================================================================*/
static void onDevOpen(ios_fd_t fd, usr_t unused);
static void onDevOpenUsb(ios_fd_t fd, usr_t unused);
uint16_t last = 0;
uint8_t last_ext = 0;
static void MyWPADRead(int wiiremote, WPADData_t *data) {
    if (fake_devices[wiiremote].valid) {
        uint32_t isr = OSDisableInterrupts();
        memset(data, 0, WPADDataFormatSize(fake_devices[wiiremote].currentFormat));
        if (fake_devices[wiiremote].currentFormat == fake_devices[wiiremote].format) {
            memcpy(data, &fake_devices[wiiremote].wpadData, WPADDataFormatSize(fake_devices[wiiremote].currentFormat));
        } else {
            // Copy the fields common to all formats.
            memcpy(data, &fake_devices[wiiremote].wpadData, WPADDataFormatSize(WPAD_FORMAT_NONE));
        }
        OSRestoreInterrupts(isr);
    } else {
        WPADRead(wiiremote, data);
    }
}

static WPADStatus_t MyWPADProbe(int wiimote, WPADExtension_t *extension) {
    if (fake_devices[wiimote].valid) {
        uint32_t isr = OSDisableInterrupts();
        if (extension) {
            *extension = fake_devices[wiimote].extension;
        }
        // printf("Wpad probe %d\r\n", fake_devices[wiimote].extension);
        OSRestoreInterrupts(isr);
        return WPAD_STATUS_OK;
    }

    WPADStatus_t ret = WPADProbe(wiimote, extension);
    fake_devices[wiimote].real = ret == WPAD_STATUS_OK;
    return ret;
}

#define NUM_DESC 1
static void MyWPADInit(void) {
    WPADInit();
    initCalled = true;
    printf("Instrument Support Starting\r\n");
    char *gameid = (char *)0x80000000;
    for (int i = 0; i < MAX_FAKE_WIIMOTES; i++) {
        memset(&fake_devices[i], 0, sizeof(usb_input_device_t));
        fake_devices[i].valid = 0;
        fake_devices[i].real = 0;
        fake_devices[i].wiimote = i;
        fake_devices[i].autoSamplingBuffer = 0;
        // GH3 and GH:A predate WPADGtr, and thus the whammy works differently
        fake_devices[i].old_wpad = gameid[0] == 'R' && gameid[1] == 'G' && (gameid[2] == 'H' || gameid[2] == 'V');
    }
    /* On first call only, initialise USB and globals. */
    started = 1;

    printf("OpenAsync: %d\r\n", IOS_OpenAsync(DEV_USB_HID_PATH, 0, onDevOpen, NULL));
}

static WPADConnectCallback_t MyWPADSetConnectCallback(int wiimote, WPADConnectCallback_t newCallback) {
    // printf("Set sc\r\n");
    fake_devices[wiimote].connectCallback = newCallback;
    if (fake_devices[wiimote].valid) {
        // printf("calling cb\r\n");
        newCallback(wiimote, WPAD_STATUS_OK);
    }
    return 0;
}

static WPADExtensionCallback_t MyWPADSetExtensionCallback(int wiimote, WPADExtensionCallback_t newCallback) {
    // printf("Set ec\r\n");
    fake_devices[wiimote].extensionCallback = newCallback;
    return 0;
}

static WPADSamplingCallback_t MyWPADSetSamplingCallback(int wiimote, WPADSamplingCallback_t newCallback) {
    // remember their callback
    // printf("set auto sample cb! %d %d\r\n", wiimote, fake_devices[wiimote].valid);
    if (fake_devices[wiimote].valid) {
        fake_devices[wiimote].samplingCallback = newCallback;
    }
    // return WPADSetSamplingCallback(wiimote, passedCB);
    return 0;
}

static void MyWPADSetAutoSamplingBuf(int wiimote, void *buffer, int count) {
    // printf("set auto sample buf! %d %d\r\n", wiimote, count);
    WPADSetAutoSamplingBuf(wiimote, buffer, count);
    if (!fake_devices[wiimote].valid) {
        return;
    }
    uint32_t isr = OSDisableInterrupts();
    fake_devices[wiimote].autoSamplingBuffer = buffer;
    fake_devices[wiimote].autoSamplingBufferCount = count;
    OSRestoreInterrupts(isr);
}

static int MyWPADGetLatestIndexInBuf(int wiimote) {
    // printf("get auto sample buf! %d\r\n", wiimote);
    if (!fake_devices[wiimote].valid) {
        return WPADGetLatestIndexInBuf(wiimote);
    }
    return fake_devices[wiimote].autoSamplingBufferIndex;
}
static void MyWPADGetAccGravityUnit(int wiimote, WPADExtension_t extension, WPADAccGravityUnit_t *result) {
    if (!fake_devices[wiimote].valid) {
        WPADGetAccGravityUnit(wiimote, extension, result);
        return;
    }
    uint32_t isr = OSDisableInterrupts();
    if (extension == WPAD_EXTENSION_NONE) {
        *result = fake_devices[wiimote].gravityUnit[0];
    } else if (extension == WPAD_EXTENSION_NUNCHUCK) {
        *result = fake_devices[wiimote].gravityUnit[1];
    } else {
        result->acceleration[0] = 0;
        result->acceleration[1] = 0;
        result->acceleration[2] = 0;
    }
    OSRestoreInterrupts(isr);
}

static int MyWPADSetDataFormat(int wiimote, WPADDataFormat_t format) {
    // printf("set df! %d %d %d\r\n", wiimote, format, fake_devices[wiimote].valid);
    if (fake_devices[wiimote].valid) {
        fake_devices[wiimote].currentFormat = format;
        return WPAD_STATUS_OK;
    }
    return WPADSetDataFormat(wiimote, format);
}

static WPADDataFormat_t MyWPADGetDataFormat(int wiimote) {
    // printf("get df! %d %d %d\r\n", wiimote, fake_devices[wiimote].currentFormat, WPADGetDataFormat(wiimote));
    if (!fake_devices[wiimote].valid) {
        return WPADGetDataFormat(wiimote);
    }
    return fake_devices[wiimote].currentFormat;
}

static int MyWPADControlDpd(int wiimote, int command, WPADControlDpdCallback_t callback) {
    if (!fake_devices[wiimote].valid) {
        return WPADControlDpd(wiimote, command, callback);
    }
    fake_devices[wiimote].dpdEnabled = command > 0;
    fake_devices[wiimote].controlDpdCallback = callback;
    if (callback) {
        callback(wiimote, WPAD_STATUS_OK);
    }
    return WPAD_STATUS_OK;
}
static bool MyWPADIsDpdEnabled(int wiimote) {
    if (!fake_devices[wiimote].valid) {
        return WPADIsDpdEnabled(wiimote);
    }
    return fake_devices[wiimote].dpdEnabled;
}
static bool MySCGetScreenSaverMode() {
    return false;
}
static void MyWPADControlMotor(int wiimote, int cmd) {
    if (!fake_devices[wiimote].valid) {
        WPADControlMotor(wiimote, cmd);
        return;
    }
    // GH games pulse rumble when star power is ready or active
    printf("motor! %d %d\r\n", wiimote, cmd);
}
static void MyWPADWriteExtReg(int wiimote, void *buffer, int size, WPADPeripheralSpace_t space, int address, WPADMemoryCallback_t callback) {
    WPADWriteExtReg(wiimote, buffer, size, space, address, callback);
    // DJH writes to this address to turn the euphoria led on and off
    if (address == 0xFB && size == 1 && fake_devices[wiimote].valid) {
        printf("DJH Euphoria LED: %d %d\r\n", wiimote, ((uint8_t *)buffer)[0]);
    }
}
BSLUG_REPLACE(WPADControlMotor, MyWPADControlMotor);
BSLUG_MUST_REPLACE(WPADRead, MyWPADRead);
BSLUG_MUST_REPLACE(WPADInit, MyWPADInit);
BSLUG_MUST_REPLACE(WPADSetConnectCallback, MyWPADSetConnectCallback);
BSLUG_MUST_REPLACE(WPADSetExtensionCallback, MyWPADSetExtensionCallback);
BSLUG_REPLACE(WPADSetSamplingCallback, MyWPADSetSamplingCallback);
BSLUG_REPLACE(WPADSetAutoSamplingBuf, MyWPADSetAutoSamplingBuf);
BSLUG_REPLACE(WPADGetLatestIndexInBuf, MyWPADGetLatestIndexInBuf);
BSLUG_MUST_REPLACE(WPADSetDataFormat, MyWPADSetDataFormat);
BSLUG_REPLACE(WPADGetDataFormat, MyWPADGetDataFormat);
BSLUG_MUST_REPLACE(WPADProbe, MyWPADProbe);
BSLUG_MUST_REPLACE(WPADGetAccGravityUnit, MyWPADGetAccGravityUnit);
BSLUG_REPLACE(WPADControlDpd, MyWPADControlDpd);
BSLUG_MUST_REPLACE(WPADIsDpdEnabled, MyWPADIsDpdEnabled);
BSLUG_REPLACE(SCGetScreenSaverMode, MySCGetScreenSaverMode);
BSLUG_REPLACE(WPADWriteExtReg, MyWPADWriteExtReg);
/*============================================================================*/
/* USB support */
/*============================================================================*/

static void callbackIgnore(ios_ret_t ret, usr_t unused);
static void onDevUsbPoll(ios_ret_t ret, usr_t unused);

#ifdef SUPPORT_DEV_USB_HID4
/* The basic flow for version 4:
 *  1) ioctl GET_VERSION
 *       Check the return value is 0x00040001.
 *  2) ioctl GET_DEVICE_CHANGE
 *       Returns immediately + every time a device is added or removed. Output
 *       describes what is connected.
 *  3) Find an interesting device.
 *  4) ioctl INTERRUPT_OUT
 *       USB interrupt packet to send initialise command to WUP-028.
 *  5) ioctl INTERRUPT_IN
 *       USB interrupt packet to poll device for inputs.
 */

/* Size of the DeviceChange ioctl's return (in words). */
#define DEV_USB_HID4_DEVICE_CHANGE_SIZE 0x180
/* IOCTL numbering for the device. */
#define DEV_USB_HID4_IOCTL_GET_DEVICE_CHANGE 0
#define DEV_USB_HID4_IOCTL_CONTROL 2
#define DEV_USB_HID4_IOCTL_INTERRUPT_IN 3
#define DEV_USB_HID4_IOCTL_INTERRUPT_OUT 4
#define DEV_USB_HID4_IOCTL_GET_VERSION 6
/* Version id. */
#define DEV_USB_HID4_VERSION 0x00040001

static uint32_t dev_usb_hid4_devices[DEV_USB_HID4_DEVICE_CHANGE_SIZE] IOS_ALIGN;
struct interrupt_msg4 {
    uint8_t padding[16];
    uint32_t device;
    uint32_t endpoint;
    uint32_t length;
    void *ptr;
};

static void onDevGetVersion4(ios_ret_t ret, usr_t unused);
static void onDevUsbChange4(ios_ret_t ret, usr_t unused);

static int checkVersion4(ios_cb_t cb, usr_t data) {
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID4_IOCTL_GET_VERSION,
        NULL, 0,
        NULL, 0,
        cb, data);
}
static int getDeviceChange4(ios_cb_t cb, usr_t data) {
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID4_IOCTL_GET_DEVICE_CHANGE,
        NULL, 0,
        dev_usb_hid4_devices, sizeof(dev_usb_hid4_devices),
        cb, data);
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
/* The basic flow for version 5:
 *  1) ioctl GET_VERSION
 *       Check the return value is 0x00050001.
 *  2) ioctl GET_DEVICE_CHANGE
 *       Returns immediately + every time a device is added or removed. Output
 *       describes what is connected.
 *  3) ioctl ATTACH_FINISH
 *       Don't know why, but you have to do this!
 *  4) Find an interesting device.
 *  5) ioctl SET_RESUME
 *       Turn the device on.
 *  5) ioctl GET_DEVICE_PARAMETERS
 *       You have to do this, even if you don't care about the result.
 *  6) ioctl INTERRUPT
 *       USB interrupt packet to send initialise command to WUP-028.
 *  7) ioctl INTERRUPT
 *       USB interrupt packet to  poll device for inputs.
 */

/* Size of the DeviceChange ioctl's return (in strctures). */
#define DEV_USB_HID5_DEVICE_CHANGE_SIZE 0x20
/* Total size of all IOS buffers (in words). */
#define DEV_USB_HID5_TMP_BUFFER_SIZE 0x20
/* IOCTL numbering for the device. */
#define DEV_USB_HID5_IOCTL_GET_VERSION 0
#define DEV_USB_HID5_IOCTL_GET_DEVICE_CHANGE 1
#define DEV_USB_HID5_IOCTL_GET_DEVICE_PARAMETERS 3
#define DEV_USB_HID5_IOCTL_ATTACH_FINISH 6
#define DEV_USB_HID5_IOCTL_SET_RESUME 16
#define DEV_USB_HID5_IOCTL_CONTROL 18
#define DEV_USB_HID5_IOCTL_INTERRUPT 19
/* Version id. */
#define DEV_USB_HID5_VERSION 0x00050001

#define OS_IPC_HEAP_HIGH ((void **)0x80003134)

static struct {
    uint32_t id;
    uint32_t vid_pid;
    uint16_t number;
    uint8_t interface_number;
    uint8_t num_altsettings;
} *dev_usb_hid5_devices;

/* This buffer gets managed pretty carefully. During init it's split 0x20 bytes
 * to 0x60 bytes to store the descriptor. The rest of the time it's split
 * evenly, one half for rumble messages and one half for polls. Be careful! */
static uint32_t *dev_usb_hid5_buffer;

/* Annoyingly some of the buffers for v5 MUST be in MEM2, so we wrap _start to
 * allocate these before the application boots. */
void _start(void);

static void my_start(void) {
    /* Allocate some area in MEM2. */
    dev_usb_hid5_devices = *OS_IPC_HEAP_HIGH;
    dev_usb_hid5_devices -= DEV_USB_HID5_DEVICE_CHANGE_SIZE;
    *OS_IPC_HEAP_HIGH = dev_usb_hid5_devices;

    dev_usb_hid5_buffer = *OS_IPC_HEAP_HIGH;
    dev_usb_hid5_buffer -= DEV_USB_HID5_TMP_BUFFER_SIZE;
    *OS_IPC_HEAP_HIGH = dev_usb_hid5_buffer;

    _start();
}

BSLUG_MUST_REPLACE(_start, my_start);

static void onDevGetVersion5(ios_ret_t ret, usr_t unused);
static void onDevUsbAttach5(ios_ret_t ret, usr_t vcount);
static void onDevUsbChange5(ios_ret_t ret, usr_t unused);
static void onDevUsbResume5(ios_ret_t ret, usr_t unused);
static void onDevUsbParams5(ios_ret_t ret, usr_t unused);

static inline int usb_oh0_ctrl_transfer_async(usb_input_device_t *device, uint8_t bmRequestType,
                                              uint8_t bmRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength,
                                              void *rpData, void (*callback)(int, void *)) {
    static ioctlv vectors[7];

    static uint8_t bmRequestTypeData __attribute__((aligned(32)));
    static uint8_t bmRequestData __attribute__((aligned(32)));
    static uint16_t wValueData __attribute__((aligned(32)));
    static uint16_t wIndexData __attribute__((aligned(32)));
    static uint16_t wLengthData __attribute__((aligned(32)));
    static uint8_t unknownData __attribute__((aligned(32))) = 0;
    bmRequestTypeData = bmRequestType;
    bmRequestData = bmRequest;
    wValueData = __builtin_bswap16(wValue);
    wIndexData = __builtin_bswap16(wIndex);
    wLengthData = __builtin_bswap16(wLength);
    vectors[0].data = &bmRequestTypeData;
    vectors[0].len = sizeof(bmRequestTypeData);
    vectors[1].data = &bmRequestData;
    vectors[1].len = sizeof(bmRequestData);
    vectors[2].data = &wValueData;
    vectors[2].len = sizeof(wValueData);
    vectors[3].data = &wIndexData;
    vectors[3].len = sizeof(wIndexData);
    vectors[4].data = &wLengthData;
    vectors[4].len = sizeof(wLengthData);
    vectors[5].data = &unknownData;
    vectors[5].len = sizeof(unknownData);
    vectors[6].data = rpData;
    vectors[6].len = wLength;

    return IOS_IoctlvAsync(device->host_fd, USBV0_IOCTL_CTRLMSG, 6, 1, vectors,
                           callback, device);
}

static inline int usb_oh0_ctrl_transfer(usb_input_device_t *device, uint8_t bmRequestType, uint8_t bmRequest,
                                        uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *rpData) {
    static ioctlv vectors[7];
    static uint8_t bmRequestTypeData __attribute__((aligned(32)));
    static uint8_t bmRequestData __attribute__((aligned(32)));
    static uint16_t wValueData __attribute__((aligned(32)));
    static uint16_t wIndexData __attribute__((aligned(32)));
    static uint16_t wLengthData __attribute__((aligned(32)));
    static uint8_t unknownData __attribute__((aligned(32))) = 0;
    bmRequestTypeData = bmRequestType;
    bmRequestData = bmRequest;
    wValueData = __builtin_bswap16(wValue);
    wIndexData = __builtin_bswap16(wIndex);
    wLengthData = __builtin_bswap16(wLength);
    vectors[0].data = &bmRequestTypeData;
    vectors[0].len = sizeof(bmRequestTypeData);
    vectors[1].data = &bmRequestData;
    vectors[1].len = sizeof(bmRequestData);
    vectors[2].data = &wValueData;
    vectors[2].len = sizeof(wValueData);
    vectors[3].data = &wIndexData;
    vectors[3].len = sizeof(wIndexData);
    vectors[4].data = &wLengthData;
    vectors[4].len = sizeof(wLengthData);
    vectors[5].data = &unknownData;
    vectors[5].len = sizeof(unknownData);
    vectors[6].data = rpData;
    vectors[6].len = wLength;

    return IOS_Ioctlv(device->host_fd, USBV0_IOCTL_CTRLMSG, 6, 1, vectors);
}

static inline int usb_oh0_intr_transfer(usb_input_device_t *device, bool out, uint16_t wLength, void *rpData) {
    static ioctlv vectors[3];

    static uint8_t endpoint __attribute__((aligned(32)));
    static uint16_t len __attribute__((aligned(32)));
    endpoint = out ? device->endpoint_address_out : device->endpoint_address_in;
    len = wLength;
    vectors[0].data = &endpoint;
    vectors[0].len = sizeof(endpoint);
    vectors[1].data = &len;
    vectors[1].len = sizeof(len);
    vectors[2].data = rpData;
    vectors[2].len = wLength;
    return IOS_Ioctlv(device->host_fd, USBV0_IOCTL_INTRMSG, 2, 1, vectors);
}

static inline int usb_oh0_intr_transfer_async(usb_input_device_t *device, bool out, void *rpData, uint16_t wLength) {
    static ioctlv vectors[3];

    static uint8_t endpoint __attribute__((aligned(32)));
    static uint16_t len __attribute__((aligned(32)));
    endpoint = out ? device->endpoint_address_out : device->endpoint_address_in;
    len = wLength;
    vectors[0].data = &endpoint;
    vectors[0].len = sizeof(endpoint);
    vectors[1].data = &len;
    vectors[1].len = sizeof(len);
    vectors[2].data = rpData;
    vectors[2].len = wLength;

    return IOS_IoctlvAsync(device->host_fd, USBV0_IOCTL_INTRMSG, 2, 1, vectors,
                           onDevUsbPoll, device);
}

static inline void build_v5_ctrl_transfer(struct usb_hid_v5_transfer *transfer, int dev_id,
                                          uint8_t bmRequestType, uint8_t bmRequest, uint16_t wValue, uint16_t wIndex) {
    memset(transfer, 0, sizeof(*transfer));
    transfer->dev_id = dev_id;
    transfer->ctrl.bmRequestType = bmRequestType;
    transfer->ctrl.bmRequest = bmRequest;
    transfer->ctrl.wValue = wValue;
    transfer->ctrl.wIndex = wIndex;
}

static inline void build_v5_intr_transfer(struct usb_hid_v5_transfer *transfer, int dev_id, int endpoint, uint16_t wLength,
                                          void *rpData) {
    memset(transfer, 0, sizeof(*transfer));
    transfer->dev_id = dev_id;
    transfer->intr.bEndpoint = endpoint;
    transfer->intr.rpData = rpData;
    transfer->intr.wLength = wLength;
}

static inline int usb_hid_v5_ctrl_transfer_async(usb_input_device_t *device, uint8_t bmRequestType,
                                                 uint8_t bmRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength,
                                                 void *rpData, void (*callback)(int, void *)) {
    static ioctlv vectors[2];
    int out = !(bmRequestType & USB_ENDPOINT_IN);

    build_v5_ctrl_transfer(&device->transferV5, device->dev_id, bmRequestType, bmRequest, wValue, wIndex);

    vectors[0].data = &device->transferV5;
    vectors[0].len = sizeof(device->transferV5);
    vectors[1].data = rpData;
    vectors[1].len = wLength;

    return IOS_IoctlvAsync(dev_usb_hid_fd, DEV_USB_HID5_IOCTL_CONTROL, 1 - out, 1 + out, vectors,
                           callback, device);
}

static inline int usb_hid_v5_ctrl_transfer(usb_input_device_t *device, uint8_t bmRequestType, uint8_t bmRequest,
                                           uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *rpData) {
    static ioctlv vectors[2];
    int out = !(bmRequestType & USB_ENDPOINT_IN);

    build_v5_ctrl_transfer(&device->transferV5, device->dev_id, bmRequestType, bmRequest, wValue, wIndex);

    vectors[0].data = &device->transferV5;
    vectors[0].len = sizeof(device->transferV5);
    vectors[1].data = rpData;
    vectors[1].len = wLength;

    return IOS_Ioctlv(dev_usb_hid_fd, DEV_USB_HID5_IOCTL_CONTROL, 1 - out, 1 + out, vectors);
}

static inline int usb_hid_v5_intr_transfer(usb_input_device_t *device, bool out, uint16_t wLength, void *rpData) {
    static ioctlv vectors[2];

    build_v5_intr_transfer(&device->transferV5, device->dev_id, out ? device->endpoint_address_out : device->endpoint_address_in, wLength, rpData);

    vectors[0].data = &device->transferV5;
    vectors[0].len = sizeof(device->transferV5);
    vectors[1].data = rpData;
    vectors[1].len = wLength;
    return IOS_Ioctlv(dev_usb_hid_fd, DEV_USB_HID5_IOCTL_INTERRUPT, 2 - (!out), !out, vectors);
}

static inline int usb_hid_v5_intr_transfer_async(usb_input_device_t *device, bool out, void *rpData, uint16_t length) {
    static ioctlv vectors[2];

    build_v5_intr_transfer(&device->transferV5, device->dev_id, out ? device->endpoint_address_out : device->endpoint_address_in, length, rpData);

    vectors[0].data = &device->transferV5;
    vectors[0].len = sizeof(device->transferV5);
    vectors[1].data = rpData;
    vectors[1].len = length;

    return IOS_IoctlvAsync(dev_usb_hid_fd, DEV_USB_HID5_IOCTL_INTERRUPT, 2 - (!out), !out, vectors,
                           onDevUsbPoll, device);
}

static inline void build_v4_ctrl_transfer(struct usb_hid_v4_transfer *transfer, int dev_id,
                                          uint8_t bmRequestType, uint8_t bmRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *rpData) {
    memset(transfer, 0, sizeof(*transfer));
    transfer->dev_id = dev_id;
    transfer->ctrl.bmRequestType = bmRequestType;
    transfer->ctrl.bmRequest = bmRequest;
    transfer->ctrl.wValue = wValue;
    transfer->ctrl.wIndex = wIndex;
    transfer->ctrl.wLength = wLength;
    transfer->data = rpData;
}

static inline void build_v4_intr_transfer(struct usb_hid_v4_transfer *transfer, int dev_id, int out, int dLength, void *rpData) {
    memset(transfer, 0, sizeof(*transfer));
    transfer->dev_id = dev_id;
    transfer->intr.out = out;
    transfer->intr.dLength = dLength;
    transfer->data = rpData;
}

static inline int usb_hid_v4_ctrl_transfer_async(usb_input_device_t *device, uint8_t bmRequestType,
                                                 uint8_t bmRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength,
                                                 void *rpData) {
    build_v4_ctrl_transfer(&device->transferV4, device->dev_id, bmRequestType, bmRequest, wValue, wIndex, wLength, rpData);

    return IOS_IoctlAsync(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_CONTROL, &device->transferV4, sizeof(device->transferV4), NULL, 0,
                          onDevUsbPoll, device);
}

static inline int usb_hid_v4_intr_transfer_async(usb_input_device_t *device, bool out, void *rpData, uint16_t length) {
    if (out) {
        build_v4_intr_transfer(&device->transferV4, device->dev_id, device->endpoint_address_out, length, rpData);
        return IOS_IoctlAsync(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_INTERRUPT_OUT, &device->transferV4, sizeof(device->transferV4), NULL, 0,
                              onDevUsbPoll, device);
    }

    build_v4_intr_transfer(&device->transferV4, device->dev_id, device->endpoint_address_in, length, rpData);
    return IOS_IoctlAsync(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_INTERRUPT_IN, &device->transferV4, sizeof(device->transferV4), NULL, 0,
                          onDevUsbPoll, device);
}

static inline int usb_hid_v4_intr_transfer(usb_input_device_t *device, bool out, void *rpData, uint16_t length) {
    if (out) {
        build_v4_intr_transfer(&device->transferV4, device->dev_id, device->endpoint_address_out, length, rpData);
        return IOS_Ioctl(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_INTERRUPT_OUT, &device->transferV4, sizeof(device->transferV4), NULL, 0);
    }

    build_v4_intr_transfer(&device->transferV4, device->dev_id, device->endpoint_address_in, length, rpData);
    return IOS_Ioctl(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_INTERRUPT_IN, &device->transferV4, sizeof(device->transferV4), NULL, 0);
}

static inline int usb_hid_v4_ctrl_transfer(usb_input_device_t *device, uint8_t bmRequestType, uint8_t bmRequest,
                                           uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *rpData) {
    build_v4_ctrl_transfer(&device->transferV4, device->dev_id, bmRequestType, bmRequest, wValue, wIndex, wLength, rpData);
    return IOS_Ioctl(dev_usb_hid_fd, DEV_USB_HID4_IOCTL_CONTROL, &device->transferV4, sizeof(device->transferV4), NULL, 0);
}

static int checkVersion5(ios_cb_t cb, usr_t data) {
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID5_IOCTL_GET_VERSION,
        NULL, 0,
        dev_usb_hid5_buffer, 0x20,
        cb, data);
}
static int getDeviceChange5(ios_cb_t cb, usr_t data) {
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID5_IOCTL_GET_DEVICE_CHANGE,
        NULL, 0,
        dev_usb_hid5_devices, sizeof(dev_usb_hid5_devices[0]) * DEV_USB_HID5_DEVICE_CHANGE_SIZE,
        cb, data);
}

static int sendAttach5(ios_cb_t cb, usr_t data) {
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID5_IOCTL_ATTACH_FINISH,
        NULL, 0,
        NULL, 0,
        cb, data);
}

static int sendResume5(ios_cb_t cb, usr_t data) {
    usb_input_device_t *device = (usb_input_device_t *)data;
    dev_usb_hid5_buffer[0] = device->dev_id;
    dev_usb_hid5_buffer[1] = 0;
    dev_usb_hid5_buffer[2] = 1;
    dev_usb_hid5_buffer[3] = 0;
    dev_usb_hid5_buffer[4] = 0;
    dev_usb_hid5_buffer[5] = 0;
    dev_usb_hid5_buffer[6] = 0;
    dev_usb_hid5_buffer[7] = 0;
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID5_IOCTL_SET_RESUME,
        dev_usb_hid5_buffer, 0x20,
        NULL, 0,
        cb, data);
}

static int sendParams5(ios_cb_t cb, usr_t data) {
    /* Assumes buffer still in state from sendResume5 */
    return IOS_IoctlAsync(
        dev_usb_hid_fd, DEV_USB_HID5_IOCTL_GET_DEVICE_PARAMETERS,
        dev_usb_hid5_buffer, 0x20,
        dev_usb_hid5_buffer + 8, 0xc0,
        cb, data);
}
#endif

static void onError(void) {
    IOS_CloseAsync(dev_usb_hid_fd, callbackIgnore, NULL);
    dev_usb_hid_fd = -1;
}

static uint32_t dev_oh0_buffer[0x180] IOS_ALIGN;
static void onDevGetDesc2(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    uint16_t size = __builtin_bswap16(dev_oh0_buffer[0]);
    printf("Desc size2: %02x\r\n", size);
    uint8_t *desc = (uint8_t *)dev_oh0_buffer;
    uint8_t *end = desc + size;
    while (desc < end) {
        uint8_t bLength = desc[0];
        uint8_t bDescriptorType = desc[1];
        printf("Len: %01x\r\n", bLength);
        printf("Type: %01x\r\n", bDescriptorType);
        if (bDescriptorType == USB_DT_INTERFACE) {
            usb_interfacedesc *intf = (usb_interfacedesc *)desc;
            printf("Class: %02x %02x %02x\r\n", intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
            if (intf->bInterfaceClass == 0xFF && intf->bInterfaceSubClass == 0x5D && intf->bInterfaceProtocol == 0x01) {
                printf("Found Xbox 360 Wired Controller!\r\n");
                desc += bLength;
                // Xbox desc directly follows interface desc
                xboxiddesc *idf = (xboxiddesc *)desc;
                printf("Subtype: %02x\r\n", idf->subtype);
                device->endpoint_address_in = idf->bEndpointAddressIn;
                device->endpoint_address_out = idf->bEndpointAddressOut;
                device->max_packet_len_in = idf->bMaxDataSizeIn;
                device->max_packet_len_out = idf->bMaxDataSizeOut;
                device->sub_type = idf->subtype;
                device->type = XINPUT_TYPE_WIRED;
                // We found what we are looking for
                break;
            }
            if (intf->bInterfaceClass == 0xFF && intf->bInterfaceSubClass == 0x5D && intf->bInterfaceProtocol == 0x81) {
                printf("Found Xbox 360 Wireless receiver!\r\n");
                desc += bLength;
                bLength = desc[0];
                bDescriptorType = desc[1];
                // Skip over 0x22 descriptor
                desc += bLength;
                uint8_t endpoints = intf->bNumEndpoints;
                while (endpoints--) {
                    usb_endpointdesc *desc_ep = (usb_endpointdesc *)desc;
                    if (desc_ep->bEndpointAddress & 0x80) {
                        device->endpoint_address_in = desc_ep->bEndpointAddress;
                        device->max_packet_len_in = __builtin_bswap16(desc_ep->wMaxPacketSize);
                    } else {
                        device->endpoint_address_out = desc_ep->bEndpointAddress;
                        device->max_packet_len_out = __builtin_bswap16(desc_ep->wMaxPacketSize);
                    }
                    desc += desc_ep->bLength;
                }
                device->type = XINPUT_TYPE_WIRELESS;
                // We found what we are looking for
                break;
            }
        }
        desc += bLength;
    }
    if (device->type != 0) {
        if (device->connectCallback != NULL) {
            printf("Connect callback call!\r\n");
            device->connectCallback(device->wiimote, WPAD_STATUS_OK);
        }
        device->driver->init(device);
    }
}
static void onDevGetDesc1(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    uint16_t size = __builtin_bswap16(dev_oh0_buffer[0]);
    printf("Desc size1: %02x\r\n", size);
    usb_oh0_ctrl_transfer_async(device, 0b10000000, 0x06, USB_DT_CONFIG << 8, 0, size, dev_oh0_buffer, onDevGetDesc2);
}

static void onDevOpenUsbv0(ios_fd_t fd, usr_t usr) {
    usb_input_device_t *device = (usb_input_device_t *)usr;
    const usb_device_driver_t *driver = &xbox_controller_usb_device_driver;
    device->host_fd = fd;
    device->driver = driver;
    device->valid = true;
    usb_oh0_ctrl_transfer_async(device, 0b10000000, 0x06, USB_DT_CONFIG << 8, 0, 4, dev_oh0_buffer, onDevGetDesc1);
}
/*============================================================================*/
/* Start of USB callback chain. Each method calls another as a callback after an
 * IOS command. */
/*============================================================================*/
static ios_fd_t usb_fd;
static uint8_t num_descr __attribute__((aligned(32))) = 1;
static uint8_t iclass __attribute__((aligned(32))) = 0xFF;
static uint8_t cntdevs __attribute__((aligned(32))) = 0;
static void onUsbV0DevList(ios_ret_t ret, usr_t user) {
    IOS_CloseAsync(usb_fd, callbackIgnore, NULL);
    usb_fd = -1;
    printf("Devices: %d\r\n", cntdevs);
    while (cntdevs--) {
        uint16_t vid = (uint16_t)(dev_oh0_devices[cntdevs * 2 + 1] >> 16);
        uint16_t pid = (uint16_t)dev_oh0_devices[cntdevs * 2 + 1];

        usb_input_device_t *device;
        printf("Found device oh0 %04x %04x!\r\n", vid, pid);
        for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
            device = &fake_devices[i];
            if (device->valid || device->real)
                continue;
            break;
        }
        if (!device->valid && !device->real) {
            char devicepath[23];
            printf("VID: %04x, PID: %04x\r\n", vid, pid);
            snprintf(devicepath, sizeof(devicepath), "/dev/usb/oh0/%x/%x", vid, pid);
            printf("Dev path: %s\r\n", devicepath);

            printf("OpenAsync Dev: %d\r\n", IOS_OpenAsync(devicepath, 0, onDevOpenUsbv0, device));
        }
    }
}

static void onDevOpenUsb(ios_fd_t fd, usr_t unused) {
    static ioctlv vectors[4];
    vectors[0].data = &num_descr;
    vectors[0].len = 1;
    vectors[1].data = &iclass;
    vectors[1].len = 1;
    vectors[2].data = &cntdevs;
    vectors[2].len = 1;
    vectors[3].data = dev_oh0_devices;
    vectors[3].len = num_descr << 3;
    usb_fd = fd;
    IOS_IoctlvAsync(fd, USBV0_IOCTL_GETDEVLIST, 2, 2, vectors, onUsbV0DevList, NULL);
}

static void onDevOpen(ios_fd_t fd, usr_t unused) {
    int ret;
    (void)unused;
    printf("    hid dev opened %d\r\n", fd);
    dev_usb_hid_fd = fd;
    if (fd >= 0)
        ret =
#if defined(SUPPORT_DEV_USB_HID4)
            checkVersion4(onDevGetVersion4, NULL);
#elif defined(SUPPORT_DEV_USB_HID5)
            checkVersion5(onDevGetVersion5, NULL);
#else
            -1;
#endif
    else
        ret = fd;
    if (ret) {
        printf("    ... error %d\r\n", fd);
        error = ret;
        errorMethod = 1;
    }
}

static void callbackIgnore(ios_ret_t ret, usr_t unused) {
    (void)unused;
    (void)ret;
}

#ifdef SUPPORT_DEV_USB_HID4
static void onDevGetVersion4(ios_ret_t ret, usr_t unused) {
    (void)unused;
    printf("    onDevGetVersion4 %d\r\n", ret);
    if (ret == DEV_USB_HID4_VERSION) {
#ifdef HAVE_VERSION
        version = 4;
        printf("    Found hidv4\r\n");
#endif

        printf("Opening oh0: %d\r\n", IOS_OpenAsync(DEV_USB_OH0_PATH, 0, onDevOpenUsb, NULL));
        ret = getDeviceChange4(onDevUsbChange4, NULL);
    } else {
#ifdef SUPPORT_DEV_USB_HID5
        ret = checkVersion5(onDevGetVersion5, NULL);
#endif
    }
    if (ret) {
        error = ret;
        errorMethod = 2;
        onError();
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
static void onDevOpenVen(ios_fd_t fd, usr_t usr) {
    printf("Opened ven: %d\r\n", fd);
    IOS_CloseAsync(dev_usb_hid_fd, callbackIgnore, NULL);
    dev_usb_hid_fd = fd;
    getDeviceChange5(onDevUsbChange5, NULL);
}
static void onDevGetVersion5(ios_ret_t ret, usr_t unused) {
    (void)unused;
    if (ret == 0 && dev_usb_hid5_buffer[0] == DEV_USB_HID5_VERSION) {
#ifdef HAVE_VERSION
        version = 5;
        printf("    Found hidv5\r\n");

#endif
        printf("Opening ven: %d\r\n", IOS_OpenAsync(DEV_USB_VEN_PATH, 0, onDevOpenVen, NULL));
    } else if (ret == 0) {
        ret = dev_usb_hid5_buffer[0];
    }
    if (ret) {
        error = ret;
        errorMethod = 3;
        onError();
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID4
static void onDevUsbChange4(ios_ret_t ret, usr_t unused) {
    printf("onDevUsbChange4 %d\r\n", ret);
    if (ret >= 0) {
        int found = 0;
        usb_input_device_t *device;
        const usb_device_driver_t *driver;
        uint16_t packet_size_in = 128;
        uint16_t packet_size_out = 128;
        uint8_t endpoint_address_in = 0;
        uint8_t endpoint_address_out = 0;
        uint32_t vid_pid;
        uint16_t vid, pid;
        for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
            device = &fake_devices[i];
            if (!device->valid || device->host_fd)
                continue;

            found = false;
            for (int i = 0; i < DEV_USB_HID4_DEVICE_CHANGE_SIZE && dev_usb_hid4_devices[i] < sizeof(dev_usb_hid4_devices); i += dev_usb_hid4_devices[i] / 4) {
                uint32_t device_id = dev_usb_hid4_devices[i + 1];
                if (device->dev_id == device_id) {
                    found = true;
                    break;
                }
            }

            /* Oops, it got disconnected */
            if (!found) {
                if (device->driver->disconnect)
                    device->driver->disconnect(device);

                if (device->connectCallback) {
                    device->connectCallback(device->wiimote, WPAD_STATUS_DISCONNECTED);
                }
                /* Set this device as not valid */
                device->valid = false;
            }
        }

        found = false;
        for (int i = 0; i < DEV_USB_HID4_DEVICE_CHANGE_SIZE && dev_usb_hid4_devices[i] < sizeof(dev_usb_hid4_devices); i += dev_usb_hid4_devices[i] / 4) {
            uint32_t device_id = dev_usb_hid4_devices[i + 1];
            vid_pid = dev_usb_hid4_devices[i + 4];
            vid = (vid_pid >> 16) & 0xFFFF;
            pid = vid_pid & 0xFFFF;
            printf("       Found device v4 %04x!\r\n", vid_pid);
            driver = NULL;
            for (int i = 0; i < ARRAY_SIZE(usb_device_drivers); i++) {
                if (usb_device_drivers[i]->probe(vid, pid)) {
                    driver = usb_device_drivers[i];
                    break;
                }
            }
            if (driver != NULL) {
                for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
                    device = &fake_devices[i];
                    if (device->dev_id == device_id) {
                        break;
                    }
                    if (device->valid || device->real)
                        continue;
                    break;
                }
                if (!device->valid && !device->real) {
                    endpoint_address_in = 0;
                    endpoint_address_out = 0;
                    uint32_t total_len = dev_usb_hid4_devices[i] / 4;
                    int j = 2;
                    bool is_hid = false;
                    while (j < total_len) {
                        uint8_t len = (dev_usb_hid4_devices[i + j] >> 24) & 0xFF;
                        uint8_t type = (dev_usb_hid4_devices[i + j] >> 16) & 0xFF;
                        printf("Found desc: %02x %02x\r\n", type, len);
                        if (type == USB_DT_ENDPOINT && is_hid) {
                            uint8_t addr = (dev_usb_hid4_devices[i + j] >> 8) & 0xFF;
                            int in = (addr & USB_ENDPOINT_IN);
                            if (in) {
                                packet_size_in = (dev_usb_hid4_devices[i + j + 1] >> 16) & 0xFFFF;
                                endpoint_address_in = addr;
                                printf("Found in endpoint %02x, size %04d\r\n", endpoint_address_in, packet_size_in);
                            } else {
                                packet_size_out = (dev_usb_hid4_devices[i + j + 1] >> 16) & 0xFFFF;
                                endpoint_address_out = addr;
                                printf("Found out endpoint %02x, size %04d\r\n", endpoint_address_out, packet_size_out);
                            }
                            if (endpoint_address_in && endpoint_address_out) {
                                break;
                            }
                        }
                        if (type == USB_DT_INTERFACE) {
                            uint8_t class = (dev_usb_hid4_devices[i + j + 1] >> 16) & 0xFF;
                            uint8_t subclass = (dev_usb_hid4_devices[i + j + 1] >> 8) & 0xFF;
                            uint8_t protocol = (dev_usb_hid4_devices[i + j + 1]) & 0xFF;
                            printf("Class: %02x, Subclass: %02x, Protocol: %02x\r\n", class, subclass, protocol);
                            is_hid = class == USB_CLASS_HID;
                        }
                        j += (len / 4) + 1;
                    }

                    device->endpoint_address_in = endpoint_address_in;
                    device->endpoint_address_out = endpoint_address_out;
                    device->max_packet_len_in = packet_size_in;
                    device->max_packet_len_out = packet_size_out;
                    found = 1;
                    printf("Found!\r\n");
                    device->dev_id = device_id;
                    device->valid = true;
                    if (device->connectCallback != NULL) {
                        printf("Connect callback call!\r\n");
                        device->connectCallback(device->wiimote, WPAD_STATUS_OK);
                    }
                    device->driver = driver;
                    device->driver->init(device);
                }
                break;
            }
        }
        ret = getDeviceChange4(onDevUsbChange4, NULL);
    }
    if (ret) {
        error = ret;
        errorMethod = 5;
        onError();
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
static void onDevUsbChange5(ios_ret_t ret, usr_t unused) {
    if (ret >= 0) {
        ret = sendAttach5(onDevUsbAttach5, (usr_t)ret);
    }
    if (ret) {
        error = ret;
        errorMethod = 4;
        onError();
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
static void onDevUsbAttach5(ios_ret_t ret, usr_t vcount) {
    if (ret == 0) {
        uint32_t vid_pid;
        uint16_t vid, pid;
        int found = 0;
        usb_input_device_t *device;
        const usb_device_driver_t *driver;
        for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
            device = &fake_devices[i];
            if (!device->valid)
                continue;

            found = false;
            for (int i = 0; i < DEV_USB_HID5_DEVICE_CHANGE_SIZE && i < (ios_ret_t)vcount; i++) {
                uint32_t device_id = dev_usb_hid5_devices[i].id;
                if (device->dev_id == device_id) {
                    found = true;
                    break;
                }
            }

            /* Oops, it got disconnected */
            if (!found) {
                if (device->driver->disconnect)
                    device->driver->disconnect(device);

                /* Tell the fake Wiimote manager we got an input device removal */
                if (device->connectCallback) {
                    device->connectCallback(device->wiimote, WPAD_STATUS_DISCONNECTED);
                }
                /* Set this device as not valid */
                device->valid = false;
            }
        }
        found = false;
        for (int i = 0; i < DEV_USB_HID5_DEVICE_CHANGE_SIZE && i < (ios_ret_t)vcount; i++) {
            uint32_t device_id = dev_usb_hid5_devices[i].id;
            vid_pid = dev_usb_hid5_devices[i].vid_pid;
            vid = (vid_pid >> 16) & 0xFFFF;
            pid = vid_pid & 0xFFFF;
            printf("       Found device v5 %04x %04x!\r\n", vid_pid, device_id);
            driver = NULL;
            for (int i = 0; i < ARRAY_SIZE(usb_device_drivers); i++) {
                if (usb_device_drivers[i]->probe(vid, pid)) {
                    driver = usb_device_drivers[i];
                    break;
                }
            }
            // if (driver != NULL) {
            for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
                device = &fake_devices[i];
                if (device->dev_id == device_id) {
                    break;
                }
                if (device->valid || device->real)
                    continue;
                break;
            }
            if (!device->valid && !device->real) {
                printf("Found!\r\n");
                device->dev_id = device_id;
                device->valid = true;
                device->driver = driver;
                device->waiting = true;
                if (device->connectCallback != NULL) {
                    printf("Connect callback call!\r\n");
                    device->connectCallback(device->wiimote, WPAD_STATUS_OK);
                }
                if (!found) {
                    ret = sendResume5(onDevUsbResume5, device);
                    found = true;
                }
            }
            // }
        }
        ret = getDeviceChange5(onDevUsbChange5, NULL);
    }
    if (ret) {
        error = ret;
        errorMethod = 5;
        onError();
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
static void onDevUsbResume5(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    printf("resume %d %02x\r\n", ret, device->dev_id);
    if (ret == 0) {
        device->waiting = false;
        ret = sendParams5(onDevUsbParams5, device);
    }
    if (ret) {
        error = ret;
        errorMethod = 6;
    }
}
#endif

#ifdef SUPPORT_DEV_USB_HID5
static void onHidV5Desc(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    printf("v5 desc %d, ", ret);
    if (ret > 0) {
        uint16_t size = __builtin_bswap16(dev_oh0_buffer[0]);
        printf("Desc size2: %02x\r\n", size);
        uint8_t *desc = (uint8_t *)dev_oh0_buffer;
        uint8_t *end = desc + size;
        while (desc < end) {
            uint8_t bLength = desc[0];
            uint8_t bDescriptorType = desc[1];
            printf("Len: %01x\r\n", bLength);
            printf("Type: %01x\r\n", bDescriptorType);
            if (bDescriptorType == USB_DT_INTERFACE) {
                usb_interfacedesc *intf = (usb_interfacedesc *)desc;
                printf("Class: %02x %02x %02x\r\n", intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
                if (intf->bInterfaceClass == 0xFF && intf->bInterfaceSubClass == 0x5D && intf->bInterfaceProtocol == 0x01) {
                    printf("Found Xbox 360 Wired Controller descriptor!\r\n");
                    desc += bLength;
                    // Xbox desc directly follows interface desc
                    xboxiddesc *idf = (xboxiddesc *)desc;
                    printf("Subtype: %02x\r\n", idf->subtype);
                    device->endpoint_address_in = idf->bEndpointAddressIn;
                    device->endpoint_address_out = idf->bEndpointAddressOut;
                    device->max_packet_len_in = idf->bMaxDataSizeIn;
                    device->max_packet_len_out = idf->bMaxDataSizeOut;
                    device->sub_type = idf->subtype;
                    device->type = XINPUT_TYPE_WIRED;
                    device->driver->init(device);
                    // We found what we are looking for
                    break;
                }
            }
            desc += bLength;
        }
    }
    if (ret) {
        error = ret;
        errorMethod = 6;
    }
}
static void onDevUsbParams5(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    printf("params %d %02x\r\n", ret, device->dev_id);
    if (ret == 0) {
        usb_configurationdesc *dev = (usb_configurationdesc *)(dev_usb_hid5_buffer + 18);
        usb_interfacedesc *intf = (usb_interfacedesc *)(dev_usb_hid5_buffer + 21);
        usb_endpointdesc *endp = (usb_endpointdesc *)(dev_usb_hid5_buffer + 24);
        printf("len: %02x\r\n", dev->wTotalLength);
        printf("class: %02x %02x %02x\r\n", intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
        device->type = 0;
        if (intf->bInterfaceClass == 0xFF && intf->bInterfaceSubClass == 0x5D && intf->bInterfaceProtocol == 0x01) {
            printf("Found Xbox 360 Wired Controller!\r\n");
            device->type = XINPUT_TYPE_WIRED;
            device->driver = &xbox_controller_usb_device_driver;
            usb_hid_v5_ctrl_transfer_async(device, 0b10000000, 0x06, USB_DT_CONFIG << 8, 0, dev->wTotalLength, dev_oh0_buffer, onHidV5Desc);
        } else if (intf->bInterfaceClass == 0xFF && intf->bInterfaceSubClass == 0x5D && intf->bInterfaceProtocol == 0x81) {
            printf("Found Xbox 360 Wireless receiver!\r\n");
            device->driver = &xbox_controller_usb_device_driver;
            device->type = XINPUT_TYPE_WIRELESS;
        }
        // X360 Wired will handle endpoints and init itself later
        if (device->driver != NULL && device->type != XINPUT_TYPE_WIRED) {
            for (int i = 0; i < intf->bNumEndpoints; i++) {
                printf("Endpoint: %02x\r\n", endp->bEndpointAddress);
                int in = (endp->bEndpointAddress & USB_ENDPOINT_IN);
                if (in) {
                    device->max_packet_len_in = endp->wMaxPacketSize;
                    device->endpoint_address_in = endp->bEndpointAddress;
                    printf("Found in endpoint %02x, size %04d\r\n", device->endpoint_address_in, device->max_packet_len_in);
                } else {
                    device->max_packet_len_out = endp->wMaxPacketSize;
                    device->endpoint_address_out = endp->bEndpointAddress;
                    printf("Found out endpoint %02x, size %04d\r\n", device->endpoint_address_out, device->max_packet_len_out);
                }
                if (device->endpoint_address_in && device->endpoint_address_out) {
                    break;
                }
                endp = (usb_endpointdesc *)(((uint8_t *)endp) + ((endp->bLength + 3) & ~3));
            }
            device->driver->init(device);
        }
        /* 0-7 are already correct :) */
        dev_usb_hid5_buffer[8] = 0;
        dev_usb_hid5_buffer[9] = 0;
        dev_usb_hid5_buffer[10] = 0;
        dev_usb_hid5_buffer[11] = 0;
        dev_usb_hid5_buffer[12] = 0;
        dev_usb_hid5_buffer[13] = 0;
        dev_usb_hid5_buffer[14] = 0;
        dev_usb_hid5_buffer[15] = 0;
        dev_usb_hid5_buffer[16] = device->dev_id;
        dev_usb_hid5_buffer[17] = 0;
        dev_usb_hid5_buffer[18] = 0;
        dev_usb_hid5_buffer[19] = 0;
        dev_usb_hid5_buffer[20] = 0;
        dev_usb_hid5_buffer[21] = 0;
        dev_usb_hid5_buffer[22] = 0;
        dev_usb_hid5_buffer[23] = 0;
        dev_usb_hid5_buffer[24] = 0;
        dev_usb_hid5_buffer[25] = 0;
        dev_usb_hid5_buffer[26] = 0;
        dev_usb_hid5_buffer[27] = 0;
        dev_usb_hid5_buffer[28] = 0;
        dev_usb_hid5_buffer[29] = 0;
        dev_usb_hid5_buffer[30] = 0;
        dev_usb_hid5_buffer[31] = 0;
        if (device->driver == NULL) {
            device->valid = false;
        }
        for (int i = 0; i < ARRAY_SIZE(fake_devices); i++) {
            device = &fake_devices[i];
            if (device->waiting && device->valid) {
                ret = sendResume5(onDevUsbResume5, device);
                break;
            }
        }
    }
    if (ret) {
        error = ret;
        errorMethod = 7;
    }
}
#endif

int usb_device_driver_issue_ctrl_transfer(usb_input_device_t *device, uint8_t requesttype,
                                          uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length) {
    if (device->host_fd) {
        return usb_oh0_ctrl_transfer(device, requesttype, request, value, index, length, data);
    }
#ifdef SUPPORT_DEV_USB_HID5
#ifdef HAVE_VERSION
    if (version == 5)
#endif
        return usb_hid_v5_ctrl_transfer(device, requesttype, request, value, index, length, data);
#endif
#ifdef SUPPORT_DEV_USB_HID4
#ifdef HAVE_VERSION
    if (version == 4)
#endif
        return usb_hid_v4_ctrl_transfer(device, requesttype, request, value, index, length, data);
#endif
    return -1;
}
int usb_device_driver_issue_intr_transfer(usb_input_device_t *device, bool out, void *data, uint16_t length) {
    if (device->host_fd) {
        return usb_oh0_intr_transfer(device, out, length, data);
    }
#ifdef SUPPORT_DEV_USB_HID5
#ifdef HAVE_VERSION
    if (version == 5)
#endif
        return usb_hid_v5_intr_transfer(device, out, length, data);
#endif
#ifdef SUPPORT_DEV_USB_HID4
#ifdef HAVE_VERSION
    if (version == 4)
#endif
        return usb_hid_v4_intr_transfer(device, out, data, length);
#endif
    return -1;
}

int usb_device_driver_issue_ctrl_transfer_async(usb_input_device_t *device, uint8_t requesttype,
                                                uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length) {
    if (device->host_fd) {
        return usb_oh0_ctrl_transfer_async(device, requesttype, request, value, index, length, data, onDevUsbPoll);
    }
#ifdef SUPPORT_DEV_USB_HID5
#ifdef HAVE_VERSION
    if (version == 5)
#endif
        return usb_hid_v5_ctrl_transfer_async(device, requesttype, request, value, index, length, data, onDevUsbPoll);
#endif
#ifdef SUPPORT_DEV_USB_HID4
#ifdef HAVE_VERSION
    if (version == 4)
#endif
        return usb_hid_v4_ctrl_transfer_async(device, requesttype, request, value, index, length, data);
#endif
    return -1;
}
int usb_device_driver_issue_intr_transfer_async(usb_input_device_t *device, bool out, void *data, uint16_t length) {
    if (device->host_fd) {
        return usb_oh0_intr_transfer_async(device, out, data, length);
    }
#ifdef SUPPORT_DEV_USB_HID5
#ifdef HAVE_VERSION
    if (version == 5)
#endif
        return usb_hid_v5_intr_transfer_async(device, out, data, length);
#endif
#ifdef SUPPORT_DEV_USB_HID4
#ifdef HAVE_VERSION
    if (version == 4)
#endif
        return usb_hid_v4_intr_transfer_async(device, out, data, length);
#endif
    return -1;
}

static void onDevUsbPoll(ios_ret_t ret, usr_t user) {
    usb_input_device_t *device = (usb_input_device_t *)user;
    if (ret >= 0) {
        device->driver->usb_async_resp(device);
        if (device->autoSamplingBuffer) {
            int autoSampleIndex = device->autoSamplingBufferIndex;
            int autoSampleNext = (autoSampleIndex + 1) % device->autoSamplingBufferCount;
            WPADData_t *nextPos = (WPADData_t *)((char *)device->autoSamplingBuffer + autoSampleNext * WPADDataFormatSize(device->currentFormat));
            MyWPADRead(device->wiimote, nextPos);
            device->autoSamplingBufferIndex = autoSampleNext;
        }
        if (device->samplingCallback != 0) {
            device->samplingCallback(device->wiimote);
        }
    }
    if (ret < 0) {
        printf("Error: %d\r\n", ret);
        error = ret;
        errorMethod = 9;
    }
}
