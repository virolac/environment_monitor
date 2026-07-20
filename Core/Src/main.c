#include "gpio.h"
#include "i2c.h"
#include "ssd1306.h"

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
    SSD1306_Init();

    while (1) {
        SSD1306_Clear();

        SSD1306_DrawString(0, 0, "Hello");
        SSD1306_DrawString(0, 12, "World!");

        SSD1306_DrawString(0, 28, "0123456789");
        SSD1306_DrawString(0, 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        SSD1306_Update();
    }

    return 0;
}
