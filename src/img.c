#include <stdlib.h>
#include <stddef.h>
#include "img.h"
#include "color.h"

void indexed_img_init(IndexedImg *img, uint32_t width, uint32_t height, unsigned int colors_count) {
    img->width = width;
    img->height = height;
    img->colors_count = colors_count;

    img->palette = malloc(sizeof(uint8_t) * colors_count * COLOR_CHANNELS_COUNT);
    img->data = malloc(sizeof(uint8_t) * width * height);
}

void indexed_img_clear(IndexedImg *img) {
    free(img->palette);
    img->palette = NULL;
    free(img->data);
    img->data = NULL;
}

void flat_img_init(FlatImg *img, uint32_t width, uint32_t height) {
    img->width = width;
    img->height = height;

    img->data = malloc(sizeof(uint8_t) * COLOR_CHANNELS_COUNT * width * height);
}

void flat_img_clear(FlatImg *img) {
    free(img->data);
    img->data = NULL;
}
