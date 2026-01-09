#ifndef TILES_H
#define TILES_H

#include <stdint.h>
#include <string.h>
#include "../utils/utils_msx.h"

#define TILEMAP_W 32
#define TILEMAP_H 24
#define TILEMAP_SIZE (TILEMAP_W * TILEMAP_H)

#define TILE_EMPTY_IDX 0
#define TILE_SEG_BASE     1
#define TILE_SEG_COUNT    64
#define TEXTURE_COUNT     3
#define TILE_TOTAL        (TILE_SEG_BASE + TILE_SEG_COUNT * TEXTURE_COUNT)

#define MAP_W 16
#define MAP_H 16

void init_raycast(void);
void render_frame(uint16_t pos_x_fp, uint16_t pos_y_fp, uint8_t angle);
void get_dir(uint8_t angle, int16_t* dx, int16_t* dy);

#endif /* TILES_H */
