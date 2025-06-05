#ifndef TILES_H
#define TILES_H

#include <string.h>
#include "../utils/utils_msx.h"

/* Bases de VRAM en SCREEN 2 (sense moure registres)          */


#define TILEMAP_W   32  
#define TILEMAP_H   24 
#define VIEW_W      32
#define VIEW_H      24
#define TILE_LIMIT_W (TILEMAP_W - VIEW_W)
#define TILE_LIMIT_H (TILEMAP_H - VIEW_H)
#define VDP_BUFFER_SIZE (VIEW_W * VIEW_H)

#define TILE_0      0u
#define TILE_1      1u
#define TILE_2      2u

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

void draw_blockB(unsigned char bx,unsigned char by,unsigned char set);

void draw_lineB(unsigned char bx0,unsigned char by0,
                unsigned char bx1,unsigned char by1,
                unsigned char set);

void write_buffer_to_vram(void);

#endif /* TILES_H */
