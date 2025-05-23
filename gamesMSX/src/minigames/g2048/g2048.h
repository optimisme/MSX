#ifndef G2048_H
#define G2048_H

#include <conio.h>

#include "g2048_bitmap.h"
#include "g2048_sprites.h"
#include "../buffers.h"
#include "../../utils/fps.h"
#include "../../utils/random.h"
#include "../../utils/utils_msx.h"

void init_game(void) __banked;
void update_game() __banked;

void vdp_define_sprite16(uint8_t pattern_index, const uint8_t data[32]) __banked;
void load_all_sprite_graphics(void) __banked;

uint8_t random_cell_index() __banked;
uint8_t value_to_sprite_index(uint16_t value) __banked ;
void restart_game(void) __banked;

#endif 