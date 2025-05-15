#ifndef TILES_H
#define TILES_H

#include "../utils/utils_msx.h"

/**
 * Public tile data (8 × 8 one‑bit patterns)
 */
extern const unsigned char bck_tile_0[8];
extern const unsigned char bck_tile_1[8];

/**
 * Initialisation routines
 *   • init_tileset()     – upload patterns to VRAM
 *   • init_colortable()  – upload default colours
 *   • init_tilemap()     – build the screen background
 */
void init_tileset(void);
void init_colortable(void);
void init_tilemap(void);

#endif /* TILES_H */
