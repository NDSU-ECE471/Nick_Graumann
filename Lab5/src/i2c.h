#ifndef __I2C_H__
#define __I2C_H__

#include <stdbool.h>
#include <stdint.h>

#define I2C_SDA_PIN (1<<19)
#define I2C_SCL_PIN (1<<24)

void I2cInit();
bool I2cRead(uint8_t address, uint8_t *buffer);
void I2cWrite(uint8_t address, uint8_t *data, uint8_t size);

#endif //__I2C_H__
