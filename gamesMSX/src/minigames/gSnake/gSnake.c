#pragma bank 4
#include "gSnake.h"

// Bases de VRAM per a Screen Mode 3
#define PT_BASE 0x0000   // Pattern Generator Table a 0x0000
#define NT_BASE 0x0800   // Name Table a  0x0800
#define NT_W    32
#define NT_H    24

static uint8_t next_pattern = 2;  // Començarem a clonar a partir de pattern 2

static void load_solid_pattern(void) {
    // 1 patró en Mode 3 ocupa 8 bytes (2 bytes/set × 4 sets)
    uint8_t orange_pattern[8];
    for (int i = 0; i < 8; i++) {
        orange_pattern[i] = 0xBB;  // 0xB = color 11 (orange)
    }

    // Hem d'escriure AQUÍ els 8 bytes intercalats per a set 0, 1, 2 i 3:
    //   PT_BASE + (1*8 + 0) … PT_BASE + (1*8 + 7)
    //
    // És a dir, escriurem directament a l'adreça:
    //    PT_BASE + 8  → byte 0 (set 0, fila 0 de pattern 1)
    //    PT_BASE + 9  → byte 1 (set 0, fila 1 de pattern 1)
    //    PT_BASE + 10 → byte 2 (set 1, fila 0 de pattern 1)
    //    PT_BASE + 11 → byte 3 (set 1, fila 1 de pattern 1)
    //    PT_BASE + 12 → byte 4 (set 2, fila 0 de pattern 1)
    //    PT_BASE + 13 → byte 5 (set 2, fila 1 de pattern 1)
    //    PT_BASE + 14 → byte 6 (set 3, fila 0 de pattern 1)
    //    PT_BASE + 15 → byte 7 (set 3, fila 1 de pattern 1)

    vdp_set_address(PT_BASE + 8);
    vdp_write_bytes(orange_pattern, 8);
}

static void fill_screen_orange(void) {
    // A la Name Table (NT_BASE) hi posem l'índex “1” en cada patró
    // La Name Table fa 32×24 = 768 entrades, cadascuna 1 byte
    vdp_set_address(NT_BASE);
    for (uint16_t i = 0; i < NT_W * NT_H; i++) {
        vdp_write_byte(1);
    }
}

