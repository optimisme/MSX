#include "constants.h"
#include "vdp.h"
#include "tiles.h"

#include <stdint.h>
#include <string.h>

extern unsigned char msx_get_stick(unsigned char port);
extern unsigned char msx_get_trigger(unsigned char port);
extern unsigned char wait_fps(void);

#ifndef STICK_UP
#define STICK_UP 1
#endif
#ifndef STICK_UP_RIGHT
#define STICK_UP_RIGHT 2
#endif
#ifndef STICK_RIGHT
#define STICK_RIGHT 3
#endif
#ifndef STICK_DOWN_RIGHT
#define STICK_DOWN_RIGHT 4
#endif
#ifndef STICK_DOWN
#define STICK_DOWN 5
#endif
#ifndef STICK_DOWN_LEFT
#define STICK_DOWN_LEFT 6
#endif
#ifndef STICK_LEFT
#define STICK_LEFT 7
#endif
#ifndef STICK_UP_LEFT
#define STICK_UP_LEFT 8
#endif

#ifdef TILE_EMPTY
#undef TILE_EMPTY
#endif
#ifdef TILE_FULL
#undef TILE_FULL
#endif
#ifdef TILE_X
#undef TILE_X
#endif

#define TILE_EMPTY 0
#define TILE_FULL  1
#define TILE_X     2  // rendered as FULL (no X)

#define TILE_BASE_RECT 3
#define NUM_INTERVALS 10

static const uint8_t intervals[NUM_INTERVALS][2] = {
    {0,2},{0,4},{0,6},{0,8},
    {2,4},{2,6},{2,8},
    {4,6},{4,8},
    {6,8}
};

// Camera steps (16.16)
#define CAM_X_STEP_FP (FP_ONE/4)   // 0.25
#define CAM_X_MIN_FP  (-(FP_FROM_INT(20)))
#define CAM_X_MAX_FP  (FP_FROM_INT(20))

static uint8_t vdp_buffer[NAMETABLE_SIZE];

static uint8_t vdp_prev[NAMETABLE_SIZE];

static uint8_t vdp_prev[NAMETABLE_SIZE];

static void flush_nametable(void) {
    uint8_t ty;
    for (ty = 0; ty < TILES_Y; ++ty) {
        uint8_t *cur = &vdp_buffer[(uint16_t)ty * TILES_X];
        uint8_t *pre = &vdp_prev[(uint16_t)ty * TILES_X];
        uint8_t tx = 0;

        while (tx < TILES_X) {
            while (tx < TILES_X && cur[tx] == pre[tx]) ++tx;
            if (tx >= TILES_X) break;

            uint8_t start = tx;
            while (tx < TILES_X && cur[tx] != pre[tx]) {
                pre[tx] = cur[tx];
                ++tx;
            }

            vdp_write((uint16_t)(VRAM_NAME_TABLE + (uint16_t)ty * TILES_X + start),
                      &cur[start],
                      (uint16_t)(tx - start));
        }
    }
}

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}


static int qfloor2(int v) { return v & ~1; }
static int qceil2(int v)  { return (v + 1) & ~1; }

static uint8_t interval_index(uint8_t a, uint8_t b) {
    uint8_t k = (uint8_t)((a << 4) | b);
    switch (k) {
        case 0x02: return 0; // 0..2
        case 0x04: return 1; // 0..4
        case 0x06: return 2; // 0..6
        case 0x08: return 3; // 0..8
        case 0x24: return 4; // 2..4
        case 0x26: return 5; // 2..6
        case 0x28: return 6; // 2..8
        case 0x46: return 7; // 4..6
        case 0x48: return 8; // 4..8
        default:   return 9; // 6..8 (0x68)
    }
}

