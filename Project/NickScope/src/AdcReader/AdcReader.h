#ifndef __ADC_READER_H__
#define __ADC_READER_H__

#include "Types.h"


///////////////////////////////////////////////////////////////////////////////
//
// Global definitions
//
///////////////////////////////////////////////////////////////////////////////
#define ADC_READER_TASK_NAME        (signed char *)"AdcReader"
#define ADC_READER_TASK_STACK       128
#define ADC_READER_TASK_PRIORITY    (configMAX_PRIORITIES-1)
#define ADC_READER_TASK_DELAY_TICKS (10/portTICK_RATE_MS)


#define ADC_READER_BUF_LEN          1024
#define ADC_READER_MAX_COUNTS       255


#define ADC_SAMPLE_MUTEX_TIMEOUT    (10/portTICK_RATE_MS)


///////////////////////////////////////////////////////////////////////////////
//
// Enumerations
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
   ADC_READER_STOP,
   ADC_READER_READ_SINGLE,
   ADC_READER_READ_CONTINUOUS,
   ADC_READER_READ_BURST,
   ADC_READER_SAMPLING,
   ADC_READER_INC_SAMPLERATE,
   ADC_READER_DEC_SAMPLERATE
} AdcReaderCommand_E;


///////////////////////////////////////////////////////////////////////////////
//
// Structures
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
   AdcReaderCommand_E type;
} AdcReaderCommand_T;


///////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
///////////////////////////////////////////////////////////////////////////////
bool AdcReaderInit();
void AdcReaderQueueEvent(AdcReaderCommand_T *event);
void AdcReaderGetSampleBuffer(volatile AdcCounts_T **bufPtr, size_t *size);

uint8_t AdcTrimSampleData(AdcCounts_T data);


#endif //__ADC_READER_H__
