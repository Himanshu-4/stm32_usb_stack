#ifndef USB_FRAMEWORK_H_
#define USB_FRAMEWROK_H_

#include "usb_driver.h"
#include "usb_standard.h"
#include <string.h>

#define PACKED __attribute__((__packed__))

#define lock 0
#define unlock 1

extern const  uint8_t device_descriptor[18];
extern const  uint8_t hid_report_descriptor[50];
extern const  uint8_t configuration_desc_comb[34];

// string descriptors 
extern const unsigned char string_0[4];
extern const unsigned char string_1[18];
extern const unsigned char string_2[48];

//////////////////////////////////////////////////////////////
/////////////  defination of the macro ///////////////////
#define MIN(a,b) (((a) >= (b))?(b):(a))
#define MAX(a,b) (((a) >= (b))?(a):(b))

/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// decelartion of functions 
 void usbd_init();

 void usb_send_report(uint8_t * , uint16_t ); 
 
 void init_endp(void);
extern const USBdriver usb_driver;

#endif
