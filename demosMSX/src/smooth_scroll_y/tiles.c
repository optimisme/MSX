#include "tiles.h"

#define NUM_SCROLL_FRAMES       4
#define SCROLL_PIXEL_MASK       (NUM_SCROLL_FRAMES - 1)
#define SCROLL_PIXELS_PER_FRAME (8 / NUM_SCROLL_FRAMES)
#define FRAME_PIXEL_MASK        ((NUM_SCROLL_FRAMES * SCROLL_PIXELS_PER_FRAME) - 1)

#define FRAME_TILEMAP_SIZE      (VIEW_W * VIEW_H)
#define CAMERA_MAX_Y_PIXELS     ((TILEMAP_H - VIEW_H) * 8)

#define TILEMAP_COL_SHIFT       5
#define VIEW_COL_SHIFT          5
#define MUL_TILES(id)           ((id) * NUM_TILE_TYPES)

uint8_t  vdp_buffer[NUM_SCROLL_FRAMES][FRAME_TILEMAP_SIZE] = {0};
uint8_t  buf_first_row = 0;
uint16_t camera_tile_y = 0;

uint8_t tilemap[TILEMAP_W * TILEMAP_H] = {0};
uint8_t lut[NUM_SCROLL_FRAMES][NUM_TILE_TYPES*NUM_TILE_TYPES] = {0};
uint8_t colmap[TILEMAP_W * TILEMAP_H] = {0};

const uint8_t bck_patterns[NUM_TILE_TYPES][8] = {
    {0b10000000,0b00001000,0b10000000,0b00001000,0b10000000,0b00001000,0b10000000,0b00001000},
    {0b10000001,0b01000010,0b00100100,0b00011000,0b00011000,0b00100100,0b01000010,0b10000001},
    {0b10101010,0b01010101,0b10101010,0b01010101,0b10101010,0b01010101,0b10101010,0b01010101},
    {0b11100111,0b11000011,0b10000001,0b00000000,0b00000000,0b10000001,0b11000011,0b11100111},
    {0b11100111,0b11000011,0b10000001,0b00000000,0b00000000,0b10000001,0b11000011,0b11100111}
};

const uint8_t fg_static[NUM_TILE_TYPES] = {
    COLOR_WHITE, COLOR_DARK_RED, COLOR_LIGHT_GREEN, COLOR_LIGHT_BLUE, COLOR_LIGHT_RED
};
const uint8_t bg_static[NUM_TILE_TYPES] = {
    COLOR_BLACK, COLOR_LIGHT_YELLOW, COLOR_BLACK, COLOR_CYAN, COLOR_MAGENTA
};

void fill_buffer_row(uint16_t map_row, uint8_t buf_row_idx) {
    // Assegurem-nos que map_row està dins dels límits
    if (map_row >= TILEMAP_H)
        map_row = TILEMAP_H - 1;
    
    uint16_t buffer_offset = buf_row_idx * VIEW_W;
    uint8_t *b0 = &vdp_buffer[0][buffer_offset];
    uint8_t *b1 = &vdp_buffer[1][buffer_offset];
    uint8_t *b2 = &vdp_buffer[2][buffer_offset];
    uint8_t *b3 = &vdp_buffer[3][buffer_offset];

    // First column pointer for this row
    uint16_t map_offset = map_row * TILEMAP_W;
    uint8_t *cp = &colmap[map_offset];

    for (uint8_t x = 0; x < VIEW_W; ++x) {
        uint8_t col = *cp++;         
        b0[x] = lut[0][col];   
        b1[x] = lut[1][col];
        b2[x] = lut[2][col];
        b3[x] = lut[3][col];
    }
}

void write_buffer_to_vram(uint8_t frame) {
    uint8_t *buf = vdp_buffer[frame];

    if (buf_first_row == 0) {
        // Without rotation
        vdp_set_address(NAME_TABLE);
        vdp_write_bytes(buf, FRAME_TILEMAP_SIZE);
    } else {
        // With rotation
        uint8_t full_buffer[FRAME_TILEMAP_SIZE];
        uint16_t top_len = (VIEW_H - buf_first_row) * VIEW_W;

        memcpy(full_buffer, buf + buf_first_row * VIEW_W, top_len);
        memcpy(full_buffer + top_len, buf, buf_first_row * VIEW_W);

        vdp_set_address(NAME_TABLE);
        vdp_write_bytes(full_buffer, FRAME_TILEMAP_SIZE);
    }
}

void init_tiles_0(void) {

}