static void draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    // En Mode 3 la resolució lògica “de pixels” és 64×48.
    if (x >= 64 || y >= 48) return;

    // Quina “cel·la” de 2×2 conté (x,y)?
    uint8_t tile_x = x >> 1;  // x/2
    uint8_t tile_y = y >> 1;  // y/2
    uint16_t tile_index = tile_y * NT_W + tile_x;

    // 1) Mireu quin patró hi ha ara (abans d’escriure res)
    vdp_set_address(NT_BASE + tile_index);
    uint8_t old_pat = vdp_read_byte();

    // 2) Llegim els 8 bytes del “old_pat” (a la Pattern Table)
    uint16_t old_addr = PT_BASE + (old_pat * 8);
    uint8_t buffer[8];
    for (int i = 0; i < 8; i++) {
        vdp_set_address(old_addr + i);
        buffer[i] = vdp_read_byte();
    }

    // 3) Calculem exactament quin byte i quin nibble tocar.
    //    - Cada patró té 2 files (car cada patró és 2×2). Per tant:
    //       - fila 0 = byte index 0,2,4,6  (sets 0..3)
    //       - fila 1 = byte index 1,3,5,7
    //    - Dins d’aquella fila, el “x” té 2 píxels → left nibble (si x&1==0) o right nibble (si x&1==1).
    //
    //    Concretament:
    //      uint8_t pixel_x = x & 1;  // 0=left, 1=right
    //      uint8_t pixel_y = y & 1;  // 0=primera fila del patró, 1=segona fila
    //
    uint8_t pixel_x = x & 1;
    uint8_t pixel_y = y & 1;
    // byte_offset dins dels 8 èbytes: per al set 0 el tenim a buffer[ pixel_y * 2 + 0 ],
    // per al set 1 a buffer[ pixel_y * 2 + 2 ],  per set 2 a buffer[pixel_y*2 + 4], set 3 a buffer[pixel_y*2 + 6].
    //
    // Però nosaltres només modifiquem el set corresponent a la posició (tile_x, tile_y) en pantalla:
    //   en Mode 3, cada “patró” s’utilitza en un dels 4 sets en funció de si (tile_x,tile_y) és
    //   parell/imparell en horitzontal i vertical. Concretament:
    //     set_index = (tile_y & 1) << 1 | (tile_x & 1)
    //
    //   - Si tile_x parell i tile_y parell → set_index = 0
    //   - Si tile_x senar  i tile_y parell → set_index = 1
    //   - Si tile_x parell i tile_y senar  → set_index = 2
    //   - Si tile_x senar  i tile_y senar  → set_index = 3
    uint8_t set_index = ((tile_y & 1) << 1) | (tile_x & 1);
    // Dins del bloc de 8 bytes, els bytes estan organitzats així per a cada pattern:
    //    index:   0   1   2   3   4   5   6   7
    //  content:  F0  F1  S0  S1  T0  T1  Q0  Q1   (F=set0, S=set1, T=set2, Q=set3; fila0=f0,s0,t0,q0; fila1=f1,s1,t1,q1)
    //
    // Si set_index=0, el byte que toca modificar és “fila”=pixel_y, “cols”=set0 → offset = pixel_y*2 + 0
    // Si set_index=1, offset = pixel_y*2 + 2
    // Si set_index=2, offset = pixel_y*2 + 4
    // Si set_index=3, offset = pixel_y*2 + 6
    uint8_t byte_offset = (pixel_y * 2) + (set_index * 2);

    // Ara modifiquem ​només​ el nibble corresponent dins d’aquest buffer[byte_offset]
    uint8_t old_byte = buffer[byte_offset];
    if (pixel_x == 0) {
        // left nibble
        old_byte = (old_byte & 0x0F) | (color << 4);
    } else {
        // right nibble
        old_byte = (old_byte & 0xF0) | (color & 0x0F);
    }
    buffer[byte_offset] = old_byte;

    // 4) Ara escrivim aquests 8 bytes “buffer” a un “new pattern” lliure (per exemple next_pattern=2)
    uint8_t new_pat = next_pattern++;
    uint16_t new_addr = PT_BASE + (new_pat * 8);
    vdp_set_address(new_addr);
    vdp_write_bytes(buffer, 8);

    // 5) Finalment, a la Name Table fem que la casella (tile_index) apunti ara a new_pat
    vdp_set_address(NT_BASE + tile_index);
    vdp_write_byte(new_pat);
}

void vdp_set_screen_modex(uint8_t value) {

    msx_screen(value);
    if (value == 3) {
        vdp_set_reg(2, 0x02);   // Name Table = 0x0800
        vdp_set_reg(4, 0x00);   // Pattern Table = 0x0000
    }
}

void main_gSnake(void) __banked {
    init_fps();
    vdp_set_screen_modex(3);

    load_solid_pattern();    // Escriu els 8 bytes de 0xBB en els 4 sets del pattern 1
    fill_screen_orange();    // Omple tota la NT amb l’índex “1”

    // Ara hauríem de veure tota la pantalla taronja. Per comprovar el draw_pixel:
    draw_pixel(10, 10, 15); // blanc a (10,10)
    draw_pixel(20, 20, 4);  // blau a (20,20)
    draw_pixel(30, 30, 7);  // groc a (30,30)

    // Mirar un “bloc” 2×2 diferents a (0,0),(0,1),(1,0),(1,1):
    draw_pixel(0, 0, COLOR_LIGHT_YELLOW);
    draw_pixel(0, 1, COLOR_DARK_RED);
    draw_pixel(1, 0, COLOR_DARK_GREEN);
    draw_pixel(1, 1, COLOR_DARK_BLUE);

    for (;;) {
        if (wait_fps()) {
            // loop… si vols anar dibuixant més tard
        }
    }
}
