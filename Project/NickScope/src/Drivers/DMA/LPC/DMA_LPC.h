#ifndef __DMA_LPC_H__
#define __DMA_LPC_H__

#include <stdbool.h>
#include "../DMA_Facade.h"


///////////////////////////////////////////////////////////////////////////////
//
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E LPC_DMA_Init();
DMA_Error_E LPC_DMA_InitChannel(DMA_Channel_T channel,
                                DMA_Address_T srcAddr, DMA_Address_T destAddr,
                                DMA_Peripheral_E srcPeriph, DMA_Peripheral_E destPeriph,
                                DMA_TransferSize_T xferSize,
                                DMA_BurstSize_E srcBurstSize, DMA_BurstSize_E destBurstSize,
                                DMA_TransferWidth_E srcWidth, DMA_TransferWidth_E destWidth,
                                bool incSrcAddr, bool incDestAddr);
DMA_Error_E LPC_DMA_BeginTransfer(DMA_Channel_T channel, DMA_Callback_T callback);
DMA_Error_E LPC_DMA_FindFreeChannel(DMA_Channel_T *channel);


#endif //__DMA_LPC_H__