void init_tiles_1() {
    // Generate animated tileset
    uint8_t pat[8], col[8];

    for (uint8_t top = 0; top < NUM_TILE_TYPES; ++top) {
        for (uint8_t bot = 0; bot < NUM_TILE_TYPES; ++bot) {
            uint8_t base = (top * NUM_TILE_TYPES + bot) << 2; // Base times 4 frames

            for (uint8_t d = 0; d < NUM_SCROLL_FRAMES; ++d) {
                uint8_t idx = base | d;

                for (uint8_t y = 0; y < 8; ++y) {
                    // Generate frames for positions 0, 2, 4, 6 pixels
                    uint8_t src = y + (d * 2); // Positions 0, 2, 4, 6
                    uint8_t sel = (src < 8) ? top : bot;
                    pat[y] = bck_patterns[sel][src & 7];
                    col[y] = (fg_static[sel] << 4) | (bg_static[sel] & 0x0F);
                }

                for (uint8_t block = 0; block < 3; ++block)
                    vdp_set_tile(block, idx, pat, col);
            }
        }
    }
}

void init_tiles_2() {
    // Generate tilemap
    uint8_t t;
    uint16_t idx = 0;
    
    for (uint8_t y = 0; y < TILEMAP_H; ++y) {
        for (uint8_t x = 0; x < TILEMAP_W; ++x, ++idx) {
            t = 0;
            if ((x == 0 || x == TILEMAP_W-1) || 
                (y == 0 || y == TILEMAP_H-1)) {
                t = (((x + y) % 5) == 0) ? 2 : 1;
            } else {
                for (uint8_t z = 16; z < 200; z += 16) {
                    if (y == (x + z)) {
                        if ((z % 32) == 0) {
                            t = (y % 5) ? 2 : 3;
                        } else {
                            t = (y % 5) ? 2 : 4;
                        }
                    }
                }
            }
            tilemap[idx] = t;
        }
    }
}

void init_tiles_3(uint8_t *mul_tiles_lut) {

    // Initialize "mul_tiles_lut"
    for (uint8_t i = 0; i < NUM_TILE_TYPES; ++i) {
        mul_tiles_lut[i] = MUL_TILES(i);
    }
}

void init_tiles_4(uint8_t *mul_tiles_lut) {

    // Every "top/bottom" combination
    for (uint8_t top = 0; top < NUM_TILE_TYPES; ++top) {
        for (uint8_t bot = 0; bot < NUM_TILE_TYPES; ++bot) {
          uint8_t row  = mul_tiles_lut[top] + bot;   // 0…24
          uint8_t base = row << 2;                   // = row*4
  
          lut[0][row] = base + 0;
          lut[1][row] = base + 1;
          lut[2][row] = base + 2;
          lut[3][row] = base + 3;
        }
    }
}

void init_tiles_5(uint8_t *mul_tiles_lut) {

    // Initialize "colmap"
    for (uint16_t y = 0; y < TILEMAP_H - 1; ++y) {
        uint16_t base = y * TILEMAP_W;
        uint16_t next_base = (y + 1) * TILEMAP_W;

        for (uint16_t x = 0; x < TILEMAP_W; ++x) {
            uint8_t top_tile = tilemap[base + x];
            uint8_t bot_tile = tilemap[next_base + x];
            colmap[base + x] = mul_tiles_lut[top_tile] + bot_tile;
        }
    }
}

void init_tiles_6(uint8_t *mul_tiles_lut)  {

    // Last row - use same tile for top and bottom or the first row's tiles as bottom
    uint16_t last_base = (TILEMAP_H - 1) * TILEMAP_W;
    for (uint16_t x = 0; x < TILEMAP_W; ++x) {
        uint8_t top_tile = tilemap[last_base + x];
        uint8_t bot_tile = tilemap[x]; 
        colmap[last_base + x] = mul_tiles_lut[top_tile] + bot_tile;
    }
}

void init_tiles_7(void) {

    for (uint8_t r = 0; r < VIEW_H; ++r)
        fill_buffer_row(r, r);

    scroll_to(0);
}

void scroll_to(unsigned int cam_y) {

    if (cam_y > CAMERA_MAX_Y_PIXELS) cam_y = CAMERA_MAX_Y_PIXELS;
    uint8_t disp  = (cam_y % (NUM_SCROLL_FRAMES * SCROLL_PIXELS_PER_FRAME)) / SCROLL_PIXELS_PER_FRAME;
    uint8_t frame = (cam_y & FRAME_PIXEL_MASK) / SCROLL_PIXELS_PER_FRAME;

    uint16_t new_sy = cam_y >> 3;
    int16_t diff = (int16_t)new_sy - (int16_t)camera_tile_y;

    uint16_t sy = camera_tile_y;
    uint8_t idx = buf_first_row;

    if (diff > 0) {
        while (diff--) {
            fill_buffer_row(sy + VIEW_H, idx);
            sy++;
            if (++idx == VIEW_H) idx = 0;
        }
    } else if (diff < 0) {
        while (diff++) {
            if (idx == 0) idx = VIEW_H - 1;
            else idx--;
            fill_buffer_row(sy - 1, idx);
            sy--;
        }
    }

    buf_first_row = idx;
    camera_tile_y = sy;

    write_buffer_to_vram(frame);
}