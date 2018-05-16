#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "color.h"

/** node_t of the colorspace octree */
typedef struct node_t {
    /** Pointers to children */
    struct node_t *children[8];
    bool is_leaf;
    uint32_t color_sum[3];
    uint32_t pixels_count;
    uint64_t error;
    /** Index of color in palette (initially null)*/
    uint8_t palette_index;
} node_t;
