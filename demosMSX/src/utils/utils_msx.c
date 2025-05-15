#include "utils_msx.h"

/**
 * Initialize the video display processor (VDP) and set up screen mode.
 *
 * This function configures the MSX video mode and, if using Mode 2,
 * forces the pattern and color table bases to the correct VRAM addresses.
 */
void init_screen(void) {
    // Switch to the defined screen mode
    msx_screen(SCREEN_MODE);

#if SCREEN_MODE == 2
    vdp_set_reg(3, 0xFF); // Color base
    vdp_set_reg(4, 0x03); // Pattern base
#endif
}

/**
 * Load an 8-byte tile pattern into the VDP pattern generator table.
 *
 * In Mode 2, the pattern table is split into three 0x800-byte blocks,
 * so we replicate the tile data across all three banks for full coverage.
 * In other modes, only one block at PATTERN_TABLE_BASE is used.
 *
 * @param group 1 is top, 2 is middle, 3 is bottom of the screen
 * @param index Pattern index (0–255) where the tile will be stored.
 * @param tile  Pointer to an array of 8 bytes defining the 8×8 pattern.
 */
void init_tile(const unsigned char group, const unsigned char index, const unsigned char *tile) {
    unsigned char i;

#if SCREEN_MODE == 2
    // Graphics II: pattern table has 3 banks of 0x800
    unsigned int addr = VRAM_PATTERN_BASE + (group * PATTERN_BLOCK_SIZE) + (index << 3);
    vdp_set_address(addr);
    vdp_write_bytes(tile, 8);
#else
    // Other modes: single block at PATTERN_TABLE_BASE
    unsigned int addr = VRAM_PATTERN_BASE + (group * PATTERN_BLOCK_SIZE) + (index << 3);
    vdp_set_address(addr);
    vdp_write_bytes(tile, 8);
#endif
}

/**
 * Set the foreground and background colors for a given tile index.
 *
 * Writes an attribute byte (high nibble=FG, low nibble=BG) for
 * each of the 8 rows of the tile across all 3 color bank blocks.
 *
 * @param group 1 is top, 2 is middle, 3 is bottom of the screen
 * @param index Tile index (0–255) in the color table.
 * @param fg    Foreground color code (0–15).
 * @param bg    Background color code (0–15).
 */
void init_color_for_tile(const unsigned char group, const unsigned char index, const unsigned char fg, const unsigned char bg) {
    unsigned char col_line = (fg << 4) | bg;
    unsigned char buf[8];
    memset(buf, col_line, 8);

    unsigned int addr = VRAM_COLOR_BASE + (group * COLOR_BLOCK_SIZE) + (index << 3);
    vdp_set_address(addr);
    vdp_write_bytes(buf, 8);
}

/**
 * Set the color attributes de totes les 8 línies
 * d’un tile en SCREEN 2, rebent per separat
 * els colors de foreground i de background.
 *
 * @param group    1–3 banc de blocs (top, middle, bottom)
 * @param index    tile index (0–255)
 * @param fgCols   punter a 8 bytes: color de primer pla (0–15) per línia
 * @param bgCols   punter a 8 bytes: color de fons      (0–15) per línia
 */
void init_color_for_tile_lines(uint8_t group,
        uint8_t index,
        const uint8_t fgCols[8],
        const uint8_t bgCols[8]) {
    uint8_t buf[8];

    for (int i = 0; i < 8; ++i) {
        // cada byte: high nibble = fg, low nibble = bg
        buf[i] = (uint8_t)((fgCols[i] << 4) | (bgCols[i] & 0x0F));
    }

    unsigned int addr = VRAM_COLOR_BASE + group * (256 * 8) + (index << 3);
    vdp_set_address(addr);
    vdp_write_bytes(buf, 8);
}

/**
 * Initialize a hardware sprite with given pattern, color, and position.
 *
 * This function writes the 8-byte pattern to VRAM, sets sprite mode,
 * and updates the sprite attribute table (Y, X, pattern index, color).
 *
 * @param spr_id  Sprite number (0–31).
 * @param pattern Pointer to 8-byte sprite pattern.
 * @param pidx    Pattern index in sprite pattern table.
 */
