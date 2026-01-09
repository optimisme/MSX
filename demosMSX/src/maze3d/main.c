#include <msx.h>
#include <video/tms99x8.h>

#include "tiles.h"
#include "../utils/utils_fps.h"
#include "../utils/utils_msx.h"

#define FP_SHIFT 8
#define FP_ONE (1 << FP_SHIFT)
#define MOVE_STEP (FP_ONE / 12)
#define ROT_STEP 2

static void try_move(uint16_t* x_fp, uint16_t* y_fp, int16_t dx, int16_t dy)
{
    uint16_t nx = (uint16_t)(*x_fp + dx);
    uint16_t ny = (uint16_t)(*y_fp + dy);

    uint16_t cell_x = (uint16_t)(nx >> FP_SHIFT);
    uint16_t cell_y = (uint16_t)(ny >> FP_SHIFT);

    if (cell_x == 0 || cell_y == 0 || cell_x >= MAP_W - 1 || cell_y >= MAP_H - 1) {
        return;
    }

    *x_fp = nx;
    *y_fp = ny;
}

void main(void)
{
    uint16_t pos_x = (uint16_t)(3 * FP_ONE);
    uint16_t pos_y = (uint16_t)(3 * FP_ONE);
    uint8_t angle = 0;

    init_fps();
    vdp_set_screen_mode(2);
    msx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);

    init_raycast();

    while (1) {
        if (wait_fps()) continue;

        uint8_t stick = msx_get_stick(0);
        int16_t dir_x = 0;
        int16_t dir_y = 0;

        if (stick == STICK_LEFT) {
            angle = (uint8_t)(angle - ROT_STEP);
        } else if (stick == STICK_RIGHT) {
            angle = (uint8_t)(angle + ROT_STEP);
        } else if (stick == STICK_UP || stick == STICK_DOWN) {
            get_dir(angle, &dir_x, &dir_y);
            int16_t step_x = (int16_t)((MOVE_STEP * dir_x) / 128);
            int16_t step_y = (int16_t)((MOVE_STEP * dir_y) / 128);
            if (stick == STICK_DOWN) {
                step_x = (int16_t)(-step_x);
                step_y = (int16_t)(-step_y);
            }
            try_move(&pos_x, &pos_y, step_x, step_y);
        }

        render_frame(pos_x, pos_y, angle);
    }
}
