#include "sprites.h"

/* Player – top (round head with eyes and smile) */
const unsigned char sprite_player_top[8] = {
    0b00111100,
    0b01111110,
    0b11100111, /* eyes */
    0b11100111,
    0b11111111,
    0b11101111, /* smile row */
    0b01111110,
    0b00111100
};

/* Player – legs (two frames, rounded feet) */
const unsigned char sprite_player_legs_a[8] = {
    0b00000000,
    0b00000000,
    0b00100100,
    0b00100100,
    0b00100100,
    0b01011010, /* legs apart */
    0b01011010, /* feet angled */
    0b00100100
};

const unsigned char sprite_player_legs_b[8] = {
    0b00000000,
    0b00000000,
    0b00100100,
    0b00100100,
    0b00111100, /* one leg forward */
    0b00100100,
    0b01011010,
    0b00100100
};

/* Enemy – round red blob */
const unsigned char sprite_enemy[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100
};

/* Pushable block */
const unsigned char sprite_block[8] = {
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111
};
