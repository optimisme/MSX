#pragma bank 2

#include "g2048.h"
#include <string.h>   
#include <conio.h>

#define b_key               (vars_buff[0])
#define b_stick             (vars_buff[1])

#define BOARD_ROWS          4
#define BOARD_COLUMNS       4
#define BOARD_CELLS         (BOARD_ROWS * BOARD_COLUMNS)

#define BOARD_X0            65
#define BOARD_Y0            32
#define CELL_WIDTH          32
#define CELL_HEIGHT         32
#define CELL_EMPTY          0xFF

typedef struct {
    uint8_t value;   //  tile value (0,2,4,8…)
    uint8_t sprite;  //  sprite index to display that tile
} CellVars;

#define CELL_BASE_POS       2
#define CELL(r,c)           ((CellVars*)&vars_buff[CELL_BASE_POS + ((r)*BOARD_COLUMNS + (c))*2])
#define CELL_VALUE(r,c)     (CELL(r,c)->value)
#define CELL_SPRITE(r,c)    (CELL(r,c)->sprite)

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t start_x;
    uint8_t start_y;
    uint8_t dest_x;
    uint8_t dest_y;
    uint8_t frames;
    uint8_t frame;
} SpriteVars;

#define SPRITES_COUNT       16
#define SPRITE_BASE_POS     100
#define SPRITE(i)           ((SpriteVars*)&vars_buff[SPRITE_BASE_POS + (i)*8])
#define PATTERN_SLOT(i)     ((i) * 4)

#define S_SET_2048          (g2048_sprites_bitmap_spriteset[0])
#define S_SET_1024          (g2048_sprites_bitmap_spriteset[1])
#define S_SET_512           (g2048_sprites_bitmap_spriteset[2])
#define S_SET_256           (g2048_sprites_bitmap_spriteset[3])
#define S_SET_128           (g2048_sprites_bitmap_spriteset[4])
#define S_SET_64            (g2048_sprites_bitmap_spriteset[5])
#define S_SET_32            (g2048_sprites_bitmap_spriteset[6])
#define S_SET_16            (g2048_sprites_bitmap_spriteset[7])
#define S_SET_8             (g2048_sprites_bitmap_spriteset[8])
#define S_SET_4             (g2048_sprites_bitmap_spriteset[9])
#define S_SET_2             (g2048_sprites_bitmap_spriteset[10])
#define S_SET_0             (g2048_sprites_bitmap_spriteset[11])

void main_g2048() __banked {

    //random_set_seed();
    init_fps();
    init_game();

    while (1) {

        if (wait_fps()) continue;

        update_game();

        if (kbhit()) {
            b_key = cgetc();
            if (b_key == ' ' || b_key == '\n') {
            } else if (b_key == 'r') {
                restart_game();
            } else if (b_key == 0x1B || b_key == 'e') {
                return;
            }
        }
    }
}

void init_game() __banked {

    // Set tilemap to blank
    vdp_set_screen_mode(2);

    // Set tilemap to blank
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
    load_all_sprite_graphics();
    set_sprites_config(true, true);

    //vdp_update_sprite(0, PATTERN_SLOT(value_to_sprite_index(256)), COLOR_DARK_RED, 65, 32);
    //vdp_update_sprite(1, PATTERN_SLOT(value_to_sprite_index(4)), COLOR_DARK_GREEN, 129, 32);


    //vdp_update_sprite(2, PATTERN_SLOT(value_to_sprite_index(512)), COLOR_DARK_BLUE, 65, 64);
    //vdp_update_sprite(3, PATTERN_SLOT(value_to_sprite_index(64)), COLOR_DARK_YELLOW, 129, 64);

    restart_game();
}

void update_game() __banked {
    // ... el teu codi de dibuix posterior ...
}

void vdp_define_sprite16(uint8_t sprite_idx, const uint8_t data[32]) __banked
{
    uint8_t pat = sprite_idx * 4;
    uint16_t addr = 0x3800 + (pat << 3);
    for (uint8_t i = 0; i < 32; ++i) {
        msx_vpoke(addr + i, data[i]);
    }
}

void load_all_sprite_graphics(void) __banked {

    // Sprite patterns
    for (uint8_t i = 0; i < G2048_SPRITES_SPRITE_COUNT; ++i) {
        vdp_define_sprite16(i, g2048_sprites_bitmap_spriteset[i]);
    }
}

uint8_t random_cell_index() __banked {
    return random_8() % BOARD_CELLS;
}

uint8_t value_to_sprite_index(uint16_t value) __banked {
    switch (value) {
        case   2:   return 10;  // 2048
        case   4:   return 9;   // 1024
        case   8:   return 8;   // ...
        case  16:   return 7;
        case  32:   return 6;
        case  64:   return 5;
        case 128:   return 4;
        case 256:   return 3;
        case 512:   return 2;
        case 1024:  return 1;
        case 2048:  return 0;
        default:    return 11;   // fallback a “0”
    }
}

void restart_game(void) __banked {

    // 1) clear all cells
    for (uint8_t i = 0; i < BOARD_ROWS; ++i) {
        for (uint8_t j = 0; j < BOARD_COLUMNS; ++j) {
            CELL_VALUE(i,j)  = 0;
            CELL_SPRITE(i,j) = CELL_EMPTY;
        }
    }

    // 2) pick two distinct random cells
    uint8_t idx1 = random_cell_index();
    uint8_t idx2;
    do {
        idx2 = random_cell_index();
    } while (idx2 == idx1);

    // convert flat idx to (r,c)
    uint8_t r1 = idx1 / BOARD_COLUMNS, c1 = idx1 % BOARD_COLUMNS;
    uint8_t r2 = idx2 / BOARD_COLUMNS, c2 = idx2 % BOARD_COLUMNS;

    // 3) set value=2 and sprite for both
    CELL_VALUE(r1,c1)  = 2;
    CELL_SPRITE(r1,c1) = 0;

    CELL_VALUE(r2,c2)  = 2;
    CELL_SPRITE(r2,c2) = 1;

    for (uint8_t sprite_id = 0; sprite_id < 2; ++sprite_id) {
        uint8_t rr        = (sprite_id == 0 ? r1 : r2);
        uint8_t cc        = (sprite_id == 0 ? c1 : c2);
        uint8_t spr_idx   = value_to_sprite_index(CELL_VALUE(rr, cc));
        uint8_t pattern   = PATTERN_SLOT(spr_idx);

        uint8_t x = BOARD_X0 + cc * CELL_WIDTH;
        uint8_t y = BOARD_Y0 + rr * CELL_HEIGHT;

        uint8_t color = (sprite_id == 0 ? COLOR_DARK_RED : COLOR_DARK_GREEN);
        vdp_update_sprite(sprite_id, pattern, color, x, y);
    }

}
