#include "usb_framework.h"
#include <stdlib.h>

uint8_t *outbuff;
uint16_t outsize;

uint8_t *inbuff;
uint16_t insize;

DEVICE_HANDLE device_handle;

uint8_t req_buff[8];
USBrequest *request;


// used to when iff app wants to send a stall to the endpoint 
 uint8_t send_stall = false;

 // used to figure out if control read transfer is sucess or not
 uint8_t control_read_cmpt = false;


 void usbd_init()
{
    
    // init all 
    usb_driver.USB_Msp_init();
    usb_driver.initialize_core();
    usb_driver.connect();
    
}



////////////////////////////////////////////////////////////////////////////////////////
////////////////     static function start here /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/*********
 * 
 */
static void sendZLP(uint8_t ep_num)
{
    printf("sending ZLP\r\n");
    usb_driver.write_packet(ep_num, NULL , 0);
}
//////////////////////////////////////////////////////////////////////////////
/******
 * @name reset the device handle 
 * @note used to reset the device handle 
 */
static void reset_devicehandle()
{
    printf("resetting device_handle\r\n");
     insize =0;
     inbuff = NULL;
     outbuff = NULL;
     outsize =0;
     device_handle.control_stage =idle;
}

/***************
 * @name reset request stucture 
 * @note used to reset the global request structure 
 */
static void reset_request_structure()
{
    printf("resetting request structure\r\n");
     memset(request ,0 ,sizeof(request));
     memset(req_buff , 0, sizeof(req_buff));
}
////////////////////////////////////////////////////////////////////////////
/***********
 * @name transfer data
 * @param void
 * @note used to send data to host
 */
static void transfer_data()
{
    if(device_handle.control_stage == data_in)
    {
       printf("inside transfer data\r\n");
    if(insize > MAX_SIZE_EP0)  // if the size of data to send is greater than max size of endpoint 
    {
       // write data to the buff and also increment the buffer and then again send the data when staus is done 
         usb_driver.write_packet(0, inbuff ,  insize);
        
        // after transmission increment the bufferpointer  and decrement the size 
        inbuff += MAX_SIZE_EP0;
        insize -= MAX_SIZE_EP0;

        // now there is still data present in the buffer which is pending to send so move the stage to data idle
        device_handle.control_stage = data_in_idle; 
    }

    else if(insize == MAX_SIZE_EP0 ) // if size of data is equal to the size of endpoint 
    {
        // write data to the buff and also we have to send a zlp to indicate the end of transaction
         usb_driver.write_packet(0, inbuff , insize);

        // after transmission remove the data from the buffer 
        inbuff = NULL;
        insize = 0;

        // wait for the status stage or transfer complete interrupt from the usb core
        // send a zero length packet and then wait for status stage 
        device_handle.control_stage = data_in_zero;
    }
    else if (insize < MAX_SIZE_EP0 ) // if size of data is less than the max endpoint size
    {
        // we have to transmit the data to the fifo and wait for the transfer to complete 
         usb_driver.write_packet(0, inbuff , insize);

         // after transmission remove the data from the buff
         insize =0;
         inbuff =NULL;
         
         // wait for the status stage or xfr cmpt interrupt from usb core
         // initiate the status stage 
         device_handle.control_stage = status_out;
    }
}
}

//////////////////////////////////////////////////////////////////////////////
/************
 * @name usbd_configure
 * @param 
 * @note used to configure the usb device 
 */
static void usbd_configure()
{

}


/////////////////////////////////////////////////////////////////////////
/*******
 * @name process get descriptor
 * @param void 
 * @note used to process the different descriptor that host need ffrom the device 
 */
static inline void process_get_descriptor()
{
    switch (((uint8_t)(request->wValue >>8)))
    {
    case  USB_DESCRIPTOR_TYPE_DEVICE:
        /* code */
        printf("device descriptor\r\n");
        inbuff = &device_descriptor;
        insize = MIN(request->wLength , sizeof(device_descriptor));
        device_handle.control_stage = data_in;
     
        break;
    case USB_DESCRIPTOR_TYPE_CONFIGURATION:
     printf("confgiuration descriptor \r\n");
    // we have to gic=ve the configuration descriptor
    inbuff = &configuration_desc_comb;
    insize = MIN(request->wLength, sizeof(configuration_desc_comb));
    device_handle.control_stage = data_in;
     
        break;
    
    default:
        break;
    }
  
  printf("value of controlxferstage %d \r\n", device_handle.control_stage);
    transfer_data();
}
////////////////////////////////////////////////////////////////////////////
/*********
 * @name process standard request 
 * @param void 
 * @note used for processing the standard request from the host 
 */
