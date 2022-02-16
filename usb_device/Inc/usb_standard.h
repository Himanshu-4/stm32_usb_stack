#ifndef _USB_STANDARD_H_
#define _USB_STANDARD_H_


// defination of the usb standard type 
#define STM32_VID 0x0483
#define STM32_PID 0x5740

#define HIGH_BYTE(x) ((uint8_t )((0xFF00U & (x))>>8))
#define LOW_BYTE(x) ((uint8_t)(0x00FFU & (x)))  

#define ALIGN __attribute__((aligned(4)))
//////////////////////////////////////////////////////
///////////  the maximum packet size for the default pipe 
#define MAX_SIZE_EP0  (64)
#include <stdint.h>
// some enum defination here 
typedef enum _USB_ENDPOINT_TYPEDEF 
{
control =0,
isochronus =1, 
bulk, 
interrupt
}endpoint_typedef;

// different device state
typedef enum _DEVICE_STATE
{
    powered,
    default_,
    addressed,
    configured,
    suspended

}USBdevicestate;

// control transfer stages 
typedef enum _CONTROL_TRANSFER_STAGES
{
    idle =0,
    setup,   // here the control transfer wait for the setup packet so it is a waiting stage 
    data_out,
    data_out_idle,
    data_in,
    data_in_idle,
    data_in_zero,
    status_out,
    status_in

}control_xfr_stage;

//////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _DEVICE_HANDLE_
{
   uint8_t config_value;
   control_xfr_stage control_stage;
   USBdevicestate dev_state;

}DEVICE_HANDLE;
/////////////////////////////////////////////////////////////////////////
/////////////////////////  defination of USB standard request 
//////////////////////////////////////////////////////////////////////////////
typedef struct  _USB_REQUEST_TYPE 
{
    uint8_t bmRequesttype;  // request type with 
    uint8_t bRequest;        // the actual request 
    uint16_t wValue;         // request specific value 
    uint16_t wIndex;         // request specific value 
    uint16_t wLength;        // number of bytes to transfer

}USBrequest;

//////////////////////////////////////
////////   usbdedvice bit mapped request type field 
#define USB_BM_REQUEST_TYPE_DIRECTION_MASK        (1<<7)
#define USB_BM_REQUEST_TYPE_DIRECTION_TODEVICE   (0<<7)
#define USB_BM_REQUEST_TYPE_DIRECTION__TOHOST    (1<<7)

#define USB_BM_REQUEST_TYPE_TYPE_MASK            (3<<5)
#define USB_BM_REQUEST_TYPE_TYPE_STANDARD        (0<<5)
#define USB_BM_REQUEST_TYPE_TYPE_CLASS           (1<<5)
#define USB_BM_REQUEST_TYPE_TYPE_VENDOR          (2<<5)

#define USB_BM_REQUEST_TYPE_RECIPIENT_MASK       (3<<0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_DEVICE     (0<<0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_INTERFACE  (1<<0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_ENDPOINT   (2<<0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_OTHER      (3<<0)


//////////////////////////////////////////////////////////////////////////////
/////////////////////////   USB standard Request /////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define  USB_REQUEST_GET_STATUS         0x00   // the host request the status of the feature of a device , interface or endpoint 
#define  USB_REQUEST_CLEAR_FEATURE      0x01   //  the host request to disable a feature on a device, endpoint or interface 
#define  USB_REQUEST_SET_FEATURE        0x03   // the host request to enable a feature on the device, endpoint or interface 
#define  USB_REQUEST_SET_ADDRESS        0x05   // host specify an address to use in future communication
#define  USB_REQUEST_GET_DESCRIPTOR     0x06   // host request a specific descriptor 
#define  USB_REQUEST_SET_DESCRIPTOR     0x07   // host add a descriptor or updates an existing descriptor
#define  USB_REQUEST_GET_CONFIGURATION  0x08   // host request the value of the current device configuration
#define  USB_REQUEST_SET_CONFIGURATION  0x09   // host request the device to use the specified configuration
#define  USB_REQUEST_GET_INTERFACE      0x0A   // for interface that have alternative , mutually exclusive setting ,, the host request the currently active interface setting 
#define  USB_REQUEST_SET_INTERFACE      0x0B   // for interface that have alternative , mutually exclusive setting ,, the host request the device to use a specific  interface setting 
#define  USB_REQUEST_SYNCH_FRAME        0x0C   // set and then report an endpoint synchronization frame
#define  USB_REQUEST_SET_SEL            0x30   // for enhanced superspeed devices , set system exit latencies for power management 
#define  USB_REQUEST_SET_ISOCH_DELAY    0x31   // 


