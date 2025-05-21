#include "tiles.h"

#define TILE_BLOCK_SIZE         (256 * 8)
#define PAT_OFFSET(g,id,row)    ((g) * TILE_BLOCK_SIZE + (id) * 8 + (row))
#define PAT_PTR(g,id)           (&tile_patterns[(g) * TILE_BLOCK_SIZE + (id) * 8])

unsigned char tile_patterns[3 * TILE_BLOCK_SIZE];
unsigned char vdp_buffer[VDP_BUFFER_SIZE];

void init_tileset(void)
{
    memset(tile_patterns, 0, sizeof(tile_patterns));

    for (unsigned char g = 0; g < 3; ++g) {
        for (unsigned int id = 0; id < 256; ++id) {
            vdp_set_tile_pattern(g, id, PAT_PTR(g, id)); 
        }
    }
}

void init_colortable(void)
{
    for (unsigned char g = 0; g < 3; ++g) {
        for (unsigned int id = 0; id < 256; ++id) {
            vdp_set_tile_color(g, id, COLOR_WHITE, COLOR_BLACK);
        }
    }
}

void init_tilemap(void) {
    for (unsigned char y = 0; y < VIEW_H; ++y) {
        for (unsigned char x = 0; x < VIEW_W; ++x) {
            unsigned char group = y / 8;            // 0-2 (per cada 8 files de tiles)
            unsigned char tile_y_in_group = y % 8;  // 0-7 dins del grup
            unsigned char tile_id = tile_y_in_group * VIEW_W + x; // 0-255
            vdp_buffer[y * VIEW_W + x] = tile_id;
        }
    }
    
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_buffer);
}

void draw_pixel(unsigned int x, unsigned int y, unsigned char set)
{
    unsigned char tile_x = x >> 3;
    unsigned char tile_y = y >> 3;
    unsigned char group  = y >> 6;               /* 0-2               */
    unsigned char tile_y_in_group = tile_y & 7;  /* 0-7 dins del grup */
    unsigned char tile_id = tile_y_in_group * VIEW_W + tile_x;   /* 0-255 */
    unsigned char row_in_tile = y & 7;           /* 0-7               */
    unsigned char mask = 0x80 >> (x & 7);

    unsigned int idx = PAT_OFFSET(group, tile_id, row_in_tile);

    if (set) tile_patterns[idx] |=  mask;
    else     tile_patterns[idx] &= ~mask;

    vdp_set_tile_pattern(group, tile_id, PAT_PTR(group, tile_id));
}

/**
 * Draws a line from (x0, y0) to (x1, y1).
 *
 * @param x0 Starting x coordinate in pixels.
 * @param y0 Starting y coordinate in pixels.
 * @param x1 Ending x coordinate in pixels.
 * @param y1 Ending y coordinate in pixels.
 * @param set 1 to draw (set) pixels, 0 to erase (clear) pixels.
 */
void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char set) {
    int dx = (int)x1 - (int)x0;
    int dy = (int)y1 - (int)y0;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    dx = sx * dx;
    dy = sy * dy;

    int err = (dx > dy ? dx : -dy) >> 1;
    int e2;

    int x = x0;
    int y = y0;

    while (1) {
        draw_pixel((unsigned char)x, (unsigned char)y, set);
        if (x == x1 && y == y1) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x += sx;
        }
        if (e2 < dy) {
            err += dx;
            y += sy;
        }
    }
}
