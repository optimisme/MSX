#include "main.h"

#define VIEW_W   32
#define VIEW_H   24
#define BUFFER_SIZE (VIEW_W * VIEW_H)

#define IDX_BLACK 0
#define IDX_RED   1
#define IDX_GREEN 2 
#define IDX_BLUE  3

static const uint8_t palette[4] = {
    COLOR_BLACK,
    COLOR_DARK_RED,
    COLOR_DARK_GREEN,
    COLOR_DARK_BLUE
};

static uint8_t nt_buffer[BUFFER_SIZE];
static uint8_t pattern_lookup[4][4][4][4];

static const uint8_t shift_tbl[4] = {6,4,2,0};
static const uint8_t mask_tbl[4]  = {0x3F,0xCF,0xF3,0xFC};

void load_all_subpatterns(void) {
    // Generate all 4⁴ = 256 subpatterns
    for (uint8_t i0 = 0; i0 < 4; i0++) {
        for (uint8_t i1 = 0; i1 < 4; i1++) {
            for (uint8_t i2 = 0; i2 < 4; i2++) {
                for (uint8_t i3 = 0; i3 < 4; i3++) {
                    uint16_t idx = (i0 << 6) | (i1 << 4) | (i2 << 2) | i3;
                    
                    // Use palette colors
                    uint8_t c0 = palette[i0];
                    uint8_t c1 = palette[i1];
                    uint8_t c2 = palette[i2];
                    uint8_t c3 = palette[i3];
                    
                    uint8_t buf[8];
                    // set 0: row0 left=c0, right=c1; row1 left=c2, right=c3
                    buf[0] = (c0 << 4) | (c1 & 0x0F);
                    buf[1] = (c2 << 4) | (c3 & 0x0F);
                    // duplicate for sets 1..3
                    buf[2] = buf[0]; buf[3] = buf[1];
                    buf[4] = buf[0]; buf[5] = buf[1];
                    buf[6] = buf[0]; buf[7] = buf[1];
                    
                    vdp_set_address(MODE_3_VRAM_PATTERN_BASE + (idx * 8));
                    vdp_write_bytes(buf, 8);
                }
            }
        }
    }
}

void init_pattern_lookup(void) {
    for (uint8_t i0 = 0; i0 < 4; i0++) {
        for (uint8_t i1 = 0; i1 < 4; i1++) {
            for (uint8_t i2 = 0; i2 < 4; i2++) {
                for (uint8_t i3 = 0; i3 < 4; i3++) {
                    pattern_lookup[i0][i1][i2][i3] = (i0 << 6) | (i1 << 4) | (i2 << 2) | i3;
                }
            }
        }
    }
}

void fill_buffer_with(uint8_t color_index) {
    uint8_t pat = pattern_lookup[color_index][color_index][color_index][color_index];
    memset(nt_buffer, pat, BUFFER_SIZE);
}

void draw_pixel_to_buffer(uint8_t x, uint8_t y, uint8_t col) {
    if (x>=64 || y>=48) return;

    uint16_t idx = (y>>1)*VIEW_W + (x>>1);
    uint8_t q = ((y&1)<<1) | (x&1);  
    uint8_t pat = nt_buffer[idx];
    
    pat = (pat & mask_tbl[q]) | (col << shift_tbl[q]);
    
    if (pat != nt_buffer[idx]) {
        nt_buffer[idx] = pat;
    }
}

void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t col) {
    int dx = abs((int)x1 - x0);
    int dy = -abs((int)y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        draw_pixel_to_buffer(x0, y0, col);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err << 1;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_circle(uint8_t cx, uint8_t cy, uint8_t r, uint8_t col) {
    int x = r;
    int y = 0;
    int err = 1 - x;

    while (x >= y) {
        /* 8-way symmetry */
        draw_pixel_to_buffer(cx + x, cy + y, col);
        draw_pixel_to_buffer(cx + y, cy + x, col);
        draw_pixel_to_buffer(cx - y, cy + x, col);
        draw_pixel_to_buffer(cx - x, cy + y, col);
        draw_pixel_to_buffer(cx - x, cy - y, col);
        draw_pixel_to_buffer(cx - y, cy - x, col);
        draw_pixel_to_buffer(cx + y, cy - x, col);
        draw_pixel_to_buffer(cx + x, cy - y, col);

        y++;
        if (err < 0)           err += 2 * y + 1;
        else { x--;            err += 2 * (y - x + 1); }
    }
}

static void update_vram_from_buffer(void) {
    vdp_set_address(MODE_3_TILEMAP_BASE);
    vdp_write_bytes_otir(&nt_buffer[0], BUFFER_SIZE);
}

void main(void) {
    init_fps();
    vdp_set_screen_mode(3);
    load_all_subpatterns(); 
    init_pattern_lookup();
    
    uint8_t x = 32;
    for (;;) {
        if (wait_fps()) { continue; }

        fill_buffer_with(IDX_RED);
    
        draw_pixel_to_buffer(20, 10, IDX_BLACK);
        draw_pixel_to_buffer(20, 11, IDX_GREEN);
        draw_pixel_to_buffer(21, 10, IDX_BLUE);
        draw_pixel_to_buffer(21, 11, IDX_BLACK);

        draw_line(0, 0, 63, 47, IDX_BLACK);
        draw_line(63, 0, x, 47, IDX_BLUE);
        draw_circle(x, 23, 10, IDX_GREEN);

        x++;
        if (x >= 64) x = 0;

        update_vram_from_buffer();
    }
}