//////////////////////////////////////////////////////////////
///////////   name the standard USB descriptor type
//////////////////////////////////////////////////////////
#define USB_DESCRIPTOR_TYPE_DEVICE           0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION    0x02
#define USB_DESCRIPTOR_TYPE_STRING           0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE        0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT         0x05
#define USB_DESCRIPTOR_TYPE_QUALIFIER        0x06
#define USB_DESCRIPTOR_TYPE_OTHER            0x07
#define USB_DESCRIPTOR_TYPE_INTERFACEPOWER   0x08
#define USB_DESCRIPTOR_TYPE_OTG              0x09
#define USB_DESCRIPTOR_TYPE_DEBUG            0x0A
#define USB_DESCRIPTOR_TYPE_INTERFACEASSOC   0x0B
#define USB_DESCRIPTOR_TYPE_CS_INTERFACE     0x24
#define USB_DESCRIPTOR_TYPE_CS_ENDPOINT      0x25

////////////////////////////////////////////////////////////////////////
////////////////////////   name USB class //////////////////////

#define USB_CLASS_PER_INTERFACE  0x00   // class defined on interface level
#define USB_CLASS_AUDIO          0x01   // audio device class
#define USB_CLASSS_COMM_INTERFACE 0x02  // the communication interface
#define USB_CLASS_HID            0x03    // hid class 
#define USB_CLASS_PHYSICAL       0x05   // physical device class
#define USB_CLASS_STILL_IMAGE    0x06   // still imaging device class
#define USB_CLASS_PRINTER        0x07   // printer device class 
#define USB_CLASS_MASS_STORAGE   0x08   // mass storagge device class
#define USB_CLASS_HUB            0x09   // hub 
#define USB_CLASS_DATA_INTERFACE 0x0A   // communication class data interface
#define USB_CLASS_CSCID          0x0B   // smart card 
#define USB_CLASS_CONTENT_SEC    0x0D   // content security 
#define USB_CLASS_VIDEO          0x0E   // video
#define USB_CLASS_HEALTHCARE     0x0F   // personal healthcare
#define USB_CLASS_AV             0x10   // audio/video
#define USB_CLASS_BILLBOARD      0x11   // billboard 
#define USB_CLASS_CBRIDGE        0x12   // type-c bridge device 
#define USB_CLASS_DIAGONOSTIC    0xDC   // diagonostic
#define USB_CLASS_WIRELESS       0xE0   // wireless controller 
#define USB_CLASS_MISC           0xEF   // miscallenous
#define USB_CLASS_IAD            0xEF   // class defined on interface association
#define USB_CLASS_APP_SPEC       0xFE   // application specific
#define USB_CLASS_VENDOR         0xFF   // vendor specific 


///////////////////////////////////////////////////////////
/////////////   name the subclass 
//////////////////////////////////////////////////////////////

#define USB_SUBCLASS_NONE   0x00    // no subclass
#define USB_SUBCLASS_IAD    0x02    // subclass defined on interface association level
#define USB_SUBCLASS_VENDOR 0xFF    // vendor specific subclass

//////////////////////////////////////////////////////////////
/////////////  name the protocol ////////////////////////////

#define USB_PROTOCOL_NONE  0x00   // no protocol
#define USB_PROTOCOL_IAD   0x01  // protocol defined on interface association level 
#define USB_POTOCOL_VENDOR 0xFF  // vendor specific

///////////////////////////////////////////////////////////////////////////////////////////
///////////////   endpoint defination /////////////////////////////////////////////////
#define ENDPOINT1 0x01
#define ENDPOINT2 0x02
#define ENDPOINT3 0x03

#define DIRECTION_IN 0x80
#define DIRECTION_OUT 0x00

