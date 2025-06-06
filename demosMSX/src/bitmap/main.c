#include <msx.h>
#include <stdio.h>       
#include <video/tms99x8.h>
#include "constants.h"
#include "sprites.h"
#include "tiles.h"
#include "../utils/utils_fps.h"
#include "../utils/utils_msx.h"

void main(void) {

    int x1      = 100;
    int prevX   = x1;
    int sx1     = x1;
    int y1      = 100;
    int prevY   = y1;
    int sy1     = y1;
    const unsigned char sprite_w   = 8;
    const unsigned char sprite_h   = 8;
    const unsigned int max_world_px = TILEMAP_W * 8;   
    const unsigned int max_world_py = TILEMAP_H * 8;   
    const unsigned int screen_w    = 256; 
    const unsigned int screen_h    = 192; 
    const unsigned int half_screen_w = screen_w / 2;
    const unsigned int half_screen_h = screen_h / 2;
    const unsigned int max_cam_x   = max_world_px - screen_w;

    unsigned int x2      = 50;
    unsigned int y2      = 50;
    signed   char dir     = 1;

    unsigned char stick;
    unsigned char key;

    unsigned int cam_x    = 0;
    unsigned int cam_y    = 0;

    init_fps();
    vdp_set_screen_mode(2);
    init_tileset();
    init_colortable();
    init_tilemap();

    vdp_set_sprite(0, sprite_0, 0);
    vdp_set_sprite(1, sprite_1, 1);

    while (1) {

        if (wait_fps()) continue;

        stick = msx_get_stick(0);
        key = msx_get_trigger(0);

        switch(stick) {
            case STICK_UP:    
                y1 -= SPEED1;
                break;
            case STICK_UP_RIGHT:  
                y1 -= SPEED1;
                x1 += SPEED1;
                break;
            case STICK_RIGHT: 
                x1 += SPEED1;
                break;
            case STICK_DOWN_RIGHT:  
                y1 += SPEED1;
                x1 += SPEED1;
                break;
            case STICK_DOWN:  
                y1 += SPEED1;
                break;
            case STICK_DOWN_LEFT:  
                y1 += SPEED1;
                x1 -= SPEED1;
                break;
            case STICK_LEFT:  
                x1 -= SPEED1;
                break;
            case STICK_UP_LEFT: 
                y1 -= SPEED1;
                x1 -= SPEED1;
                break;
        }

        unsigned int sprite_limit_w = (max_world_px - sprite_w);
        if (x1 < 0)
            x1 = 0;
        if (x1 > sprite_limit_w)
            x1 = sprite_limit_w;

        unsigned int sprite_limit_h = (max_world_py - sprite_h);
        if (y1 < 0)
            y1 = 0;
        if (y1 > sprite_limit_h)
            y1 = sprite_limit_h;

        cam_x = x1;
        cam_y = y1;

        if (x1 < half_screen_w) {
            cam_x = 0;
            sx1   = x1;             
        }
        else if (x1 < max_world_px - half_screen_w) {
            cam_x = x1 - half_screen_w;
            sx1   = half_screen_w;    
        }
        else {
            cam_x = max_world_px - screen_w;
            sx1   = x1 - cam_x;  
        }

        if (y1 < half_screen_h) {
            cam_y = 0;
            sy1   = y1;             
        }
        else if (y1 < max_world_py - half_screen_h) {
            cam_y = y1 - half_screen_h;
            sy1   = half_screen_h;    
        }
        else {
            cam_y = max_world_py - screen_h;
            sy1   = y1 - cam_y; 
        }
        vdp_update_sprite(0, 0, COLOR_CYAN, sx1, sy1);

        x2 += dir * SPEED2;
        if (x2 > 150) dir = -1;
        if (x2 <  50) dir =  1;
        vdp_update_sprite(1, 1, COLOR_DARK_YELLOW, x2, y2);

        if (!key) {
            draw_line(prevX + 4, prevY + 4, x1 + 4, y1 + 4, 1);
        }

        prevX = x1;
        prevY = y1;
    }
}
