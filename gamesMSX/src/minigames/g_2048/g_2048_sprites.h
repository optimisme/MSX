#pragma bank 2
#ifndef G_2048_SPRITES_H
#define G_2048_SPRITES_H

#include <stdint.h>

#define G_2048_SPRITES_SPRITE_ROWS    1
#define G_2048_SPRITES_SPRITE_COLUMNS 12
#define G_2048_SPRITES_SPRITE_BYTES   32
#define G_2048_SPRITES_SPRITE_COUNT   12

extern const uint8_t g_2048_sprites_bitmap_spriteset[G_2048_SPRITES_SPRITE_COUNT][G_2048_SPRITES_SPRITE_BYTES];
extern const uint8_t g_2048_sprites_bitmap_sprites[G_2048_SPRITES_SPRITE_ROWS * G_2048_SPRITES_SPRITE_COLUMNS];

#endif /* G_2048_SPRITES_H */