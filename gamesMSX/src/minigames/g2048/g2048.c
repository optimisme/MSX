#pragma bank 2

#include "g2048.h"
#include <string.h>   
#include <conio.h>

#define b_key               (vars_buff[0])
#define b_stick             (vars_buff[1])
#define b_game_state        (vars_buff[2])

#define BOARD_ROWS          4
#define BOARD_COLUMNS       4
#define BOARD_CELLS         (BOARD_ROWS * BOARD_COLUMNS)

#define CELL_BASE_POS       10
#define CELL(r,c)           ((uint8_t*)&vars_buff[CELL_BASE_POS + ((r)*BOARD_COLUMNS + (c))])
#define CELL_VALUE(r,c)     (*CELL(r,c))

#define BOARD_X0            65
#define BOARD_Y0            32
#define CELL_WIDTH          32
#define CELL_HEIGHT         32

#define ANIMATION_MS        250
#define GAME_STATE_PLAYING  0
#define GAME_STATE_MOVING   1
#define GAME_STATE_SPAWNING 2

#define DIR_LEFT    0
#define DIR_RIGHT   1  
#define DIR_UP      2
#define DIR_DOWN    3

#define SPRITES_VARS_COUNT  9
#define SPRITES_COUNT       16
#define SPRITE_BASE_POS     100
#define SPRITE(i)           ((SPRITE_t*)&vars_buff[SPRITE_BASE_POS + (i)*SPRITES_VARS_COUNT])
#define PATTERN_SLOT(i)     ((i) * 4)

void main_g2048() __banked {
    init_fps();
    init_game();

    while (1) {
        if (wait_fps()) continue;
        handle_input();
        update_game();
    }
}

void init_game() __banked {
    vdp_set_screen_mode(2);

    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

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

    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(g2048_bitmap_tilemap);

    load_sprite_patterns();
    set_sprites_config(true, true);

    restart_game();
}

void handle_input() __banked {
    if (b_game_state != GAME_STATE_PLAYING) return;
    if (kbhit()) {
        b_key = cgetc();
        if (b_key == 'r') {
            restart_game();
        } else if (b_key == 0x1B || b_key == 'e') {
            return;
        }
    }
    b_stick = msx_get_stick(0);
    switch (b_stick) {
        case STICK_UP: move_tiles(DIR_UP); break;
        case STICK_RIGHT: move_tiles(DIR_RIGHT); break;
        case STICK_DOWN: move_tiles(DIR_DOWN); break;
        case STICK_LEFT: move_tiles(DIR_LEFT); break;

    }
}

void apply_post_move(void) __banked {
    for (uint8_t i = 0; i < SPRITES_COUNT; ++i) {
        uint8_t row = i / BOARD_COLUMNS;
        uint8_t col = i % BOARD_COLUMNS;
        
        SPRITE(i)->x = BOARD_X0 + col * CELL_WIDTH;
        SPRITE(i)->y = BOARD_Y0 + row * CELL_HEIGHT;
        SPRITE(i)->start_x = SPRITE(i)->x;
        SPRITE(i)->start_y = SPRITE(i)->y;
        SPRITE(i)->dest_x = SPRITE(i)->x;
        SPRITE(i)->dest_y = SPRITE(i)->y;
        SPRITE(i)->frames = 0;
        SPRITE(i)->frame = 0;
        SPRITE(i)->value = CELL_VALUE(row, col);
        
        vdp_update_sprite(i,
            PATTERN_SLOT(value_to_sprite_index(SPRITE(i)->value)),
            value_to_color(SPRITE(i)->value),
            SPRITE(i)->x, SPRITE(i)->y);
    }
}

void update_game() __banked {
    if (b_game_state == GAME_STATE_MOVING) {
        if (!update_animations()) {
            apply_post_move();
            b_game_state = GAME_STATE_SPAWNING;
        }
    } else if (b_game_state == GAME_STATE_SPAWNING) {
        if (spawn_random_tile()) {
            b_game_state = GAME_STATE_PLAYING;
        } else {
            // No s'ha pogut afegir una nova peça
            if (!board_has_moves()) {
                // Game Over - podries afegir aquí la lògica de fi de joc
                b_game_state = GAME_STATE_PLAYING; // o un nou estat GAME_OVER
            } else {
                b_game_state = GAME_STATE_PLAYING;
            }
        }
    }
}

void load_sprite_patterns(void) __banked {
    for (uint8_t sprite_idx = 0; sprite_idx < G2048_SPRITES_SPRITE_COUNT; ++sprite_idx) {
        uint8_t pat = sprite_idx * 4; 
        uint16_t addr = MODE_2_SPRITES_PATTERN_BASE + (pat << 3); 
        for (uint8_t d = 0; d < 32; ++d) {
            msx_vpoke(addr + d, g2048_sprites_bitmap_spriteset[sprite_idx][d]);
        }
    }
}

