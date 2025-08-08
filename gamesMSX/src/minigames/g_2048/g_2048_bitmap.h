#pragma bank 2
#ifndef G_2048_BITMAP_H
#define G_2048_BITMAP_H

#include <stdint.h>

#define G_2048_BITMAP_MAP_ROWS 24
#define G_2048_BITMAP_MAP_COLUMNS 32
#define G_2048_BITMAP_TILE_BYTES 8
#define G_2048_BITMAP_TILE_COUNT 10

extern const uint8_t g_2048_bitmap_tileset[G_2048_BITMAP_TILE_COUNT][G_2048_BITMAP_TILE_BYTES];
extern const uint8_t g_2048_bitmap_tilemap[G_2048_BITMAP_MAP_ROWS * G_2048_BITMAP_MAP_COLUMNS];

#endif /* G_2048_BITMAP_H */