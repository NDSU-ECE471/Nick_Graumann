#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader/AdcReader.h"
#include "Drivers/GPIO/GPIO_Facade.h"
#include "ScopeDisplay/ScopeDisplay.h"
#include "UserInput.h"


///////////////////////////////////////////////////////////////////////////////
//
// Enumerations
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
   BUTTON_START = 0,
   BUTTON_TIMEBASE_INC,
   BUTTON_TIMEBASE_DEC
} UI_Button_E;


typedef enum
{
   BUTTON_NOT_PRESSED,
   BUTTON_PRESSED
} ButtonState_E;



///////////////////////////////////////////////////////////////////////////////
//
// Button GPIO definitions
//
///////////////////////////////////////////////////////////////////////////////
#define BUTTON_GPIO_PORT_NUM  0


const uint32_t GPIO_BUTTON_BITS[] =
{
   [BUTTON_START]        = (1<<2),
   [BUTTON_TIMEBASE_INC] = (1<<4),
   [BUTTON_TIMEBASE_DEC] = (1<<5),
};


static uint8_t buttonStates = 0;


static portTASK_FUNCTION(UserInputTask, pvParameters);


///////////////////////////////////////////////////////////////////////////////
//
// See UserInput.h
//
///////////////////////////////////////////////////////////////////////////////
bool UserInputInit()
{
   ScopeDisplayEvent_T displayEvent;
   displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_VDIV;
   displayEvent.VdivData.value = 400;
   displayEvent.VdivData.units = VDIV_mV;
   ScopeDisplayQueueEvent(&displayEvent);

   bool retVal = xTaskCreate(UserInputTask,
                             USER_INPUT_TASK_NAME,
                             USER_INPUT_TASK_STACK,
                             NULL,
                             USER_INPUT_TASK_PRIORITY,
                             NULL);

   return retVal;
}


///////////////////////////////////////////////////////////////////////////////
//
// Queries a button given its 32-bit GPIO port state and past state
//
///////////////////////////////////////////////////////////////////////////////
static ButtonState_E QueryButton(uint32_t portState, UI_Button_E button)
{
   ButtonState_E ret = BUTTON_NOT_PRESSED;

   // Buttons are active low, check if pressed
   if(!(portState & GPIO_BUTTON_BITS[button]))
   {
      // Button states are active high. This acts as a "debounce" so that we
      // only say the button is pressed once until we detect that it is no longer pressed.
      if(!(buttonStates & GPIO_BUTTON_BITS[button]))
      {
         buttonStates |= GPIO_BUTTON_BITS[button];
         ret = BUTTON_PRESSED;
      }
   }
   // If not pressed but the state says we are, clear it
   else if(buttonStates & GPIO_BUTTON_BITS[button])
   {
      buttonStates &= ~GPIO_BUTTON_BITS[button];
   }

   return ret;
}


///////////////////////////////////////////////////////////////////////////////
//
// FreeRTOS task
//
///////////////////////////////////////////////////////////////////////////////
static portTASK_FUNCTION(UserInputTask, pvParameters)
{
   (void)pvParameters;
   portTickType lastWakeTime = xTaskGetTickCount();

   AdcReaderCommand_T adcCommand;
   ScopeDisplayEvent_T scopeDispCmd;
   uint32_t gpioPortState;

   while(1)
   {
      GPIO_PortGet(BUTTON_GPIO_PORT_NUM, &gpioPortState);

      if(BUTTON_PRESSED == QueryButton(gpioPortState, BUTTON_START))
      {
         adcCommand.type = ADC_READER_READ_BURST;
         AdcReaderQueueEvent(&adcCommand);
      }

      if(BUTTON_PRESSED == QueryButton(gpioPortState, BUTTON_TIMEBASE_INC))
      {
         scopeDispCmd.type = SCOPE_DISPLAY_EVENT_TIMEBASE_DEC;
         ScopeDisplayQueueEvent(&scopeDispCmd);

         adcCommand.type = ADC_READER_INC_SAMPLERATE;
         AdcReaderQueueEvent(&adcCommand);
      }

      if(BUTTON_PRESSED == QueryButton(gpioPortState, BUTTON_TIMEBASE_DEC))
      {
         scopeDispCmd.type = SCOPE_DISPLAY_EVENT_TIMEBASE_INC;
         ScopeDisplayQueueEvent(&scopeDispCmd);

         adcCommand.type = ADC_READER_DEC_SAMPLERATE;
         AdcReaderQueueEvent(&adcCommand);
      }

      vTaskDelayUntil(&lastWakeTime, USER_INPUT_TASK_DELAY_TICKS);
   }
}

