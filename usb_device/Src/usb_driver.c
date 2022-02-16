#include "usb_driver.h"

///////////////////static function start here //////////////////////

/********************
 * @name flush the rxfifo
 */
static usb_status flush_rxfifo()
{

   __IO uint32_t count = 0U;
   SET_bit(USB_OTG_GLOBAL->GRSTCTL, USB_OTG_GRSTCTL_RXFFLSH);
   do
   {
      if (++count > 200000U)
      {
         return timeout;
      }
   } while (READ_bit(USB_OTG_GLOBAL->GRSTCTL, USB_OTG_GRSTCTL_TXFFLSH));

   return ok;
}

/*****************
 * @name flush tx fifo
 */
static usb_status flush_txfifo(endp_no num)
{

   // set the tx fifo to be flushed and then set the flush

   __IO uint32_t count = 0U;

   SET_bit(USB_OTG_GLOBAL->GRSTCTL, VAL2FLD(USB_OTG_GRSTCTL_TXFNUM, num) | USB_OTG_GRSTCTL_TXFFLSH);

   do
   {
      if (++count > 200000U)
      {
         return timeout;
      }
   } while (READ_bit(USB_OTG_GLOBAL->GRSTCTL, USB_OTG_GRSTCTL_TXFFLSH));

   return ok;
}

//////////////////////////////////////////////////
/************************
 * @name refresh fifo start address
 * @brief call this function after configure the fifo size
 *
 */
static void refresh_FIFO_start_address()
{
   // the first changeable address begins after the region of rxfifo
   // first take the depth of Rx fifo
   uint16_t start_addr = (USB_OTG_GLOBAL->GRXFSIZ & 0xFFFF) * 4;

   // update the start address of txfifo 0
   SET_bit(USB_OTG_GLOBAL->DIEPTXF0_HNPTXFSIZ, start_addr);

   // the next start address is where the last tx fifo  ends
   start_addr += ((USB_OTG_GLOBAL->DIEPTXF0_HNPTXFSIZ >> 16) * 4);

   // update the tx fifo for the rest of the endpoint

   for (uint8_t endp_num = 0; endp_num < ENDPOINTCOUNT - 1; endp_num++)
   {
      SET_bit(USB_OTG_GLOBAL->DIEPTXF[endp_num], start_addr);

      // update the start addressof the new fifo
      start_addr += ((USB_OTG_GLOBAL->DIEPTXF[endp_num] >> 16) * 4);
   }
}

/////////////////////////////////////////////////////
/*****************
 * @name configure rx fifo
 * @param size the size of the max out endpoint
 * @note this fifo is shared between all of the out endpoint
 */
static void configure_rx_fifo(uint16_t size)
{
   // consider the space required to save status packet in rxfifo and get space in words
   size = 10 + (2 * ((size / 4) + 1));

   // first clear the size then set it
   CLEAR_bit(USB_OTG_GLOBAL->GRXFSIZ, USB_OTG_GRXFSIZ_RXFD);
   // configure the depth of the fifo
   SET_bit(USB_OTG_GLOBAL->GRXFSIZ, VAL2FLD(USB_OTG_GRXFSIZ_RXFD, size));

   // update the new start addresses
   refresh_FIFO_start_address();
}

/////////////////////////////////////////////
/**************
 * @name configure tx fifo
 * @param endp_no
 * @param size
 * @note configure the tx fifo size
 */
static void configure_tx_fifo(endp_no num, uint16_t size)
{
   // get the FIFO size in 32 but words
   size = (size + 3) / 4;

   if (num == 0)
   {
      // first clear the size then set it
      WRITE_reg(USB_OTG_GLOBAL->DIEPTXF0_HNPTXFSIZ, 0);
      SET_bit(USB_OTG_GLOBAL->DIEPTXF0_HNPTXFSIZ, VAL2FLD(USB_OTG_TX0FD, size));
   }

   else
   {
      // first clear the size then set it
      WRITE_reg(USB_OTG_GLOBAL->DIEPTXF[num], 0);
      SET_bit(USB_OTG_GLOBAL->DIEPTXF[num], VAL2FLD(USB_OTG_DIEPTXF_INEPTXFD, size));
   }
   // update the new start addresses
   refresh_FIFO_start_address();
}

