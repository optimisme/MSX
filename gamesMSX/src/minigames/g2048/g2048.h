#ifndef G2048_H
#define G2048_H

#include <conio.h>

#include "g2048_bitmap.h"
#include "../buffers.h"
#include "../../utils/fps.h"
#include "../../utils/utils_msx.h"

void init_game(void) __banked;
void update_game() __banked;

#endif 