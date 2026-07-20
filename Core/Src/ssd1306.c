#include <stdlib.h>
#include <string.h>

#include "i2c.h"
#include "ssd1306.h"

#define NUM_SEGMENTS 128U
#define NUM_COMMONS 64U
#define NUM_PAGES 8U
#define PAGE_HEIGHT 8U
#define FRAMEBUFFER_SIZE (NUM_PAGES * NUM_SEGMENTS)

#define DEVICE_ADDRESS 0x3CU

/*****************
 * CONTROL BYTES *
 *****************/

#define CONTROL_COMMAND 0x00U
#define CONTROL_DATA 0x40U

/************
 * COMMANDS *
 ************/

#define CMD_SET_DISPLAY_START_LINE 0x40
#define CMD_SET_DISPLAY_OFFSET 0xD3
#define CMD_SET_SEGMENT_REMAP_NORMAL 0xA0
#define CMD_SET_SEGMENT_REMAP_FLIPPED 0xA1
#define CMD_SET_COM_SCAN_NORMAL 0xC0
#define CMD_SET_COM_SCAN_FLIPPED 0xC8
#define CMD_SET_COM_PINS 0xDA
#define COM_PIN_SEQUENTIAL_CONFIG 0x02
#define COM_PIN_ALTERNATIVE_CONFIG 0x12
#define CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define CMD_NORMAL_DISPLAY 0xA6
#define CMD_SET_DISPLAY_CLK_DIV 0xD5
#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_SET_CONTRAST 0x81
#define CMD_CHARGE_PUMP 0x8D
#define CHARGE_PUMP_ENABLE 0x14
#define CMD_SET_MUX_RATIO 0xA8
#define CMD_SET_MEMORY_ADDRESSING_MODE 0x20
#define HORIZONTAL_ADDRESSING_MODE 0x00
#define VERTICAL_ADDRESSING_MODE 0x01
#define PAGE_ADDRESSING_MODE 0x02
#define CMD_SET_COLUMN_ADDRESS 0x21
#define CMD_SET_PAGE_ADDRESS 0x22

/**********
 * MACROS *
 **********/

#define FB(page, segment) \
    (framebuffer[(page) * NUM_SEGMENTS + (segment)])

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

uint8_t SSD1306_GetWidth(void)
{
    return NUM_SEGMENTS;
}

uint8_t SSD1306_GetHeight(void)
{
    return NUM_COMMONS;
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
        CMD_SET_COLUMN_ADDRESS, 0x00, NUM_SEGMENTS - 1,
        CMD_SET_PAGE_ADDRESS, 0x00, NUM_PAGES - 1,
    };

    SSD1306_Status status = SSD1306_SendCommands(addressing_mode, sizeof(addressing_mode));

    if (status != SSD1306_OK)
        return status;

    return SSD1306_SendData(framebuffer, FRAMEBUFFER_SIZE);
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= NUM_SEGMENTS || y >= NUM_COMMONS) return;

    size_t page = y / PAGE_HEIGHT;
    uint8_t pixel_mask = 1U << (y % PAGE_HEIGHT);

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
