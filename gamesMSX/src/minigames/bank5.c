#include "banks.h"
#include <conio.h>

#pragma bank 5

void bank5_content() __banked
{
    vdp_set_screen_mode(1);
    clrscr();
    cputs("Bank 5 contents\r\n");
    cputs("\nPress a key to continue (b5)");
    cgetc();
}
