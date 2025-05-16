#include "utils_msx.h"

/**
 * Configure the video display processor (VDP) and set up the screen mode.
 *
 * This function sets the MSX video mode and, if using Mode 2,
 * forces the pattern and color table bases to the correct VRAM addresses.
 */
void vdp_set_screen_mode(void) {
    // Switch to the defined screen mode
    msx_screen(SCREEN_MODE);

#if SCREEN_MODE == 2
    vdp_set_reg(3, 0xFF); // Set color table base
    vdp_set_reg(4, 0x03); // Set pattern table base
#endif
}

/**
 * Configure an 8-byte tile pattern in the VDP pattern generator table.
 *
 * In Mode 2, the pattern table is split into three 0x800-byte banks,
 * so the tile data is written to the appropriate bank based on tile_bank.
 * In other modes, only one block at VRAM_PATTERN_BASE is used.
 *
 * @param tile_bank        Bank number (0–3) representing vertical screen section
 * @param tile_index       Pattern index (0–255) where the tile will be stored
 * @param pattern_data     Pointer to an array of 8 bytes defining the 8×8 pattern
 */
void vdp_set_tile_pattern(uint8_t tile_bank,
                           uint8_t tile_index,
                           const uint8_t pattern_data[8])
{
    uint16_t offset = (tile_bank << 11) | (tile_index << 3);
    uint16_t address = VRAM_PATTERN_BASE + offset;

    vdp_set_address(address);
    vdp_write_bytes(pattern_data, 8);
}

/**
 * Configure a single color for all 8 rows of a given tile index.
 *
 * Writes an attribute byte (high nibble=foreground, low nibble=background)
 * for each of the 8 rows in the specified color bank.
 *
 * @param tile_bank        Bank number (0–3) representing vertical screen section
 * @param tile_index       Tile index (0–255) in the color table
 * @param fg_color         Foreground color code (0–15)
 * @param bg_color         Background color code (0–15)
 */
void vdp_set_tile_color(uint8_t tile_bank,
                         uint8_t tile_index,
                         uint8_t fg_color,
                         uint8_t bg_color)
{
    uint8_t attribute = (fg_color << 4) | (bg_color & 0x0F);
    uint8_t buffer[8];
    memset(buffer, attribute, 8);

    uint16_t offset = (tile_bank << 11) | (tile_index << 3);
    uint16_t address = VRAM_COLOR_BASE + offset;

    vdp_set_address(address);
    vdp_write_bytes(buffer, 8);
}

/**
 * Configure color attributes per row for a given tile index.
 *
 * Writes separate foreground and background color codes for each of the
 * 8 rows in the specified color bank.
 *
 * @param tile_bank        Bank number (0–3)
 * @param tile_index       Tile index (0–255)
 * @param fg_colors        Pointer to 8 bytes: foreground color per row
 * @param bg_colors        Pointer to 8 bytes: background color per row
 */
void vdp_set_tile_colors_per_rows(uint8_t tile_bank,
                                  uint8_t tile_index,
                                  const uint8_t fg_colors[8],
                                  const uint8_t bg_colors[8])
{
    uint8_t buffer[8];

    for (int i = 0; i < 8; ++i) {
        buffer[i] = (uint8_t)((fg_colors[i] << 4) | (bg_colors[i] & 0x0F));
    }

    uint16_t offset = (tile_bank << 11) | (tile_index << 3);
    uint16_t address = VRAM_COLOR_BASE + offset;

    vdp_set_address(address);
    vdp_write_bytes(buffer, 8);
}

/**
 * Upload both pattern and color data for a tile to VRAM in one call.
 *
 * This function writes the 8-byte pattern and the 8-byte color attributes
 * to the corresponding banks in VRAM based on the tile bank and index.
 *
 * @param tile_bank        Bank number (0–3)
 * @param tile_index       Tile index (0–255)
 * @param pattern_data     Pointer to 8 bytes of pattern data
 * @param color_data       Pointer to 8 bytes of color attribute data
 */
void vdp_set_tile(uint8_t tile_bank,
                     uint8_t tile_index,
                     const uint8_t pattern_data[8],
                     const uint8_t color_data[8])
{
    uint16_t offset = (tile_bank << 11) | (tile_index << 3);

    vdp_set_address(VRAM_PATTERN_BASE + offset);
    vdp_write_bytes(pattern_data, 8);

    vdp_set_address(VRAM_COLOR_BASE + offset);
    vdp_write_bytes(color_data, 8);
}

