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

void draw_pixel(unsigned int x, unsigned int y, unsigned char set);

void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char set);

/**
 * Send bitmap to VRAM
 */
void flush_bitmap();

/**
 * Copy 24 rows of 32 tiles from a tilemap into a buffer.
 *
 * @param dest_buffer Pointer to 768-byte buffer (e.g., vdp_buffer)
 * @param tilemap     Pointer to full tilemap (e.g., TILEMAP_W * TILEMAP_H)
 * @param start_tile_x X offset in tiles
 * @param start_tile_y Y offset in tiles
 * @param tilemap_w
 * @param view_w
 * @param view_h
 */
void copy_tilemap_rows_asm(unsigned char* dest_buffer, const unsigned char* tilemap, unsigned int start_tile_x, unsigned int start_tile_y, unsigned char tilemap_w, unsigned char view_w, unsigned char view_h) __z88dk_callee;

#endif /* TILES_H */
