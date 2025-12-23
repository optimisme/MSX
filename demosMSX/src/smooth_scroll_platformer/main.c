#include "main.h"

void load() {

    uint8_t mul_tiles_lut[NUM_TILE_TYPES] = {0};

    loading_init(10, 11);
    loading_draw_progress(10, 22, 12, 10);
    init_tiles_0();
    loading_draw_progress(10, 22, 12, 20);
    init_tiles_1();
    loading_draw_progress(10, 22, 12, 30);
    init_tiles_2();
    loading_draw_progress(10, 22, 12, 40);
    init_tiles_3(mul_tiles_lut);
    loading_draw_progress(10, 22, 12, 50);
    init_tiles_4(mul_tiles_lut);
    loading_draw_progress(10, 22, 12, 60);
    init_tiles_5(mul_tiles_lut);
    loading_draw_progress(10, 22, 12, 70);
    init_tiles_6(mul_tiles_lut);
    loading_draw_progress(10, 22, 12, 80);
    init_tiles_7();
    loading_draw_progress(10, 22, 12, 90);
}

typedef struct {
    int x;
    int y;
    int vx;
    int vy;
    bool on_ground;
} Actor;

static Actor block;

static bool is_solid_tile(uint8_t t) {
    return (t == 1 || t == 2); /* only ground/platform */
}

static bool collides_box(int x, int y, int height) {
    const int w = TILE_SIZE - 1;
    const int h = height - 1;
    return is_solid_tile(get_tile_at_pixel(x, y)) ||
           is_solid_tile(get_tile_at_pixel(x + w, y)) ||
           is_solid_tile(get_tile_at_pixel(x, y + h)) ||
           is_solid_tile(get_tile_at_pixel(x + w, y + h));
}

static bool solid_below(int x, int y, int height) {
    return is_solid_tile(get_tile_at_pixel(x, y + height)) ||
           is_solid_tile(get_tile_at_pixel(x + (TILE_SIZE - 1), y + height));
}

static bool collides_player(int x, int y) {
    return collides_box(x, y + PLAYER_COL_TOP, PLAYER_COL_H);
}

static bool solid_below_player(int x, int y) {
    int foot_y = y + PLAYER_COL_TOP + PLAYER_COL_H;
    return is_solid_tile(get_tile_at_pixel(x, foot_y)) ||
           is_solid_tile(get_tile_at_pixel(x + (TILE_SIZE - 1), foot_y));
}

static bool aabb_overlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2) && (x1 + w1 > x2) &&
           (y1 < y2 + h2) && (y1 + h1 > y2);
}

static bool collides_enemy(int x, int y) {
    if (aabb_overlap(x, y, TILE_SIZE, TILE_SIZE, block.x, block.y, TILE_SIZE, TILE_SIZE)) return true;
    return collides_box(x, y, TILE_SIZE);
}

static bool solid_below_enemy(int x, int y) {
    int foot_y = y + TILE_SIZE;
    if (aabb_overlap(x, foot_y, TILE_SIZE, 1, block.x, block.y, TILE_SIZE, TILE_SIZE)) return true;
    return is_solid_tile(get_tile_at_pixel(x, foot_y)) ||
           is_solid_tile(get_tile_at_pixel(x + (TILE_SIZE - 1), foot_y));
}

static void draw_sprite_clipped(uint8_t slot, uint8_t pattern, uint8_t color, int x, int y) {
    /* If far offscreen, park sprite transparent to avoid MSX Y=208 terminator */
    if (y >= 208 || y < -16 || x < -32 || x > 287) {
        vdp_update_sprite(slot, pattern, 0, 0, 0); /* color 0 = transparent */
        return;
    }
    if (x < 0) x = 0;
    if (x > 255) x = 255;
    vdp_update_sprite(slot, pattern, color, (uint8_t)x, (uint8_t)y);
}

