#include "colorcrush.h"
#include "dither.h"
#include "heap.h"
#include "node.h"
#include "pool.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define COLOR_DIFF_THRESHOLD 10

static unsigned int ccrush_child_index_for_color(unsigned int level, uint8_t *color);
static unsigned int ccrush_fill_palette(uint8_t *palette, unsigned int palette_size, ccrush_node_t *node);
static uint8_t ccrush_index_of_cluster_color(ccrush_node_t *octree, int octree_depth, uint8_t *color);
static unsigned int ccrush_index_of_nearest_color(uint8_t *palette, unsigned int palette_size, uint8_t *color);

void ccrush_idx_img_init(ccrush_idx_img_t *img, uint32_t width, uint32_t height, unsigned int colors_count) {
    img->width = width;
    img->height = height;
    img->colors_count = colors_count;

    img->palette = malloc(sizeof(uint8_t) * colors_count * 3);
    img->data = malloc(sizeof(uint8_t) * width * height);
}

void ccrush_idx_img_clear(ccrush_idx_img_t *img) {
    free(img->palette);
    img->palette = NULL;
    free(img->data);
    img->data = NULL;
}

void ccrush_flat_img_init(ccrush_flat_img_t *img, uint32_t width, uint32_t height) {
    img->width = width;
    img->height = height;

    img->data = malloc(sizeof(uint8_t) * 3 * width * height);
}

void ccrush_flat_img_clear(ccrush_flat_img_t *img) {
    free(img->data);
    img->data = NULL;
}

void ccrush_img_quantize(
    ccrush_flat_img_t *flat_img,
    unsigned int max_colors_count,
    unsigned int octree_depth,
    bool use_dither,
    ccrush_idx_img_t *indexed_img) {
    uint8_t *data = flat_img->data;
    uint32_t data_size = flat_img->width * flat_img->height * 3;

    // Step 1: build an octree subdividing the color space of the image
    ccrush_pool_t pool;
    ccrush_pool_init(&pool);
    ccrush_node_t *octree = ccrush_pool_next(&pool);

    unsigned int max_heap_size = 0;
    unsigned int leaves_count = 0;
    // for each pixel, build the octree path down to the leaf of its color
    for (uint32_t i = 0; i < data_size; i += 3) {
        ccrush_node_t *node = octree;
        uint8_t *color = &data[i];

        // walk down the octree creating new nodes as we go
        for (unsigned int j = 0; j <= octree_depth; j++) {
            node->pixels_count++;
            node->color_sum[CCRUSH_R] += color[CCRUSH_R];
            node->color_sum[CCRUSH_G] += color[CCRUSH_G];
            node->color_sum[CCRUSH_B] += color[CCRUSH_B];

            if (j == octree_depth) {
                assert(node->is_leaf);
                continue;
            }

            unsigned int child_index = ccrush_child_index_for_color(j, color);
            ccrush_node_t *child = node->children[child_index];
            // init new node
            if (child == NULL) {
                child = ccrush_pool_next(&pool);
                node->children[child_index] = child;

                // max depth reach, it's a leaf
                if (j + 1 == octree_depth) {
                    child->is_leaf = true;
                    leaves_count++;
                } else {
                    max_heap_size++;
                }
            }

            node = child;
        }
    }

    // Step 2: prune the octree (ie drop colors)
    // compute error for all nodes
    ccrush_node_compute_error(octree);
    // sort nodes by error with heap sort
    ccrush_heap_t heap;
    ccrush_heap_init(&heap, max_heap_size);
    ccrush_heap_fill(&heap, octree);

    // reduce node, high error values first
    while (leaves_count > max_colors_count) {
        ccrush_node_t *node = ccrush_heap_pop(&heap);
        // we might encounter nodes that have been leaves in the heap before they were folded up, ignore them
        if (node->is_leaf) {
            continue;
        }
        int removed_leaves_count = ccrush_node_reduce(node);
        leaves_count -= removed_leaves_count;
    }
    ccrush_heap_clear(&heap);

    // now that the actual colors count is known, instanciate the indexed image
    ccrush_idx_img_init(indexed_img, flat_img->width, flat_img->height, leaves_count);
    // fill palette with colors from the remaining leaves
    unsigned int palette_size = ccrush_fill_palette(indexed_img->palette, 0, octree);

    // Step 3: assign palette colors to pixels
    uint32_t pixel_index = 0;
    if (!use_dither) {
        for (uint32_t i = 0; i < data_size; i += 3) {
            uint8_t *color = &data[i];
            uint8_t palette_index = ccrush_index_of_cluster_color(octree, octree_depth, color);
            indexed_img->data[pixel_index] = palette_index;

            pixel_index++;
        }
    } else {
        ccrush_dither_t dither;
        ccrush_dither_init(&dither, indexed_img->width);

        uint8_t dither_color[3];
        for (uint32_t i = 0; i < data_size; i += 3) {
            // apply dither error
            uint8_t *source_color = &data[i];
            ccrush_dither_apply_error(&dither, pixel_index, source_color, dither_color);

            // assign palette color
            uint8_t palette_index = ccrush_index_of_nearest_color(indexed_img->palette, palette_size, dither_color);
            indexed_img->data[pixel_index] = palette_index;

            // diffuse error
            uint8_t *palette_color = &indexed_img->palette[palette_index * 3];
            ccrush_dither_diffuse_error(&dither, pixel_index, dither_color, palette_color);
            pixel_index++;
        }
        ccrush_dither_clear(&dither);
    }

    // dont release before because we need the octree until the end!
    ccrush_pool_clear(&pool);
}

