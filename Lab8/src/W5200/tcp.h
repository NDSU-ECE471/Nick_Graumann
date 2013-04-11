/*
 * tcp.h
 *
 *  Created on: Apr 7, 2013
 *      Author: Nate
 */

#ifndef TCP_H_
#define TCP_H_

#include "stdint.h"
#include "util.h"
#include "w5200.h"
#include "stdbool.h"
#include "SPI2.h"
#include "socket.h"

typedef struct
{
	SOCKET 				s;
	uint16_t			Source_Port;
	uint8_t	*			Source_IP;
	uint8_t *			Gateway;
	uint8_t	*		 	Subnet;
	uint8_t *			MAC;

} TCP_CONFIG;


uint8_t tcp_socket_init(TCP_CONFIG *config);
uint8_t process_request();
uint8_t check_for_connections(TCP_CONFIG *config);

#endif /* TCP_H_ */
