#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../utils/utils_msx.h"
#include "../utils/utils_fps.h"

#define SCREEN_MODE 0
#define SCREEN_COLS 40
#define SCREEN_ROWS 24

#define CANVAS_W 160
#define CANVAS_H 132
#define BLOCK_W   4
#define BLOCK_H   6
#define GRID_W   (CANVAS_W / BLOCK_W)   // 40
#define GRID_H   (CANVAS_H / BLOCK_H)   // 22

#define CANVAS_ROW_BYTES (CANVAS_W / 4) // 4 pixels per byte (2bpp)

#define BLOCK_PIXELS (BLOCK_W * BLOCK_H)
#define SUM_MAX (BLOCK_PIXELS * 3)

#define LUMINANCE_LEVELS 10

static uint8_t text_buffer[SCREEN_ROWS][SCREEN_COLS];
static uint16_t name_table_base = 0x1800;

static const char demo_text[] = "HELLO BUFFERED TEXT ANIMATION";
static const char luminance_chars[LUMINANCE_LEVELS] = { ' ', '.', ':', '-', '=', '+', '*', '#', '%', '@' };

// 2bpp packed canvas: 4 pixels per byte -> 160/4 = 40 bytes per row
static uint8_t canvas_bits[CANVAS_H * CANVAS_ROW_BYTES];

// For each byte (4 pixels, 2bpp each), sum pixel values (0..3) -> 0..12
static uint8_t sum4_table[256];

// Map sum (0..SUM_MAX) -> level (0..LUMINANCE_LEVELS - 1)
static uint8_t sum_to_level[SUM_MAX + 1];

// 256-entry sine table scaled to [-127, 127]
static const int8_t sin_table[256] = {
     0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,  46,
    49,  51,  54,  57,  60,  63,  65,  68,  71,  73,  76,  78,  81,  83,  85,  88,
    90,  92,  94,  96,  98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
   117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
   127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
   117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100,  98,  96,  94,  92,
    90,  88,  85,  83,  81,  78,  76,  73,  71,  68,  65,  63,  60,  57,  54,  51,
    49,  46,  43,  40,  37,  34,  31,  28,  25,  22,  19,  16,  12,   9,   6,   3,
     0,  -3,  -6,  -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
   -90, -92, -94, -96, -98,-100,-102,-104,-106,-107,-109,-111,-112,-113,-115,-116,
  -117,-118,-120,-121,-122,-122,-123,-124,-125,-125,-126,-126,-126,-127,-127,-127,
  -127,-127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118,
  -117,-116,-115,-113,-112,-111,-109,-107,-106,-104,-102,-100, -98, -96, -94, -92,
   -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
   -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9,  -6,  -3
};

static uint16_t compute_name_base(uint8_t r2) {
    return (uint16_t)(r2 & 0x0F) << 10;
}

static void clear_buffer(void) {
    memset(&text_buffer[0][0], ' ', sizeof(text_buffer));
}

static void blit_buffer(void) {
    vdp_set_address(name_table_base);
    vdp_write_bytes_otir(&text_buffer[0][0], (uint16_t)(SCREEN_COLS * SCREEN_ROWS));
}

static void write_text_to_buffer(uint8_t x, uint8_t y, const char *text) {
    if (y >= SCREEN_ROWS) return;
    uint8_t *dst = &text_buffer[y][0];
    while (*text && x < SCREEN_COLS) {
        dst[x++] = (uint8_t)*text++;
    }
}

static void init_screen(void) {
    vdp_set_screen_mode(SCREEN_MODE);

    uint8_t r2 = vdp_get_reg(2);
    if ((r2 & 0x0F) != 0x06) {
        vdp_set_reg(2, 0x06);
        r2 = 0x06;
    }
    name_table_base = compute_name_base(r2);

    msx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    vdp_set_reg(7, (COLOR_WHITE << 4) | COLOR_BLACK);
}

static int8_t cos_from_sin(uint8_t angle) {
    return sin_table[(uint8_t)(angle + 64)];
}

static void build_sum4_table(void) {
    for (uint16_t i = 0; i < 256; ++i) {
        uint8_t p0 = (uint8_t)((i >> 6) & 3);
        uint8_t p1 = (uint8_t)((i >> 4) & 3);
        uint8_t p2 = (uint8_t)((i >> 2) & 3);
        uint8_t p3 = (uint8_t)(i & 3);
        sum4_table[i] = (uint8_t)(p0 + p1 + p2 + p3); // 0..12
    }
}

static void build_sum_to_level(void) {
    for (uint16_t s = 0; s <= SUM_MAX; ++s) {
        uint16_t scaled = (uint16_t)s * (LUMINANCE_LEVELS - 1);
        sum_to_level[s] = (uint8_t)((scaled + (SUM_MAX / 2)) / SUM_MAX);
    }
}

static void clear_canvas(void) {
    memset(canvas_bits, 0, sizeof(canvas_bits));
}

