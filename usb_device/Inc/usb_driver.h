#ifndef USB_DRIVER_H_
#define USB_DRIVER_H_

#include "stm32f411xe.h"
#include "usb_standard.h"
#include <stdio.h>

#define ENDPOINTCOUNT 4

#ifndef NULL
#define NULL ((void *)0)
#endif

#define true 1
#define false 0


/////////////////////////////////////////////////////////////////////////////
#define USB_OTG_GLOBAL          ((USB_OTG_GlobalTypeDef *) USB_OTG_FS_PERIPH_BASE)

///////////////////////////////////////////////////////////////////////////////////////////
#define USB_OTG_DEVICE          ((USB_OTG_DeviceTypeDef *)( USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
//////////////////////////////////////////////////////
#define USB_OTG_PCGCCTL  (*((__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)))


// in endpoint control and status reg
#define IN_ENDPOINT(i)    ((USB_OTG_INEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE\
                                                       + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))

#define OUT_ENDPOINT(i)   ((USB_OTG_OUTEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE\
                                                        + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))

// function used to get the fifo pop and push  address 


#define FIFO(i) (( uint32_t * )(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE)))

///////////////////////////////////////////
// define some useful macro 
#define _BV(bit)              (0x1U << bit)
#define SET_bit(reg,bit )     ( reg |= bit)
#define CLEAR_bit(reg, bit)   ( reg &= ~(bit))
#define READ_bit(reg,bit)     ( reg & bit )
#define VAL2FLD(field,value)  (((uint32_t)(value) << field ## _Pos ) & field ## _Msk)   // here we are using token pasting operator 
#define FLD2VAL(field,value)  (((uint32_t)(value) & field ) >> field ## _Pos)
#define WRITE_reg(reg,val)    (reg = val)
// define some USB protocols and specification

// some useful definations
#define CLEAR_IT(x)   (USB_OTG_GLOBAL->GINTSTS &= (x) )

#define USB_TIMEOUT 200000U

/////// ///// some enum defination here

//////////////////////
typedef enum _STATUS_
{
    error =1,
    ok,
    timeout,
    halt,
}usb_status;

//////////////////
// size in bytes 
typedef enum _ENDPOINT_SIZE
{
 _8  =  0x3,
 _16 = 0x2,
 _32 = 0x1,
 _64 = 0x0
}endp_size;

/////////////////////////
typedef enum _ENDPOINT_NO_
{
endp0 =0,
endp1 =1,
endp2,
endp3
}endp_no;

//////////////////////////////////////////////////////////////////////////////////
typedef enum _ENDPOINT_TYPE_
{
    IN =1,
    OUT
}eptype;
typedef enum _IN_ENDPOINT_INT_
{
    nak =0,
    xfr_cmpt,
    epdisd ,
    inepne,
    txfe,
    toc,
    in_token_txfe

}in_endp_int;

typedef enum _OUT_ENDPOINT_
{
    _nak =0,
    sts_phs_rx,
    stup,
    ep_disd,
    xfrc

}out_endp_int;
//////////////////////////////////////////////////
//////////   Weak function defination here that user want to implement in the main.c
/////////////////////////////////////////////////////////////////////////////////////////
// initialize the usb pinout for the MCU and enable the irq for the usb
 void  USB_Msp_init();

// deinitialize the usb pins 
 void  USB_Msp_deinit();


////////////////////////////////////////////////////////////////////////////////////
///////////////// these are the callback routines which are called in framework layer ///////////////

 void  setup_data_recv_callback(endp_no endpoint_num , uint16_t size);

 void  usb_reset_recv_callback(void);

 void global_out_nak_callback();
 
 void out_data_recv_callback(uint8_t , uint16_t );

 void out_Xfer_cmpt_callbcak(uint8_t );

 void setup_stage_cmpt_callback(uint8_t );

 void  IN_endpoint_callback(endp_no num , in_endp_int int_val);

 void  OUT_endpoint_callback(endp_no num ,out_endp_int int_val);

 void error_handler(usb_status usb_Status);
////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////    Weak function End here 
/////////////////////////////////////////////////////////////////////////////


// reset the usb engine to its default address 
usb_status reset_handler();

// interrupt handler for the USB core
void gintsts_handler();  // call this function inside the nvic handler for the USB 
//////////////////////////////////////////////////////////////////////////////////
////// used to activate and deactivate the endpoint in different configuration and interface 
usb_status ActivateEP(endp_no , eptype);
//////////////////////////////////////////////////////////////////////////////////
///// used to deactivate the endpoint in certain interfaces and configuration
usb_status DeactivateEP(endp_no , eptype);
//////////////////////////////////////////////////////////////////////////////////
usb_status setstall(endp_no , eptype  );
/////////////////////////////////////////////////////////////////////////////////////////////
usb_status clearstall(endp_no , eptype );
/////////////////////////////////////////////////////////////////////////////////////////////////////
// configuring the endpoint 0 
void configure_endpoint0(endp_size  size);

// configures the in endpoint for the device 
void configure_in_endpoint(endp_no endpoint_number, endpoint_typedef usb_endpoint_type, uint16_t endpoint_size);

// configure the out endpoint for the device
void configure_out_endpoint(endp_no endpoint_num, endpoint_typedef usb_endpoint_type , uint16_t endpoint_size);

// deconfigure the in endpoint 
void deconfigure_in_endpoint(endp_no endpoint_number);

//deconfigure the out endpoint
void deconfigure_out_endpoint(endp_no endpoint_number); 

// structure for using the functions in other files

typedef struct _USB_DRIVER
{   
    //init the core 
    usb_status (*initialize_core)(void);
    // Msp init 
    void (* USB_Msp_init)(void);
    // Msp deinit
    void (* USB_Msp_deinit)(void);
    // connect and disconnect 
    void (* connect)(void);
    // disconnect 
    void (* disconnect)(void);
    // configure endpoint 0 
    void (* configure_endpoint0)(endp_size size);
    // configure the in endpoint
    void (* configure_in_endpoint)(endp_no endpoint_number, endpoint_typedef usb_endpoint_type, uint16_t endpoint_size);
    // deconfigure the in endpoint 
    void (* deconfigure_in_endpoint)(endp_no endpoint_number);
    //deconfigure the out endpoint
    void (* deconfigure_out_endpoint)(endp_no endpoint_number); 
    // flush the tx fifo 
    usb_status (* flush_txfifo)(endp_no num);
    // flush the rx fifo 
    usb_status (* flush_rxfifo)(void);
    // read the incoming packet 
    void (*read_packet)(uint8_t *, uint16_t );
    // write the packet 
    void (* write_packet)(endp_no num, uint8_t const *buffer , uint16_t size);
    // set the device address 
    void (* set_deviceaddr)(uint8_t addr);
}USBdriver;

////////////////////////////////////////////////////////////////
////////////////////////////////      inclusion of extern function here
//////////////////////////////////////////////////////////////////////


#endif