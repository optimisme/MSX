#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen 2 VRAM layout (TMS9918/MSX1)
#define VRAM_PATTERN_TABLE   0x0000
#define VRAM_COLOR_TABLE     0x2000
#define VRAM_NAME_TABLE      0x1800

// Screen geometry
#define SCREEN_W_PX 256
#define SCREEN_H_PX 192
#define TILE_W_PX   8
#define TILE_H_PX   8
#define TILES_X     32
#define TILES_Y     24
#define NAMETABLE_SIZE (TILES_X*TILES_Y)

// Tile indices
#define TILE_EMPTY 0
#define TILE_FULL  1
#define TILE_X     2

// Fixed-point (16.16)
#define FP_SHIFT 16
#define FP_ONE   (1L<<FP_SHIFT)
#define FP_FROM_INT(x) ((long)(x) << FP_SHIFT)
#define FP_MUL(a,b) ((long)(((long long)(a)*(long long)(b)) >> FP_SHIFT))
#define FP_DIV(a,b) ((long)(((long long)(a) << FP_SHIFT) / (b)))

// Tuning
#define CAM_STEP_FP   (FP_ONE/4)   // 0.25 units per step
#define CAM_Z_MIN_FP  (FP_FROM_INT(1))   // avoid z<=0
#define CAM_Z_MAX_FP  (FP_FROM_INT(30))

#endif