uint8_t get_random_sprite_index() __banked {
    return random_8() % SPRITES_COUNT;
}

uint8_t value_to_sprite_index(uint16_t value) __banked {
    switch (value) {
        case   2:   return 10;  
        case   4:   return 9;  
        case   8:   return 8;  
        case  16:   return 7;
        case  32:   return 6;
        case  64:   return 5;
        case 128:   return 4;
        case 256:   return 3;
        case 512:   return 2;
        case 1024:  return 1;
        case 2048:  return 0;
        default:    return 11;
    }
}

uint8_t value_to_color(uint16_t value) __banked {
    switch (value) {
        case   2:   return COLOR_LIGHT_GREEN;
        case   4:   return COLOR_MEDIUM_GREEN;
        case   8:   return COLOR_DARK_GREEN;
        case  16:   return COLOR_CYAN;
        case  32:   return COLOR_LIGHT_BLUE;
        case  64:   return COLOR_DARK_BLUE;
        case 128:   return COLOR_DARK_YELLOW;
        case 256:   return COLOR_LIGHT_RED;
        case 512:   return COLOR_MEDIUM_RED;
        case 1024:  return COLOR_DARK_RED;
        case 2048:  return COLOR_MAGENTA;
        default:    return COLOR_BLACK;
    }
}

void reset_sprite(uint8_t i) __banked {
    uint8_t value = 0;
    uint8_t pattern = PATTERN_SLOT(value_to_sprite_index(value)); 
    uint8_t color = value_to_color(value);

    uint8_t row = i / BOARD_COLUMNS;
    uint8_t col = i % BOARD_COLUMNS;
    SPRITE(i)->x = BOARD_X0 + col * CELL_WIDTH;
    SPRITE(i)->y = BOARD_Y0 + row * CELL_HEIGHT;
    SPRITE(i)->start_x = SPRITE(i)->x;
    SPRITE(i)->start_y = SPRITE(i)->y;
    SPRITE(i)->dest_x = SPRITE(i)->x;
    SPRITE(i)->dest_y = SPRITE(i)->y;
    SPRITE(i)->frames = 0;
    SPRITE(i)->frame = 0;
    SPRITE(i)->value = value;
    vdp_update_sprite(i, pattern, color, SPRITE(i)->x, SPRITE(i)->y);
}

void restart_game(void) __banked {
    for (uint8_t r = 0; r < BOARD_ROWS; ++r) {
        for (uint8_t c = 0; c < BOARD_COLUMNS; ++c) {
            CELL_VALUE(r, c) = 0;
        }
    }

    for (uint8_t i = 0; i < SPRITES_COUNT; ++i) {
        reset_sprite(i);
    }

    uint8_t idx1 = get_random_sprite_index();
    uint8_t idx2;
    do {
        idx2 = get_random_sprite_index();
    } while (idx2 == idx1);

    uint8_t value = 2;
    uint8_t pattern = PATTERN_SLOT(value_to_sprite_index(value)); 
    uint8_t color = value_to_color(value);

    SPRITE(idx1)->value = value;
    vdp_update_sprite(idx1, pattern, color, SPRITE(idx1)->x, SPRITE(idx1)->y);
    uint8_t row1 = idx1 / BOARD_COLUMNS;
    uint8_t col1 = idx1 % BOARD_COLUMNS;
    CELL_VALUE(row1, col1) = value;

    SPRITE(idx2)->value = value;
    vdp_update_sprite(idx2, pattern, color, SPRITE(idx2)->x, SPRITE(idx2)->y);
    uint8_t row2 = idx2 / BOARD_COLUMNS;
    uint8_t col2 = idx2 % BOARD_COLUMNS;
    CELL_VALUE(row2, col2) = value;

    b_game_state = GAME_STATE_PLAYING;
}

void move_tiles(uint8_t direction) __banked {
    uint8_t moved = 0;
    for (uint8_t j = 0; j < BOARD_COLUMNS; ++j) {
        switch (direction) {
            case DIR_UP:    if (move_col_up(j))    moved = 1; break;
            case DIR_DOWN:  if (move_col_down(j))  moved = 1; break;
        }
    }
    for (uint8_t i = 0; i < BOARD_ROWS; ++i) {
        switch (direction) {
            case DIR_LEFT:  if (move_row_left(i))  moved = 1; break;
            case DIR_RIGHT: if (move_row_right(i)) moved = 1; break;
        }
    }
    if (moved) b_game_state = GAME_STATE_MOVING;
}

