#include "tiles.h"
#include "vdp.h"
#include "constants.h"

#include <stdint.h>

#define TILE_EMPTY 0
#define TILE_FULL  1
#define TILE_X     2  // rendered as FULL (no X)

#define TILE_BASE_RECT 3
#define NUM_INTERVALS 10
#define NUM_RECT_TILES (NUM_INTERVALS * NUM_INTERVALS)

static const uint8_t intervals[NUM_INTERVALS][2] = {
    {0,2},{0,4},{0,6},{0,8},
    {2,4},{2,6},{2,8},
    {4,6},{4,8},
    {6,8}
};

static void write_tile(uint16_t pat_base, uint16_t col_base, uint8_t idx, const uint8_t *pat) {
    uint16_t p = pat_base + (uint16_t)idx * 8;
    uint16_t c = col_base + (uint16_t)idx * 8;

    uint8_t col[8];
    uint8_t i;
    for (i = 0; i < 8; ++i) col[i] = 0xF1; // FG white, BG black

    vdp_write(p, pat, 8);
    vdp_write(c, col, 8);
}

static void gen_pat_rect(uint8_t *out, uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1) {
    uint8_t y;
    for (y = 0; y < 8; ++y) {
        if (y < y0 || y >= y1) {
            out[y] = 0x00;
        } else {
            uint8_t m = 0;
            uint8_t x;
            for (x = x0; x < x1; ++x) m |= (uint8_t)(1u << (7 - x));
            out[y] = m;
        }
    }
}

static void write_block(uint16_t pat_base, uint16_t col_base) {
    static const uint8_t pat_empty[8] = {0,0,0,0,0,0,0,0};
    static const uint8_t pat_full[8]  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    write_tile(pat_base, col_base, TILE_EMPTY, pat_empty);
    write_tile(pat_base, col_base, TILE_FULL,  pat_full);
    write_tile(pat_base, col_base, TILE_X,     pat_full);

    // 2px-quantized filled rectangles inside a tile.
    {
        uint8_t ix, iy;
        uint8_t pat[8];
        uint8_t idx = TILE_BASE_RECT;

        for (iy = 0; iy < NUM_INTERVALS; ++iy) {
            uint8_t y0 = intervals[iy][0];
            uint8_t y1 = intervals[iy][1];
            for (ix = 0; ix < NUM_INTERVALS; ++ix) {
                uint8_t x0 = intervals[ix][0];
                uint8_t x1 = intervals[ix][1];
                gen_pat_rect(pat, x0, x1, y0, y1);
                write_tile(pat_base, col_base, idx++, pat);
            }
        }
    }
}

void tiles_init(void) {
    write_block(0x0000, 0x2000);
    write_block(0x0800, 0x2800);
    write_block(0x1000, 0x3000);
}
