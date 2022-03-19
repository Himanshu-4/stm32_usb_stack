#include "usb_standard.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////  defination of descripotr here /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define PACKED __attribute__((__packed__))
#define EXTERNAL __attribute__((externally_visible))
///////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t device_descriptor[] ALIGN =
{
    0x12,             //  bLength;    // the size of the descriptor  (0x12)
    USB_DESCRIPTOR_TYPE_DEVICE,            //  bDescriptorType;  // the descriptor type aka constant device (0x01)
    0x00,0x02,      //  bcdUSB;     // USB specification release number (BCD)
    0x00,            //  bDeviceClass;  //  USB device class 
    0x00,            //  bDeviceSubClass;  // Usb device sub class
    0x00,            //  bDeviceProtocol;   // USB protocol code 
    0x40,           //   64 bytes in hex// max packet size for endpoint 0
    LOW_BYTE(STM32_VID),HIGH_BYTE(STM32_VID),              //  idVendor;         // vendor Id
    LOW_BYTE(STM32_PID), HIGH_BYTE(STM32_PID),                //  idProduct;        // product id 
    0x00,0x01,           //  bcdDevice;       // device release number (BCD)
    0x01,                //  iManufacturer;   // index of the string descriptor for the manufacturer
    0x02,                //  iProduct;        // index of string descriptor for the product 
    0x00,               //  iSerialNumber;   // index of string descriptor for the serial number 
    0x01      //  bNumConfigurations;  // number o
  
};



///////////////////////////////////////////////////////////////////////////////////
/////////////////// define the hid report descriptor/////////////////////////////////////////
const uint8_t hid_report_descriptor[] ALIGN EXTERNAL  =
{
0x05, 0x01, // Usage Page (Generic Desktop Ctrls) 
 
0x09, 0x02, // Usage (Mouse) 
0xA1, 0x01, // Collection (Application) 
0x09, 0x01, // Usage (Pointer) 
0xA1, 0x00, // Collection (Physical) 
0x05, 0x09, // Usage Page (Button) 
0x19, 0x01, // Usage Minimum (0x01) 
0x29, 0x03, // Usage Maximum (0x03) 
0x15, 0x00, // Logical Minimum (0) 
0x25, 0x01, // Logical Maximum (1) 
0x95, 0x03, // Report Count (3) 
0x75, 0x01, // Report Size (1) 
0x81, 0x02, // Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) 
0x95, 0x01, // Report Count (1) 
0x75, 0x05, // Report Size (5) 
0x81, 0x01, // Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position) 
 
0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
0x09, 0x30, // Usage (X) 
0x09, 0x31, // Usage (Y) 
0x09, 0x38, // Usage (Wheel) 
0x15, 0x81, // Logical Minimum (-127) 
0x25, 0x7F, // Logical Maximum (127) 
0x75, 0x08, // Report Size (8) 
0x95, 0x03, // Report Count (3) 
0x81, 0x06, // Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position) 
0xC0, // End Collection 
 
0x09, 0x3C, // Usage (Motion Wakeup) 
0x05, 0xFF, // Usage Page (Reserved 0xFF) 
0x09, 0x01, // Usage (0x01) 
0x15, 0x00, // Logical Minimum (0) 
0x25, 0x01, // Logical Maximum (1) 
0x75, 0x01, // Report Size (1) 
0x95, 0x02, // Report Count (2) 
0xB1, 0x22, // Feature (Data,Var,Abs,No Wrap,Linear,No Preferred State,No Null Position,Non-volatile) 
0x75, 0x06, // Report Size (6) 
0x95, 0x01, // Report Count (1) 
0xB1, 0x01, // Feature (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile) 
0xC0, // End Collection


};



