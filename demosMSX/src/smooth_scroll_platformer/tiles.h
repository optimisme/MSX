#ifndef TILES_H
#define TILES_H

#include <stdint.h>

#include "constants.h"
#include "../utils/utils_msx.h"

#define NUM_TILE_TYPES  7       // Max 7
#define TILEMAP_W       150
#define TILEMAP_H       24      // 24
#define VIEW_W          32      // 32
#define VIEW_H          24      // Max 24
#define TILE_LIMIT_W (TILEMAP_W - VIEW_W)
#define TILE_LIMIT_H (TILEMAP_H - VIEW_H)
#define VDP_BUFFER_SIZE (VIEW_W * VIEW_H)

/**
 * Initialisation routines
 */
void init_tiles_0();
void init_tiles_1();
void init_tiles_2();
void init_tiles_3(uint8_t *mul_tiles_lut);
void init_tiles_4(uint8_t *mul_tiles_lut);
void init_tiles_5(uint8_t *mul_tiles_lut);
void init_tiles_6(uint8_t *mul_tiles_lut);
void init_tiles_7();

/**
 * Set camera horizontal position in pixels (will clamp).
 */
void scroll_to(unsigned int cam_x);

/**
 * Read tile id from world pixel coordinate (for collisions).
 */
uint8_t get_tile_at_pixel(int16_t px, int16_t py);

#endif /* TILES_H */
