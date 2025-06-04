// random.h
#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include "utils_fps.h"
#include "utils_msx.h"

typedef struct {
    uint16_t lfsr;
    uint32_t last_frame;
} RANDOM_t;

extern RANDOM_t random_buff[1];

#define random_lfsr (random_buff[0].lfsr)
#define random_last_frame (random_buff[0].last_frame)

void random_set_seed(void);
uint8_t random_8(void);
uint16_t random_16(void);

#endif
