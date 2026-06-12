#ifndef I2C_H
#define I2C_H

#include <stddef.h>

typedef enum
{
    I2C_OK,
    I2C_INVALID_ARGUMENT,
    I2C_NACK,
    I2C_BUS_ERROR,
    I2C_ARBITRATION_LOST,
} I2C_Status;

void I2C_Init(void);

I2C_Status I2C_Probe(uint8_t address);

I2C_Status I2C_WriteByte(uint8_t address, uint8_t byte);

I2C_Status I2C_Write(uint8_t address, const uint8_t *buffer, uint8_t length);

I2C_Status I2C_Read(uint8_t address, uint8_t *buffer, uint8_t length);

#endif /* I2C_H */
