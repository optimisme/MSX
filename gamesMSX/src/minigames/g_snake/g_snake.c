#include "g_snake.h"
#include "../game_utils.h"
#include <stdbool.h>
#include <string.h>

#define GRID_W 32
#define GRID_H 24
#define MAX_LEN 128

#define TILE_BLANK  0x00
#define TILE_SNAKE  0x01
#define TILE_FOOD   0x02
#define TILE_BORDER 0x03
#define TILE_SNAKE_PART_RIGHT_BASE 0x04
#define TILE_SNAKE_PART_LEFT_BASE  0x0B
#define TILE_SNAKE_PART_DOWN_BASE  0x12
#define TILE_SNAKE_PART_UP_BASE    0x19
#define SNAKE_PART_STEPS 7

#define BOARD_X 2
#define BOARD_Y 2
#define BOARD_W 28
#define BOARD_H 20

static uint8_t snake_x[MAX_LEN];
static uint8_t snake_y[MAX_LEN];
static uint8_t snake_len;
static int8_t dir_x, dir_y;
static int8_t next_dir_x, next_dir_y;
static uint8_t food_x, food_y;
static bool game_over;
static bool exit_now = false;
static uint8_t substep;
static uint8_t move_next_x, move_next_y;
static bool move_eat;
static int8_t tail_dir_x, tail_dir_y;

static void init_tiles(void);
static void set_tile(uint8_t x, uint8_t y, uint8_t t);
static void place_food(void);
static void init_game(void);
static void restart_game(void);
static void input(void);
static void update(void);
static void render(void);
static void draw_hud(void);
static void draw_ui_texts(void);
static uint8_t snake_part_tile(int8_t dx, int8_t dy, uint8_t step);

static const uint8_t pat_blank[8] = {0};
static const uint8_t pat_solid[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t pat_food[8]  = {0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};
static const uint8_t pat_snake_part_right[SNAKE_PART_STEPS][8] = {
    {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
    {0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0},
    {0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0},
    {0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0},
    {0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8},
    {0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC},
    {0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE}
};
static const uint8_t pat_snake_part_left[SNAKE_PART_STEPS][8] = {
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
    {0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
    {0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07},
    {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F},
    {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
    {0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F},
    {0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F}
};
static const uint8_t pat_snake_part_down[SNAKE_PART_STEPS][8] = {
    {0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00}
};
static const uint8_t pat_snake_part_up[SNAKE_PART_STEPS][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF},
    {0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF},
    {0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF},
    {0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF},
    {0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
    {0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
};

static const uint8_t col_white[8]  = {COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
                                      COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE};
static const uint8_t col_snake[8]  = {COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN,
                                      COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN};
static const uint8_t col_food[8] = {
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK,
    (COLOR_DARK_GREEN << 4) | COLOR_BLACK
};
static const uint8_t col_border[8] = {COLOR_GRAY, COLOR_GRAY, COLOR_GRAY, COLOR_GRAY,
                                      COLOR_GRAY, COLOR_GRAY, COLOR_GRAY, COLOR_GRAY};

// --- VRAM helpers ---
static void set_tile(uint8_t x, uint8_t y, uint8_t t) {
    msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)y * GRID_W + x, t);
}

static uint8_t rand_x(void) { return (uint8_t)(BOARD_X + (random_8() % BOARD_W)); }
static uint8_t rand_y(void) { return (uint8_t)(BOARD_Y + (random_8() % BOARD_H)); }

// --- Tileset ---
static void init_tiles(void) {
    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile(bank, TILE_BLANK,  pat_blank,  col_white);
        vdp_set_tile(bank, TILE_SNAKE,  pat_solid,  col_snake);
        vdp_set_tile(bank, TILE_FOOD,   pat_food,   col_food);
        vdp_set_tile(bank, TILE_BORDER, pat_solid,  col_border);
        for (uint8_t i = 0; i < SNAKE_PART_STEPS; ++i) {
            vdp_set_tile(bank, (uint8_t)(TILE_SNAKE_PART_RIGHT_BASE + i), pat_snake_part_right[i], col_snake);
            vdp_set_tile(bank, (uint8_t)(TILE_SNAKE_PART_LEFT_BASE + i),  pat_snake_part_left[i],  col_snake);
            vdp_set_tile(bank, (uint8_t)(TILE_SNAKE_PART_DOWN_BASE + i),  pat_snake_part_down[i],  col_snake);
            vdp_set_tile(bank, (uint8_t)(TILE_SNAKE_PART_UP_BASE + i),    pat_snake_part_up[i],    col_snake);
        }
    }
}

// --- UI ---
static void draw_ui_texts(void) {
    // bottom help
    write_text_to_vram("(E)xit game", 23 * 32 + 1);
    write_text_to_vram("(R)estart",   23 * 32 + 22);
}

static void draw_hud(void) {
    // top-left HUD label
    char buf[32];
    sprintf(buf, "Length: %u", snake_len);
    write_text_to_vram(buf, 0 * 32 + 1);
}

// --- Game setup ---
static void init_game(void) {

    // Set tilemap to blank
    vdp_set_screen_mode(2);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

    init_tiles();

    // clear tilemap + colors
    memset(vdp_tilemap_buff, TILE_BLANK, GRID_W * GRID_H);
    memset(vdp_global_buff, (COLOR_WHITE<<4)|COLOR_BLACK, VDP_GLOBAL_SIZE);
    // set FOOD colors
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t fidx = bank * 256 + TILE_FOOD;
        for (uint8_t r = 0; r < 8; ++r) vdp_global_buff[fidx*8 + r] = (col_food[r]<<4)|col_food[r];
    }
    // set BORDER colors
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bidx = bank * 256 + TILE_BORDER;
        for (uint8_t r = 0; r < 8; ++r) vdp_global_buff[bidx*8 + r] = (col_border[r]<<4)|col_border[r];
    }

    // push to VRAM
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    load_alphabet_tileset();
    load_alphabet_colors();
    draw_ui_texts();

    // border
    for (uint8_t x = 1; x < GRID_W-1; ++x) {
        set_tile(x, 1,        TILE_BORDER);
        set_tile(x, GRID_H-2, TILE_BORDER);
    }
    for (uint8_t y = 1; y < GRID_H-1; ++y) {
        set_tile(1,        y, TILE_BORDER);
        set_tile(GRID_W-2, y, TILE_BORDER);
    }

    // hide sprites (safety)
    {
        uint8_t empty_pat[8] = {0};
        for (uint8_t i = 0; i < 16; ++i) vdp_update_sprite(i, empty_pat, COLOR_BLACK, 0, 0);
    }

    restart_game();
}

