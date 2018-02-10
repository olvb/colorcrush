#pragma once
#include <stdint.h>
#include "color.h"

/** Applies error diffusion to a picture by using a 2x2 Sierra Lite matrix */
typedef struct Dither {
    /** Width of the image and of rows of @errors */
    uint32_t width;
    /** Index of the row in @errors representing the current line of the image */
    unsigned int current_row;
    uint32_t current_row_begin;
    /** Accumulated error for pixels channels */
    int *errors;
} Dither;

void dither_init(Dither *dither, uint32_t width);
void dither_clear(Dither *dither);
/**
Computes and stores error for pixel located at @pixel_index
@src_color: pixel color in source image
@rounded_color: actual color that was assigned to pixel after quantization and dithering
*/
void dither_diffuse_error(Dither *dither, uint32_t pixel_index, uint8_t *src_color, uint8_t *rounded_color);
/**
Retrieves color value with accumulated error for pixel located at @pixel_index
@color: pixel color value
@corrected_color: @color with accumulated error added
*/
void dither_apply_error(Dither *dither, uint32_t pixel_index, uint8_t *color, uint8_t *corrected_color);