uint8_t move_col_up(uint8_t c) __banked {
    uint8_t tmp[4], write = 0, moved = 0, last_merge = -1;
    // copy & clear
    for (uint8_t r = 0; r < 4; ++r) {
        tmp[r] = CELL_VALUE(r, c);
        CELL_VALUE(r, c) = 0;
    }
    // merge/move up
    for (uint8_t i = 0; i < 4; ++i) {
        uint8_t val = tmp[i];
        if (!val) continue;
        uint8_t src = i * BOARD_COLUMNS + c;
        if (write > 0
            && CELL_VALUE(write-1, c) == val
            && last_merge != write-1) {
            // merge
            CELL_VALUE(write-1, c) *= 2;
            last_merge = write-1;
            set_sprite_destination(SPRITE(src),
                BOARD_X0 + c*CELL_WIDTH,
                BOARD_Y0 + (write-1)*CELL_HEIGHT,
                ANIMATION_MS);
            moved = 1;
        } else {
            // move
            CELL_VALUE(write, c) = val;
            if (i != write) {
                moved = 1;
                set_sprite_destination(SPRITE(src),
                    BOARD_X0 + c*CELL_WIDTH,
                    BOARD_Y0 + write*CELL_HEIGHT,
                    ANIMATION_MS);
            }
            write++;
        }
    }
    return moved;
}

uint8_t move_row_right(uint8_t r) __banked {
    uint8_t tmp_vals[4];
    uint8_t write = BOARD_COLUMNS - 1;
    uint8_t moved = 0;
    int8_t last_merge = -1;

    // copy and clear
    for (uint8_t c = 0; c < 4; ++c) {
        tmp_vals[c] = CELL_VALUE(r, c);
        CELL_VALUE(r, c) = 0;
    }

    // merge/move right
    for (int8_t i = BOARD_COLUMNS - 1; i >= 0; --i) {
        uint8_t val = tmp_vals[i];
        if (!val) continue;
        uint8_t src = r * BOARD_COLUMNS + i;

        if (write < BOARD_COLUMNS - 1
            && CELL_VALUE(r, write + 1) == val
            && last_merge != write + 1) {
            CELL_VALUE(r, write + 1) *= 2;
            last_merge = write + 1;
            set_sprite_destination(SPRITE(src),
                BOARD_X0 + (write + 1)*CELL_WIDTH,
                BOARD_Y0 + r*CELL_HEIGHT,
                ANIMATION_MS);
            moved = 1;
        } else {
            CELL_VALUE(r, write) = val;
            if (i != write) {
                moved = 1;
                set_sprite_destination(SPRITE(src),
                    BOARD_X0 + write*CELL_WIDTH,
                    BOARD_Y0 + r*CELL_HEIGHT,
                    ANIMATION_MS);
            }
            write--;
        }
    }
    return moved;
}

uint8_t move_col_down(uint8_t c) __banked {
    uint8_t tmp[4], write = BOARD_ROWS-1, moved = 0, last_merge = -1;
    // copy & clear
    for (uint8_t r = 0; r < 4; ++r) {
        tmp[r] = CELL_VALUE(r, c);
        CELL_VALUE(r, c) = 0;
    }
    // merge/move down
    for (int8_t i = 3; i >= 0; --i) {
        uint8_t val = tmp[i];
        if (!val) continue;
        uint8_t src = i * BOARD_COLUMNS + c;
        if (write < BOARD_ROWS-1
            && CELL_VALUE(write+1, c) == val
            && last_merge != write+1) {
            // merge
            CELL_VALUE(write+1, c) *= 2;
            last_merge = write+1;
            set_sprite_destination(SPRITE(src),
                BOARD_X0 + c*CELL_WIDTH,
                BOARD_Y0 + (write+1)*CELL_HEIGHT,
                ANIMATION_MS);
            moved = 1;
        } else {
            // move
            CELL_VALUE(write, c) = val;
            if (i != write) {
                moved = 1;
                set_sprite_destination(SPRITE(src),
                    BOARD_X0 + c*CELL_WIDTH,
                    BOARD_Y0 + write*CELL_HEIGHT,
                    ANIMATION_MS);
            }
            write--;
        }
    }
    return moved;
}

uint8_t move_row_left(uint8_t r) __banked {
    uint8_t tmp_vals[4];
    uint8_t write = 0;
    uint8_t moved = 0;
    int8_t last_merge = -1;

    for (uint8_t c = 0; c < 4; c++) {
        tmp_vals[c] = CELL_VALUE(r, c);
    }

    for (uint8_t c = 0; c < 4; c++) {
        CELL_VALUE(r, c) = 0;
    }

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t val = tmp_vals[i];
        if (!val) continue;

        uint8_t src_sprite = r * BOARD_COLUMNS + i;

        if (write > 0 && CELL_VALUE(r, write - 1) == val && last_merge != write - 1) {
            CELL_VALUE(r, write - 1) *= 2;
            last_merge = write - 1;
            set_sprite_destination(SPRITE(src_sprite),
                BOARD_X0 + (write - 1) * CELL_WIDTH,
                BOARD_Y0 + r * CELL_HEIGHT,
                ANIMATION_MS);
            moved = 1;
        } else {
            CELL_VALUE(r, write) = val;
            if (i != write) {
                moved = 1;
                set_sprite_destination(SPRITE(src_sprite),
                    BOARD_X0 + write * CELL_WIDTH,
                    BOARD_Y0 + r * CELL_HEIGHT,
                    ANIMATION_MS);
            }
            write++;
        }
    }

    return moved;
}

