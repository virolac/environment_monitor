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

                /* Outer border */
                SSD1306_DrawRectangle(0, 0, 128, 64);

                /* Nested rectangles */
                SSD1306_DrawRectangle(8, 8, 112, 48);
                SSD1306_DrawRectangle(16, 16, 96, 32);
                SSD1306_DrawRectangle(24, 24, 80, 16);

                /* Small center rectangle */
                SSD1306_DrawRectangle(56, 28, 16, 8);

                SSD1306_Update();
            }
        }
    }

    return 0;
}
