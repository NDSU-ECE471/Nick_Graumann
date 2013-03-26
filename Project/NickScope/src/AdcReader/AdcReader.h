#ifndef __ADC_READER_H__
#define __ADC_READER_H__

#include <stdbool.h>

#define ADC_READER_TASK_NAME        (signed char *)"AdcReader"
#define ADC_READER_TASK_STACK       128
#define ADC_READER_TASK_PRIORITY    (tskIDLE_PRIORITY+1)
#define ADC_READER_TASK_DELAY_TICKS (10/portTICK_RATE_MS)


bool AdcReaderInit();

#endif //__ADC_READER_H__
