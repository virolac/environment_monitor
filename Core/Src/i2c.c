#include <stdbool.h>
#include <stddef.h>

#include "stm32c0xx.h"

#include "i2c.h"

/**********************
 * CONSTANTS / MACROS *
 **********************/

#define RCC_CCIPR_I2C1SEL_SHIFT 12U
#define RCC_CCIPR_I2C1SEL_PCLK (0U << RCC_CCIPR_I2C1SEL_SHIFT)
#define RCC_CCIPR_I2C1SEL_SYSCLK (1U << RCC_CCIPR_I2C1SEL_SHIFT)
#define RCC_CCIPR_I2C1SEL_HSIKER (2U << RCC_CCIPR_I2C1SEL_SHIFT)

#define CR1_PE (1U << 0)
#define CR1_DNF (0xFU << 8)
#define CR1_ANFOFF (1U << 12)

#define CR2_SADD_SHIFT 1U
#define CR2_SADD (0xFFU << CR2_SADD_SHIFT)
#define CR2_RD_WRN (1U << 10)
#define CR2_ADDR_MODE (1U << 11)
#define CR2_START (1U << 13)
#define CR2_STOP (1U << 14)
#define CR2_RELOAD (1U << 24)
#define CR2_AUTOEND (1U << 25)
#define CR2_NBYTES_SHIFT 16U
#define CR2_NBYTES (0xFFU << CR2_NBYTES_SHIFT)

#define TIMINGR_SCLL_SHIFT 0U
#define TIMINGR_SCLH_SHIFT 8U
#define TIMINGR_SDADEL_SHIFT 16U
#define TIMINGR_SCLDEL_SHIFT 20U
#define TIMINGR_PRESC_SHIFT 28U

#define ISR_TXIS (1U << 1)
#define ISR_RXNE (1U << 2)
#define ISR_NACKF (1U << 4)
#define ISR_STOPF (1U << 5)
#define ISR_TC (1U << 6)
#define ISR_TCR (1U << 7)
#define ISR_BERR (1U << 8)
#define ISR_ARLO (1U << 9)
#define ISR_BUSY (1U << 15)

#define ICR_NACKCF (1U << 4)
#define ICR_STOPCF (1U << 5)
#define ICR_BERRCF (1U << 8)
#define ICR_ARLOCF (1U << 9)

#define REQUEST_NO_STARTSTOP 0x00000000U
#define REQUEST_START_READ (CR2_START | CR2_RD_WRN)
#define REQUEST_START_WRITE (CR2_START)

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

static I2C_Status I2C_CheckErrors(void);
static uint8_t I2C_ConfigTransferPhase(uint8_t address, size_t remaining, uint32_t request);
static void I2C_WaitBusReady(void);
static void I2C_Stop(void);

/********************
 * PUBLIC INTERFACE *
 ********************/

void I2C_Init(void)
{
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN; /* Enable the I2C1 peripheral clock */

    /* Select PCLK as the peripheral clock source */
    RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;
    RCC->CCIPR |= RCC_CCIPR_I2C1SEL_PCLK;

    I2C1->CR1 &= ~CR1_PE; /* Disable the I2C1 peripheral for configuration */
    I2C1->CR1 &= ~CR1_ANFOFF; /* Enable the analog noise filter */
    I2C1->CR1 &= ~CR1_DNF; /* Disable the digital noise filter */

    /*
     * Assumes reset clock configuration:
     * SYSCLK = 12 MHz
     * HCLK   = 12 MHz
     * PCLK   = 12 MHz
     * I2C1 clock source = PCLK
     */
    I2C1->TIMINGR =
        (3U << TIMINGR_PRESC_SHIFT) |
        (2U << TIMINGR_SCLDEL_SHIFT) |
        (1U << TIMINGR_SDADEL_SHIFT) |
        (14U << TIMINGR_SCLH_SHIFT) |
        (16U << TIMINGR_SCLL_SHIFT);

    I2C1->CR1 |= CR1_PE; /* Enable the I2C1 peripheral */
}

I2C_Status I2C_Read(uint8_t address, uint8_t *buffer, uint8_t length)
{
    I2C_RxBuffer i2c_buffer = {
        .data = buffer,
        .length = length,
    };

    return I2C_Readv(address, &i2c_buffer, 1);
}

I2C_Status I2C_Write(uint8_t address, const uint8_t *buffer, uint8_t length)
{
    I2C_TxBuffer i2c_buffer = {
        .data = buffer,
        .length = length,
    };

    return I2C_Writev(address, &i2c_buffer, 1);
}

