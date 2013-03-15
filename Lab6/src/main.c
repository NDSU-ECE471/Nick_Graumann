#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "lcd/lcd.h"

// Gah, I absolutely hate not having these. So I made them :)
LPC_GPIO_TypeDef *LPC_GPIO[] =
{
   LPC_GPIO0,
   LPC_GPIO1,
   LPC_GPIO2,
   LPC_GPIO3,
   LPC_GPIO4
};

static portTASK_FUNCTION(ButtonTask, pvParameters);

#define BUTTON_POLL_TICKS        10
#define BUTTON_DEBOUNCE_TICKS    250

#define BUTTON_START_STOP_PORT   0
#define BUTTON_START_STOP_PIN    2

#define BUTTON_RESET_PORT        0
#define BUTTON_RESET_PIN         3

#define BUTTON_TASK_STACK_SIZE  128
#define BUTTON_TASK_PRIORITY    (tskIDLE_PRIORITY+2UL)

#define TIME_TASK_POLL_TICKS              10
#define TIME_TASK_STACK_SIZE              128
#define TIME_TASK_PRIORITY                (tskIDLE_PRIORITY+3UL)

static portTASK_FUNCTION(TimeTask, pvParameters);


#define TEXT_COLOR               ST7735_16_BLUE
#define BG_COLOR                 ST7735_16_BLACK

#define LCD_TASK_UPDATE_TICKS    50
#define LCD_TASK_STACK_SIZE      128
#define LCD_TASK_PRIORITY        (tskIDLE_PRIORITY+1)

static portTASK_FUNCTION(LcdTask, pvParameters);

#define LCD_EVENT_QUEUE_SIZE     10
xQueueHandle LcdEventQueue = NULL;


#define LED_TASK_DELAY_TICKS     50
#define LED_TASK_STACK_SIZE      128
#define LED_TASK_PRIORITY        (tskIDLE_PRIORITY+1UL)

#define NUM_LEDS                 8
#define LED_PORT_NUM             1
#define LED_PIN_NUM_BASE         19
#define LED_PIN_NUM_HIGH         26
#define LED_PINS_ALL             ((0xFF<<19))
#define LED_ENABLE_BUTTON_PORT   0
#define LED_ENABLE_BUTTON_PIN    5

static portTASK_FUNCTION(LedTask, pvParameters);


typedef enum
{
   LCD_EVENT_TIME,
   LCD_EVENT_BUTTON_START_STOP,
   LCD_EVENT_BUTTON_RESET,
   LCD_EVENT_LEDS
} LcdEventType_E;

typedef struct
{
   LcdEventType_E type;
} LcdEvent_T;


typedef struct
{
   LcdEventType_E eventType;
   uint32_t gpioPort;
   uint32_t gpioPin;
} ButtonTaskData_T;


typedef struct
{
   uint8_t hours;
   uint8_t minutes;
   uint8_t seconds;
   uint8_t tenMs;
} Time_T;

#define TIME_MUTEX_TICK_TIMEOUT  5
xSemaphoreHandle TimeMutex = NULL;
Time_T time;


typedef enum
{
   STATE_STOPPED,
   STATE_RUNNING
} WatchState_E;

WatchState_E WatchState = STATE_STOPPED;
bool enableLeds = true;


bool Initialize();


int main(void)
{
   SystemCoreClockUpdate();

   static ButtonTaskData_T startStopButtonData;
   startStopButtonData.eventType = LCD_EVENT_BUTTON_START_STOP;
   startStopButtonData.gpioPort = BUTTON_START_STOP_PORT;
   startStopButtonData.gpioPin = BUTTON_START_STOP_PIN;

   xTaskCreate(ButtonTask,
              (signed char *)"BtnStartStop",
              BUTTON_TASK_STACK_SIZE,
              &startStopButtonData,
              BUTTON_TASK_PRIORITY,
              NULL);

   static ButtonTaskData_T resetButtonData;
   resetButtonData.eventType = LCD_EVENT_BUTTON_RESET;
   resetButtonData.gpioPort = BUTTON_RESET_PORT;
   resetButtonData.gpioPin = BUTTON_RESET_PIN;

   xTaskCreate(ButtonTask,
               (signed char *)"BtnReset",
               BUTTON_TASK_STACK_SIZE,
               &resetButtonData,
               BUTTON_TASK_PRIORITY,
               NULL);

   static ButtonTaskData_T ledButtonData;
   ledButtonData.eventType = LCD_EVENT_LEDS;
   ledButtonData.gpioPort = LED_ENABLE_BUTTON_PORT;
   ledButtonData.gpioPin = LED_ENABLE_BUTTON_PIN;

   xTaskCreate(ButtonTask,
               (signed char *)"BtnLeds",
               BUTTON_TASK_STACK_SIZE,
               &ledButtonData,
               BUTTON_TASK_PRIORITY,
               NULL);

   xTaskCreate(TimeTask,
               (signed char *)"TimeTask",
               TIME_TASK_STACK_SIZE,
               NULL,
               TIME_TASK_PRIORITY,
               NULL);

   xTaskCreate(LcdTask,
               (signed char *)"LcdTask",
               LCD_TASK_STACK_SIZE,
               NULL,
               LCD_TASK_PRIORITY,
               NULL);

   LcdEventQueue = xQueueCreate(LCD_EVENT_QUEUE_SIZE, sizeof(LcdEvent_T));

   xTaskCreate(LedTask,
               (signed char *)"LedTask",
               LED_TASK_STACK_SIZE,
               NULL,
               LED_TASK_PRIORITY,
               NULL);

   // LEDs outputs
   LPC_GPIO[LED_PORT_NUM]->FIODIR |= LED_PINS_ALL;
   // LEDS off (active low)
   LPC_GPIO[LED_PORT_NUM]->FIOSET = LED_PINS_ALL;

   TimeMutex = xSemaphoreCreateMutex();
   memset(&time, 0, sizeof(time));

   vTaskStartScheduler();

   // Error trap
   while(1);
}


