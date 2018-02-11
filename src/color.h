#pragma once
#include <stdint.h>
#include <stdbool.h>

#define COLOR_R 0
#define COLOR_G 1
#define COLOR_B 2

uint32_t color_diff_uint8(uint8_t *lhs, uint8_t *rhs);
uint64_t color_diff_uint32(uint32_t *lhs, uint32_t *rhs);
