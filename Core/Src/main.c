#include "gpio.h"
#include "i2c.h"

int main(void)
{
	GPIO_EnableClock(GPIOA);

	GPIO_Config scl = {
		.mode = GPIO_MODE_AF,
		.otype = GPIO_OTYPE_OPEN_DRAIN,
		.ospeed = GPIO_OSPEED_VERY_HIGH,
		.pupd = GPIO_PUPD_NONE,
		.af = GPIO_AF6,
	};

	GPIO_Config sda = scl;

	GPIO_Init(GPIOA, 9, &scl);
	GPIO_Init(GPIOA, 10, &sda);

	I2C_Init();
	I2C_Probe(0x3C);

	while (1) {
	}

	return 0;
}
