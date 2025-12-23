#ifndef FPS_H
#define FPS_H

#include <stdint.h>    

#define TARGET_FPS 50

#define JIFFY    (*(volatile uint16_t*) 0xFC9E)
#define SYSFLAGS (*(volatile uint8_t* ) 0x002B)

typedef struct {
    uint8_t  frames_to_wait;
    uint16_t current_jiffy;
    uint16_t previous_jiffy;
    uint8_t  is_pal;
    uint32_t frame_count;
} FPS_t;

extern FPS_t fps_buff[1];

#define fps_frames_to_wait  (fps_buff[0].frames_to_wait)
#define fps_current_jiffy   (fps_buff[0].current_jiffy)
#define fps_previous_jiffy  (fps_buff[0].previous_jiffy)
#define fps_is_pal          (fps_buff[0].is_pal)
#define fps_frame_count     (fps_buff[0].frame_count)

void init_fps(void);
uint8_t wait_fps(void);
void wait_vblank(void);

#endif
