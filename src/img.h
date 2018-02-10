#pragma once
#include <stdint.h>

#define MAX_COLORS_COUNT 256

typedef struct FlatImg {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} FlatImg;

typedef struct IndexedImg {
    uint32_t width;
    uint32_t height;
    unsigned int colors_count;
    uint8_t *palette;
    uint8_t *data;
} IndexedImg;

void flat_img_init(FlatImg *img, uint32_t width, uint32_t height);
void flat_img_clear(FlatImg *img);

void indexed_img_init(IndexedImg *indexed_img, uint32_t width, uint32_t height, unsigned int colors_count);
void indexed_img_clear(IndexedImg *img);
