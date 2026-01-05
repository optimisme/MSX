#pragma bank 1
#ifndef MENU_BITMAP_H
#define MENU_BITMAP_H

#include <stdint.h>

#define MENU_BITMAP_MAP_ROWS 24
#define MENU_BITMAP_MAP_COLUMNS 32
#define MENU_BITMAP_TILE_BYTES 8
#define MENU_BITMAP_TILE_COUNT 103

extern const uint8_t menu_bitmap_tileset[MENU_BITMAP_TILE_COUNT][MENU_BITMAP_TILE_BYTES];
extern const uint8_t menu_bitmap_tilemap[MENU_BITMAP_MAP_ROWS * MENU_BITMAP_MAP_COLUMNS];

#endif /* MENU_BITMAP_H */