#define ENDP_TYPE_INTERRUPT 0x03
#define ENDP_TYPE_BULK      0x02
#define ENDP_TYPE_ISOCH     0x01
#define ENDP_TYPE_CONTROL   0x00

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////   HID specific Request ////////////////////////////////////////

#define HID_GET_REPORT   0x01
#define HID_GET_IDLE     0x02
#define HID_GET_PROTOCOL 0x03
#define HID_SET_REPORT   0x09
#define HID_SET_IDLE     0x0A
#define HID_SET_PROTOCOL 0x0B

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////       Descriptor defination starts here //////////////////
//////////////////////////////////////////////////////////////////////////////////////
/////////////  BCD   means binary coded decimal 
/////////    the device descriptor//////////////////////
typedef struct _USB_DEVICE_DESCRIPTOR_
{
    uint8_t  bLength;    // the size of the descriptor  (0x12)
    uint8_t  bDescriptorType;  // the descriptor type aka constant device (0x01)
    uint16_t bcdUSB;     // USB specification release number (BCD)
    uint8_t  bDeviceClass;  //  USB device class 
    uint8_t  bDeviceSubClass;  // Usb device sub class
    uint8_t  bDeviceProtocol;   // USB protocol code 
    uint8_t  bMaxPacketSize0;   // max packet size for endpoint 0
    uint16_t idVendor;         // vendor Id
    uint16_t idProduct;        // product id 
    uint16_t bcdDevice;       // device release number (BCD)
    uint8_t  iManufacturer;   // index of the string descriptor for the manufacturer
    uint8_t  iProduct;        // index of string descriptor for the product 
    uint8_t  iSerialNumber;   // index of string descriptor for the serial number 
    uint8_t  bNumConfigurations;  // number of possible configurations 
}USBdeviceDescriptor;


///////////////////////////////    Configuration descriptor /////////////////////////////
typedef struct _CONFIGURATION_
{
    uint8_t  bLength;    // the size of the descriptor (0x09)
    uint8_t  bDescriptorType;  // the descriptor type aka constant configuration (0x02)
    uint16_t wTotalLength;     // the number of the bytes in the configuration descriptor and all of its subordinate descriptor 
    uint8_t  bNumInterfaces;   // number of interfaces in the configuration
    uint8_t  bConfigurationValue;  // identifier for set configuration and get configuration request 
    uint8_t  iConfiguration;     //index of string descriptor for the configuration 
    uint8_t  bmAttributes;       // self/bus power and remote wakeup settings
    uint8_t  bMaxPower;         // bus power required in units of 2ma(usb 2.0) and for (USB 3.0) 8ma  

}USBConfiguration;

/////////// these two descriptor are for dual spped devices ///////////
/*
///////////    device qualifier descriptor ///////////////////////////////////////////
typedef struct _DEVICE_QUALIFIER_
{
    uint8_t  bLength;    // the size of the descriptor (0x0A)
    uint8_t  bDescriptorType;  // the descriptor type aka constant device qualifier 0x06
    uint16_t bcdUSB;     // USB specification release number  
    uint8_t  bDeviceClass;  //  USB device class 
    uint8_t  bDeviceSubClass;  // Usb device sub class
    uint8_t  bDeviceProtocol;   // USB protocol code 
    uint8_t  BMaxPacketSize0;   // max packet size for endpoint 0
    uint8_t  bNumConfigurations;  // no of posiible configurations 
    uint8_t  Reserved;             // for future use 

}USBdeviceQualifierdescriptor;

////////////////////////////   other spped configuration /////////////////////////////////
typedef struct _OTHER_SPEED_CONF
{

    uint8_t  bLength;    // the size of the descriptor (0x09)
    uint8_t  bDescriptorType;  // the descriptor type aka other speed  configuration (0x07)
    uint16_t wTotalLength;     // the number of the bytes in the configuration descriptor and all of its subordinate descriptor 
    uint8_t  bNumInterfaces;   // number of interfaces in the configuration
    uint8_t  bConfigurationValue;  // identifier for set configuration and get configuration request 
    uint8_t  iConfiguration;     //index of string descriptor for the configuration 
    uint8_t  bmAttributes;       // self/bus power and remote wakeup settings
    uint8_t  bMaxPower;         // bus power required in units of 2ma(usb 2.0) and for (USB 3.0) 8ma 
}USBother_speedconf;
*/

