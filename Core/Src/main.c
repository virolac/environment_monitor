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

    uint8_t width = SSD1306_GetWidth();
    uint8_t height = SSD1306_GetHeight();

    while (1) {
        for (uint8_t col = 0; col < width; col++) {
            for (uint8_t row = 0; row < height; row++) {
                SSD1306_Clear();

                SSD1306_DrawCircle(64, 32, 4);
                SSD1306_DrawCircle(64, 32, 8);
                SSD1306_DrawCircle(64, 32, 12);
                SSD1306_DrawCircle(64, 32, 16);
                SSD1306_DrawCircle(64, 32, 20);
                SSD1306_DrawCircle(64, 32, 24);
                SSD1306_DrawCircle(64, 32, 28);

                SSD1306_Update();
            }
        }
    }

    return 0;
}
