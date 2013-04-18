/*
 * udp.c
 *
 *  Created on: Mar 20, 2013
 *      Author: Nate
 */

#include "udp.h"
#include "stdbool.h"
#include "util.h"
#include "SPI2.h"

uint8_t udp_socket_init(UDP_CONFIG *config)
{
	static bool startup=false;
	if(!startup)
	{
		GPIO_Configuration();
		Reset_W5200();
		WIZ_SPI_Init();
		//void Set_network(uint8_t * ip,uint8_t * gateway,uint8_t * mac, uint8_t *sub)
		Set_network(config->Source_IP, config->Gateway, config->MAC, config->Subnet);
		startup=true;
		while (getSn_SR(config->s) != SOCK_CLOSED);
	}

	if(socket(config->s,Sn_MR_UDP,config->Source_Port,0x00)== 0)    /* reinitialize the socket */
	{
		return 0; //UDP failed
	}
	while (getSn_SR(0) != SOCK_UDP);
	return 1;
}

uint8_t udp_send(SOCKET s,uint8_t * data_buf,uint16_t length,uint8_t *dest_ip, uint16_t dest_port)
{
	while(!sendto(s, data_buf, length, dest_ip, dest_port));
	return 1;
}