///////////////////////////////////////////////////////////////////////
/*******
 * @name enumuration done interrupt
 * @note calls when enumuretion done irq fires
 */
static void enumdone_handler()
{
   // we have to confgiures the endpoint 0 for recieving the setup packet
   // you can hardcore the endpoint 0 size but it may not be the case you can use 64 max value here but for low spped use only 8
   configure_endpoint0(_64);
}

//////////////////////////////////////////////////////////////////////////////////
/***********
 * @name enable endp0
 * @param void
 * @note used to enable endppoint 0 after a transfer
 */
static void enableep0( uint16_t size)
{
   if(READ_bit(OUT_ENDPOINT(0)->DOEPTSIZ , USB_OTG_DOEPTSIZ_XFRSIZ )){printf("error enabling out enp\r\n");}
    SET_bit( OUT_ENDPOINT(0)->DOEPTSIZ , USB_OTG_DOEPTSIZ_STUPCNT_0 | USB_OTG_DOEPTSIZ_PKTCNT | VAL2FLD(USB_OTG_DOEPTSIZ_XFRSIZ , size) );
    SET_bit(OUT_ENDPOINT(0)->DOEPCTL, USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
}
/////////////////////////////////////////////////////////////////////////////
/*********
 * @name recieve fifo not empty
 * @param void
 * @note if some data is arrived this is the handler fun for the recieve fifo
 */
static void RXflvl_handler()
{
   // first get the status information from the GRXSTSP reg
   // only read the reg at once because it also pop the fifo once the read is complete
   __IO uint32_t status = USB_OTG_GLOBAL->GRXSTSP;

   // the status is then split into some field
   // the endpoint which recieved the data
   uint8_t ep_num = FLD2VAL(USB_OTG_GRXSTSP_EPNUM, status);

   // the packet count of the data
   uint16_t size = FLD2VAL(USB_OTG_GRXSTSP_BCNT, status);

   // the packet status rt544
   uint8_t pktsts = FLD2VAL(USB_OTG_GRXSTSP_PKTSTS, status);

   // the frame number
   uint8_t frame_no = FLD2VAL(USB_OTG_GRXSTSP_DPID, status);

   // the data pid
   uint8_t dpid = FLD2VAL(USB_OTG_GRXSTSP_DPID, status);

   // now based on the pktsts we have to call some func
   // m now after the transaction is finshed the core disabled that endpoint so (0x03) and (0x04) we have to reenable the endpoint

   switch (pktsts)
   {
   case 0x01: // Global out NAK

      global_out_nak_callback();
      break;
   //////////////////////  this indicate that an OUT transaction is complete i.e. only one transaction
   case 0x02: // out data packet recieved (include data)

      // contains the data for the out endpoint
      out_data_recv_callback(ep_num, size);

      break;
   ////////////  out transfer completed (triggers an interrupt)
   case 0x03: // out transfer completed
      out_Xfer_cmpt_callbcak(ep_num);

      break;
   /////////////////////  setup transaction complete (triggeras an interrupt)
   case 0x04: // setup transaction complete ()
      setup_stage_cmpt_callback(ep_num);

      break;
   ///////////////////////
   case 0x06:                                 // setup data packet recieved (include data)
      setup_data_recv_callback(ep_num, size); // implement in fraework layer
      break;
   /////////////////////////
   default: // default handler
      break;
   }
}

////////////////////////////////////////////
/*************************
 * @name to read the packet from the rxfifo
 * @param pointer to a buffer
 * @param size of the buffer
 */
static void read_packet(uint8_t *buffer, uint16_t size)
{
   // first to get the pop addr there is only one rx fifio for all the endpoints
  __IO uint32_t * __IO fifo = FIFO(0);
   for (; size >= 4; size -= 4, buffer += 4)
   {
      // pops one 32 bit word of data
      *((uint32_t *)buffer) = *fifo;
   }

   if (size > 0)
   {
      // pops the last remaining byte
      uint32_t data = *fifo;
      for (; size > 0; size--, data >>= 8)
      {
         *((uint8_t *)buffer++) = ((uint8_t)(0xff & data));
      }
   }
}

/////////////////////////////////////////////////
/**********
 * @name func to write packet to the fifo
 * @param endpoint number
 * @param pointer to bufer
 * @param size of the pointer
 *
 */
static void write_packet(uint8_t num,  const void *buffer, uint16_t size)
{

   // set the size to the ITXFSIZ , packet count and Xfersize
   CLEAR_bit(IN_ENDPOINT(num)->DIEPTSIZ, USB_OTG_DIEPTSIZ_PKTCNT | USB_OTG_DIEPTSIZ_XFRSIZ);
   SET_bit(IN_ENDPOINT(num)->DIEPTSIZ, VAL2FLD(USB_OTG_DIEPTSIZ_PKTCNT, 1) | VAL2FLD(USB_OTG_DIEPTSIZ_XFRSIZ, size));
   CLEAR_bit( IN_ENDPOINT(num)->DIEPCTL , USB_OTG_DIEPCTL_STALL );
   // after writing data to the FIFO clear nak , activate the endpoint
   SET_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK);

    __IO uint32_t * __IO fifo = FIFO(num);

   // get the size in 32 bit word
   size = (size + 3) / 4;

   for (; size > 0; size--, buffer += 4)
   {
      // push the data to the tx fifo
            *fifo = *((uint32_t *)buffer);
   }
 
   // enable the in endpoint tx fifo empty mask
   SET_bit(USB_OTG_DEVICE->DIEPEMPMSK, _BV(num));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static void reset_fifo()
{
   // reset the RX FIFO
   WRITE_reg(USB_OTG_GLOBAL->GRXFSIZ, 0);
   WRITE_reg(USB_OTG_GLOBAL->DIEPTXF0_HNPTXFSIZ, 0);
   for (uint8_t i = 1; i < ENDPOINTCOUNT; i++)
   {
      WRITE_reg(USB_OTG_GLOBAL->DIEPTXF[i], 0);
   }
}
//////////////////////////////////////////////////////////////////////////////
/***************
 * @name set device addr
 * @param address of device
 *
 */
static void set_deviceaddr(uint8_t addr)
{
   // first clear the device addr
   CLEAR_bit(USB_OTG_DEVICE->DCFG, USB_OTG_DCFG_DAD);
   SET_bit(USB_OTG_DEVICE->DCFG, VAL2FLD(USB_OTG_DCFG_DAD, addr));
}

////////////////////////////////////////////////////////////////////////////////////
/*************
 * @name out endpoint interrupt handler
 * @param void
 * @note out endpoint interrupt handler
 */
static void OUT_endpoint_Irqhandler()
{
   // get the endpoint who fiers the interrupt
   uint8_t num = ((USB_OTG_DEVICE->DAINT >> 16) & 0x0f);
   switch (num)
   {
   case 0x01:
      num = 0;
      break;
   case 0x02:
      num = 1;
      break;
   case 0x04:
      num = 2;
      break;
   case 0x08:
      num = 3;
      break;
   }

   uint32_t status = OUT_ENDPOINT(num)->DOEPINT;
   out_endp_int int_value = 0;

   if (READ_bit(status, USB_OTG_DOEPINT_XFRC))
   {
      // handle the in transfer complete callback in framework layer
      int_value = xfrc;
      // after handling the interrupt clear the interrupt flag
      SET_bit(OUT_ENDPOINT(num)->DOEPINT, USB_OTG_DOEPINT_XFRC);
     
   }
   else if (READ_bit(status, USB_OTG_DOEPINT_EPDISD))
   {
      int_value = ep_disd;
      SET_bit(OUT_ENDPOINT(num)->DOEPINT, USB_OTG_DOEPINT_EPDISD);
   }
   else if (READ_bit(status, USB_OTG_DOEPINT_STUP))
   {
      int_value = stup;
      SET_bit(OUT_ENDPOINT(num)->DOEPINT, USB_OTG_DOEPINT_STUP);
   }
   else if (READ_bit(status, USB_OTG_DOEPINT_OTEPSPR))
   {
      int_value = sts_phs_rx;
      SET_bit(OUT_ENDPOINT(num)->DOEPINT, USB_OTG_DOEPINT_OTEPSPR);
   }
   else if (READ_bit(status, USB_OTG_DOEPINT_NAK))
   {
      int_value = _nak;
      SET_bit(OUT_ENDPOINT(num)->DOEPINT, USB_OTG_DOEPINT_NAK);
   }
   // you can implement more irq handler function here based on your requirement
   OUT_endpoint_callback(num, int_value);
}

////////////////////////////////////////////////////////////////////////////////////
/********
 * @name in endpoint irq handler
 * @param void
 * @note used to handle the interrupt on in endpoint
 */
static void IN_endpoint_Irqhandler()
{
   uint8_t num = (USB_OTG_DEVICE->DAINT & 0x0f);
   switch (num)
   {
   case 0x01:
      num = 0;
      break;
   case 0x02:
      num = 1;
      break;
   case 0x04:
      num = 2;
      break;
   case 0x08:
      num = 3;
      break;
   }
   uint32_t status = IN_ENDPOINT(num)->DIEPINT;
   in_endp_int int_Value = 0;
   if (READ_bit(status, USB_OTG_DIEPINT_XFRC))
   {
      // handle the in transfer complete callback in framework layer
      int_Value = xfr_cmpt;
      // after handling the interrupt clear the interrupt flag
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_XFRC);
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_TXFE))
   {
      int_Value = txfe;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_TXFE);
      // disable the interrupt after getiing the txfifo empty
      CLEAR_bit(USB_OTG_DEVICE->DIEPEMPMSK, _BV(num));
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_INEPNE))
   {
      int_Value = inepne;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_INEPNE);
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_NAK))
   {
      int_Value = nak;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_NAK);
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_EPDISD))
   {
      int_Value = epdisd;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_EPDISD);
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_TOC))
   {
      int_Value = toc;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_TOC);
   }
   else if (READ_bit(status, USB_OTG_DIEPINT_ITTXFE))
   {
      int_Value = in_token_txfe;
      SET_bit(IN_ENDPOINT(num)->DIEPINT, USB_OTG_DIEPINT_ITTXFE);
   }

   IN_endpoint_callback(num, int_Value);

   // you can implement more irq handler function here based on your requirement
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// reset callback must be done after init the core
static void reset_callback()
{

   // configure the device after a reset callback

   // enable the core interrupts
   SET_bit(USB_OTG_GLOBAL->GINTMSK, USB_OTG_GINTMSK_RXFLVLM | USB_OTG_GINTMSK_ESUSPM |
                                        USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM |
                                        USB_OTG_GINTMSK_IEPINT | USB_OTG_GINTMSK_OEPINT);

   // clear all pending core interrupt
   WRITE_reg(USB_OTG_GLOBAL->GINTSTS, 0xFFFFFFFF);
   // unmask the USB global interrupt
   SET_bit(USB_OTG_GLOBAL->GAHBCFG, USB_OTG_GAHBCFG_GINT | USB_OTG_GAHBCFG_TXFELVL);

   // /// set nak for all the out endpoints
   // // first check that the GONAKEFF bit is cleared in GINTSTS
   // if( READ_bit(USB_OTG_GLOBAL->GINTSTS , USB_OTG_GINTSTS_BOUTNAKEFF))
   // {
   //    printf("error in out naks\r\n");

   // }
   // SET_bit(USB_OTG_DEVICE->DCTL , USB_OTG_DCTL_SGONAK );

   // // set the nak for the in endpoint also
   // if ( READ_bit(USB_OTG_GLOBAL->GINTSTS , USB_OTG_GINTSTS_GINAKEFF))
   //  {
   //     printf("error in IN NAK\r\n");
   //  }
   //  SET_bit( USB_OTG_DEVICE->DCTL , USB_OTG_DCTL_SGINAK);
   // unmask the all global interrupts for all endpoint
   SET_bit(USB_OTG_DEVICE->DIEPMSK, USB_OTG_DIEPMSK_XFRCM | USB_OTG_DIEPMSK_ITTXFEMSK | USB_OTG_DIEPMSK_INEPNEM | USB_OTG_DIEPMSK_TOM | USB_OTG_DIEPMSK_EPDM);
   SET_bit(USB_OTG_DEVICE->DOEPMSK, USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_OTEPSPRM | USB_OTG_DOEPMSK_STUPM | USB_OTG_DOEPMSK_EPDM);
}
// ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////****
/***
 * @name dev init
 * @param void
 * @note used to init the device
 *
 */
