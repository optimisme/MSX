#ifndef UTILS_MSX_H
#define UTILS_MSX_H

#include <msx.h>
#include <string.h>
#include <video/tms99x8.h> 

#define MODE_1_VRAM_PATTERN_BASE     0x0000  
#define MODE_1_TILEMAP_BASE          0x1800
#define MODE_1_COLOR_BASE            0x2000

#define MODE_1_PATTERN_BLOCK_SIZE    0x2000  // 8 KB max
#define MODE_1_VRAM_COLOR_BASE       0x0000  

#define MODE_2_VRAM_PATTERN_BASE     0x0000  
#define MODE_2_TILEMAP_BASE          0x1800
#define MODE_2_COLOR_BASE            0x2000
#define MODE_2_SPRITES_BASE          0x1B00
#define MODE_2_SPRITES_PATTERN_BASE  0x3800

#define MODE_2_PATTERN_BLOCK_SIZE    0x0800  // 2 KB = 256 patterns × 8 bytes
#define MODE_2_VRAM_COLOR_BASE       0x2000  // 3 blocs
#define MODE_2_COLOR_BLOCK_SIZE      0x0800  // 2 KB = 256 colors   × 8 bytes

#define MODE_2_CT_REG_BANK0  0x7F   // 0x2000 + 0*0x800
#define MODE_2_CT_REG_BANK1  0xFF   // 0x2000 + 1*0x800
#define MODE_2_CT_REG_BANK2  0xBF   // 0x2000 + 2*0x800

#define MODE_2_PT_REG_BANK0  0x03   // 0x0000 + 0*0x800
#define MODE_2_PT_REG_BANK1  0x07   // 0x0000 + 1*0x800
#define MODE_2_PT_REG_BANK2  0x0B   // 0x0000 + 2*0x800

#define MODE_3_VRAM_PATTERN_BASE     0x0000  
#define MODE_3_TILEMAP_BASE          0x0800

// Ports
#define VDP_DATA_PORT           0x98
#define VDP_CTRL_PORT           0x99

// Sprites
#define SPRITE_MAGNIFY_BIT  (1 << 0)  // bit 0: 0 = normal, 1 = magnify 2×
#define SPRITE_SIZE_BIT     (1 << 1)  // bit 1: 0 = 8×8, 1 = 16×16
#define SPRITE_VBLANK_BIT   (1 << 5)  // bit 5: enable V-Blank interrupt
#define SPRITE_DISPLAY_BIT  (1 << 6)  // bit 6: 1 = display on, 0 = blank
#define SPRITE_VRAM16K_BIT  (1 << 7)  // bit 7: 1 = 16 KB VRAM, 0 = 4 KB VRAM

// Stick values
#define STICK_NONE              0
#define STICK_UP                1
#define STICK_UP_RIGHT          2
#define STICK_RIGHT             3
#define STICK_DOWN_RIGHT        4
#define STICK_DOWN              5
#define STICK_DOWN_LEFT         6
#define STICK_LEFT              7
#define STICK_UP_LEFT           8

// MSX colors
#define COLOR_TRANSPARENT       0
#define COLOR_BLACK             1
#define COLOR_MEDIUM_GREEN      2
#define COLOR_LIGHT_GREEN       3
#define COLOR_DARK_BLUE         4
#define COLOR_LIGHT_BLUE        5
#define COLOR_DARK_RED          6
#define COLOR_CYAN              7
#define COLOR_MEDIUM_RED        8
#define COLOR_LIGHT_RED         9
#define COLOR_DARK_YELLOW       10
#define COLOR_LIGHT_YELLOW      11
#define COLOR_DARK_GREEN        12
#define COLOR_MAGENTA           13
#define COLOR_GRAY              14
#define COLOR_WHITE             15

extern void suspend_interrupts(void) __z88dk_callee;
extern void resume_interrupts(void) __z88dk_callee;
extern void do_nop(void) __z88dk_callee;
extern void vdp_write_byte(const unsigned char *src) __z88dk_fastcall;
extern uint8_t vdp_read_byte() __z88dk_callee;
extern void vdp_write_bytes_otir(const uint8_t *src, uint16_t len) __z88dk_callee;


/**
 * Sets the VDP write address to prepare for sequential VRAM output.
 *
 * Writes the specified VRAM address to VDP control ports (0x99),
 * preparing the VDP to accept data writes via port 0x98.
 *
 * The address is split across two writes with appropriate control bits.
 * A short delay (three NOPs) is included for VDP timing stability.
 *
 * @param addr 16-bit VRAM address to begin writing to.
 */
