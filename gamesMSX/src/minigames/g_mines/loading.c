#pragma bank 3

#include "loading.h"

void loading_init(uint8_t tile_x, uint8_t tile_y) {
    uint8_t tile_loading_bar[8]    = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
    uint8_t tile_loading_part[8]   = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    uint8_t tile_loading_part_0[8] = { 0x00,0x83,0x84,0x84,0x84,0x84,0xF3,0x00 };
    uint8_t tile_loading_part_1[8] = { 0x00,0x8E,0x51,0x51,0x5F,0x51,0x91,0x00 };
    uint8_t tile_loading_part_2[8] = { 0x00,0x79,0x44,0x44,0x44,0x44,0x79,0x00 };
    uint8_t tile_loading_part_3[8] = { 0x00,0xD1,0x99,0x95,0x93,0x91,0xD1,0x00 };
    uint8_t tile_loading_part_4[8] = { 0x00,0x3C,0x40,0x5C,0x44,0x44,0x3C,0x00 };

    for (uint8_t y = 0; y < 24; ++y) {
        for (uint8_t x = 0; x < 32; ++x) {
            msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)y * 32 + x, LOADING_BACK);
        }
    }

    for (uint8_t block = 0; block < 3; ++block) {
        vdp_set_tile_pattern(block, LOADING_BACK, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_BAR_EMPTY, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_BAR_FULL, tile_loading_bar);
        vdp_set_tile_pattern(block, LOADING_BAR_PART, tile_loading_part);
        vdp_set_tile_pattern(block, LOADING_PART_0, tile_loading_part_0);
        vdp_set_tile_pattern(block, LOADING_PART_1, tile_loading_part_1);
        vdp_set_tile_pattern(block, LOADING_PART_2, tile_loading_part_2);
        vdp_set_tile_pattern(block, LOADING_PART_3, tile_loading_part_3);
        vdp_set_tile_pattern(block, LOADING_PART_4, tile_loading_part_4);
    }

    for (uint8_t block = 0; block < 3; ++block) {
        vdp_set_tile_color(block, LOADING_BACK, COLOR_BLACK, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_BAR_EMPTY, COLOR_WHITE, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_BAR_FULL, COLOR_MEDIUM_GREEN, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_BAR_PART, COLOR_MEDIUM_GREEN, COLOR_WHITE);
        vdp_set_tile_color(block, LOADING_PART_0, COLOR_WHITE, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_PART_1, COLOR_WHITE, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_PART_2, COLOR_WHITE, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_PART_3, COLOR_WHITE, COLOR_BLACK);
        vdp_set_tile_color(block, LOADING_PART_4, COLOR_WHITE, COLOR_BLACK);
    }

    uint16_t pos = (uint16_t)tile_x + (uint16_t)tile_y * 32;
    msx_vpoke(MODE_2_TILEMAP_BASE + pos + 0, LOADING_PART_0);
    msx_vpoke(MODE_2_TILEMAP_BASE + pos + 1, LOADING_PART_1);
    msx_vpoke(MODE_2_TILEMAP_BASE + pos + 2, LOADING_PART_2);
    msx_vpoke(MODE_2_TILEMAP_BASE + pos + 3, LOADING_PART_3);
    msx_vpoke(MODE_2_TILEMAP_BASE + pos + 4, LOADING_PART_4);
}

void loading_draw_progress(uint8_t tile_start, uint8_t tile_end, uint8_t tile_y, uint8_t percentage) {
    uint8_t span = (uint8_t)(tile_end - tile_start);
    uint16_t total_pixels = (uint16_t)percentage * span * 8 / 100;
    uint8_t filled = (uint8_t)(total_pixels / 8);
    uint8_t remainder = (uint8_t)(total_pixels % 8);

    if (remainder > 0) {
        uint8_t part_pattern[8];
        uint8_t mask = (uint8_t)(0xFF << (8 - remainder));
        for (uint8_t i = 0; i < 8; ++i) {
            part_pattern[i] = mask;
        }
        for (uint8_t block = 0; block < 3; ++block) {
            vdp_set_tile_pattern(block, LOADING_BAR_PART, part_pattern);
        }
    }

    for (uint8_t x = 0; x < span; ++x) {
        uint8_t value;
        if (x < filled) {
            value = LOADING_BAR_FULL;
        } else if (x == filled && remainder > 0) {
            value = LOADING_BAR_PART;
        } else {
            value = LOADING_BAR_EMPTY;
        }
        msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)tile_y * 32 + (uint8_t)(tile_start + x), value);
    }
}
