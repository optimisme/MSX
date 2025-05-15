#include <msx.h>
#include <stdio.h>  
#include <video/tms99x8.h>

#include "constants.h"
#include "fps.h"
#include "sprites.h"
#include "tiles.h"
#include "../utils/utils_msx.h"

void main(void) {

    int x1 = 100;  
    int y1 = 100; 
    int x2 = 50;
    int y2 = 50;
    signed char dir = 1;
    unsigned char stick;
    unsigned char key;
    unsigned char i;

    init_fps();
    init_screen(); 
    init_tileset();
    init_colortable();
    init_tilemap();

    init_sprite(0, sprite_0, 0);
    init_sprite(1, sprite_1, 1);

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
        
        if (x1 < MIN_X) x1 = MIN_X;
        if (x1 > MAX_X) x1 = MAX_X;
        if (y1 < MIN_Y) y1 = MIN_Y;
        if (y1 > MAX_Y) y1 = MAX_Y;
        update_sprite(0, 0, COLOR_CYAN, x1, y1);

        x2 += (dir * SPEED2);
        if (x2 > 150) dir = -1;
        if (x2 < 50) dir = 1;
        update_sprite(1, 1, COLOR_DARK_YELLOW, x2, y2);
    }
}