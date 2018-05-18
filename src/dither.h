#pragma once
#include "color.h"
#include <stdint.h>

/** Applies error diffusion to a picture by using a 2x2 Sierra Lite matrix */
typedef struct ccrush_dither_t {
    /** Width of the image and of rows of @p errors */
    uint32_t width;
    /** Index of the row in @p errors representing the current line of the image */
    unsigned int current_row;
    uint32_t current_row_begin;
    /** Accumulated error for pixels channels */
    int *errors;
} ccrush_dither_t;

void ccrush_dither_init(ccrush_dither_t *dither, uint32_t width);
void ccrush_dither_clear(ccrush_dither_t *dither);
/**
Computes and stores error for pixel located at @p pixel_index
@p src_color: pixel color in source image
@p rounded_color: actual color that was assigned to pixel after quantization and dithering
*/
void ccrush_dither_diffuse_error(ccrush_dither_t *dither, uint32_t pixel_index, uint8_t *src_color, uint8_t *rounded_color);
/**
Retrieves color value with accumulated error for pixel located at @p pixel_index
@p color: pixel color value
@p corrected_color: @p color with accumulated error added
*/
void ccrush_dither_apply_error(ccrush_dither_t *dither, uint32_t pixel_index, uint8_t *color, uint8_t *corrected_color);
