#include "g_pong.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define GRID_W 32
#define GRID_H 24

#define TILE_BLANK  0x00
#define TILE_BORDER 0x03

#define BOARD_X 2
#define BOARD_Y 2
#define BOARD_W 28
#define BOARD_H 20

#define PADDLE_W 5
#define PADDLE_Y (BOARD_Y + BOARD_H - 2)
#define PADDLE_H 4

#define TILE_SIZE 8

#define BOARD_LEFT_PX   (BOARD_X * TILE_SIZE)
#define BOARD_TOP_PX    (BOARD_Y * TILE_SIZE)
#define BOARD_RIGHT_PX  ((BOARD_X + BOARD_W) * TILE_SIZE - 1)
#define BOARD_LEFT_WALL_PX  (BOARD_LEFT_PX - 1)
#define BOARD_RIGHT_WALL_PX ((BOARD_X + BOARD_W) * TILE_SIZE)
#define BOARD_TOP_WALL_PX   (BOARD_TOP_PX - 1)
#define BOARD_BOTTOM_WALL_PX ((BOARD_Y + BOARD_H) * TILE_SIZE)

#define PADDLE_Y_PX     (PADDLE_Y * TILE_SIZE)
#define PADDLE_W_PX     (PADDLE_W * TILE_SIZE)
#define PADDLE_CENTER_Y_PX (PADDLE_Y_PX + (PADDLE_H / 2))

#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)
#define FP_FROM_INT(x) ((int32_t)(x) << FP_SHIFT)
#define FP_TO_INT(x) ((int16_t)((x) >> FP_SHIFT))
#define FP_MUL(a, b) ((int32_t)(((a) * (b)) >> FP_SHIFT))
#define FP_DIV(a, b) ((int32_t)(((a) << FP_SHIFT) / (b)))

#define BALL_DRAW_RADIUS_PX 4
#define BALL_COLLISION_RADIUS_PX 3

#define PADDLE_SPRITE_BASE 0
#define BALL_SPRITE        (PADDLE_SPRITE_BASE + PADDLE_W)
#define PADDLE_PATTERN     0
#define BALL_PATTERN       1

#define PADDLE_STEP_START  2
#define PADDLE_STEP_MAX    4

#define BALL_SPEED_START_FP 205
#define BALL_SPEED_MAX_FP   1024
#define BALL_SPEED_INC_FP   4

static int32_t ball_cx, ball_cy;
static int8_t ball_vx, ball_vy;
static int32_t ball_speed;
static int16_t paddle_x;
static uint8_t hits;
static uint8_t paddle_step;
static bool game_over;
static bool exit_now;
static bool ball_missed;

static void init_tiles(void);
static void set_tile(uint8_t x, uint8_t y, uint8_t t);
static void init_game(void);
static void restart_game(void);
static void input_step(void);
static void update_step(void);
static void draw_hud(void);
static void draw_ui_texts(void);
static void move_paddle(int8_t delta);
static void update_paddle_sprites(void);

static const uint8_t pat_blank[8] = {0};
static const uint8_t pat_solid[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t pat_paddle[8] = {0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00};
static const uint8_t pat_ball[8]  = {0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};

static const uint8_t col_white[8]  = {COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
                                      COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE};
static const uint8_t col_border[8] = {COLOR_GRAY, COLOR_GRAY, COLOR_GRAY, COLOR_GRAY,
                                      COLOR_GRAY, COLOR_GRAY, COLOR_GRAY, COLOR_GRAY};

static void set_tile(uint8_t x, uint8_t y, uint8_t t) {
    msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)y * GRID_W + x, t);
}

static void init_tiles(void) {
    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile(bank, TILE_BLANK,  pat_blank, col_white);
        vdp_set_tile(bank, TILE_BORDER, pat_solid, col_border);
    }
}

static void draw_ui_texts(void) {
    write_text_to_vram("(E)xit game", 23 * 32 + 1);
    write_text_to_vram("(R)estart",   23 * 32 + 22);
}

static void draw_hud(void) {
    char buf[32];
    sprintf(buf, "Hits: %3u", hits);
    write_text_to_vram(buf, 0 * 32 + 1);
}

