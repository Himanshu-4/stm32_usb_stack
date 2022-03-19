#ifndef USB_FRAMEWORK_H_
#define USB_FRAMEWROK_H_

#include "usb_driver.h"
#include "usb_standard.h"
#include <string.h>

#define PACKED __attribute__((__packed__))

#define lock 1
#define unlock 0

extern const uint8_t device_descriptor[18];
extern const uint8_t configuration_desc_comb[59];
extern const uint8_t keyboard_descriptor[45];
extern const uint8_t mouse_descriptor[74];


// string descriptors 
extern const uint8_t string_0[4];
extern const uint8_t string_1[18];
extern const uint8_t string_2[48];

//////////////////////////////////////////////////////////////
/////////////  defination of the macro ///////////////////
#define MIN(a,b) (((a) >= (b))?(b):(a))
#define MAX(a,b) (((a) >= (b))?(a):(b))

/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// decelartion of functions 
 void usbd_init();

 void usb_send_report(uint8_t * , uint16_t ); 
 

extern const USBdriver usb_driver;

#endif
