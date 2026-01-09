#ifndef TILES_H
#define TILES_H

#include <stdint.h>
#include <string.h>
#include "../utils/utils_msx.h"

#define TILEMAP_W 32
#define TILEMAP_H 24
#define TILEMAP_SIZE (TILEMAP_W * TILEMAP_H)

#define ANIM_W 15
#define ANIM_H 10
#define ANIM_TILE_COUNT (ANIM_W * ANIM_H)

#define TILE_EMPTY_IDX    240
#define TILE_TOTAL        (TILE_EMPTY_IDX + 1)

void init_anim_demo(void);
void render_anim_frame(void);

#endif /* TILES_H */
