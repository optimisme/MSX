#include "tiles.h"

#include <string.h>

#define NUM_SCROLL_FRAMES       4
#define SCROLL_PIXEL_MASK       (NUM_SCROLL_FRAMES - 1)
#define SCROLL_PIXELS_PER_FRAME (8 / NUM_SCROLL_FRAMES) 
#define FRAME_PIXEL_MASK        ((NUM_SCROLL_FRAMES * SCROLL_PIXELS_PER_FRAME) - 1)

#define FRAME_TILEMAP_SIZE      (VIEW_W * VIEW_H)
#define CAMERA_MAX_X_PIXELS     ((TILEMAP_W - VIEW_W) * 8)

#define TILEMAP_COL_SHIFT       5
#define VIEW_COL_SHIFT          5

#define MUL_TILES(id)          ((id) * NUM_TILE_TYPES)

uint8_t  vdp_buffer[NUM_SCROLL_FRAMES][FRAME_TILEMAP_SIZE] = {0};
uint8_t  buf_first_col = 0;
uint16_t camera_tile_x = 0;

uint8_t tilemap_layer[TILEMAP_W * TILEMAP_H] = {0};
uint8_t lut[NUM_SCROLL_FRAMES][NUM_TILE_TYPES*NUM_TILE_TYPES] = {0};
uint8_t rowmap[TILEMAP_W * TILEMAP_H] = {0};

const uint8_t bck_patterns[NUM_TILE_TYPES][8] = {
    {0,0,0,0,0,0,0,0}, /* tile 0: empty black sky */
    {0b10000001, 0b01000010, 0b00100100, 0b00011000, 0b00011000, 0b00100100, 0b01000010, 0b10000001},
    {0b10101010, 0b01010101, 0b10101010, 0b01010101, 0b10101010, 0b01010101, 0b10101010, 0b01010101},
    {0b11111111, 0b10100001, 0b11000101, 0b10001001, 0b10010001, 0b10100011, 0b10000101, 0b11111111 },    
    {0b00111100, 0b01000010, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b01000010, 0b00111100 },  
    {0b11000011, 0b10011001, 0b10100101, 0b10100101, 0b10100101, 0b10100101, 0b10011001, 0b11000011 },   
    {0b11000011, 0b10011001, 0b10111101, 0b11111111, 0b11111111, 0b10111101, 0b10011001, 0b11000011 }
};

const uint8_t fg_static[NUM_TILE_TYPES] = {
    COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE
};
const uint8_t bg_static[NUM_TILE_TYPES] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK
};

static void fill_buffer_col(uint16_t map_col, uint8_t buf_col_idx)
{
    uint8_t *b0 = &vdp_buffer[0][buf_col_idx];
    uint8_t *b1 = &vdp_buffer[1][buf_col_idx];
    uint8_t *b2 = &vdp_buffer[2][buf_col_idx];
    uint8_t *b3 = &vdp_buffer[3][buf_col_idx];

    // First row pointer
    uint8_t *row_pointer = &rowmap[map_col];
    uint16_t sum_y = 0;

    for (uint8_t y = 0; y < VIEW_H; ++y) {
        uint8_t row = *row_pointer;
        b0[sum_y] = lut[0][row];
        b1[sum_y] = lut[1][row];
        b2[sum_y] = lut[2][row];
        b3[sum_y] = lut[3][row];

        row_pointer += TILEMAP_W; // pointer to next line
        sum_y += VIEW_W;
    }
}

void write_buffer_to_vram(uint8_t frame)
{
    uint8_t *buf = vdp_buffer[frame];
    
    if (buf_first_col == 0) {
        // Without rotation
        vdp_set_address(MODE_2_TILEMAP_BASE);
        vdp_write_bytes(buf, FRAME_TILEMAP_SIZE);
    } else {
        // With rotation
        uint8_t full_buffer[FRAME_TILEMAP_SIZE];
        uint16_t right_len = VIEW_W - buf_first_col;
        
        for (uint8_t y = 0; y < VIEW_H; ++y) {
            uint16_t row_start = y * VIEW_W;
            uint16_t source_offset = row_start;
            
            memcpy(full_buffer + row_start, buf + source_offset + buf_first_col, right_len);
            memcpy(full_buffer + row_start + right_len, buf + source_offset, buf_first_col);
        }
        
        vdp_set_address(MODE_2_TILEMAP_BASE);
        vdp_write_bytes(full_buffer, FRAME_TILEMAP_SIZE);
    }
}

void init_tiles_0() {

}

void init_tiles_1() {
    // Generate animated tileset
    uint8_t pat[8], col[8];

    for (uint8_t left = 0; left < NUM_TILE_TYPES; ++left) {
        for (uint8_t right = 0; right < NUM_TILE_TYPES; ++right) {
            uint8_t base = (left * NUM_TILE_TYPES + right) << 2;  

            for (uint8_t d = 0; d < NUM_SCROLL_FRAMES; ++d) {
                uint8_t idx = base | d;

                for (uint8_t y = 0; y < 8; ++y) {

                    uint8_t shift = d * SCROLL_PIXELS_PER_FRAME;  
                    uint8_t pattern_left = bck_patterns[left][y] << shift;
                    uint8_t pattern_right = bck_patterns[right][y] >> (8 - shift);                        
                    pat[y] = pattern_left | pattern_right;
                    
                    uint8_t sel = (shift == 0 || pattern_left > pattern_right) ? left : right;
                    col[y] = (fg_static[sel] << 4) | (bg_static[sel] & 0x0F);
                }

                for (uint8_t block = 0; block < 3; ++block)
                    vdp_set_tile(block, idx, pat, col);
            }
        }
    }
}

