// random.c
#include "utils_random.h"

RANDOM_t random_buff[1] = {
    { .lfsr = 0xACE1u, .last_frame = 0 }
};

static uint16_t xorshift16(uint16_t x) {
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    return x ? x : 0xACE1u;
}

void random_set_seed(void) {

    uint32_t frame = (uint16_t) fps_frame_count;

    if (frame != random_last_frame) {
        random_last_frame = frame;
        random_lfsr ^= xorshift16(fps_frames_to_wait);
        random_lfsr ^= xorshift16(fps_current_jiffy);
        random_lfsr ^= xorshift16(fps_previous_jiffy << 1);
        random_lfsr ^= xorshift16((uint16_t) fps_frame_count & 0xFF);
    }

    for (int i = 0; i < 3; ++i) {
        uint16_t bit = random_lfsr & 1;
        random_lfsr >>= 1;
        if (bit) random_lfsr ^= 0xB400u;
    }

    uint8_t x = (uint8_t)(random_lfsr & 0xFF);
    uint16_t y = (uint16_t)x * x;   // x^2
    uint16_t a =  (fps_current_jiffy % 3) + 2;
    uint16_t b =  (fps_frame_count % 2) + 1;
    y = (y * a + x * b + 1) & 0xFF; // a·x² + b·x + 1 mod 256

    srand(y);
}

uint8_t random_8(void) {
    random_set_seed();
    return (uint8_t)(rand() & 0xFF);
}

uint16_t random_16(void) {
    uint16_t hi = random_8();
    uint16_t lo = random_8();
    return (hi << 8) | lo;
}