static void devinit()
{
   // // deconfigure the endpint to the RESET level

   // for (uint8_t i = 0; i < (ENDPOINTCOUNT); i++)
   // {
   //    deconfigure_in_endpoint(i);
   //    deconfigure_out_endpoint(i);
   // }

   // restart the PHY clock
   WRITE_reg(USB_OTG_PCGCCTL, 0);

   // configure the usb to run in full speed mode b
   CLEAR_bit(USB_OTG_DEVICE->DCFG, USB_OTG_DCFG_PFIVL);
   // device mode configuration
   SET_bit(USB_OTG_DEVICE->DCFG, USB_OTG_DCFG_NZLSOHSK | USB_OTG_DCFG_PFIVL_0 | VAL2FLD(USB_OTG_DCFG_DSPD, 0x3));
   // enable the Vbus sensing device
   SET_bit(USB_OTG_GLOBAL->GCCFG, USB_OTG_GCCFG_VBUSBSEN);

   reset_fifo();
  
}

///////////////////////////////////////////////////////////////////////////////////
/*********
 * @name initialize core
 * @param void
 * @note used to init the USB core
 */
static void initialize_core()
{
   __IO uint32_t count = 0;

   WRITE_reg(USB_OTG_GLOBAL->GUSBCFG, 0);

   SET_bit(USB_OTG_GLOBAL->GUSBCFG, USB_OTG_GUSBCFG_PHYSEL);

   reset_handler();

   SET_bit(USB_OTG_GLOBAL->GUSBCFG, USB_OTG_GUSBCFG_FDMOD | VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x06) | _VAL2FLD(USB_OTG_GUSBCFG_TOCAL, 0x01));

   do
   {
      if (++count > 2000U)
      {
         printf("error");
         return;

      }
      /* code */
   } while (READ_bit(USB_OTG_GLOBAL->GINTSTS, USB_OTG_GINTSTS_CMOD));

   // CLEAR_bit( USB_OTG_GLOBAL->GUSBCFG , USB_OTG_GUSBCFG_TRDT | USB_OTG_GUSBCFG_FDMOD );
   // // configure the usb to run in device full speed mode and selesct the embedded phy
   // SET_bit(USB_OTG_GLOBAL->GUSBCFG, USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | VAL2FLD(USB_OTG_GUSBCFG_TRDT , 0x06));

   reset_callback();
   devinit();
   // must reset the whole usb engine

}

