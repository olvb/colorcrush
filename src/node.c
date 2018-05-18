#include "node.h"
#include "color.h"
#include <assert.h>
#include <stdlib.h>

void ccrush_node_compute_error(ccrush_node_t *node) {
    assert(!node->is_leaf);
    assert(node->error == 0);

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        ccrush_node_t *child = node->children[i];
        node->error += ccrush_color_diff_uint32(node->color_sum, child->color_sum);
        if (!child->is_leaf) {
            ccrush_node_compute_error(child);
        }
    }
}

unsigned int ccrush_node_reduce(ccrush_node_t *node) {
    assert(!node->is_leaf);

    unsigned int removed_leaves_count = 0;
    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        ccrush_node_t *child = node->children[i];
        // reduce child if not a leaf
        if (!child->is_leaf) {
            removed_leaves_count += ccrush_node_reduce(child);
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
