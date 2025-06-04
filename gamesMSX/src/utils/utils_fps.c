#include "utils_fps.h"

FPS_t fps_buff[1];

void init_fps(void) {
    fps_is_pal = SYSFLAGS & 0x80;  // bit 7 = 1 → PAL
    fps_frames_to_wait = fps_is_pal ? (50 / TARGET_FPS) : (60 / TARGET_FPS);
    fps_previous_jiffy = JIFFY;
    fps_frame_count = 0;
}

uint8_t wait_fps(void) {
    fps_current_jiffy = JIFFY;
    if ((uint16_t)(fps_current_jiffy - fps_previous_jiffy)  < fps_frames_to_wait) {
        return 1;
    }
    fps_previous_jiffy = fps_current_jiffy;
    fps_frame_count++;
    return 0;
}
