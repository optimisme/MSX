#include "tiles.h"
#include <stdint.h>

#define PIX_W 256
#define PIX_H 192
#define VRAM_NT 0x1800

#define FP_SHIFT 8
#define FP_ONE (1 << FP_SHIFT)
#define MAX_DDA_STEPS 24
#define WALL_HEIGHT 192

#define VIEW_COLS 24
#define VIEW_ROWS 16
#define VIEW_X 4
#define VIEW_Y 2

static const uint8_t texture_colors[TEXTURE_COUNT] = {
    COLOR_LIGHT_RED,
    COLOR_LIGHT_BLUE,
    COLOR_LIGHT_YELLOW
};

static uint8_t nt_buffer[TILEMAP_SIZE];
static uint8_t tile_patterns[TILE_TOTAL][8];

static const uint8_t map_data[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,2,0,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,2,0,0,1},
    {1,0,2,0,0,0,0,0,0,1,1,0,2,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,2,2,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,2,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,2,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static const int16_t sin_table[256] = {
    0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,  46,
   49,  51,  54,  57,  60,  63,  65,  68,  71,  73,  76,  78,  81,  83,  85,  88,
   90,  92,  94,  96,  98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
  117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 127, 127,
  127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118,
  117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100,  98,  96,  94,  92,
   90,  88,  85,  83,  81,  78,  76,  73,  71,  68,  65,  63,  60,  57,  54,  51,
   49,  46,  43,  40,  37,  34,  31,  28,  25,  22,  19,  16,  12,   9,   6,   3,
    0,  -3,  -6,  -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
  -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
  -90, -92, -94, -96, -98,-100,-102,-104,-106,-107,-109,-111,-112,-113,-115,-116,
 -117,-118,-119,-120,-121,-122,-123,-123,-124,-125,-125,-126,-126,-126,-127,-127,
 -127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-123,-122,-121,-120,-119,-118,
 -117,-116,-115,-113,-112,-111,-109,-107,-106,-104,-102,-100, -98, -96, -94, -92,
  -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
  -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9,  -6,  -3
};

static void build_tileset(void)
{
    for (uint8_t i = 0; i < TILE_TOTAL; ++i) {
        memset(tile_patterns[i], 0x00, 8);
    }

    for (uint8_t tex = 0; tex < TEXTURE_COUNT; ++tex) {
        for (uint8_t start = 0; start < 8; ++start) {
            for (uint8_t end = 0; end < 8; ++end) {
                uint8_t idx = (uint8_t)(TILE_SEG_BASE + tex * TILE_SEG_COUNT + start * 8 + end);
                if (start > end) {
                    memset(tile_patterns[idx], 0x00, 8);
                    continue;
                }
                for (uint8_t row = 0; row < 8; ++row) {
                    if (row >= start && row <= end) {
                        if (tex == 0) {
                            tile_patterns[idx][row] = 0xFF; // brick: solid
                        } else if (tex == 1) {
                            tile_patterns[idx][row] = (row & 1) ? 0x55 : 0xAA; // checker
                        } else {
                            tile_patterns[idx][row] = (row & 1) ? 0xFF : 0x00; // stripe
                        }
                    } else {
                        tile_patterns[idx][row] = 0x00;
                    }
                }
            }
        }
    }
}

static void upload_tileset(void)
{
    for (uint8_t bank = 0; bank < 3; ++bank) {
        for (uint8_t i = 0; i < TILE_TOTAL; ++i) {
            vdp_set_tile_pattern(bank, i, tile_patterns[i]);
        }
    }
}

static void init_color_table(void)
{
    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile_color(bank, TILE_EMPTY_IDX, COLOR_WHITE, COLOR_BLACK);
        for (uint8_t tex = 0; tex < TEXTURE_COUNT; ++tex) {
            uint8_t fg = texture_colors[tex];
            for (uint8_t i = 0; i < TILE_SEG_COUNT; ++i) {
                uint8_t idx = (uint8_t)(TILE_SEG_BASE + tex * TILE_SEG_COUNT + i);
                vdp_set_tile_color(bank, idx, fg, COLOR_BLACK);
            }
        }
    }
}

static uint8_t is_wall(int16_t mx, int16_t my)
{
    if (mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H) {
        return 1;
    }
    return map_data[my][mx] != 0;
}