static uint8_t pick_tile_quant2(int tile_x0, int tile_y0, int x0, int y0, int x1, int y1) {
    int tile_x1 = tile_x0 + 8;
    int tile_y1 = tile_y0 + 8;

    int ox0 = (x0 > tile_x0) ? x0 : tile_x0;
    int oy0 = (y0 > tile_y0) ? y0 : tile_y0;
    int ox1 = (x1 < tile_x1) ? x1 : tile_x1;
    int oy1 = (y1 < tile_y1) ? y1 : tile_y1;

    int w = ox1 - ox0;
    int h = oy1 - oy0;
    if (w <= 0 || h <= 0) return TILE_EMPTY;
    if (w == 8 && h == 8) return TILE_FULL;

    // Local coords inside tile (0..8), guaranteed even by qfloor2/qceil2.
    uint8_t lx0 = (uint8_t)(ox0 - tile_x0);
    uint8_t lx1 = (uint8_t)(ox1 - tile_x0);
    uint8_t ly0 = (uint8_t)(oy0 - tile_y0);
    uint8_t ly1 = (uint8_t)(oy1 - tile_y0);

    uint8_t ix = interval_index(lx0, lx1);
    uint8_t iy = interval_index(ly0, ly1);

    return (uint8_t)(TILE_BASE_RECT + (uint8_t)(iy * NUM_INTERVALS + ix));
}

static void draw_rect_quant2(int x0, int y0, int x1, int y1) {
    // Quantize to 2px grid: floor for start, ceil for end.
    x0 = qfloor2(x0);
    y0 = qfloor2(y0);
    x1 = qceil2(x1);
    y1 = qceil2(y1);

    x0 = clampi(x0, 0, SCREEN_W_PX);
    x1 = clampi(x1, 0, SCREEN_W_PX);
    y0 = clampi(y0, 0, SCREEN_H_PX);
    y1 = clampi(y1, 0, SCREEN_H_PX);

    if (x1 <= x0 || y1 <= y0) return;

    {
        int tx0 = x0 >> 3;
        int tx1 = (x1 - 1) >> 3;
        int ty0 = y0 >> 3;
        int ty1 = (y1 - 1) >> 3;

        int tx, ty;

        tx0 = clampi(tx0, 0, TILES_X - 1);
        tx1 = clampi(tx1, 0, TILES_X - 1);
        ty0 = clampi(ty0, 0, TILES_Y - 1);
        ty1 = clampi(ty1, 0, TILES_Y - 1);

        // Fill interior tiles (fully covered)
        {
            int fx0 = (x0 + 7) >> 3;
            int fx1 = (x1 >> 3) - 1;
            int fy0 = (y0 + 7) >> 3;
            int fy1 = (y1 >> 3) - 1;

            fx0 = clampi(fx0, 0, TILES_X - 1);
            fx1 = clampi(fx1, 0, TILES_X - 1);
            fy0 = clampi(fy0, 0, TILES_Y - 1);
            fy1 = clampi(fy1, 0, TILES_Y - 1);

            if (fx0 <= fx1 && fy0 <= fy1) {
                for (ty = fy0; ty <= fy1; ++ty) {
                    uint8_t *row = &vdp_buffer[ty * TILES_X];
                    for (tx = fx0; tx <= fx1; ++tx) row[tx] = TILE_FULL;
                }
            }
        }

        // Perimeter tiles: top & bottom rows
        for (tx = tx0; tx <= tx1; ++tx) {
            int tile_x0 = tx << 3;
            int tile_y0 = ty0 << 3;
            vdp_buffer[ty0 * TILES_X + tx] = pick_tile_quant2(tile_x0, tile_y0, x0, y0, x1, y1);
        }
        if (ty1 != ty0) {
            for (tx = tx0; tx <= tx1; ++tx) {
                int tile_x0 = tx << 3;
                int tile_y0 = ty1 << 3;
                vdp_buffer[ty1 * TILES_X + tx] = pick_tile_quant2(tile_x0, tile_y0, x0, y0, x1, y1);
            }
        }

        // Left & right columns (excluding corners already set)
        if (tx1 != tx0) {
            for (ty = ty0 + 1; ty <= ty1 - 1; ++ty) {
                int tile_x0 = tx0 << 3;
                int tile_y0 = ty << 3;
                vdp_buffer[ty * TILES_X + tx0] = pick_tile_quant2(tile_x0, tile_y0, x0, y0, x1, y1);

                tile_x0 = tx1 << 3;
                vdp_buffer[ty * TILES_X + tx1] = pick_tile_quant2(tile_x0, tile_y0, x0, y0, x1, y1);
            }
        } else {
            // Single column: set middle tiles too
            for (ty = ty0 + 1; ty <= ty1 - 1; ++ty) {
                int tile_x0 = tx0 << 3;
                int tile_y0 = ty << 3;
                vdp_buffer[ty * TILES_X + tx0] = pick_tile_quant2(tile_x0, tile_y0, x0, y0, x1, y1);
            }
        }
    }
}
// "BST" (binary subdivision tree) rasterization: recursively split the segment until it falls in a single tile.
// This avoids scanning the full bounding box area just to find border tiles.
// Corner pick based on how the square overlaps the tile (no dependency on which segment called it).
// We decide which *tile corner* is filled by checking which sides are clipped.
static void render_square(long cam_x_fp, long cam_z_fp) {
    const long quad_z_fp    = FP_FROM_INT(-5);
    const long quad_size_fp = FP_ONE; // 1.0

    long z_fp = cam_z_fp - quad_z_fp;
    if (z_fp < FP_FROM_INT(1)) z_fp = FP_FROM_INT(1);

    // focal so that at cam_z=5 => z=10 size=8px
    const long focal_fp = FP_FROM_INT(80);

    // projected size
    long proj_size_fp = FP_DIV(FP_MUL(focal_fp, quad_size_fp), z_fp);
    if (proj_size_fp < 1) proj_size_fp = 1;

    // center shift by camera X (object x=0, camera x=cam_x)
    // screen_shift = focal * (-cam_x) / z
    long shift_fp = FP_DIV(FP_MUL(focal_fp, -cam_x_fp), z_fp);
    int cx = (SCREEN_W_PX / 2) + (int)(shift_fp >> FP_SHIFT);
    int cy = (SCREEN_H_PX / 2);

    // min=floor, max=ceil, x1/y1 exclusive
    {
        long half_fp = proj_size_fp / 2;
        int half_floor = (int)(half_fp >> FP_SHIFT);
        int half_ceil  = (int)((half_fp + (FP_ONE - 1)) >> FP_SHIFT);

        int x0 = cx - half_floor;
        int x1 = cx + half_ceil;
        int y0 = cy - half_floor;
        int y1 = cy + half_ceil;

        x0 = clampi(x0, 0, SCREEN_W_PX);
        x1 = clampi(x1, 0, SCREEN_W_PX);
        y0 = clampi(y0, 0, SCREEN_H_PX);
        y1 = clampi(y1, 0, SCREEN_H_PX);

        memset(vdp_buffer, TILE_EMPTY, NAMETABLE_SIZE);
        if (x1 <= x0 || y1 <= y0) return;

        // Single rectangle for now (projected square), but the drawing path supports multiple rectangles:
        // just call draw_rect_quant2() multiple times before flush_nametable().
        draw_rect_quant2(x0, y0, x1, y1);
        }
    }
}

