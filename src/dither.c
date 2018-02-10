#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "dither.h"

#define DITHER_HEIGHT 2
#define DITHER_COEF_RIGHT 2
#define DITHER_COEF_BOTTOM 1
#define DITHER_COEF_BOTTOM_LEFT 1
#define DITHER_COEF_BOTTOM_RIGHT 0
#define DITHER_COEF_DIVIDER 4

/** Accumulated error values for each channel of a pixel */
// TODO switch to table?
struct QuantError {
    int r;
    int g;
    int b;
};

void dither_init(Dither *dither, uint32_t width) {
    dither->width = width;
    dither->current_row = 0;
    dither->current_row_begin = 0;
    // calloc: error values initialised at 0
    dither->errors = calloc(width * DITHER_HEIGHT, sizeof(QuantError));
}

void dither_clear(Dither *dither) {
    free(dither->errors);
    dither->errors = NULL;
}

void dither_next_row(Dither *dither) {
    // wipe current row error values
    memset(dither->errors + dither->current_row_begin, 0, sizeof(QuantError) * dither->width);
    
    // shift current row
    dither->current_row = (dither->current_row + 1) % DITHER_HEIGHT;
    dither->current_row_begin = dither->current_row * dither->width;
}

void dither_diffuse_error(
    Dither *dither, uint32_t pixel_index,
    uint8_t *src_color, uint8_t *rounded_color
) {
    // compute error
    QuantError error;
    error.r = (int) src_color[COLOR_R] - rounded_color[COLOR_R];
    error.g = (int) src_color[COLOR_G] - rounded_color[COLOR_G];
    error.b = (int) src_color[COLOR_B] - rounded_color[COLOR_B];
    
    uint32_t col = pixel_index % dither->width;
    uint32_t next_row_begin = (dither->current_row + 1) % DITHER_HEIGHT * dither->width;
    uint32_t error_index;
    
    #if DITHER_COEF_BOTTOM
        error_index = dither->current_row_begin + col;
        dither->errors[error_index].r += error.r * DITHER_COEF_BOTTOM;
        dither->errors[error_index].g += error.g * DITHER_COEF_BOTTOM;
        dither->errors[error_index].b += error.b * DITHER_COEF_BOTTOM;
    #endif
    
    #if DITHER_COEF_RIGHT
        if (col != dither->width - 1) {
            error_index = dither->current_row_begin + col + 1;
            dither->errors[error_index].r += error.r * DITHER_COEF_RIGHT;
            dither->errors[error_index].g += error.g * DITHER_COEF_RIGHT;
            dither->errors[error_index].b += error.b * DITHER_COEF_RIGHT;
        }
    #endif

    #if DITHER_COEF_BOTTOM_RIGHT
        if (col != dither->width - 1) {
            error_index = next_row_begin + col + 1;
            dither->errors[error_index].r += error.r * DITHER_COEF_BOTTOM_RIGHT;
            dither->errors[error_index].g += error.g * DITHER_COEF_BOTTOM_RIGHT;
            dither->errors[error_index].b += error.b * DITHER_COEF_BOTTOM_RIGHT;
        }
    #endif

    #if DITHER_COEF_BOTTOM_LEFT
        if (col != 0) {
            error_index = next_row_begin + col - 1;        
            dither->errors[error_index].r += error.r * DITHER_COEF_BOTTOM_LEFT;
            dither->errors[error_index].g += error.g * DITHER_COEF_BOTTOM_LEFT;
            dither->errors[error_index].b += error.b * DITHER_COEF_BOTTOM_LEFT;
        }
    #endif
}

#define CLAMP(x, ceil) ((x) > (ceil) ? (ceil) : ((x) < 0 ? 0 : (x)))

void dither_apply_error(Dither *dither, uint32_t pixel_index, uint8_t *color) {
    uint32_t col = pixel_index % dither->width;
    uint32_t error_index = dither->current_row_begin + col;
    QuantError error = dither->errors[error_index];

    int r = error.r / DITHER_COEF_DIVIDER + color[COLOR_R];
    color[COLOR_R] = CLAMP(r, 255);
    int g = error.g / DITHER_COEF_DIVIDER + color[COLOR_G];
    color[COLOR_G] = CLAMP(g, 255);
    int b = error.b / DITHER_COEF_DIVIDER + color[COLOR_B];
    color[COLOR_B] = CLAMP(b, 255);
}
