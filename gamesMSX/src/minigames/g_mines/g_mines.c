#pragma bank 3

#include "g_mines.h"
#include "loading.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define GRID_W 32
#define GRID_H 24

#define BOARD_SIZE 16
#define BOARD_X 8
#define BOARD_Y 4
#define MINE_COUNT 40

#define TILE_BLANK  0x00
#define TILE_COVER_A 0x01
#define TILE_COVER_B 0x02
#define TILE_BORDER  0x04
#define TILE_FLAG    0x05
#define TILE_MINE    0x06

#define CURSOR_SPRITE 0
#define CURSOR_PATTERN 0

static uint8_t mines[BOARD_SIZE][BOARD_SIZE];
static uint8_t revealed[BOARD_SIZE][BOARD_SIZE];
static uint8_t flagged[BOARD_SIZE][BOARD_SIZE];
static uint8_t counts[BOARD_SIZE][BOARD_SIZE];

static uint8_t cursor_x;
static uint8_t cursor_y;
static uint8_t revealed_count;
static uint8_t flags_count;
static bool first_reveal;
static bool game_over;
static bool exit_now;
static bool game_won;
static uint8_t prev_stick;
static uint8_t move_cooldown;

static void init_tiles(void);
static void set_tile(uint8_t x, uint8_t y, uint8_t t);
static void set_char_tile(uint8_t x, uint8_t y, char c);
static void init_game(void);
static void restart_game(void);
static void input_step(void);
static void update_step(void);
static void draw_hud(void);
static void draw_ui_texts(void);
static void place_mines(void);
static void compute_counts(void);
static void reveal_cell(uint8_t x, uint8_t y);
static void reveal_all_mines(void);
static void update_cursor_sprite(void);
static void hide_sprites(void);
static bool check_win(void);
static void set_cover_tile(uint8_t x, uint8_t y);
static void run_loading_sequence(void);
static void ensure_safe_first(uint8_t x, uint8_t y);

