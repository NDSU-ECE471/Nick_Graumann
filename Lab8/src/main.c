//Lab7 Drone / RTOS
//Base written Nathan Zimmerman

#include <string.h>

#include "lpc17xx.h" // processor includes
#include "stdint.h" // includes for uint8_t nstuff
#include "W5200/tcp.h"
#include "LCD/lcd.h" // includes for ldc
#include "DRIVERS/adc.h"
#include "DEV/ece471.h"

#include "web_page.h"

static char buffer[40];

static TCP_CONFIG tcp_setup;
uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};//MAC Address
uint8 IP[4] = {192, 168, 1, 23};//IP Address
uint8 GateWay[4] = {192, 168, 1, 1};//Gateway Address
uint8 SubNet[4] = {255, 255, 255, 0};//SubnetMask Address
uint8_t Dest_IP[4] = {192, 168, 1, 103}; //DST_IP Address


uint8_t get_command();
float adc_calc = 0;
bool startup =false;


#define TCP_DATA_BUFFER_SIZE  2048
static char tcp_data_buffer[TCP_DATA_BUFFER_SIZE];

#define HTTP_BUFFER_SIZE      1024
static char http_buffer[HTTP_BUFFER_SIZE];

uint8_t get_request_str(const char *request, size_t request_size, char *buffer, size_t buf_size);
void handle_request(const char *req);

int main(void)
{  

	delay_ms(100);
	led_setup();
	adc_init();
	LCD_SETUP(buffer);

	tcp_setup.Gateway = GateWay;//Gateway Address
	tcp_setup.MAC = MAC;
	tcp_setup.Source_IP = IP;//IP Address
	tcp_setup.Source_Port= 80; // Web Port
	tcp_setup.Subnet= SubNet;//SubnetMask Address
	tcp_setup.s= 0;
	tcp_socket_init(&tcp_setup);

	LPC_GPIO1->FIODIR |= (1<<19);
	LPC_GPIO1->FIOSET |= (1<<19);

	while(1)
	{
		if(tcp_check_for_connections(&tcp_setup)) // Wait for connection)
		{
			tcp_receive(&tcp_setup, tcp_data_buffer, TCP_DATA_BUFFER_SIZE);

			if(get_request_str(tcp_data_buffer, TCP_DATA_BUFFER_SIZE, http_buffer, HTTP_BUFFER_SIZE))
			{
			   handle_request(http_buffer);
			}

         tcp_send(&tcp_setup, website, strlen(website));

			tcp_socket_init(&tcp_setup);
		}

	}

}


uint8_t get_request_str(const char *request, size_t request_size, char *buffer, size_t buf_size)
{
   uint8_t ret = 1;

   const char *start_pos = strstr(request, "GET /");
   if(!start_pos)
   {
      ret = 0;
   }

   const char *end_pos;
   if(ret)
   {
      // Go past the GET / string
      start_pos += 5;

      end_pos = strstr(request, " HTTP/1.1");
      if(!end_pos || end_pos <= start_pos)
      {
         ret = 0;
      }
   }

   size_t len;
   if(ret)
   {
      len = end_pos - start_pos;
      if(len > buf_size)
      {
         ret = 0;
      }
   }

   if(ret)
   {
      strncpy(buffer, start_pos, len);
      buffer[len] = '\0';
   }

   return ret;
}


void handle_request(const char *req)
{
   if(strcmp(req, "led_on") == 0)
   {
      LPC_GPIO1->FIOCLR |= (1<<19);
   }
   else if(strcmp(req, "led_off") == 0)
   {
      LPC_GPIO1->FIOSET |= (1<<19);
   }
}

