#include "tiles.h"

/**
 * Tile 0 : diagonal white line on black
 * Each byte represents one row; MSB = left‑most pixel.
 * Bit 1 = foreground colour, bit 0 = background colour.
 */
const unsigned char bck_tile_0[8] = {
    0b10000000, 
    0b00001000, 
    0b10000000, 
    0b00001000, 
    0b10000000, 
    0b00001000, 
    0b10000000, 
    0b00001000  
};

/**
 * Tile 1 : red “X” on yellow background (colours set later)
 */
const unsigned char bck_tile_1[8] = {
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001
};

/**
 * Upload both patterns to the pattern generator table in VRAM.
 */
void init_tileset(void)
{
    for (unsigned char block = 0; block < 3; ++block) {
        init_tile(block, 0, bck_tile_0);
        init_tile(block, 1, bck_tile_1);
    }
}

/**
 * Write default colour attributes for the two tiles.
 *   high nibble = foreground, low nibble = background
 */
void init_colortable(void)
{
    for (unsigned char block = 0; block < 3; ++block) {
        init_color_for_tile(block, 0, COLOR_WHITE,     COLOR_BLACK);        
        init_color_for_tile(block, 1, COLOR_DARK_RED,  COLOR_LIGHT_YELLOW); 
    }
}

/**
 * Build the background:
 *   • Fill the whole NAME table with tile‑0
 *   • Overwrite the outer border (columns 0 & 31, rows 0 & 23)
 *     with tile‑1 to create a frame.
 */
void init_tilemap(void)
{
    for (unsigned char y = 0; y < 24; ++y) {
        for (unsigned char x = 0; x < 32; ++x) {

            /* Default tile everywhere */
            unsigned char tile = 0;

            /* Switch to border tile on the screen edges */
            if (x == 0 || x == 31 || y == 0 || y == 23)
                tile = 1;

            /* NAME_TABLE starts at 0x1800 (see constants_msx.h) */
            msx_vpoke(NAME_TABLE + y * 32 + x, tile);
        }
    }
}