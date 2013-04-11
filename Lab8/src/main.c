//Lab7 Drone / RTOS
//Base written Nathan Zimmerman

#include "lpc17xx.h" // processor includes
#include "stdint.h" // includes for uint8_t nstuff
#include "W5200/tcp.h"
#include "LCD/lcd.h" // includes for ldc
#include "DRIVERS/adc.h"
#include "DEV/ece471.h"

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

	while(1)
	{
		if(check_for_connections(&tcp_setup)) // Wait for connection)
		{
			process_request(&tcp_setup);
			tcp_socket_init(&tcp_setup);
		}

	}

}





