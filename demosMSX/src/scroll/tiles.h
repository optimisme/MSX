#ifndef TILES_H
#define TILES_H

#include <string.h>      /* memcpy() */
#include "../utils/utils_msx.h"   /* init_tile(), init_color_for_tile(), msx_vpoke() */


#define TILES       3
#define TILEMAP_W   64   
#define TILEMAP_H   48 
#define VIEW_W      32
#define VIEW_H      24
#define TILE_LIMIT_W (TILEMAP_W - VIEW_W)
#define TILE_LIMIT_H (TILEMAP_H - VIEW_H)
#define VDP_BUFFER_SIZE (VIEW_W * VIEW_H)

/**
 * Initialisation routines
 *   • init_tileset()     – upload patterns to VRAM
 *   • init_colortable()  – upload default colours
 *   • init_tilemap()     – build the screen background
 */
void init_tileset(void);
void init_colortable(void);
void init_tiles(void);

/**
 * Set camera position in pixels
 * - 0..128: camera at 0 (show 32 first tiles)
 * - 129..(TILEMAP_W*8 - 128): move camera every 8 pixels
 * - > (TILEMAP_W*8 - 128): camera at the end (show 32 last tiles)
 */
void set_camera(unsigned int cam_x, unsigned int cam_y);

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