static void clear_text_rows(void) {
    memset(&text_buffer[GRID_H][0], ' ', SCREEN_COLS);
    memset(&text_buffer[GRID_H + 1][0], ' ', SCREEN_COLS);
}

static void set_pixel(uint8_t x, uint8_t y, uint8_t value) {
    if (x >= CANVAS_W || y >= CANVAS_H) return;

    uint16_t idx = (uint16_t)y * CANVAS_ROW_BYTES + (x >> 2);
    uint8_t shift = (uint8_t)((3 - (x & 3)) << 1);
    uint8_t mask = (uint8_t)(3 << shift);

    uint8_t old = canvas_bits[idx];
    canvas_bits[idx] = (uint8_t)((old & (uint8_t)(~mask)) | (uint8_t)((value & 3) << shift));
}

static void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t value) {
    int16_t dx = x1 - x0;
    int16_t dy = y1 - y0;
    int16_t sx = (dx >= 0) ? 1 : -1;
    int16_t sy = (dy >= 0) ? 1 : -1;
    dx = dx >= 0 ? dx : -dx;
    dy = dy >= 0 ? dy : -dy;
    int16_t err = dx - dy;

    while (1) {
        if ((uint16_t)x0 < CANVAS_W && (uint16_t)y0 < CANVAS_H) {
            set_pixel((uint8_t)x0, (uint8_t)y0, value);
        }
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = err << 1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

static void draw_rotating_rect(uint8_t angle, uint8_t w, uint8_t h, uint8_t value) {
    int8_t s = sin_table[angle];
    int8_t c = sin_table[(uint8_t)(angle + 64)];

    int16_t hw = (int16_t)w / 2;
    int16_t hh = (int16_t)h / 2;
    int16_t cx = CANVAS_W / 2;
    int16_t cy = CANVAS_H / 2;

    int16_t vx[4];
    int16_t vy[4];
    int16_t rx[4];
    int16_t ry[4];

    vx[0] = -hw; vy[0] = -hh;
    vx[1] =  hw; vy[1] = -hh;
    vx[2] =  hw; vy[2] =  hh;
    vx[3] = -hw; vy[3] =  hh;

    for (uint8_t i = 0; i < 4; ++i) {
        int16_t x = vx[i];
        int16_t y = vy[i];
        int16_t tx = (x * c - y * s) >> 7;
        int16_t ty = (x * s + y * c) >> 7;
        rx[i] = cx + tx;
        ry[i] = cy + ty;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        uint8_t j = (uint8_t)((i + 1) & 3);
        draw_line(rx[i], ry[i], rx[j], ry[j], value);
        draw_line(rx[i], (int16_t)(ry[i] + 1), rx[j], (int16_t)(ry[j] + 1), value);
        draw_line((int16_t)(rx[i] + 1), ry[i], (int16_t)(rx[j] + 1), ry[j], value);
    }
}


static void canvas_to_ascii_into_buffer(void) {
    for (uint8_t by = 0; by < GRID_H; ++by) {
        uint8_t *dst = &text_buffer[by][0];

        // Start at row y = by*6, byte offset in that row = bx
        uint16_t offset_base = (uint16_t)by * BLOCK_H * CANVAS_ROW_BYTES;

        for (uint8_t bx = 0; bx < GRID_W; ++bx) {
            uint16_t sum = 0;

            const uint8_t *row = &canvas_bits[offset_base + bx];

            sum += sum4_table[row[0]]; row += CANVAS_ROW_BYTES;
            sum += sum4_table[row[0]]; row += CANVAS_ROW_BYTES;
            sum += sum4_table[row[0]]; row += CANVAS_ROW_BYTES;
            sum += sum4_table[row[0]]; row += CANVAS_ROW_BYTES;
            sum += sum4_table[row[0]]; row += CANVAS_ROW_BYTES;
            sum += sum4_table[row[0]];

            dst[bx] = (uint8_t)luminance_chars[ sum_to_level[(uint8_t)sum] ];
        }
    }
}

void main(void) {
    init_fps();
    init_screen();

    build_sum4_table();
    build_sum_to_level();

    clear_buffer();
    blit_buffer();

    uint8_t text_len = (uint8_t)strlen(demo_text);
    uint8_t x = 0;
    uint8_t y = 22;
    int8_t dx = 1;
    int8_t dy = 1;

    uint8_t angle = 0;

    while (1) {
        wait_vblank();

        clear_canvas();
        draw_rotating_rect(angle, 96, 64, 3);
        angle += 2;

        canvas_to_ascii_into_buffer();
        clear_text_rows();

        write_text_to_buffer(x, y, demo_text);

        blit_buffer();

        if ((dx > 0 && (x + text_len + dx > SCREEN_COLS)) || (dx < 0 && x == 0)) {
            dx = (int8_t)(-dx);
        }
        if (dy > 0 && y >= 23) dy = -1;
        if (dy < 0 && y <= 22) dy = 1;

        x = (uint8_t)(x + dx);
        y = (uint8_t)(y + dy);
    }
}
