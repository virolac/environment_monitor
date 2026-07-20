#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SSD1306_OK,
    SSD1306_I2C_ERROR,
    SSD1306_INVALID_ARGUMENT,
} SSD1306_Status;

/**
 * @brief Initialize the SSD1306 display.
 *
 * Configures the controller using the required initialization
 * sequence and prepares the internal display buffer.
 *
 * The display contents after initialization are undefined until
 * SSD1306_Clear() or SSD1306_Update() is called.
 *
 * @return SSD1306_OK on success, error code otherwise.
 */
SSD1306_Status SSD1306_Init(void);

/**
 * @brief Gets the width of the display in pixels.
 *
 * Returns the number of horizontal pixels available on the display.
 *
 * @return The display width in pixels.
 */
uint8_t SSD1306_GetWidth(void);

/**
 * @brief Gets the height of the display in pixels.
 *
 * Returns the number of vertical pixels available on the display.
 *
 * @return The display height in pixels.
 */
uint8_t SSD1306_GetHeight(void);

/**
 * @brief Turn the OLED display on.
 *
 * Enables the display output. The contents of the display RAM
 * are preserved while the display is off, so the previous image
 * will reappear when the display is turned on again.
 *
 * @return SSD1306_OK on success, error code otherwise.
 */
SSD1306_Status SSD1306_DisplayOn(void);

/**
 * @brief Turn the OLED display off.
 *
 * Disables the display output without clearing the display RAM.
 * The display contents are preserved and can be shown again by
 * calling SSD1306_DisplayOn().
 *
 * @return SSD1306_OK on success, error code otherwise.
 */
SSD1306_Status SSD1306_DisplayOff(void);

/**
 * @brief Clear the internal display buffer.
 *
 * Sets all pixels in the software framebuffer to the OFF state.
 * The change is not visible until SSD1306_Update() is called.
 */
void SSD1306_Clear(void);

/**
 * @brief Copy the internal display buffer to the OLED display.
 *
 * Transfers the complete framebuffer over I2C to the SSD1306
 * display RAM, making all pending drawing operations visible.
 *
 * @return SSD1306_OK on success, error code otherwise.
 */
SSD1306_Status SSD1306_Update(void);

/**
 * @brief Set or clear a single pixel in the framebuffer.
 *
 * Modifies the state of a pixel in the internal framebuffer.
 * The display is not updated until SSD1306_Update() is called.
 *
 * @param x Horizontal pixel coordinate.
 * @param y Vertical pixel coordinate.
 * @param on true to set the pixel, false to clear it.
 */
void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool on);

/**
 * @brief Set the display contrast.
 *
 * Adjusts the OLED brightness by changing the SSD1306 contrast
 * control register.
 *
 * @param contrast Contrast value in the range 0-255.
 *
 * @return SSD1306_OK on success, error code otherwise.
 */
SSD1306_Status SSD1306_SetContrast(uint8_t contrast);

#endif