// VERY good idea to turn these on in FreeRTOSconfig.h
void vApplicationStackOverflowHook()
{
   while(1);
}


void vApplicationMallocFailedHook()
{
   while(1);
}


static portTASK_FUNCTION(ButtonTask, pvParameters)
{
   if(!pvParameters)
   {
      vTaskDelete(NULL);
   }

   ButtonTaskData_T *taskData = (ButtonTaskData_T *)pvParameters;

   portTickType nextWakeTime = xTaskGetTickCount();
   LcdEvent_T event;
   event.type = taskData->eventType;

   while(1)
   {
      if(!(LPC_GPIO[taskData->gpioPort]->FIOPIN & (1<<taskData->gpioPin)))
      {
         xQueueSendToBack(LcdEventQueue, &event, 0);
         vTaskDelay(BUTTON_DEBOUNCE_TICKS);
      }

      vTaskDelayUntil(&nextWakeTime, BUTTON_POLL_TICKS);
   }
}


static portTASK_FUNCTION(TimeTask, pvParameters)
{
   portTickType nextWakeTime = xTaskGetTickCount();
   LcdEvent_T event;
   event.type = LCD_EVENT_TIME;

   while(1)
   {
      // todo: need to add a timeout here?
      if(xSemaphoreTake(TimeMutex, TIME_MUTEX_TICK_TIMEOUT))
      {
         if(WatchState == STATE_RUNNING && 100 == ++time.tenMs)
         {
            time.tenMs = 0;

            if(60 == ++time.seconds)
            {
               time.seconds = 0;

               if(60 == ++time.minutes)
               {
                  time.minutes = 0;
                  time.hours++;
               }
            }
         }

         xSemaphoreGive(TimeMutex);
      }

      xQueueSendToBack(LcdEventQueue, &event, 0);

      vTaskDelayUntil(&nextWakeTime, TIME_TASK_POLL_TICKS);
   }
}


static portTASK_FUNCTION(LcdTask, pvParameters)
{
   lcd_init();
   fillScreen(BG_COLOR);
   setColor16(TEXT_COLOR);
   setBackgroundColor16(BG_COLOR);
   drawString(10, 10, "-- Nick's Stopwatch --");
   drawString(10, 90, "P0.2: Start/stop");
   drawString(10, 100, "P0.3: Reset");
   drawString(10, 110, "P0.5: Toggle LEDs");

   portTickType lastLcdUpdate = xTaskGetTickCount();
   char strBuf[12];
   LcdEvent_T event;

   // LCD updates every 50ms but yet the task needs to receive data every 10ms.
   // In order to meet both, receive from the queue constantly (which should be at minimum every 10ms), and
   // manually check the tick count to see if an update is needed
   while(xQueueReceive(LcdEventQueue, &event, portMAX_DELAY))
   {
      switch(event.type)
      {
      case LCD_EVENT_TIME:
         if(xTaskGetTickCount() - lastLcdUpdate > LCD_TASK_UPDATE_TICKS &&
            xSemaphoreTake(TimeMutex, TIME_MUTEX_TICK_TIMEOUT))
         {
            snprintf(strBuf, 12, "%02u:%02u:%02u:%02u", time.hours,
                                                        time.minutes,
                                                        time.seconds,
                                                        time.tenMs);

            xSemaphoreGive(TimeMutex);

            drawString(10, 30, strBuf);
            lastLcdUpdate = xTaskGetTickCount();
         }
         break;

      case LCD_EVENT_BUTTON_START_STOP:
         if(WatchState == STATE_STOPPED)
         {
            WatchState = STATE_RUNNING;
         }
         else
         {
            WatchState = STATE_STOPPED;
         }
         break;

      case LCD_EVENT_BUTTON_RESET:
         if(xSemaphoreTake(TimeMutex, TIME_MUTEX_TICK_TIMEOUT))
         {
            time.hours = 0;
            time.minutes = 0;
            time.seconds = 0;
            time.tenMs = 0;

            xSemaphoreGive(TimeMutex);
         }
         break;

      case LCD_EVENT_LEDS:
         enableLeds = !enableLeds;
         break;
      }
   }
}


static portTASK_FUNCTION(LedTask, pvParameters)
{
   portTickType nextWakeTime = xTaskGetTickCount();
   uint8_t ledPos = 0;
   int8_t inc = 1;

   while(1)
   {
      if(enableLeds)
      {
         LPC_GPIO[LED_PORT_NUM]->FIOSET |= 1<<(LED_PIN_NUM_BASE + ledPos);

         ledPos += inc;
         if(ledPos == 0 || ledPos == 7)
         {
            inc = -inc;
         }

         LPC_GPIO[LED_PORT_NUM]->FIOCLR |= 1<<(LED_PIN_NUM_BASE + ledPos);
      }
      else
      {
         LPC_GPIO[LED_PORT_NUM]->FIOSET = LED_PINS_ALL;
      }

      vTaskDelayUntil(&nextWakeTime, LED_TASK_DELAY_TICKS);
   }
}