static void connect()
{
   // in case PHY is stooped
   WRITE_reg(USB_OTG_PCGCCTL, 0);
   // power on the transreciever
   SET_bit(USB_OTG_GLOBAL->GCCFG, USB_OTG_GCCFG_PWRDWN);
   // connect the device to the bus
   CLEAR_bit(USB_OTG_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
}

/////////////////////////////////////////////////////////////////////////////
static void disconnect()
{
   // disconnect the device from the bus
   SET_bit(USB_OTG_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
   // power off the transreciever
   CLEAR_bit(USB_OTG_GLOBAL->GCCFG, USB_OTG_GCCFG_PWRDWN);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************
 * ***********************   static functionn end here
 * *****************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
void __attribute__((externally_visible)) gintsts_handler()
{
   volatile uint32_t gintsts = USB_OTG_GLOBAL->GINTSTS;

   // after the handling of the interrupt we have to clear it by setting the bit

   // if(READ_bit(gintsts , USB_OTG_GINTSTS_WKUINT))
   // {
   //    // interrupt generated by the remote wakeup

   //   SET_bit(USB_OTG_GLOBAL->GINTSTS, USB_OTG_GINTSTS_WKUINT);

   // }
   if (READ_bit(gintsts, USB_OTG_GINTSTS_OEPINT))
   {
      OUT_endpoint_Irqhandler();
      // this intrerrupt dont require to be cleared automatically cleared by the usb core
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_IEPINT))
   {
      IN_endpoint_Irqhandler();

      // this intrerrupt dont require to be cleared automatically cleared by the usb core
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_ENUMDNE))
   {
      printf("enumdone\r\n");
      // enumeration done interrupt
      enumdone_handler();

      CLEAR_IT(USB_OTG_GINTSTS_ENUMDNE);
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_USBRST))
   {
      printf(" usb reset\r\n");

      for (uint8_t i = 0; i < (ENDPOINTCOUNT - 1); i++)
      {
         deconfigure_out_endpoint(i);
         deconfigure_in_endpoint(i);
      }
      reset_fifo();

      // this function is implemented in the framework layer
      usb_reset_recv_callback();

      CLEAR_IT(USB_OTG_GINTSTS_USBRST);
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_USBSUSP))
   {
      // usb suspended
      printf("usb suspended\r\n");

      CLEAR_IT(USB_OTG_GINTSTS_USBSUSP);
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_ESUSP))
   {
      // early suspended detected
      printf("early suspended\r\n");

      CLEAR_IT(USB_OTG_GINTSTS_ESUSP);
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_RXFLVL))
   {
      // mask the interrupt of the RXFLVL
      CLEAR_bit(USB_OTG_GLOBAL->GINTMSK, USB_OTG_GINTMSK_RXFLVLM);

      RXflvl_handler();
      // this intrerrupt dont require to be cleared automatically cleared by the usb core
 
      // unmask the RXFLFVL interrupt
      SET_bit(USB_OTG_GLOBAL->GINTMSK, USB_OTG_GINTMSK_RXFLVLM);
      CLEAR_IT(USB_OTG_GINTSTS_RXFLVL);
     
   }

   else if (READ_bit(gintsts, USB_OTG_GINTSTS_GINAKEFF))
   {
      // thiis indicaate that the NAK bit set in the IN DCTL has taken effect
      printf("globalin nak effective interrupt\r\n");
      // to clear the bit we need to set the CGINAK in DCTL reg
      SET_bit(USB_OTG_DEVICE->DCTL, USB_OTG_DCTL_CGINAK);

      // call the handler routine
   }
   else if (READ_bit(gintsts, USB_OTG_GINTSTS_BOUTNAKEFF))
   {
      // this bit is cleared by the setting the global out nak bit in the otg_FS_DCT
      printf("global outnak\r\n");
      SET_bit(USB_OTG_DEVICE->DCTL, USB_OTG_DCTL_CGONAK);
   }
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void ActivateEP(uint8_t  num, eptype type)
{
   /// we only need to set the endpoint USBAEP and set the nak bit
   if (num == endp0)
   {
      return;
   }
   else
   {
      if (type == IN)
      {
         
            // activate the endpoint in the curret configuration
            SET_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_USBAEP);
                 
      }
      else if (type == OUT)
      {
          // activate the endpoint in the current configuration
            SET_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_USBAEP);
        
      }
}
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void DeactivateEP(uint8_t num, eptype type)
{

   /// we only need to reset the endpoint USBAEP and set the nak bit
   if (num == endp0)
   {
      return;
   }
   else
   {
      if (type == IN)
      {
            // activate the endpoint in the curret configuration
            CLEAR_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_USBAEP);
      }
      else if (type == OUT)
      {
           // activate the endpoint in the current configuration
            CLEAR_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_USBAEP);
      }
}
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
/// used to stall the endpoint or to deactivate the endpoint
void setstall(endp_no num, eptype type)
{
   // first make sure that endpoint is enable
   if (type == IN)
   {
         SET_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_STALL);
    
   }
   else
   {
      SET_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_STALL);
    
   }

}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// used to clear the stall of the endpoint
void clearstall(endp_no num, eptype type)
{
   // first make sure that endpoint is enable
   if (type == IN)
   {
     CLEAR_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_STALL);
     
   }
   else
   {
     CLEAR_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_STALL);
   
   }

}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void actv_remote_signal(void)
{
 // before wakimg up the host first check if the usb is in suspended state 
  if( READ_bit(USB_OTG_DEVICE->DSTS , USB_OTG_DSTS_SUSPSTS ))
  {
     // activate the remote wakeup signaling
     SET_bit( USB_OTG_DEVICE->DCTL , USB_OTG_DCTL_RWUSIG );
  }

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void deacv_remote_signal(void)
{
  // deactivate the remote wakeup signal 
  CLEAR_bit( USB_OTG_DEVICE->DCTL , USB_OTG_DCTL_RWUSIG );

}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void reset_handler()
{

   __IO uint32_t count = 0U;

   /* Wait for AHB master IDLE state. */
   do
   {
      if (++count > 200000U)
      {
         printf("error");
         return;
      }
   } while ((USB_OTG_GLOBAL->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U);

   /* Core Soft Reset */
   count = 0U;
   SET_bit(USB_OTG_GLOBAL->GRSTCTL, USB_OTG_GRSTCTL_CSRST);

   do
   {
      if (++count > 200000U)
      {
         printf("error");
         return;
      }
   } while ((USB_OTG_GLOBAL->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);


}

/////////////////////////////////////////////////////////////////////////////////
/**********
 * @name configure endpoint 0
 * @param endpoint0 size
 */
void configure_endpoint0(endp_size size)
{
   // unmask all interrupt for in and out endpoint0
   SET_bit(USB_OTG_DEVICE->DAINTMSK, _BV(USB_OTG_DAINTMSK_OEPM_Pos) | _BV(USB_OTG_DAINTMSK_IEPM_Pos));

   // configure the maximim packet size , activate the endpoint and nack the endpoint
   SET_bit(IN_ENDPOINT(0)->DIEPCTL, VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, size));

   // set the max packet size of the endpoint
   SET_bit(OUT_ENDPOINT(0)->DOEPCTL, VAL2FLD(USB_OTG_DOEPCTL_MPSIZ, size));

   // we have to configure the size of fifo
   // 64 bytes is the max packet size for the full speed out endp0
   configure_rx_fifo(64);
   configure_tx_fifo(0, 64);

   // enable endp0 for receving out request
   // since the size of the request is 8 bytes
   enableep0(8);
}

////////////////////////////////////////////////////////////////
void configure_in_endpoint(uint8_t endpoint_number, endpoint_typedef usb_endpoint_type, uint16_t endpoint_size)
{
   /// unmask all interrrupt for the targeted endpoint
   SET_bit(USB_OTG_DEVICE->DAINTMSK, _BV(endpoint_number));
   //// activate the endpoint , set ep handshake to nack , set DATA0 PID ()
   SET_bit(IN_ENDPOINT(endpoint_number)->DIEPCTL, USB_OTG_DIEPCTL_SD0PID_SEVNFRM | USB_OTG_DIEPCTL_SNAK |
                                                      VAL2FLD(USB_OTG_DIEPCTL_EPTYP, usb_endpoint_type) | VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, endpoint_size) | VAL2FLD(USB_OTG_DIEPCTL_TXFNUM, endpoint_number));
   ActivateEP(1 ,IN);
   configure_tx_fifo(endpoint_number, endpoint_size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void configure_out_endpoint(uint8_t endpoint_num, endpoint_typedef usb_endpoint_type, uint16_t endpoint_size)
{
   // unmask all interrupt from the tagrgeted endpoint
   SET_bit(USB_OTG_DEVICE->DAINTMSK, _BV((endpoint_num + 16)));

   // activate the endpoint , set the ep hanshake to nack , set data pid
   SET_bit(OUT_ENDPOINT(endpoint_num)->DOEPCTL, USB_OTG_DOEPCTL_SNAK |
                                                    VAL2FLD(USB_OTG_DOEPCTL_EPTYP, usb_endpoint_type) | VAL2FLD(USB_OTG_DOEPCTL_MPSIZ, endpoint_size));

   // the size of the endpoint is fixed to maximum i.e. 64 bytes
}

//////////////////////////////////////////////////////////////////////
void deconfigure_in_endpoint(endp_no num)
{
   // mask all interrupt of the targeted endpoint
   CLEAR_bit(USB_OTG_DEVICE->DAINTMSK, _BV(num));
   // clear the interrupts of the endpoint
   SET_bit(IN_ENDPOINT(num)->DIEPINT, 0xFB7FU);

   // disable the endpoint
   if (READ_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_EPENA))
   {
      SET_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_EPDIS);
   }

   // deactivate the endpoint
   CLEAR_bit(IN_ENDPOINT(num)->DIEPCTL, USB_OTG_DIEPCTL_MPSIZ | USB_OTG_DIEPCTL_EPTYP | USB_OTG_DIEPCTL_USBAEP);

   // clears the transfer size
   WRITE_reg(IN_ENDPOINT(num)->DIEPTSIZ, 0);
   // flush the tx fifo
   if (flush_txfifo(num) != ok)
   {
      error_handler(error);
   }
}

///////////////////////////////////////////////////////////////////////
void deconfigure_out_endpoint(endp_no num)
{
   // mask all the interrupt for out endpoint
   CLEAR_bit(USB_OTG_DEVICE->DAINTMSK, (1 << (16 + num)));
   // clear the interrupt for the endpoint
   SET_bit(OUT_ENDPOINT(num)->DOEPINT, 0xFB7FU);

   // flush the rx fifo
   if (flush_rxfifo() != ok)
   {
      error_handler(error);
   }

   // clears the out endpoint size
   WRITE_reg(OUT_ENDPOINT(num)->DOEPTSIZ, 0);
   
   if (num == 0)
      return;
   // disable the endpoint
   if (READ_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_EPENA))
   {
      SET_bit(OUT_ENDPOINT(num)->DOEPCTL, USB_OTG_DOEPCTL_EPDIS);
   }
}

