
#include "app_usbd_cfg.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4) const uint8_t USB_DeviceDescriptor[] = {

	/* 0  bLength            1  0x12  Size of this descriptor, in bytes. */
	/* 1  bDescriptorType    1  0x01  DEVICE descriptor. */
	/* 2  bcdUSB             2 0x0110 1.10 - current revision of USB */
	/*                                specification. */
	/* 4  bDeviceClass       1  0x00  Device defined at Interface level. */
	/* 5  bDeviceSubClass    1  0x00  Unused. */
	/* 6  bDeviceProtocol    1  0x00  Unused. */
	/* 7  bMaxPacketSize0    1  0x08  8 bytes. */
	/* 8  idVendor           2 0xXXXX Vendor ID. */
	/* 10 idProduct          2 0xXXXX Product ID. */
	/* 12 bcdDevice          2 0xXXXX Device Release Code. */
	/* 14 iManufacturer      1  0x01  Index to string descriptor that contains */
	/*                                the string <Your Name> in Unicode. */
	/* 15 iProduct           1  0x02  Index to string descriptor that contains */
	/*                                the string <Your Product Name> in */
	/*                                Unicode. */
	/* 16 iSerialNumber      1  0x00  Unused. */
	/* 17 bNumConfigurations 1  0x01  One configuration */

	0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 64, 
	0x8c, 0x19, 
	0x0d, 0x1c,
	0x00, 0x01, 0x01, 0x02, 0x00, 0x01, 
};

/**
 * USB Device Qualifier
 */
