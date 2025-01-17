#ifndef __USER_INPUT_H__
#define __USER_INPUT_H__

#include "Types.h"


///////////////////////////////////////////////////////////////////////////////
//
// Global definitions
//
///////////////////////////////////////////////////////////////////////////////
#define USER_INPUT_TASK_NAME        (signed char *)"UserInput"
#define USER_INPUT_TASK_STACK       128
#define USER_INPUT_TASK_PRIORITY    (configMAX_PRIORITIES-3)
#define USER_INPUT_TASK_DELAY_TICKS (10/portTICK_RATE_MS)


///////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
///////////////////////////////////////////////////////////////////////////////
bool UserInputInit();


#endif //__USER_INPUT_H__
