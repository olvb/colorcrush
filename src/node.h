#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "color.h"

/** Node of the colorspace octree */
typedef struct Node {
    /** Accumulated color values */
    unsigned int color[COLOR_CHANNELS_COUNT];
    /** Number of pixels in node and its descendants */
    uint32_t pixels_count;
    /** Index of color in palette */
    uint8_t palette_index;
    bool is_leaf;
    struct Node *children[8];
} Node;

/** @returns true if @node is immediate parent of one or more leaves */
bool node_is_leaf_parent(Node *node);
/** @returns number of leaves in all children of tree or node @node */
uint32_t node_count_leaves(Node *node);
/**
Tranforms an immediate leaves parent @node into a leaf by removing its leaves
and transfering on itself the cumulated color values of its leaves.
@returns leaves count reduction (ie. number of deleted leaves minus the newly created leaf)
*/
unsigned int node_reduce(Node *node);