I2C_Status I2C_Readv(uint8_t address, I2C_RxBuffer *buffers, size_t count)
{
    if (count == 0 || buffers == NULL)
        return I2C_INVALID_ARGUMENT;

    size_t total_remaining = 0;
    for (size_t i = 0; i < count; i++) {
        if (buffers[i].length == 0 || buffers[i].data == NULL)
            return I2C_INVALID_ARGUMENT;

        total_remaining += buffers[i].length;
    }

    I2C_WaitBusReady();
    size_t chunk_remaining = I2C_ConfigTransferPhase(
            address, total_remaining, REQUEST_START_READ);

    I2C_Status status = I2C_OK;

    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < buffers[i].length; j++) {
            /* Wait for RXDR to be non-empty empty */
            while ((I2C1->ISR & ISR_RXNE) == 0) {
                status = I2C_CheckErrors();
                if (status != I2C_OK)
                    goto stop;
            }

            buffers[i].data[j] = I2C1->RXDR & 0xFF;

            total_remaining--;
            chunk_remaining--;

            if (chunk_remaining == 0 && total_remaining > 0) {
                /* Wait for transfer complete reload */
                while ((I2C1->ISR & ISR_TCR) == 0) {
                    status = I2C_CheckErrors();
                    if (status != I2C_OK)
                        goto stop;
                }

                chunk_remaining = I2C_ConfigTransferPhase(
                        address, total_remaining, REQUEST_NO_STARTSTOP);
            }
        }
    }

    /* Wait until transfer is complete */
    while ((I2C1->ISR & ISR_TC) == 0) {
        status = I2C_CheckErrors();
        if (status != I2C_OK)
            goto stop;
    }

stop:
    I2C_Stop();
    return status;
}

I2C_Status I2C_Writev(uint8_t address, const I2C_TxBuffer *buffers, size_t count)
{
    if (count == 0 || buffers == NULL)
        return I2C_INVALID_ARGUMENT;

    size_t total_remaining = 0;
    for (size_t i = 0; i < count; i++) {
        if (buffers[i].length == 0 || buffers[i].data == NULL)
            return I2C_INVALID_ARGUMENT;

        total_remaining += buffers[i].length;
    }


    I2C_WaitBusReady();
    size_t chunk_remaining = I2C_ConfigTransferPhase(
            address, total_remaining, REQUEST_START_WRITE);

    I2C_Status status = I2C_OK;

    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < buffers[i].length; j++) {
            /* Wait for TXDR to be empty */
            while ((I2C1->ISR & ISR_TXIS) == 0) {
                status = I2C_CheckErrors();
                if (status != I2C_OK)
                    goto stop;
            }

            I2C1->TXDR = buffers[i].data[j]; /* Set byte to send */

            total_remaining--;
            chunk_remaining--;

            if (chunk_remaining == 0 && total_remaining > 0) {
                /* Wait for transfer complete reload */
                while ((I2C1->ISR & ISR_TCR) == 0) {
                    status = I2C_CheckErrors();
                    if (status != I2C_OK)
                        goto stop;
                }

                chunk_remaining = I2C_ConfigTransferPhase(
                        address, total_remaining, REQUEST_NO_STARTSTOP);
            }
        }
    }

    /* Wait until transfer is complete */
    while ((I2C1->ISR & ISR_TC) == 0) {
        status = I2C_CheckErrors();
        if (status != I2C_OK)
            goto stop;
    }

stop:
    I2C_Stop();
    return status;
}

/*********************
 * PRIVATE INTERFACE *
 *********************/

static I2C_Status I2C_CheckErrors(void)
{
    if (I2C1->ISR & ISR_NACKF) { /* NOT ACKNOWLEDGE */
        I2C1->ICR |= ICR_NACKCF;
        return I2C_NACK;
    } else if (I2C1->ISR & ISR_BERR) { /* BUS ERROR */
        I2C1->ICR |= ICR_BERRCF;
        return I2C_BUS_ERROR;
    } else if (I2C1->ISR & ISR_ARLO) { /* ARBITRATION LOST */
        I2C1->ICR |= ICR_ARLOCF;
        return I2C_ARBITRATION_LOST;
    } else { /* NO ERROR */
        return I2C_OK;
    }
}

static uint8_t I2C_ConfigTransferPhase(uint8_t address, size_t remaining, uint32_t request)
{
    uint32_t cr2_value = I2C1->CR2;

    cr2_value &= (uint32_t)~(CR2_NBYTES | CR2_AUTOEND | CR2_RELOAD | CR2_STOP |
                             CR2_START | CR2_ADDR_MODE | CR2_RD_WRN | CR2_SADD);

    /* Set target address */
    cr2_value |= ((uint32_t)address << CR2_SADD_SHIFT) & CR2_SADD;

    uint8_t chunk;
    if (remaining > 255) {
        chunk = 255;
        cr2_value |= CR2_RELOAD;
    } else {
        chunk = (uint8_t)remaining;
    }

    /* Set number of bytes to transfer */
    cr2_value |= ((uint32_t)chunk << CR2_NBYTES_SHIFT) & CR2_NBYTES;

    cr2_value |= (uint32_t)request;

    I2C1->CR2 = cr2_value;

    return chunk;
}

static void I2C_WaitBusReady(void)
{
    while (I2C1->ISR & ISR_BUSY) {
        /* Wait while the bus is busy */
    }
}

static void I2C_Stop(void)
{
    I2C1->CR2 |= CR2_STOP; /* Generate stop condition and release the bus */

    while ((I2C1->ISR & ISR_STOPF) == 0) {
        /* Wait for stop flag */
    }

    I2C1->ICR |= ICR_STOPCF; /* Clear stop flag */
}
