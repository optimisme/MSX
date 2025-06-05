#include "main.h"

#define VIEW_W   32
#define VIEW_H   24
#define BUFFER_SIZE (VIEW_W * VIEW_H)

uint8_t  nt_buffer[BUFFER_SIZE];
uint16_t row_base[VIEW_H];

void load_solid_patterns(void)
{
    uint8_t buf[8];
    for (uint8_t c = 0; c < 16; ++c) {
        uint8_t b = (c << 4) | c; 
        for (uint8_t i = 0; i < 8; ++i) buf[i] = b;
        vdp_set_address(MODE_3_VRAM_PATTERN_BASE + (c * 8));
        vdp_write_bytes(buf, 8);
    }
}

void init_row_base(void)
{
    for (uint8_t y = 0; y < VIEW_H; ++y)
        row_base[y] = ((uint16_t)y) << 5;  /* y * 32 */
}

void fill_buffer_with(uint8_t color)
{
    memset(nt_buffer, color, BUFFER_SIZE);
}

void draw_pixel_to_buffer(uint8_t x, uint8_t y, uint8_t color)
{
    
    if (x >= VIEW_W || y >= VIEW_H) return;
    nt_buffer[row_base[y] + x] = color;
}

static void update_vram_from_buffer(void)
{
    vdp_set_address(MODE_3_TILEMAP_BASE);
    vdp_write_bytes_otir(nt_buffer, BUFFER_SIZE);
}


void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    int dx = abs((int)x1 - x0);
    int dy = -abs((int)y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        draw_pixel_to_buffer(x0, y0, color);
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

void main(void)
{
    init_fps();
    vdp_set_screen_mode(3);
    load_solid_patterns();
    init_row_base();

    uint8_t x = 16, y = 12;
    bool dir = true;

    for (;;) {
        if (wait_fps()) continue;

        fill_buffer_with(COLOR_DARK_GREEN);

        draw_pixel_to_buffer(10, 5, COLOR_BLACK);
        draw_pixel_to_buffer(11, 5, COLOR_CYAN);
        draw_pixel_to_buffer(10, 6, COLOR_DARK_YELLOW);
        draw_pixel_to_buffer(11, 6, COLOR_LIGHT_BLUE);

        draw_line(0, 0, 31, 23, COLOR_BLACK);
        draw_line(16, 0, x, y, COLOR_DARK_RED);
        draw_circle(x, y, 3, COLOR_MAGENTA);

        if (dir) {
            if (++x >= 31) dir = false;
        } else {
            if (--x == 0)  dir = true;
        }

        switch (msx_get_stick(0)) {
            case STICK_DOWN:  if (y < 23) ++y; break;
            case STICK_UP:    if (y > 0)  --y; break;
        }

        update_vram_from_buffer();
    }
}