ALIGNED(4) const uint8_t USB_DeviceQualifier[] = {
	USB_DEVICE_QUALI_SIZE,				/* bLength */
	USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0200),					/* bcdUSB: 2.00 */
	0x00,						/* bDeviceClass */
	0x00,						/* bDeviceSubClass */
	0x00,						/* bDeviceProtocol */
	USB_MAX_PACKET0,				/* bMaxPacketSize0 */
	0x01,						/* bNumOtherSpeedConfigurations */
	0x00						/* bReserved */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4) uint8_t USB_FsConfigDescriptor[] = {

	/* Configuration Descriptor */

	/* 0 bLength         1  0x09  Size of this descriptor, in bytes. */
	/* 1 bDescriptorType 1  0x02  CONFIGURATION descriptor. */
	/* 2 wTotalLength    2 0x00XX Length of the total configuration block, */
	/*                            including this descriptor, in bytes. */
	/* 4 bNumInterfaces     1 0x02 Two interfaces. */
	/* 5 bConfigurationValu 1 0x01 ID of this configuration. */
	/* 6 iConfiguration     1 0x00 Unused. */
	/* 7 bmAttributes       1 0x80 Bus Powered device, not Self Powered, */
	/*                             no Remote wakeup capability. */
	/* 8 MaxPower           1 0x32 100 mA Max. power consumption. */

	0x09, 0x02, 0x7c, 0x00, 0x03, 0x01, 0x00, 0xa0, 0x32, 

	/* Standard AC Interface Descriptor */

	/* 0 bLength             1    0x09 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType     1    0x04 INTERFACE descriptor. */
	/* 2 bInterfaceNumber    1    0x00 Index of this interface. */
	/* 3 bAlternateSetting   1    0x00 Index of this setting. */
	/* 4 bNumEndpoints       1    0x00 0 endpoints. */
	/* 5 bInterfaceClass     1    0x01 AUDIO. */
	/* 6 bInterfaceSubclass  1    0x01 AUDIO_CONTROL. */
	/* 7 bInterfaceProtocol  1    0x00 Unused. */
	/* 8 iInterface          1    0x00 Unused. */

	0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 

	/* Class-specific AC Interface Descriptor */

	/* 0 bLength            1  0x09  Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1  0x24  CS_INTERFACE. */
	/* 2 bDescriptorSubtype 1  0x01  HEADER subtype. */
	/* 3 bcdADC             2 0x0100 Revision of class specification - 1.0 */
	/* 5 wTotalLength       2 0x0009 Total size of class specific descriptors. */
	/* 7 bInCollection      1  0x01  Number of streaming interfaces. */
	/* 8 baInterfaceNr(1)   1  0x01  MIDIStreaming interface 1 belongs to this */
	/*                               AudioControl interface. */

	0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, 

	/* Standard MS Interface Descriptor */

	/* 0 bLength            1 0x09 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x04 INTERFACE descriptor. */
	/* 2 bInterfaceNumber   1 0x01 Index of this interface. */
	/* 3 bAlternateSetting  1 0x00 Index of this alternate setting. */
	/* 4 bNumEndpoints      1 0x02 2 endpoints. */
	/* 5 bInterfaceClass    1 0x01 AUDIO. */
	/* 6 bInterfaceSubclass 1 0x03 MIDISTREAMING. */
	/* 7 bInterfaceProtocol 1 0x00 Unused. */
	/* 8 iInterface         1 0x00 Unused. */

	0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, 

	/* Class-specific MS Interface Descriptor */
	/*  */
	/* 0 bLength            1  0x07  Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1  0x24  CS_INTERFACE descriptor. */
	/* 2 bDescriptorSubtype 1  0x01  MS_HEADER subtype. */
	/* 3 BcdADC             2 0x0100 Revision of this class specification. */
	/* 5 wTotalLength       2 0x0041 Total size of class-specific descriptors . */

	0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,

	/* MIDI IN Jack Descriptor */

	/* 0 bLength            1 0x06 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x24 CS_INTERFACE descriptor. */
	/* 2 bDescriptorSubtype 1 0x02 MIDI_IN_JACK subtype. */
	/* 3 bJackType          1 0x01 EMBEDDED. */
	/* 4 bJackID            1 0x01 ID of this Jack. */
	/* 5 iJack              1 0x00 Unused. */

	0x06, 0x24, 0x02, 0x01, 0x01, 0x00, 

	/* MIDI Adapter MIDI IN Jack Descriptor (External) */

	/* 0 bLength            1 0x06 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x24 CS_INTERFACE descriptor. */
	/* 2 bDescriptorSubtype 1 0x02 MIDI_IN_JACK subtype. */
	/* 3 bJackType          1 0x02 EXTERNAL. */
	/* 4 bJackID            1 0x02 ID of this Jack. */
	/* 5 iJack              1 0x00 Unused. */

	0x06, 0x24, 0x02, 0x02, 0x02, 0x00, 

	/* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */

	/* 0 bLength 1 0x09 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x24 CS_INTERFACE descriptor. */
	/* 2 bDescriptorSubtype 1 0x03 MIDI_OUT_JACK subtype. */
	/* 3 bJackType          1 0x01 EMBEDDED. */
	/* 4 bJackID            1 0x03 ID of this Jack. */
	/* 5 bNrInputPins       1 0x01 Number of Input Pins of this Jack. */
	/* 6 BaSourceID(1)      1 0x02 ID of the Entity to which this Pin is */
	/*                             connected. */
	/* 7 BaSourcePin(1)     1 0x01 Output Pin number of the Entity to which */
	/*                             this Input Pin is connected. */
	/* 8 iJack              1 0x00 Unused. */

	0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00, 

	/* MIDI Adapter MIDI OUT Jack Descriptor (External) */

	/* 0 bLength            1 0x09 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x24 CS_INTERFACE descriptor. */
	/* 2 bDescriptorSubtype 1 0x03 MIDI_OUT_JACK subtype. */
	/* 3 bJackType          1 0x02 EXTERNAL. */
	/* 4 bJackID            1 0x04 ID of this Jack. */
	/* 5 bNrInputPins       1 0x01 Number of Input Pins of this Jack. */
	/* 6 BaSourceID(1)      1 0x01 ID of the Entity to which this Pin is */
	/*                             connected. */
	/* 7 BaSourcePin(1)     1 0x01 Output Pin number of the Entity to which */
	/*                             this Input Pin is connected. */
	/* 8 iJack              1 0x00 Unused. */

	0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00, 

	/* Standard Bulk OUT Endpoint Descriptor */

	/* 0 bLength               1      0x09     Size of this descriptor, in bytes. */
	/* 1 bDescriptorType       1      0x05     ENDPOINT descriptor. */
	/* 2 bEndpointAddress      1      0x01     OUT Endpoint 1. */
	/* 3 bmAttributes          1      0x02     Bulk, not shared. */
	/* 4 wMaxPacketSize        2     0x0040    64 bytes per packet. */
	/* 6 bInterval             1      0x00     Ignored for Bulk. Set to zero. */
	/* 7 bRefresh              1      0x00     Unused. */
	/* 8 bSynchAddress         1      0x00     Unused. */

	0x09, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 

	/* Class-specific MS Bulk OUT Endpoint Descriptor */

	/* 0 bLength            1 0x05 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType    1 0x25 CS_ENDPOINT descriptor */
	/* 2 bDescriptorSubtype 1 0x01 MS_GENERAL subtype. */
	/* 3 bNumEmbMIDIJack    1 0x01 Number of embedded MIDI IN Jacks. */
	/* 4 BaAssocJackID(1)   1 0x01 ID of the Embedded MIDI IN Jack. */

	0x05, 0x25, 0x01, 0x01, 0x01, 
	
	/* Standard Bulk IN Endpoint Descriptor */

	/* 0 bLength         1 0x09 Size of this descriptor, in bytes. */
	/* 1 bDescriptorType 1 0x05 ENDPOINT descriptor. */
	/* 2 bEndpointAddress 1  0x81  IN Endpoint 1. */
	/* 3 bmAttributes     1  0x02  Bulk, not shared. */
	/* 4 wMaxPacketSize   2 0x0040 64 bytes per packet. */
	/* 6 bInterval        1  0x00  Ignored for Bulk. Set to zero. */
	/* 7 bRefresh         1  0x00  Unused. */
	/* 8 bSynchAddress    1  0x00  Unused. */
	
	0x09, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 
	
	/* Class-specific MS Bulk IN Endpoint Descriptor */

	/* 0        bLength               1      0x05     Size of this descriptor, in bytes. */
	/* 1        bDescriptorType       1      0x25     CS_ENDPOINT descriptor */
	/* 2        bDescriptorSubtype    1      0x01     MS_GENERAL subtype. */
	/* 3        bNumEmbMIDIJack       1      0x01     Number of embedded MIDI OUT Jacks. */
	/* 4        BaAssocJackID(1)      1      0x03     ID of the Embedded MIDI OUT Jack. */
	
	0x05, 0x25, 0x01, 0x01, 0x03,

	/* Interface Descriptor: */
	/*   bLength                 9 */
	/*   bDescriptorType         4 */
	/*   bInterfaceNumber        0 */
	/*   bAlternateSetting       0 */
	/*   bNumEndpoints           2 */
	/*   bInterfaceClass       255 Vendor Specific Class */
	/*   bInterfaceSubClass      0  */
	/*   bInterfaceProtocol      0  */
	/*   iInterface              0  */

	9, 4, 2, 0, 2, 255, 0, 0, 0,

	/*   Endpoint Descriptor: */
	/*     bLength                 7 */
	/*     bDescriptorType         5 */
	/*     bEndpointAddress     0x82  EP 1 IN */
	/*     bmAttributes            2 */
	/*       Transfer Type            Bulk */
	/*       Synch Type               None */
	/*       Usage Type               Data */
	/*     wMaxPacketSize     0x0040  1x 64 bytes */
	/*     bInterval               1 */

	7, 5, 0x82, 2, 0x40, 0x00, 1,

	/*   Endpoint Descriptor: */
	/*     bLength                 7 */
	/*     bDescriptorType         5 */
	/*     bEndpointAddress     0x02  EP 2 OUT */
	/*     bmAttributes            2 */
	/*       Transfer Type            Bulk */
	/*       Synch Type               None */
	/*       Usage Type               Data */
	/*     wMaxPacketSize     0x0040  1x 64 bytes */
	/*     bInterval               0 */

	7, 5, 0x02, 2, 0x40, 0x00, 0,


	/* Terminator */
	0						/* bLength */
};

