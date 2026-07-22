#include <stdlib.h>
#include <string.h>

#include "font5x7.h"
#include "i2c.h"
#include "ssd1306.h"

#define FRAMEBUFFER_SIZE (SSD1306_NUM_PAGES * SSD1306_NUM_SEGMENTS)

#define DEVICE_ADDRESS 0x3CU

/*****************
 * CONTROL BYTES *
 *****************/

#define CONTROL_COMMAND 0x00U
#define CONTROL_DATA 0x40U

/************
 * COMMANDS *
 ************/

#define CMD_SET_DISPLAY_START_LINE 0x40U
#define CMD_SET_DISPLAY_OFFSET 0xD3U
#define CMD_SET_SEGMENT_REMAP_NORMAL 0xA0U
#define CMD_SET_SEGMENT_REMAP_FLIPPED 0xA1U
#define CMD_SET_COM_SCAN_NORMAL 0xC0U
#define CMD_SET_COM_SCAN_FLIPPED 0xC8U
#define CMD_SET_COM_PINS 0xDAU
#define COM_PIN_SEQUENTIAL_CONFIG 0x02U
#define COM_PIN_ALTERNATIVE_CONFIG 0x12U
#define CMD_DISPLAY_ALL_ON_RESUME 0xA4U
#define CMD_NORMAL_DISPLAY 0xA6U
#define CMD_SET_DISPLAY_CLK_DIV 0xD5U
#define CMD_DISPLAY_OFF 0xAEU
#define CMD_DISPLAY_ON 0xAFU
#define CMD_SET_CONTRAST 0x81U
#define CMD_CHARGE_PUMP 0x8DU
#define CHARGE_PUMP_ENABLE 0x14U
#define CMD_SET_MUX_RATIO 0xA8U
#define CMD_SET_MEMORY_ADDRESSING_MODE 0x20U
#define HORIZONTAL_ADDRESSING_MODE 0x00U
#define VERTICAL_ADDRESSING_MODE 0x01U
#define PAGE_ADDRESSING_MODE 0x02U
#define CMD_SET_COLUMN_ADDRESS 0x21U
#define CMD_SET_PAGE_ADDRESS 0x22U
#define CMD_DEACTIVATE_SCROLL 0x2EU
#define CMD_ACTIVATE_SCROLL 0x2FU

/**********
 * MACROS *
 **********/

#define FB(page, segment) \
    (framebuffer[(page) * SSD1306_NUM_SEGMENTS + (segment)])

/********************
 * GLOBAL VARIABLES *
 ********************/

static uint8_t framebuffer[FRAMEBUFFER_SIZE] = { 0 };

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

static SSD1306_Status SSD1306_SendCommands(const uint8_t *cmds, size_t size);
static SSD1306_Status SSD1306_SendData(const uint8_t *data, size_t size);

/********************
 * PUBLIC INTERFACE *
 ********************/

SSD1306_Status SSD1306_Init(void)
{
    static const uint8_t init_sequence[] = {
        CMD_SET_MUX_RATIO, 0x3F,
        CMD_SET_DISPLAY_OFFSET, 0x00,
        CMD_SET_DISPLAY_START_LINE,
        CMD_SET_SEGMENT_REMAP_FLIPPED,
        CMD_SET_COM_SCAN_FLIPPED,
        CMD_SET_COM_PINS, COM_PIN_ALTERNATIVE_CONFIG,
        CMD_SET_CONTRAST, 0x7F,
        CMD_DISPLAY_ALL_ON_RESUME,
        CMD_NORMAL_DISPLAY,
        CMD_SET_DISPLAY_CLK_DIV, 0x80,
        CMD_CHARGE_PUMP,
        CHARGE_PUMP_ENABLE,
        CMD_DISPLAY_ON,
    };

    return SSD1306_SendCommands(init_sequence, sizeof(init_sequence));
}

SSD1306_Status SSD1306_DisplayOn(void)
{
    static const uint8_t cmd = CMD_DISPLAY_ON;

    return SSD1306_SendCommands(&cmd, sizeof(cmd));
}

SSD1306_Status SSD1306_DisplayOff(void)
{
    static const uint8_t cmd = CMD_DISPLAY_OFF;

    return SSD1306_SendCommands(&cmd, sizeof(cmd));
}

SSD1306_Status SSD1306_SetContrast(uint8_t contrast)
{
    const uint8_t cmd[] = { CMD_SET_CONTRAST, contrast };

    return SSD1306_SendCommands(cmd, 2);
}

void SSD1306_Clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