static void restart_game(void) {
    exit_now = false;

    // clear board area
    for (uint8_t y = BOARD_Y; y < BOARD_Y + BOARD_H; ++y)
        for (uint8_t x = BOARD_X; x < BOARD_X + BOARD_W; ++x)
            set_tile(x, y, TILE_BLANK);

    // snake center
    snake_len = 3;
    snake_x[0] = BOARD_X + BOARD_W/2; snake_y[0] = BOARD_Y + BOARD_H/2;
    snake_x[1] = snake_x[0] - 1;      snake_y[1] = snake_y[0];
    snake_x[2] = snake_x[1] - 1;      snake_y[2] = snake_y[0];
    dir_x = 1; dir_y = 0;
    next_dir_x = 1; next_dir_y = 0;
    game_over = false;
    substep = 0;

    // draw initial snake
    for (uint8_t i = 0; i < snake_len; ++i) set_tile(snake_x[i], snake_y[i], TILE_SNAKE);

    place_food();
    draw_hud(); // refresh length counter
}

// --- Game logic ---
static void place_food(void) {
    while (1) {
        uint8_t x = rand_x(), y = rand_y();
        bool hit = false;
        for (uint8_t i = 0; i < snake_len; ++i)
            if (snake_x[i] == x && snake_y[i] == y) { hit = true; break; }
        if (!hit) { food_x = x; food_y = y; break; }
    }
    set_tile(food_x, food_y, TILE_FOOD);
}

static void input(void) {
    // arrows/joystick
    uint8_t s = msx_get_stick(0);
    if (s == STICK_UP    && dir_y == 0) { next_dir_x = 0; next_dir_y = -1; }
    if (s == STICK_DOWN  && dir_y == 0) { next_dir_x = 0; next_dir_y =  1; }
    if (s == STICK_LEFT  && dir_x == 0) { next_dir_x = -1; next_dir_y = 0; }
    if (s == STICK_RIGHT && dir_x == 0) { next_dir_x =  1; next_dir_y = 0; }

    // keys: restart / exit
    if (kbhit()) {
        uint8_t c = cgetc();
        if (c == 'e' || c == 'E' || c == 0x1B) { exit_now = true; game_over = true; return; }
        if (c == 'r' || c == 'R') { restart_game(); }
    }
}

