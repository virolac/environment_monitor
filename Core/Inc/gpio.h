/**
 * @file gpio.h
 * @brief GPIO driver configuration interface.
 *
 * This module provides a lightweight abstraction over STM32 GPIO configuration,
 * including mode selection, output type, speed, pull-up/pull-down configuration,
 * and alternate function mapping.
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32c0xx.h"

/**
 * @enum GPIO_Mode
 * @brief GPIO pin operating mode.
 */
typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_AF,
    GPIO_MODE_ANALOG,
} GPIO_Mode;

/**
 * @enum GPIO_OType
 * @brief GPIO output type configuration (only relevant in output/AF modes).
 */
typedef enum {
    GPIO_OTYPE_PUSH_PULL,
    GPIO_OTYPE_OPEN_DRAIN,
} GPIO_OType;

/**
 * @enum GPIO_OSpeed
 * @brief GPIO output speed (slew rate control).
 *
 * Higher speeds improve signal rise/fall time but increase power consumption
 * and electromagnetic noise.
 */
typedef enum {
    GPIO_OSPEED_VERY_LOW,
    GPIO_OSPEED_LOW,
    GPIO_OSPEED_HIGH,
    GPIO_OSPEED_VERY_HIGH,
} GPIO_OSpeed;

/**
 * @enum GPIO_PuPd
 * @brief Internal pull-up / pull-down resistor configuration.
 */
typedef enum {
    GPIO_PUPD_NONE,
    GPIO_PUPD_UP,
    GPIO_PUPD_DOWN,
} GPIO_PuPd;

/**
 * @enum GPIO_AF
 * @brief Alternate function selection (AF0–AF15).
 *
 * Each GPIO pin can be mapped to a peripheral function depending on device
 * alternate function mapping (e.g. I2C, USART, SPI).
 */
typedef enum {
    GPIO_AF0,
    GPIO_AF1,
    GPIO_AF2,
    GPIO_AF3,
    GPIO_AF4,
    GPIO_AF5,
    GPIO_AF6,
    GPIO_AF7,
    GPIO_AF8,
    GPIO_AF9,
    GPIO_AF10,
    GPIO_AF11,
    GPIO_AF12,
    GPIO_AF13,
    GPIO_AF14,
    GPIO_AF15,
} GPIO_AF;

/**
 * @struct GPIO_Config
 * @brief GPIO pin configuration descriptor.
 *
 * Used to initialize a GPIO pin with a specific mode and electrical behavior.
 */
typedef struct {
    GPIO_Mode mode;
    GPIO_OType otype;
    GPIO_OSpeed ospeed;
    GPIO_PuPd pupd;
    GPIO_AF af; /* only used if AF mode */
} GPIO_Config;

/**
 * @brief Enable the clock for a GPIO port.
 *
 * Must be called before using GPIO_Init on a given port.
 *
 * @param port GPIO port base address (e.g. GPIOA, GPIOB)
 */
void GPIO_EnableClock(GPIO_TypeDef *port);

/**
 * @brief Initialize a GPIO pin according to the given configuration.
 *
 * Configures mode, output type, speed, pull resistors, and alternate function
 * (if applicable).
 *
 * @param port GPIO port base address
 * @param pin  Pin number (0–15)
 * @param cfg  Pointer to configuration structure
 */
void GPIO_Init(GPIO_TypeDef *port, uint8_t pin, const GPIO_Config *cfg);

#endif /* GPIO_H */
