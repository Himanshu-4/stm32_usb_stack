#include "usb_framework.h"
#include <stdlib.h>

uint8_t ep_1 = unlock;

uint8_t *outbuff;
uint16_t outsize;

uint8_t *inbuff;
uint16_t insize;

DEVICE_HANDLE device_handle;

uint8_t req_buff[8];
USBrequest *request;

// since we declared it in global scope so by default it is init to zero

// used to when iff app wants to send a stall to the endpoint
uint8_t set_stall = false;

static void usbd_configure(void);
void usbd_init()
{

    // init all
    usb_driver.USB_Msp_init();
    usb_driver.initialize_core();
    usb_driver.connect();

  
}

///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
////////////////     static function start here /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/*********
 *@name sendZLP
 *@param edpoint number
 @note used to send zerol length data packet
 */
static void sendZLP(uint8_t ep_num)
{

    write_packet(ep_num, NULL, 0);
}
//////////////////////////////////////////////////////////////////////////////
/******
 * @name reset the device handle
 * @note used to reset the device handle
 */
static void reset_devicehandle()
{
    insize = 0;
    inbuff = NULL;
    outbuff = NULL;
    outsize = 0;
    device_handle.control_stage = idle;
}

/***************
 * @name reset request stucture
 * @note used to reset the global request structure
 */
static void reset_request_structure()
{
    memset(request, 0, sizeof(USBrequest));
    memset(req_buff, 0, sizeof(req_buff));
}
////////////////////////////////////////////////////////////////////////////
/***********
 * @name transfer data
 * @param void
 * @note used to send data to host
 */
