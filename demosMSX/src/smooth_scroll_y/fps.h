#ifndef FPS_H
#define FPS_H 

#define TARGET_FPS 30 // Max 50 for PAL and 60 for NTSC

#define JIFFY    ((unsigned int*)0xFC9E)  // Frame counter
#define SYSFLAGS ((unsigned char*)0xFFE8) // Flags (bit 7 = 0 for NTSC, 1 for PAL)

/**
 * Initialisation of FPS
 */
void init_fps(void);

/**
 * Returns if 'wait' is needed
 */
unsigned char wait_fps(void);

#endif