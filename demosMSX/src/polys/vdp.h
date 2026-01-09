#ifndef VDP_H
#define VDP_H

#include <stdint.h>

void vdp_init_screen2(void);
void vdp_write(uint16_t vram_addr, const uint8_t* src, uint16_t len);
void vdp_fill(uint16_t vram_addr, uint8_t value, uint16_t len);

#endif
