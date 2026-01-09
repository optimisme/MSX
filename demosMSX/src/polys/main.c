#include <msx.h>
#include <video/tms99x8.h>

#include "tiles.h"
#include "../utils/utils_fps.h"
#include "../utils/utils_msx.h"

void main(void)
{
    init_fps();
    vdp_set_screen_mode(2);
    msx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);

    init_anim_demo();

    while (1) {
        if (wait_fps()) continue;
        render_anim_frame();
    }
}
