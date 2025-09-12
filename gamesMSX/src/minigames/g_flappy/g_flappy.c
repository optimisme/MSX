#pragma bank 3

// Flappy mínim (MSX1 SCREEN 2, sprites 8x8)
#include <conio.h>                // kbhit(), getch()
#include <video/tms99x8.h>        // vdp_vpoke, taules VDP
#include "../../utils/utils_msx.h" // msx_set_mode(2)
#include "g_flappy.h"

// Taules SCREEN 2 (TMS9918A)
#define NAME_TABLE_BASE  0x1800   // Tilemap (32x24 = 768 bytes)
#define SPR_PAT_BASE     0x3800   // Sprite Pattern Table
#define SPR_ATTR_BASE    0x1B00   // Sprite Attribute Table

// Slots i colors
#define SP_BIRD          0
#define COL_YELLOW       14       // groc (MSX1)

// Patró 8x8 (ocell)
static const unsigned char BIRD8[8] = {
    0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x7E, 0x3C, 0x18
};

// Helpers
static void clear_name_table(void) {
    unsigned int addr = NAME_TABLE_BASE;
    for (unsigned int i = 0; i < 32u * 24u; ++i) vdp_vpoke(addr + i, 0x00);
}
static void sprite_upload_pattern(unsigned char patIndex, const unsigned char *pat8) {
    unsigned int base = SPR_PAT_BASE + ((unsigned int)patIndex << 3);
    for (unsigned char i = 0; i < 8; ++i) vdp_vpoke(base + i, pat8[i]);
}
static void sprite_set(unsigned char slot, unsigned char x, unsigned char y,
                       unsigned char patIndex, unsigned char color) {
    unsigned int a = SPR_ATTR_BASE + ((unsigned int)slot << 2);
    vdp_vpoke(a + 0, y);
    vdp_vpoke(a + 1, x);
    vdp_vpoke(a + 2, patIndex);
    vdp_vpoke(a + 3, color);
}
static void sprite_endlist(unsigned char nextSlot) {
    unsigned int a = SPR_ATTR_BASE + ((unsigned int)nextSlot << 2);
    vdp_vpoke(a + 0, 0xD0);   // Y=0xD0 → end-of-list
}

void main_g_flappy(void) {
    // MODE
    msx_set_mode(2);          // SCREEN 2

    // Neteja bàsica i estat inicial sprites
    clear_name_table();
    sprite_endlist(1);        // llista d'sprites acaba després del slot 0

    // Carrega el patró de l’ocell al patró 0
    sprite_upload_pattern(0, BIRD8);

    // Estat del joc
    int x = 48;
    int y = 80;
    int vy = 0;
    const int GRAV = 1;
    const int FLAP = -4;

    // Bucle principal
    for (;;) {
        // Input: ESPAI salta, ESC surt
        if (kbhit()) {
            unsigned char c = getch();
            if (c == ' ')       vy = FLAP;
            else if (c == 27)   break;    // ESC
        }

        // Física senzilla
        vy += GRAV; if (vy > 4) vy = 4;
        y += vy;
        if (y < 16)  { y = 16;  vy = 0; }
        if (y > 176) { y = 176; vy = 0; }

        // Dibuix
        sprite_set(SP_BIRD, (unsigned char)x, (unsigned char)y, 0, COL_YELLOW);

        // Delay curt (estabilitzar FPS)
        for (volatile unsigned int d = 0; d < 2000; ++d) { }
    }

    // Sortir: amaga l’sprite
    sprite_set(SP_BIRD, 0, 0xD0, 0, 0);
}
