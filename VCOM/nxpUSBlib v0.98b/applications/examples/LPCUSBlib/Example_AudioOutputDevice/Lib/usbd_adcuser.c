/*
* Copyright(C) NXP Semiconductors, 2012
* All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* LPC products.  This software is supplied "AS IS" without any warranties of
* any kind, and NXP Semiconductors and its licensor disclaim any and
* all warranties, express or implied, including all implied warranties of
* merchantability, fitness for a particular purpose and non-infringement of
* intellectual property rights.  NXP Semiconductors assumes no responsibility
* or liability for the use of the software, conveys no license or rights under any
* patent, copyright, mask work right, or any other intellectual property rights in
* or to any products. NXP Semiconductors reserves the right to make changes
* in the software without notification. NXP Semiconductors also makes no
* representation or warranty that such application will be suitable for the
* specified use without further testing or modification.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors' and its
* licensor's relevant copyrights in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
*/

#define  __INCLUDE_FROM_USB_DRIVER
#include "../AudioOutput.h"

#if defined(USB_DEVICE_ROM_DRIVER)

#include "usbd_adcuser.h"

/** Internal definition */
#define AUDIO_MAX_SAMPLE_FREQ 48000

/* control interface number */
#define USB_ADC_CIF_NUM     	0
/* streaming interface number */
#define USB_ADC_SIF1_NUM    	1
/* Volume definitions */
#define VOLUME_MIN          0x0000
#define VOLUME_MAX          0x003F
#define VOLUME_RES          0x0001

uint32_t ISO_packet_size = 0;
uint32_t ISO_buffer_address;

/* Device Transfer Descriptor used in Custom ROM mode */
DeviceTransferDescriptor Rom_dTD ATTR_ALIGNED(32);
/* external Audio Sample Frequency variable */
extern uint32_t CurrentAudioSampleFrequency;
/* Current Volume */
uint32_t curr_vol;

extern uint32_t CALLBACK_HAL_GetISOBufferAddress(const uint32_t EPNum, uint32_t *last_packet_size);
extern void Audio_Reset_Data_Buffer(void);

/* inline functions */
static __INLINE DeviceQueueHead* Usbd_GetEpQH(USB_CORE_CTRL_T* pCtrl, uint8_t ep)
{
    DeviceQueueHead* ep_QH = (DeviceQueueHead*)(*((uint32_t*)pCtrl->hw_data));
    uint32_t ep_idx = (ep & 0x0F) << 1;

    if(ep & 0x80)
       ep_idx++;

    return &ep_QH[ep_idx];
}

/** Get Transfer packet size */
static __INLINE uint32_t Usbd_GetTransferSize(USB_CORE_CTRL_T* pCtrl, uint8_t ep)
{
	DeviceQueueHead* ep_QH = Usbd_GetEpQH(pCtrl, ep);
	return (ep_QH->TransferCount - ep_QH->overlay.TotalBytes);
}

void UsbdDcdDataTransfer(uint8_t EPNum, uint8_t *pData, uint32_t length)
{
	uint8_t PhyEP = (EPNum<<1) | (EPNum>>7); /* Rotate left without carry */
	DeviceQueueHead* ep_QH = Usbd_GetEpQH((USB_CORE_CTRL_T*) UsbHandle, EPNum);
	DeviceTransferDescriptor*  pDTD = (DeviceTransferDescriptor*) &Rom_dTD;

	while ( LPC_USB0->ENDPTSTAT & _BIT( EP_Physical2BitPosition(PhyEP) ) )	/* Endpoint is already primed */
	{
	}

	/* Zero out the device transfer descriptors */
	memset((void*)pDTD, 0, sizeof(DeviceTransferDescriptor));

	if(((ENDPTCTRL_REG(PhyEP/2)>>2)&EP_TYPE_MASK)==EP_TYPE_ISOCHRONOUS)	// iso out endpoint
	{
		uint32_t mult = (USB_DATA_BUFFER_TEM_LENGTH + 1024) / 1024;
		pDTD->NextTD = LINK_TERMINATE ;
		ep_QH->Mult = mult;
	}
	else if(((ENDPTCTRL_REG(PhyEP/2)>>18)&EP_TYPE_MASK)==EP_TYPE_ISOCHRONOUS)// iso in endpoint
	{

	}
	pDTD->TotalBytes = length;
	pDTD->IntOnComplete = 1;
	pDTD->Active = 1;
	pDTD->MultiplierOverride = 1;

	pDTD->BufferPage[0] = (uint32_t) pData;
	pDTD->BufferPage[1] = ((uint32_t) pData + 0x1000) & 0xfffff000;
	pDTD->BufferPage[2] = ((uint32_t) pData + 0x2000) & 0xfffff000;
	pDTD->BufferPage[3] = ((uint32_t) pData + 0x3000) & 0xfffff000;
	pDTD->BufferPage[4] = ((uint32_t) pData + 0x4000) & 0xfffff000;

	ep_QH->Mult = 1;
	ep_QH->MaxPacketSize = 512;
	ep_QH->overlay.NextTD = (uint32_t) pDTD;
	ep_QH->TransferCount = length;

	/* prime the endpoint for transmit */
	LPC_USB0->ENDPTPRIME |= _BIT( EP_Physical2BitPosition(PhyEP) ) ;
}