static void init_game(void) {
    vdp_set_screen_mode(2);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

    init_tiles();

    memset(vdp_tilemap_buff, TILE_BLANK, GRID_W * GRID_H);
    memset(vdp_global_buff, (COLOR_WHITE << 4) | COLOR_BLACK, VDP_GLOBAL_SIZE);

    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t border_idx = bank * 256 + TILE_BORDER;
        for (uint8_t r = 0; r < 8; ++r) {
            vdp_global_buff[border_idx * 8 + r] = (COLOR_GRAY << 4) | COLOR_BLACK;
        }
    }

    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    load_alphabet_tileset();
    load_alphabet_colors();
    draw_ui_texts();

    for (uint8_t x = 1; x < GRID_W - 1; ++x) {
        set_tile(x, 1,        TILE_BORDER);
        set_tile(x, GRID_H-2, TILE_BORDER);
    }
    for (uint8_t y = 1; y < GRID_H - 1; ++y) {
        set_tile(1,        y, TILE_BORDER);
        set_tile(GRID_W-2, y, TILE_BORDER);
    }

    {
        for (uint8_t i = 0; i < 16; ++i) vdp_update_sprite(i, 0, COLOR_BLACK, 0, 208);
    }

    vdp_set_sprite(0, pat_paddle, PADDLE_PATTERN);
    vdp_set_sprite(1, pat_ball, BALL_PATTERN);

    restart_game();
}

static void restart_game(void) {
    exit_now = false;
    game_over = false;
    ball_missed = false;
    hits = 0;
    ball_speed = BALL_SPEED_START_FP;
    paddle_step = PADDLE_STEP_START;

    for (uint8_t y = BOARD_Y; y < BOARD_Y + BOARD_H; ++y) {
        for (uint8_t x = BOARD_X; x < BOARD_X + BOARD_W; ++x) {
            set_tile(x, y, TILE_BLANK);
        }
    }

    paddle_x = BOARD_LEFT_PX + (BOARD_W * TILE_SIZE - PADDLE_W_PX) / 2;
    update_paddle_sprites();

    ball_cx = FP_FROM_INT(BOARD_LEFT_PX + (BOARD_W * TILE_SIZE) / 2);
    ball_cy = FP_FROM_INT(BOARD_TOP_PX + (BOARD_H * TILE_SIZE) / 2);
    ball_vx = (random_8() % 2 == 0) ? -1 : 1;
    ball_vy = -1;
    vdp_update_sprite(BALL_SPRITE, BALL_PATTERN, COLOR_LIGHT_YELLOW,
                      (uint8_t)(FP_TO_INT(ball_cx) - BALL_DRAW_RADIUS_PX),
                      (uint8_t)(FP_TO_INT(ball_cy) - BALL_DRAW_RADIUS_PX));

    draw_hud();
}

static void update_paddle_sprites(void) {
    for (uint8_t i = 0; i < PADDLE_W; ++i) {
        uint8_t x = (uint8_t)(paddle_x + i * TILE_SIZE);
        vdp_update_sprite((uint8_t)(PADDLE_SPRITE_BASE + i),
                          PADDLE_PATTERN, COLOR_LIGHT_BLUE, x, (uint8_t)PADDLE_Y_PX);
    }
}

static void move_paddle(int8_t delta) {
    int16_t new_x = paddle_x + delta;
    int16_t min_x = BOARD_LEFT_PX;
    int16_t max_x = BOARD_LEFT_PX + BOARD_W * TILE_SIZE - PADDLE_W_PX + TILE_SIZE;

    if (new_x < min_x) new_x = min_x;
    if (new_x > max_x) new_x = max_x;

    if (new_x != paddle_x) {
        paddle_x = new_x;
        update_paddle_sprites();
    }
}

static void input_step(void) {
    int8_t move = 0;
    uint8_t s = msx_get_stick(0);
    if (s == STICK_LEFT || s == STICK_UP_LEFT || s == STICK_DOWN_LEFT) move = -(int8_t)paddle_step;
    if (s == STICK_RIGHT || s == STICK_UP_RIGHT || s == STICK_DOWN_RIGHT) move = (int8_t)paddle_step;

    if (kbhit()) {
        uint8_t c = cgetc();
        if (c == 'e' || c == 'E' || c == 0x1B) { exit_now = true; game_over = true; return; }
        if (c == 'r' || c == 'R') { restart_game(); return; }
        if (c == 'a' || c == 'A') move = -(int8_t)paddle_step;
        if (c == 'd' || c == 'D') move = (int8_t)paddle_step;
    }

    if (move != 0) move_paddle(move);
}