static uint16_t cast_ray(uint16_t pos_x_fp, uint16_t pos_y_fp, int16_t dir_x, int16_t dir_y, uint8_t* hit_tile)
{
    int16_t map_x = (int16_t)(pos_x_fp >> FP_SHIFT);
    int16_t map_y = (int16_t)(pos_y_fp >> FP_SHIFT);

    uint16_t abs_x = (uint16_t)(dir_x < 0 ? -dir_x : dir_x);
    uint16_t abs_y = (uint16_t)(dir_y < 0 ? -dir_y : dir_y);
    if (abs_x == 0) abs_x = 1;
    if (abs_y == 0) abs_y = 1;

    uint16_t delta_x = (uint16_t)((FP_ONE * 128u) / abs_x);
    uint16_t delta_y = (uint16_t)((FP_ONE * 128u) / abs_y);

    int16_t step_x = (dir_x < 0) ? -1 : 1;
    int16_t step_y = (dir_y < 0) ? -1 : 1;

    uint16_t frac_x = (uint16_t)(pos_x_fp & (FP_ONE - 1));
    uint16_t frac_y = (uint16_t)(pos_y_fp & (FP_ONE - 1));

    uint16_t side_x = (dir_x < 0)
        ? (uint16_t)(((uint32_t)frac_x * delta_x) >> FP_SHIFT)
        : (uint16_t)(((uint32_t)(FP_ONE - frac_x) * delta_x) >> FP_SHIFT);

    uint16_t side_y = (dir_y < 0)
        ? (uint16_t)(((uint32_t)frac_y * delta_y) >> FP_SHIFT)
        : (uint16_t)(((uint32_t)(FP_ONE - frac_y) * delta_y) >> FP_SHIFT);

    uint16_t perp = (uint16_t)(FP_ONE * 8);

    for (uint8_t i = 0; i < MAX_DDA_STEPS; ++i) {
        if (side_x < side_y) {
            side_x = (uint16_t)(side_x + delta_x);
            map_x = (int16_t)(map_x + step_x);
            perp = (uint16_t)(side_x - delta_x);
        } else {
            side_y = (uint16_t)(side_y + delta_y);
            map_y = (int16_t)(map_y + step_y);
            perp = (uint16_t)(side_y - delta_y);
        }
        if (is_wall(map_x, map_y)) {
            if (hit_tile) {
                *hit_tile = map_data[map_y][map_x];
            }
            break;
        }
    }

    if (perp < (FP_ONE / 8)) {
        perp = (uint16_t)(FP_ONE / 8);
    }
    return perp;
}

/* draw_column removed: per-texture column rendering is handled inline */

void init_raycast(void)
{
    build_tileset();
    upload_tileset();
    init_color_table();
}

void render_frame(uint16_t pos_x_fp, uint16_t pos_y_fp, uint8_t angle)
{
    memset(nt_buffer, TILE_EMPTY_IDX, sizeof(nt_buffer));

    for (uint8_t col = 0; col < VIEW_COLS; ++col) {
        int8_t offset = (int8_t)((int8_t)col - (VIEW_COLS / 2));
        uint8_t ray_angle = (uint8_t)(angle + (offset >> 1));

        int16_t dir_x = sin_table[(uint8_t)(ray_angle + 64)];
        int16_t dir_y = sin_table[ray_angle];

        uint8_t hit_tile = 1;
        uint16_t dist_fp = cast_ray(pos_x_fp, pos_y_fp, dir_x, dir_y, &hit_tile);
        dist_fp = (uint16_t)(dist_fp >> 2);
        if (dist_fp == 0) {
            dist_fp = 1;
        }
        uint16_t h = (uint16_t)(((uint32_t)WALL_HEIGHT * FP_ONE) / dist_fp);
        if (h > PIX_H) h = PIX_H;

        uint8_t tex = (hit_tile > 2) ? 2 : (hit_tile == 2 ? 1 : 0);
        uint16_t view_top = (uint16_t)(VIEW_Y << 3);
        uint16_t view_h = (uint16_t)(VIEW_ROWS << 3);
        int16_t view_rel_top = (int16_t)(((int16_t)view_h - (int16_t)h) >> 1);
        if (view_rel_top < 0) view_rel_top = 0;
        int16_t top = (int16_t)(view_top + view_rel_top);
        int16_t bottom = (int16_t)(top + h);
        if (bottom > (int16_t)(view_top + view_h)) bottom = (int16_t)(view_top + view_h);

        for (uint8_t row = 0; row < VIEW_ROWS; ++row) {
            uint16_t tile_start = (uint16_t)((VIEW_Y + row) << 3);
            uint16_t tile_end = (uint16_t)(tile_start + 7);
            uint16_t idx = (uint16_t)((VIEW_Y + row) * TILEMAP_W + (VIEW_X + col));

            if ((bottom <= tile_start) || (top > tile_end)) {
                nt_buffer[idx] = TILE_EMPTY_IDX;
                continue;
            }

            uint8_t start = (top > tile_start) ? (uint8_t)(top - tile_start) : 0;
            uint8_t end = (bottom <= tile_end) ? (uint8_t)(bottom - tile_start - 1) : 7;

            if (start > 7) start = 7;
            if (end > 7) end = 7;

            if (start > end) {
                nt_buffer[idx] = TILE_EMPTY_IDX;
            } else {
                nt_buffer[idx] = (uint8_t)(TILE_SEG_BASE + tex * TILE_SEG_COUNT + (start << 3) + end);
            }
        }
    }

    vdp_set_address(VRAM_NT);
    vdp_blast_tilemap(nt_buffer);
}

void get_dir(uint8_t angle, int16_t* dx, int16_t* dy)
{
    *dx = sin_table[(uint8_t)(angle + 64)];
    *dy = sin_table[angle];
}
