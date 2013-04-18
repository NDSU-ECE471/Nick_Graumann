#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lpc17xx.h" // processor includes
#include "W5200/udp.h" //ethernet driver includes
#include "LCD/lcd.h" // includes for ldc
#include "DEV/ece471.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static char buffer[40];
static UDP_CONFIG udp_setup;

uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};//MAC Address
uint8 IP[4] = {192, 168, 1, 23};//IP Address
uint8 GateWay[4] = {192, 168, 1, 1};//Gateway Address
uint8 SubNet[4] = {255, 255, 255, 0};//SubnetMask Address
uint8_t Dest_IP[4] = {192, 168, 1, 1}; //DST_IP Address
uint16_t DEST_PORT = 5556;

static portTASK_FUNCTION(InitTask, pvParameters);
static portTASK_FUNCTION(InputTask, pvParameters);
static portTASK_FUNCTION(UdpTask, pvParameters);
static portTASK_FUNCTION(WatchdogTask, pvParameters);

xQueueHandle udpQueue = NULL;

typedef struct
{
   uint8_t command[64];
} UdpData_T;

static const UdpData_T NAVDATA_PACKET =
{
   .command = ""
};

static const char *INIT_COMMAND_TEMPLATES[] =
{
   "AT*CONFIG=%u,\"general:navdata_demo\",\"FALSE\"\r",
   "AT*ZAP=%u,0\r",
   //video here?
   "AT*PMODE=%u,2\r",
   "AT*MISC=%u,2,20,2000,3000\r",
   "AT*CONFIG=%u,\"control:outdoor\",\"FALSE\"\r",
   "AT*CONFIG=%u,\"control:flight_without_shell\",\"FALSE\"\r",
   "AT*CONFIG=%u,\"control:altitude_max\",\"5000\"\r",
   "AT*CONFIG=%u,\"control:altitude_min\",\"500\"\r",
   //video bitrate
   "AT*ZAP=%u,0\r",
   //video channel
};
static size_t INIT_COMMAND_TEMPLATES_SIZE = sizeof(INIT_COMMAND_TEMPLATES)/sizeof(INIT_COMMAND_TEMPLATES[0]);

static const char *TAKEOFF_PACKET_TEMPLATE = "AT*REF=%u,290718208\r";
static const char *LAND_PACKET_TEMPLATE = "AT*REF=%u,290717696\r";
static const char *WATCHDOG_PACKET_TEMPLATE = "AT*COMWDG=%u\r";

void SendPacket(const char *packetTemplate);

#define BUTTON_TAKEOFF_BIT 0x01
#define BUTTON_LAND_BIT    0x02

uint8_t buttonStates = 0;


uint16_t commandSeq = 1;
xQueueHandle commandSeqMutex = NULL;

int main(void)
{
	udp_setup.Gateway = GateWay;//Gateway Address
	udp_setup.MAC = MAC;
	udp_setup.Source_IP = IP;//IP Address
	udp_setup.Source_Port= 5556;
	udp_setup.Subnet= SubNet;//SubnetMask Address
	udp_setup.s= 0;

	lcd_init();
   fillScreen(ST7735_16_GREEN);
   setColor16(ST7735_16_BLACK);
   setBackgroundColor16(ST7735_16_GREEN);
   drawString(10, 10, "P0.2: Takeoff");
   drawString(10, 20, "P0.3: Land");

	udp_socket_init(&udp_setup);

	udpQueue = xQueueCreate(8, sizeof(UdpData_T));
	commandSeqMutex = xSemaphoreCreateMutex();

	xTaskCreate(InitTask, "InitTask", 256, NULL, tskIDLE_PRIORITY+4, NULL);
	xTaskCreate(InputTask, "InputTask", 256, NULL, tskIDLE_PRIORITY+1, NULL);
	xTaskCreate(UdpTask, "UdpTask", 256, NULL, tskIDLE_PRIORITY+2, NULL);
	xTaskCreate(WatchdogTask, "WatchdogTask", 256, NULL, tskIDLE_PRIORITY+3, NULL);

	vTaskStartScheduler();
}


void SendPacket(const char *packetTemplate)
{
   UdpData_T dataPacket;

   if(xSemaphoreTake(commandSeqMutex, 10))
   {
      snprintf(dataPacket.command, sizeof(dataPacket.command), packetTemplate, commandSeq++);
      xSemaphoreGive(commandSeqMutex);

      xQueueSendToBack(udpQueue, &dataPacket, 10);
   }
}


static portTASK_FUNCTION(InitTask, pvParameters)
{
   for(size_t i=0; i<INIT_COMMAND_TEMPLATES_SIZE; i++)
   {
      SendPacket(INIT_COMMAND_TEMPLATES[i]);
   }

   vTaskDelete(NULL);
}


static portTASK_FUNCTION(InputTask, pvParameters)
{
   UdpData_T udpData;

   portTickType prevWakeTime = xTaskGetTickCount();

   while(1)
   {
      if(!(LPC_GPIO0->FIOPIN & (1<<2)))
      {
         if(!(buttonStates & BUTTON_TAKEOFF_BIT))
         {
            buttonStates |= BUTTON_TAKEOFF_BIT;
            SendPacket(TAKEOFF_PACKET_TEMPLATE);
         }
      }
      else if(buttonStates & BUTTON_TAKEOFF_BIT)
      {
         buttonStates &= ~BUTTON_TAKEOFF_BIT;
      }

      if(!(LPC_GPIO0->FIOPIN & (1<<3)))
      {
         if(!(buttonStates & BUTTON_LAND_BIT))
         {
            buttonStates |= BUTTON_LAND_BIT;
            SendPacket(LAND_PACKET_TEMPLATE);
         }
      }
      else if(buttonStates & BUTTON_LAND_BIT)
      {
         buttonStates &= ~BUTTON_LAND_BIT;
      }

      vTaskDelayUntil(&prevWakeTime, 10);
   }
}


static portTASK_FUNCTION(UdpTask, pvParameters)
{
   UdpData_T udpPacket;

   while(1)
   {
      if(xQueueReceive(udpQueue, &udpPacket, portMAX_DELAY))
      {
         udp_send(0, udpPacket.command, strlen(udpPacket.command), Dest_IP, DEST_PORT);
      }
   }
}


static portTASK_FUNCTION(WatchdogTask, pvParameters)
{
   UdpData_T watchdogPacket;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      SendPacket(WATCHDOG_PACKET_TEMPLATE);

      vTaskDelayUntil(&lastWakeTime, 250);
   }
}


void vApplicationStackOverflowHook()
{
   while(1);
}


void vApplicationMallocFailedHook()
{
   while(1);
}


void vApplicationIdleHook()
{
   __WFI();
}