static const uint8_t pat_blank[8] = {0};
static const uint8_t pat_border[8]  = {0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF};
static const uint8_t pat_cover_a[8] = {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
static const uint8_t pat_cover_b[8] = {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA};
static const uint8_t pat_flag[8]    = {0x10,0x30,0x7C,0x30,0x30,0x30,0x30,0x00};
static const uint8_t pat_mine[8]    = {0x10,0x54,0x38,0xFE,0x38,0x54,0x10,0x00};
static const uint8_t pat_cursor[8]= {0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xFF,0xFF};

static const uint8_t col_blank[8] = {
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK,
    (COLOR_BLACK << 4) | COLOR_BLACK
};
static const uint8_t col_cover[8] = {
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK
};
static const uint8_t col_border[8] = {
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK,
    (COLOR_GRAY << 4) | COLOR_BLACK
};
static const uint8_t col_flag[8]  = {
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK,
    (COLOR_CYAN << 4) | COLOR_BLACK
};
static const uint8_t col_mine[8]  = {
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK,
    (COLOR_LIGHT_RED << 4) | COLOR_BLACK
};
static void set_tile(uint8_t x, uint8_t y, uint8_t t) {
    msx_vpoke(MODE_2_TILEMAP_BASE + (uint16_t)y * GRID_W + x, t);
}

static void set_char_tile(uint8_t x, uint8_t y, char c) {
    uint8_t idx = CHAR_TO_INDEX(c);
    if (idx == INVALID_CHAR) return;
    set_tile(x, y, (uint8_t)(ALPHABET_BASE + idx));
}

static void init_tiles(void) {
    for (uint8_t bank = 0; bank < 3; ++bank) {
        vdp_set_tile(bank, TILE_BLANK, pat_blank, col_blank);
        vdp_set_tile(bank, TILE_COVER_A, pat_cover_a, col_cover);
        vdp_set_tile(bank, TILE_COVER_B, pat_cover_b, col_cover);
        vdp_set_tile(bank, TILE_BORDER,  pat_border,  col_border);
        vdp_set_tile(bank, TILE_FLAG,    pat_flag,    col_flag);
        vdp_set_tile(bank, TILE_MINE,    pat_mine,    col_mine);
    }
}

static void draw_ui_texts(void) {
    write_text_to_vram("(F)lag",           22 * 32 + 1);
    write_text_to_vram("(S)pace",          22 * 32 + 22);
    write_text_to_vram("(E)xit game",      23 * 32 + 1);
    write_text_to_vram("(R)estart",        23 * 32 + 22);
}

static void draw_hud(void) {
    char buf[32];
    int16_t remaining = (int16_t)MINE_COUNT - (int16_t)flags_count;
    sprintf(buf, "Minecount: %3d", remaining);
    write_text_to_vram(buf, 0 * 32 + 1);
}

static void init_game(void) {
    vdp_set_screen_mode(2);

    memset(vdp_tilemap_buff, TILE_BLANK, GRID_W * GRID_H);
    memset(vdp_global_buff, (COLOR_WHITE << 4) | COLOR_BLACK, VDP_GLOBAL_SIZE);

    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    init_tiles();

    load_alphabet_tileset();
    load_alphabet_colors();
    draw_ui_texts();

    for (uint8_t i = 0; i < 16; ++i) vdp_update_sprite(i, 0, COLOR_BLACK, 0, 208);
    vdp_set_sprite(CURSOR_SPRITE, pat_cursor, CURSOR_PATTERN);
}

static void restart_game(void) {
    exit_now = false;
    game_over = false;
    game_won = false;
    first_reveal = true;
    revealed_count = 0;
    flags_count = 0;
    prev_stick = 0;
    move_cooldown = 0;

    memset(mines, 0, sizeof(mines));
    memset(revealed, 0, sizeof(revealed));
    memset(flagged, 0, sizeof(flagged));
    memset(counts, 0, sizeof(counts));

    run_loading_sequence();

    init_game();

    for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
        for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
            set_cover_tile(x, y);
        }
    }
    for (uint8_t x = 0; x < BOARD_SIZE + 2; ++x) {
        set_tile((uint8_t)(BOARD_X - 1 + x), (uint8_t)(BOARD_Y - 1), TILE_BORDER);
        set_tile((uint8_t)(BOARD_X - 1 + x), (uint8_t)(BOARD_Y + BOARD_SIZE), TILE_BORDER);
    }
    for (uint8_t y = 0; y < BOARD_SIZE + 2; ++y) {
        set_tile((uint8_t)(BOARD_X - 1), (uint8_t)(BOARD_Y - 1 + y), TILE_BORDER);
        set_tile((uint8_t)(BOARD_X + BOARD_SIZE), (uint8_t)(BOARD_Y - 1 + y), TILE_BORDER);
    }

    cursor_x = 0;
    cursor_y = 0;
    update_cursor_sprite();
    draw_hud();
}

static void place_mines(void) {
    uint8_t placed = 0;
    while (placed < MINE_COUNT) {
        uint8_t x = (uint8_t)(random_8() % BOARD_SIZE);
        uint8_t y = (uint8_t)(random_8() % BOARD_SIZE);
        if (mines[y][x]) continue;
        mines[y][x] = 1;
        ++placed;
    }
}

static void compute_counts(void) {
    for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
        for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
            if (mines[y][x]) { counts[y][x] = 0; continue; }
            uint8_t c = 0;
            for (int8_t dy = -1; dy <= 1; ++dy) {
                for (int8_t dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int8_t nx = (int8_t)x + dx;
                    int8_t ny = (int8_t)y + dy;
                    if (nx < 0 || ny < 0 || nx >= BOARD_SIZE || ny >= BOARD_SIZE) continue;
                    if (mines[(uint8_t)ny][(uint8_t)nx]) ++c;
                }
            }
            counts[y][x] = c;
        }
    }
}

static void reveal_all_mines(void) {
    for (uint8_t y = 0; y < BOARD_SIZE; ++y) {
        for (uint8_t x = 0; x < BOARD_SIZE; ++x) {
            if (mines[y][x]) {
                set_tile((uint8_t)(BOARD_X + x), (uint8_t)(BOARD_Y + y), TILE_MINE);
            }
        }
    }
}

