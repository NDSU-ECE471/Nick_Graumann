/*
*
@file		util.h
@brief	
*/

#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include "Types.h"

void Set_network(uint8_t * ip,uint8_t * gateway,uint8_t * mac, uint8_t *sub);
void Reset_W5200(void);

void Delay_us(uint8 time_us);
void Delay_ms(uint32_t time_ms);

uint32_t time_return(void);

#endif
