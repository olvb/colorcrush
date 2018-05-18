#pragma once
#include "color.h"
#include <stdbool.h>
#include <stdint.h>

/** node of the colorspace octree */
typedef struct ccrush_node_t {
    /** Pointers to children */
    struct ccrush_node_t *children[8];
    bool is_leaf;
    uint32_t color_sum[3];
    uint32_t pixels_count;
    uint64_t error;
    /** Index of color in palette (initially null)*/
    uint8_t palette_index;
} ccrush_node_t;

void ccrush_node_compute_error(ccrush_node_t *node);
/**
Tranforms an immediate leaves parent @p node into a leaf by removing its leaves
and transfering on itself the cumulated color values of its leaves.
@returns leaves count reduction (ie. number of deleted leaves minus the newly created leaf)
*/
unsigned int ccrush_node_reduce(ccrush_node_t *node);
