#include <stdio.h>
#include <stdlib.h>
#include "../utils/utils_msx.h"

#define JIFFY    (*(volatile uint16_t*) 0xFC9E)
#define VRAM_DATA   (*(volatile uint8_t*)0x98)
#define VDP_STATUS  (*(volatile uint8_t*)0x99)


void clear_screen() {
    putchar(12); // ASCII 12 = CLS al MSX
}


void draw_menu() {
    uint16_t j = JIFFY;

    printf("MSX HELLO TXT PROGRAM (%u)\r\n", (unsigned)j);
    printf("---------------------\r\n\r\n");
    printf("1. Print 'Hello'\r\n");
    printf("2. Print 'MSX program'\r\n");
    printf("0. Exit program\r\n\r\n");
    printf("Enter your choice (0-2): ");
}

void wait_key() {
    printf("\nPress any key to continue...\n");
    getchar();
}

void main() {
    char input;

    while (1) {
        clear_screen();
        draw_menu();
        input = getchar();
        getchar(); // consumir ENTER

        switch (input) {
            case '1':
                printf("\nHello\n");
                wait_key();
                break;
            case '2':
                printf("\nMSX program\n");
                wait_key();
                break;
            case '0':
                printf("\nExiting program...\n");
                return;
            default:
                printf("\nInvalid choice! Please select 0 to 2.\n");
                wait_key();
        }
    }
}
