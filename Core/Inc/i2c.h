/**
 * @file i2c.h
 * @brief I²C driver interface for STM32 (blocking mode).
 *
 * This module provides basic blocking I²C communication primitives,
 * including device probing, byte-level write, multi-byte write,
 * and multi-byte read operations.
 */
#ifndef I2C_H
#define I2C_H

#include <stddef.h>
#include <stdint.h>

/**
 * @enum I2C_Status
 * @brief Status codes returned by I²C operations.
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
 * @brief Initialize the I²C peripheral.
 *
 * Configures GPIO pins, clock source, timing parameters, and enables
 * the I²C peripheral. Must be called before any I²C operation.
 */
void I2C_Init(void);

/**
 * @brief Read multiple bytes from an I²C device.
 *
 * @param address 7-bit I²C slave address
 * @param buffer  Pointer to buffer where received data will be stored
 * @param length  Number of bytes to read
 * @return I2C_OK on success, error status otherwise
 */
I2C_Status I2C_Read(uint8_t address, uint8_t *buffer, uint8_t length);

/**
 * @brief Write multiple bytes to an I²C device.
 *
 * @param address 7-bit I²C slave address
 * @param buffer  Pointer to data buffer
 * @param length  Number of bytes to transmit
 * @return I2C_OK on success, error status otherwise
 */
I2C_Status I2C_Write(uint8_t address, const uint8_t *buffer, uint8_t length);

/**
 * @brief A buffer used as part of a vectored I²C read operation.
 *
 * Each buffer represents a contiguous memory region that will be filled
 * with data received as part of a single I²C transaction.
 */
typedef struct {
    /**
     * @brief Pointer to the memory where received data will be stored.
     */
    uint8_t *data;

    /**
     * @brief Number of bytes to receive into this buffer.
     */
    size_t length;
} I2C_RxBuffer;

/**
 * @brief Read multiple buffers from an I²C device in a single transaction.
 *
 * This function performs a sequential I²C read from the specified slave
 * address and fills multiple buffers in order.
 *
 * The buffers are treated as a continuous byte stream. There are no
 * repeated START conditions between buffers.
 *
 * Internally, the transfer may be split into multiple hardware phases
 * using the STM32 RELOAD mechanism if the total transfer exceeds 255 bytes.
 *
 * @param address 7-bit I²C slave address.
 * @param buffers Array of buffers to fill with received data.
 *                The data is written sequentially across all buffers.
 * @param count   Number of buffers in the array.
 *
 * @return I2C_OK on success.
 * @return I2C_INVALID_ARGUMENT if buffers is NULL, count is 0,
 *         or any buffer has invalid length or NULL pointer.
 * @return I2C_NACK if the device did not acknowledge the address or data.
 * @return I2C_BUS_ERROR on bus-level error.
 * @return I2C_ARBITRATION_LOST if arbitration was lost.
 *
 * @note
 * The function assumes the I²C peripheral is configured in 7-bit addressing mode.
 * It uses hardware RELOAD if necessary for transfers larger than 255 bytes.
 *
 * @note
 * The function performs a single I²C transaction (START → ADDRESS → DATA → STOP).
 */
I2C_Status I2C_Readv(uint8_t address, I2C_RxBuffer *buffers, size_t count);

/**
 * @brief A buffer used as part of a vectored I²C write operation.
 *
 * Each buffer represents a contiguous sequence of bytes that will be sent
 * as part of a single I²C transaction.
 */
typedef struct {
    /**
     * @brief Pointer to the data to transfer.
     */
    const uint8_t *data;

    /**
     * @brief Number of bytes in the buffer.
     */
    size_t length;
} I2C_TxBuffer;

/**
 * @brief Write multiple buffers to an I²C device in a single transaction.
 *
 * This function transmits multiple buffers sequentially over I²C without
 * issuing repeated START conditions between them.
 *
 * All buffers are treated as a continuous byte stream. The data is written
 * in the order they appear in the buffer array.
 *
 * If the total transfer size exceeds the hardware limit of 255 bytes per
 * I²C transfer phase, the function automatically splits the transfer into
 * multiple phases using the STM32 RELOAD mechanism.
 *
 * @param address 7-bit I²C slave address.
 * @param buffers Array of buffers containing the data to transmit.
 *                Buffers are sent sequentially in the order provided.
 * @param count   Number of buffers in the array.
 *
 * @return I2C_OK on success.
 * @return I2C_INVALID_ARGUMENT if buffers is NULL, count is 0,
 *         or any buffer has invalid length or NULL pointer.
 * @return I2C_NACK if the device did not acknowledge the address or data.
 * @return I2C_BUS_ERROR on bus-level error.
 * @return I2C_ARBITRATION_LOST if arbitration was lost.
 *
 * @note
 * This function performs a single I²C transaction (START → ADDRESS → DATA → STOP).
 *
 * @note
 * The function uses the STM32 RELOAD mechanism internally when the total
 * transfer exceeds 255 bytes per hardware phase.
 */
I2C_Status I2C_Writev(uint8_t address, const I2C_TxBuffer *buffers, size_t count);

#endif /* I2C_H */
