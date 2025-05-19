#ifndef LOADING_H
#define LOADING_H 

#include <msx.h>
#include <stdio.h>       
#include <video/tms99x8.h>

#include "constants.h"
#include "../utils/utils_msx.h"

#define NUM_LOADING_TILES   6

#define  LOADING_BACK       255
#define  LOADING_BAR_EMPTY  254
#define  LOADING_BAR_FULL   253
#define  LOADING_PART_0     252
#define  LOADING_PART_1     251
#define  LOADING_PART_2     250
#define  LOADING_PART_3     249
#define  LOADING_PART_4     248

/**
 * Initializes loading screen
 * @param tile_x   x tile position for "Loading" text
 * @param tile_y   y tile position for "Loading" text
 */
void loading_init(uint8_t tile_x, uint8_t tile_y);

/**
 * Draw a horizontal progress bar using tile values.
 *
 * Tiles from tile_start..tile_end-1 on row tile_y are set:
 *   – value 2 for the first N tiles, where N = percentage% of the span
 *   – value 1 for the rest
 *
 * @param tile_start   First tile X index (inclusive)
 * @param tile_end     Last tile X index (exclusive)
 * @param tile_y       Tile row Y index
 * @param percentage   Fill percentage (0–100)
 */
void loading_draw_progress(uint8_t tile_start, uint8_t tile_end, uint8_t tile_y, uint8_t percentage);

#endif