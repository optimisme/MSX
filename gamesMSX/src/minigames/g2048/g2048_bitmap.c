#include "g2048_bitmap.h"

const uint8_t g2048_bitmap_tileset[G2048_BITMAP_TILE_COUNT][G2048_BITMAP_TILE_BYTES] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x3f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f },
    { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0x00, 0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe },
    { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f },
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe },
    { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3f, 0x00 },
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 },
    { 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0x00 },
};

const uint8_t g2048_bitmap_tilemap[G2048_BITMAP_MAP_ROWS * G2048_BITMAP_MAP_COLUMNS] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  1,  2,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  4,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  7,  8,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};