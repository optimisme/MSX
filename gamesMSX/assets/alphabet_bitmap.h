#pragma bank 2
#ifndef ALPHABET_BITMAP_H
#define ALPHABET_BITMAP_H

#include <stdint.h>

#define ALPHABET_BITMAP_MAP_ROWS 3
#define ALPHABET_BITMAP_MAP_COLUMNS 32
#define ALPHABET_BITMAP_TILE_BYTES 8
#define ALPHABET_BITMAP_TILE_COUNT 96

extern const uint8_t alphabet_bitmap_tileset[ALPHABET_BITMAP_TILE_COUNT][ALPHABET_BITMAP_TILE_BYTES];
extern const uint8_t alphabet_bitmap_tilemap[ALPHABET_BITMAP_MAP_ROWS * ALPHABET_BITMAP_MAP_COLUMNS];

#endif /* ALPHABET_BITMAP_H */