#ifndef MENU_H
#define MENU_H

#include <conio.h>

#include "menu_bitmap.h"
#include "../banks.h"
#include "../buffers.h"
#include "../../utils/fps.h"
#include "../../utils/utils_msx.h"

// void main_menu(uint8_t selected_menu_option, uint8_t *out) __banked;
void init_menu(void) __banked;
void set_cursor_destination(uint8_t option, uint8_t animation_ms) __banked;
void update_cursor_animation(void) __banked;
void draw_menu() __banked;

void init_menu_vars(void) __banked;
void free_menu_vars(void) __banked;

#endif 