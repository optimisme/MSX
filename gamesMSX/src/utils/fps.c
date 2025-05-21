#include "fps.h"

unsigned char frames_to_wait;
unsigned int current_jiffy;
unsigned int prev_jiffy;
unsigned char is_pal;

void init_fps(void) {
    is_pal = (*SYSFLAGS) & 0x80;  // bit 7 = 1 → PAL
    frames_to_wait = is_pal ? (50 / TARGET_FPS) : (60 / TARGET_FPS);
    prev_jiffy = *JIFFY;
}

unsigned char wait_fps(void) {
    current_jiffy = *JIFFY;
    if ((unsigned int)(current_jiffy - prev_jiffy) < frames_to_wait)
        return 1;
    prev_jiffy = current_jiffy;
    return 0;
}
