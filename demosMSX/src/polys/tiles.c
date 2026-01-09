#include "tiles.h"
#include "../utils/utils_fps.h"
#include <stdint.h>
#include <string.h>

#define VRAM_NT 0x1800
#define WORLD_W_UNITS (ANIM_W * 4)
#define WORLD_H_UNITS (ANIM_H * 4)
#define RECT_COUNT 3
#define COLOR_ATTR_WHITE_ON_BLACK ((COLOR_WHITE << 4) | COLOR_BLACK)

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int8_t vx;
    int8_t vy;
} Rect;

static uint8_t nt_buffer[TILEMAP_SIZE];
static uint8_t mask_lr[9][9];
static uint8_t dirty_tiles[ANIM_H][ANIM_W];
static uint8_t color_block[MODE_2_COLOR_BLOCK_SIZE];

static Rect rects[RECT_COUNT] = {
    {2, 2, 14, 10, 1, 0},
    {28, 6, 12, 12, 0, 1},
    {42, 20, 10, 8, -1, -1}
};

static void clamp_rect(Rect *rect)
{
    if (rect->x < 0) rect->x = 0;
    if (rect->y < 0) rect->y = 0;
    if (rect->x + rect->w > WORLD_W_UNITS) rect->x = (int16_t)(WORLD_W_UNITS - rect->w);
    if (rect->y + rect->h > WORLD_H_UNITS) rect->y = (int16_t)(WORLD_H_UNITS - rect->h);
}

static void update_rects(void)
{
    for (uint8_t i = 0; i < RECT_COUNT; ++i) {
        Rect *rect = &rects[i];
        rect->x = (int16_t)(rect->x + rect->vx);
        rect->y = (int16_t)(rect->y + rect->vy);

        if (rect->x <= 0 || rect->x + rect->w >= WORLD_W_UNITS) {
            rect->vx = (int8_t)-rect->vx;
        }
        if (rect->y <= 0 || rect->y + rect->h >= WORLD_H_UNITS) {
            rect->vy = (int8_t)-rect->vy;
        }
        clamp_rect(rect);
    }
}

static void clear_dirty(void)
{
    memset(dirty_tiles, 0x00, sizeof(dirty_tiles));
}

static void mark_dirty_for_rect(const Rect *rect)
{
    if (rect->w <= 0 || rect->h <= 0) return;

    int16_t x0 = rect->x;
    int16_t y0 = rect->y;
    int16_t x1 = (int16_t)(rect->x + rect->w - 1);
    int16_t y1 = (int16_t)(rect->y + rect->h - 1);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= WORLD_W_UNITS) x1 = (int16_t)(WORLD_W_UNITS - 1);
    if (y1 >= WORLD_H_UNITS) y1 = (int16_t)(WORLD_H_UNITS - 1);

    int16_t tx0 = (int16_t)(x0 >> 2);
    int16_t ty0 = (int16_t)(y0 >> 2);
    int16_t tx1 = (int16_t)(x1 >> 2);
    int16_t ty1 = (int16_t)(y1 >> 2);

    tx0 = (int16_t)(tx0 - 1);
    ty0 = (int16_t)(ty0 - 1);
    tx1 = (int16_t)(tx1 + 1);
    ty1 = (int16_t)(ty1 + 1);

    if (tx0 < 0) tx0 = 0;
    if (ty0 < 0) ty0 = 0;
    if (tx1 >= ANIM_W) tx1 = (int16_t)(ANIM_W - 1);
    if (ty1 >= ANIM_H) ty1 = (int16_t)(ANIM_H - 1);

    for (int16_t ty = ty0; ty <= ty1; ++ty) {
        for (int16_t tx = tx0; tx <= tx1; ++tx) {
            dirty_tiles[ty][tx] = 1;
        }
    }
}

static void mark_dirty_for_rects(const Rect *a, const Rect *b)
{
    mark_dirty_for_rect(a);
    mark_dirty_for_rect(b);
}

static void init_masks(void)
{
    for (uint8_t left = 0; left < 9; ++left) {
        for (uint8_t right = 0; right < 9; ++right) {
            uint8_t mask = 0;
            if (right > left) {
                for (uint8_t col = left; col < right; ++col) {
                    mask |= (uint8_t)(0x80 >> col);
                }
            }
            mask_lr[left][right] = mask;
        }
    }
}