/** Initialize USBD ADC driver */
void UsbdAdc_Init(void)
{
	uint32_t ep_indx;
	USBD_API->hw->ForceFullSpeed(UsbHandle,1);

	/* register ep0 handler */
	USBD_API->core->RegisterClassHandler(UsbHandle, UsbdAdc_ep0_hdlr, NULL);

	/* register ISO OUT endpoint interrupt handler */
	ep_indx = ((AUDIO_STREAM_EPNUM & 0x0F) << 1);
	USBD_API->core->RegisterEpHandler (UsbHandle, ep_indx, UsbdAdc_ISO_OUT_Hdlr, NULL);

}


/**----------------------------------------------------------------------------
  ADC_IF_GetRequest: Audio Device Class Interface Get Request Callback
    Called automatically on ADC Interface Get Request
 *----------------------------------------------------------------------------*/
ErrorCode_t ADC_IF_GetRequest (USB_CORE_CTRL_T* pCtrl)
{

    /*
      Interface = SetupPacket.wIndex.WB.L;
      EntityID  = SetupPacket.wIndex.WB.H;
      Request   = SetupPacket.bRequest;
      Value     = SetupPacket.wValue.W;
      ...
    */
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;

    if (pCtrl->SetupPacket.wIndex.W == 0x0200) {
        /* Feature Unit: Interface = 0, ID = 2 */
        if (pCtrl->SetupPacket.wValue.WB.L == 0) {
            /* Master Channel */
            switch (pCtrl->SetupPacket.wValue.WB.H) {
            case AUDIO_MUTE_CONTROL:
                if (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_GET_CUR) {
                	/*TODO: Get MUTE */
                    //pCtrl->EP0Buf[0] = (ADC_PLAY_MUTE)?1:0;
                    ret = LPC_OK;
                }
                break;
            case AUDIO_VOLUME_CONTROL:
                switch (pCtrl->SetupPacket.bRequest) {
                case AUDIO_REQUEST_GET_CUR:
                    *((ATTR_PACKED uint16_t *)pCtrl->EP0Buf) = curr_vol;
                    ret = LPC_OK;
                    break;
                case AUDIO_REQUEST_GET_MIN:
                    *((ATTR_PACKED uint16_t *)pCtrl->EP0Buf) = VOLUME_MIN;
                    ret = LPC_OK;
                    break;
                case AUDIO_REQUEST_GET_MAX:
                    *((ATTR_PACKED uint16_t *)pCtrl->EP0Buf) = VOLUME_MAX;
                    ret = LPC_OK;
                    break;
                case AUDIO_REQUEST_GET_RES:
                    *((ATTR_PACKED uint16_t *)pCtrl->EP0Buf) = VOLUME_RES;
                    ret = LPC_OK;
                    break;
                }
                break;
            }
        }
    }
    
    return (ret);  /* Not Supported */
}


/**----------------------------------------------------------------------------
  ADC_IF_SetRequest: Audio Device Class Interface Set Request Callback
    Called automatically on ADC Interface Set Request
 *----------------------------------------------------------------------------*/
ErrorCode_t ADC_IF_SetRequest (USB_CORE_CTRL_T* pCtrl)
{

/*
  Interface = SetupPacket.wIndex.WB.L;
  EntityID  = SetupPacket.wIndex.WB.H;
  Request   = SetupPacket.bRequest;
  Value     = SetupPacket.wValue.W;
  ...
*/
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;
    if (pCtrl->SetupPacket.wIndex.W == 0x0200) {
        /* Feature Unit: Interface = 0, ID = 2 */
        if ((pCtrl->SetupPacket.wValue.WB.L == 0) &&
            (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_SET_CUR)) {
            /* Master Channel */
            switch (pCtrl->SetupPacket.wValue.WB.H) {
            case AUDIO_MUTE_CONTROL:
                if (pCtrl->EP0Buf[0])
                {    /*TODO: set MUTE here */
                }else
                {   /*TODO: disable MUTE here */
                }
                ret = (LPC_OK);
                break;
            case AUDIO_VOLUME_CONTROL:
                /*TODO: Set volume here */
				curr_vol = *((ATTR_PACKED uint16_t *)pCtrl->EP0Buf);
                ret =  (LPC_OK);
                break;
            }
        }
    }

    return ret;  /* Not Supported */
}