static void transfer_data()
{
    if (insize > MAX_SIZE_EP0) // if the size of data to send is greater than max size of endpoint
    {
        // write data to the buff and also increment the buffer and then again send the data when staus is done
        write_packet(0, inbuff, MAX_SIZE_EP0);

        // after transmission increment the bufferpointer  and decrement the size
        inbuff += MAX_SIZE_EP0;
        insize -= MAX_SIZE_EP0;

        // now there is still data present in the buffer which is pending to send so move the stage to data idle
        device_handle.control_stage = data_in_idle;
    }

    else if (insize == MAX_SIZE_EP0) // if size of data is equal to the size of endpoint
    {
        // write data to the buff and also we have to send a zlp to indicate the end of transaction
        write_packet(0, inbuff, MAX_SIZE_EP0);

        // wait for the status stage or transfer complete interrupt from the usb core
        // send a zero length packet and then wait for status stage
        device_handle.control_stage = data_in_zero;
    }
    else if (insize < MAX_SIZE_EP0) // if size of data is less than the max endpoint size
    {
        // we have to transmit the data to the fifo and wait for the transfer to complete
        write_packet(0, inbuff, insize);

        // wait for the status stage or xfr cmpt interrupt from usb core
        // initiate the status stage
        device_handle.control_stage = status_out;
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
    // activate the endpoint in the current configuration
   configure_in_endpoint(1, interrupt , 4);
//    configure_in_endpoint(2, interrupt , 8);

  sendZLP(1);
//   sendZLP(2);
}

/////////////////////////////////////////////////////////////////////////
/*******
 * @name process get descriptor
 * @param void
 * @note used to process the different descriptor that host need ffrom the device
 */
static void process_get_descriptor()
{
    switch (HIGH_BYTE(request->wValue))
    {
    case USB_DESCRIPTOR_TYPE_DEVICE:
        inbuff = device_descriptor;
        insize = MIN(request->wLength, sizeof(device_descriptor));
        device_handle.control_stage = data_in;

        break;
    case USB_DESCRIPTOR_TYPE_CONFIGURATION:
        // we have to gic=ve the configuration descriptor
        inbuff = configuration_desc_comb;
        insize = MIN(request->wLength, sizeof(configuration_desc_comb));
        device_handle.control_stage = data_in;

        break;

    case USB_DESCRIPTOR_TYPE_STRING:
        // we have to give the string descriptor here
        switch (LOW_BYTE(request->wValue))
        {
        case 0x00:
            printf("string0");
            inbuff = string_0;
            insize = MIN(request->wLength, sizeof(string_0));
            device_handle.control_stage = data_in;
            break;
        case 0x01: // the manufacturer string
            printf("str1");
            inbuff = string_1;
            insize = MIN(request->wLength, sizeof(string_1));
            device_handle.control_stage = data_in;
            break;
        case 0x02: // the product string
            printf("str2");
            inbuff = string_2;
            insize = MIN(request->wLength, sizeof(string_2));
            device_handle.control_stage = data_in;
            break;

        default:
                set_stall = true;
            break;
        }
        break;

    default:
        set_stall = true;
        break;
    }
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
        set_stall = true;

        break;

    ///////////////////////////////////////// usb request clear feature //////////////////////////
    case USB_REQUEST_CLEAR_FEATURE:
        // to do
        break;

    ///////////////////////////////////////////////  usb request set feature //////////////////////
    case USB_REQUEST_SET_FEATURE:
        // to do
        set_stall = true;
        break;

    //////////////////////////////  set address request //////////////////////////
    case USB_REQUEST_SET_ADDRESS:
        usb_driver.set_deviceaddr(LOW_BYTE(request->wValue));
        device_handle.dev_state = addressed;
        device_handle.control_stage = status_in;

        break;

    ////////////////////////////////////////////////////  get descriptor ////////////////////
    case USB_REQUEST_GET_DESCRIPTOR: // 1  /* this is request to get the descriptor now read the wValue to identidfy which descriptor the host need */
        process_get_descriptor();
        /* code */
        break;

    ////////////////////////////////////////// set descriptor //////////////////////////////////
    case USB_REQUEST_SET_DESCRIPTOR:
        // to do
        set_stall = true;
        break;

    //////////////////////////////////////////////// get configuration ///////////////////////////////////////
    case USB_REQUEST_GET_CONFIGURATION:
        // to do
        set_stall = true;
        break;

    ///////////////////////////////////  set configuration request /////////////////////
    case USB_REQUEST_SET_CONFIGURATION:
        device_handle.config_value = LOW_BYTE(request->wValue);
        // implementation specific
        usbd_configure();
        device_handle.dev_state = configured;
        // switch to status state to respond that the operation is perfromed
        device_handle.control_stage = status_in;
        break;

    /////////////////////////////////////////////// get interface request////////////////////
    case USB_REQUEST_GET_INTERFACE:
        // to do
        set_stall = true;
        break;

    //////////////////////////////////////////////////  set interface ////////////////////////////
    case USB_REQUEST_SET_INTERFACE:
        // to do
        set_stall = true;
        break;

    ////////////////////////////////////////////////////// synch frame ////////////////////////////
    case USB_REQUEST_SYNCH_FRAME:
        // to do
        set_stall = true;
        break;

    ////////////////////////////////////////////////////////// set sel ////////////////////////////////////////
    case USB_REQUEST_SET_SEL:
        // to do
        set_stall = true;
        break;

    ////////////////////////////////////////////////////// set isochronus delay//////////////////////////////////
    case USB_REQUEST_SET_ISOCH_DELAY:
        // to do
        set_stall = true;
        break;
    ////////////////////////////////////////////////////
    default:
        set_stall = true;
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************
 * @name process idle request
 * @note process the set idle rate of the endpoint
 */
static void process_idle_0(void)
{
    printf("idle0\r\n");
    // send zero report
//    sendZLP(1);

}

static void process_idle_1(void)
{
    printf("idle1\r\n");
    // sendZLP(2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************
 *
 */
static void process_class_request(void)
{
    // code for HID request

    switch (request->bRequest)
    {
    case HID_SET_IDLE:
        printf("set_isle\r\n");
         switch (LOW_BYTE(request->wIndex))
         {
         case 0: /// mouse report
             process_idle_0();
             break;
         case 1:  ///// keyboard report
            process_idle_1();
            break;
         default:
         set_stall =  true;
             break;
         }
        device_handle.control_stage = status_in;
        break;
    case HID_SET_REPORT:
        printf("set_report\r\n");
        device_handle.control_stage = data_out;
        break;
    default:
        set_stall = true;
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************
 *
 */
static void process_standard_interface(void)
{
    // since this request is a standard request and interface specific of get descriptor (HID report )
    switch (HIGH_BYTE(request->wValue))
    {
    case DESCRIPTOR_TYPE_HID_REPORT:
        //// mouse descriptor
        switch (LOW_BYTE(request->wIndex))
        {
        case 0:
            printf("mouse desc\r\n");
            inbuff = mouse_descriptor;
            insize = MIN(request->wLength, sizeof(mouse_descriptor));
            device_handle.control_stage = data_in;
            break;       
        case 1:
         //// keyboard descriptor
            printf("keyboard desc\r\n");
            inbuff = keyboard_descriptor;
            insize = MIN(request->wLength, sizeof(keyboard_descriptor));
            device_handle.control_stage = data_in;
             break;
        }
        break;
        default:
            set_stall = true;
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
    printf("%d, %d, %d, %d, %d\r\n", request->bmRequesttype, request->bRequest, request->wValue, request->wIndex, request->wLength);

    switch ((request->bmRequesttype) & (USB_BM_REQUEST_TYPE_TYPE_MASK | USB_BM_REQUEST_TYPE_RECIPIENT_MASK))
    {
    ///////////////////////////////////////////   the standard request and the recipient is device ///////////////////
    case (USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIPIENT_DEVICE):
        process_standard_request();
        break;
    case (USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIPIENT_INTERFACE):
        process_standard_interface();
        break;
        ////////////////////////////////////////////  the class specific request and interface recipient //////////////////////////////////////////
    case (USB_BM_REQUEST_TYPE_TYPE_CLASS | USB_BM_REQUEST_TYPE_RECIPIENT_INTERFACE):

        process_class_request();
        break;

    //////////////////////////////////////////// vendor specific request ///////////////////////////////////////////////////
    case (USB_BM_REQUEST_TYPE_TYPE_VENDOR):
        //// to do
        set_stall = true;
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

///////////////////////////////////////// /////////////////////////////////////////////////////////////////////////
/////
void usb_send_report(uint8_t *ptr, uint16_t size)
{
    if (device_handle.dev_state != configured)
        return;
    printf("valep %d\r\n", ep_1);
    if (ep_1 == lock)
        return;
    else
        ep_1 = lock;
    // send the report via the endpoint
    usb_driver.write_packet(1, ptr, size);
    // activate the USB to send report
    actv_remote_signal();
}

////////////   defination of callback function from the driver layer ///////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Implement the driver layer functions here///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void usb_reset_recv_callback(void)
{

    device_handle.dev_state = default_;
    reset_devicehandle();
    reset_request_structure();
    usb_driver.set_deviceaddr(0); // set the default device addr
}

////////////////////////////////////////////////////////////////
/**************
 * @name setup data recieved
 * @param void
 * @note call when setup data recv int is fired
 */
void setup_data_recv_callback(endp_no endpoint_num, uint16_t size)
{
    // now the reception of this interrupt means that some data is arrived int th rx fifo
    usb_driver.read_packet(req_buff /* the buffer in which data is stored */, size /* size of the data recieved */);
    device_handle.control_stage = setup;
    process_request();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****
 * @name setup stage complete
 * @param endp_num
 * @note is asserted when setup stage changes to in/out stage
 */
void setup_stage_cmpt_callback(uint8_t endp_num)
{

    if (set_stall == true)
    {
        printf("stall endp\r\n");
        setstall(endp_num, IN);
        set_stall = false;
        return;
    }
    if (device_handle.control_stage == status_in)
    {
        sendZLP(0);
    }
    else if (device_handle.control_stage == data_in)
    {
        transfer_data();
    }
    else if (device_handle.control_stage == data_out)
    {
        usb_driver.enableep0(request->wLength);
    }
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
void out_data_recv_callback(uint8_t endp_num, uint16_t datalen)
{
    if (device_handle.control_stage == status_out)
    {
        // we are waiting for the host ACK in data stage
        uint8_t var = 0;

        usb_driver.read_packet(&var, datalen);
        device_handle.control_stage = idle;
    }
    else if (device_handle.control_stage == data_in_idle)
    {
        // means te host abonden the transfer so we have to move to status stage
        device_handle.control_stage = idle;
    }
    else if (device_handle.control_stage == data_out)
    {
        // read the data from the RX fifo
        uint8_t set_report =0;
        usb_driver.read_packet(&set_report , datalen);
        device_handle.control_stage = status_in;
        printf("set_report %d\r\n",set_report);
    }
}

void out_Xfer_cmpt_callbcak(uint8_t endp_num)
{

    if (device_handle.control_stage == idle)
    {
        reset_request_structure();
        reset_devicehandle();
        // enable the endpoint to recieve setup packet again
        usb_driver.enableep0(8);
        printf("ctrl read cmpt\r\n");
    }

    else if (device_handle.control_stage == status_in)
    {
        // send the zLP
        sendZLP(0);
    }
    else if (device_handle.control_stage == data_out_idle)
    {
        // switch to data out
        device_handle.control_stage = data_out;
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
    if (num == 0)
    {

        switch (int_val)
        {
        case xfr_cmpt:

            // arms the endpoint to send another data packet
            if (device_handle.control_stage == idle)
            {
            }

            else if (device_handle.control_stage == status_out)
            {
                printf("status out\r\n");
                // enable the out endpoint to recieve ZLP but no setup packet so it is zero
                usb_driver.enableep0(0);
            }
            break;
        case txfe:

            if (device_handle.control_stage == status_in)
            {
                printf("status in\r\n");
                // this mean the ZLP send by the device is transmitted through the core
                device_handle.control_stage = idle;
                reset_devicehandle();
                reset_request_structure();
                printf("write cmpt\r\n");
                // enable the endpoint to recieve setup packet again
                usb_driver.enableep0(8);
            }
            if (device_handle.control_stage == data_in_idle)
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
            break;
        case toc:
            printf("timeout condition occur for control in\r\n");
            break;
        case in_token_txfe:

            printf("in token rx txfifo empty\r\n");
            break;
        case nak:
            printf("nak interrupt\r\n");
            break;
        case inepne:

            printf("core sampled nak\r\n");
            break;
        case epdisd:
            printf("in endp disbl\r\n");
            break;
        }
    }

    else if (num == 1)
    {
        switch (int_val)
        {
        case xfr_cmpt:
            ep_1 = unlock;
            printf("in1cmp\r\n");
            break;
        case txfe:
            printf("1txfifo empty\r\n");
            break;
        case toc:
            printf("1 timeout condition\r\n");
            break;
        case in_token_txfe:

            printf("1 in token rx txfifo empty\r\n");
            break;
        case nak:
            printf("1 nak interrupt\r\n");
            break;
        case inepne:

            printf("1 core sampled nak\r\n");
            break;
        case epdisd:
            printf("1 in endp disbl\r\n");
            break;
        }
    }

}
    void OUT_endpoint_callback(endp_no num, out_endp_int int_val)
    {
        printf("out endp call\r\n");
        // after the generation of the output endpoint the core disable the endpoint
        if (num == 0)
        {
            switch (int_val)
            {
            case _nak:
                /* code */
                printf("nak out");
                break;

            case sts_phs_rx:
                // code
                printf("status phase recieve");

                break;

            case stup:
                // code
                printf("setuphase done");
                break;

            case ep_disd:
                // code
                printf("ep disable as per app request");
                break;

            case xfrc:
                // code
                printf("xfrcmpt");
                break;

            default:
                break;
            
           }
        }
        else
        {
        }
    }

    void error_handler(usb_status usb_Status)
    {
        printf("error %d \r\n", usb_Status);
    }
