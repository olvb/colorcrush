#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "quantize.h"
#include "color.h"
#include "node.h"
#include "pool.h"
#include "heap.h"
#include "dither.h"

/**
@returns index of child in octree at a given @level for @color, by checking the value of the bit signifiant
at @level for color channel.
*/
static unsigned int child_index_for_color(unsigned int level, uint8_t *color) {
    int index = 0;
    int bit_index = 7 - level;
    int bit = 1 << bit_index;
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

/**
Fills @palette with colors taken from the leaves of the descendants of @node
@palette_size : size of the palette before (number of colors * 3)
@returns size of the palette after
*/
static unsigned int fill_palette(uint8_t *palette, unsigned int palette_size, Node *node) {
    if (node->is_leaf) {
        node->palette_index = palette_size / COLOR_CHANNELS_COUNT;
        palette[palette_size + COLOR_R] = round((double) node->color[COLOR_R] / (double) node->pixels_count);
        palette[palette_size + COLOR_G] = round((double) node->color[COLOR_G] / (double) node->pixels_count);
        palette[palette_size + COLOR_B] = round((double) node->color[COLOR_B] / (double) node->pixels_count);
        return palette_size + COLOR_CHANNELS_COUNT;
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
@returns palette index of @color by walking down @octree until reaching a leaf. Used when no
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

/**
@returns palette index of the palette color nearest to @color.
Much slower than index_of_cluster_color but necessary because in case of dithering error diffusion may
(and in most cases will) generate new colors that were not in the original image, which makes it impossible
to walk down the octree to find the cluster they belong to.
*/
static unsigned int index_of_nearest_color(uint8_t* palette, unsigned int palette_size, uint8_t *color) {
    unsigned int smallest_diff = UINT_MAX;
    unsigned int palette_index = 0;
    for (int i = 0; i < palette_size; i += COLOR_CHANNELS_COUNT) {
        unsigned int diff = color_diff(color, &palette[i]);
        // if we find the exact color, early return its index
        if (diff == 0) {
            return (unsigned int) i / COLOR_CHANNELS_COUNT;
        }
        if (diff < smallest_diff) {
            smallest_diff = diff;
            palette_index = (unsigned int) i ;
        }
    }
    
    assert(smallest_diff != UINT_MAX);
    return palette_index / COLOR_CHANNELS_COUNT;
}

void img_quantize(
    FlatImg *flat_img,
    unsigned int max_colors_count,
    unsigned int max_octree_depth,
    bool use_dither,
    IndexedImg *indexed_img
) {
    uint8_t *data = flat_img->data;
    uint32_t data_size = flat_img->width * flat_img->height * COLOR_CHANNELS_COUNT;

    // step 1: build the octree subdividing the color space
    Pool pool;
    pool_init(&pool);
    Node *octree = pool_next(&pool);

    unsigned int colors_count = 0;
    unsigned int max_heap_size = 0;
    for (uint32_t i = 0; i < data_size; i += COLOR_CHANNELS_COUNT) {
        Node *node = octree;
        uint8_t *color = &data[i];
        // walk down the octree creating new nodes as we go
        for (unsigned int j = 0; j < max_octree_depth; j++) {
            node->pixels_count++;
            int child_index = child_index_for_color(j, color);

            // new node
            if (node->children[child_index] == NULL) {
                node->children[child_index] = pool_next(&pool);
                // max depth reach, it's a leaf
                if (j == max_octree_depth - 1) {
                    node->children[child_index]->is_leaf = 1;
                    colors_count++;
                } else if (j == max_octree_depth - 2) {
                    max_heap_size++;
                }
            }
            node = node->children[child_index];
        }

        // leaf reached
        assert(node->is_leaf);
        node->pixels_count++;
        // update accumulated color values
        node->color[COLOR_R] += color[COLOR_R];
        node->color[COLOR_G] += color[COLOR_G];
        node->color[COLOR_B] += color[COLOR_B];
    }

    // step 2: reduce the octree (ie drop colors)
    Heap heap;
    heap_init(&heap, max_heap_size);
    /*
    We use heap sort, filling heap with nodes located at a given level and that are direct parents of leaves,
    sorting them by pixels count. We start with the deepest level, when the heap is empty we refill it with nodes
    from the upper level.
    This algorithm makes sure that deeper leaves are reduced before, which is good because the upper the leaf, the more
    it costs in quality to reduce it, because one unique color will be use for colors further appart.
    */
    // start from max depth, going up
    for (int i = max_octree_depth; i >= 0; i--) {
        assert(heap.size == 0);
        // fill heap with nodes from level
        heap_fill(&heap, octree, i);
        assert(heap.size > 0);
        while (heap.size > 0) {
            // pop node with lowest pixels count
            Node *node = heap_pop(&heap);
            // fold node leaves
            int removed_leaves_count = node_reduce(node);
            // keep track of number of colors in octree
            colors_count -= removed_leaves_count;

            // early out if we reached max_colors_count
            if (colors_count <= max_colors_count) {
                break;
            }
        }

        // early out if we reached max_colors_count
        if (colors_count <= max_colors_count) {
            break;
        }
    }
    heap_clear(&heap);

    // now the actual colors count is known, instanciate the indexed image
    indexed_img_init(indexed_img, flat_img->width, flat_img->height, (unsigned int) colors_count);
    // fill palette with colors from remaining leaves
    unsigned int palette_size = fill_palette(indexed_img->palette, 0, octree);
    
    // step 3: color index pixels
    uint32_t pixel_index = 0;
    if (!use_dither) {
        for (uint32_t i = 0; i < data_size; i += COLOR_CHANNELS_COUNT) {
            uint8_t* color = &data[i];
            uint8_t palette_index = index_of_cluster_color(octree, max_octree_depth, color);
            indexed_img->data[pixel_index] = palette_index;
            pixel_index++;
        }
    } else {
        Dither dither;
        dither_init(&dither, indexed_img->width);
        
        uint8_t corrected_color[COLOR_CHANNELS_COUNT];

        for (uint32_t i = 0; i < data_size; i += COLOR_CHANNELS_COUNT) {
            uint8_t *src_color = &data[i];
            dither_apply_error(&dither, pixel_index, src_color, corrected_color);

            uint8_t palette_index = index_of_nearest_color(indexed_img->palette, palette_size, corrected_color);
            indexed_img->data[pixel_index] = palette_index;
            uint8_t *palette_color = &indexed_img->palette[palette_index * COLOR_CHANNELS_COUNT];
            dither_diffuse_error(&dither, pixel_index, corrected_color, palette_color);
            
            pixel_index++;
            if (pixel_index % indexed_img->width == 0) {
                dither_next_row(&dither);
            }
        }
        dither_clear(&dither);
    }

    pool_clear(&pool);
}
