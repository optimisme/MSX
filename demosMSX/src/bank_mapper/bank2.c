#include "banks.h"
#include <conio.h>

#pragma bank 2

void bank2_content(void) __banked
{
    clrscr();
    cputs("Bank 2 contents\r\n");
    cputs("\nPress a key to continue (b2)");
    cgetc();
}