//////////////////////////////////////////////////
//////////   Weak function defination here that user want to implement in the main.c
/////////////////////////////////////////////////////////////////////////////////////////
// these weak functions are implemented in the fraewrok layer

// initialize the usb pinout for the MCU and enable the irq for the usb
__WEAK void USB_Msp_init(void)
{
}

// deinitialize the usb pins
__WEAK void USB_Msp_deinit(void)
{
}
// callback of setup data packet recieved
__WEAK void setup_data_recv_callback(endp_no endpoint_num, uint16_t size)
{
}

__WEAK void usb_reset_recv_callback(void)
{
}

__WEAK void global_out_nak_callback(void)
{
}

__WEAK void out_data_recv_callback(uint8_t endp_num, uint16_t xfer_size)
{
}

__WEAK void out_Xfer_cmpt_callbcak(uint8_t ep_num)
{
}

__WEAK void setup_stage_cmpt_callback(uint8_t ep_num)
{
}

__WEAK void IN_endpoint_callback(endp_no num, in_endp_int int_val)
{
}

__WEAK void OUT_endpoint_callback(endp_no num, out_endp_int int_val)
{
}

__WEAK void error_handler(usb_status usb_Status)
{
}
////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////    Weak function End here
/////////////////////////////////////////////////////////////////////////////

/// give all function defination here
const USBdriver usb_driver =
    {
        .initialize_core = initialize_core,
        .USB_Msp_init = USB_Msp_init,
        .USB_Msp_deinit = USB_Msp_deinit,
        .connect = connect,
        .disconnect = disconnect,
        .configure_endpoint0 = configure_endpoint0,
        .configure_in_endpoint = configure_in_endpoint,
        .deconfigure_in_endpoint = deconfigure_in_endpoint,
        .deconfigure_out_endpoint = deconfigure_out_endpoint,
        .enableep0 = enableep0,
        .read_packet = read_packet,
        .write_packet = write_packet,
        .set_deviceaddr = set_deviceaddr};
