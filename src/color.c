#include "color.h"
#include "stdint.h"

/**
theses two functions are somewhat problematics because they do not take into account the human eye's sensibility
to different colors, but even more because they will evaluate the difference between (100, 0, 0) and (101, 0, 0)
to be greater than the one between (100, 0, 0) and (50, 50, 0). In pratices it seems to seldom cause any artifacts though.
EDIT: the addition of YUV coefficients provides a better approximation without impacting performance
*/
uint32_t ccrush_color_diff_uint8(uint8_t *lhs, uint8_t *rhs) {
    int32_t delta_r = (int32_t) lhs[CCRUSH_R] - rhs[CCRUSH_R];
    int32_t delta_g = (int32_t) lhs[CCRUSH_G] - rhs[CCRUSH_G];
    int32_t delta_b = (int32_t) lhs[CCRUSH_B] - rhs[CCRUSH_B];
    return (uint32_t)(3 * delta_r * delta_r) + (uint32_t)(6 * delta_g * delta_g) + (uint32_t)(delta_b * delta_b);
}

uint64_t ccrush_color_diff_uint32(uint32_t *lhs, uint32_t *rhs) {
    int64_t delta_r = (int64_t) lhs[CCRUSH_R] - rhs[CCRUSH_R];
    int64_t delta_g = (int64_t) lhs[CCRUSH_G] - rhs[CCRUSH_G];
    int64_t delta_b = (int64_t) lhs[CCRUSH_B] - rhs[CCRUSH_B];
    return (uint64_t)(3 * delta_r * delta_r) + (uint64_t)(6 * delta_g * delta_g) + (uint64_t)(delta_b * delta_b);
}