const uint8_t configuration_desc_comb[] ALIGN  =
{ 
    // configuration desc

    0x09,     // bLength;    // the size of the descriptor (0x09)
    USB_DESCRIPTOR_TYPE_CONFIGURATION,     // bDescriptorType;  // the descriptor type aka constant configuration (0x02)
    0x22, 0x00, // wTotalLength;     // the number of the bytes in the configuration descriptor and all of its subordinate descriptor 
    0x01,     // bNumInterfaces;   // number of interfaces in the configuration
    0x01,    // bConfigurationValue;  // identifier for set configuration and get configuration request 
    0x00,    // iConfiguration;     //index of string descriptor for the configuration 
    0xA0,   // bmAttributes;       // bus power and remote wakeup settings
    0x50,   // bMaxPower;         // bus power required in units of 2ma(usb 2.0) and for (USB 3.0) 8ma  

    //interface descriptor
    
    0x09,                              //   bLength; // size of descriptor  (0x09)
    USB_DESCRIPTOR_TYPE_INTERFACE,       //   bDescriptorType; // the constant interface (0x04)
    0x00,       //   bInterfaceNumber; // number identifying this interface 
    0x00,       //   bAlternateSetting;  // a number that identifies a descriptor with alternate setting for this interface number
    0x01,       //   bNumEndpoints; only one endp
    USB_CLASS_HID,     //   bInterfaceClass;  // hid class  
    0x01,       //   bInterfaceSubClass; // boot interface  
    0x02,       //   bInterfaceProtocol;  //  mouse  
    0x00,       //   iInterface;          // index of string descriptor for the interface 

    /// HID descriptor
   
    0x09,                       //  bLength; // size 
    DESCRIPTOR_TYPE_HID,       //  bDescriptorType; // 
    0x11, 0x01,               //  bcdHID;  // bcd encoded version that the hid descriptor and device complies too
    HID_COUNTRY_NONE,        //  bCountryCode; // the country code
    0x01,                   //  bNumDescriptor;  // total number of hid report descripotr for the interface 
    DESCRIPTOR_TYPE_HID_REPORT,       //  bDescriptorType0; // first hid report descripotr type
    0x4A , 0x00,       //   wDescriptorLength0; // first hid report descripotr length 

    // usb endpoint desc

    0x07,         // bLength; // descriptor size (0x07)
    USB_DESCRIPTOR_TYPE_ENDPOINT,        // bDescriptorType;  // constant endpoint (0x05)
    (ENDPOINT1 | DIRECTION_IN),        // bEndpointAddress;  // endpoint number and direction
    ENDP_TYPE_INTERRUPT,       // bmAttributes;      // transfer type and supplementary information
    0x03, 0x00,         // wMaxPacketSize;    // maximum packet size suported 
    0x02      // bInterval; 
   
  
};
//////////////////// string descriptor /////////////////////////////////////////////////////////////////////
const uint8_t string_0[] ALIGN =
{
  0x04,                         // descriptor size
  USB_DESCRIPTOR_TYPE_STRING,   // descriptor type string 
  0x09,0x04                    // language id (US english)
};

const uint8_t  string_1[] ALIGN = 
{
    16,   // length of string 
   USB_DESCRIPTOR_TYPE_STRING,    // string descriptor 
    0x48, 0x00,
    0x69, 0x00,              // manufactureer string himanshu 
    0x6d, 0x00,
    0x61, 0x00,
    0x6e, 0x00,
    0x73, 0x00,
    0x68, 0x00,
    0x75, 0x00
};

const uint8_t string_2[] ALIGN = 
{
    46,     // length of string 
    USB_DESCRIPTOR_TYPE_STRING,  // string descriptor
    0x48,0x00,
    0x69,0x00,
    0x6d,0x00,
    0x61,0x00,
    0x6e,0x00,
    0x73,0x00,
    0x68,0x00,
    0x75,0x00,
    0x20,0x00,
    0x6d,0x00,  // himansu mouse tgo beta 
    0x6f,0x00,
    0x75,0x00,
    0x73,0x00,
    0x65,0x00,
    0x20,0x00,
    0x74,0x00,
    0x67,0x00,
    0x6f,0x00,
    0x20,0x00,
    0x62,0x00,
    0x65,0x00,
    0x74,0x00,
    0x61,0x00


};