/**
 * Configure a hardware sprite with given pattern and default settings.
 *
 * This function writes the 8-byte sprite pattern to VRAM and ensures
 * sprites are in 8×8 pixel mode.
 *
 * @param sprite_id        Sprite number (0–31)
 * @param sprite_pattern   Pointer to 8-byte sprite pattern data
 * @param pattern_index    Pattern index in the sprite pattern table
 */
void vdp_set_sprite(uint8_t sprite_id,
                           const uint8_t sprite_pattern[8],
                           uint8_t pattern_index)
{
    uint16_t pattern_address = 0x3800 + (pattern_index << 3);

    // Write sprite pattern data to VRAM
    for (uint8_t i = 0; i < 8; ++i) {
        msx_vpoke(pattern_address + i, sprite_pattern[i]);
    }

    // Ensure sprites are in 8×8 mode
    msx_set_sprite_mode(0);
}

/**
 * Update sprite attributes: pattern index, color, and position.
 *
 * @param sprite_id        Sprite number (0–31)
 * @param pattern_index    Pattern index in the sprite pattern table
 * @param color_code       Color code for the sprite (0–15)
 * @param x_pos            X coordinate (0–255)
 * @param y_pos            Y coordinate (0–191)
 */
void vdp_update_sprite(uint8_t sprite_id,
                       uint8_t pattern_index,
                       uint8_t color_code,
                       uint8_t x_pos,
                       uint8_t y_pos)
{
    uint16_t attr_address = 0x1B00 + (sprite_id << 2);

    msx_vpoke(attr_address + 0, y_pos);
    msx_vpoke(attr_address + 1, x_pos);
    msx_vpoke(attr_address + 2, pattern_index);
    msx_vpoke(attr_address + 3, color_code);
}

/**
 * Write a null-terminated ASCII string to the screen in Mode 0/2.
 *
 * Only the character codes are written; color attributes remain unchanged.
 *
 * @param x_pos            Horizontal character coordinate (0–31)
 * @param y_pos            Vertical character coordinate (0–23)
 * @param text             Zero-terminated C-string to display
 */
void vdp_write_text(uint8_t x_pos, uint8_t y_pos, const char *text)
{
#if SCREEN_MODE == 0 || SCREEN_MODE == 2
    uint16_t name_address = 0x1800 + (y_pos * 32) + x_pos;
    while (*text) {
        msx_vpoke(name_address++, (uint8_t)*text++);
    }
#endif
}

/**
 * Write a three-digit number to the screen in Mode 0/2.
 *
 * Leading zeros are replaced with spaces.
 *
 * @param x_pos            Horizontal character coordinate (0–31)
 * @param y_pos            Vertical character coordinate (0–23)
 * @param number           Value to display (0–999)
 */
void vdp_write_number(uint8_t x_pos, uint8_t y_pos, uint16_t number)
{
#if SCREEN_MODE == 0 || SCREEN_MODE == 2
    char digits[4] = {0};
    uint16_t address = 0x1800 + (y_pos * 32) + x_pos;

    digits[0] = '0' + (number / 100);
    digits[1] = '0' + ((number / 10) % 10);
    digits[2] = '0' + (number % 10);

    // Replace leading zeros with spaces
    if (digits[0] == '0') digits[0] = ' ';
    if (digits[0] == ' ' && digits[1] == '0') digits[1] = ' ';

    msx_vpoke(address++, digits[0]);
    msx_vpoke(address++, digits[1]);
    msx_vpoke(address++, digits[2]);
#endif
}

/**
 * Write a colored ASCII string to the screen in Mode 1.
 *
 * For each character, both the code and its color attribute are written.
 *
 * @param x_pos            Horizontal character coordinate (0–31)
 * @param y_pos            Vertical character coordinate (0–23)
 * @param text             Zero-terminated C-string to display
 * @param fg_color         Foreground (ink) color code (0–15)
 * @param bg_color         Background (paper) color code (0–15)
 */
void vdp_write_text_color(uint8_t x_pos,
                          uint8_t y_pos,
                          const char *text,
                          uint8_t fg_color,
                          uint8_t bg_color)
{
#if SCREEN_MODE == 1
    uint16_t name_address  = 0x1800 + (y_pos * 32) + x_pos;
    uint16_t color_address = 0x2000 + (y_pos * 32) + x_pos;
    uint8_t attribute      = (bg_color << 4) | (fg_color & 0x0F);

    while (*text) {
        msx_vpoke(name_address++, (uint8_t)*text++);
        msx_vpoke(color_address++, attribute);
    }
#endif
}
