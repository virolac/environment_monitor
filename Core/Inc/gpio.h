#ifndef GPIO_H
#define GPIO_H

#include "stm32c0xx.h"

typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_AF,
    GPIO_MODE_ANALOG,
} GPIO_Mode;

typedef enum {
    GPIO_OTYPE_PUSH_PULL,
    GPIO_OTYPE_OPEN_DRAIN,
} GPIO_OType;

typedef enum {
    GPIO_OSPEED_VERY_LOW,
    GPIO_OSPEED_LOW,
    GPIO_OSPEED_HIGH,
    GPIO_OSPEED_VERY_HIGH,
} GPIO_OSpeed;

typedef enum {
    GPIO_PUPD_NONE,
    GPIO_PUPD_UP,
    GPIO_PUPD_DOWN,
} GPIO_PuPd;

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

typedef struct {
    GPIO_Mode mode;
    GPIO_OType otype;
    GPIO_OSpeed ospeed;
    GPIO_PuPd pupd;
    GPIO_AF af; /* only used if AF mode */
} GPIO_Config;

void GPIO_EnableClock(GPIO_TypeDef *port);
void GPIO_Init(GPIO_TypeDef *port, uint8_t pin, const GPIO_Config *cfg);

#endif /* GPIO_H */
