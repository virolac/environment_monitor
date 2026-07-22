#include "gpio.h"
#include "i2c.h"
#include "ssd1306.h"

static void Delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

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

    SSD1306_Clear();

    SSD1306_DrawString(0, 0,  "Page 0");
    SSD1306_DrawString(0, 8,  "Page 1");
    SSD1306_DrawString(0, 16, "Page 2");
    SSD1306_DrawString(0, 24, "Page 3");
    SSD1306_DrawString(0, 32, "Page 4");
    SSD1306_DrawString(0, 40, "Page 5");
    SSD1306_DrawString(0, 48, "Page 6");
    SSD1306_DrawString(0, 56, "Page 7");

    SSD1306_Update();

    Delay(5000000);

    SSD1306_StartHorizontalScroll(
        SSD1306_SCROLL_RIGHT,
        SSD1306_SCROLL_INTERVAL_5_FRAMES,
        2,
        5
    );

    Delay(5000000);

    SSD1306_StopScroll();

    while (1) {
    }

    return 0;
}
