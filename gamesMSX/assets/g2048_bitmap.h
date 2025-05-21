#pragma bank 2
#ifndef G2048_BITMAP_H
#define G2048_BITMAP_H

#include <stdint.h>

#define G2048_BITMAP_MAP_ROWS 24
#define G2048_BITMAP_MAP_COLUMNS 32
#define G2048_BITMAP_TILE_BYTES 8
#define G2048_BITMAP_TILE_COUNT 10

extern const uint8_t g2048_bitmap_tileset[G2048_BITMAP_TILE_COUNT][G2048_BITMAP_TILE_BYTES];
extern const uint8_t g2048_bitmap_tilemap[G2048_BITMAP_MAP_ROWS * G2048_BITMAP_MAP_COLUMNS];

#endif /* G2048_BITMAP_H */