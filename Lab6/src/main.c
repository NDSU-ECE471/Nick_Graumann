#include <stdio.h>
#include "LPC17xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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

typedef struct
{
   uint8_t hours;
   uint8_t minutes;
   uint8_t seconds;
   uint8_t tenMs;
} Time_T;

#define BUTTON_POLL_TICKS        10
#define BUTTON_DEBOUNCE_TICKS    250

#define BUTTON_START_STOP_PORT   0
#define BUTTON_START_STOP_PIN    2

#define BUTTON_RESET_PORT        0
#define BUTTON_RESET_PIN         3

#define BUTTON_STARTSTOP_TASK_STACK_SIZE   128
#define BUTTON_STARTSTOP_TASK_PRIORITY     (tskIDLE_PRIORITY+2)
static portTASK_FUNCTION(ButtonStartStopTask, pvParameters);

#define BUTTON_RESET_TASK_STACK_SIZE   128
#define BUTTON_RESET_TASK_PRIORITY     (tskIDLE_PRIORITY+2)
static portTASK_FUNCTION(ButtonResetTask, pvParameters);

#define TIME_TASK_POLL_TICKS     10
#define TIME_TASK_STACK_SIZE     128
#define TIME_TASK_PRIORITY       (tskIDLE_PRIORITY+3)
static portTASK_FUNCTION(TimeTask, pvParameters);

#define TEXT_COLOR   ST7735_16_BLUE
#define BG_COLOR     ST7735_16_BLACK

#define LCD_TASK_UPDATE_TICKS    50
#define LCD_TASK_STACK_SIZE      128
#define LCD_TASK_PRIORITY        (tskIDLE_PRIORITY+1)
static portTASK_FUNCTION(LcdTask, pvParameters);

#define LCD_EVENT_QUEUE_SIZE     10
xQueueHandle LcdEventQueue = NULL;

typedef enum
{
   LCD_EVENT_TIME,
   LCD_EVENT_BUTTON_START_STOP,
   LCD_EVENT_BUTTON_RESET
} LcdEventType_E;

typedef struct
{
   LcdEventType_E type;
} LcdEvent_T;


typedef enum
{
   STATE_STOPPED,
   STATE_RUNNING
} WatchState_E;

WatchState_E WatchState = STATE_RUNNING;
Time_T time =
{
   .hours = 0,
   .minutes = 0,
   .seconds = 0,
   .tenMs = 0
};

int main(void)
{
   SystemCoreClockUpdate();

   if(xTaskCreate(ButtonStartStopTask,
                  (signed char *)"StartStop",
                  BUTTON_STARTSTOP_TASK_STACK_SIZE,
                  NULL,
                  BUTTON_STARTSTOP_TASK_PRIORITY,
                  NULL) &&

      xTaskCreate(ButtonResetTask,
                        (signed char *)"Reset",
                        BUTTON_RESET_TASK_STACK_SIZE,
                        NULL,
                        BUTTON_RESET_TASK_PRIORITY,
                        NULL) &&

      xTaskCreate(TimeTask,
                  (signed char *)"TimeTask",
                  TIME_TASK_STACK_SIZE,
                  NULL,
                  TIME_TASK_PRIORITY,
                  NULL) &&

      xTaskCreate(LcdTask,
                  (signed char *)"LcdTask",
                  LCD_TASK_STACK_SIZE,
                  NULL,
                  LCD_TASK_PRIORITY,
                  NULL) &&

      (LcdEventQueue = xQueueCreate(LCD_EVENT_QUEUE_SIZE, sizeof(LcdEvent_T))))
   {
      vTaskStartScheduler();
   }

   // Error trap
   while(1);
}


void vApplicationStackOverflowHook()
{
   while(1);
}


void vApplicationMallocFailedHook()
{
   while(1);
}


static portTASK_FUNCTION(ButtonStartStopTask, pvParameters)
{
   portTickType nextWakeTime = xTaskGetTickCount();
   LcdEvent_T event;
   event.type = LCD_EVENT_BUTTON_START_STOP;

   while(1)
   {
      if(!(LPC_GPIO[BUTTON_START_STOP_PORT]->FIOPIN & (1<<BUTTON_START_STOP_PIN)))
      {
         xQueueSendToBack(LcdEventQueue, &event, 0);
         vTaskDelay(BUTTON_DEBOUNCE_TICKS);
      }

      vTaskDelayUntil(&nextWakeTime, BUTTON_POLL_TICKS);
   }
}


static portTASK_FUNCTION(ButtonResetTask, pvParameters)
{
   portTickType nextWakeTime = xTaskGetTickCount();
   LcdEvent_T event;
   event.type = LCD_EVENT_BUTTON_RESET;

   while(1)
   {
      if(!(LPC_GPIO[BUTTON_RESET_PORT]->FIOPIN & (1<<BUTTON_RESET_PIN)))
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
      taskENTER_CRITICAL();
      if(100 == ++time.tenMs)
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
      taskEXIT_CRITICAL();

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

   portTickType lastLcdUpdate = xTaskGetTickCount();
   char strBuf[12];
   LcdEvent_T event;

   // Strange timing requirements in this lab...
   // LCD updates every 50ms but yet the task needs to receive data every 10ms.
   // In order to meet both, receive from the queue constantly (which should be at minimum every 10ms), and
   // manually check the tick count to see if an update is needed
   while(xQueueReceive(LcdEventQueue, &event, portMAX_DELAY))
   {
      if(xTaskGetTickCount() - lastLcdUpdate > LCD_TASK_UPDATE_TICKS)
      {
         snprintf(strBuf, 12, "%02u:%02u:%02u:%02u", time.hours,
                                                     time.minutes,
                                                     time.seconds,
                                                     time.tenMs);

         drawString(10, 10, strBuf);
         lastLcdUpdate = xTaskGetTickCount();
      }
   }
}