/**----------------------------------------------------------------------------
  ADC_EP_GetRequest: Audio Device Class EndPoint Get Request Callback
    Called automatically on ADC EndPoint Get Request
 *----------------------------------------------------------------------------*/
ErrorCode_t ADC_EP_GetRequest (USB_CORE_CTRL_T* pCtrl)
{
    /*
      EndPoint = SetupPacket.wIndex.WB.L;
      Request  = SetupPacket.bRequest;
      Value    = SetupPacket.wValue.W;
      ...
    */
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;
    
    switch(pCtrl->SetupPacket.wIndex.W) {
    case AUDIO_STREAM_EPNUM:
        /* Feature Unit: Interface = 0, ID = 2 */
        if (pCtrl->SetupPacket.wValue.WB.L == 0) {
            /* Master Channel */
            if ((pCtrl->SetupPacket.wValue.WB.H == AUDIO_CONTROL_SAMPLING_FREQ) &&
                (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_GET_CUR) ) {
                pCtrl->EP0Buf[0] = (uint8_t)(CurrentAudioSampleFrequency & 0xFF);
                pCtrl->EP0Buf[1] = (uint8_t)((CurrentAudioSampleFrequency >> 8) & 0xFF);
                pCtrl->EP0Buf[2] = (uint8_t)((CurrentAudioSampleFrequency >> 16) & 0xFF);
                ret = (LPC_OK);
            }
        }
        break;
    default:
        break;
    }
    return ret;  /* Not Supported */
}


/**----------------------------------------------------------------------------
  ADC_EP_SetRequest: Audio Device Class EndPoint Set Request Callback
    Called automatically on ADC EndPoint Set Request
 *----------------------------------------------------------------------------*/
