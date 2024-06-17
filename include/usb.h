#ifndef _USB2_H_
#define _USB2_H_
#include "defines.h"
#include <stdint.h>

/* Constants */
#define USB_MAXPATH			64

#define USB_OK				0
#define USB_FAILED			1

#define USB_CLASS_HID			0x03
#define USB_SUBCLASS_BOOT		0x01
#define USB_PROTOCOL_KEYBOARD		0x01
#define USB_PROTOCOL_MOUSE		0x02
#define USB_REPTYPE_INPUT		0x01
#define USB_REPTYPE_OUTPUT		0x02
#define USB_REPTYPE_FEATURE		0x03
#define USB_REQTYPE_GET			0xA1
#define USB_REQTYPE_SET			0x21

/* Descriptor types */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_HID				0x21
#define USB_DT_REPORT			0x22

/* Standard requests */
#define USB_REQ_GETSTATUS		0x00
#define USB_REQ_CLEARFEATURE		0x01
#define USB_REQ_SETFEATURE		0x03
#define USB_REQ_SETADDRESS		0x05
#define USB_REQ_GETDESCRIPTOR		0x06
#define USB_REQ_SETDESCRIPTOR		0x07
#define USB_REQ_GETCONFIG		0x08
#define USB_REQ_SETCONFIG		0x09
#define USB_REQ_GETINTERFACE		0x0a
#define USB_REQ_SETINTERFACE		0x0b
#define USB_REQ_SYNCFRAME		0x0c

#define USB_REQ_GETPROTOCOL		0x03
#define USB_REQ_SETPROTOCOL		0x0B
#define USB_REQ_GETREPORT		0x01
#define USB_REQ_SETREPORT		0x09

/* Descriptor sizes per descriptor type */
#define USB_DT_DEVICE_SIZE		18
#define USB_DT_CONFIG_SIZE		9
#define USB_DT_INTERFACE_SIZE		9
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define USB_DT_HID_SIZE				9
#define USB_DT_HUB_NONVAR_SIZE		7

/* control message request type bitmask */
#define USB_CTRLTYPE_DIR_HOST2DEVICE	(0<<7)
#define USB_CTRLTYPE_DIR_DEVICE2HOST	(1<<7)
#define USB_CTRLTYPE_TYPE_STANDARD	(0<<5)
#define USB_CTRLTYPE_TYPE_CLASS		(1<<5)
#define USB_CTRLTYPE_TYPE_VENDOR	(2<<5)
#define USB_CTRLTYPE_TYPE_RESERVED	(3<<5)
#define USB_CTRLTYPE_REC_DEVICE		0
#define USB_CTRLTYPE_REC_INTERFACE	1
#define USB_CTRLTYPE_REC_ENDPOINT	2
#define USB_CTRLTYPE_REC_OTHER		3

#define USB_REQTYPE_INTERFACE_GET	(USB_CTRLTYPE_DIR_DEVICE2HOST | USB_CTRLTYPE_TYPE_CLASS | USB_CTRLTYPE_REC_INTERFACE)
#define USB_REQTYPE_INTERFACE_SET	(USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_CLASS | USB_CTRLTYPE_REC_INTERFACE)
#define USB_REQTYPE_ENDPOINT_GET	(USB_CTRLTYPE_DIR_DEVICE2HOST | USB_CTRLTYPE_TYPE_CLASS | USB_CTRLTYPE_REC_ENDPOINT)
#define USB_REQTYPE_ENDPOINT_SET	(USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_CLASS | USB_CTRLTYPE_REC_ENDPOINT)

#define USB_FEATURE_ENDPOINT_HALT	0

#define USB_ENDPOINT_INTERRUPT		0x03
#define USB_ENDPOINT_IN			0x80
#define USB_ENDPOINT_OUT		0x00

#define USB_OH0_DEVICE_ID		0x00000000				// for completion
#define USB_OH1_DEVICE_ID		0x00200000


/* Structures */
typedef struct _usbendpointdesc
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} ATTRIBUTE_PACKED usb_endpointdesc;

typedef struct _usbinterfacedesc
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
	uint8_t *extra;
	uint8_t extra_size;
	struct _usbendpointdesc *endpoints;
} ATTRIBUTE_PACKED usb_interfacedesc;

typedef struct _usbconfdesc
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
	struct _usbinterfacedesc *interfaces;
} ATTRIBUTE_PACKED usb_configurationdesc;

typedef struct _usbdevdesc
{
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
	struct _usbconfdesc *configurations;
} ATTRIBUTE_PACKED usb_devdesc;

typedef struct _usbhiddesc
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	struct {
		uint8_t bDescriptorType;
		uint16_t wDescriptorLength;
	} descr[1];
} ATTRIBUTE_PACKED usb_hiddesc;

typedef struct _usb_device_entry {
	int32_t device_id;
	uint16_t vid;
	uint16_t pid;
	uint32_t token;
} usb_device_entry;

#endif