void init_sprite(int spr_id,
    const unsigned char *pattern,
    unsigned char pidx)
{
    unsigned char i;
    unsigned int pat_addr = 0x3800 + (unsigned int)pidx * 8;
    unsigned int attr_addr = 0x1B00 + spr_id * 4;

    // 1) Write sprite pattern data to VRAM
    for (i = 0; i < 8; ++i) {
        msx_vpoke(pat_addr + i, pattern[i]);
    }

    // 2) Ensure sprites are in 8×8 mode
    msx_set_sprite_mode(0);
}

/**
 * Update sprite color and position.
 *
 * @param spr_id  Sprite number (0–31).
 * @param pidx    Pattern index in sprite pattern table.
 * @param color   Color code for the sprite (0–15).
 * @param x       X coordinate (0–255).
 * @param y       Y coordinate (0–191).
 */
void update_sprite(int spr_id,
    unsigned char pidx,
    unsigned char color,
    unsigned char x,
    unsigned char y) 
{
    unsigned int attr_addr = 0x1B00 + spr_id * 4;
    msx_vpoke(attr_addr + 0, y);
    msx_vpoke(attr_addr + 1, x);
    msx_vpoke(attr_addr + 2, pidx);
    msx_vpoke(attr_addr + 3, color);
}

/**
 * Writes an ASCII string to the screen in SCREEN 0/2 (Mode 0/2) using
 * the default color set by BIOS.
 *
 * @param x     Horizontal character coordinate (0‒31).
 * @param y     Vertical   character coordinate (0‒23).
 * @param text  Zero‑terminated C‑string to display.
 *
 * The routine calculates the VRAM address of the first character in the
 * MSX Name Table (VRAM 0x1800) and sends each byte with `msx_vpoke()`.
 * Only the character codes are written; color attributes remain unchanged.
 */
void put_text(unsigned char x, unsigned char y, const char *text)
{
#if SCREEN_MODE == 1
    unsigned int addr = 0x1800 + y * 32 + x;   /* start position in Name Table */
    while (*text) {
        msx_vpoke(addr++, *text++);            /* write char and advance */
    }
#endif
}

void put_number(unsigned char x, unsigned char y, unsigned int value)
{
#if SCREEN_MODE == 1
    unsigned char digits[4];
    unsigned int addr = 0x1800 + y * 32 + x;

    digits[0] = '0' + (value / 100);             // Centenes
    digits[1] = '0' + ((value / 10) % 10);       // Desenes
    digits[2] = '0' + (value % 10);              // Unitats

    // Si vols suprimir zeros a l'esquerra:
    if (digits[0] == '0') digits[0] = ' ';
    if (digits[0] == ' ' && digits[1] == '0') digits[1] = ' ';

    msx_vpoke(addr++, digits[0]);
    msx_vpoke(addr++, digits[1]);
    msx_vpoke(addr++, digits[2]);
#endif
}


/**
 * Writes an ASCII string to the screen in SCREEN 1 (Mode 1) and sets both
 * the character code and its color attribute.
 *
 * @param x     Horizontal character coordinate (0‒31).
 * @param y     Vertical   character coordinate (0‒23).
 * @param text  Zero‑terminated C‑string to display.
 * @param fg    Foreground (ink) color 0‒15.
 * @param bg    Background (paper) color 0‒15.
 *
 * Mode 1 stores color information per character, so for each symbol the
 * routine writes two bytes:
 *   • Name  Table byte  → character code
 *   • Color Table byte → 0xBGFG (high nibble = background, low = foreground)
 */
void put_text_color(unsigned char x, unsigned char y,
                    const char *text, unsigned char fg, unsigned char bg)
{
#if SCREEN_MODE == 1
    unsigned int name_addr  = 0x1800 + y * 32 + x;  /* Name  Table address */
    unsigned int color_addr = 0x2000 + y * 32 + x;  /* Color Table address */
    unsigned char attr = (bg << 4) | (fg & 0x0F);   /* packed BG/FG nibble */

    while (*text) {
        msx_vpoke(name_addr++,  *text++);           /* write character code */
        msx_vpoke(color_addr++, attr);              /* write color attribute */
    }
#endif
}

