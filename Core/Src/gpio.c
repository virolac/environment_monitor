#include <stddef.h>

#include "gpio.h"

/**********************
 * CONSTANTS / MACROS *
 **********************/

#define GPIO_2BIT_FIELD(pin) (0x3U << (2U * (pin)))
#define GPIO_2BIT_SET(pin, value) ((uint32_t)(value) << (2U * (pin)))

#define OTYPER_SHIFT(pin) (pin)
#define OTYPER_FIELD(pin) (1U << OTYPER_SHIFT(pin))
#define OTYPER(pin, type) ((uint32_t)(type) << OTYPER_SHIFT(pin))

#define AF_SHIFT(pin) (4U * ((pin) % 8))
#define AF_FIELD(pin) (0xFU << AF_SHIFT(pin))
#define AF(pin, af) ((uint32_t)(af) << AF_SHIFT(pin))

/********************
 * PUBLIC INTERFACE *
 ********************/

void GPIO_EnableClock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    else if (port == GPIOB)
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    else if (port == GPIOC)
        RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    else if (port == GPIOD)
        RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    else if (port == GPIOF)
        RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
}

void GPIO_Init(GPIO_TypeDef *port, uint8_t pin, const GPIO_Config *cfg)
{
    if (port == NULL || cfg == NULL) return;

    /* Mode */
    port->MODER &= ~GPIO_2BIT_FIELD(pin);
    port->MODER |= GPIO_2BIT_SET(pin, cfg->mode);

    /* Output Type */
    port->OTYPER &= ~OTYPER_FIELD(pin);
    port->OTYPER |= OTYPER(pin, cfg->otype);

    /* Output Speed */
    port->OSPEEDR &= ~GPIO_2BIT_FIELD(pin);
    port->OSPEEDR |= GPIO_2BIT_SET(pin, cfg->ospeed);

    /* Pull-up / Pull-down */
    port->PUPDR &= ~GPIO_2BIT_FIELD(pin);
    port->PUPDR |= GPIO_2BIT_SET(pin, cfg->pupd);

    /* Alternate Function */
    if (cfg->mode == GPIO_MODE_AF) {
        port->AFR[pin / 8] &= ~AF_FIELD(pin);
        port->AFR[pin / 8] |= AF(pin, cfg->af);
    }
}
