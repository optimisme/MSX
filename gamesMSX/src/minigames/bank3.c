#include "banks.h"
#include <conio.h>

#pragma bank 3

void bank3_content() __banked
{
    vdp_set_screen_mode(1);
    clrscr();
    cputs("Bank 3 contents\r\n");
    cputs("\nPress a key to continue (b3)");
    cgetc();
}