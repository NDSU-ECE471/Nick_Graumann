/*
 * udp.h
 *
 *  Created on: Mar 20, 2013
 *      Author: Nate
 */

#ifndef UDP_H_
#define UDP_H_

#include "stdint.h"
#include "util.h"
#include "socket.h"
#include "w5200.h"


typedef struct
{
	SOCKET 				s;
	uint16_t			Source_Port;
	uint8_t	*			Source_IP;
	uint8_t *			Gateway;
	uint8_t	*		 	Subnet;
	uint8_t *			MAC;

} UDP_CONFIG;

uint8_t udp_socket_init(UDP_CONFIG *config);
uint8_t udp_send(SOCKET s,uint8_t * data_buf,uint16_t length,uint8_t *dest_ip, uint16_t dest_port);

#endif /* UDP_H_ */