void main(void) {

    Actor p = {.x = 24, .y = 120, .vx = 0, .vy = 0, .on_ground = false};
    Actor enemy = {.x = 32, .y = 120, .vx = 1, .vy = 0, .on_ground = false};
    block.x = 25 * TILE_SIZE;              /* middle of second platform */
    block.y = (17 * TILE_SIZE) - TILE_SIZE; /* on top of platform row 17 */
    block.vx = 0; block.vy = 0; block.on_ground = false;

    const unsigned char sprite_w = 8;
    const unsigned char sprite_h = PLAYER_HEIGHT;
    const unsigned int max_world_px = TILEMAP_W * TILE_SIZE;
    const unsigned int max_world_py = TILEMAP_H * TILE_SIZE;
    const unsigned int screen_w = 256;
    const unsigned int half_screen_w = screen_w / 2;

    unsigned char stick;
    unsigned char btn;

    unsigned int cam_x = 0;
    uint8_t anim = 0;
    uint8_t anim_timer = 0;
    uint8_t enemy_timer = 0;
    bool prev_jump = false;

    init_fps();
    vdp_set_screen_mode(2);
    load();
    /* upload sprite patterns */
    vdp_set_sprite(0, sprite_player_top, 0);
    vdp_set_sprite(0, sprite_player_legs_a, 1);
    vdp_set_sprite(0, sprite_player_legs_b, 2);
    vdp_set_sprite(0, sprite_enemy, 3);
    vdp_set_sprite(0, sprite_block, 4);

    while (1) {
        if (wait_fps()) continue;

        stick = msx_get_stick(0);
        btn   = msx_get_trigger(0);

        /* horizontal input */
        p.vx = 0;
        if (stick == STICK_LEFT || stick == STICK_UP_LEFT || stick == STICK_DOWN_LEFT) {
            p.vx = -PLAYER_SPEED;
        } else if (stick == STICK_RIGHT || stick == STICK_UP_RIGHT || stick == STICK_DOWN_RIGHT) {
            p.vx = PLAYER_SPEED;
        }

        /* jump */
        bool jump_edge = btn && !prev_jump;
        if (jump_edge && p.on_ground) {
            p.vy = PLAYER_JUMP_VEL;
            p.on_ground = false;
        }
        prev_jump = btn;

        /* gravity */
        p.vy += GRAVITY;
        if (p.vy > MAX_FALL_SPEED) p.vy = MAX_FALL_SPEED;

        /* move horizontally stepwise to avoid tunneling; allow pushing block */
        if (p.vx) {
            int step = (p.vx > 0) ? 1 : -1;
            int steps = p.vx * step;
            for (int i = 0; i < steps; ++i) {
                int next_px = p.x + step;
                /* check block overlap */
                if (aabb_overlap(next_px, p.y + PLAYER_COL_TOP, TILE_SIZE, PLAYER_COL_H,
                                 block.x, block.y, TILE_SIZE, TILE_SIZE)) {
                    int next_bx = block.x + step;
                    /* can block move? */
                    if (!collides_box(next_bx, block.y, TILE_SIZE)) {
                        block.x = next_bx;
                        p.x = next_px;
                    } else {
                        break; /* blocked */
                    }
                } else if (!collides_player(next_px, p.y)) {
                    p.x = next_px;
                } else {
                    break;
                }
            }
        }

        /* if we walked off an edge, clear ground flag so we fall */
        if (p.on_ground && !solid_below_player(p.x, p.y)) {
            p.on_ground = false;
        }

        /* move vertically stepwise */
        if (p.vy) {
            int step = (p.vy > 0) ? 1 : -1;
            int steps = p.vy * step;
            for (int i = 0; i < steps; ++i) {
                int next_py = p.y + step;
                bool hit_block = aabb_overlap(p.x, next_py + PLAYER_COL_TOP, TILE_SIZE, PLAYER_COL_H,
                                              block.x, block.y, TILE_SIZE, TILE_SIZE);
                if (!collides_player(p.x, next_py) && !hit_block) {
                    p.y = next_py;
                    p.on_ground = false;
                } else {
                    if (step > 0) p.on_ground = true;
                    p.vy = 0;
                    break;
                }
            }
        }

        /* clamp to world */
        if (p.x < 0) p.x = 0;
        if (p.x > (int)(max_world_px - sprite_w)) p.x = max_world_px - sprite_w;
        if (p.y < 0) { p.y = 0; p.vy = 0; }
        if (p.y > (int)(max_world_py - sprite_h)) { p.y = max_world_py - sprite_h; p.on_ground = true; p.vy = 0; }

        /* camera follow with small dead-zone */
        int target_cam = cam_x;
        int center = cam_x + half_screen_w;
        if (p.x > center + CAMERA_MARGIN) {
            target_cam = p.x - (half_screen_w + CAMERA_MARGIN);
        } else if (p.x < center - CAMERA_MARGIN) {
            target_cam = p.x - (half_screen_w - CAMERA_MARGIN);
        }
        if (target_cam < 0) target_cam = 0;
        int max_cam = max_world_px - screen_w;
        if (target_cam > max_cam) target_cam = max_cam;
        cam_x = (uint16_t)(target_cam & ~1); /* align to 2 px for scroll tables */

        /* enemy AI: walk, drop off edges, bounce on walls */
        enemy.vy += GRAVITY;
        if (enemy.vy > MAX_FALL_SPEED) enemy.vy = MAX_FALL_SPEED;

        /* horizontal walk */
        if (enemy.vx) {
            int step = (enemy.vx > 0) ? 1 : -1;
            int steps = enemy.vx * step;
            for (int i = 0; i < steps; ++i) {
                if (!collides_enemy(enemy.x + step, enemy.y)) {
                    enemy.x += step;
                } else {
                    enemy.vx = -enemy.vx;
                    break;
                }
            }
        }

        /* if no floor under feet, let it fall */
        if (!solid_below_enemy(enemy.x, enemy.y)) {
            enemy.on_ground = false;
        }

        /* vertical movement */
        if (enemy.vy) {
            int step = (enemy.vy > 0) ? 1 : -1;
            int steps = enemy.vy * step;
            for (int i = 0; i < steps; ++i) {
                if (!collides_enemy(enemy.x, enemy.y + step)) {
                    enemy.y += step;
                    enemy.on_ground = false;
                } else {
                    if (step > 0) enemy.on_ground = true;
                    enemy.vy = 0;
                    break;
                }
            }
        }

        /* turn around if next step hits a wall */
        if (collides_enemy(enemy.x + enemy.vx, enemy.y)) enemy.vx = -enemy.vx;

        /* clamp enemy */
        if (enemy.x < 0) { enemy.x = 0; enemy.vx = 1; }
        if (enemy.x > (int)(max_world_px - sprite_w)) { enemy.x = max_world_px - sprite_w; enemy.vx = -1; }
        if (enemy.y > (int)(max_world_py - TILE_SIZE)) { enemy.y = max_world_py - TILE_SIZE; enemy.vy = 0; enemy.on_ground = true; }

        /* block physics */
        block.vy += GRAVITY;
        if (block.vy > MAX_FALL_SPEED) block.vy = MAX_FALL_SPEED;
        if (block.vy || !solid_below_enemy(block.x, block.y)) {
            int step = (block.vy > 0) ? 1 : -1;
            int steps = block.vy * step;
            for (int i = 0; i < steps; ++i) {
                if (!collides_box(block.x, block.y + step, TILE_SIZE)) {
                    block.y += step;
                    block.on_ground = false;
                } else {
                    if (step > 0) block.on_ground = true;
                    block.vy = 0;
                    break;
                }
            }
        }
        if (block.y > (int)(max_world_py - TILE_SIZE)) { block.y = max_world_py - TILE_SIZE; block.vy = 0; block.on_ground = true; }

        /* animations */
        bool player_moving = (p.vx != 0);
        bool player_air = !p.on_ground;
        if (player_moving && !player_air) {
            if (++anim_timer > 6) { anim_timer = 0; anim ^= 1; }
        } else {
            anim_timer = 0;
            anim = 0;
        }
        if (++enemy_timer > 10) { enemy_timer = 0; }

        /* draw player (two stacked sprites) */
        int sx_i = p.x - (int)cam_x;
        uint8_t sy = p.y;
        if (sy > 199) sy = 199; /* keep 16px stack below disable marker */
        uint8_t legs_pattern = player_air ? 2 : (anim ? 2 : 1);
        draw_sprite_clipped(0, 0, COLOR_LIGHT_BLUE, sx_i, sy);
        draw_sprite_clipped(1, legs_pattern, COLOR_LIGHT_BLUE, sx_i, sy + 8);

        /* draw enemy */
        int ex = enemy.x - (int)cam_x;
        uint8_t ey = enemy.y;
        if (ey >= 208) ey = 207;
        draw_sprite_clipped(2, 3, COLOR_MEDIUM_RED, ex, ey);

        /* draw block */
        int bx = block.x - (int)cam_x;
        uint8_t by = block.y;
        draw_sprite_clipped(3, 4, COLOR_LIGHT_YELLOW, bx, by);

        scroll_to(cam_x);
    }
}
