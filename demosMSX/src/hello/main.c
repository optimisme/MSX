#include <stdio.h>
#include <stdlib.h>

void clear_screen() {
    putchar(12); // ASCII 12 = CLS al MSX
}

void draw_menu() {
    printf("MSX HELLO TXT PROGRAM\r\n");
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