static void process_standard_request()
{

    // use a switch case statement for the jumping through the request 
    switch (request->bRequest)
    {
    //////////////////////////////  get status request ////////////////////////////////////
     case USB_REQUEST_GET_STATUS:
            // to do
            // stall the unsupported request
            send_stall = true;
        break;

    ///////////////////////////////////////// usb request clear feature //////////////////////////
    case USB_REQUEST_CLEAR_FEATURE:
         // to do
        break;

    ///////////////////////////////////////////////  usb request set feature //////////////////////
    case USB_REQUEST_SET_FEATURE:
        // to do
        send_stall =true ;
        break;

    //////////////////////////////  set address request //////////////////////////
    case USB_REQUEST_SET_ADDRESS :
       printf("set addr req \r\n");
       usb_driver.set_deviceaddr((request->wValue & 0xff));
        
        device_handle.dev_state =addressed;
        device_handle.control_stage = status_in;

        break;

    ////////////////////////////////////////////////////  get descriptor ////////////////////  
    case USB_REQUEST_GET_DESCRIPTOR:  //1  /* this is request to get the descriptor now read the wValue to identidfy which descriptor the host need */
        printf("req for get desc\r\n");
        process_get_descriptor();
        /* code */
        break;
    
    ////////////////////////////////////////// set descriptor //////////////////////////////////
    case USB_REQUEST_SET_DESCRIPTOR:
         // to do
         send_stall = true;
         break;

    //////////////////////////////////////////////// get configuration ///////////////////////////////////////
    case USB_REQUEST_GET_CONFIGURATION:
        // to do
        break;

    ///////////////////////////////////  set configuration request /////////////////////
    case USB_REQUEST_SET_CONFIGURATION:
       printf("set configuration request\r\n");     
       device_handle.config_value = request->wValue;
        // implementation specific 
        usbd_configure();
        
        device_handle.dev_state = configured;
        // switch to status state to respond that the operation is perfromed 
        device_handle.control_stage = status_in;
        break;
    
    /////////////////////////////////////////////// get interface request////////////////////
    case USB_REQUEST_GET_INTERFACE:
        // to do
        break;
    
    //////////////////////////////////////////////////  set interface ////////////////////////////
    case USB_REQUEST_SET_INTERFACE :
        // to do
        break;

    ////////////////////////////////////////////////////// synch frame ////////////////////////////
    case USB_REQUEST_SYNCH_FRAME:
        // to do
        send_stall = true;
        break;

    ////////////////////////////////////////////////////////// set sel ////////////////////////////////////////
    case USB_REQUEST_SET_SEL:
        // to do 
        send_stall = true;
        break;

    ////////////////////////////////////////////////////// set isochronus delay//////////////////////////////////
    case USB_REQUEST_SET_ISOCH_DELAY:
        // to do
        send_stall = true;
        break;
    //////////////////////////////////////////////////// 
    default:
       send_stall = true;
        break;
    }
    
}
//////////////////////////////////////////////////////////////////////////
/*******
 * @name process reuest
 * @note processs the standard usb input request 
 */
