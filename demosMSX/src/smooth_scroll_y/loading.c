#include "loading.h"

/**
 * Initializes loading screen
 * @param tile_x   x tile position for "Loading" text
 * @param tile_y   y tile position for "Loading" text
 */
void loading_init(uint8_t tile_x, uint8_t tile_y) {

    uint8_t tile_loading_bar[8]    = { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 }; 
    uint8_t tile_loading_part_0[8] = { 0b00000000, 0b10000011, 0b10000100, 0b10000100, 0b10000100, 0b10000100, 0b11110011, 0b00000000 };
    uint8_t tile_loading_part_1[8] = { 0b00000000, 0b10001110, 0b01010001, 0b01010001, 0b01011111, 0b01010001, 0b10010001, 0b00000000 };
    uint8_t tile_loading_part_2[8] = { 0b00000000, 0b01111001, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b01111001, 0b00000000 };
    uint8_t tile_loading_part_3[8] = { 0b00000000, 0b11010001, 0b10011001, 0b10010101, 0b10010011, 0b10010001, 0b11010001, 0b00000000 };
    uint8_t tile_loading_part_4[8] = { 0b00000000, 0b00111100, 0b01000000, 0b01011100, 0b01000100, 0b01000100, 0b00111100, 0b00000000 };

    // Empty tilemap
    for (unsigned char y = 0; y < 24; ++y) {
        for (unsigned char x = 0; x < 32; ++x) {
            msx_vpoke(NAME_TABLE + y * 32 + x, LOADING_BACK);
        }
    }

    // Set loading tiles
    for (unsigned char block = 0; block < 3; ++block) {
        vdp_set_tile_pattern(block, LOADING_BACK, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_BAR_EMPTY, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_BAR_FULL, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_PART_0, tile_loading_part_0);
        vdp_set_tile_pattern(block, LOADING_PART_1, tile_loading_part_1);
        vdp_set_tile_pattern(block, LOADING_PART_2, tile_loading_part_2);
        vdp_set_tile_pattern(block, LOADING_PART_3, tile_loading_part_3);
        vdp_set_tile_pattern(block, LOADING_PART_4, tile_loading_part_4);
    }

    // Set loading tile colors
    for (unsigned char block = 0; block < 3; ++block) {
        vdp_set_tile_color(block, LOADING_BACK, COLOR_BLACK, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_BAR_EMPTY, COLOR_WHITE, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_BAR_FULL, COLOR_MEDIUM_GREEN, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_PART_0, COLOR_WHITE, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_PART_1, COLOR_WHITE, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_PART_2, COLOR_WHITE, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_PART_3, COLOR_WHITE, COLOR_BLACK); 
        vdp_set_tile_color(block, LOADING_PART_4, COLOR_WHITE, COLOR_BLACK); 
    }

    // Set "LOADING..." text tiles
    int pos = tile_x + tile_y * 32;
    msx_vpoke(NAME_TABLE + pos + 0, LOADING_PART_0);
    msx_vpoke(NAME_TABLE + pos + 1, LOADING_PART_1);
    msx_vpoke(NAME_TABLE + pos + 2, LOADING_PART_2);
    msx_vpoke(NAME_TABLE + pos + 3, LOADING_PART_3);
    msx_vpoke(NAME_TABLE + pos + 4, LOADING_PART_4);
}

/**
 * Draw a horizontal progress bar using tile values.
 *
 * Tiles from tile_start..tile_end-1 on row tile_y are set:
 *   – value 2 for the first N tiles, where N = percentage% of the span
 *   – value 1 for the rest
 *
 * @param tile_start   First tile X index (inclusive)
 * @param tile_end     Last tile X index (exclusive)
 * @param tile_y       Tile row Y index
 * @param percentage   Fill percentage (0–100)
 */
void loading_draw_progress(uint8_t tile_start, uint8_t tile_end, uint8_t tile_y, uint8_t percentage)
{
    uint8_t span = tile_end - tile_start;
    /* how many tiles to fill as “2” */
    uint8_t filled = (uint16_t)percentage * span / 100;

    for (uint8_t x = tile_start; x < tile_end; ++x) {
        /* if this tile is within the filled count, use 2, else 1 */
        uint8_t value = (x - tile_start < filled) ?  LOADING_BAR_FULL :  LOADING_BAR_EMPTY;
        msx_vpoke(NAME_TABLE + tile_y * 32 + x, value);
    }
}