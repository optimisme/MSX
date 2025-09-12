#ifndef BANKS_H
#define BANKS_H

#include <conio.h>
#include "../utils/utils_msx.h"

void main_menu(uint8_t selected_menu_option, uint8_t *out) __banked;
void main_g_2048() __banked;
void main_g_flappy() __banked;
void main_g_snake() __banked;
void bank5_content() __banked;

#endif
