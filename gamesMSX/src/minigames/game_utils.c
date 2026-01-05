#include "game_utils.h"
#include "buffers.h"
#include "../utils/utils_msx.h"
#include <msx.h>
#include <string.h>

void game_transition_black(void) {
    vdp_set_screen_mode(2);
    vdp_set_reg(7, (COLOR_BLACK << 4) | COLOR_BLACK);

    memset(vdp_tilemap_buff, 0x00, VDP_TILEMAP_SIZE);
    vdp_set_address(MODE_2_TILEMAP_BASE);
    vdp_blast_tilemap(vdp_tilemap_buff);

    memset(vdp_global_buff, (COLOR_BLACK << 4) | COLOR_BLACK, VDP_GLOBAL_SIZE);
    vdp_set_address(MODE_2_VRAM_COLOR_BASE);
    vdp_write_bytes(vdp_global_buff, VDP_GLOBAL_SIZE);

    for (uint8_t i = 0; i < 32; ++i) {
        vdp_update_sprite(i, 0, COLOR_BLACK, 0, 208);
    }
}

