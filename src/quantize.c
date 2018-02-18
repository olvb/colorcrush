#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "color.h"
#include "colorcrush.h"
#include "dither.h"
#include "heap.h"
#include "node.h"
#include "pool.h"

/**
@returns index of child in octree at a given @p level for @p color, by checking the value of the bit
signifiant at @p level for color channel.
*/
static unsigned int child_index_for_color(unsigned int level, uint8_t *color) {
    unsigned int index = 0;
    unsigned int bit_index = 7 - level;
    unsigned int bit = 1 << bit_index;
    if (((color[COLOR_R]) & bit) == bit) {
        index |= (1 << 2);
    }
    if (((color[COLOR_G]) & bit) == bit) {
        index |= (1 << 1);
    }
    if (((color[COLOR_B]) & bit) == bit) {
        index |= (1 << 0);
    }
    return index;
}

static void compute_octree_error(Node *node) {
    assert(!node->is_leaf);
    assert(node->error == 0);

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        Node *child = node->children[i];
        node->error += color_diff_uint32(node->color_sum, child->color_sum);
        if (!child->is_leaf) {
            compute_octree_error(child);
        }
    }
}

static void fill_heap(Heap *heap, Node *node) {
    assert(!node->is_leaf);

    heap_push(heap, node);

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        Node *child = node->children[i];
        if (!child->is_leaf) {
            fill_heap(heap, child);
        }
    }
}

/**
Tranforms an immediate leaves parent @p node into a leaf by removing its leaves
and transfering on itself the cumulated color values of its leaves.
@returns leaves count reduction (ie. number of deleted leaves minus the newly created leaf)
*/
static unsigned int reduce_node(Node *node) {
    assert(!node->is_leaf);

    unsigned int removed_leaves_count = 0;
    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        Node *child = node->children[i];
        // reduce child if not a leaf
        if (!child->is_leaf) {
            removed_leaves_count += reduce_node(child);
        }
        // fold leaf
        node->children[i] = NULL;
        removed_leaves_count += 1;
    }

    node->is_leaf = true;
    // number of leaves removed = number of leaves foled minus self (new leaf)
    if (removed_leaves_count > 0) {
        removed_leaves_count -= 1;
    }
    return removed_leaves_count;
}

/**
Fills @p palette with colors taken from the leaves of the descendants of @p node
@p palette_size : size of the palette before (number of colors * 3)
@returns size of the palette after
*/
static unsigned int fill_palette(uint8_t *palette, unsigned int palette_size, Node *node) {
    if (node->is_leaf) {
        node->palette_index = palette_size / 3;

        palette[palette_size + COLOR_R] = round((double)node->color_sum[COLOR_R] / (double)node->pixels_count);
        palette[palette_size + COLOR_G] = round((double)node->color_sum[COLOR_G] / (double)node->pixels_count);
        palette[palette_size + COLOR_B] = round((double)node->color_sum[COLOR_B] / (double)node->pixels_count);

        return palette_size + 3;
    }

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }
        palette_size = fill_palette(palette, palette_size, node->children[i]);
    }

    return palette_size;
}

/**
@returns palette index of @p color by walking down @p octree until reaching a leaf. Used when no
dithering is applied (as we are sure to reach a leaf in this case).
*/
static uint8_t index_of_cluster_color(Node *octree, int octree_depth, uint8_t *color) {
    Node *node = octree;
    for (int i = 0; i < octree_depth; i++) {
        unsigned int child_index = child_index_for_color(i, color);
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

#define COLOR_DIFF_THRESHOLD 10
/**
@returns palette index of the palette color nearest to @p color.
Much slower than index_of_cluster_color but necessary because in case of dithering error diffusion may
(and in most cases will) generate new colors that were not in the original image, which makes it impossible
to walk down the octree to find the cluster they belong to.
*/
static unsigned int index_of_nearest_color(uint8_t *palette, unsigned int palette_size, uint8_t *color) {
    uint32_t smallest_diff = UINT32_MAX;
    unsigned int palette_index = 0;
    for (int i = 0; i < palette_size; i += 3) {
        uint32_t diff = color_diff_uint8(color, &palette[i]);
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

void ccrush_img_quantize(
    ccrush_FlatImg *flat_img,
    unsigned int max_colors_count,
    unsigned int octree_depth,
    bool use_dither,
    ccrush_IndexedImg *indexed_img) {
    uint8_t *data = flat_img->data;
    uint32_t data_size = flat_img->width * flat_img->height * 3;

    // Step 1: build an octree subdividing the color space of the image
    Pool pool;
    pool_init(&pool);
    Node *octree = pool_next(&pool);

    unsigned int max_heap_size = 0;
    unsigned int leaves_count = 0;
    // for each pixel, build the octree path down to the leaf of its color
    for (uint32_t i = 0; i < data_size; i += 3) {
        Node *node = octree;
        uint8_t *color = &data[i];

        // walk down the octree creating new nodes as we go
        for (unsigned int j = 0; j <= octree_depth; j++) {
            node->pixels_count++;
            node->color_sum[COLOR_R] += color[COLOR_R];
            node->color_sum[COLOR_G] += color[COLOR_G];
            node->color_sum[COLOR_B] += color[COLOR_B];

            if (j == octree_depth) {
                assert(node->is_leaf);
                continue;
            }

            unsigned int child_index = child_index_for_color(j, color);
            Node *child = node->children[child_index];
            // init new node
            if (child == NULL) {
                child = pool_next(&pool);
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
    compute_octree_error(octree);
    // sort nodes by error with heap sort
    Heap heap;
    heap_init(&heap, max_heap_size);
    fill_heap(&heap, octree);

    // reduce node, high error values first
    while (leaves_count > max_colors_count) {
        Node *node = heap_pop(&heap);
        // we might encounter nodes that have been leaves in the heap before they were folded up, ignore them
        if (node->is_leaf) {
            continue;
        }
        int removed_leaves_count = reduce_node(node);
        leaves_count -= removed_leaves_count;
    }
    heap_clear(&heap);

    // now that the actual colors count is known, instanciate the indexed image
    ccrush_indexed_img_init(indexed_img, flat_img->width, flat_img->height, leaves_count);
    // fill palette with colors from the remaining leaves
    unsigned int palette_size = fill_palette(indexed_img->palette, 0, octree);

    // Step 3: assign palette colors to pixels
    uint32_t pixel_index = 0;
    if (!use_dither) {
        for (uint32_t i = 0; i < data_size; i += 3) {
            uint8_t *color = &data[i];
            uint8_t palette_index = index_of_cluster_color(octree, octree_depth, color);
            indexed_img->data[pixel_index] = palette_index;

            pixel_index++;
        }
    } else {
        Dither dither;
        dither_init(&dither, indexed_img->width);

        uint8_t dither_color[3];
        for (uint32_t i = 0; i < data_size; i += 3) {
            // apply dither error
            uint8_t *source_color = &data[i];
            dither_apply_error(&dither, pixel_index, source_color, dither_color);

            // assign palette color
            uint8_t palette_index = index_of_nearest_color(indexed_img->palette, palette_size, dither_color);
            indexed_img->data[pixel_index] = palette_index;

            // diffuse error
            uint8_t *palette_color = &indexed_img->palette[palette_index * 3];
            dither_diffuse_error(&dither, pixel_index, dither_color, palette_color);
            pixel_index++;
        }
        dither_clear(&dither);
    }

    // dont release before because we need the octree until the end!
    pool_clear(&pool);
}
