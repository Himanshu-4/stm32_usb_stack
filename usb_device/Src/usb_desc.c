#include "usb_standard.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////  defination of descripotr here /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define PACKED __attribute__((__packed__))
///////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t device_descriptor[] ALIGN  =
{
    0x12,             //  bLength;    // the size of the descriptor  (0x12)
    USB_DESCRIPTOR_TYPE_DEVICE,            //  bDescriptorType;  // the descriptor type aka constant device (0x01)
    0x02, 0x00,      //  bcdUSB;     // USB specification release number (BCD)
    0x00,            //  bDeviceClass;  //  USB device class 
    0x00,            //  bDeviceSubClass;  // Usb device sub class
    0x00,            //  bDeviceProtocol;   // USB protocol code 
    MAX_SIZE_EP0,    //  bMaxPacketSize0;   // max packet size for endpoint 0
    LOW_BYTE(STM32_VID),HIGH_BYTE(STM32_VID),              //  idVendor;         // vendor Id
    LOW_BYTE(STM32_PID), HIGH_BYTE(STM32_PID),                //  idProduct;        // product id 
    0x02, 0x00,               //  bcdDevice;       // device release number (BCD)
    0x00,                //  iManufacturer;   // index of the string descriptor for the manufacturer
    0x00,                //  iProduct;        // index of string descriptor for the product 
    0x00,               //  iSerialNumber;   // index of string descriptor for the serial number 
    0x01           //  bNumConfigurations;  // number o
  
};



///////////////////////////////////////////////////////////////////////////////////
/////////////////// define the hid report descriptor/////////////////////////////////////////
const uint8_t hid_report_descriptor[] =
{
0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
0x09, 0x02,                    // USAGE (Mouse)
0xa1, 0x01,                    // COLLECTION (Application)
0x09, 0x01,                    //   USAGE (Pointer)
0xa1, 0x00,                    //   COLLECTION (Physical)
0x05, 0x09,                    //     USAGE_PAGE (Button)
0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
0x95, 0x03,                    //     REPORT_COUNT (3)
0x75, 0x01,                    //     REPORT_SIZE (1)
0x81, 0x02,                    //     INPUT (Data,Var,Abs)
0x95, 0x01,                    //     REPORT_COUNT (1)
0x75, 0x05,                    //     REPORT_SIZE (5)
0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
0x09, 0x30,                    //     USAGE (X)
0x09, 0x31,                    //     USAGE (Y)
0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
0x75, 0x08,                    //     REPORT_SIZE (8)
0x95, 0x02,                    //     REPORT_COUNT (2)
0x81, 0x06,                    //     INPUT (Data,Var,Rel)
0xc0,                          //   END_COLLECTION
0xc0   
};



const uint8_t configuration_desc_comb[] =
{ 
    // configuration desc

    0x09,     // bLength;    // the size of the descriptor (0x09)
    USB_DESCRIPTOR_TYPE_CONFIGURATION,     // bDescriptorType;  // the descriptor type aka constant configuration (0x02)
    0x00, 0x33, // wTotalLength;     // the number of the bytes in the configuration descriptor and all of its subordinate descriptor 
    0x01,     // bNumInterfaces;   // number of interfaces in the configuration
    0x01,    // bConfigurationValue;  // identifier for set configuration and get configuration request 
    0x00,    // iConfiguration;     //index of string descriptor for the configuration 
    0x80,   // bmAttributes;       // self/bus power and remote wakeup settings
    0x64,   // bMaxPower;         // bus power required in units of 2ma(usb 2.0) and for (USB 3.0) 8ma  

    //interface descriptor
    
    0x09,                              //   bLength; // size of descriptor  (0x09)
    USB_DESCRIPTOR_TYPE_INTERFACE,       //   bDescriptorType; // the constant interface (0x04)
    0x01,        //   bInterfaceNumber; // number identifying this interface 
    0x00,        //   bAlternateSetting;  // a number that identifies a descriptor with alternate setting for this interface number
    0x01,        //   bNumEndpoints; only one endp
    USB_CLASS_HID,     //   bInterfaceClass;  // hid class  
    0x01,       //   bInterfaceSubClass; // boot interface  
    0x02,  //   bInterfaceProtocol;  //  mouse  
    0x00,     //   iInterface;          // index of string descriptor for the interface 

    // usb endpoint desc

    0x07,         // bLength; // descriptor size (0x07)
    USB_DESCRIPTOR_TYPE_ENDPOINT,        // bDescriptorType;  // constant endpoint (0x05)
    (ENDPOINT1 | DIRECTION_IN),        // bEndpointAddress;  // endpoint number and direction
    ENDP_TYPE_INTERRUPT,       // bmAttributes;      // transfer type and supplementary information
    0x00, 0x40,         // wMaxPacketSize;    // maximum packet size suported 
    0x32,        // bInterval; 
   
   /// HID descriptor
   
    0x09,                       //  bLength; // size 
    DESCRIPTOR_TYPE_HID,       //  bDescriptorType; // 
    0x01, 0x00,               //  bcdHID;  // bcd encoded version that the hid descriptor and device complies too
    HID_COUNTRY_NONE,        //  bCountryCode; // the country code
    0x01,                   //  bNumDescriptor;  // total number of hid report descripotr for the interface 
    DESCRIPTOR_TYPE_HID_REPORT,       //  bDescriptorType0; // first hid report descripotr type
    0x00 , 0x32       //   wDescriptorLength0; // first hid report descripotr length 

};

typedef struct mouse_report_t
{
    uint8_t buttons;
    int8_t x;
    int8_t y;
}HID_report;