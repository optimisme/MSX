#pragma bank 2
#ifndef G2048_SPRITES_H
#define G2048_SPRITES_H

#include <stdint.h>

#define G2048_SPRITES_SPRITE_ROWS    1
#define G2048_SPRITES_SPRITE_COLUMNS 12
#define G2048_SPRITES_SPRITE_BYTES   32
#define G2048_SPRITES_SPRITE_COUNT   12

extern const uint8_t g2048_sprites_bitmap_spriteset[G2048_SPRITES_SPRITE_COUNT][G2048_SPRITES_SPRITE_BYTES];
extern const uint8_t g2048_sprites_bitmap_sprites[G2048_SPRITES_SPRITE_ROWS * G2048_SPRITES_SPRITE_COLUMNS];

#endif /* G2048_SPRITES_H */