ErrorCode_t ADC_EP_SetRequest (USB_CORE_CTRL_T* pCtrl) 
{

    /*
      EndPoint = SetupPacket.wIndex.WB.L;
      Request  = SetupPacket.bRequest;
      Value    = SetupPacket.wValue.W;
      ...
    */
    uint32_t rate;
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;
    
    switch(pCtrl->SetupPacket.wIndex.W) {
    case AUDIO_STREAM_EPNUM:
        /* Feature Unit: Interface = 0, ID = 2 */
        if (pCtrl->SetupPacket.wValue.WB.L == 0) {
            /* Master Channel */
            if (pCtrl->SetupPacket.wValue.WB.H == AUDIO_CONTROL_SAMPLING_FREQ) {
                rate = pCtrl->EP0Buf[0] | (pCtrl->EP0Buf[1] << 8) | (pCtrl->EP0Buf[2] << 16);
                if (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_SET_CUR) {
                	CurrentAudioSampleFrequency = rate;
                	if(CurrentAudioSampleFrequency <= AUDIO_MAX_SAMPLE_FREQ)
                	{
                		Audio_Init(CurrentAudioSampleFrequency);
                        ret = LPC_OK;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return (ret);  /* Not Supported */
}

/**----------------------------------------------------------------------------
  Override standard Interface Event
 *----------------------------------------------------------------------------*/
ErrorCode_t USB_Interface_Event (USBD_HANDLE_T hUsb)
{
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    uint16_t wIndex = pCtrl->SetupPacket.wIndex.W;
    uint16_t wValue = pCtrl->SetupPacket.wValue.W;
    
    /* write code to enable/disable audo playback when interface 
    ALT setting is changed */
    if (wIndex == USB_ADC_SIF1_NUM) {
    	if((wValue == 0x0001)){
    		UsbdAdc_start_xfr();
    	}else
    	{
    		UsbdAdc_stop_xfr();
    	}

    }

    return LPC_OK;
}
/** disable configure event in usbd_rom.c */
ErrorCode_t USB_Configure_Event (USBD_HANDLE_T hUsb)
{
	return LPC_OK;
}
/**----------------------------------------------------------------------------
  Audio Class handler
 *----------------------------------------------------------------------------*/
ErrorCode_t UsbdAdc_ep0_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    ErrorCode_t ret = ERR_USBD_UNHANDLED;
    
    if (pCtrl->SetupPacket.bmRequestType.BM.Type == REQUEST_CLASS) {
        switch (event) {
        case USB_EVT_SETUP:
            if ((pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) &&
            ((pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||       /* IF number correct? */
            (pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM)) ) {
                switch (pCtrl->SetupPacket.bRequest) {
                case AUDIO_REQUEST_GET_CUR:
                case AUDIO_REQUEST_GET_MIN:
                case AUDIO_REQUEST_GET_MAX:
                case AUDIO_REQUEST_GET_RES:
                    
                    ret = ADC_IF_GetRequest(pCtrl);
                    if (ret == LPC_OK) {
                        pCtrl->EP0Data.pData = pCtrl->EP0Buf;                     /* point to data to be sent */
                        USBD_API->core->DataInStage(pCtrl);                       /* send requested data */
                    }
                    break;
                case AUDIO_REQUEST_SET_CUR:
                    //                case AUDIO_REQUEST_SET_MIN:
                    //                case AUDIO_REQUEST_SET_MAX:
                    //                case AUDIO_REQUEST_SET_RES:
                    pCtrl->EP0Data.pData = pCtrl->EP0Buf;                              /* data to be received */ 
					
                    ret = LPC_OK;
                    break;
                }
            } else if (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_ENDPOINT) {
                switch (pCtrl->SetupPacket.bRequest) {
                case AUDIO_REQUEST_GET_CUR:
                case AUDIO_REQUEST_GET_MIN:
                case AUDIO_REQUEST_GET_MAX:
                case AUDIO_REQUEST_GET_RES:
                    ret = ADC_EP_GetRequest(pCtrl);
                    if (ret == LPC_OK) {
                        pCtrl->EP0Data.pData = pCtrl->EP0Buf;                              /* point to data to be sent */
                        USBD_API->core->DataInStage(pCtrl);                                   /* send requested data */
                    }
                    break;
                case AUDIO_REQUEST_SET_CUR:
                    //              case AUDIO_REQUEST_SET_MIN:
                    //              case AUDIO_REQUEST_SET_MAX:
                    //              case AUDIO_REQUEST_SET_RES:
                    pCtrl->EP0Data.pData = pCtrl->EP0Buf;                                /* data to be received */ 
                    ret = LPC_OK;
                    break;
                }
            } 
            break;
        case USB_EVT_OUT:
            if ((pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) &&
            ((pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||       /* IF number correct? */
            (pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM)) ) {
                switch (pCtrl->SetupPacket.bRequest) {
                case AUDIO_REQUEST_SET_CUR:
                    //                      case AUDIO_REQUEST_SET_MIN:
                    //                      case AUDIO_REQUEST_SET_MAX:
                    //                      case AUDIO_REQUEST_SET_RES:
                    ret = ADC_IF_SetRequest(pCtrl);
                    if (ret == LPC_OK) {
                        USBD_API->core->StatusInStage(pCtrl);                         /* send Acknowledge */
                    }
                    break;
                }
            } else if (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_ENDPOINT) {
                switch (pCtrl->SetupPacket.bRequest) {
                case AUDIO_REQUEST_SET_CUR:
                    //                    case AUDIO_REQUEST_SET_MIN:
                    //                    case AUDIO_REQUEST_SET_MAX:
                    //                    case AUDIO_REQUEST_SET_RES:
                    ret = ADC_EP_SetRequest(pCtrl);
                    if (ret == LPC_OK) {
                        USBD_API->core->StatusInStage(pCtrl);                           /* send Acknowledge */
                    }
                    break;
                }
            }
            break;
        
        default:
            break;
        }
    }  
    return ret;
}

/**----------------------------------------------------------------------------
  Audio Start Transfer Callback
 *----------------------------------------------------------------------------*/
void UsbdAdc_start_xfr(void)
{
	ISO_packet_size = 0;

	/* reset audio buffer */
	Audio_Reset_Data_Buffer();
	ISO_buffer_address = CALLBACK_HAL_GetISOBufferAddress(AUDIO_STREAM_EPNUM, &ISO_packet_size);
	UsbdDcdDataTransfer(AUDIO_STREAM_EPNUM, (uint8_t*)ISO_buffer_address, 512);
}

/**----------------------------------------------------------------------------
  Audio Stop Transfer Callback
 *----------------------------------------------------------------------------*/
void UsbdAdc_stop_xfr(void)
{
	ISO_packet_size = 0;
	/* reset audio buffer */
	Audio_Reset_Data_Buffer();
	USBD_API->hw->ResetEP(UsbHandle, AUDIO_STREAM_EPNUM);
}


/**----------------------------------------------------------------------------
  Audio ISO OUT handler
 *----------------------------------------------------------------------------*/
ErrorCode_t UsbdAdc_ISO_OUT_Hdlr (USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
	if (event == USB_EVT_OUT)
	{
		ISO_packet_size = Usbd_GetTransferSize((USB_CORE_CTRL_T*) hUsb, AUDIO_STREAM_EPNUM);
		if(ISO_packet_size !=0)
			ISO_packet_size = ISO_packet_size;
		/* Send DMA request */
		ISO_buffer_address = CALLBACK_HAL_GetISOBufferAddress(AUDIO_STREAM_EPNUM, &ISO_packet_size);
		UsbdDcdDataTransfer(AUDIO_STREAM_EPNUM, (uint8_t*)ISO_buffer_address, 512);
	}

    return LPC_OK;
}

#endif