/**
 * USB HSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4) uint8_t USB_HsConfigDescriptor[] = {

	/* Cloned from Akai keyboard */

	0x09, 0x02, 0x65, 0x00, 0x02, 0x01, 0x00, 0xa0, 0x32, 0x09, 0x04, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x00, 0x00, 0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, 0x09, 0x04, 0x01,
	0x00, 0x02, 0x01, 0x03, 0x00, 0x00, 0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00, 0x06, 0x24, 0x02,
	0x01, 0x01, 0x00, 0x06, 0x24, 0x02, 0x02, 0x02, 0x00, 0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02,
	0x01, 0x00, 0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00, 0x09, 0x05, 0x01, 0x02, 0x40,
	0x00, 0x00, 0x00, 0x00, 0x05, 0x25, 0x01, 0x01, 0x01, 0x09, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x05, 0x25, 0x01, 0x01, 0x03,

	/* Terminator */
	0						/* bLength */
};

/**
 * USB String Descriptor (optional)
 */
ALIGNED(4) const uint8_t USB_StringDescriptor[] = {
	/* Index 0x00: LANGID Codes */
	0x04,						/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	WBVAL(0x0409),					/* wLANGID  0x0409 = US English*/
	/* Index 0x01: Manufacturer */
	(3 * 2 + 2),					/* bLength (13 Char + Type + length) */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	'M', 0,
	'o', 0,
	'd', 0,
	/* Index 0x02: Product */
	(9 * 2 + 2),					/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	'B', 0,
	'l', 0,
	'i', 0,
	'p', 0,
	' ', 0,
	'P', 0,
	'o', 0,
	'r', 0,
	't', 0,
	/* Index 0x03: Serial Number */
	(6 * 2 + 2),					/* bLength (8 Char + Type + length) */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	'I', 0,
	'c', 0,
	'o', 0,
	'-', 0,
	'7', 0,
	'4', 0,
	/* Index 0x04: Interface 1, Alternate Setting 0 */
	( 4 * 2 + 2),					/* bLength (4 Char + Type + length) */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	'B', 0,
	'l', 0,
	'i', 0,
	'p', 0,
};