void set_sprite_destination(SPRITE_t* s, uint8_t dst_x, uint8_t dst_y, uint16_t animation_ms) __banked {
    s->start_x = s->x;
    s->start_y = s->y;
    s->dest_x = dst_x;
    s->dest_y = dst_y;

    int16_t dx = dst_x - s->x;
    int16_t dy = dst_y - s->y;
    uint8_t distance = (dx >= 0 ? dx : -dx) + (dy >= 0 ? dy : -dy);

    uint16_t total_frames = (animation_ms * TARGET_FPS) / 1000;
    if (distance == 0 || total_frames == 0) total_frames = 1;

    s->frames = (uint8_t)total_frames;
    s->frame = 0;
}

uint8_t update_animations() __banked {
    uint8_t still_animating = 0;

    for (uint8_t i = 0; i < SPRITES_COUNT; ++i) {
        SPRITE_t* s = SPRITE(i);

        if (s->frames == 0) continue;

        if (s->frame < s->frames) {
            still_animating = 1;

            int dx = s->dest_x - s->start_x;
            int dy = s->dest_y - s->start_y;

            if (dx >= 0)
                s->x = s->start_x + (dx * s->frame) / s->frames;
            else
                s->x = s->start_x - ((-dx) * s->frame) / s->frames;

            if (dy >= 0)
                s->y = s->start_y + (dy * s->frame) / s->frames;
            else
                s->y = s->start_y - ((-dy) * s->frame) / s->frames;

            s->frame++;
        } else {
            s->x = s->dest_x;
            s->y = s->dest_y;
        }
        
        vdp_update_sprite(i, PATTERN_SLOT(value_to_sprite_index(s->value)),
                        value_to_color(s->value),
                        s->x, s->y);
    }

    return still_animating;
}

uint8_t board_has_moves(void) __banked {
    // Comprovar si hi ha cel·les buides
    for (uint8_t r = 0; r < BOARD_ROWS; ++r) {
        for (uint8_t c = 0; c < BOARD_COLUMNS; ++c) {
            if (CELL_VALUE(r, c) == 0) return 1;
        }
    }

    // Comprovar si hi ha moviments possibles (cel·les adjacents iguals)
    for (uint8_t r = 0; r < BOARD_ROWS; ++r) {
        for (uint8_t c = 0; c < BOARD_COLUMNS; ++c) {
            uint16_t v = CELL_VALUE(r, c);
            // Comprovar cel·la de la dreta
            if (c < BOARD_COLUMNS - 1 && v == CELL_VALUE(r, c + 1)) return 1;
            // Comprovar cel·la de sota
            if (r < BOARD_ROWS - 1 && v == CELL_VALUE(r + 1, c)) return 1;
        }
    }
    return 0;   
}

uint8_t spawn_random_tile(void) __banked {
    uint8_t empties[BOARD_CELLS];
    uint8_t n = 0;
    
    // Trobar totes les cel·les buides basant-se en CELL_VALUE
    for (uint8_t r = 0; r < BOARD_ROWS; ++r) {
        for (uint8_t c = 0; c < BOARD_COLUMNS; ++c) {
            if (CELL_VALUE(r, c) == 0) {
                empties[n++] = r * BOARD_COLUMNS + c;
            }
        }
    }
    
    if (n == 0) return 0; // No hi ha cel·les buides
    
    // Seleccionar una cel·la buida aleatòria
    uint8_t cell_idx = empties[random_8() % n];
    uint8_t row = cell_idx / BOARD_COLUMNS;
    uint8_t col = cell_idx % BOARD_COLUMNS;
    uint8_t value = (random_8() % 10 == 0) ? 4 : 2; // 10% probabilitat de 4, 90% de 2
    
    // Actualitzar tant la matriu de cel·les com l'sprite
    CELL_VALUE(row, col) = value;
    SPRITE(cell_idx)->value = value;
    
    // Actualitzar l'sprite visual
    vdp_update_sprite(cell_idx,
        PATTERN_SLOT(value_to_sprite_index(value)),
        value_to_color(value),
        SPRITE(cell_idx)->x, SPRITE(cell_idx)->y);
    
    return 1;
}