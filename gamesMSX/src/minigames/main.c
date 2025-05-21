#include <conio.h>
#include <malloc.h>

#include "banks.h"
#include "buffers.h"
#include "../utils/fps.h"

void flush_keyboard(void) {
    for (uint8_t x = 0; x < 50; x++){ 
        while (kbhit()) cgetc();
    }
}

void main(void) {

    uint8_t selected_menu_option = 1;
    uint8_t selected_bank = 1;

    while (1) {

        clean_buffers();
        flush_keyboard();

        if (selected_bank == 1) {

            main_menu(selected_menu_option, &selected_bank);

        } else if (selected_bank == 2) {

            bank2_content();
            selected_menu_option = 1;
            selected_bank = 1;

        } else if (selected_bank == 3) {

            bank3_content();
            selected_menu_option = 2;
            selected_bank = 1;

        } else if (selected_bank == 4) {

            bank4_content();
            selected_menu_option = 3;
            selected_bank = 1;

        } else if (selected_bank == 5) {

            bank5_content();
            selected_menu_option = 4;
            selected_bank = 1;
        }
    }
}