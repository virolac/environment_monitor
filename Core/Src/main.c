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

    uint8_t cx = width / 2;
    uint8_t cy = height / 2;

    while (1) {
        for (uint8_t col = 0; col < width; col++) {
            for (uint8_t row = 0; row < height; row++) {
                SSD1306_Clear();

                /* Horizontal */
                SSD1306_DrawLine(cx, cy, 127, cy);
                SSD1306_DrawLine(cx, cy,   0, cy);

                /* Vertical */
                SSD1306_DrawLine(cx, cy, cx,   0);
                SSD1306_DrawLine(cx, cy, cx,  63);

                /* Diagonals (45°) */
                SSD1306_DrawLine(cx, cy, 127, 63);
                SSD1306_DrawLine(cx, cy, 127,  0);
                SSD1306_DrawLine(cx, cy,   0, 63);
                SSD1306_DrawLine(cx, cy,   0,  0);

                /* Shallow slopes */
                SSD1306_DrawLine(cx, cy, 127, 48);
                SSD1306_DrawLine(cx, cy, 127, 16);
                SSD1306_DrawLine(cx, cy,   0, 48);
                SSD1306_DrawLine(cx, cy,   0, 16);

                /* Steep slopes */
                SSD1306_DrawLine(cx, cy,  80, 63);
                SSD1306_DrawLine(cx, cy,  80,  0);
                SSD1306_DrawLine(cx, cy,  48, 63);
                SSD1306_DrawLine(cx, cy,  48,  0);

                SSD1306_Update();
            }
        }
    }

    return 0;
}
