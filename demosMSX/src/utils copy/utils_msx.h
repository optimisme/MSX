#ifndef UTILS_MSX_H
#define UTILS_MSX_H

#include <msx.h>
#include <string.h>
#include <video/tms99x8.h> 

#if !defined(SCREEN_MODE)
  #error "Define SCREEN_MODE (0, 1 o 2) at compile time with '-DSCREEN_MODE={0|1|2}'"
#elif (SCREEN_MODE < 0) || (SCREEN_MODE > 2)
  #error "SCREEN_MODE can only be 0, 1 o 2."
#endif

#if SCREEN_MODE == 1

  #define VRAM_PATTERN_BASE     0x0000  
  #define PATTERN_BLOCK_SIZE    0x2000  // 8 KB max
  #define VRAM_COLOR_BASE       0x0000  
  #define NAME_TABLE            0x1800
  #define COLOR_TABLE           0x2000

#elif SCREEN_MODE == 2

  #define VRAM_PATTERN_BASE     0x0000  // 3 blocs
  #define PATTERN_BLOCK_SIZE    0x0800  // 2 KB = 256 patterns × 8 bytes
  #define VRAM_COLOR_BASE       0x2000  // 3 blocs
  #define COLOR_BLOCK_SIZE      0x0800  // 2 KB = 256 colors   × 8 bytes
  #define NAME_TABLE            0x1800
  #define COLOR_TABLE           0x2000

  #define CT_REG_BANK0  0x7F   // 0x2000 + 0*0x800
  #define CT_REG_BANK1  0xFF   // 0x2000 + 1*0x800
  #define CT_REG_BANK2  0xBF   // 0x2000 + 2*0x800
  #define PT_REG_BANK0  0x03   // 0x0000 + 0*0x800
  #define PT_REG_BANK1  0x07   // 0x0000 + 1*0x800
  #define PT_REG_BANK2  0x0B   // 0x0000 + 2*0x800

#endif

// Ports
#define VDP_DATA_PORT           0x98
#define VDP_CTRL_PORT           0x99

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
 * Initialize the video display processor (VDP) and set up screen mode.
 *
 * Configures the MSX screen mode and adjusts VDP registers for Mode 2.
 */
void init_screen(void);

/**
 * Load an 8-byte tile pattern into the VDP pattern generator table.
 *
 * In Mode 2, replicates the pattern across three banks of 0x800 bytes.
 * In other modes, writes to the base PATTERN_TABLE.
 *
 * @param group 1 is top, 2 is middle, 3 is bottom of the screen.
 * @param index Pattern index (0–255) where the tile will be stored.
 * @param tile  Pointer to an array of 8 bytes defining the pattern.
 */
void init_tile(const unsigned char group, const unsigned char index, const unsigned char *tile);

/**
 * Set the foreground/background colors for a specific tile index.
 *
 * Writes an attribute byte (FG in high nibble, BG in low nibble) for
 * each of the 8 rows of the tile across all three color table banks.
 *
 * @param group 1 is top, 2 is middle, 3 is bottom of the screen
 * @param index Tile index (0–255) in the color table.
 * @param fg    Foreground color code (0–15).
 * @param bg    Background color code (0–15).
 */
void init_color_for_tile(const unsigned char group, const unsigned char index, const unsigned char fg, const unsigned char bg);

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
void init_color_for_tile_lines(uint8_t group, uint8_t index, const uint8_t fgCols[8], const uint8_t bgCols[8]);

/**
 * Initialize a hardware sprite with given pattern, color, and position.
 *
 * Writes the 8-byte sprite pattern to VRAM, sets sprite mode,
 * and updates the sprite attribute table (Y, X, pattern, color).
 *
 * @param spr_id  Sprite number (0–31).
 * @param pattern Pointer to 8-byte sprite pattern data.
 * @param pidx    Pattern index in sprite pattern table.
 */
void init_sprite(int spr_id, const unsigned char *pattern, unsigned char pidx);

/**
 * Update sprite color and position.
 *
 * @param spr_id  Sprite number (0–31).
 * @param pidx    Pattern index in sprite pattern table.
 * @param color   Color code for the sprite (0–15).
 * @param x       X coordinate (0–255).
 * @param y       Y coordinate (0–191).
 */
void update_sprite(int spr_id, unsigned char pidx, unsigned char color, unsigned char x, unsigned char y) ;

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
void put_text(unsigned char x, unsigned char y, const char *text);

/**
 * Writes a number to the screen in SCREEN 0/2 (Mode 0/2) using
 * the default color set by BIOS.
 *
 * @param x     Horizontal character coordinate (0‒31).
 * @param y     Vertical   character coordinate (0‒23).
 * @param value  integer value to display.
 *
 * The routine calculates the VRAM address of the first character in the
 * MSX Name Table (VRAM 0x1800) and sends each byte with `msx_vpoke()`.
 * Only the character codes are written; color attributes remain unchanged.
 */
void put_number(unsigned char x, unsigned char y, unsigned int value);

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
void put_text_color(unsigned char x, unsigned char y, const char *text, unsigned char fg, unsigned char bg);

#endif
