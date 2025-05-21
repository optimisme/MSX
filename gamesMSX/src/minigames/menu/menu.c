#pragma bank 1

#include "menu.h"

#define b_key               (vars_buff[0])
#define b_stick             (vars_buff[1])
#define b_option            (vars_buff[2])
#define b_previous_option   (vars_buff[3])
#define b_cursor_x          (vars_buff[4])
#define b_cursor_y          (vars_buff[5])
#define b_start_x           (vars_buff[6])
#define b_start_y           (vars_buff[7])
#define b_destination_x     (vars_buff[8])
#define b_destination_y     (vars_buff[9])
#define b_total_frames      (vars_buff[10])
#define b_current_frame     (vars_buff[11])

#define C_ANIMATION_MS      250
#define C_POS_1_X           35
#define C_POS_1_Y           109
#define C_POS_2_X           35
#define C_POS_2_Y           132
#define C_POS_3_X           135
#define C_POS_3_Y           109
#define C_POS_4_X           135
#define C_POS_4_Y           132

const unsigned char sprite_arrow[8] = {
    0b11001000,
    0b01100100,
    0b00110010,
    0b00011001,
    0b00011001,
    0b00110010,
    0b01100100,
    0b11001000
};

void main_menu(uint8_t selected_menu_option, uint8_t *out) __banked {

    b_option = selected_menu_option;
    set_cursor_destination(selected_menu_option, 0);

    init_fps();
    init_menu();

    while (1) {

        if (wait_fps()) continue;
        
        draw_menu();
        
        b_stick = msx_get_stick(0);

        if (kbhit()) {
            b_key = cgetc();
            if (b_key == ' ' || b_key == '\n') {
                *out = b_option + 1;
                return;
            } else if (b_key == '1') {
                *out = 2;
                return;
            } else if (b_key == '2') {
                *out = 3;
                return;
            } else if (b_key == '3') {
                *out = 4;
                return;
            } else if (b_key == '4') {
                *out = 5;
                return;
            }
        }

        b_previous_option = b_option;
        switch (b_stick) {
            case STICK_UP:
                if (b_option == 2) b_option = 1;
                if (b_option == 4) b_option = 3;
                break;
            case STICK_UP_RIGHT:
                b_option = 3;
                break;
            case STICK_RIGHT:
                if (b_option == 1) b_option = 3;
                if (b_option == 2) b_option = 4;
                break;
            case STICK_DOWN_RIGHT:
                b_option = 4;
                break;
            case STICK_DOWN:
                if (b_option == 1) b_option = 2;
                if (b_option == 3) b_option = 4;
                break; 
            case STICK_DOWN_LEFT:
                b_option = 2;
                break;
            case STICK_LEFT:
                if (b_option == 3) b_option = 1;
                if (b_option == 4) b_option = 2;
                break;
            case STICK_UP_LEFT:
                b_option = 1;
                break;
            default:
                break;
        }
        
        if (b_previous_option != b_option) {
            set_cursor_destination(b_option, C_ANIMATION_MS);
        }
    }
}

void init_menu(void) __banked {

    vdp_set_screen_mode(2);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

    // Set patterns
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        for (uint16_t i = 0; i < MENU_BITMAP_TILE_COUNT; ++i) {
            uint16_t global_idx = bank_base + i;
            const uint8_t *src = &menu_bitmap_tileset[i][0];
            uint8_t *dst = &vdp_global_buff[global_idx * 8];
            memcpy(dst, src, 8);
        }
    }
    vdp_set_address(MODE_2_VRAM_PATTERN_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    // Set colors
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        for (uint16_t i = 0; i < MENU_BITMAP_TILE_COUNT; ++i) {
            uint16_t global_idx = bank_base + i;
            for (uint8_t y = 0; y < 8; ++y) {
                vdp_global_buff[global_idx * 8 + y] = (COLOR_WHITE << 4) | COLOR_BLACK;
            }
        }
    }
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    // Set tilemap
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(menu_bitmap_tilemap);

    // Set sprite
    vdp_set_sprite(0, sprite_arrow, 0);
    vdp_update_sprite(0, 0, COLOR_CYAN, b_cursor_x, b_cursor_y);
}

void set_cursor_destination(uint8_t option, uint8_t animation_ms) __banked {

    switch (option) {
    case 1:
        b_destination_x = C_POS_1_X;
        b_destination_y = C_POS_1_Y;
        break;
    case 2:
        b_destination_x = C_POS_2_X;
        b_destination_y = C_POS_2_Y;
        break;
    case 3:
        b_destination_x = C_POS_3_X;
        b_destination_y = C_POS_3_Y;
        break;
    case 4:
        b_destination_x = C_POS_4_X;
        b_destination_y = C_POS_4_Y;
        break;
    }

    b_start_x = b_cursor_x;
    b_start_y = b_cursor_y;

    // Calcular distància màxima entre punts (Manhattan o Euclidiana aprox)
    int dx = b_destination_x - b_cursor_x;
    int dy = b_destination_y - b_cursor_y;
    uint8_t distance = (dx >= 0 ? dx : -dx) + (dy >= 0 ? dy : -dy); // manhattan

    // Calcular quants frames han de passar durant `animation_ms`
    // frames = (mil·lisegons / 1000) * FPS = animation_ms * FPS / 1000
    uint16_t total_frames = (animation_ms * TARGET_FPS) / 1000;

    // Si la distància és petita, com a mínim 1 frame
    if (distance == 0 || total_frames == 0) total_frames = 1;

    b_total_frames = (uint8_t)total_frames;
    b_current_frame = 0;
}

void update_cursor_animation(void) __banked {
    if (b_current_frame < b_total_frames) {
        int dx = b_destination_x - b_start_x;
        int dy = b_destination_y - b_start_y;

        if (dx >= 0)
            b_cursor_x = b_start_x + (dx * b_current_frame) / b_total_frames;
        else
            b_cursor_x = b_start_x - ((-dx) * b_current_frame) / b_total_frames;

        if (dy >= 0)
            b_cursor_y = b_start_y + (dy * b_current_frame) / b_total_frames;
        else
            b_cursor_y = b_start_y - ((-dy) * b_current_frame) / b_total_frames;

        b_current_frame++;
    } else {
        b_cursor_x = b_destination_x;
        b_cursor_y = b_destination_y;
    }
}

void draw_menu() __banked {
    update_cursor_animation();
    vdp_update_sprite(0, 0, COLOR_CYAN, b_cursor_x, b_cursor_y);
}
