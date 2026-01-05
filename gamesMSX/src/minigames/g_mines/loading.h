#ifndef LOADING_H
#define LOADING_H

#include <msx.h>
#include <stdint.h>
#include "../../utils/utils_msx.h"

#define NUM_LOADING_TILES   7

#define  LOADING_BACK       255
#define  LOADING_BAR_EMPTY  254
#define  LOADING_BAR_FULL   253
#define  LOADING_BAR_PART   247
#define  LOADING_PART_0     252
#define  LOADING_PART_1     251
#define  LOADING_PART_2     250
#define  LOADING_PART_3     249
#define  LOADING_PART_4     248

void loading_init(uint8_t tile_x, uint8_t tile_y);
void loading_draw_progress(uint8_t tile_start, uint8_t tile_end, uint8_t tile_y, uint8_t percentage);

#endif