static void update_step(void) {
    int32_t left_bound = FP_FROM_INT(BOARD_LEFT_WALL_PX + BALL_COLLISION_RADIUS_PX);
    int32_t right_bound = FP_FROM_INT(BOARD_RIGHT_WALL_PX - BALL_COLLISION_RADIUS_PX);
    int32_t top_bound = FP_FROM_INT(BOARD_TOP_WALL_PX + BALL_COLLISION_RADIUS_PX);
    int32_t bottom_bound = FP_FROM_INT(BOARD_BOTTOM_WALL_PX - BALL_COLLISION_RADIUS_PX);
    int32_t delta_x = (int32_t)ball_vx * ball_speed;
    int32_t delta_y = (int32_t)ball_vy * ball_speed;
    int32_t next_cx = ball_cx + delta_x;
    int32_t next_cy = ball_cy + delta_y;

    if (next_cx < left_bound) {
        next_cx = left_bound;
        ball_vx = 1;
    } else if (next_cx > right_bound) {
        next_cx = right_bound;
        ball_vx = -1;
    }

    if (next_cy < top_bound) {
        next_cy = top_bound;
        ball_vy = 1;
    }

    delta_x = next_cx - ball_cx;
    delta_y = next_cy - ball_cy;

    if (!ball_missed && ball_vy > 0 && delta_y > 0) {
        int32_t line_y = FP_FROM_INT(PADDLE_CENTER_Y_PX - BALL_COLLISION_RADIUS_PX);
        if (ball_cy <= line_y && next_cy >= line_y) {
            int32_t t = FP_DIV(line_y - ball_cy, delta_y);
            int32_t hit_x = ball_cx + FP_MUL(delta_x, t);
            int32_t left = FP_FROM_INT(paddle_x - BALL_COLLISION_RADIUS_PX);
            int32_t right = FP_FROM_INT(paddle_x + PADDLE_W_PX - 1 + BALL_COLLISION_RADIUS_PX);

            if (hit_x >= left && hit_x <= right) {
                int32_t remaining = FP_ONE - t;
                ball_vy = -1;
                next_cy = line_y - FP_MUL(delta_y, remaining);
                next_cx = hit_x + FP_MUL(delta_x, remaining);

                ++hits;
                if (ball_speed < BALL_SPEED_MAX_FP) {
                    ball_speed += BALL_SPEED_INC_FP;
                    if (ball_speed > BALL_SPEED_MAX_FP) ball_speed = BALL_SPEED_MAX_FP;
                }
                if (paddle_step < PADDLE_STEP_MAX) {
                    ++paddle_step;
                }
                draw_hud();
            } else {
                ball_missed = true;
            }
        }
    }

    if (next_cx < left_bound) {
        next_cx = left_bound;
        ball_vx = 1;
    } else if (next_cx > right_bound) {
        next_cx = right_bound;
        ball_vx = -1;
    }

    if (ball_missed && next_cy >= bottom_bound) {
        ball_cx = next_cx;
        ball_cy = bottom_bound;
        vdp_update_sprite(BALL_SPRITE, BALL_PATTERN, COLOR_LIGHT_YELLOW,
                          (uint8_t)(FP_TO_INT(ball_cx) - BALL_DRAW_RADIUS_PX),
                          (uint8_t)(FP_TO_INT(ball_cy) - BALL_DRAW_RADIUS_PX));
        game_over = true;
        return;
    }

    ball_cx = next_cx;
    ball_cy = next_cy;

    vdp_update_sprite(BALL_SPRITE, BALL_PATTERN, COLOR_LIGHT_YELLOW,
                      (uint8_t)(FP_TO_INT(ball_cx) - BALL_DRAW_RADIUS_PX),
                      (uint8_t)(FP_TO_INT(ball_cy) - BALL_DRAW_RADIUS_PX));
}

void main_g_pong(void) {
    init_fps();
    init_game();

    for (;;) {
        game_over = false;
        exit_now = false;

        while (!game_over) {
            if (wait_fps()) continue;
            input_step();
            if (exit_now) return;
            update_step();
        }

        if (exit_now) return;

        write_text_to_vram("GAME OVER", 12 * 32 + 11);

        for (;;) {
            if (kbhit()) {
                uint8_t c = cgetc();
                if (c == 'e' || c == 'E' || c == 0x1B) return;
                if (c == 'r' || c == 'R') { restart_game(); break; }
            }
        }
    }
}
