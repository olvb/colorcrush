#include "color.h"

/**
this function is somewhat problematic because it does not take into account the human eye's sensibility
to different colors, but even more because it will evaluate the different between (100, 0, 0) and (101, 0, 0)
to be greater than the one between (100, 0, 0) and (50, 50, 0)
In pratices it seems to seldom cause any artifacts though.
*/
unsigned int color_diff(uint8_t *lhs, uint8_t *rhs) {
    int delta_r = (int) lhs[COLOR_R] - rhs[COLOR_R];
    int delta_g = (int) lhs[COLOR_G] - rhs[COLOR_G];
    int delta_b = (int) lhs[COLOR_B] - rhs[COLOR_B];
    // Alternate calculus, less problematic but slower:
    //int r_mean = ((int) lhs[COLOR_R] + rhs[COLOR_R]) >> 2;
    //return (unsigned int) (((512 + r_mean) * delta_r * delta_r) >> 8) + 4 * delta_g * delta_g + (((767 - r_mean) * delta_b * delta_b) >> 8);
    return (unsigned int) delta_r * delta_r + delta_g * delta_g + delta_b * delta_b;
}
