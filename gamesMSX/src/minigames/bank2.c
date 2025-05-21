#pragma bank 2

#include "banks.h"
#include <conio.h>



void bank2_content() __banked
{
    vdp_set_screen_mode(1);
    clrscr();
    cputs("Bank 2 contents\r\n");
    cputs("\nPress a key to continue (b2)");
    cgetc();
}
