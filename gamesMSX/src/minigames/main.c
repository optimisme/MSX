#include <conio.h>
#include <malloc.h>

#include "banks.h"
#include "buffers.h"
#include "../utils/utils_fps.h"
#include "../utils/utils_random.h"

void flush_keyboard(void) {
    suspend_interrupts();
    for (int x = 0; x < 500; x++){ 
        while (kbhit()) {
            cgetc();
            do_nop();
        }
    }
    resume_interrupts();
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

            main_g2048();
            selected_menu_option = 1;
            selected_bank = 1;

        } else if (selected_bank == 3) {

            bank3_content();
            selected_menu_option = 2;
            selected_bank = 1;

        } else if (selected_bank == 4) {

            main_gSnake();
            selected_menu_option = 3;
            selected_bank = 1;

        } else if (selected_bank == 5) {

            bank5_content();
            selected_menu_option = 4;
            selected_bank = 1;
        }
    }
}