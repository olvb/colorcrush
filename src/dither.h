#pragma once
#include <stdint.h>
#include "color.h"

typedef struct QuantError QuantError;

/** Applies error diffusion to a picture by using a Sierra Lite matrix (2x2, forward propagation only) */
typedef struct Dither {
    /** Width of the image and of rows of @errors */
    uint32_t width;
    /** Index of the row in @errors representing the current line of the image */
    unsigned int current_row;
    uint32_t current_row_begin;
    /** Accumulated error for each pixel */
    QuantError *errors;
} Dither;

void dither_init(Dither *dither, uint32_t width);
void dither_clear(Dither *dither);
/** Notifies @dither that we are processing a new line */
void dither_next_row(Dither *dither);
/**
Computes and stores error for pixel located at @pixel_index
@src_color : pixel color in source image
@rounded_color : actual color that was assigned to pixel after quantization and dithering
*/
void dither_diffuse_error(Dither *dither, uint32_t pixel_index, uint8_t *src_color, uint8_t *rounded_color);
/** Applies to @color accumulated error for pixel located at @pixel_index */
void dither_apply_error(Dither *dither, uint32_t pixel_index, uint8_t *color);