extern void vdp_set_address(unsigned int addr) __z88dk_fastcall;

/**
 * Writes an arbitrary number of bytes from RAM to VRAM via the VDP data port (0x98).
 *
 * The routine expects that the VRAM write address has already been programmed
 * with `vdp_set_address()`. After each byte is output, the VDP automatically
 * increments its internal address register, so the data are laid out
 * sequentially in VRAM.
 *
 * Internally the implementation is a tight Z80 loop that uses `OTIR`
 * (or `OUTI` when `len` < 256), making it suitable for copying buffers of
 * any size whose length is only known at run-time.
 *
 * @param src Pointer to the first byte to transfer from RAM.
 * @param len Number of bytes to write to VRAM.
 */
extern void vdp_write_bytes(const unsigned char *src, unsigned int len) __z88dk_callee;

/**
 * Writes exactly 32 bytes from RAM to VRAM through the VDP data port (0x98).
 *
 * Assumes that the VDP write address has already been set via `vdp_set_address`.
 * This function is optimized for writing a full 32-byte tile line (one screen row).
 *
 * @param src Pointer to 32 bytes of RAM to be copied into VRAM.
 */
extern void vdp_blast_line(const unsigned char* src) __z88dk_fastcall;

/**
 * Writes 768 bytes (32×24) from RAM to VRAM via the VDP data port (0x98).
 *
 * This function is designed to write an entire screen worth of tile indices
 * to the Name Table (typically at 0x1800 in SCREEN 2). The write address
 * must be set beforehand using `vdp_set_address`.
 *
 * @param src Pointer to 768 bytes of RAM to be transferred into VRAM.
 */
extern void vdp_blast_tilemap(const unsigned char* src) __z88dk_fastcall;


/**
 * Configure the video display processor (VDP) and set up the screen mode.
 *
 * This function sets the MSX video mode and, if using Mode 2,
 * forces the pattern and color table bases to the correct VRAM addresses.
 */
void vdp_set_screen_mode(uint8_t value);

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
void vdp_set_tile_pattern(uint8_t tile_bank, uint8_t tile_index, const uint8_t pattern_data[8]);

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
void vdp_set_tile_color(uint8_t tile_bank, uint8_t tile_index, uint8_t fg_color, uint8_t bg_color);

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
void vdp_set_tile_colors_per_rows(uint8_t tile_bank, uint8_t tile_index, const uint8_t fg_colors[8], const uint8_t bg_colors[8]);

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
void vdp_set_tile(uint8_t tile_bank, uint8_t tile_index, const uint8_t pattern_data[8], const uint8_t color_data[8]);

/**
 * Configure sprite size and magnification in VDP register 1.
 *
 * @param is_magnified   true = enable 2× magnification, false = normal size
 * @param is_double      true = use 16×16 sprites, false = use 8×8 sprites
 */
void set_sprites_config(bool is_magnified, bool is_double);

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
void vdp_set_sprite(uint8_t sprite_id, const uint8_t sprite_pattern[8], uint8_t pattern_index);
/**
 * Update sprite attributes: pattern index, color, and position.
 *
 * @param sprite_id        Sprite number (0–31)
 * @param pattern_index    Pattern index in the sprite pattern table
 * @param color_code       Color code for the sprite (0–15)
 * @param x_pos            X coordinate (0–255)
 * @param y_pos            Y coordinate (0–191)
 */
void vdp_update_sprite(uint8_t sprite_id, uint8_t pattern_index, uint8_t color_code, uint8_t x_pos, uint8_t y_pos);

/**
 * Write a null-terminated ASCII string to the screen in Mode 0/2.
 *
 * Only the character codes are written; color attributes remain unchanged.
 *
 * @param x_pos            Horizontal character coordinate (0–31)
 * @param y_pos            Vertical character coordinate (0–23)
 * @param text             Zero-terminated C-string to display
 */
void vdp_write_text(uint8_t x_pos, uint8_t y_pos, const char *text);

/**
 * Write a three-digit number to the screen in Mode 0/2.
 *
 * Leading zeros are replaced with spaces.
 *
 * @param x_pos            Horizontal character coordinate (0–31)
 * @param y_pos            Vertical character coordinate (0–23)
 * @param number           Value to display (0–999)
 */
void vdp_write_number(uint8_t x_pos, uint8_t y_pos, uint16_t number);

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
void vdp_write_text_color(uint8_t x_pos, uint8_t y_pos, const char *text, uint8_t fg_color, uint8_t bg_color);

void vdp_display_off(void);
void vdp_display_on(void);

#endif
