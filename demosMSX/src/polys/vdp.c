#include "vdp.h"
#include "constants.h"

#include <stdint.h>
#include <arch/z80.h>
#include <msx.h>

#define VDP_DATA 0x98
#define VDP_CTRL 0x99

static void vdp_set_write_addr(uint16_t addr) {
    outp(VDP_CTRL, (uint8_t)(addr & 0xFF));
    outp(VDP_CTRL, (uint8_t)((addr >> 8) | 0x40));
}

void vdp_write(uint16_t vram_addr, const uint8_t* src, uint16_t len) {
    vdp_set_write_addr(vram_addr);
    while (len--) outp(VDP_DATA, *src++);
}

void vdp_fill(uint16_t vram_addr, uint8_t value, uint16_t len) {
    vdp_set_write_addr(vram_addr);
    while (len--) outp(VDP_DATA, value);
}

void vdp_init_screen2(void) {
    msx_set_mode(2);
    vdp_fill(VRAM_NAME_TABLE, 0, NAMETABLE_SIZE);
}
