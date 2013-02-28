#ifndef __I2C_H__
#define __I2C_H__

#include <stdbool.h>
#include <stdint.h>

void I2cInit();
bool I2cRx(uint8_t address, uint8_t reg, uint8_t *buffer);
bool I2cTx(uint8_t address, const uint8_t *data, uint8_t size);

#endif //__I2C_H__
