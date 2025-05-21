#ifndef FPS_H
#define FPS_H

#define TARGET_FPS 50

#define JIFFY ((volatile unsigned int*)0xFC9E)
#define SYSFLAGS ((volatile unsigned char*)0x002B)

extern unsigned char frames_to_wait;
extern unsigned int current_jiffy;
extern unsigned int prev_jiffy;
extern unsigned char is_pal;

void init_fps(void);
unsigned char wait_fps(void);

#endif
