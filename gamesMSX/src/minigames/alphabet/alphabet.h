#ifndef ALPHABET_H
#define ALPHABET_H

#include <stdint.h>
#include "alphabet_bitmap.h"
#include "../buffers.h"
#include "../../utils/utils_msx.h"

#define ALPHABET_BASE 160

#define INVALID_CHAR        0xFF
#define FIRST_CHAR          0x20
#define LAST_CHAR           (FIRST_CHAR + ALPHABET_BITMAP_TILE_COUNT - 1)
#define CHAR_TO_INDEX(c)    (((c) >= FIRST_CHAR && (c) <= LAST_CHAR) \
                                ? ((uint8_t)(c) - FIRST_CHAR)       \
                                : INVALID_CHAR) 

void load_alphabet_tileset(void);
void load_alphabet_colors(void);

void write_text_to_vram(const char *text, uint16_t pos);
void write_text_to_tilemap_buff(const char *text, uint16_t pos);

#endif /* ALPHABET_H */