/////////////////////    interface association descriptor ///////////////////////////////
typedef struct _INTERFACE_ASSOC_
{
    uint8_t  bLength;    // the size of the descriptor (0x08)
    uint8_t  bDescriptorType;  // the descriptor type aka constant interface association (0x0B)
    uint8_t  bFirstInterface;  // number identifying the first interface associated with the function 
    uint8_t  bInterfaceCount;  // the number of contiguous interface associated with the function
    uint8_t  bFunctionClass;   // class code 
    uint8_t  bFunctionSubClass;  // subclass code 
    uint8_t  bFunctionProtocol;  // protocol code 
    uint8_t  iFunction;          // index of string descriptor for the function

}USBInterfaceAssoc;

////////////////////  interface descriptor ////////////////////////////////////////
typedef struct _INTERFACE_DESCRIPTOR_
{
    uint8_t  bLength; // size of descriptor  (0x09)
    uint8_t  bDescriptorType; // the constant interface (0x04)
    uint8_t  bInterfaceNumber; // number identifying this interface 
    uint8_t  bAlternateSetting;  // a number that identifies a descriptor with alternate setting for this interface number
    uint8_t  bNumEndpoints; // no of endpoint supported not counting endpoint 0
    uint8_t  bInterfaceClass;  // class code 
    uint8_t  bInterfaceSubClass; // subclass code 
    uint8_t  bInterfaceProtocol;  //  protocol code 
    uint8_t  iInterface;          // index of string descriptor for the interface 

}USBInterface;

//////////////////   endpoint decsriptor //////////////////////////////////////////////
typedef struct _ENDPOINT_
{
    uint8_t  bLength; // descriptor size (0x07)
    uint8_t  bDescriptorType;  // constant endpoint (0x05)
    uint8_t  bEndpointAddress;  // endpoint number and direction
    uint8_t  bmAttributes;      // transfer type and supplementary information
    uint8_t  wMaxPacketSize;    // maximum packet size suported 
    uint8_t  bInterval;         // service interval or NAK rate 

}USBENDPoint;

/////////////////  string descriptor /////////////////////////////////////////////////////
typedef struct _STRING_
{
    uint8_t  bLength; // descriptor size (variable)
    uint8_t  bDescriptorType;  // constant strting (0x03)
    uint8_t  *string;     // an array of string descriptor 

}USBString;

/////////////////////  binary device object store //////////////////////////////////////////////
typedef struct _BINARY_DEVICE_OBJECT_STORE_
{
    uint8_t  bLength; // descriptor size (0x05)
    uint8_t  bDescriptorType;  // BOS (0x0f)
    uint8_t  wTotalLength;     // the number of bytes in the descriptor and all of its subordinate descriptor
    uint8_t  bNumDeviceCaps;   // the number of device capability descriptor subordinate to this BOS descriptor 

}USB_BOS;


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////  HID CLASS DESCRIPTOR ////////////////////////////////////////////
#define DESCRIPTOR_TYPE_HID  0x21
#define DESCRIPTOR_TYPE_HID_REPORT 0x22
#define HID_COUNTRY_NONE           0x00

typedef struct _USB_HID_DESCRIPTOR_
{
    uint8_t  bLength; // size 
    uint8_t  bDescriptorType; // 
    uint16_t bcdHID;  // bcd encoded version that the hid descriptor and device complies too
    uint8_t  bCountryCode; // the country code
    uint8_t  bNumDescriptor;  // total number of hid report descripotr for the interface 
    uint8_t  bDescriptorType0; // first hid report descripotr type
    uint16_t  wDescriptorLength0; // first hid report descripotr length 
}USBHIDdescriiptor;



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// now define the config , interface , endpoint descritor associatively
typedef struct  _USB_CONFIG_COMBINATION
{
   const USBConfiguration usb_config_descriptor;
   const USBInterface usb_interface_descripotr;
   const USBENDPoint usb_endpoint_descriptor;

    // the hid descripotr type
    const USBHIDdescriiptor usb_hid_descriptor;
   //todo
}USBconfig_combin;


#endif
