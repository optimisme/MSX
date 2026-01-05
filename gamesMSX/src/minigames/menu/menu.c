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
#define C_POS_1_X           6
#define C_POS_1_Y           16
#define C_POS_2_X           6
#define C_POS_2_Y           18
#define C_POS_3_X           18
#define C_POS_3_Y           16
#define C_POS_4_X           18
#define C_POS_4_Y           18

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
        update_menu();
        
        b_stick = msx_get_stick(0);

        if (kbhit()) {   
            uint8_t leave = 0;
            b_key = cgetc();
            if (b_key == ' ' || b_key == '\n' || b_key == '\r') {
                leave = b_option + 1;
            } else if (b_key == '1') {
                leave = 2;
            } else if (b_key == '2') {
                leave = 3;
            } else if (b_key == '3') {
                leave = 4;
            } else if (b_key == '4') {
                leave = 5;
            }
            if (leave != 0) {
                *out = leave;
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

    // Set tilemap to blank
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

    // Set full tileset
    vdp_set_address(MODE_2_VRAM_PATTERN_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    load_alphabet_tileset();

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

    load_alphabet_colors();

    // Set tilemap
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(menu_bitmap_tilemap);
    write_text_to_vram("1.2048",    C_POS_1_X + (C_POS_1_Y << 5));
    write_text_to_vram("2.Flappy",  C_POS_2_X + (C_POS_2_Y << 5));
    write_text_to_vram("3.Snake",   C_POS_3_X + (C_POS_3_Y << 5));
    write_text_to_vram("4.Pong",    C_POS_4_X + (C_POS_4_Y << 5));
    
    // Set sprite
    vdp_set_sprite(0, sprite_arrow, 0);
    vdp_update_sprite(0, 0, COLOR_CYAN, b_cursor_x, b_cursor_y);
}

void tile_to_pixel(uint8_t tile_x, uint8_t tile_y, uint16_t *out_x, uint16_t *out_y) {
    *out_x = (uint16_t)tile_x << 3;
    *out_y = (uint16_t)tile_y << 3;
}

void set_cursor_destination(uint8_t option, uint8_t animation_ms) __banked {

    uint16_t px, py;

    switch (option) {
    case 1:
        tile_to_pixel(C_POS_1_X, C_POS_1_Y, &px, &py);
        break;
    case 2:
        tile_to_pixel(C_POS_2_X, C_POS_2_Y, &px, &py);
        break;
    case 3:
        tile_to_pixel(C_POS_3_X, C_POS_3_Y, &px, &py);
        break;
    case 4:
        tile_to_pixel(C_POS_4_X, C_POS_4_Y, &px, &py);
        break;
    }

    // Offset cursor 
    b_destination_x = px - 10;
    b_destination_y = py - 2;

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

void update_menu() __banked {
    update_cursor_animation();
    vdp_update_sprite(0, 0, COLOR_CYAN, b_cursor_x, b_cursor_y);
}
