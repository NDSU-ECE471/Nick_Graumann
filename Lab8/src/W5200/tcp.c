/*
 * tcp.c
 *
 *  Created on: Apr 7, 2013
 *      Author: Nate
 */

#include "tcp.h"
#include "web_page.h"
#include "string.h"

#define tcp_data_buffer_size 	2000
static uint8_t tcp_data_buffer[tcp_data_buffer_size];
static uint16_t bytesReceived = 0;

uint8_t tcp_socket_init(TCP_CONFIG *config)
{

	static bool startup=false;
	if(!startup)
	{
		GPIO_Configuration();
		Reset_W5200();
		WIZ_SPI_Init();
		Set_network(config->Source_IP, config->Gateway, config->MAC, config->Subnet);
		startup=true;
		while (getSn_SR(config->s) != SOCK_CLOSED);
	}

	if(socket(config->s,Sn_MR_TCP,config->Source_Port,0x00)== 0)    /* reinitialize the socket */
	{
		return 0; //TCP socket failed
	}

	while (getSn_SR(config->s) != SOCK_INIT); // Wait for socket to open

	IINCHIP_WRITE(Sn_CR(config->s), Sn_CR_LISTEN); // Start listening on port specified
	while (IINCHIP_READ(Sn_CR(config->s))); // Check status
	while (getSn_SR(config->s) != SOCK_LISTEN); // Wait for listen
	return 1;
}

uint8_t check_for_connections(TCP_CONFIG *config)
{
	if(!(getSn_SR(config->s) != SOCK_ESTABLISHED))
	{
	while ((bytesReceived = getSn_RX_RSR(config->s)) == 0); // wait for connection
	return 1;
	}
	return 0;
}

uint8_t process_request(TCP_CONFIG *config)
{
	uint16_t bytes_to_send =0;
	uint8_t rets=0;

	if(bytesReceived>tcp_data_buffer_size)
		return 0; // Content overflow

	// Assuming GIT request, read and parse if you with to use urls for anything
	recv_data_processing(config->s, tcp_data_buffer, bytesReceived); // Data from the GIT request

	bytes_to_send = strlen((char *)website);
	rets = send(config->s, website,bytes_to_send , 0); // Send Response with html
	bytesReceived=0;

	IINCHIP_WRITE(Sn_CR(config->s), Sn_CR_DISCON); 	// Shut down connection
	while (IINCHIP_READ(Sn_CR(config->s)));			//
	IINCHIP_WRITE(Sn_CR(config->s), Sn_CR_CLOSE);	//
	while (IINCHIP_READ(Sn_CR(config->s)));			//
	IINCHIP_WRITE(Sn_IR(config->s), 0xFF);			//

	return rets;
}

