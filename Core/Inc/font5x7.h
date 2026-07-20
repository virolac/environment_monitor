#ifndef FONT5X7_H
#define FONT5X7_H

#include <stdint.h>
#include <stdbool.h>

#define FONT5X7_WIDTH 5U
#define FONT5X7_HEIGHT 7U

/**
 * @brief Checks whether a character is supported by the font.
 *
 * @param c Character to check.
 *
 * @return true if the character has a glyph in the font, false otherwise.
 */
bool Font5x7_IsCharacterSupported(char c);

/**
 * @brief Retrieves the bitmap data for an ASCII character.
 *
 * The returned glyph consists of FONT5X7_WIDTH bytes, where each byte
 * represents one vertical column of the character. The least significant
 * seven bits correspond to the character rows.
 *
 * Only printable ASCII characters (' ' through '~') are supported.
 * Unsupported characters are replaced with '?'.
 *
 * @param c ASCII character to retrieve.
 *
 * @return Pointer to the glyph bitmap stored in font data.
 */
const uint8_t *Font5x7_GetGlyph(char c);

#endif /* FONT5X7_H */
