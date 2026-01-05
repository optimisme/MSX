#include "g_flappy.h"
#include "../game_utils.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define GRID_W 32
#define GRID_H 24

// Tiles
#define TILE_BLANK   0x00
#define TILE_BORDER  0x03

// Àrea jugable com a Snake
#define BOARD_X 2
#define BOARD_Y 2
#define BOARD_W 28
#define BOARD_H 20

// Sprite ocell
#define SP_BIRD 0
#define COL_BIRD COLOR_LIGHT_YELLOW   // o COLOR_DARK_YELLOW si el prefereixes

// --- Prototips ---
static void init_tiles(void);
static void set_tile(uint8_t x, uint8_t y, uint8_t t);
static void init_game(void);
static void restart_game(void);
static void draw_ui_texts(void);
static void draw_hud(void);
static void input_step(void);
static void update_step(void);
static void render_step(void);

// --- Patrons (8x8) ---
static const uint8_t pat_blank[8] = {0};
static const uint8_t pat_solid[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t pat_bird[8]  = {0x18,0x3C,0x7E,0xDB,0xFF,0x7E,0x3C,0x18};

// --- Colors per línia (SCREEN2)
static const uint8_t col_white[8] = {
    COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,
    COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE
};

// --- Estat ---
static int16_t bird_x, bird_y, bird_vy;
static uint16_t pipes_count;
static bool game_over;
static bool exit_now;

// ================== Helpers ==================
static void set_tile(uint8_t x, uint8_t y, uint8_t t) {
    msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)y * GRID_W + x, t);
}

static void init_tiles(void) {
    // Idèntic a Snake: definim tile BLANK i BORDER, però BORDER en blanc (ple)
    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile(bank, TILE_BLANK,  pat_blank, col_white);
        vdp_set_tile(bank, TILE_BORDER, pat_solid, col_white);
    }
}

// ================== UI ==================
static void draw_ui_texts(void) {
    write_text_to_vram("(E)xit game", 23 * 32 + 1);
    write_text_to_vram("(R)estart",   23 * 32 + 22);
}

static void draw_hud(void) {
    char buf[32];
    sprintf(buf, "Pipes: %u", (unsigned)pipes_count);
    write_text_to_vram(buf, 0 * 32 + 1);
}

// ================== Game setup ==================
static void init_game(void) {
    // — seqüència igual que Snake —
    vdp_set_screen_mode(2);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);
    init_tiles();

    // 1) neteja buffers locals
    memset(vdp_tilemap_buff, TILE_BLANK, GRID_W * GRID_H);       // tilemap a 0
    memset(vdp_global_buff, (COLOR_WHITE<<4)|COLOR_BLACK, VDP_GLOBAL_SIZE);

    // 2) ajusta COLORS del tile BORDER a blanc/ blanc (ple)
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bidx = bank * 256 + TILE_BORDER;
        for (uint8_t r = 0; r < 8; ++r)
            vdp_global_buff[bidx*8 + r] = (COLOR_WHITE<<4) | COLOR_WHITE;
    }

    // 3) puja buffers a VRAM
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    // 4) alfabet per text (mateix que Snake/2048)
    load_alphabet_tileset();
    load_alphabet_colors();                                      
    draw_ui_texts();
    draw_hud();

    // 5) marc blanc com Snake
    for (uint8_t x = 1; x < GRID_W-1; ++x) {
        set_tile(x, 1,        TILE_BORDER);
        set_tile(x, GRID_H-2, TILE_BORDER);
    }
    for (uint8_t y = 1; y < GRID_H-1; ++y) {
        set_tile(1,        y, TILE_BORDER);
        set_tile(GRID_W-2, y, TILE_BORDER);
    }

    // 6) amaga sprites i carrega ocell
    {
        uint8_t empty[8] = {0};
        for (uint8_t i = 0; i < 16; ++i)
            vdp_update_sprite(i, empty, COLOR_BLACK, 0, 0);      // mateix call que Snake
    }

    restart_game();
}

static void restart_game(void) {
    exit_now    = false;
    game_over   = false;
    pipes_count = 0;

    // buida zona jugable (per si venim de GAME OVER)
    for (uint8_t y = BOARD_Y; y < BOARD_Y + BOARD_H; ++y)
        for (uint8_t x = BOARD_X; x < BOARD_X + BOARD_W; ++x)
            set_tile(x, y, TILE_BLANK);

    bird_x  = 64;
    bird_y  = 96;
    bird_vy = 0;

    draw_hud();
}

// ================== Loop ==================
static void input_step(void) {
    // Tecles com a Snake: E/ESC surt, R reinicia
    if (kbhit()) {
        uint8_t c = cgetc();
        if (c == 'e' || c == 'E' || c == 0x1B) { exit_now = true; game_over = true; return; }
        if (c == 'r' || c == 'R') { restart_game(); }
        if (c == ' ') { bird_vy = -4; } // flap
    }
}

static void update_step(void) {
    const int8_t GRAV = 1;
    if (bird_vy < 4) bird_vy += GRAV;
    bird_y += bird_vy;

    // Adjust boundaries to match your actual play area
    if (bird_y <= 24 || bird_y >= 160) game_over = true;  // Less restrictive
}

static void render_step(void) {
    // Dibuixa l’ocell: patró 8x8 + color groc (mateix API que Snake)
    vdp_update_sprite(SP_BIRD, pat_bird, COL_BIRD, (uint8_t)bird_x, (uint8_t)bird_y);
}

// ================== Main ==================
void main_g_flappy(void) {
    init_fps();
    game_transition_black();
    init_game();

    for (;;) {
        uint8_t frame = 0;
        const uint8_t SPEED = 2;
        game_over = false;
        exit_now  = false;

        while (!game_over) {
            if (wait_fps()) continue;
            input_step();
            if (exit_now) { game_transition_black(); return; } // sortir immediat
            if (++frame >= SPEED) { frame = 0; update_step(); render_step(); }
        }

        if (exit_now) { game_transition_black(); return; }

        write_text_to_vram("GAME OVER", 12 * 32 + 11);

        // espera R o E com a Snake
        for (;;) {
            if (kbhit()) {
                uint8_t c = cgetc();
                if (c == 'e' || c == 'E' || c == 0x1B) { game_transition_black(); return; }
                if (c == 'r' || c == 'R') { restart_game(); break; }
            }
        }
    }
}
