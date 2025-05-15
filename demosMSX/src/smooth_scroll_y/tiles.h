#ifndef TILES_H
#define TILES_H

#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "../utils/utils_msx.h"

#define TILEMAP_W   32   
#define TILEMAP_H   125
#define VIEW_W      32
#define VIEW_H      24
#define TILE_LIMIT_W (TILEMAP_W - VIEW_W)
#define TILE_LIMIT_H (TILEMAP_H - VIEW_H)
#define VDP_BUFFER_SIZE (VIEW_W * VIEW_H)

/**
 * Initialisation routines
 *   • initialize_tilesystem() – build the screen background
 */
void init_tiles(void);

/**
 * Set camera vertical position in pixels
 * - 0..128: camera at 0 (show 32 first tiles)
 * - 129..(TILEMAP_H*8 - 128): move camera every 8 pixels
 * - > (TILEMAP_H*8 - 128): camera at the end (show 32 last tiles)
 */
void scroll_to(unsigned int cam_y);

#endif /* TILES_H */