static void set_pattern_rect(uint8_t pattern[8], uint8_t left_px, uint8_t top_px, uint8_t right_px, uint8_t bottom_px)
{
    uint8_t mask = mask_lr[left_px][right_px];
    for (uint8_t row = top_px; row < bottom_px; ++row) {
        pattern[row] |= mask;
    }
}

static void build_tile_pattern(uint8_t tile_x, uint8_t tile_y, uint8_t pattern[8])
{
    uint8_t tile_left = (uint8_t)(tile_x * 4);
    uint8_t tile_top = (uint8_t)(tile_y * 4);
    uint8_t tile_right = (uint8_t)(tile_left + 4);
    uint8_t tile_bottom = (uint8_t)(tile_top + 4);

    memset(pattern, 0x00, 8);

    for (uint8_t i = 0; i < RECT_COUNT; ++i) {
        const Rect *rect = &rects[i];
        int16_t left = rect->x;
        int16_t right = (int16_t)(rect->x + rect->w);
        int16_t top = rect->y;
        int16_t bottom = (int16_t)(rect->y + rect->h);

        if (right <= tile_left || left >= tile_right || bottom <= tile_top || top >= tile_bottom) {
            continue;
        }

        uint8_t inter_left = (uint8_t)((left > tile_left) ? left : tile_left);
        uint8_t inter_right = (uint8_t)((right < tile_right) ? right : tile_right);
        uint8_t inter_top = (uint8_t)((top > tile_top) ? top : tile_top);
        uint8_t inter_bottom = (uint8_t)((bottom < tile_bottom) ? bottom : tile_bottom);

        uint8_t left_px = (uint8_t)((inter_left - tile_left) * 2);
        uint8_t right_px = (uint8_t)((inter_right - tile_left) * 2);
        uint8_t top_px = (uint8_t)((inter_top - tile_top) * 2);
        uint8_t bottom_px = (uint8_t)((inter_bottom - tile_top) * 2);

        set_pattern_rect(pattern, left_px, top_px, right_px, bottom_px);
    }
}

void init_anim_demo(void)
{
    uint8_t empty_pattern[8];
    memset(empty_pattern, 0x00, sizeof(empty_pattern));
    init_masks();

    memset(nt_buffer, TILE_EMPTY_IDX, sizeof(nt_buffer));
    for (uint8_t y = 0; y < ANIM_H; ++y) {
        for (uint8_t x = 0; x < ANIM_W; ++x) {
            uint16_t idx = (uint16_t)(y * TILEMAP_W + x);
            nt_buffer[idx] = (uint8_t)(y * ANIM_W + x);
        }
    }

    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile_pattern(bank, TILE_EMPTY_IDX, empty_pattern);
        vdp_set_tile_color(bank, TILE_EMPTY_IDX, COLOR_WHITE, COLOR_BLACK);
    }

    memset(color_block, COLOR_ATTR_WHITE_ON_BLACK, sizeof(color_block));
    vdp_set_address((uint16_t)(MODE_2_VRAM_COLOR_BASE + 0u * MODE_2_COLOR_BLOCK_SIZE));
    vdp_write_bytes_otir(color_block, sizeof(color_block));
    vdp_set_address((uint16_t)(MODE_2_VRAM_COLOR_BASE + 1u * MODE_2_COLOR_BLOCK_SIZE));
    vdp_write_bytes_otir(color_block, sizeof(color_block));

    vdp_set_address(VRAM_NT);
    vdp_blast_tilemap(nt_buffer);
}

void render_anim_frame(void)
{
    Rect old_rects[RECT_COUNT];
    memcpy(old_rects, rects, sizeof(rects));

    update_rects();
    clear_dirty();
    for (uint8_t i = 0; i < RECT_COUNT; ++i) {
        mark_dirty_for_rects(&old_rects[i], &rects[i]);
    }

    wait_vblank();
    for (uint8_t y = 0; y < ANIM_H; ++y) {
        for (uint8_t x = 0; x < ANIM_W; ++x) {
            if (!dirty_tiles[y][x]) continue;
            uint8_t pattern[8];
            build_tile_pattern(x, y, pattern);

            uint8_t bank = (y < 8) ? 0 : 1;
            uint8_t idx = (y < 8) ? (uint8_t)(y * ANIM_W + x) : (uint8_t)((y - 8) * ANIM_W + x);
            uint16_t address = (uint16_t)(MODE_2_VRAM_PATTERN_BASE + (bank * MODE_2_PATTERN_BLOCK_SIZE) + ((uint16_t)idx << 3));
            vdp_set_address(address);
            vdp_write_bytes_otir(pattern, 8);
        }
    }
}
