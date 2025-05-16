#include "tiles.h"

const unsigned char bck_tiles[TILES][8] = {
    {
        0b10000000,
        0b00001000,
        0b10000000,
        0b00001000,
        0b10000000,
        0b00001000,
        0b10000000,
        0b00001000
    },
    {
        0b10000001,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b10000001
    },
    {
        0b10101010,
        0b01010101,
        0b10101010,
        0b01010101,
        0b10101010,
        0b01010101,
        0b10101010,
        0b01010101
    }
};

unsigned char tilemap[TILEMAP_W * TILEMAP_H];
unsigned char vdp_buffer[VDP_BUFFER_SIZE];

void build_tilemap(void)
{
    for (unsigned char y = 0; y < TILEMAP_H; ++y) {
        for (unsigned char x = 0; x < TILEMAP_W; ++x) {
            unsigned char tile = 0;
            if (x == 0 || x == (TILEMAP_W - 1)) {
                if ((y % 5) == 0) {
                    tile = 2;
                } else {
                    tile = 1;
                }
            } else if (y == 0 || y == (TILEMAP_H - 1)) {
                if ((x % 5) == 0) {
                    tile = 2;
                } else {
                    tile = 1;
                }
            } else if (x == y || (x - 16) == y) {
                if ((x % 5) == 0) {
                    tile = 2;
                } else {
                    tile = 1;
                }
            }
            tilemap[y * TILEMAP_W + x] = tile;            
        }
    }
}

void init_tileset(void)
{
    for (unsigned char block = 0; block < 3; ++block) {
        vdp_set_tile_pattern(block, 0, bck_tiles[0]);
        vdp_set_tile_pattern(block, 1, bck_tiles[1]);
        vdp_set_tile_pattern(block, 2, bck_tiles[2]);
    }
}

void init_colortable(void)
{
    for (unsigned char block = 0; block < 3; ++block) {
        vdp_set_tile_color(block, 0, COLOR_WHITE,    COLOR_BLACK);
        vdp_set_tile_color(block, 1, COLOR_DARK_RED, COLOR_LIGHT_YELLOW);
        vdp_set_tile_color(block, 2, COLOR_LIGHT_GREEN, COLOR_BLACK);
    }
}

void init_tiles(void)
{
    init_tileset();
    init_colortable();
    build_tilemap();
    set_camera(0, 0);
}

void set_camera(unsigned int cam_x, unsigned int cam_y) {

    unsigned int start_tile_x = cam_x >> 3; // cam_x / 8
    unsigned int start_tile_y = cam_y >> 3; // cam_y / 8

    if (start_tile_x > TILE_LIMIT_W)
        start_tile_x = TILE_LIMIT_W;

    if (start_tile_y > TILE_LIMIT_H)
        start_tile_y = TILE_LIMIT_H;

    // Copy tilemap rows (C version)
    for (unsigned char y = 0; y < VIEW_H; ++y) {
        const unsigned char* src = &tilemap[(start_tile_y + y) * TILEMAP_W + start_tile_x];
        memcpy(&vdp_buffer[y * VIEW_W], src, VIEW_W);
    }  

    vdp_set_address(NAME_TABLE);
    vdp_blast_tilemap(vdp_buffer);
}