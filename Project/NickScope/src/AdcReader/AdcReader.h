#ifndef __ADC_READER_H__
#define __ADC_READER_H__

#include "Types.h"

#define ADC_READER_TASK_NAME        (signed char *)"AdcReader"
#define ADC_READER_TASK_STACK       128
#define ADC_READER_TASK_PRIORITY    (tskIDLE_PRIORITY+1)
#define ADC_READER_TASK_DELAY_TICKS (10/portTICK_RATE_MS)


#define ADC_READER_BUF_LEN          1024


typedef enum
{
   ADC_READER_STOP,
   ADC_READER_READ_SINGLE,
   ADC_READER_READ_CONTINUOUS,
   ADC_READER_READ_BURST,
   ADC_READER_SAMPLING
} AdcReaderCommand_E;


typedef struct
{
   AdcReaderCommand_E type;
} AdcReaderCommand_T;


bool AdcReaderInit();
void AdcReaderQueueEvent(AdcReaderCommand_T *event);

uint8_t AdcTrimSampleData(AdcCounts_T data);

#endif //__ADC_READER_H__