static void reveal_cell(uint8_t x, uint8_t y) {
    if (revealed[y][x] || flagged[y][x]) return;

    if (first_reveal) {
        ensure_safe_first(x, y);
        first_reveal = false;
    }

    if (mines[y][x]) {
        set_tile((uint8_t)(BOARD_X + x), (uint8_t)(BOARD_Y + y), TILE_MINE);
        reveal_all_mines();
        game_over = true;
        return;
    }

    uint8_t queue_x[BOARD_SIZE * BOARD_SIZE];
    uint8_t queue_y[BOARD_SIZE * BOARD_SIZE];
    uint16_t qh = 0, qt = 0;

    queue_x[qt] = x;
    queue_y[qt] = y;
    ++qt;

    uint8_t visited[BOARD_SIZE][BOARD_SIZE] = {0};
    visited[y][x] = 1;

    while (qh < qt) {
        uint8_t cx = queue_x[qh];
        uint8_t cy = queue_y[qh];
        ++qh;

        if (revealed[cy][cx] || flagged[cy][cx]) continue;
        revealed[cy][cx] = 1;
        ++revealed_count;

        uint8_t count = counts[cy][cx];
        uint8_t tile_x = (uint8_t)(BOARD_X + cx);
        uint8_t tile_y = (uint8_t)(BOARD_Y + cy);

        if (count == 0) {
            set_tile(tile_x, tile_y, TILE_BLANK);
            for (int8_t dy = -1; dy <= 1; ++dy) {
                for (int8_t dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int8_t nx = (int8_t)cx + dx;
                    int8_t ny = (int8_t)cy + dy;
                    if (nx < 0 || ny < 0 || nx >= BOARD_SIZE || ny >= BOARD_SIZE) continue;
                    if (!revealed[(uint8_t)ny][(uint8_t)nx] &&
                        !mines[(uint8_t)ny][(uint8_t)nx] &&
                        !visited[(uint8_t)ny][(uint8_t)nx]) {
                        queue_x[qt] = (uint8_t)nx;
                        queue_y[qt] = (uint8_t)ny;
                        ++qt;
                        visited[(uint8_t)ny][(uint8_t)nx] = 1;
                    }
                }
            }
        } else {
            set_char_tile(tile_x, tile_y, (char)('0' + count));
        }
    }

    draw_hud();
    if (check_win()) {
        game_over = true;
        game_won = true;
    }
}

static void run_loading_sequence(void) {
    uint8_t total_frames = fps_is_pal ? 50 : 60;
    bool placement_done = false;

    vdp_set_screen_mode(2);
    loading_init(10, 11);

    for (uint8_t frame = 0; frame < total_frames; ) {
        if (wait_fps()) continue;
        uint8_t percent = (uint8_t)(((uint16_t)(frame + 1) * 100) / total_frames);
        loading_draw_progress(10, 22, 12, percent);
        if (!placement_done && frame >= (total_frames / 3)) {
            place_mines();
            compute_counts();
            placement_done = true;
        }
        ++frame;
    }

    if (!placement_done) {
        place_mines();
        compute_counts();
    }
}

static void ensure_safe_first(uint8_t x, uint8_t y) {
    if (!mines[y][x]) return;

    mines[y][x] = 0;
    for (uint16_t tries = 0; tries < BOARD_SIZE * BOARD_SIZE; ++tries) {
        uint8_t nx = (uint8_t)(random_8() % BOARD_SIZE);
        uint8_t ny = (uint8_t)(random_8() % BOARD_SIZE);
        if ((nx == x && ny == y) || mines[ny][nx]) continue;
        mines[ny][nx] = 1;
        compute_counts();
        return;
    }

    for (uint8_t ny = 0; ny < BOARD_SIZE; ++ny) {
        for (uint8_t nx = 0; nx < BOARD_SIZE; ++nx) {
            if ((nx == x && ny == y) || mines[ny][nx]) continue;
            mines[ny][nx] = 1;
            compute_counts();
            return;
        }
    }
}

