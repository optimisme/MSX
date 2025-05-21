#include "buffers.h"

uint8_t vdp_global_buff[VDP_GLOBAL_SIZE];
uint8_t vdp_tilemap_buff[VDP_TILEMAP_SIZE];
uint8_t vars_buff[VARS_SIZE];  

void clean_buffers(void) {
    memset(vdp_global_buff, 0, VDP_GLOBAL_SIZE);
    memset(vdp_tilemap_buff, 0, VDP_TILEMAP_SIZE);
    memset(vars_buff, 0, VARS_SIZE);
}