static void process_request()
{
    // cast the pointer of USBrequest type to our input bufferr
   request = req_buff;
        printf(" the reqtype is %d , req  is %d , wvalue %d, windex %d , wlength %d \r\n",request->bmRequesttype ,request->bRequest ,request->wValue , request->wIndex , request->wLength );
        switch ((request->bmRequesttype)  &  USB_BM_REQUEST_TYPE_TYPE_MASK )
        {
            ///////////////////////////////////////////   the standard request and the recipient is device ///////////////////
            case (USB_BM_REQUEST_TYPE_TYPE_STANDARD ):
            printf("into standard request\r\n");
                process_standard_request();
                break;

            ////////////////////////////////////////////  the class specific request //////////////////////////////////////////
            case (USB_BM_REQUEST_TYPE_TYPE_CLASS ):
                /// to do 
                break;

            //////////////////////////////////////////// vendor specific request ///////////////////////////////////////////////////
            case (USB_BM_REQUEST_TYPE_TYPE_VENDOR):
                //// to do 
                send_stall = true;
                break;        
        
        }
        
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////  static function end here //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////   defination of callback function from the driver layer ///////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Implement the driver layer functions here///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void usb_reset_recv_callback(void)
{
    printf("usb reset rtecv callback\r\n");
    device_handle.dev_state = default_;
    reset_devicehandle();
    reset_request_structure();
    usb_driver.set_deviceaddr(0);   // set the default device addr
}

////////////////////////////////////////////////////////////////
/**************
 * @name setup data recieved 
 * @param void 
 * @note call when setup data recv int is fired  
 */
void setup_data_recv_callback(endp_no endpoint_num , uint16_t size)
{ 
    printf(" enter setup data recv callback size is %d \r\n", size);
    // now the reception of this interrupt means that some data is arrived int th rx fifo
    usb_driver.read_packet(req_buff/* the buffer in which data is stored */, size/* size of the data recieved */);
    device_handle.control_stage = setup;
    process_request();
    
}

/******
 * @name global nak callback
 * @note calls when the global out nak bit is set 
 * 
 */
void global_out_nak_callback()
{


}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******
 * @name out data recv callback
 * @param endpoint num and data len
 * @note calls when out data is recieved 
 */
void out_data_recv_callback(uint8_t endp_num , uint16_t datalen)
{
    if(device_handle.control_stage == status_out )
    {
        // we are waiting for the host ACK in data stage
        uint8_t var;

        usb_driver.read_packet(&var, datalen);
        if(var == 0)
        {
            // HOST give a ACK 
            control_read_cmpt =true;

        } 
        else 
        {
            control_read_cmpt = false;
        }
        printf(" the value of status is %d", var);
        device_handle.control_stage = idle;
    }
    else if (device_handle.control_stage == data_in_idle)
    {
        // means te host abonden the transfer so we have to move to status stage 
        device_handle.control_stage = idle;
    }
    else if(device_handle.control_stage == data_out)
    {
        // read the data from the RX fifo
        
    }
}

void out_Xfer_cmpt_callbcak(uint8_t endp_num)
{

    if(device_handle.control_stage == idle )
    {
        if(control_read_cmpt == true)
        {
            printf(" the control read request is completed");
            // that means the host send a ZLP and the control read transfer was completes succesfullt
            // now we have to reset the request structure and the device_handle
            reset_request_structure();
            reset_devicehandle();
          control_read_cmpt = false;
        }

    }

    else if(device_handle.control_stage == status_in)
    {
       // send the zLP 


    }
    else if (device_handle.control_stage == data_out_idle)
    {
        // switch to data out
        device_handle.control_stage = data_out;
    }
}

void setup_stage_cmpt_callback(uint8_t endp_num)
{
    printf("the setup stage is completed\r\n");
    // if the request is not the supported request then send a stall
    if (endp_num != endp0)return;

        if(send_stall == true)
        {
            setstall(endp0, IN);
            printf("the endpoint 0 is stalled");
            send_stall  = false;
            return;
        }

    

    if (device_handle.control_stage == data_out)
    {
        // do for data out transfer
    }
    else if (device_handle.control_stage == status_in)
    {
        sendZLP(0);
     
    }
}
 
/////////////////////////////////////////////////////////////////////////////
/***********
 * @name in endpoint transfer complete stage
 * @param endpoint number 
 * @param endpoint int type
 * @note call when an interrupt is fired on in endpoint 
 */
void IN_endpoint_callback(endp_no num, in_endp_int int_val)
{
    if(num ==0)
    {
    // to do implement
    printf("in endpoint callbcak\r\n");

    if(int_val == xfr_cmpt)
    {
     printf("in transfer is completed\r\n");
        // arms the endpoint to send another data packet 
       if(device_handle.control_stage == idle)
       {
           reset_devicehandle();
           reset_request_structure();
          
       } 
       else if(device_handle.control_stage == 0)
       {
          // all the data is being transmitted by the device mpve to status out stage 
        device_handle.control_stage = status_out;

    
       }
       else 
       {
           // there is still data left in the buffer transfer it
        device_handle.control_stage = data_in_idle;
               

       }
    }
    else if (int_val == txfe)
    {
        printf("tx fifo is empty\r\n");
         // put another data in the the txfifo 
        if(device_handle.control_stage == data_in_idle)
        {
            printf("txfifoempty data_in_idle\r\n");
            device_handle.control_stage = data_in;
            transfer_data();
        }
        else if (device_handle.control_stage == data_in_zero)
        {
            printf("txfifoempty data_in_zero\r\n");
            sendZLP(0);
            device_handle.control_stage = status_out;
        }
       else if(device_handle.control_stage == status_in)
        {
            printf("txfifoempty status_in\r\n");
                // this mean the ZLP send by the device is transmitted through the core
                device_handle.control_stage = idle;
        }
    }
    else if(int_val == toc)
    {
        printf("timeout condition occur for control in\r\n");
    }
    else if(int_val == in_token_txfe)
    {
        printf("in token rx txfifo empty\r\n");
    }
    else if(int_val == nak)
    {
        printf("nak interrupt\r\n");
    }
    else if (int_val == inepne)
    {
        printf("core sampled nak\r\n");
    }
}

}


void  OUT_endpoint_callback(endp_no num ,out_endp_int int_val)
{


}