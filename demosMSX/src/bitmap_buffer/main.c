#include <msx.h>
#include <stdio.h>       
#include <video/tms99x8.h>
#include "constants.h"
#include "sprites.h"
#include "tiles.h"
#include "../utils/utils_fps.h"
#include "../utils/utils_msx.h"

void main(void){
    unsigned char bx1=16,by1=12,prevBX=bx1,prevBY=by1;
    unsigned char bx2=8,by2=8;
    signed char dir=1;

    const unsigned char SPR_WB=2,SPR_HB=2;
    const unsigned char WORLD_WB=TILEMAP_W*2;
    const unsigned char WORLD_HB=TILEMAP_H*2;

    init_fps();
    vdp_set_screen_mode(2);
    init_tileset();
    init_colortable();
    init_tilemap();

    vdp_set_sprite(0,sprite_0,0);
    vdp_set_sprite(1,sprite_1,1);

    while(1){
        if(wait_fps()) continue;

        unsigned char stick=msx_get_stick(0);
        unsigned char key=msx_get_trigger(0);

        switch(stick){
            case STICK_UP:           if(by1) --by1; break;
            case STICK_UP_RIGHT:     if(by1) --by1; if(bx1<WORLD_WB-SPR_WB) ++bx1; break;
            case STICK_RIGHT:        if(bx1<WORLD_WB-SPR_WB) ++bx1; break;
            case STICK_DOWN_RIGHT:   if(by1<WORLD_HB-SPR_HB) ++by1; if(bx1<WORLD_WB-SPR_WB) ++bx1; break;
            case STICK_DOWN:         if(by1<WORLD_HB-SPR_HB) ++by1; break;
            case STICK_DOWN_LEFT:    if(by1<WORLD_HB-SPR_HB) ++by1; if(bx1) --bx1; break;
            case STICK_LEFT:         if(bx1) --bx1; break;
            case STICK_UP_LEFT:      if(by1) --by1; if(bx1) --bx1; break;
        }

        vdp_update_sprite(0,0,COLOR_CYAN,bx1<<2,by1<<2);

        bx2+=dir;
        if(bx2>37) dir=-1;
        if(bx2<13) dir=1;
        vdp_update_sprite(1,1,COLOR_DARK_YELLOW,bx2<<2,by2<<2);

        if(!key) draw_lineB(prevBX,prevBY,bx1,by1,1);

        prevBX=bx1; prevBY=by1;

        write_buffer_to_vram();
    }
}
