#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "color.h"

/** Node of the colorspace octree */
typedef struct Node {
    /** Pointers to children */
    struct Node *children[8];
    bool is_leaf;
    uint32_t color_sum[3];
    uint32_t pixels_count;
    uint64_t error;
    /** Index of color in palette (initially null)*/
    uint8_t palette_index;
} Node;
