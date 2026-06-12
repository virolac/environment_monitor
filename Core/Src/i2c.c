#include <stdbool.h>
#include <stddef.h>

#include "stm32c0xx.h"

#include "i2c.h"

/**********************
 * CONSTANTS / MACROS *
 **********************/

#define RCC_CCIPR_I2C1SEL_SHIFT 12UL
#define RCC_CCIPR_I2C1SEL_PCLK (0UL << RCC_CCIPR_I2C1SEL_SHIFT)
#define RCC_CCIPR_I2C1SEL_SYSCLK (1UL << RCC_CCIPR_I2C1SEL_SHIFT)
#define RCC_CCIPR_I2C1SEL_HSIKER (2UL << RCC_CCIPR_I2C1SEL_SHIFT)

#define CR1_PE (1UL << 0)
#define CR1_DNF (0xFUL << 8)
#define CR1_ANFOFF (1UL << 12)

#define CR2_SADD_SHIFT 1UL
#define CR2_SADD (0xFFUL << CR2_SADD_SHIFT)
#define CR2_RD_WRN (1UL << 10)
#define CR2_ADDR_MODE (1UL << 11)
#define CR2_START (1UL << 13)
#define CR2_STOP (1UL << 14)
#define CR2_RELOAD (1UL << 24)
#define CR2_AUTOEND (1UL << 25)
#define CR2_NBYTES_SHIFT 16UL
#define CR2_NBYTES (0xFFUL << CR2_NBYTES_SHIFT)

#define TIMINGR_SCLL_SHIFT 0UL
#define TIMINGR_SCLH_SHIFT 8UL
#define TIMINGR_SDADEL_SHIFT 16UL
#define TIMINGR_SCLDEL_SHIFT 20UL
#define TIMINGR_PRESC_SHIFT 28UL

#define ISR_TXIS (1UL << 1)
#define ISR_RXNE (1UL << 2)
#define ISR_NACKF (1UL << 4)
#define ISR_STOPF (1UL << 5)
#define ISR_TC (1UL << 6)
#define ISR_BERR (1UL << 8)
#define ISR_ARLO (1UL << 9)
#define ISR_BUSY (1UL << 15)

#define ICR_NACKCF (1UL << 4)
#define ICR_STOPCF (1UL << 5)
#define ICR_BERRCF (1UL << 8)
#define ICR_ARLOCF (1UL << 9)

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

static I2C_Status I2C_CheckErrors(void);
static void I2C_WaitBusReady(void);
static void I2C_Start(uint8_t address, uint8_t length, bool is_read);
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
        (3UL << TIMINGR_PRESC_SHIFT) |
        (2UL << TIMINGR_SCLDEL_SHIFT) |
        (1UL << TIMINGR_SDADEL_SHIFT) |
        (14UL << TIMINGR_SCLH_SHIFT) |
        (16UL << TIMINGR_SCLL_SHIFT);

    I2C1->CR1 |= CR1_PE; /* Enable the I2C1 peripheral */
}

I2C_Status I2C_Probe(uint8_t address)
{
    I2C_WaitBusReady();
    I2C_Start(address, 0, false);

    I2C_Status status = I2C_OK;

    /* Wait for ACK/NACK or error */
    while ((I2C1->ISR & ISR_TXIS) == 0) {
        status = I2C_CheckErrors();
        if (status != I2C_OK)
            goto stop;
    }

stop:
    I2C_Stop();
    return status;
}

I2C_Status I2C_WriteByte(uint8_t address, uint8_t byte)
{
    const uint8_t buffer[1] = { byte };

    return I2C_Write(address, buffer, 1);
}

I2C_Status I2C_Write(uint8_t address, const uint8_t *buffer, uint8_t length)
{
    if (length == 0 || buffer == NULL)
        return I2C_INVALID_ARGUMENT;

    I2C_WaitBusReady();
    I2C_Start(address, length, false);

    I2C_Status status = I2C_OK;

    for (int i = 0; i < length; i++) {
        /* Wait for TXDR to be empty */
        while ((I2C1->ISR & ISR_TXIS) == 0) {
            status = I2C_CheckErrors();
            if (status != I2C_OK)
                goto stop;
        }

        I2C1->TXDR = buffer[i]; /* Set byte to send */
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

I2C_Status I2C_Read(uint8_t address, uint8_t *buffer, uint8_t length)
{
    if (length == 0 || buffer == NULL)
        return I2C_INVALID_ARGUMENT;

    I2C_WaitBusReady();
    I2C_Start(address, length, true);

    I2C_Status status = I2C_OK;

    for (int i = 0; i < length; i++) {
        /* Wait for RXDR to be non-empty */
        while ((I2C1->ISR & ISR_RXNE) == 0) {
            status = I2C_CheckErrors();
            if (status != I2C_OK)
                goto stop;
        }

        buffer[i] = I2C1->RXDR & 0xFF;
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

static void I2C_WaitBusReady(void)
{
    while (I2C1->ISR & ISR_BUSY) {
        /* Wait while the bus is busy */
    }
}

static void I2C_Start(uint8_t address, uint8_t length, bool is_read)
{
    I2C1->CR2 &= ~CR2_RELOAD; /* Clear RELOAD; only handle transfer sizes <= 255 */
    I2C1->CR2 &= ~CR2_AUTOEND; /* Don't send STOP bit automatically */
    I2C1->CR2 &= ~CR2_ADDR_MODE; /* Use 7-bit addressing mode */

    /* Set target address */
    I2C1->CR2 &= ~CR2_SADD;
    I2C1->CR2 |= ((uint32_t)address << CR2_SADD_SHIFT) & CR2_SADD;

    /* Set number of bytes to transfer */
    I2C1->CR2 &= ~CR2_NBYTES;
    I2C1->CR2 |= ((uint32_t)length << CR2_NBYTES_SHIFT) & CR2_NBYTES;

    if (is_read)
        I2C1->CR2 |= CR2_RD_WRN; /* Request a read transfer */
    else
        I2C1->CR2 &= ~CR2_RD_WRN; /* Request a write transfer */

    I2C1->CR2 |= CR2_START; /* Start the transfer and begin address phase */
}

static void I2C_Stop(void)
{
    I2C1->CR2 |= CR2_STOP; /* Generate stop condition and release the bus */

    while ((I2C1->ISR & ISR_STOPF) == 0) {
        /* Wait for stop flag */
    }

    I2C1->ICR |= ICR_STOPCF; /* Clear stop flag */
}
