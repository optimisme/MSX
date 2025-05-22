#pragma bank 2

#include "g2048.h"
#include <string.h>    // per memset
#include <conio.h>

#define b_key               (vars_buff[0])
#define b_stick             (vars_buff[1])

void main_g2048() __banked {

    init_fps();
    init_game();

    while (1) {

        if (wait_fps()) continue;

        update_game();

        if (kbhit()) {
            b_key = cgetc();
            if (b_key == ' ' || b_key == '\n') {
            } else if (b_key == 0x1B || b_key == 'e') {
                return;
            }
        }
    }
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
    set_sprites_config(true, true);
    vdp_define_sprite16(0,  g2048_sprites_bitmap_spriteset[5]);
    vdp_update_sprite(0,  0, COLOR_DARK_RED, 65, 32);
}

void vdp_define_sprite16(uint8_t pattern_index, const uint8_t data[32]) __banked {
    uint16_t addr = 0x3800 + (pattern_index << 3);
    for (uint8_t i = 0; i < 32; ++i) {
        msx_vpoke(addr + i, data[i]);
    }
}

void update_game() __banked {
    // ... el teu codi de dibuix posterior ...
}
