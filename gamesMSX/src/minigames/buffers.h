#ifndef BUFFERS_H
#define BUFFERS_H

#include <conio.h>
#include <string.h>

#define VIEW_W              32
#define VIEW_H              24
#define VDP_GLOBAL_SIZE     (VIEW_W * VIEW_H * 8)
#define VDP_TILEMAP_SIZE    (VIEW_W * VIEW_H)
#define VARS_SIZE           512

extern uint8_t vdp_global_buff[VDP_GLOBAL_SIZE];
extern uint8_t vdp_tilemap_buff[VDP_TILEMAP_SIZE];
extern uint8_t vars_buff[VARS_SIZE];

void clean_buffers(void);

#endif