static bool check_win(void) {
    uint8_t safe_cells = (uint8_t)(BOARD_SIZE * BOARD_SIZE - MINE_COUNT);
    return revealed_count >= safe_cells;
}

static void update_cursor_sprite(void) {
    uint8_t x = (uint8_t)((BOARD_X + cursor_x) * 8);
    uint8_t y = (uint8_t)((BOARD_Y + cursor_y) * 8 - 1);
    vdp_update_sprite(CURSOR_SPRITE, CURSOR_PATTERN, COLOR_LIGHT_GREEN, x, y);
}

static void hide_sprites(void) {
    for (uint8_t i = 0; i < 16; ++i) {
        vdp_update_sprite(i, 0, COLOR_BLACK, 0, 208);
    }
}

static void set_cover_tile(uint8_t x, uint8_t y) {
    uint8_t tile_x = (uint8_t)(BOARD_X + x);
    uint8_t tile_y = (uint8_t)(BOARD_Y + y);
    uint8_t tile = (((x + y) & 1) == 0) ? TILE_COVER_A : TILE_COVER_B;
    set_tile(tile_x, tile_y, tile);
}

static void input_step(void) {
    uint8_t s = msx_get_stick(0);
    if (s == 0) {
        move_cooldown = 0;
    } else if (s != prev_stick) {
        move_cooldown = 0;
    }

    if (s != 0 && move_cooldown == 0) {
        if (s == STICK_UP && cursor_y > 0) cursor_y--;
        if (s == STICK_DOWN && cursor_y < BOARD_SIZE - 1) cursor_y++;
        if (s == STICK_LEFT && cursor_x > 0) cursor_x--;
        if (s == STICK_RIGHT && cursor_x < BOARD_SIZE - 1) cursor_x++;
        move_cooldown = 6;
    }
    if (move_cooldown > 0) --move_cooldown;
    prev_stick = s;

    if (kbhit()) {
        uint8_t c = cgetc();
        if (c == 'e' || c == 'E' || c == 0x1B) { exit_now = true; game_over = true; return; }
        if (c == 'r' || c == 'R') { restart_game(); return; }
        if (c == 'f' || c == 'F') {
            if (!revealed[cursor_y][cursor_x]) {
                if (flagged[cursor_y][cursor_x]) {
                    flagged[cursor_y][cursor_x] = 0;
                    if (flags_count > 0) --flags_count;
                    set_cover_tile(cursor_x, cursor_y);
                } else {
                    flagged[cursor_y][cursor_x] = 1;
                    ++flags_count;
                    set_tile((uint8_t)(BOARD_X + cursor_x), (uint8_t)(BOARD_Y + cursor_y), TILE_FLAG);
                }
                draw_hud();
            }
        }
        if (c == 's' || c == 'S' || c == ' ') {
            reveal_cell(cursor_x, cursor_y);
        }
    }
}

static void update_step(void) {
    update_cursor_sprite();
}

void main_g_mines(void) __banked {
    init_fps();
    restart_game();

    for (;;) {
        game_over = false;
        exit_now = false;

        while (!game_over) {
            if (wait_fps()) continue;
            input_step();
            if (exit_now) { hide_sprites(); return; }
            update_step();
        }

    if (exit_now) { hide_sprites(); return; }

    if (game_won) {
            const char *msg = "YOU WIN";
            uint8_t len = (uint8_t)strlen(msg);
            uint8_t start_col = (uint8_t)(30 + 1 - len);
            write_text_to_vram(msg, 0 * 32 + start_col);
    } else {
            const char *msg = "GAME OVER";
            uint8_t len = (uint8_t)strlen(msg);
            uint8_t start_col = (uint8_t)(30 + 1 - len);
            write_text_to_vram(msg, 0 * 32 + start_col);
    }

        for (;;) {
            if (kbhit()) {
                uint8_t c = cgetc();
                if (c == 'e' || c == 'E' || c == 0x1B) { hide_sprites(); return; }
                if (c == 'r' || c == 'R') { restart_game(); break; }
            }
        }
    }
}
