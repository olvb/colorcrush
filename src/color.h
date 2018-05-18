#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum ccrush_color_channel_t {
    CCRUSH_R = 0,
    CCRUSH_G = 1,
    CCRUSH_B = 2
} ccrush_color_channel_t;

uint32_t ccrush_color_diff_uint8(uint8_t *lhs, uint8_t *rhs);
uint64_t ccrush_color_diff_uint32(uint32_t *lhs, uint32_t *rhs);
