#include "tiles.h"

static const unsigned char quad_patterns[16][8]={
 {0,0,0,0,0,0,0,0},
 {0xF0,0xF0,0xF0,0xF0,0,0,0,0},
 {0x0F,0x0F,0x0F,0x0F,0,0,0,0},
 {0xFF,0xFF,0xFF,0xFF,0,0,0,0},
 {0,0,0,0,0xF0,0xF0,0xF0,0xF0},
 {0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0},
 {0x0F,0x0F,0x0F,0x0F,0xF0,0xF0,0xF0,0xF0},
 {0xFF,0xFF,0xFF,0xFF,0xF0,0xF0,0xF0,0xF0},
 {0,0,0,0,0x0F,0x0F,0x0F,0x0F},
 {0xF0,0xF0,0xF0,0xF0,0x0F,0x0F,0x0F,0x0F},
 {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F},
 {0xFF,0xFF,0xFF,0xFF,0x0F,0x0F,0x0F,0x0F},
 {0,0,0,0,0xFF,0xFF,0xFF,0xFF},
 {0xF0,0xF0,0xF0,0xF0,0xFF,0xFF,0xFF,0xFF},
 {0x0F,0x0F,0x0F,0x0F,0xFF,0xFF,0xFF,0xFF},
 {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
};

static unsigned char tile_mask[VIEW_H][VIEW_W];
static unsigned char tilemap_buf[VIEW_H][VIEW_W];

void init_tileset(void){
    for(unsigned char g=0;g<3;++g)
        for(unsigned char id=0;id<16;++id)
            vdp_set_tile_pattern(g,id,quad_patterns[id]);
    memset(tile_mask,0,sizeof(tile_mask));
    memset(tilemap_buf,0,sizeof(tilemap_buf));
}

void init_colortable(void){
    for(unsigned char g=0;g<3;++g)
        for(unsigned char id=0;id<16;++id)
            vdp_set_tile_color(g,id,COLOR_WHITE,COLOR_BLACK);
}

void init_tilemap(void){
    write_buffer_to_vram();
}

void draw_blockB(unsigned char bx,unsigned char by,unsigned char set)
{
    unsigned char tx = bx >> 1;                  /* tile x 0-31  */
    unsigned char ty = by >> 1;                  /* tile y 0-23  */
    unsigned char q  = ((by & 1) << 1) | (bx & 1); /* 0..3       */

    unsigned char *m = &tile_mask[ty][tx];
    if(set) *m |= 1 << q; else *m &= ~(1 << q);
    tilemap_buf[ty][tx] = *m;
}

void draw_lineB(unsigned char bx0,unsigned char by0,
                unsigned char bx1,unsigned char by1,
                unsigned char set)
{
    int dx = bx1 - bx0, dy = by1 - by0;
    int sx = (dx >= 0) ? 1 : -1, sy = (dy >= 0) ? 1 : -1;
    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;

    if(dx >= dy){
        int err = dx >> 1;
        while(bx0 != bx1){
            draw_blockB(bx0,by0,set);
            bx0 += sx; err -= dy;
            if(err < 0){ by0 += sy; err += dx; }
        }
    }else{
        int err = dy >> 1;
        while(by0 != by1){
            draw_blockB(bx0,by0,set);
            by0 += sy; err -= dx;
            if(err < 0){ bx0 += sx; err += dy; }
        }
    }
    draw_blockB(bx1,by1,set);
}

void write_buffer_to_vram(void){
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_write_bytes_otir(&tilemap_buf[0][0],VIEW_W*VIEW_H);
}