#include "stdint.h"
#include "defines.h"
#include "rvl/WPAD.h"
#include <rvl/ipc.h>

/* List of Vendor IDs */
#define SONY_VID				0x054c
#define SONY_INST_VID			0x12ba
#define SANTROLLER_VID			0x1209
#define HORI_VID				0x0f0d

/* List of Product IDs */
#define GH_GUITAR_PID			0x0100
#define GH_DRUM_PID				0x0120
#define DJ_TURNTABLE_PID		0x0140
#define SANTROLLER_PID			0x2882
#define HORI_SWITCH_TAIKO_PID	0x00f0
#define DS3_PID					0x0268
#define DS4_PID_1				0x05c4
#define DS4_PID_2				0x09cc

#define XINPUT_TYPE_WIRED 1
#define XINPUT_TYPE_WIRELESS 2

#define API_TYPE_VEN 1
#define API_TYPE_OH0 2
#define API_TYPE_HIDV4 3
#define API_TYPE_HIDV5 4


typedef struct usb_device_driver_t usb_device_driver_t;
typedef struct usb_input_device_t usb_input_device_t;

typedef struct usb_device_driver_t {
	bool (*probe)(uint16_t vid, uint16_t pid);
	bool hid;
	int (*init)(usb_input_device_t *device);
	int (*disconnect)(usb_input_device_t *device);
	int (*usb_async_resp)(usb_input_device_t *device);
} usb_device_driver_t;

/* USBv5 HID message structure */
struct usb_hid_v5_transfer {
	uint32_t dev_id;
	uint32_t zero;

	union {
		struct {
			uint8_t bmRequestType;
			uint8_t bmRequest;
			uint16_t wValue;
			uint16_t wIndex;
		} ctrl;

		struct {
			void *rpData;
			uint16_t wLength;
			uint8_t bEndpoint;
		} intr;


		struct {
			uint32_t out;
		} intr_hid;

		uint32_t data[14];
	};
} ATTRIBUTE_PACKED;


struct usb_hid_v4_transfer {
	uint8_t padding[16]; // anything you want can go here
	int32_t dev_id;
	union {
		struct {
			uint8_t bmRequestType;
			uint8_t bmRequest;
			uint16_t wValue;
			uint16_t wIndex;
			uint16_t wLength;
		} ctrl;
		struct {
			uint32_t out;
			uint32_t dLength;
		} intr;
	};
	void* data; // virtual pointer, not physical!
} ATTRIBUTE_PACKED; // 32 bytes


struct device_id_t {
	uint16_t vid;
	uint16_t pid;
};



typedef struct usb_input_device_t {
	bool valid;
	bool real;
	bool suspended;
	bool old_wpad;
	/* VID and PID */
	uint16_t vid;
	uint16_t pid;
	uint16_t max_packet_len_in;
	uint16_t max_packet_len_out;
	uint8_t endpoint_address_in;
	uint8_t endpoint_address_out;
	uint8_t sub_type;
	uint8_t type;
	uint8_t state;
	uint8_t api_type;
    uint8_t wiimote;
	/* Used to communicate with Wii's USB module */
	ios_fd_t host_fd;
    bool dpdEnabled;
	bool waiting;
	int led_state;
	bool last_rumble_on;
	bool rumble_on;
	bool last_euphoria_led;
	bool euphoria_led;
	uint32_t dev_id;
	uint16_t number;
    WPADDataFormat_t format;
    WPADDataFormat_t currentFormat;
    WPADStatus_t status;
    WPADExtension_t extension;
    WPADData_t input;
    WPADAccGravityUnit_t gravityUnit[2]; 
    WPADConnectCallback_t connectCallback;
    WPADExtensionCallback_t extensionCallback;
    WPADSamplingCallback_t samplingCallback;
    WPADControlDpdCallback_t controlDpdCallback;
    void *autoSamplingBuffer;
    int autoSamplingBufferCount;
    int autoSamplingBufferIndex;
	/* Driver that handles this device */
	const usb_device_driver_t *driver;
	/* Buffer where we store the USB async respones */
	uint8_t usb_async_resp[128] IOS_ALIGN;
	struct usb_hid_v4_transfer transferV4 IOS_ALIGN; 
    WPADData_t wpadData;
} usb_input_device_t;

static inline bool usb_driver_is_comaptible(uint16_t vid, uint16_t pid, const struct device_id_t *ids, int num)
{
	for (int i = 0; i < num; i++) {
		if (ids[i].vid == vid && ids[i].pid == pid)
			return true;
	}

	return false;
}

extern const usb_device_driver_t gh_guitar_usb_device_driver;
extern const usb_device_driver_t gh_drum_usb_device_driver;
extern const usb_device_driver_t turntable_usb_device_driver;
extern const usb_device_driver_t santroller_usb_device_driver;
extern const usb_device_driver_t xbox_controller_usb_device_driver;
extern const usb_device_driver_t ds3_usb_device_driver;
extern const usb_device_driver_t ds4_usb_device_driver;
extern const usb_device_driver_t switch_taiko_usb_device_driver;



/* Used by USB device drivers */
int usb_device_driver_issue_ctrl_transfer_async(usb_input_device_t *device, uint8_t requesttype,
						uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length);
int usb_device_driver_issue_intr_transfer_async(usb_input_device_t *device, bool out, void *data, uint16_t length);
int usb_device_driver_issue_ctrl_transfer(usb_input_device_t *device, uint8_t requesttype,
						uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length);
int usb_device_driver_issue_intr_transfer(usb_input_device_t *device, bool out, void *data, uint16_t length);
int ps3_set_leds(usb_input_device_t *device);