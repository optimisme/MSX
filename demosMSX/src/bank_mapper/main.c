#include <conio.h>
#include "banks.h"

static void draw_menu(void) {
    clrscr();
    cputs("=== MSX Bank Switch Demo ===\r\n");
    cputs("1: Bank 1 (add)\r\n2: Bank 2\r\nChoose an option: ");
}

void main(void) {
    unsigned int a = 1;
    unsigned int b = 4;
    unsigned int result;

    for (;;) {
        draw_menu();
        switch (cgetc()) {
            case '1':
                bank1_content(a, b, &result);  
                cprintf("\n\nMain:\n    Received: %u\n    Result: %u + %u = %u", result, a, b, result);
                cprintf("\nPress a key to continue (m1)");
                cgetc();
                break;
            case '2':
                bank2_content();
                break;
        }
    }
}
