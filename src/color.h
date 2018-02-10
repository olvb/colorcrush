#pragma once
#include <stdint.h>

#define COLOR_CHANNELS_COUNT 3
#define COLOR_R 0
#define COLOR_G 1
#define COLOR_B 2

#define MAX_COLOR_DIFF 4294967295 // UINT_MAX

/** @returns an evaluation of the difference between @lhs and @rhs */
unsigned int color_diff(uint8_t *lhs, uint8_t *rhs);
