#include "fps.h"
#include <msx.h>  

unsigned char frames_to_wait;
unsigned int current_jiffy;
unsigned int prev_jiffy;
unsigned char is_pal;

/**
 * Initialisation of FPS
 */
void init_fps(void) {

    is_pal = (*((unsigned char*)SYSFLAGS)) & 0x80;  // Comprovar bit 7    
    if (is_pal) {
        // PAL - 50 Hz
        frames_to_wait = 50 / TARGET_FPS;
    } else {
        // NTSC - 60 Hz
        frames_to_wait = 60 / TARGET_FPS;
    }

    prev_jiffy = *((unsigned int*)JIFFY);
}

/**
 * Returns if 'wait' is needed
 */
unsigned char wait_fps(void) {
    current_jiffy = *JIFFY;

    if ((current_jiffy - prev_jiffy) < frames_to_wait)
        return 1;

    prev_jiffy = current_jiffy;
    return 0;
}