static void update(void) {
    if (substep == 0) {
        dir_x = next_dir_x;
        dir_y = next_dir_y;

        int8_t nx = (int8_t)snake_x[0] + dir_x;
        int8_t ny = (int8_t)snake_y[0] + dir_y;

        // walls
        if (nx < BOARD_X || nx >= BOARD_X + BOARD_W ||
            ny < BOARD_Y || ny >= BOARD_Y + BOARD_H) { game_over = true; return; }

        move_next_x = (uint8_t)nx;
        move_next_y = (uint8_t)ny;
        move_eat = (move_next_x == food_x && move_next_y == food_y);

        // self-collision (ignore old tail if not eating)
        uint8_t last = snake_len - (move_eat ? 0 : 1);
        for (uint8_t i = 0; i < last; ++i) {
            if (snake_x[i] == move_next_x && snake_y[i] == move_next_y) { game_over = true; return; }
        }

        if (!move_eat && snake_len > 1) {
            tail_dir_x = (int8_t)snake_x[snake_len - 1] - (int8_t)snake_x[snake_len - 2];
            tail_dir_y = (int8_t)snake_y[snake_len - 1] - (int8_t)snake_y[snake_len - 2];
        } else {
            tail_dir_x = 0;
            tail_dir_y = 0;
        }
    }

    ++substep;
    if (substep < 8) {
        set_tile(move_next_x, move_next_y, snake_part_tile(dir_x, dir_y, substep));
        if (!move_eat && snake_len > 1) {
            set_tile(snake_x[snake_len - 1], snake_y[snake_len - 1],
                     snake_part_tile(tail_dir_x, tail_dir_y, (uint8_t)(8 - substep)));
        }
        return;
    }

    substep = 0;

    // tail erase or grow
    if (move_eat) {
        if (snake_len < MAX_LEN) ++snake_len;
        place_food();
    } else {
        set_tile(snake_x[snake_len-1], snake_y[snake_len-1], TILE_BLANK);
    }

    // shift
    for (int i = snake_len - 1; i > 0; --i) {
        snake_x[i] = snake_x[i-1];
        snake_y[i] = snake_y[i-1];
    }
    snake_x[0] = move_next_x;
    snake_y[0] = move_next_y;

    set_tile(snake_x[0], snake_y[0], TILE_SNAKE);

    // refresh HUD (cheap)
    draw_hud();
}

static void render(void) { }

static uint8_t snake_part_tile(int8_t dx, int8_t dy, uint8_t step) {
    if (step < 1) step = 1;
    if (step > SNAKE_PART_STEPS) step = SNAKE_PART_STEPS;
    if (dx > 0) return (uint8_t)(TILE_SNAKE_PART_RIGHT_BASE + (step - 1));
    if (dx < 0) return (uint8_t)(TILE_SNAKE_PART_LEFT_BASE + (step - 1));
    if (dy > 0) return (uint8_t)(TILE_SNAKE_PART_DOWN_BASE + (step - 1));
    return (uint8_t)(TILE_SNAKE_PART_UP_BASE + (step - 1));
}

// --- Main ---
void main_g_snake(void) {
    init_fps();
    game_transition_black();
    init_game();

    for (;;) {                       // play → game-over → restart loop
        game_over = false;
        exit_now  = false;

        while (!game_over) {
            if (wait_fps()) continue;
            input();
            if (exit_now) { game_transition_black(); return; } // exit anytime
            update();
            render();
        }

        if (exit_now) { game_transition_black(); return; } // exit after loop without showing message

        write_text_to_vram("GAME OVER", 12 * 32 + 11);

        // wait for R/E at game over
        for (;;) {
            if (kbhit()) {
                uint8_t c = cgetc();
                if (c == 'e' || c == 'E' || c == 0x1B) { game_transition_black(); return; }
                if (c == 'r' || c == 'R') { restart_game(); break; } // back to outer loop
            }
        }
    }
}
