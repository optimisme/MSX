#include "banks.h"
#include <conio.h>

/* Tot el que hi hagi després d’aquest pragma anirà al banc 1 */
#pragma bank 1

/* La funció és bancada: el linker crearà el pont de crida */
void bank1_content(unsigned char a, unsigned char b, unsigned int *out) __banked
{
    unsigned int c = a + b;
    *out = c;

    clrscr();
    cprintf("\nBank 1:\n    Received: %u and %u\n    Add: %u + %u = %u", a, b, a, b, c);
    cputs("\nPress a key to continue (b1)");
    cgetc(); 
}
