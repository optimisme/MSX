#ifndef MODE3_ASM_H
#define MODE3_ASM_H

#include <msx.h>
#include <string.h>
#include <video/tms99x8.h> 

extern void draw_pixel_to_buffer_asm(uint8_t x, uint8_t y, uint8_t col) __z88dk_callee;

#endif