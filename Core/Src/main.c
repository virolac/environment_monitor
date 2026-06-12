#include "gpio.h"

int main(void)
{
	GPIO_EnableClock(GPIOA);

	GPIO_Init(GPIOA, 9, &(GPIO_Config){.mode = GPIO_MODE_OUTPUT});
	GPIO_Init(GPIOA, 10, &(GPIO_Config){.mode = GPIO_MODE_OUTPUT});

	while (1) {
	    GPIOA->ODR ^= (1 << 9);
	    GPIOA->ODR ^= (1 << 10);

	    for (volatile int i = 0; i < 100000; i++);
	}

	return 0;
}
