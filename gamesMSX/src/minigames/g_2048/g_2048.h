#ifndef G2048_H
#define G2048_H
#include <string.h>   
#include <conio.h>
#include "g_2048_bitmap.h"
#include "g_2048_sprites.h"
#include "../alphabet/alphabet.h"
#include "../buffers.h"
#include "../../utils/utils_fps.h"
#include "../../utils/utils_random.h"
#include "../../utils/utils_msx.h"

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t start_x;
    uint8_t start_y;
    uint8_t dest_x;
    uint8_t dest_y;
    uint8_t frames;
    uint8_t frame;
    uint16_t value;  
} SPRITE_t;

void init_game(void) __banked;
void update_game() __banked;
uint8_t handle_input() __banked;
void write_text_to_tilemap_buff(const char *text, uint16_t pos) __banked;
void write_text_to_vram(const char *text, uint16_t pos) __banked;

void load_sprite_patterns(void) __banked;
uint8_t get_random_sprite_index() __banked;
uint8_t value_to_sprite_index(uint16_t value) __banked;
uint8_t value_to_color(uint16_t value) __banked;
void restart_game(void) __banked;
void fill_board() __banked;
void move_tiles(uint8_t direction) __banked;
uint8_t move_col_up(uint8_t c) __banked;
uint8_t move_row_right(uint8_t r) __banked;
uint8_t move_col_down(uint8_t c) __banked;
uint8_t move_row_left(uint8_t r) __banked;
void set_sprite_destination(SPRITE_t* s, uint8_t dst_x, uint8_t dst_y, uint16_t animation_ms) __banked;
uint8_t update_animations() __banked;
uint8_t board_has_moves(void) __banked;
uint8_t spawn_random_tile(void) __banked;

uint8_t check_game_won(void) __banked;
void show_game_lost(void) __banked;
void show_game_won(void) __banked;
void clean_vdp(void) __banked;

#endif