void main(void) {
    long cam_x_fp = 0;
    long cam_z_fp = FP_FROM_INT(5);

    long prev_x, prev_z;
    unsigned char stick, key;

    vdp_init_screen2();
    tiles_init();

    memset(vdp_prev, 0xFF, NAMETABLE_SIZE);

    render_square(cam_x_fp, cam_z_fp);
    flush_nametable();

    while (1) {
        if (wait_fps()) continue;

        stick = msx_get_stick(0);
        key   = msx_get_trigger(0);
        (void)key;

        prev_x = cam_x_fp;
        prev_z = cam_z_fp;

        // Z movement
        if (stick == STICK_UP || stick == STICK_UP_LEFT || stick == STICK_UP_RIGHT) cam_z_fp -= CAM_STEP_FP;
        if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT || stick == STICK_DOWN_RIGHT) cam_z_fp += CAM_STEP_FP;

        // X movement
        if (stick == STICK_LEFT || stick == STICK_UP_LEFT || stick == STICK_DOWN_LEFT) cam_x_fp -= CAM_X_STEP_FP;
        if (stick == STICK_RIGHT || stick == STICK_UP_RIGHT || stick == STICK_DOWN_RIGHT) cam_x_fp += CAM_X_STEP_FP;

        if (cam_z_fp < CAM_Z_MIN_FP) cam_z_fp = CAM_Z_MIN_FP;
        if (cam_z_fp > CAM_Z_MAX_FP) cam_z_fp = CAM_Z_MAX_FP;

        if (cam_x_fp < CAM_X_MIN_FP) cam_x_fp = CAM_X_MIN_FP;
        if (cam_x_fp > CAM_X_MAX_FP) cam_x_fp = CAM_X_MAX_FP;

        if (cam_x_fp != prev_x || cam_z_fp != prev_z) {
            render_square(cam_x_fp, cam_z_fp);
            flush_nametable();
        }
    }
}
