#include <msx.h>
#include <string.h>
#include <video/tms99x8.h> 

void vdp_set_address(unsigned int addr) __naked __z88dk_fastcall;
void vdp_blast_tilemap(const unsigned char* src) __naked __z88dk_fastcall;
void vdp_write_bytes(const unsigned char *src, unsigned int len) __z88dk_callee;
void vdp_set_tile_pattern(uint8_t bank, uint8_t id, const uint8_t* data);


#define DATA_PORT 0x98
#define CRTL_PORT 0x99
#define MODE_2_VRAM_PATTERN_BASE     0x0000  
#define MODE_2_TILEMAP_BASE          0x1800
#define MODE_2_COLOR_BASE            0x2000
#define MODE_2_PATTERN_BLOCK_SIZE    0x0800
#define MODE_2_VRAM_COLOR_BASE       0x2000
#define MODE_2_COLOR_BLOCK_SIZE      0x0800
#define COLOR_BLACK             1
#define COLOR_WHITE             15

#define VIEW_W 32
#define VIEW_H 24
#define TILE_BLOCK_SIZE (256 * 8)

#define PAT_OFFSET(g,id,row) ((g) * TILE_BLOCK_SIZE + (id) * 8 + (row))
#define PAT_PTR(g,id) (&tile_patterns[(g) * TILE_BLOCK_SIZE + (id) * 8])

unsigned char tile_patterns[3 * TILE_BLOCK_SIZE];
unsigned char vdp_buffer[768];

#define MAX_DIRTY_TILES 256

typedef struct {
    uint8_t group;
    uint8_t tile_id;
} DirtyTile;

DirtyTile dirty_tiles[MAX_DIRTY_TILES];
uint8_t dirty_flags[3][256];
uint8_t dirty_count = 0;

void mark_dirty_tile(uint8_t group, uint8_t tile_id) {
    if (!dirty_flags[group][tile_id]) {
        if (dirty_count < MAX_DIRTY_TILES) {
            dirty_tiles[dirty_count].group = group;
            dirty_tiles[dirty_count].tile_id = tile_id;
            dirty_flags[group][tile_id] = 1;
            dirty_count++;
        }
    }
}

void flush_dirty_tiles(void) {
    for (uint8_t i = 0; i < dirty_count; i++) {
        uint8_t g = dirty_tiles[i].group;
        uint8_t id = dirty_tiles[i].tile_id;
        vdp_set_tile_pattern(g, id, PAT_PTR(g, id));
        dirty_flags[g][id] = 0;
    }
    dirty_count = 0;
}

uint current_screen_mode = 0xFF;
void vdp_set_screen_mode(uint8_t value) {
    if (current_screen_mode == value) return;
    current_screen_mode = value;
    msx_screen(value);
    
    if (value == 2) {
        vdp_set_reg(3, 0xFF);
        vdp_set_reg(4, 0x03);
    } else if (value == 3) {
        vdp_set_reg(0, 0x00);
        vdp_set_reg(2, 0x02);
        vdp_set_reg(4, 0x00);
    }
}

void vdp_write_bytes(const unsigned char *src, unsigned int len) __naked __z88dk_callee {
__asm
    pop  bc 
    pop  de
    pop  hl
    push bc
    ld   c, DATA_PORT
vwb_loop:            
    ld   a, (hl)         
    out  (c), a          
    inc  hl              
    dec  de              
    ld   a, d
    or   e
    jr   nz, vwb_loop
    ret 
__endasm;
}


void vdp_set_tile_pattern(uint8_t bank, uint8_t id, const uint8_t* data) {
    vdp_set_address(MODE_2_VRAM_PATTERN_BASE + (bank << 11) + (id << 3));
    vdp_write_bytes(data, 8);
}


void vdp_set_tile_color(uint8_t tile_bank, uint8_t tile_index, uint8_t fg_color, uint8_t bg_color) {
    uint8_t attribute = (fg_color << 4) | (bg_color & 0x0F);
    uint8_t buffer[8];
    memset(buffer, attribute, 8);

    uint16_t offset = (tile_bank << 11) | (tile_index << 3);
    uint16_t address = MODE_2_VRAM_COLOR_BASE + offset;

    vdp_set_address(address);
    vdp_write_bytes(buffer, 8);
}

void init_tile_patterns(void) {
    memset(tile_patterns, 0, sizeof(tile_patterns));
    for (unsigned char g = 0; g < 3; ++g) {
        for (unsigned int id = 0; id < 256; ++id) {
            vdp_set_tile_pattern(g, id, PAT_PTR(g, id));
        }
    }
}

void init_tile_colors(void) {
    for (unsigned char g = 0; g < 3; ++g) {
        for (unsigned int id = 0; id < 256; ++id) {
            vdp_set_tile_color(g, id, COLOR_WHITE, COLOR_BLACK);
        }
    }
}

void vdp_set_address(unsigned int addr) __naked __z88dk_fastcall {
__asm
    ld a, l
    out (CRTL_PORT), a        
    ld a, h
    or 0x40              
    out (CRTL_PORT), a
    REPT 12
      nop
    ENDR
    ret
__endasm;
}

void init_tile_map(void) {
    for (unsigned char y = 0; y < VIEW_H; ++y) {
        for (unsigned char x = 0; x < VIEW_W; ++x) {
            unsigned char group = y / 8;
            unsigned char tile_y_in_group = y % 8;
            unsigned char tile_id = tile_y_in_group * VIEW_W + x;
            vdp_buffer[y * VIEW_W + x] = tile_id;
        }
    }
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_buffer);
}

void vdp_blast_tilemap(const unsigned char* src) __naked __z88dk_fastcall {
__asm
    ld c, DATA_PORT           
    ld de, 768           
vbt_loop:
    ld a, (hl)           
    out (c), a           
    inc hl               
    dec de               
    ld a, d
    or e                 
    jp nz, vbt_loop
    ret
__endasm;
}

void draw_pixel(unsigned int x, unsigned int y, unsigned char set) {
    unsigned char tile_x = x >> 3;
    unsigned char tile_y = y >> 3;
    unsigned char group  = y >> 6;
    unsigned char tile_y_in_group = tile_y & 7;
    unsigned char tile_id = tile_y_in_group * VIEW_W + tile_x;
    unsigned char row_in_tile = y & 7;
    unsigned char mask = 0x80 >> (x & 7);
    unsigned int idx = PAT_OFFSET(group, tile_id, row_in_tile);

    if (set) tile_patterns[idx] |=  mask;
    else     tile_patterns[idx] &= ~mask;

    mark_dirty_tile(group, tile_id);
}

void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char set) {
    int dx = (int)x1 - (int)x0;
    int dy = (int)y1 - (int)y0;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    dx = sx * dx;
    dy = sy * dy;

    int err = (dx > dy ? dx : -dy) >> 1;
    int e2;
    int x = x0;
    int y = y0;

    while (1) {
        draw_pixel(x, y, set);
        if (x == x1 && y == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x += sx; }
        if (e2 < dy)  { err += dx; y += sy; }
    }
}

void main(void) {
    vdp_set_screen_mode(2);
    init_tile_map();
    init_tile_colors();
    init_tile_patterns();

    draw_line(0, 96, 255, 96, 1);

    uint8_t i = 0;
    while (1) {
        draw_line(i, 0, 255, 191, 1);
        flush_dirty_tiles();
        if (i < 255) {
            i += 5;
        } else {
            i = 0;
        }
    }
}
