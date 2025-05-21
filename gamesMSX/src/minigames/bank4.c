#include "banks.h"
#include <conio.h>

#pragma bank 4

void bank4_content() __banked
{
    vdp_set_screen_mode(1);
    clrscr();
    cputs("Bank 4 contents\r\n");
    cputs("\nPress a key to continue (b4)");
    cgetc();
}
