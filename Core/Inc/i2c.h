/**
 * @file i2c.h
 * @brief I2C driver interface for STM32 (blocking mode).
 *
 * This module provides basic blocking I2C communication primitives,
 * including device probing, byte-level write, multi-byte write,
 * and multi-byte read operations.
 */
#ifndef I2C_H
#define I2C_H

#include <stddef.h>

/**
 * @enum I2C_Status
 * @brief Status codes returned by I2C operations.
 */
typedef enum
{
    I2C_OK,                /**< Operation completed successfully */
    I2C_INVALID_ARGUMENT,  /**< Invalid parameter passed */
    I2C_NACK,              /**< No acknowledgment from slave device */
    I2C_BUS_ERROR,         /**< Bus error detected (misplaced START/STOP, etc.) */
    I2C_ARBITRATION_LOST,  /**< Arbitration lost in multi-master environment */
} I2C_Status;

/**
 * @brief Initialize the I2C peripheral.
 *
 * Configures GPIO pins, clock source, timing parameters, and enables
 * the I2C peripheral. Must be called before any I2C operation.
 */
void I2C_Init(void);

/**
 * @brief Probe an I2C device by address.
 *
 * Sends a write transaction to the specified 7-bit address and checks
 * whether the device responds with an ACK.
 *
 * @param address 7-bit I2C slave address
 * @return I2C_OK if device acknowledged, otherwise error status
 */
I2C_Status I2C_Probe(uint8_t address);

/**
 * @brief Write a single byte to an I2C device.
 *
 * @param address 7-bit I2C slave address
 * @param byte    Data byte to transmit
 * @return I2C_OK on success, error status otherwise
 */
I2C_Status I2C_WriteByte(uint8_t address, uint8_t byte);

/**
 * @brief Write multiple bytes to an I2C device.
 *
 * @param address 7-bit I2C slave address
 * @param buffer  Pointer to data buffer
 * @param length  Number of bytes to transmit
 * @return I2C_OK on success, error status otherwise
 */
I2C_Status I2C_Write(uint8_t address, const uint8_t *buffer, uint8_t length);

/**
 * @brief Read multiple bytes from an I2C device.
 *
 * @param address 7-bit I2C slave address
 * @param buffer  Pointer to buffer where received data will be stored
 * @param length  Number of bytes to read
 * @return I2C_OK on success, error status otherwise
 */
I2C_Status I2C_Read(uint8_t address, uint8_t *buffer, uint8_t length);

#endif /* I2C_H */
