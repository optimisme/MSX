#pragma bank 2

#include "g2048.h"
#include <string.h>    // per memset
#include <conio.h>

const unsigned char sprite_2[8] = {
    0b00111100,
    0b01000010,
    0b00000010,
    0b00001100,
    0b00110000,
    0b01000000,
    0b01000000,
    0b01111110
};
const unsigned char sprite_4[8] = {
    0b00011100,
    0b00100100,
    0b01000100,
    0b10000100,
    0b10000100,
    0b11111110,
    0b00000100,
    0b00000100
};

const unsigned char sprite_8[8] = {
    0b00111100,
    0b01000010,
    0b01000010,
    0b00110100,
    0b00101100,
    0b01000010,
    0b01000010,
    0b00111100
};

const unsigned char sprite_20[8] = {
    0b01000010,
    0b10100101,
    0b00100101,
    0b01000101,
    0b01000101,
    0b10000101,
    0b10000101,
    0b11100010
};

const unsigned char sprite_48[8] = {
    0b00100010,
    0b01100101,
    0b01100101,
    0b10100010,
    0b10100010,
    0b11110101,
    0b00100101,
    0b00100010
};

void vdp_define_sprite16(uint8_t pattern_index,
                         const uint8_t data[32]) __banked
{
    uint16_t addr = 0x3800 + (pattern_index << 3);
    for (uint8_t i = 0; i < 32; ++i) {
        msx_vpoke(addr + i, data[i]);
    }
}


void main_g2048() __banked {
    init_fps();
    init_game();
    cgetc();
}

void init_game() __banked {

    // Set tilemap to blank
    vdp_set_screen_mode(2);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

    // Set patterns
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        for (uint16_t i = 0; i < G2048_BITMAP_TILE_COUNT; ++i) {
            uint16_t global_idx = bank_base + i;
            const uint8_t *src = &g2048_bitmap_tileset[i][0];
            uint8_t *dst = &vdp_global_buff[global_idx * 8];
            memcpy(dst, src, 8);
        }
    }
    vdp_set_address(MODE_2_VRAM_PATTERN_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    // Set colors
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        for (uint16_t i = 0; i < G2048_BITMAP_TILE_COUNT; ++i) {
            uint16_t global_idx = bank_base + i;
            for (uint8_t y = 0; y < 8; ++y) {
                vdp_global_buff[global_idx * 8 + y] = (COLOR_WHITE << 4) | COLOR_BLACK;
            }
        }
    }
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    // Set tilemap
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(g2048_bitmap_tilemap);

    // Set sprite boxes
    vdp_define_sprite16(0, sprite_48);
    vdp_update_sprite(0, 0, COLOR_DARK_RED, 65, 32);
}

void update_game() __banked {
    // ... el teu codi de dibuix posterior ...
}