SSD1306_Status SSD1306_Update(void)
{
    static const uint8_t addressing_mode[] = {
        CMD_SET_MEMORY_ADDRESSING_MODE, HORIZONTAL_ADDRESSING_MODE,
        CMD_SET_COLUMN_ADDRESS, 0x00, SSD1306_NUM_SEGMENTS - 1,
        CMD_SET_PAGE_ADDRESS, 0x00, SSD1306_NUM_PAGES - 1,
    };

    SSD1306_Status status = SSD1306_SendCommands(addressing_mode, sizeof(addressing_mode));

    if (status != SSD1306_OK)
        return status;

    return SSD1306_SendData(framebuffer, FRAMEBUFFER_SIZE);
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= SSD1306_NUM_SEGMENTS || y >= SSD1306_NUM_COMMONS) return;

    size_t page = y / SSD1306_PAGE_HEIGHT;
    uint8_t pixel_mask = 1U << (y % SSD1306_PAGE_HEIGHT);

    if (on)
        FB(page, x) |= pixel_mask;
    else
        FB(page, x) &= ~pixel_mask;
}

void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    int x = x0;
    int y = y0;

    while (true) {
        SSD1306_DrawPixel(x, y, true);

        if (x == x1 && y == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void SSD1306_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    int right = x + width - 1;
    int bottom = y + height - 1;

    SSD1306_DrawLine(x, y, right, y); /* Top */
    SSD1306_DrawLine(right, y, right, bottom); /* Right */
    SSD1306_DrawLine(x, bottom, right, bottom); /* Bottom */
    SSD1306_DrawLine(x, y, x, bottom); /* Left */
}

void SSD1306_DrawCircle(uint8_t cx, uint8_t cy, uint8_t radius)
{
    int x = 0;
    int y = -radius; /* Start with the top of the circle */
    int p = -radius;

    while (x < -y) {
        if (p > 0) {
            y++;
            p += 2*(x + y) + 1;
        } else {
            p += 2*x + 1;
        }

        SSD1306_DrawPixel(cx + x, cy + y, true);
        SSD1306_DrawPixel(cx - x, cy + y, true);
        SSD1306_DrawPixel(cx + x, cy - y, true);
        SSD1306_DrawPixel(cx - x, cy - y, true);
        SSD1306_DrawPixel(cx + y, cy + x, true);
        SSD1306_DrawPixel(cx + y, cy - x, true);
        SSD1306_DrawPixel(cx - y, cy + x, true);
        SSD1306_DrawPixel(cx - y, cy - x, true);

        x++;
    }
}

void SSD1306_DrawChar(uint8_t x, uint8_t y, char c)
{
    const uint8_t *glyph = Font5x7_GetGlyph(c);

    for (uint8_t col = 0; col < FONT5X7_WIDTH; col++) {
        uint8_t column_data = glyph[col];

        for (uint8_t row = 0; row < FONT5X7_HEIGHT; row++) {
            if (column_data & (1U << row)) {
                SSD1306_DrawPixel(x + col, y + row, true);
            }
        }
    }
}

void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str)
{
    while (*str) {
        SSD1306_DrawChar(x, y, *str);

        x += FONT5X7_WIDTH + 1; /* 1 column spacing */
        str++;
    }
}

SSD1306_Status SSD1306_StartHorizontalScroll(
    SSD1306_HorizontalScrollDirection direction,
    SSD1306_ScrollInterval interval,
    uint8_t start_page,
    uint8_t end_page
)
{
    if (
        start_page >= SSD1306_NUM_PAGES ||
        end_page >= SSD1306_NUM_PAGES ||
        start_page > end_page
    ) {
        return SSD1306_INVALID_ARGUMENT;
    }

    const uint8_t cmds[] = {
        direction,
        0x00, /* Dummy byte */
        start_page,
        interval,
        end_page,
        0x00, /* Dummy byte */
        0xFF, /* Dummy byte */
        CMD_ACTIVATE_SCROLL,
    };

    return SSD1306_SendCommands(cmds, 8);
}

SSD1306_Status SSD1306_StopScroll(void)
{
    const uint8_t cmd[] = { CMD_DEACTIVATE_SCROLL };

    return SSD1306_SendCommands(cmd, 1);
}

/*********************
 * PRIVATE INTERFACE *
 *********************/

static SSD1306_Status SSD1306_SendCommands(const uint8_t *cmds, size_t size)
{
    static const uint8_t control = CONTROL_COMMAND;
    const I2C_TxBuffer buffers[] = {
        { .data = &control, .length = 1 },
        { .data = cmds, .length = size },
    };

    if (I2C_Writev(DEVICE_ADDRESS, buffers, 2) != I2C_OK)
        return SSD1306_I2C_ERROR;

    return SSD1306_OK;
}

static SSD1306_Status SSD1306_SendData(const uint8_t *data, size_t size)
{
    static const uint8_t control = CONTROL_DATA;
    const I2C_TxBuffer buffers[] = {
        { .data = &control, .length = 1 },
        { .data = data, .length = size },
    };

    if (I2C_Writev(DEVICE_ADDRESS, buffers, 2) != I2C_OK)
        return SSD1306_I2C_ERROR;

    return SSD1306_OK;
}
