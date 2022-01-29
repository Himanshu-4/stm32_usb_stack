#ifndef USB_FRAMEWORK_H_
#define USB_FRAMEWROK_H_

#include "usb_driver.h"
#include "usb_standard.h"
#include <string.h>

extern const  uint8_t device_descriptor[18];
extern const  uint8_t hid_report_descriptor[50];
extern const  uint8_t configuration_desc_comb[34];


//////////////////////////////////////////////////////////////
/////////////  defination of the macro ///////////////////
#define MIN(a,b) (((a) >= (b))?(b):(a))
#define MAX(a,b) (((a) >= (b))?(a):(b))

/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// decelartion of functions 
 void usbd_init();

 void usb_send_report(uint8_t * ptr, uint32_t size);

extern const USBdriver usb_driver;

#endif