/**
@returns index of child in octree at a given @p level for @p color, by checking the value of the bit
signifiant at @p level for color channel.
*/
static unsigned int ccrush_child_index_for_color(unsigned int level, uint8_t *color) {
    unsigned int index = 0;
    unsigned int bit_index = 7 - level;
    unsigned int bit = 1 << bit_index;
    if (((color[CCRUSH_R]) & bit) == bit) {
        index |= (1 << 2);
    }
    if (((color[CCRUSH_G]) & bit) == bit) {
        index |= (1 << 1);
    }
    if (((color[CCRUSH_B]) & bit) == bit) {
        index |= (1 << 0);
    }
    return index;
}

/**
Fills @p palette with colors taken from the leaves of the descendants of @p node
@p palette_size : size of the palette before (number of colors * 3)
@returns size of the palette after
*/
static unsigned int ccrush_fill_palette(uint8_t *palette, unsigned int palette_size, ccrush_node_t *node) {
    if (node->is_leaf) {
        node->palette_index = palette_size / 3;

        palette[palette_size + CCRUSH_R] = round((double)
                                                     node->color_sum[CCRUSH_R]
            / (double) node->pixels_count);
        palette[palette_size + CCRUSH_G] = round((double)
                                                     node->color_sum[CCRUSH_G]
            / (double) node->pixels_count);
        palette[palette_size + CCRUSH_B] = round((double)
                                                     node->color_sum[CCRUSH_B]
            / (double) node->pixels_count);

        return palette_size + 3;
    }

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }
        palette_size = ccrush_fill_palette(palette, palette_size, node->children[i]);
    }

    return palette_size;
}

/**
@returns palette index of @p color by walking down @p octree until reaching a leaf. Used when no
dithering is applied (as we are sure to reach a leaf in this case).
*/
static uint8_t ccrush_index_of_cluster_color(ccrush_node_t *octree, int octree_depth, uint8_t *color) {
    ccrush_node_t *node = octree;
    for (int i = 0; i < octree_depth; i++) {
        unsigned int child_index = ccrush_child_index_for_color(i, color);
        if (node->children[child_index] == NULL) {
            continue;
        }

        node = node->children[child_index];
        if (node->is_leaf) {
            break;
        }
    }

    assert(node->is_leaf);
    return node->palette_index;
}

/**
@returns palette index of the palette color nearest to @p color.
Much slower than index_of_cluster_color but necessary because in case of dithering error diffusion may
(and in most cases will) generate new colors that were not in the original image, which makes it impossible
to walk down the octree to find the cluster they belong to.
*/
static unsigned int ccrush_index_of_nearest_color(uint8_t *palette, unsigned int palette_size, uint8_t *color) {
    uint32_t smallest_diff = UINT32_MAX;
    unsigned int palette_index = 0;
    for (int i = 0; i < palette_size; i += 3) {
        uint32_t diff = ccrush_color_diff_uint8(color, &palette[i]);
        // if we find the exact color or a super close one, early return its index
        //(our color diff method is not very acurate anyway)
        if (diff < COLOR_DIFF_THRESHOLD) {
            return i / 3;
        }
        if (diff < smallest_diff) {
            smallest_diff = diff;
            palette_index = i;
        }
    }

    assert(smallest_diff != UINT32_MAX);
    return palette_index / 3;
}
