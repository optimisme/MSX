#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include <stdint.h>

// Clears Mode 2 tilemap + color table to black and hides all sprites.
// Intended to be called before switching between menu/loading/game screens
// to avoid flicker/garbage tiles.
void game_transition_black(void);

#endif

