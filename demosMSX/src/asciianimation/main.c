#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../utils/utils_msx.h"
#include "../utils/utils_fps.h"

#define SCREEN_MODE 0
#define SCREEN_COLS 40
#define SCREEN_ROWS 24

static uint8_t text_buffer[SCREEN_ROWS][SCREEN_COLS];
static uint16_t name_table_base = 0x1800; // updated at init

static const char demo_text[] = "HELLO BUFFERED TEXT ANIMATION";

static uint16_t compute_name_base(uint8_t r2) {
    return (uint16_t)(r2 & 0x0F) << 10; // bits A10..A13 -> *0x400
}

static void clear_buffer(void) {
    for (uint8_t y = 0; y < SCREEN_ROWS; ++y) {
        memset(&text_buffer[y][0], ' ', SCREEN_COLS);
    }
}

static void blit_buffer(void) {
    for (uint8_t y = 0; y < SCREEN_ROWS; ++y) {
        uint16_t addr = name_table_base + ((uint16_t)y * SCREEN_COLS);
        vdp_set_address(addr);
        vdp_write_bytes_otir(&text_buffer[y][0], SCREEN_COLS);
    }
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
    // Ensure name table is at 0x1800 (reg2=0x06) for SCREEN 0
    uint8_t r2 = vdp_get_reg(2);
    if ((r2 & 0x0F) != 0x06) {
        vdp_set_reg(2, 0x06);
        r2 = 0x06;
    }
    name_table_base = compute_name_base(r2);
    // Set colors via BIOS and VDP reg7 to force black background & white ink.
    msx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    vdp_set_reg(7, (COLOR_WHITE << 4) | COLOR_BLACK); // reg7: FG in upper nibble, BG in lower
}

void main(void) {
    init_fps();
    init_screen();
    clear_buffer();
    blit_buffer();

    uint8_t text_len = (uint8_t)strlen(demo_text);
    uint8_t x = 0;
    uint8_t y = 0;
    int8_t dx = 1;
    int8_t dy = 1;

    while (1) {
        wait_vblank();
        clear_buffer();
        write_text_to_buffer(x, y, demo_text);
        blit_buffer();

        // Move the text diagonally every frame
        if ((dx > 0 && (x + text_len + dx > SCREEN_COLS)) || (dx < 0 && x == 0)) {
            dx = (int8_t)(-dx);
        }
        if ((dy > 0 && (y + 1 + dy > SCREEN_ROWS)) || (dy < 0 && y == 0)) {
            dy = (int8_t)(-dy);
        }

        x = (uint8_t)(x + dx);
        y = (uint8_t)(y + dy);
    }
}
