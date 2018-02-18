#include <stddef.h>
#include <stdlib.h>

#include "colorcrush.h"

void ccrush_indexed_img_init(ccrush_IndexedImg *img, uint32_t width, uint32_t height, unsigned int colors_count) {
    img->width = width;
    img->height = height;
    img->colors_count = colors_count;

    img->palette = malloc(sizeof(uint8_t) * colors_count * 3);
    img->data = malloc(sizeof(uint8_t) * width * height);
}

void ccrush_indexed_img_clear(ccrush_IndexedImg *img) {
    free(img->palette);
    img->palette = NULL;
    free(img->data);
    img->data = NULL;
}

void ccrush_flat_img_init(ccrush_FlatImg *img, uint32_t width, uint32_t height) {
    img->width = width;
    img->height = height;

    img->data = malloc(sizeof(uint8_t) * 3 * width * height);
}

void ccrush_flat_img_clear(ccrush_FlatImg *img) {
    free(img->data);
    img->data = NULL;
}
