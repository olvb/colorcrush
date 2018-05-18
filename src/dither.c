#include "dither.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DITHER_HEIGHT 2
/*
// sierra lite matrix
#define DITHER_COEF_RIGHT 2
#define DITHER_COEF_DOWN 1
#define DITHER_COEF_DOWN_LEFT 1
#define DITHER_COEF_DOWN_RIGHT 0
#define DITHER_COEF_DIVIDER 4
*/

// floyd steinberg matrix
#define DITHER_COEF_RIGHT 7
#define DITHER_COEF_DOWN 5
#define DITHER_COEF_DOWN_LEFT 3
#define DITHER_COEF_DOWN_RIGHT 1
#define DITHER_COEF_DIVIDER 16

static void ccrush_dither_shift_row(ccrush_dither_t *dither);

void ccrush_dither_init(ccrush_dither_t *dither, uint32_t width) {
    dither->width = width;
    dither->current_row = 0;
    dither->current_row_begin = 0;
    // calloc: error values initialised at 0
    dither->errors = calloc(width * DITHER_HEIGHT * 3, sizeof(int));
}

void ccrush_dither_clear(ccrush_dither_t *dither) {
    free(dither->errors);
    dither->errors = NULL;
}

static void ccrush_dither_shift_row(ccrush_dither_t *dither) {
    // wipe current row error values
    memset(dither->errors + dither->current_row_begin, 0, sizeof(int) * dither->width * 3);

    // shift current row
    dither->current_row = (dither->current_row + 1) % DITHER_HEIGHT;
    dither->current_row_begin = dither->current_row * dither->width * 3;
}

void ccrush_dither_diffuse_error(
    ccrush_dither_t *dither, uint32_t pixel_index,
    uint8_t *src_color, uint8_t *rounded_color) {
    // compute error
    int r_error = (int) src_color[CCRUSH_R] - rounded_color[CCRUSH_R];
    int g_error = (int) src_color[CCRUSH_G] - rounded_color[CCRUSH_G];
    int b_error = (int) src_color[CCRUSH_B] - rounded_color[CCRUSH_B];

    uint32_t col = pixel_index % dither->width;
    uint32_t error_index;

#if DITHER_COEF_DOWN
    error_index = dither->current_row_begin + col * 3;
    dither->errors[error_index + CCRUSH_R] += r_error * DITHER_COEF_DOWN;
    dither->errors[error_index + CCRUSH_G] += g_error * DITHER_COEF_DOWN;
    dither->errors[error_index + CCRUSH_B] += b_error * DITHER_COEF_DOWN;
#endif

#if DITHER_COEF_RIGHT
    if (col != dither->width - 1) {
        error_index = dither->current_row_begin + col * 3 + 3;
        dither->errors[error_index + CCRUSH_R] += r_error * DITHER_COEF_RIGHT;
        dither->errors[error_index + CCRUSH_G] += g_error * DITHER_COEF_RIGHT;
        dither->errors[error_index + CCRUSH_B] += b_error * DITHER_COEF_RIGHT;
    }
#endif

#if DITHER_COEF_DOWN_RIGHT || DITHER_COEF_DOWN_LEFT
    uint32_t next_row_begin = ((dither->current_row + 1) % DITHER_HEIGHT) * dither->width * 3;
#endif

#if DITHER_COEF_DOWN_RIGHT
    if (col != dither->width - 1) {
        error_index = next_row_begin + col * 3 + 3;
        dither->errors[error_index + CCRUSH_R] += r_error * DITHER_COEF_DOWN_RIGHT;
        dither->errors[error_index + CCRUSH_G] += g_error * DITHER_COEF_DOWN_RIGHT;
        dither->errors[error_index + CCRUSH_B] += b_error * DITHER_COEF_DOWN_RIGHT;
    }
#endif

#if DITHER_COEF_DOWN_LEFT
    if (col != 0) {
        error_index = next_row_begin + col * 3 - 3;
        dither->errors[error_index + CCRUSH_R] += r_error * DITHER_COEF_DOWN_LEFT;
        dither->errors[error_index + CCRUSH_G] += g_error * DITHER_COEF_DOWN_LEFT;
        dither->errors[error_index + CCRUSH_B] += b_error * DITHER_COEF_DOWN_LEFT;
    }
#endif

    if ((pixel_index + 1) % dither->width == 0) {
        ccrush_dither_shift_row(dither);
    }
}

#define CLAMP(x, ceil) ((x) > (ceil) ? (ceil) : ((x) < 0 ? 0 : (x)))

void ccrush_dither_apply_error(ccrush_dither_t *dither, uint32_t pixel_index, uint8_t *color, uint8_t *corrected_color) {
    uint32_t col = pixel_index % dither->width;
    uint32_t error_index = dither->current_row_begin + col * 3;
    int r_error = dither->errors[error_index + CCRUSH_R];
    int g_error = dither->errors[error_index + CCRUSH_G];
    int b_error = dither->errors[error_index + CCRUSH_B];

    int r = color[CCRUSH_R] + r_error / DITHER_COEF_DIVIDER;
    corrected_color[CCRUSH_R] = CLAMP(r, UINT8_MAX);
    int g = color[CCRUSH_G] + g_error / DITHER_COEF_DIVIDER;
    corrected_color[CCRUSH_G] = CLAMP(g, UINT8_MAX);
    int b = color[CCRUSH_B] + b_error / DITHER_COEF_DIVIDER;
    corrected_color[CCRUSH_B] = CLAMP(b, UINT8_MAX);
}