static void add_platform(uint16_t x0, uint8_t y, uint8_t len, uint8_t tile)
{
    if (y >= TILEMAP_H) return;
    for (uint8_t i = 0; i < len && (x0 + i) < TILEMAP_W; ++i) {
        tilemap_layer[y * TILEMAP_W + x0 + i] = tile;
    }
}

void init_tiles_2() {
    memset(tilemap_layer, 0, sizeof(tilemap_layer));

    /* ground stripe */
    for (uint8_t y = TILEMAP_H - 3; y < TILEMAP_H; ++y) {
        for (uint16_t x = 0; x < TILEMAP_W; ++x) {
            tilemap_layer[y * TILEMAP_W + x] = 1;
        }
    }

    /* steps and platforms */
    add_platform(4, 18, 12, 1);
    add_platform(20, 17, 10, 1);
    add_platform(36, 16, 10, 1);
    add_platform(52, 15, 10, 1);
    add_platform(70, 14, 12, 1);
    add_platform(90, 13, 10, 1);
    add_platform(112, 12, 8, 1);

    /* floating platforms */
    add_platform(30, 9, 6, 2);
    add_platform(64, 8, 8, 2);
    add_platform(96, 6, 8, 2);
    add_platform(126, 10, 6, 2);

    /* hazard pits */
    for (uint16_t x = 55; x < 63; ++x) tilemap_layer[(TILEMAP_H - 3) * TILEMAP_W + x] = 0;
    for (uint16_t x = 102; x < 108; ++x) tilemap_layer[(TILEMAP_H - 3) * TILEMAP_W + x] = 0;
}

void init_tiles_3(uint8_t *mul_tiles_lut) {

    // Initialize "mul_tiles_lut"
    for (uint8_t i = 0; i < NUM_TILE_TYPES; ++i) {
        mul_tiles_lut[i] = MUL_TILES(i);
    }
}

void init_tiles_4(uint8_t *mul_tiles_lut) {

    // Every "left/right" combination
    for (uint8_t left = 0; left < NUM_TILE_TYPES; ++left) {
      for (uint8_t right = 0; right < NUM_TILE_TYPES; ++right) {
        uint8_t row  = mul_tiles_lut[left] + right;   // 0…24
        uint8_t base = row << 2;                      // = row*4

        lut[0][row] = base + 0;
        lut[1][row] = base + 1;
        lut[2][row] = base + 2;
        lut[3][row] = base + 3;
      }
    }
}

void init_tiles_5(uint8_t *mul_tiles_lut) {

    // Initialize "rowmap"
    const uint8_t delta = 1;
    for (uint16_t y = 0; y < TILEMAP_H; ++y) {
        uint16_t base = y * TILEMAP_W;

        for (uint16_t x = 0; x < TILEMAP_W - delta; ++x) {
            uint8_t lt = tilemap_layer[base + x];
            uint8_t rt = tilemap_layer[base + x + delta];
            rowmap[base + x] = mul_tiles_lut[lt] + rt;
        }

        // Last column
        uint8_t last_tile = tilemap_layer[base + TILEMAP_W - 1];
        rowmap[base + TILEMAP_W - 1] = mul_tiles_lut[last_tile] + last_tile;
    }
}

void init_tiles_6(uint8_t *mul_tiles_lut)  { }

void init_tiles_7() {

    for (uint8_t c = 0; c < VIEW_W; ++c)
        fill_buffer_col(c, c);

    scroll_to(0);
}

void scroll_to(unsigned int cam_x)
{
    if (cam_x > CAMERA_MAX_X_PIXELS) cam_x = CAMERA_MAX_X_PIXELS;
    uint8_t frame = (cam_x & FRAME_PIXEL_MASK) / SCROLL_PIXELS_PER_FRAME;

    uint16_t new_sx = cam_x >> 3;
    int16_t diff = (int16_t)new_sx - (int16_t)camera_tile_x;

    uint16_t sx = camera_tile_x;
    uint8_t idx = buf_first_col;

    if (diff > 0) {
        while (diff--) {
            fill_buffer_col(sx + VIEW_W, idx);
            sx++;
            if (++idx == VIEW_W) idx = 0;
        }
    } else if (diff < 0) {
        while (diff++) {
            if (idx == 0) idx = VIEW_W - 1;
            else idx--;
            fill_buffer_col(sx - 1, idx);
            sx--;
        }
    }

    buf_first_col = idx;
    camera_tile_x = sx;

    write_buffer_to_vram(frame);
}

uint8_t get_tile_at_pixel(int16_t px, int16_t py)
{
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    uint16_t tx = (uint16_t)px >> 3;
    uint16_t ty = (uint16_t)py >> 3;
    if (tx >= TILEMAP_W) tx = TILEMAP_W - 1;
    if (ty >= TILEMAP_H) ty = TILEMAP_H - 1;
    return tilemap_layer[ty * TILEMAP_W + tx];
}
