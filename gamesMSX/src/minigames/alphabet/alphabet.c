#include "alphabet.h"

void load_alphabet_tileset(void) {
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        uint16_t vram_addr = MODE_2_VRAM_PATTERN_BASE + (bank_base + ALPHABET_BASE) * 8;
        // Copy this bank’s tiles into global buffer
        for (uint16_t i = 0; i < ALPHABET_BITMAP_TILE_COUNT; ++i) {
            uint16_t idx = bank_base + ALPHABET_BASE + i;
            memcpy(&vdp_global_buff[idx * 8],
                   &alphabet_bitmap_tileset[i][0],
                   8);
        }
        // Write this bank’s tiles to VRAM
        vdp_set_address(vram_addr);
        vdp_write_bytes(&vdp_global_buff[(bank_base + ALPHABET_BASE) * 8],
                        ALPHABET_BITMAP_TILE_COUNT * 8);
    }
}

void load_alphabet_colors(void) {
    for (uint8_t bank = 0; bank < 3; ++bank) {
        uint16_t bank_base = bank * 256;
        uint16_t vram_addr = MODE_2_VRAM_COLOR_BASE + (bank_base + ALPHABET_BASE) * 8;
        // Prepare this bank’s colors in global buffer
        for (uint16_t i = 0; i < ALPHABET_BITMAP_TILE_COUNT; ++i) {
            uint16_t idx = bank_base + ALPHABET_BASE + i;
            for (uint8_t y = 0; y < 8; ++y) {
                vdp_global_buff[idx * 8 + y] = (COLOR_WHITE << 4) | COLOR_BLACK;
            }
        }
        // Write this bank’s colors to VRAM
        vdp_set_address(vram_addr);
        vdp_write_bytes(&vdp_global_buff[(bank_base + ALPHABET_BASE) * 8],
                        ALPHABET_BITMAP_TILE_COUNT * 8);
    }
}



void write_text_to_vram(const char *text, uint16_t pos) {
    // build a small buffer of tile-indices
    uint8_t buf[32];
    uint16_t len = 0;
    for (uint16_t i = 0; text[i] && i < sizeof(buf); ++i) {
        uint8_t idx = CHAR_TO_INDEX(text[i]);
        if (idx == INVALID_CHAR) continue;
        buf[len++] = ALPHABET_BASE + idx;
    }
    // point VDP to tilemap position
    vdp_set_address(MODE_2_TILEMAP_BASE + pos);
    // write them straight into VRAM
    vdp_write_bytes(buf, len);
}

void write_text_to_tilemap_buff(const char *text, uint16_t pos) {
    for (uint16_t i = 0; text[i]; ++i) {
        uint8_t idx = CHAR_TO_INDEX(text[i]);
        if (idx == INVALID_CHAR) continue;
        vdp_tilemap_buff[pos + i] = ALPHABET_BASE + idx;
    }
}